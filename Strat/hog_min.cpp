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
// #include "RandIntTrader.hpp"
// #include "CallBackManager/TradeHandler.hpp"
#include "ExitHandler/ExitHandler.hpp"
#include "Execution/ExecutionManager.h"
#include "MarginLotSetter/MarginLotReader.hpp"
#include <algorithm>    // std::transform
#include "DumpModel.hpp"
#include "ParamsStrat.hpp"
#include "Helpers/rolling_stats.hpp"
// #include "agshocklib.h"
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

bool Tim::CombinedOrderManager::freezed_on_reject = false;
int64_t Tim::CombinedOrderManager::freeze_end_time = E9*E9;
int64_t Tim::CombinedOrderManager::error_code = -1;

ConnectorType *con;
Tim::ConfigReader *configReader_ptr;
Tim::ExecutionManager* exec_logic;
Tim::ModelHandler *modelHandler;
// Tim::RandIntTrader* trader;

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
Baryons::TimerHandler* timerHand;
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
int counter = 0, counter1 = 1, counter2 = 1, counter3 = 1, counter4 = 1, counter5 = 1;
Tim::Paramloader* params;
Tim::StratParams* strat_params;
Tim::GlobalParams* global_params;
std::string symbol_name = "";
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
    modelHandler = new Tim::DumpModel(sample_starttime, unHand, params, &tradeBook, stats);
//    modelTimerHand = new Baryons::ModelTimerHandler(sample_starttime, 10*SECOND, unHand, params, modelHandler);
//    timerHand = new Baryons::TimerHandler(unHand, SECOND, sample_starttime,
//        global_params->start_hour,
//        global_params->start_min,
//        global_params->end_hour,
//        global_params->end_min,
//        date);
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

void PrintBook(){

    for(auto itr = unHand->TradeUniverse.begin(); itr != unHand->TradeUniverse.end(); itr++){
        if(itr->second->isOption){
            itr->second->opt->printOption();
        }
    }
}


