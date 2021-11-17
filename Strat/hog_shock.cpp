#include <bits/stdc++.h>
// extern "C"{
//     #include <cblas.h>
//     #include <stdio.h>
// }
#include "s_rapidjson/document.h"
#include "connectorcrtp.h"
#include "ConnectorConfig/ConnectorInit.hpp"
#include "Helpers/helper_structs.hpp"
#include "Helpers/config_reader.hpp"
#include "Helpers/UserEvent.h"
#include "ParamsStrat.hpp"
#include "Helpers/SignalHandler.hpp"
#include "SymbolLoader/CMESymbolLoader.hpp"
#include "UniverseMaker/UniverseHandler.hpp"
#include "Execution/ExecutionManager.h"
#include "Spot/cme_spot_pricing.hpp"
#include "Spot/nse_spot_pricing.hpp"
#include "CallBackManager/GreekHandler.hpp"
#include "CallBackManager/SpotPriceHandler.hpp"
#include "CallBackManager/OptPriceHandler.hpp"
#include "CallBackManager/MDHandler.hpp"
#include "CallBackManager/MDFeedHandler.hpp"
#include "CallBackManager/TimerHandler.hpp"
#include "CallBackManager/GreekTimerHandler.hpp"
#include "CallBackManager/ModelTimerHandler.hpp"
#include "ValidityChecks/MDValid.hpp"
#include "RiskChecker/RiskChecker.hpp"
#include "CallBackManager/StatsPrinter.hpp"
#include "CallBackManager/RiskCheckTimer.hpp"
#include "CallBackManager/RiskListener.hpp"
#include "Responses/RespHandler.hpp"
#include "Stats/StatsGen.hpp"
#include "PnLCalculator/PnlCalculator.hpp"
#include "CallBackManager/PriceMapListener.hpp"
#include "Instruments/ComplexInstruments.hpp"
#include "ShockHandler.hpp"
#include "ShockHandler.cpp"
// #include "RandIntTrader.hpp"
// #include "CallBackManager/TradeHandler.hpp"
#include "ExitHandler/ExitHandler.hpp"
#include "Execution/ExecutionManager.h"
#include "MarginLotSetter/MarginLotReader.hpp"
#include <algorithm>    // std::transform
#include "DumpModel.hpp"
#include "DataDumper.hpp"
#include "DataDumper.cpp"
#include "ParamsStrat.hpp"
// #include "RollingGreeks.hpp"
// #include "RollingGreeks.cpp"
#include "Helpers/rolling_stats.hpp"
#include "csv_reader.hpp"
// #include "util.h"
#include "agshocklib.h"
// #include "util.h"
// extern "C"{
// #include "Indicators/TaLib/ta_libc.h"
// #include "Indicators/ta_libc.h"
// #include "TaLib/ta_global.h"
// }

// #include <ta_func.h>
// #include "include/armadillo"
// #include "Talib/include/ta-lib/ta_libc.h"
using namespace illuminati::md;
// using namespace arma; 

uint64_t timestamp_g = 0;
uint64_t *Tim::Helpers::timestamp_ptr = &timestamp_g;
int *Tim::Helpers::date_ptr = NULL;
int *Tim::Helpers::pid_ptr = NULL;
std::string *Tim::Helpers::folder_ptr = NULL;
std::string Tim::Helpers::underlying = "";
int Tim::Helpers::sample_period = 1;
Tim::ExchangeName Tim::Helpers::exchange = ExchangeName::NSE;
double Tim::Helpers::feed_multiplier = 0.5;
double Tim::Helpers::tick_size = 0.01;
double MIN_TICK_SIZE = 0.01;
double Tim::Helpers::commission = 0;
double Tim::Helpers::interest_rate = 0.01;
std::vector<std::vector<std::string>> csv_pos;
bool Tim::CombinedOrderManager::freezed_on_reject = false;
int64_t Tim::CombinedOrderManager::freeze_end_time = E9*E9;
int64_t Tim::CombinedOrderManager::error_code = -1;

ConnectorType *con;
Tim::ConfigReader *configReader_ptr;
Tim::ExecutionManager* exec_logic;
Tim::ModelHandler *modelHandler;
// Tim::RandIntTrader* trader;
//std::map<std::string, Tim::SpotPricing*> spot_obj_map;
std::map<std::string, Tim::SpotPricing*> spot_obj_map;
Tim::UniverseHandler* unHand;
Tim::StatsGen* stats;
Tim::PNL* pnl_calc;
Tim::MarginStats* margin_calc;
Tim::OverallOrderManager* OOM;
Tim::RespHandler* respHand;
Tim::RiskChecker* risk_checker;
// Tim::ExitHandler* exitHand;
Baryons::CallBackManager* CBM;
Baryons::MDHandler* mdHand;
Baryons::MDFeedHandler* mdFeedHand;
Baryons::OptPriceHandler* optHand;
Baryons::SpotPriceHandler* spotHand;
Baryons::PriceMap* priceHand;
Baryons::TimerHandler* timerHand;
Baryons::GreekTimerHandler* greekTimerHand;
// Baryons::RollingGreeks* rgTimerHand;
Baryons::RiskCheckTimer* riskHand;
Baryons::DataDumper* dataDumper;
Baryons::GreekHandler* greekHandler;
// Baryons::StrategyTimerHandler* strategyTimerHand;
Baryons::StatsTimeHandler* statsHand;
Baryons::ModelTimerHandler* modelTimerHand;

Baryons::ShockHandler* shockHandler;
std::vector<double> model;

// Tim::ExpectedShock* shock_ind_;
// Baryons::IndicatorHandler* indHand;

std::vector<Tim::RespData> resp_vector;
std::map<int, Tim::execution_data> executed_position;
std::map<int, Tim::TradeVals> tradeBook;
std::map<uint32_t, Tim::order_data> order_map;
std::map<int, int> lot_size_map;
std::map<int, Tim::Options*> option_data;

Tim::Paramloader* params;
Tim::StratParams* strat_params;
Tim::GlobalParams* global_params;

Tim::DumpParams* dump_params;
std::string prev_day_pos_file_path="";
std::string prev_day_pnl_file_path="";
double multiplier = 0.0;


void createGraph(int date){
    
    uint64_t sample_starttime = Tim::Helpers::epochTimeInNanos(*Tim::Helpers::date_ptr, 
        global_params->start_hour, global_params->start_min, 0, 0);
    multiplier = Tim::Helpers::feed_multiplier;
    CBM = new Baryons::CallBackManager();
    CBM->setInstance(CBM);
    
    mdHand = new Baryons::MDHandler(unHand, global_params->start_hour, global_params->start_min, 
        global_params->end_hour, global_params->end_min, date);  
    mdFeedHand = new Baryons::MDFeedHandler(unHand, global_params->start_hour, global_params->start_min, 
        global_params->end_hour, global_params->end_min, date);  
    optHand = new Baryons::OptPriceHandler(unHand);
    
    spotHand = new Baryons::SpotPriceHandler(unHand, &spot_obj_map);

    priceHand = new Baryons::PriceMap();
    pnl_calc = new Tim::PNL(unHand, &resp_vector, &priceHand->price_map->market_data_map, 
        prev_day_pnl_file_path);
    auto it = lot_size_map.begin();
    if (it == lot_size_map.end()) {
        std::cerr << "HERE " << std::endl;
        exit(-1);
    }
    modelHandler = new Tim::DumpModel(sample_starttime, unHand, params, &tradeBook, stats);
    modelTimerHand = new Baryons::ModelTimerHandler(sample_starttime, 10*SECOND, unHand, params, modelHandler);
    // shock_ind_ = new Tim::ExpectedShock(unHand, spot_obj);
    // indHand = new Baryons::IndicatorHandler(0, 1*1e9, rand_ind_);
    stats = new Tim::StatsGen(unHand, pnl_calc, margin_calc, &tradeBook, &spot_obj_map); 
    //trader = new Tim::RandIntTrader(unHand, params, stats, margin_calc,
    //    &tradeBook, &spot_obj_map, date, 1, shock_ind_);
    // int freq = dump_params->dump_frequency;
    dataDumper = new Baryons::DataDumper(date, sample_starttime,  SECOND,unHand,
		params, dump_params,&spot_obj_map, strat_params->delta_lower_cap, strat_params->delta_upper_cap, 
        modelHandler, csv_pos);
    timerHand = new Baryons::TimerHandler(unHand, SECOND, sample_starttime, 
        global_params->start_hour, 
        global_params->start_min, 
        global_params->end_hour, 
        global_params->end_min, 
        date);
        
    shockHandler = new Baryons::ShockHandler(date, sample_starttime, 1*SECOND, &tradeBook,
                unHand, params,
                &spot_obj_map, margin_calc, "/home/pranka/");
    // strategyTimerHand = new Baryons::StrategyTimerHandler(date, sample_starttime, SECOND, unHand, 
    //     trader, params, &tradeBook, &executed_position);
    // strategyTimerHand->ind_trader(trader);
    greekTimerHand = new Baryons::GreekTimerHandler(sample_starttime, SECOND, 
        unHand, 0.01, &spot_obj_map, params);
    // riskHand = new Baryons::RiskCheckTimer(sample_starttime, strat_params->trade_timer,//SECOND,  //, strat_params->trade_timer, 
        // risk_checker, pnl_calc, unHand, &tradeBook);
    statsHand = new Baryons::StatsTimeHandler(stats, sample_starttime, 30*SECOND, pnl_calc);
    CBM->addEdge(mdHand, optHand);
    CBM->addEdge(mdHand, spotHand);
    CBM->addEdge(mdHand, priceHand);
    CBM->addEdge(mdFeedHand, priceHand);
    // CBM->addEdge(optHand, greekHand);
    // CBM->addEdge(spotHand, greekHand);
    
    //*****************************************************************
    //Sahaj Changes//
    // rgTimerHand = new Baryons::RollingGreeks(sample_starttime, SECOND, 
    //     unHand, (Tim::ParamloaderDerived*)params);
    // greekTimerHand->addGreekTimerListener(rgTimerHand);
    //*****************************************************************
    CBM->addEdge(timerHand, greekTimerHand);
    // CBM->addEdge(timerHand, shockHandler);
    CBM->addEdge(greekTimerHand, modelTimerHand);
    // CBM->addEdge(greekTimerHand, rgTimerHand);
    // CBM->addEdge(timerHand, riskHand);
    // CBM->addEdge(timerHand, strategyTimerHand);
    // CBM->addEdge(timerHand, statsHand);
    // CBM->addEdge(riskHand, strategyTimerHand);
    // CBM->addEdge(greekTimerHand, strategyTimerHand);
     CBM->addEdge(timerHand, dataDumper);

     CBM->addEdge(greekTimerHand, dataDumper);
    CBM->addEdge(greekTimerHand, modelTimerHand);
    // CBM->addEdge(greekTimerHand, shockHandler);
    // CBM->addEdge(greekTimerHand,strategyTimerHand);
    // CBM->addEdge(rgTimerHand, dataDumper);


    mdHand->addMDListener(optHand);
    mdHand->addMDListener(spotHand);
    mdHand->addMDListener(priceHand);
    mdFeedHand->addMDFeedListener(priceHand);

    timerHand->addTimerListener(greekTimerHand);
    // timerHand->addTimerListener(shockHandler);
    // timerHand->addTimerListener(riskHand);
    // timerHand->addTimerListener(statsHand);
     timerHand->addTimerListener(dataDumper);
    // timerHand->addTimerListener(strategyTimerHand);
    greekTimerHand->addGreekTimerListener(modelTimerHand);
    // greekTimerHand->addGreekTimerListener(strategyTimerHand);
    // greekTimerHand->addGreekTimerListener(rgTimerHand);
     greekTimerHand->addGreekTimerListener(dataDumper);
    //  greekTimerHand->addGreekTimerListener(shockHandler);
    // greekTimerHand->addGreekTimerListener(strategyTimerHand);
    // riskHand->addRiskListener(strategyTimerHand);
    //spotHand->addSpotPx_listener(greekHand);
    //optHand->addOptPx_listener(greekHand);

    timerHand->ResolveMinTime();
}

void callUmsgListeners(const illuminati::md::MarketUpdateNew* md) {
    int s = md->m_lastTradedTime;
    int c = md->m_seqnum;
    const char* loc1 = reinterpret_cast<const char*>(md->m_bidUpdates);
    const char* loc2 = reinterpret_cast<const char*>(md->m_askUpdates);
    Baryons::UserEvent user_event_;
    user_event_.code = c;
    user_event_.pid = s;
    strncpy(user_event_.msg1, loc1, 47);
    strncpy(user_event_.msg2, loc2, 47);
    user_event_.msg1[47] = user_event_.msg2[47] = '\0';
    // for (size_t i = 0, sz = umsg_listeners_.size(); i < sz; i++) {
    //     umsg_listeners_[i]->onUmsg(&user_event_);
    // }
    
    if(user_event_.pid != *Tim::Helpers::pid_ptr) {
        return;
    }
    if (user_event_.code == 0) {
        params->resetParam(user_event_.msg1, user_event_.msg2);
        if(std::strcmp(user_event_.msg1, "margin")==0) {
            double margin = std::stod(user_event_.msg2);
            margin_calc->reInitialize(margin);
        }
        // trader->reInitialize();
    }
}


void OI_map(std::map<std::string, uint16_t>& symbol_id_map) {
//     for (auto it=symbol_id_map.begin(); it!=symbol_id_map.end(); ++it){
//         if(std::strncmp(it->first, "OI_", 3) == 0) {
//             std::string symbol = std::string(it->first).substr(3);
//             if(unHand->TradeUniverseIDs.find(symbol) != unHand->TradeUniverseIDs.end()){
//                 auto &ts = unHand->TradeUniverseIDs[symbol];
//                 if(ts->isOption) {
//                    ts->opt->updateOI(md->m_yield);
//                    //std::cout << "Opt: " << md->m_symbol << " " << md->m_timestamp << " " << md->m_yield << std::endl;
//                 }
//                 if(ts->isFut) {
//                    ts->fut->updateOI(md->m_yield);
//                    //std::cout << "Fut: " << md->m_symbol << " " << md->m_timestamp << " " << md->m_yield << std::endl;
//                 }
//             }
//         }
//     }
}

void PrintBook(){

    for(auto itr = unHand->TradeUniverse.begin(); itr != unHand->TradeUniverse.end(); itr++){
        if(itr->second->isOption){
            itr->second->opt->printOption();
        }
    }
}


static void md_cb(uint16_t symbolID, const illuminati::md::MarketUpdateNew* md){
 

    if ((md->m_timestamp == 0)) {
       if (md->m_exchangeName == 0 && md->m_exchTS == 0) {
            // usermsg
             callUmsgListeners(md);
         }
        else {
            // std::cerr << ":: ignoring MD >>> " <<  md->toString() << std::endl;
        }
        return;
    }

    bool isOkay = false;
    if (md->m_updateType=='N' && md->m_feedType=='W'){
        std::cout << "Update:" << md->m_symbol << "," << md->m_timestamp << "," << md->m_newPrice << std::endl;
        mdFeedHand->setMDFeed_(md, symbolID, &isOkay);
        CBM->notifyAll();
        return;
    }
    if (Tim::Helpers::exchange == Tim::ExchangeName::US) {
		auto spot_obj = unHand->TradeUniverseIDs[symbolID]->spot_obj;
		double underlier_bid = md->getUnderlierBid();
		double underlier_ask = md->getUnderlierAsk();
		double underlier_trade_px = (underlier_bid + underlier_ask)/2.0;
		spot_obj->updateSpotPrice("", underlier_bid, underlier_ask, underlier_trade_px);
	}
    mdHand->setMD_(md, symbolID, &isOkay);
    CBM->notifyAll();
//    if (md->m_feedType=='i'){
////    		   mdHand->setMD_(md, symbolID, &isOkay);
////    		   CBM->notifyAll();
////    		   return
//
//    	           std::cout << "Update:" << md->m_feedType  << " " << md->m_yield << " " << md->m_newPrice << std::endl;
//    	      }

    // trader->timer_man->updateTimers(md->m_timestamp);
//    bool append_flag = false;
//    if(isOkay && append_flag){
//        //params.curr_port_delta = stats->
//        //CIMaker = new Tim::ComplexInstruments(Tim::ComplexInstrumentType::CISurface, unHand->TradeUniverseIDs, params);
//        //CIMaker->printComplexInstrument();
//    }
//	std::string fname = "/home/pranka/work/Strat_Init/Strat/temp/md2.txt"; //+ std::to_string(*Tim::Helpers::date_ptr)+ ".csv";
//	static std::ofstream output3;
//	static int counter = 0;
//
//    output3.open(fname, std::ios_base::app); // append instead of overwrite
//    std::string sym = md->m_symbol;
//    uint64_t dump_st = Tim::Helpers::epochTimeInNanos(*Tim::Helpers::date_ptr,
//            global_params->start_hour, global_params->start_min, 0, 0);
//    uint64_t dump_et = Tim::Helpers::epochTimeInNanos(*Tim::Helpers::date_ptr,
//                global_params->end_hour, global_params->end_min, 0, 0);
   // if (counter == 0){
    //		output3<< "Time,Price\n";
    //		counter = 500;
   // 	}
    //if (md->m_timestamp >= dump_st && md->m_timestamp <=dump_et){
    //	if(sym.find("INDIAVIX") != std::string::npos)
//    output3 << Tim::Helpers::getHumanReadableTime(md->m_timestamp) << "," <<md->m_feedType << ", " << md->m_symbol << "\n";

    

//    output3.close();
}


static void timer_cb(uint64_t timestamp){
    // std::cout << "Timer cb timestamp: " << Tim::Helpers::getHumanReadableTime(timestamp) << std::endl;
    // std::cout << "Timer cb timestamp: " << Tim::Helpers::getHumanReadableTime(timestamp) << std::endl;
    //auto freq = std::min(uint64_t(timerHand->unit_time_this*SECOND), strat_params->improved_order_level_timeout);
    uint64_t newtime = ((timestamp/timerHand->unit_time_this) + 1) * timerHand->unit_time_this;
    timestamp_g = newtime; 
    static uint64_t params_timer = 0;
    if (newtime >= params_timer + strat_params->timer_unit){
        params_timer += strat_params->timer_unit;
        // params->reInitializeParams();
        // margin_calc->reInitialize(strat_params->margin);
        // trader->reInitialize();
        // std::cout << "margin_calc->init_margin:" << margin_calc->init_margin <<  " " << margin_calc->actual_margin << " " << params_timer << std::endl; 
    }
    timerHand->setTimer_(timestamp);
 
    //Update strat_params
    MyLogger::log(TEMPFILE, "main::timer_cb |", "Time: ", timestamp);
    static uint64_t timestamp_CBM = 0;
    uint64_t sample_freq = (timerHand->unit_time_this)*SECOND;
    if (timestamp >= timestamp_CBM + sample_freq){
        timestamp_CBM = ((timestamp)/sample_freq)*sample_freq;
        CBM->notifyAll();
    }
     
    static uint64_t timestamp_EM = 0;
    static bool send_alerts = true;
    if (strat_params->freeze_on_reject==true && send_alerts) {
        if (Tim::CombinedOrderManager::freezed_on_reject) {
            if (Tim::CombinedOrderManager::error_code == 100107) {
                Tim::CombinedOrderManager::freeze_end_time = *Tim::Helpers::timestamp_ptr + 2*SECOND;
            }
            else {
                Tim::CombinedOrderManager::freeze_end_time = *Tim::Helpers::timestamp_ptr - 600*SECOND;
                Tim::CombinedOrderManager::freeze_end_time = *Tim::Helpers::timestamp_ptr + 600*SECOND;
            }
            Aart::sendAlert("Got a reject in Strategy pid:"          + 
            std::to_string(*Tim::Helpers::pid_ptr) + ". Error_code:" + 
            std::to_string(Tim::CombinedOrderManager::error_code));
            send_alerts = false;
        }
    }
    else if (strat_params->freeze_on_reject==false || 
        Tim::CombinedOrderManager::freeze_end_time <=  *Tim::Helpers::timestamp_ptr){
        send_alerts = true;
        Tim::CombinedOrderManager::error_code = -1;
        Tim::CombinedOrderManager::freezed_on_reject = false;
    }
    con->addTimer(newtime, 0);
    return;
}

static void resp_cb(uint16_t symbolID, const illuminati::infra::ResponseMsg* resp){
    auto ins = unHand->TradeUniverseIDs[symbolID];
    respHand->processResponse(resp, ins);
}


static void nomarketdata_callback(){

}

std::vector<std::string> ticker_of_interest = {};