static void md_cb(uint16_t symbolID, const illuminati::md::MarketUpdateNew* md){
    static int logger_i = 0;
    static std::ofstream output1, output2, output3, output4, output5;
    std::string fname1 = *Tim::Helpers::folder_ptr + "spot_niftyit/" + std::to_string(*Tim::Helpers::date_ptr)+  "_NIFTYIT" +".csv";
    std::string fname2 = *Tim::Helpers::folder_ptr + "spot_niftyfmcg/" +std::to_string(*Tim::Helpers::date_ptr)+ "_NIFTYFMCG" +".csv";
    std::string fname3 = *Tim::Helpers::folder_ptr + "spot_niftyenergy/" +std::to_string(*Tim::Helpers::date_ptr)+ "_NIFTYENERGY" +".csv";
    std::string fname4 = *Tim::Helpers::folder_ptr + "spot_nifty50/" +std::to_string(*Tim::Helpers::date_ptr)+ "_NIFTY50" +".csv";
    std::string fname5 = *Tim::Helpers::folder_ptr + "spot_niftybank/" +std::to_string(*Tim::Helpers::date_ptr)+ "_NIFTYBANK" +".csv";
    uint64_t sample_starttime = Tim::Helpers::epochTimeInNanos(*Tim::Helpers::date_ptr,
        global_params->start_hour, global_params->start_min, 0, 0);
    uint64_t sample_endtime = Tim::Helpers::epochTimeInNanos(*Tim::Helpers::date_ptr,
        global_params->end_hour, global_params->end_min, 0, 0);
    if (md->m_timestamp > sample_starttime && md->m_timestamp < sample_endtime){
		if ((md->m_timestamp == 0)) {
			if (md->m_exchangeName == 0 && md->m_exchTS == 0) {
				// usermsg
				callUmsgListeners(md);
			}
			return;
		}
        if (logger_i==0) {
                output1.open(fname1, std::ios_base::trunc); 
                output2.open(fname2, std::ios_base::trunc); 
                output3.open(fname3, std::ios_base::trunc); 
                output4.open(fname4, std::ios_base::trunc); 
                output5.open(fname5, std::ios_base::trunc);
                std::string s = "time,price\n";
                output1 << s;
                output1.close();
                output2 << s;
                output2.close();
                output3 << s;
                output3.close();
                output4 << s;
                output4.close();
                output5 << s;
                output5.close();
                ++logger_i;
        }
		bool isOkay = false;
        std::string sym(md->m_symbol);
        if (counter1 % 30 == 0 && sym == "NIFTYIT"){
            output1.open(fname1, std::ios_base::app); // append instead of overwrite
            output1 << Tim::Helpers::getHumanReadableTime(sample_starttime + long(E9)*counter1) << ","  << md->m_newPrice << std::endl;
            output1.close();
            std::cout << counter1 << " Printing stuff " << Tim::Helpers::getHumanReadableTime((md->m_timestamp/long(E9))*long(E9)) << ","  << Tim::Helpers::getHumanReadableTime(sample_starttime + long(E9)*counter1) << ", " << md->m_symbol << ",  "  << md->m_newPrice << std::endl;
            
        }
        if (counter2 % 30 == 0 && sym == "NIFTYFMCG"){
            output2.open(fname2, std::ios_base::app); // append instead of overwrite
            output2 << Tim::Helpers::getHumanReadableTime(sample_starttime + long(E9)*counter2) << ","  << md->m_newPrice << std::endl;
            output2.close();
            std::cout << counter2 << " Printing stuff " << Tim::Helpers::getHumanReadableTime((md->m_timestamp/long(E9))*long(E9)) << ","  << Tim::Helpers::getHumanReadableTime(sample_starttime + long(E9)*counter2) << ", " << md->m_symbol << ",  "  << md->m_newPrice << std::endl;

        }
        if (counter3 % 30 == 0 && sym == "NIFTYENERGY"){
            output3.open(fname3, std::ios_base::app); // append instead of overwrite
            output3 << Tim::Helpers::getHumanReadableTime(sample_starttime + long(E9)*counter3) << ","  << md->m_newPrice << std::endl;
            output3.close();
            std::cout << counter3 << " Printing stuff " << Tim::Helpers::getHumanReadableTime((md->m_timestamp/long(E9))*long(E9)) << ","  << Tim::Helpers::getHumanReadableTime(sample_starttime + long(E9)*counter3) << ", " << md->m_symbol << ",  "  << md->m_newPrice << std::endl;

        }
        if (counter4 % 30 == 0 && sym == "NIFTY50"){
            output4.open(fname4, std::ios_base::app); // append instead of overwrite
            output4 << Tim::Helpers::getHumanReadableTime(sample_starttime + long(E9)*counter4) << ","  << md->m_newPrice << std::endl;
            output4.close();
            std::cout << counter4 << " Printing stuff " << Tim::Helpers::getHumanReadableTime((md->m_timestamp/long(E9))*long(E9)) << ","  << Tim::Helpers::getHumanReadableTime(sample_starttime + long(E9)*counter4) << ", " << md->m_symbol << ",  "  << md->m_newPrice << std::endl;

        }
        if (counter5 % 30 == 0 && sym == "NIFTYBANK"){
            output5.open(fname5, std::ios_base::app); // append instead of overwrite
            output5 << Tim::Helpers::getHumanReadableTime(sample_starttime + long(E9)*counter5) << ","  << md->m_newPrice << std::endl;
            output5.close();
            std::cout << counter5 << " Printing stuff " << Tim::Helpers::getHumanReadableTime((md->m_timestamp/long(E9))*long(E9)) << ","  << Tim::Helpers::getHumanReadableTime(sample_starttime + long(E9)*counter5) << ", " << md->m_symbol << ",  "  << md->m_newPrice << std::endl;

        }

        if (sym == "NIFTY50"){
            counter1++;
        }
        else if (sym == "NIFTYIT"){
            counter2++;
        }
        else if (sym == "NIFTYFMCG"){
            counter3++;
        }
        else if (sym == "NIFTYENERGY"){
            counter4++;
        }
        else if (sym == "NIFTYBANK"){
            counter5++;
        }
		mdHand->setMD_(md, symbolID, &isOkay);
		CBM->notifyAll();
    }
}


static void timer_cb(uint64_t timestamp){

    //auto freq = std::min(uint64_t(timerHand->unit_time_this*SECOND), strat_params->improved_order_level_timeout);
    uint64_t newtime = ((timestamp/timerHand->unit_time_this) + 1) * timerHand->unit_time_this;
    timestamp_g = newtime;
    static uint64_t params_timer = 0;
    if (newtime >= params_timer + strat_params->timer_unit){
        params_timer += strat_params->timer_unit;
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

    if (path_resolver.HasMember("symbol_name")){
    	const Baryons::rapidjson::Value& sym_name = path_resolver["symbol_name"];
		symbol_name = sym_name.GetString();
    }
    else{
    	symbol_name = "NIFTYBANK";
    }
    MyLogger::log(TRADEFILE, "\n");
    try{
        params = new Tim::ParamloaderDerived(configReader.param_file_path);
    }
    catch(...){
        std::cout<<"Exception| params_pointer could not be initialized"<<std::endl;
    }
    global_params = params->global_params;
    Tim::Helpers::exchange = global_params->exchange=="CME" ? Tim::ExchangeName::CME : Tim::ExchangeName::NSE;

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


    std::map<std::string, std::vector<Tim::GetInfoFields*>> mySyms;
    Tim::ExpiryTime expiry = Tim::ExpiryTime::EXPIRY_1;

    std::vector<std::string> traded_ticker_list;
    std::vector<std::string> ticker_vec;
    std::string ticker_list= "NIFTY50,NIFTYIT,NIFTYFMCG,NIFTYENERGY,NIFTYBANK";
    std::string underlier_for_this_strat = "";
    std::string ticker10len_list = "";

    std::cout << "printing all data of tickers: \n";
    std::cout <<ticker_list<< std::endl;

    MyLogger::log(TRADEFILE, "main::init |", "Trade Universe initialized\n");
    std::string unl("");
    MyLogger::log(TRADEFILE, "main::init |", "Trade Universe initialized\n");
    ticker_list = "NIFTYBANK";

    Tim::Helpers::underlying = unl;
    for (auto it = Tim::Helpers::underlying.begin(); it!=Tim::Helpers::underlying.end(); ++it) {
        *it = std::tolower(*it);
    }
    std::cout << "ticker10len_list:"  << ticker10len_list << std::endl;
    MyLogger::log(TRADEFILE, "main |", "ticker10len_list:", ticker10len_list);
    Tim::ConnectorSetup myCon(connector_file_path, ticker_list, ticker10len_list,
        traded_ticker_list, configReader.date);
    // ticker_vec.push_back("NIFTY50");
    // ticker_vec.push_back("NIFTYIT");
    // ticker_vec.push_back("NIFTYFMCG");
    // ticker_vec.push_back("NIFTYENERGY");
    ticker_vec.push_back("NIFTYBANK");
    std::vector<std::string> v;
    bool isRaw = false;
    if(connector_file_path.find("raw") != std::string::npos){
        isRaw = true;
    }
    std::cout << "isRaw is " << isRaw << "\n";
    std::cout << "Symbol list\n";
    std::cout<<" Ticker vector size: "<<ticker_vec.size()<<"\n";
    std::cout<<"Traded Ticker list size "<<traded_ticker_list.size()<<"\n";
    for (auto it=ticker_vec.begin(); it!=ticker_vec.end(); it++) {
        std::cout << *it << std::endl;
    }
    // ticker_list=",NIFTY50,NIFTYBANK";
    std::cout << "\n";
    con = new ISHM::NSEConnectorWrapper(&md_cb, &resp_cb, myCon.connectorCfg.INTERACTION_MODE,
        &(myCon.connectorCfg), {}); //isRaw? ticker_vec :v);
    // ticker_list+=",NIFTY     ";

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
    createGraph(configReader.date);
    std::cout <<"Created Graph \n";
//    con->RegisterTimerCallback(&timer_cb);
    con->addTimer(mdHand->md_check->starttime, 0);
    std::cout << "MadeMap. Gonna start sync now";
    con->StartSync();
    return 0;
}