int main(int argc, char* argv[]){
    Tim::Options::initializeMemPool();
    Tim::SimpleInstruments::initializeMemPool();
    // Tim::CI_Constrained_Surface_4::initializeMemPool();
    Tim::ConfigReader configReader = Tim::ConfigReader::GetInstance(argc, argv);
    configReader_ptr = &configReader;
    Tim::Helpers::folder_ptr = &configReader.output_folder_path;
    Aart::SignalHandler(configReader.mode);

    std::string pos_file_path = "";
    std::string pnl_file_path = "";
    std::string trade_info_file_path = "";
    int core = -1;
    if (configReader_ptr->mode == 3) {
       core = 15;
   	}
    MyLogger::InitInstance(configReader.symbol_file_path, configReader.date, configReader.pid, core);
    // MyLogger::GetInstance(configReader.symbol_file_path, configReader.date, configReader.pid, 15);
    MyLogger::log(TRADEFILE, "STRATEGY: CME SHORT GAMMA for Date: ", configReader.date, "\n\n");
    MyLogger::log(TEMPFILE, "main::init |", configReader.to_string());

    Tim::Helpers::date_ptr = &configReader.date;
    Tim::Helpers::pid_ptr = &configReader.pid;
    FILE* fp = fopen(configReader.symbol_file_path.c_str(), "r");
    Baryons::rapidjson::FileStream is(fp);
    Baryons::rapidjson::Document path_resolver;
    path_resolver.ParseStream<0>(is);
    
    std::string connector_file_path("");
    if (path_resolver.HasMember("connector_file_path")){
        const Baryons::rapidjson::Value& connector_path = path_resolver["connector_file_path"];
        connector_file_path = connector_path.GetString();
        Tim::Helpers::path_wrapper(connector_file_path);
    }
    else {
        std::cout << "Error: connector_file_path was not set" << std::endl;
        exit(0);
    }
    
    MyLogger::log(TRADEFILE, "Connector Path Used: ", connector_file_path, "\n\n");
    MyLogger::log(TRADEFILE, "Dump Paths Resolved from JSON file|");
    
    bool pos_test = false, pnl_test = false, trade_test = false;
    if(path_resolver.HasMember("pos_file_path")){
        const Baryons::rapidjson::Value& pos_path= path_resolver["pos_file_path"];
        std::string pos_path_suffix = pos_path.GetString();
        pos_file_path += pos_path_suffix + "ExSGposFile_" + std::to_string(configReader.pid) +
            "_" + std::to_string(configReader.date) + ".txt";
        pos_test = true;
    }
    else{
        MyLogger::log(TRADEFILE, "pos_file_path: UNDEFINED!");
    }

    if(path_resolver.HasMember("pnl_file_path")){
        const Baryons::rapidjson::Value& pnl_path= path_resolver["pnl_file_path"];
        std::string pnl_path_suffix = pnl_path.GetString();
        pnl_file_path += pnl_path_suffix + "ExSGpnlFile_" + std::to_string(configReader.pid) +
            "_" + std::to_string(configReader.date) + ".txt";
        pnl_test = true;
    }
    else{
        MyLogger::log(TRADEFILE, "pnl_file_path: UNDEFINED!");
    }
    
    if(path_resolver.HasMember("trade_info_path")){
        const Baryons::rapidjson::Value& trade_info_path = path_resolver["trade_info_path"];
        trade_info_file_path += trade_info_path.GetString();
        trade_test = true;
    }
    if(pos_test)
        MyLogger::log(TRADEFILE, "pos_file_path: ", pos_file_path);
    if(pnl_test)
        MyLogger::log(TRADEFILE, "pnl_file_path: ", pnl_file_path);
    if(trade_test)
        MyLogger::log(TRADEFILE, "trade_info_file_path", trade_info_file_path);

    MyLogger::log(TRADEFILE, "\n");
    try{
        params = new Tim::ParamloaderDerived(configReader.param_file_path);
    }
    catch(...){
        std::cout<<"Exception| params_pointer could not be initialized"<<std::endl;
    }
    strat_params = ((Tim::ParamloaderDerived*)params)->strat_params;
    dump_params = ((Tim::ParamloaderDerived*)params)->dump_params;
    global_params = params->global_params;
    Tim::Helpers::exchange = global_params->exchange=="CME" ? Tim::ExchangeName::CME : Tim::ExchangeName::NSE;

    int MIN_SYM_SIZE_CME = 4;
    std::map<std::string, int> lot_size_symbol_map;
    Tim::CMESymbolLoader symLoad(configReader.symbol_file_path, MIN_SYM_SIZE_CME, configReader.date, 
    prev_day_pos_file_path, params);
    symLoad.displayStratSymbols();
    
    // std::cout << "Dump Frequency: " << (*dump_params).dump_frequency << std::endl;
    
    struct tm t = {0}; 
    t.tm_year = Tim::Helpers::getYear(configReader.date);
    t.tm_mon = Tim::Helpers::getMonth(configReader.date);
    t.tm_mday = Tim::Helpers::getDay(configReader.date);
    t.tm_hour = 9;
    t.tm_min = 15;
    t.tm_sec = 0;
    std::time_t timeSinceEpoch = mktime(&t);
    uint64_t starttime = long(timeSinceEpoch)*E9; 
    timestamp_g = starttime;  
    

    std::map<std::string, std::vector<Tim::GetInfoFields*>> mySyms;
    Tim::ExpiryTime expiry = Tim::ExpiryTime::EXPIRY_1;
    symLoad.getStratSymbols(mySyms, expiry);

    // std::map<std::string, std::vector<Tim::GetInfoFields*>> mySyms;
    // const Baryons::rapidjson::Value& underliers = path_resolver["Interested_Instruments"];
    // assert(underliers.IsArray());
    // for (auto itr = underliers.Begin(); itr != underliers.End(); ++itr){
    // 	const Baryons::rapidjson::Value& UNL_doc= *itr;
    //     assert(UNL_doc.IsObject()); // each attribute is an object
    //     if(UNL_doc.HasMember("Underlier") && UNL_doc.HasMember("Path"))
    //     {
	// 	    const Baryons::rapidjson::Value& IsOption_name = UNL_doc["IsOption"];
    //         if(IsOption_name.GetBool())
    //         {
    //             const Baryons::rapidjson::Value& expiry_ = UNL_doc["expiry"];
    //             expiry  = (Tim::ExpiryTime) expiry_.GetInt();
    //             symLoad.getStratSymbols(mySyms, expiry);
	// 	    }
	//     }
    // }


    std::vector<std::string> traded_ticker_list;
    std::vector<std::string> ticker_vec;
    std::string ticker_list="";
    std::string underlier_for_this_strat = "";
    std::string ticker10len_list = "";

    // for (auto itr = spot_obj->spot_constituents.begin();
    //         itr != spot_obj->spot_constituents.end(); itr++){

    //     std::string spot_sym = std::string(itr->first);
    //     std::string spot_sym_10len = helper->make10lenString(spot_sym);
    //     ticker10len_list += spot_sym_10len + ",";
    //     ticker_vec.push_back(spot_sym_10len);
    // }
    int i = 0;
    for(auto x = mySyms.begin(); x != mySyms.end(); x++){
        for (auto y = x->second.begin(); y != x->second.end(); y++){
            ticker_list += std::string((*y)->symbol_name) + std::string(",");
            ticker_vec.push_back(std::string((*y)->symbol_name));
            std::cout << "traded_list:" << (*y)->symbol_name << std::endl;
            traded_ticker_list.push_back((*y)->symbol_name);
            // std::cout << traded_ticker_list[i];
            i++;
        }
        std::cout << "Next x: \n";
        
    }
    // std::cout << symLoad.get_ticker_list.size() << std::endl;
    // traded_ticker_list.push_back("ALL");
    std::cout <<ticker_list<< std::endl;
    unHand = new Tim::UniverseHandler(Tim::ExchangeName::NSE, traded_ticker_list, configReader.date);
    unHand->setGetInfoValues(mySyms);
    unHand->loadUniverse();
    
    MyLogger::log(TRADEFILE, "main::init |", "Trade Universe initialized\n");
    
    
    std::string unl("");
    for (auto itr = symLoad.instrument_info_vector.begin(); itr != symLoad.instrument_info_vector.end(); itr++){
        unl = (*itr).Underlier;
        Tim::SpotPricing* spot_obj = nullptr;
	if(unl == "BANKNIFTY"){
            std::string bnf_file_path = "";
            if(path_resolver.HasMember("spot_constituents_path")){
                const Baryons::rapidjson::Value& spot_constituents_path =
                    path_resolver["spot_constituents_path"];
                if(spot_constituents_path.HasMember("BNF")){
                    const Baryons::rapidjson::Value& bnf_path = spot_constituents_path["BNF"];
                    bnf_file_path += bnf_path.GetString();
                }
                spot_obj = new Tim::NSESpotPricing(bnf_file_path, configReader.date);
               //spot_obj = new Tim::CMESpotPricing(Tim::Helpers::interest_rate, Tim::SpotType::PCP, unHand);
		spot_obj_map[unl] = spot_obj;
            }
        }
    else if(unl == "NIFTY"){
            std::string bnf_file_path = "";
            if(path_resolver.HasMember("spot_constituents_path")){
                const Baryons::rapidjson::Value& spot_constituents_path =
                    path_resolver["spot_constituents_path"];
                if(spot_constituents_path.HasMember("NF")){
                    const Baryons::rapidjson::Value& bnf_path = spot_constituents_path["NF"];
                    bnf_file_path += bnf_path.GetString();
                }
                spot_obj = new Tim::NSESpotPricing(bnf_file_path, configReader.date);
               //spot_obj = new Tim::CMESpotPricing(Tim::Helpers::interest_rate, Tim::SpotType::PCP, unHand);
		spot_obj_map[unl] = spot_obj;
            }
        }
	else{
        spot_obj = new Tim::CMESpotPricing(Tim::Helpers::interest_rate, Tim::SpotType::PCP, unHand, Tim::ExchangeName::NSE);
        spot_obj->getSpotStruct()->symbol = unl;
        if (spot_obj_map.find(unl) == spot_obj_map.end()) {
            spot_obj_map[unl] = spot_obj;
        }
        else {
            MyLogger::log(TRADEFILE, "Error |", "Adding the same underler repeatedly. Underlyer:", unl);
            std::cout << "Error | Adding the same underler repeatedly. Underlyer:" << unl << "\n";
        }
	}
        for(auto it=unHand->TradeUniverse.begin(); it!=unHand->TradeUniverse.end(); ++it) {
            if (it->second->underlyer == spot_obj->getSpotStruct()->symbol) {
                it->second->spot_obj = spot_obj;
            }
        }
    }



    MyLogger::log(TRADEFILE, "main::init |", "Trade Universe initialized\n");

    for (auto itr = spot_obj_map.begin(); itr != spot_obj_map.end(); itr++){
        if(itr->first.find("NIFTY") == std::string::npos){
            if(std::find(traded_ticker_list.begin(), traded_ticker_list.end(), itr->first) == traded_ticker_list.end()){
                std::string spot_sym = std::string(itr->first);
                std::string spot_sym_10len = Tim::Helpers::make10lenString(spot_sym);
                ticker10len_list += spot_sym_10len + ",";
                ticker_vec.push_back(spot_sym_10len);
            }
        }
        else{
            Tim::NSESpotPricing* this_spot = static_cast<Tim::NSESpotPricing*>(itr->second);
            for(auto x = this_spot->spot_constituents.begin(); x != this_spot->spot_constituents.end(); x++){
                if(std::find(traded_ticker_list.begin(), traded_ticker_list.end(), x->first) == traded_ticker_list.end()){
                    std::string spot_sym = std::string(x->first);
                    std::string spot_sym_10len = Tim::Helpers::make10lenString(spot_sym);
                    ticker10len_list += spot_sym_10len + ",";
                    ticker_vec.push_back(spot_sym_10len);
                }
            }
        }
    }

    Tim::Helpers::underlying = unl;
    //std::string* su = &Tim::Helpers::underlying;
    // std::transform(su->begaddDirtyin(), su->end(), su->begin(), std::tolower);
    for (auto it = Tim::Helpers::underlying.begin(); it!=Tim::Helpers::underlying.end(); ++it) {
        *it = std::tolower(*it);
    }
    // ticker_list += "ALL,";
    std::cout << "ticker10len_list:"  << ticker10len_list << std::endl;
    MyLogger::log(TRADEFILE, "main |", "ticker10len_list:", ticker10len_list);
    Tim::ConnectorSetup myCon(connector_file_path, ticker_list, ticker10len_list,
        traded_ticker_list, configReader.date);
    

    std::vector<std::string> v;
    bool isRaw = false;
    if(connector_file_path.find("raw") != std::string::npos){
        isRaw = true;
    }
    std::cout << "Symbol list\n";
    std::cout<<" Ticker vector size: "<<ticker_vec.size()<<"\n";
    std::cout<<"Traded Ticker list size "<<traded_ticker_list.size()<<"\n";
    for (auto it=ticker_vec.begin(); it!=ticker_vec.end(); it++) {
        std::cout << *it << std::endl;
    }
    // ticker_list=",NIFTY50";
    std::cout << "\n";
    con = new ISHM::NSEConnectorWrapper(&md_cb, &resp_cb, myCon.connectorCfg.INTERACTION_MODE,
        &(myCon.connectorCfg), isRaw? ticker_vec :v);
    std::cout << "SimConf: " << myCon.connectorCfg.XSIM_CONFIG_PATH << std::endl;
    if(myCon.connectorCfg.INTERACTION_MODE == SIMULATION ||
        myCon.connectorCfg.INTERACTION_MODE == PARALLELSIM){

        illuminati::Configfile& cfg_file_ = illuminati::Configfile::GetInstance();
        cfg_file_.LoadCfg(myCon.connectorCfg.XSIM_CONFIG_PATH);
        //cfg_file_.setProperty("SETTIMEASGLOBAL", "true");
        cfg_file_.setProperty("ORDER_TICKERS", ticker_list);
        //cfg_file_.setProperty("ORDER_TICKERS_10LEN", symbol_list);
        cfg_file_.setProperty("DATES", std::to_string(configReader.date));
        cfg_file_.setProperty("TICKERS", ticker_list);

    }

    con->initSims();
    auto symbol_id_map = con->getBookIdMap();
    std:: cout << "Sym ID map: \n";
    for (auto itr = symbol_id_map.begin(); itr!= symbol_id_map.end(); itr++){
        std::cout << itr->first << " << " << itr->second << std::endl;
    }
    unHand->addSymbolIDs(symbol_id_map);

    unHand->createCustomUniverse();
    unHand->getInfoFieldsForSymbolIDs();//lot_size_map);
    unHand->makeSymbolOptionChain();
    unHand->makeOptionPairMap();

    margin_calc = new Tim::MarginStats(unHand, strat_params->margin, configReader.date, configReader.mode, Tim::ExchangeName::NSE);

    MyLogger::log(TRADEFILE, "Setting up Margin percent and lot size...");

    risk_checker = new Tim::RiskChecker(global_params->max_pos, global_params->global_max_pos, 600000000, 6000000000, unHand);
    for (auto itr = unHand->TradeUniverseIDs.begin(); itr != unHand->TradeUniverseIDs.end(); itr++){
        if(itr->second->isOption){
            // MyLogger::log(TEMPFILE, itr->first);
            int symbol_id = itr->second->symbol_id;
            int lot_size = lot_size_map[symbol_id];
            itr->second->opt->updateLotSize(lot_size_map[symbol_id]);
        }
        else if(itr->second->isFut){
            // MyLogger::log(TEMPFILE, itr->first);
            int symbol_id = itr->second->symbol_id;
            int lot_size = lot_size_map[symbol_id];
            itr->second->fut->updateLotSize(lot_size_map[symbol_id]);
        }
    }

    for(auto it=unHand->TradeUniverseIDs.begin(); it!=unHand->TradeUniverseIDs.end(); ++it) {
        if(it->second->isOption) {
            option_data[it->second->opt->symbol_id] = it->second->opt;
            Tim::TradeVals tradeVals;
            tradeBook[it->second->opt->symbol_id] = tradeVals;
            Tim::execution_data executionData;
            executed_position[it->second->opt->symbol_id] = executionData;
        }
    }
    
    exec_logic = new Tim::ExecutionManager(unHand, params);
    OOM = new Tim::OverallOrderManager();

    respHand = new Tim::RespHandler(unHand, &resp_vector, &tradeBook, &executed_position, &order_map, OOM, exec_logic);
    createGraph(configReader.date);
    std::cout <<"Created Graph \n";
    std::string shock_file_path = "";
    std::string trade_info_path = "";
    // exitHand = new Tim::ExitHandler(mdHand->md_check->endtime, &tradeBook, unHand, pnl_calc,
    //    spot_obj, pos_file_path, pnl_file_path, shock_file_path, trade_info_path);


   
    con->RegisterTimerCallback(&timer_cb);
    con->addTimer(mdHand->md_check->starttime, 0);
    OOM->MakeMap(unHand, traded_ticker_list, con, Tim::ExchangeName::NSE);
    std::cout << "MadeMap. Gonna start sync now";
    con->StartSync();
    return 0;
}




/*
#include <bits/stdc++.h>

#include "s_rapidjson/document.h"
#include "connectorcrtp.h"
#include "ConnectorConfig/ConnectorInit.hpp"
#include "ConnectorConfig/nseconnectorwrapper.h"
#include "Helpers/helper_structs.hpp"
#include "Helpers/config_reader.hpp"
#include "Helpers/UserEvent.h"
#include "ParamsStrat.hpp"
#include "Helpers/SignalHandler.hpp"
#include "SymbolLoader/NSESymbolLoader.hpp"
#include "UniverseMaker/UniverseHandler.hpp"
#include "Execution/ExecutionManager.h"
// #include "Spot/cme_spot_pricing.hpp"
#include "Spot/nse_spot_pricing.hpp"
#include "CallBackManager/GreekHandler.hpp"
#include "CallBackManager/SpotPriceHandler.hpp"
#include "CallBackManager/OptPriceHandler.hpp"
#include "CallBackManager/MDHandler.hpp"
#include "CallBackManager/MDFeedHandler.hpp"
#include "CallBackManager/TimerHandler.hpp"
#include "CallBackManager/GreekTimerHandler.hpp"
#include "CallBackManager/ModelTimerHandler.hpp"
// #include "CallBackManager/DataDumper.hpp"
#include "ValidityChecks/MDValid.hpp"
#include "RiskChecker/RiskChecker.hpp"
#include "CallBackManager/StatsPrinter.hpp"
#include "CallBackManager/RiskCheckTimer.hpp"
#include "CallBackManager/RiskListener.hpp"
#include "Responses/RespHandler.hpp"
#include "Stats/StatsGen.hpp"
#include "PnLCalculator/PnlCalculator.hpp"
#include "CallBackManager/PriceMapListener.hpp"
#include "Instruments/ComplexInstruments.hpp"
//#include "RandIntTrader.hpp"
// #include "StraddleTrader.cpp"
// #include "IVCalendarTrader.cpp"
// #include "CallBackManager/TradeHandler.hpp"
// #include "CallBackManager/IVCalendarHandler.hpp"
#include "ExitHandler/ExitHandler.hpp"
#include "Execution/ExecutionManager.h"
#include "MarginLotSetter/MarginLotReader.hpp"
#include "IVCalendarModel.cpp"
#include <algorithm>    // std::transform
#include "TradeInfoDumper/trade_info_dumper.hpp"
// #include "InfoDumper.hpp"
// #include "ParamsDump.hpp"

using namespace illuminati::md;


uint64_t timestamp_g = 0;
uint64_t *Tim::Helpers::timestamp_ptr = &timestamp_g;
int *Tim::Helpers::date_ptr = NULL;
int *Tim::Helpers::pid_ptr = NULL;
std::string *Tim::Helpers::folder_ptr = NULL;
std::string Tim::Helpers::underlying = "";
int Tim::Helpers::sample_period = 1;
Tim::ExchangeName Tim::Helpers::exchange = ExchangeName::NSE;
std::chrono::high_resolution_clock::time_point t_hle_s;
double Tim::Helpers::interest_rate = 0.06;

//remove later
double Tim::Helpers::feed_multiplier = 0.5;
double Tim::Helpers::tick_size = 0.01;
double MIN_TICK_SIZE = 0.01;
double Tim::Helpers::commission = 0;
bool Tim::CombinedOrderManager::freezed_on_reject = false;
int64_t Tim::CombinedOrderManager::freeze_end_time = E9*E9;
int64_t Tim::CombinedOrderManager::error_code = -1;

//ConnectorType *con;
ISHM::NSEConnectorWrapper *con;
Tim::ConfigReader *configReader_ptr;
Tim::ExecutionManager* exec_logic;
Tim::ModelHandler *modelHandler;
// Tim::StraddleTrader* trader;
// Tim::IVCalendarTrader* ivCalendarTrader;
//Tim::RandIntTrader* trader;
std::map<std::string, Tim::SpotPricing*> spot_obj_map;
Tim::UniverseHandler* unHand;
Tim::StatsGen* stats;
Tim::PNL* pnl_calc;
Tim::MarginStats* margin_calc;
Tim::OverallOrderManager* OOM;
Tim::RespHandler* respHand;
Tim::RiskChecker* risk_checker;
// Tim::ExitHandler* exitHand;

Baryons::CallBackManager* CBM;
Baryons::MDHandler* mdHand;
Baryons::MDFeedHandler* mdFeedHand;
Baryons::OptPriceHandler* optHand;
Baryons::SpotPriceHandler* spotHand;
Baryons::PriceMap* priceHand;
Baryons::TimerHandler* timerHand;
Baryons::GreekTimerHandler* greekTimerHand;
Baryons::RiskCheckTimer* riskHand;
// Baryons::StrategyTimerHandler* strategyTimerHand;
// Baryons::IVCalendarHandler* ivCalendarHandler;
// Baryons::DataDumper* dataDumper;
Baryons::StatsTimeHandler* statsHand;
Baryons::ModelTimerHandler* modelTimerHand;
std::vector<double> model;

// Tim::ExpectedShock* shock_ind_;
// Baryons::IndicatorHandler* indHand;

std::vector<Tim::RespData> resp_vector;
std::map<int, Tim::execution_data> executed_position;
std::map<int, Tim::TradeVals> tradeBook;
std::map<uint32_t, Tim::order_data> order_map;
std::map<int, int> lot_size_map;
std::map<int, Tim::Options*> option_data;

Tim::Paramloader* params;
Tim::StratParams* strat_params;
Tim::GlobalParams* global_params;
// Tim::DumpParams* dump_params;
std::string prev_day_pos_file_path="";
std::string prev_day_pnl_file_path="";
double multiplier = 0.01;
// Tim::TradeInfoDumper* trade_dumper;
// Baryons::InfoDumper* infoDumpHand;


void createGraph(int date){
    uint64_t sample_starttime = Tim::Helpers::epochTimeInNanos(*Tim::Helpers::date_ptr, 
        global_params->start_hour, global_params->start_min, 0, 0);
    multiplier = Tim::Helpers::feed_multiplier;
    CBM = new Baryons::CallBackManager();
    CBM->setInstance(CBM);
        
    mdHand = new Baryons::MDHandler(unHand, global_params->start_hour, global_params->start_min, 
        global_params->end_hour, global_params->end_min, date);  
    mdFeedHand = new Baryons::MDFeedHandler(unHand, global_params->start_hour, global_params->start_min, 
        global_params->end_hour, global_params->end_min, date);  
    optHand = new Baryons::OptPriceHandler(unHand);
    spotHand = new Baryons::SpotPriceHandler(unHand, &spot_obj_map);
	//unHand->PrintCustomUniverse();
    priceHand = new Baryons::PriceMap();
    pnl_calc = new Tim::PNL(unHand, &resp_vector, &priceHand->price_map->market_data_map, 
        prev_day_pnl_file_path);
    auto it = lot_size_map.begin();
    if (it == lot_size_map.end()) {
        std::cerr << "HERE " << std::endl;
        exit(-1);
    }
    // shock_ind_ = new Tim::ExpectedShock(unHand, spot_obj);
    // indHand = new Baryons::IndicatorHandler(0, 1*1e9, rand_ind_);

    //infoDumpHand = new Baryons::InfoDumper(date, 0, SECOND, unHand, stats, spot_obj_map,
    //    trade_dumper, pnl_calc, &tradeBook, margin_calc, "CMEF_ES");
    modelHandler = new Tim::IVCalendarModel(sample_starttime, unHand, params, &tradeBook, stats);
    modelTimerHand = new Baryons::ModelTimerHandler(sample_starttime, 30*SECOND, unHand, params, modelHandler);
    stats = new Tim::StatsGen(unHand, pnl_calc, margin_calc, &tradeBook, &spot_obj_map); 
    // infoDumpHand = new Baryons::InfoDumper(date, 0, SECOND, unHand, stats, spot_obj_map,
        //  trade_dumper, pnl_calc, &tradeBook, margin_calc, modelHandler, "CMEF_ES");
    //infoDumpHand = new Baryons::InfoDumper(date, 0, SECOND, unHand, stats, spot_obj_map,
     //    trade_dumper, pnl_calc, &tradeBook, margin_calc, "CMEF_ES");
    // trader = new Tim::StraddleTrader(unHand, params, stats, margin_calc,
        // &tradeBook, date, 1, spot_obj_map.begin()->second);
    // ivCalendarTrader = new Tim::IVCalendarTrader(unHand, params, stats, margin_calc,
    //    &tradeBook, date, 1, spot_obj_map.begin()->second, modelHandler, pnl_calc);
    // ivCalendarHandler = new Baryons::IVCalendarHandler(date, sample_starttime, 30*SECOND, unHand,
        //  params, &tradeBook, &executed_position);
//    ivCalendarHandler->timer_trader = ivCalendarTrader;
    
    timerHand = new Baryons::TimerHandler(unHand, E9, sample_starttime, 
        global_params->start_hour, 
        global_params->start_min, 
        global_params->end_hour, 
        global_params->end_min, 
        date);
    //  strategyTimerHand = new Baryons::StrategyTimerHandler(date, sample_starttime, 10*SECOND, unHand, 
        // params, &tradeBook, &executed_position);
	// dataDumper = new Baryons::DataDumper(date, sample_starttime, 5*SECOND,unHand,
		// params,&spot_obj_map, strat_params->delta_lower_cap, strat_params->delta_upper_cap);
    //  strategyTimerHand->timer_trader = trader;
    greekTimerHand = new Baryons::GreekTimerHandler(sample_starttime, SECOND, 
        unHand, 0.01, &spot_obj_map, params);
    // riskHand = new Baryons::RiskCheckTimer(sample_starttime, strat_params->trade_timer, 
        // risk_checker, pnl_calc, unHand, &tradeBook);
    // statsHand = new Baryons::StatsTimeHandler(stats, sample_starttime, 30*SECOND, pnl_calc);
    CBM->addEdge(mdHand, optHand);
    CBM->addEdge(mdHand, spotHand);
    CBM->addEdge(mdHand, priceHand);
    CBM->addEdge(mdFeedHand, priceHand);
    //CBM->addEdge(optHand, greekHand);
    //CBM->addEdge(spotHand, greekHand);

    CBM->addEdge(timerHand, greekTimerHand);

	//Adding strategyTimeHandler as a listener to GreekTimeHandler
    // CBM->addEdge(greekTimerHand, infoDumpHand);
    // CBM->addEdge(greekTimerHand, ivCalendarHandler);
    CBM->addEdge(greekTimerHand, modelTimerHand);
    // CBM->addEdge(greekTimerHand,strategyTimerHand);
    // CBM->addEdge(greekTimerHand, dataDumper);
    //CBM->addEdge(greekTimerHand, infoDumpHand);

    // CBM->addEdge(timerHand, riskHand);
    // CBM->addEdge(timerHand, strategyTimerHand);
    // CBM->addEdge(timerHand, dataDumper);
    // CBM->addEdge(timerHand, statsHand);
    // CBM->addEdge(riskHand, strategyTimerHand);
    mdHand->addMDListener(optHand);
    mdHand->addMDListener(spotHand);
    mdHand->addMDListener(priceHand);
    mdFeedHand->addMDFeedListener(priceHand);
    timerHand->addTimerListener(greekTimerHand);
    // timerHand->addTimerListener(riskHand);
    // timerHand->addTimerListener(statsHand);
    // timerHand->addTimerListener(strategyTimerHand);
    // timerHand->addTimerListener(dataDumper);

// adding strategyTimeHandler to GreekTimeHandler subscriber list
    // greekTimerHand->addGreekTimerListener(ivCalendarHandler);
    greekTimerHand->addGreekTimerListener(modelTimerHand);
    // greekTimerHand->addGreekTimerListener(infoDumpHand);
    // greekTimerHand->addGreekTimerListener(strategyTimerHand);
    // greekTimerHand->addGreekTimerListener(dataDumper);
    // riskHand->addRiskListener(strategyTimerHand);
    //spotHand->addSpotPx_listener(greekHand);
    //optHand->addOptPx_listener(greekHand);

    timerHand->ResolveMinTime();
}

void callUmsgListeners(const illuminati::md::MarketUpdateNew* md) {
    int s = md->m_lastTradedTime;
    int c = md->m_seqnum;
    const char* loc1 = reinterpret_cast<const char*>(md->m_bidUpdates);
    const char* loc2 = reinterpret_cast<const char*>(md->m_askUpdates);
    Baryons::UserEvent user_event_;
    user_event_.code = c;
    user_event_.pid = s;
    strncpy(user_event_.msg1, loc1, 47);
    strncpy(user_event_.msg2, loc2, 47);
    user_event_.msg1[47] = user_event_.msg2[47] = '\0';
    // for (size_t i = 0, sz = umsg_listeners_.size(); i < sz; i++) {
    //     umsg_listeners_[i]->onUmsg(&user_event_);
    // }
    
    if(user_event_.pid != *Tim::Helpers::pid_ptr) {
        return;
    }
    if (user_event_.code == 0) {
        params->resetParam(user_event_.msg1, user_event_.msg2);
        if(std::strcmp(user_event_.msg1, "margin")==0) {
            double margin = std::stod(user_event_.msg2);
            margin_calc->reInitialize(margin);
        }
        // trader->reInitialize();
    }
}


void OI_map(std::map<std::string, uint16_t>& symbol_id_map) {
    // for (auto it=symbol_id_map.begin(); it!=symbol_id_map.end(); ++it){
    //     if(std::strncmp(it->first, "OI_", 3) == 0) {
    //         std::string symbol = std::string(it->first).substr(3);
    //         if(unHand->TradeUniverseIDs.find(symbol) != unHand->TradeUniverseIDs.end()){
    //             auto &ts = unHand->TradeUniverseIDs[symbol];
    //             if(ts->isOption) {
    //                ts->opt->updateOI(md->m_yield);
    //                //std::cout << "Opt: " << md->m_symbol << " " << md->m_timestamp << " " << md->m_yield << std::endl;
    //             }
    //             if(ts->isFut) {
    //                ts->fut->updateOI(md->m_yield);
    //                //std::cout << "Fut: " << md->m_symbol << " " << md->m_timestamp << " " << md->m_yield << std::endl;
    //             }
    //         }
    //     } 
    // }
}

void PrintBook(){

    for(auto itr = unHand->TradeUniverse.begin(); itr != unHand->TradeUniverse.end(); itr++){
        if(itr->second->isOption){
            itr->second->opt->printOption();
        }
    }
}


static void md_cb(uint16_t symbolID, const illuminati::md::MarketUpdateNew* md){
   // uint64_t timestamp_latest = md->m_timestamp; 
   // `if(std::strncmp(md->m_symbol, "OI_", 3) == 0) {
   // `    std::string symbol = std::string(md->m_symbol).substr(3);
   // `    if(unHand->TradeUniverseIDs.find(symbol) != unHand->TradeUniverseIDs.end()){
   // `        auto &ts = unHand->TradeUniverseIDs[symbol];     
   // `        if(ts->isOption) {
   // `            ts->opt->updateOI(md->m_yield);
   // `            //std::cout << "Opt: " << md->m_symbol << " " << md->m_timestamp << " " << md->m_yield << std::endl;
   // `        }
   // `        if(ts->isFut) {
   // `            ts->fut->updateOI(md->m_yield);
   // `            //std::cout << "Fut: " << md->m_symbol << " " << md->m_timestamp << " " << md->m_yield << std::endl;
   // `        }
   // `    }
   // `}

	//std::string sym = md->m_symbol ;
       // if(sym.size() == 10){
       //         std::cout<<"Symbol length "<<sym<<"\n";
       //         exit(0);
       // }

    if ((md->m_timestamp == 0)) {
        if (md->m_exchangeName == 0 && md->m_exchTS == 0) {
            // usermsg
            callUmsgListeners(md);
        }
        else {
            // std::cerr << ":: ignoring MD >>> " <<  md->toString() << std::endl;
        }
	std::cout<<"Returning 1 \n";
        return;
    }
    // static int i = 0;
    // std::cout << "bid:" << md->m_symbol << " " << md->m_bidUpdates[0].price << " " << md->m_askUpdates[0].price << std::endl;
    // if (std::strcmp(md->m_symbol,"KR4105PA000")==0 || std::strcmp(md->m_symbol,"KR4105QA000")==0) {
    //     i++;
    //     std::cout << "bid:" << md->m_bidUpdates[0].price << " " << md->m_askUpdates[0].price << std::endl;
    // }
    // if (i==2000) {
    //     std::exit(0);
    // }

    bool isOkay = false;
    if (md->m_updateType=='N' && md->m_feedType=='W'){
        // std::cout << "Update:" << md->m_symbol << "," << md->m_timestamp << "," << md->m_newPrice << std::endl;
        mdFeedHand->setMDFeed_(md, symbolID, &isOkay);
        CBM->notifyAll();
	std::cout<<"Return 2 \n";
        return;
    }
    if (Tim::Helpers::exchange == Tim::ExchangeName::US) {
        auto spot_obj = unHand->TradeUniverseIDs[symbolID]->spot_obj;
        double underlier_bid = md->getUnderlierBid();
        double underlier_ask = md->getUnderlierAsk();
        double underlier_trade_px = (underlier_bid + underlier_ask)/2.0;
        spot_obj->updateSpotPrice("", underlier_bid, underlier_ask, underlier_trade_px);
    } 
    mdHand->setMD_(md, symbolID, &isOkay);
    CBM->notifyAll();
    // trader->timer_man->updateTimers(md->m_timestamp);
    bool append_flag = false;
    if(isOkay && append_flag){
        //params.curr_port_delta = stats->
        //CIMaker = new Tim::ComplexInstruments(Tim::ComplexInstrumentType::CISurface, unHand->TradeUniverseIDs, params);
        //CIMaker->printComplexInstrument();
    }


}


static void timer_cb(uint64_t timestamp){

    //auto freq = std::min(uint64_t(timerHand->unit_time_this*SECOND), strat_params->improved_order_level_timeout);
    uint64_t newtime = ((timestamp/timerHand->unit_time_this) + 1) * timerHand->unit_time_this;
    timestamp_g = newtime; 
    static uint64_t params_timer = 0;
    if (newtime >= params_timer + strat_params->timer_unit){
        params_timer += strat_params->timer_unit;
        // params->reInitializeParams();
        // margin_calc->reInitialize(strat_params->margin);
        // trader->reInitialize();
        // std::cout << "margin_calc->init_margin:" << margin_calc->init_margin <<  " " << margin_calc->actual_margin << " " << params_timer << std::endl; 
    }
    timerHand->setTimer_(timestamp);
 
    //Update strat_params
    MyLogger::log(TEMPFILE, "main::timer_cb |", "Time: ", timestamp);
    static uint64_t timestamp_CBM = 0;
    uint64_t sample_freq = (timerHand->unit_time_this)*SECOND;
    if (timestamp >= timestamp_CBM + sample_freq){
        timestamp_CBM = ((timestamp)/sample_freq)*sample_freq;
        CBM->notifyAll();
    }
     
    static uint64_t timestamp_EM = 0;
    static bool send_alerts = true;
    if (strat_params->freeze_on_reject==true && send_alerts) {
        if (Tim::CombinedOrderManager::freezed_on_reject) {
            if (Tim::CombinedOrderManager::error_code == 100107) {
                Tim::CombinedOrderManager::freeze_end_time = *Tim::Helpers::timestamp_ptr + 2*SECOND;
            }
            else {
                Tim::CombinedOrderManager::freeze_end_time = *Tim::Helpers::timestamp_ptr - 600*SECOND;
                Tim::CombinedOrderManager::freeze_end_time = *Tim::Helpers::timestamp_ptr + 600*SECOND;
            }
            Aart::sendAlert("Got a reject in Strategy pid:"          + 
            std::to_string(*Tim::Helpers::pid_ptr) + ". Error_code:" + 
            std::to_string(Tim::CombinedOrderManager::error_code));
            send_alerts = false;
        }
    }
    else if (strat_params->freeze_on_reject==false || 
        Tim::CombinedOrderManager::freeze_end_time <=  *Tim::Helpers::timestamp_ptr){
        send_alerts = true;
        Tim::CombinedOrderManager::error_code = -1;
        Tim::CombinedOrderManager::freezed_on_reject = false;
    }


	//   if(timestamp >= trader->trade_starttime && timestamp <
    //     trader->trade_endtime){
    //     std::cout << "main::timer_cb |" << "Exec Manager invoked:" << timestamp <<  ":" << trader->trade_starttime << ":" << trader->trade_endtime << "\n"; 
        
    //     MyLogger::log(TRADEFILE, "Executing orders...");
    //     int percent_tol = 0;
    //     Tim::ModeExecution mode;
    //     mode = Tim::ModeExecution::PASSIVE;
	// //mode = Tim::ModeExecution::PASSIVE;
    //      struct Tim::ExecParams exec_params;
    //      exec_params.lot_size_map = &lot_size_map;
    //      exec_params.market_data_map = &priceHand->price_map->market_data_map;
	// exec_params.mode = mode;
	// //dummy data
	// exec_params.no_of_improvements = 4;
	// exec_params.percent_spread_to_improve = 25;
	// exec_params.percent_tol = 0;
	// exec_params.orderTimeoutNanos = 0.75*1e9;
	
    //     int no_of_lots = 1;
	// exec_params.no_of_lots = no_of_lots;
	// exec_logic->initManager(exec_params);
	// 	t_hle_s = std::chrono::high_resolution_clock::now();
	// //std::cout<<"Entering manage Execution\n";
    //     exec_logic->manageExecution(tradeBook, order_map,
    //         priceHand->price_map->market_data_map, lot_size_map, OOM,
    //         0.75*1e9, timestamp, true, mode, 0,
    //         no_of_lots, 4, 25);
    // }


    // else if (timestamp > strategyTimerHand->trade_endtime) {
    //     statsHand->onNotify();
    //     pnl_calc->DisplayPnl();
    //     stats->DisplayGreeks(true, true, true, true, true);
    //     std::vector<double> addToDumpLine;
        //?
        // exitHand->dumpPositions();
        // exitHand->dumpPnl(addToDumpLine, "");                                                         
        // exitHand->dumpShockRiskNumbers();
        // exitHand->dumpTradeInfoNumbers();
        // MyLogger::flush();
        // MyLogger::Stop();
        // exit(0);
    // }

    con->addTimer(newtime, 0);
    return;
}

static void resp_cb(uint16_t symbolID, const illuminati::infra::ResponseMsg* resp){
    auto ins = unHand->TradeUniverseIDs[symbolID];
    respHand->processResponse(resp, ins);
}


static void nomarketdata_callback(){

}

std::vector<std::string> ticker_of_interest = {};

int main(int argc, char* argv[]){

    //std::cout<<"Starting Main";
    
    Tim::Options::initializeMemPool();
    Tim::SimpleInstruments::initializeMemPool();
    // Tim::CI_Constrained_Surface_4::initializeMemPool();

    Tim::ConfigReader configReader = Tim::ConfigReader::GetInstance(argc, argv);
    configReader_ptr = &configReader;
    Tim::Helpers::folder_ptr = &configReader.output_folder_path; // /spare/local/pranka
    Aart::SignalHandler(configReader.mode); // mode 1
	std::cout<<"Starting Main";
    std::string pos_file_path = "";
    std::string pnl_file_path = "";
    std::string trade_info_file_path = "";
    int core = -1;
    if (configReader_ptr->mode == 3) {
       core = 15;
   	}

    MyLogger::InitInstance(configReader.symbol_file_path, configReader.date, configReader.pid, core);
    //MyLogger::GetInstance(configReader.symbol_file_path, configReader.date, configReader.pid, 15);
    MyLogger::log(TRADEFILE, "STRATEGY: CME SHORT GAMMA for Date: ", configReader.date, "\n\n");
    MyLogger::log(TEMPFILE, "main::init |", configReader.to_string());

    Tim::Helpers::date_ptr = &configReader.date;
    Tim::Helpers::pid_ptr = &configReader.pid;
    FILE* fp = fopen(configReader.symbol_file_path.c_str(), "r");
    Baryons::rapidjson::FileStream is(fp);
    Baryons::rapidjson::Document path_resolver;
    path_resolver.ParseStream<0>(is);
    
    std::string connector_file_path("");
    if (path_resolver.HasMember("connector_file_path")){
        const Baryons::rapidjson::Value& connector_path = path_resolver["connector_file_path"];
        connector_file_path = connector_path.GetString(); //"%HOME%/data/configs/ConnectorNSE.cfg"
        Tim::Helpers::path_wrapper(connector_file_path);
    }
    else {
        std::cout << "Error: connector_file_path was not set" << std::endl;
        exit(0);
    }
    
    MyLogger::log(TRADEFILE, "Connector Path Used: ", connector_file_path, "\n\n");
    MyLogger::log(TRADEFILE, "Dump Paths Resolved from JSON file|");
    
    bool pos_test = false, pnl_test = false, trade_test = false;
    if(path_resolver.HasMember("pos_file_path")){
        const Baryons::rapidjson::Value& pos_path= path_resolver["pos_file_path"];
        std::string pos_path_suffix = pos_path.GetString();
        pos_file_path += pos_path_suffix + "ExSGposFile_" + std::to_string(configReader.pid) +
            "_" + std::to_string(configReader.date) + ".txt";
        pos_test = true;
    }
    else{
        MyLogger::log(TRADEFILE, "pos_file_path: UNDEFINED!");
    }

    if(path_resolver.HasMember("pnl_file_path")){
        const Baryons::rapidjson::Value& pnl_path= path_resolver["pnl_file_path"];
        std::string pnl_path_suffix = pnl_path.GetString();
        pnl_file_path += pnl_path_suffix + "ExSGpnlFile_" + std::to_string(configReader.pid) +
            "_" + std::to_string(configReader.date) + ".txt";
        pnl_test = true;
    }
    else{
        MyLogger::log(TRADEFILE, "pnl_file_path: UNDEFINED!");
    }
    
    if(path_resolver.HasMember("trade_info_path")){
        const Baryons::rapidjson::Value& trade_info_path = path_resolver["trade_info_path"];
        trade_info_file_path += trade_info_path.GetString();
        trade_test = true;
    }
    if(pos_test)
        MyLogger::log(TRADEFILE, "pos_file_path: ", pos_file_path);
    if(pnl_test)
        MyLogger::log(TRADEFILE, "pnl_file_path: ", pnl_file_path);
    if(trade_test)
        MyLogger::log(TRADEFILE, "trade_info_file_path", trade_info_file_path);

    MyLogger::log(TRADEFILE, "\n");
    try{
        params = new Tim::ParamloaderDerived(configReader.param_file_path);
    }
    catch(...){
        std::cout<<"Exception| params_pointer could not be initialized"<<std::endl;
    }
    strat_params = ((Tim::ParamloaderDerived*)params)->strat_params;
    global_params = params->global_params;
    //Tim::Helpers::exchange = global_params->exchange=="CME" ? Tim::ExchangeName::CME : Tim::ExchangeName::US;

    int MIN_SYM_SIZE_NSE = 4;
    std::map<std::string, int> lot_size_symbol_map;
    Tim::NSESymbolLoader symLoad(configReader.symbol_file_path, MIN_SYM_SIZE_NSE, 
    configReader.date, prev_day_pos_file_path, params);
    symLoad.displayStratSymbols();
    
   
//     trade_dumper = new Tim::TradeInfoDumper(trade_info_file_path, configReader.date, configReader.pid);
//    trade_dumper->addDumper(std::string("pnl_attr_series_pid11"), std::string(".csv"));
//    if(configReader.mode != 2){
//        trade_dumper->addDumper(std::string("trade_info_series_pid11"), std::string(".txt"));
//    }
//    std::stringstream pnl_stream;
//    pnl_stream<<"time,total_pnl,vega_pnl,delta_pnl,theta_pnl,net_slippage,vega,delta,theta,call_gamma,put_gamma,long_call_expos,long_put_expos,short_call_expos,short_put_expos,theo_short_call_expos,theo_short_put_expos,spot,theo_margin_util,actual_margin_util,near_term_tte,far_term_tte, weeks_to_expire_far_term,short_term_atm_iv,long_term_atm_iv,iv_ratio_rank,signal_pnl,short_portfolio_total_pnl, short_portfolio_vega_pnl, long_portfolio_total_pnl, long_portfolio_vega_pnl,near_term_call_volume, near_term_put_volume, far_term_call_volume, far_term_put_volume,week0_premium_call_per_tte,week0_premium_put_per_tte,week1_premium_call_per_tte,week1_premium_put_per_tte,week2_premium_call_per_tte,week2_premium_put_per_tte,monthly_permium_call_per_tte,monthly_premium_put_per_tte,week1_tte,week2_tte,near_c_iv_b1, near_c_iv_b2, near_c_iv_b3, near_c_iv_b4, near_c_iv_b5,near_c_iv_b6,near_c_iv_b7,near_c_iv_b8,near_c_iv_b9,near_c_iv_b10,near_p_iv_b1, near_p_iv_b2, near_p_iv_b3, near_p_iv_b4, near_p_iv_b5,near_p_iv_b6,near_p_iv_b7,near_p_iv_b8,near_p_iv_b9,near_p_iv_b10,far_c_iv_b1, far_c_iv_b2, far_c_iv_b3, far_c_iv_b4, far_c_iv_b5,far_c_iv_b6,far_c_iv_b7,far_c_iv_b8,far_c_iv_b9,far_c_iv_b10,far_p_iv_b1, far_p_iv_b2, far_p_iv_b3, far_p_iv_b4, far_p_iv_b5,far_p_iv_b6,far_p_iv_b7,far_p_iv_b8,far_p_iv_b9,far_p_iv_b10,week1_c_iv_b1, week1_c_iv_b2, week1_c_iv_b3, week1_c_iv_b4, week1_c_iv_b5,week1_c_iv_b6,week1_c_iv_b7,week1_c_iv_b8,week1_c_iv_b9,week1_c_iv_b10,week1_p_iv_b1,week1_p_iv_b2, week1_p_iv_b3, week1_p_iv_b4, week1_p_iv_b5,week1_p_iv_b6,week1_p_iv_b7,week1_p_iv_b8,week1_p_iv_b9,week1_p_iv_b10,near_c_iv_1above_st, near_c_iv_2above_st,near_c_iv_1below_st,near_c_iv_2below_st,near_p_iv_1above_st, near_p_iv_2above_st,near_p_iv_1below_st,near_p_iv_2below_st,far_c_iv_1above_st, far_c_iv_2above_st,far_c_iv_1below_st,far_c_iv_2below_st,far_p_iv_1above_st, far_p_iv_2above_st,far_p_iv_1below_st,far_p_iv_2below_st, week1_c_iv_1above_st, week1_c_iv_2above_st,week1_c_iv_1below_st,week1_c_iv_2below_st,week1_p_iv_1above_st, week1_p_iv_2above_st,week1_p_iv_1below_st,week1_p_iv_2below_st,   near_c_px_1above_st, near_c_px_2above_st,near_c_px_1below_st,near_c_px_2below_st,near_p_px_1above_st, near_p_px_2above_st,near_p_px_1below_st,near_p_px_2below_st,far_c_px_1above_st, far_c_px_2above_st,far_c_px_1below_st,far_c_px_2below_st,far_p_px_1above_st, far_p_px_2above_st,far_p_px_1below_st,far_p_px_2below_st, week1_c_px_1above_st, week1_c_px_2above_st,week1_c_px_1below_st,week1_c_px_2below_st,week1_p_px_1above_st, week1_p_px_2above_st,week1_p_px_1below_st,week1_p_px_2below_st,near_vega,far_vega,long_near_commission,short_near_commission,long_far_commission,short_far_commission";
//    trade_dumper->dumpLine("pnl_attr_series_pid11", pnl_stream);
 
    struct tm t = {0}; 
    t.tm_year = Tim::Helpers::getYear(configReader.date);
    t.tm_mon = Tim::Helpers::getMonth(configReader.date);
    t.tm_mday = Tim::Helpers::getDay(configReader.date);
    t.tm_hour = 9;
    t.tm_min = 30;
    t.tm_sec = 0;
    std::time_t timeSinceEpoch = mktime(&t);
    uint64_t starttime = long(timeSinceEpoch)*E9; 
    timestamp_g = starttime;  
   
    Tim::ExpiryTime expiry = Tim::ExpiryTime::EXPIRY_1;
    std::map<std::string, std::vector<Tim::GetInfoFields*>> mySyms;
    const Baryons::rapidjson::Value& underliers = path_resolver["Interested_Instruments"];
    assert(underliers.IsArray());
    for (auto itr = underliers.Begin(); itr != underliers.End(); ++itr){
    	const Baryons::rapidjson::Value& UNL_doc= *itr;
        assert(UNL_doc.IsObject()); // each attribute is an object
        if(UNL_doc.HasMember("Underlier") && UNL_doc.HasMember("Path")){
		const Baryons::rapidjson::Value& IsOption_name = UNL_doc["IsOption"];
                if(IsOption_name.GetBool()){
        		const Baryons::rapidjson::Value& expiry_ = UNL_doc["expiry"];
        		expiry  = (Tim::ExpiryTime) expiry_.GetInt();
			symLoad.getStratSymbols(mySyms, expiry);
			//TODO:: Remove this
			// const Baryons::rapidjson::Value& expiry1_ = UNL_doc["expiry1"];
			// expiry  = (Tim::ExpiryTime) expiry1_.GetInt();
			// symLoad.getStratSymbols(mySyms, expiry);
			// const Baryons::rapidjson::Value& expiry2_ = UNL_doc["expiry2"];
            //             expiry  = (Tim::ExpiryTime) expiry2_.GetInt();
            //             symLoad.getStratSymbols(mySyms, expiry);
			// const Baryons::rapidjson::Value& expiry3_ = UNL_doc["expiry3"];
            //             expiry  = (Tim::ExpiryTime) expiry3_.GetInt();
            //             symLoad.getStratSymbols(mySyms, expiry);
		}
	}
}
    //std::cout<<"******Testing Expiry******** "<<expiry<<"\n";
    //TODO:: Uncomment this later
    //std::map<std::string, std::vector<Tim::GetInfoFields*>> mySyms;
    //Tim::ExpiryTime expiry = Tim::ExpiryTime::EXPIRY_1;
    //symLoad.getStratSymbols(mySyms);
    //TODO: Uncomment this later
    //symLoad.getStratSymbols(mySyms, expiry);

    std::vector<std::string> traded_ticker_list;
    std::vector<std::string> ticker_vec;
    std::string ticker_list="";
    std::string underlier_for_this_strat = "";
    std::string ticker10len_list = "";


    // for (auto itr = spot_obj->spot_constituents.begin();
    //         itr != spot_obj->spot_constituents.end(); itr++){

    //     std::string spot_sym = std::string(itr->first);
    //     std::string spot_sym_10len = helper->make10lenString(spot_sym);
    //     ticker10len_list += spot_sym_10len + ",";
    //     ticker_vec.push_back(spot_sym_10len);
    // }

    for(auto x = mySyms.begin(); x != mySyms.end(); x++){
        for (auto y = x->second.begin(); y != x->second.end(); y++){
            ticker_list += std::string((*y)->symbol_name) + std::string(",");
            ticker_vec.push_back(std::string((*y)->symbol_name));
            std::cout << "traded_list:" << (*y)->symbol_name << std::endl;
            traded_ticker_list.push_back((*y)->symbol_name);
        }
    }
    //traded_ticker_list is a vector of symbol strings 

    unHand = new Tim::UniverseHandler(Tim::ExchangeName::NSE, traded_ticker_list, configReader.date);
    unHand->setGetInfoValues(mySyms);
    unHand->loadUniverse();
    MyLogger::log(TRADEFILE, "main::init |", "Trade Universe initialized\n");
    
    std::string unl("");
    for (auto itr = symLoad.instrument_info_vector.begin(); itr != symLoad.instrument_info_vector.end(); itr++){
        unl = (*itr).Underlier;
        Tim::SpotPricing* spot_obj = nullptr;
	if(unl == "BANKNIFTY"){
            std::string bnf_file_path = "";
            if(path_resolver.HasMember("spot_constituents_path")){
                const Baryons::rapidjson::Value& spot_constituents_path =
                    path_resolver["spot_constituents_path"];
                if(spot_constituents_path.HasMember("BNF")){
                    const Baryons::rapidjson::Value& bnf_path = spot_constituents_path["BNF"];
                    bnf_file_path += bnf_path.GetString();
                }
                spot_obj = new Tim::NSESpotPricing(bnf_file_path, configReader.date);
               //spot_obj = new Tim::CMESpotPricing(Tim::Helpers::interest_rate, Tim::SpotType::PCP, unHand);
		spot_obj_map[unl] = spot_obj;
            }
        }
	// else{
    //     spot_obj = new Tim::NSESpotPricing(bnf_file_path, configReader.date);
    //     spot_obj->getSpotStruct()->symbol = unl;
    //     if (spot_obj_map.find(unl) == spot_obj_map.end()) {
    //         spot_obj_map[unl] = spot_obj;
    //     }
    //     else {
    //         MyLogger::log(TRADEFILE, "Error |", "Adding the same underler repeatedly. Underlyer:", unl);
    //         std::cout << "Error | Adding the same underler repeatedly. Underlyer:" << unl << "\n";
    //     }
	// }
        for(auto it=unHand->TradeUniverse.begin(); it!=unHand->TradeUniverse.end(); ++it) {
            if (it->second->underlyer == spot_obj->getSpotStruct()->symbol) {
                it->second->spot_obj = spot_obj;
            }
        }
    }



    MyLogger::log(TRADEFILE, "main::init |", "Trade Universe initialized\n");

    for (auto itr = spot_obj_map.begin(); itr != spot_obj_map.end(); itr++){
        if(itr->first.find("NIFTY") == std::string::npos){
            if(std::find(traded_ticker_list.begin(), traded_ticker_list.end(), itr->first) == traded_ticker_list.end()){
                std::string spot_sym = std::string(itr->first);
                std::string spot_sym_10len = Tim::Helpers::make10lenString(spot_sym);
                ticker10len_list += spot_sym_10len + ",";
                ticker_vec.push_back(spot_sym_10len);
            }
        }
        else{
            Tim::NSESpotPricing* this_spot = static_cast<Tim::NSESpotPricing*>(itr->second);
            for(auto x = this_spot->spot_constituents.begin(); x != this_spot->spot_constituents.end(); x++){
                if(std::find(traded_ticker_list.begin(), traded_ticker_list.end(), x->first) == traded_ticker_list.end()){
                    std::string spot_sym = std::string(x->first);
                    std::string spot_sym_10len = Tim::Helpers::make10lenString(spot_sym);
                    ticker10len_list += spot_sym_10len + ",";
                    ticker_vec.push_back(spot_sym_10len);
                }
            }
        }
    }

    Tim::Helpers::underlying = unl;
    //std::string* su = &Tim::Helpers::underlying;
    // std::transform(su->begin(), su->end(), su->begin(), std::tolower);  
    for (auto it = Tim::Helpers::underlying.begin(); it!=Tim::Helpers::underlying.end(); ++it) {
        *it = std::tolower(*it);
    }

    std::cout << "ticker10len_list:"  << ticker10len_list << std::endl;
    MyLogger::log(TRADEFILE, "main |", "ticker10len_list:", ticker10len_list);
    Tim::ConnectorSetup myCon(connector_file_path, ticker_list, ticker10len_list,
        traded_ticker_list, configReader.date);
    

    std::vector<std::string> v;
    bool isRaw = false;
    if(connector_file_path.find("raw") != std::string::npos){
        isRaw = true;
    }
    std::cout << "Symbol list\n";
    std::cout<<" Ticker vector size: "<<ticker_vec.size()<<"\n";
    std::cout<<"Traded Ticker list size "<<traded_ticker_list.size()<<"\n";
    for (auto it=ticker_vec.begin(); it!=ticker_vec.end(); it++) {
        std::cout << *it << std::endl;
    }
    std::cout << "\n";
    con = new ISHM::NSEConnectorWrapper(&md_cb, &resp_cb, myCon.connectorCfg.INTERACTION_MODE,
        &(myCon.connectorCfg), isRaw? ticker_vec :v);

    if(myCon.connectorCfg.INTERACTION_MODE == SIMULATION ||
        myCon.connectorCfg.INTERACTION_MODE == PARALLELSIM){

        illuminati::Configfile& cfg_file_ = illuminati::Configfile::GetInstance();
        cfg_file_.LoadCfg(myCon.connectorCfg.XSIM_CONFIG_PATH);
        //cfg_file_.setProperty("SETTIMEASGLOBAL", "true");
        cfg_file_.setProperty("ORDER_TICKERS", ticker_list);
        //cfg_file_.setProperty("ORDER_TICKERS_10LEN", symbol_list);
        cfg_file_.setProperty("DATES", std::to_string(configReader.date));

    }

    con->initSims();
    auto symbol_id_map = con->getBookIdMap();
    unHand->addSymbolIDs(symbol_id_map);
   // unHand->createCustomUniverse();
   // Make this Enum
//    unHand->IVCalendarUniverse("Near", Tim::NearWeekly);
//    unHand->IVCalendarUniverse("Far", Tim::Monthly);
//   //TODO: Remove this later
//   unHand->IVCalendarUniverse("Week1",Tim::Week1);
//   unHand->IVCalendarUniverse("Week3",Tim::Week3); 

   //Testing custom universe function
   //for(auto i: unHand->IVUniverseMap){
   //	std::cout<<i.first<<"\n";
   //	for( auto j: i.second){
   //		std::cout<<"****"<<j.first<<"**** Expiry Date***"<<j.second->expiry_date;
  // 	}
  //  }
    unHand->getInfoFieldsForSymbolIDs(lot_size_map);
    unHand->makeSymbolOptionChain();
     unHand->makeOptionPairMap();
    std::cout<<"******************** Margin **************** "<<strat_params->margin<<"**********************\n";
    margin_calc = new Tim::MarginStats(unHand, strat_params->margin, configReader.date, configReader.mode, Tim::ExchangeName::NSE);

    MyLogger::log(TRADEFILE, "Setting up Margin percent and lot size...");

    risk_checker = new Tim::RiskChecker(global_params->max_pos, global_params->global_max_pos, 600000000, 6000000000, unHand);
    for (auto itr = unHand->TradeUniverseIDs.begin(); itr != unHand->TradeUniverseIDs.end(); itr++){
        if(itr->second->isOption){
   
            int symbol_id = itr->second->symbol_id;
            int lot_size = lot_size_map[symbol_id];
            std::cout << lot_size << "\n" << lot_size_map.size() << "\n";
            itr->second->opt->updateLotSize(lot_size_map[symbol_id]);
        }
        else if(itr->second->isFut){
       
            int symbol_id = itr->second->symbol_id;
            int lot_size = lot_size_map[symbol_id];
            itr->second->fut->updateLotSize(lot_size_map[symbol_id]);
        }
    }

    for(auto it=unHand->TradeUniverseIDs.begin(); it!=unHand->TradeUniverseIDs.end(); ++it) {
        if(it->second->isOption) {
            option_data[it->second->opt->symbol_id] = it->second->opt;
            Tim::TradeVals tradeVals;
            tradeBook[it->second->opt->symbol_id] = tradeVals;
            Tim::execution_data executionData;
            executed_position[it->second->opt->symbol_id] = executionData;
        }
    }
    
    exec_logic = new Tim::ExecutionManager(unHand, params);
    OOM = new Tim::OverallOrderManager();

    respHand = new Tim::RespHandler(unHand, &resp_vector, &tradeBook, &executed_position, &order_map, OOM, exec_logic);

	std::cout<<"Entering createGraphfunction"<<"\n";
    unHand->createCustomUniverse();
    createGraph(configReader.date);
    //unHand->createCustomUniverse();
    std::string shock_file_path = "";
    std::string trade_info_path = "";
    // exitHand = new Tim::ExitHandler(mdHand->md_check->endtime, &tradeBook, unHand, pnl_calc,
    //    spot_obj, pos_file_path, pnl_file_path, shock_file_path, trade_info_path);

    for(auto itr = unHand->TradeUniverse.begin(); itr != unHand->TradeUniverse.end(); itr++){
        if(itr->second->isOption){
            // itr->second->opt->printOption();
            std::cout << itr->second->opt->delta << "\n";
        }
    }
   
    con->RegisterTimerCallback(&timer_cb);
    con->addTimer(mdHand->md_check->starttime, 0);
    OOM->MakeMap(unHand, traded_ticker_list, con, Tim::ExchangeName::NSE);
    con->StartSync();
    return 0;
}
*/
