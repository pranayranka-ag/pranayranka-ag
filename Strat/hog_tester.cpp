
#include <bits/stdc++.h>
#include "s_rapidjson/document.h"
#include "ConnectorConfig/ConnectorInit.hpp"
#include "Params/Paramsbase.hpp"
#include "SymbolLoader/NSESymbolLoader.hpp"
#include "UniverseMaker/UniverseHandler.hpp"
#include "Helpers/helper_structs.hpp"
#include "Helpers/config_reader.hpp"
#include "Execution/ExecutionManager.h"
#include "Trade/RandIntTrader.hpp"
#include "ValidityChecks/MDValid.hpp"
#include "Spot/nse_spot_pricing.hpp"
#include "MarginLotSetter/MarginLotReader.hpp"
#include "CallBackManager/GreekHandler.hpp"
#include "CallBackManager/SpotPriceHandler.hpp"
#include "CallBackManager/OptPriceHandler.hpp"
#include "CallBackManager/MDHandler.hpp"
#include "Stats/StatsGen.hpp"
#include "PnLCalculator/PnlCalculator.hpp"
#include "CallBackManager/PriceMapListener.hpp"
#include "Responses/RespHandler.hpp"
#include "CallBackManager/TimerHandler.hpp"
#include "CallBackManager/TradeHandler.hpp"
#include "RiskChecker/RiskChecker.hpp"
#include "CallBackManager/TradeHandler.hpp"
#include "CallBackManager/StatsPrinter.hpp"
#include "CallBackManager/RiskCheckTimer.hpp"
#include "CallBackManager/RiskListener.hpp"
#include "CallBackManager/IndicatorHandler.hpp"
#include "ExitHandler/ExitHandler.hpp"
#include "Indicators/RandomIntIndicator.hpp"
#include "Indicators/ExpectedShock.hpp"

using namespace illuminati::md;

uint64_t timestamp_g = 0;
uint64_t *Tim::Helpers::timestamp_ptr = &timestamp_g; 
int *Tim::Helpers::date_ptr = NULL; 
int *Tim::Helpers::pid_ptr = NULL;

ISHM::NSEConnectorWrapper *con;
Tim::ExecutionManager* exec_logic;
Tim::Helpers* helper = new Tim::Helpers;
Tim::RandIntTrader* trader;
Tim::NSESpotPricing* spot_obj;
Tim::UniverseHandler* unHand;
Tim::StatsGen* stats;
Tim::MarginStats* margin_calc;
Tim::PNL* pnl_calc;
Tim::OverallOrderManager* OOM;
Tim::RespHandler* respHand;
Tim::RiskChecker* risk_checker;
Tim::RandomIntIndicator* rand_ind_;
Tim::ExpectedShock* shock_ind_;

Baryons::CallBackManager* CBM;
Baryons::MDHandler* mdHand;
Baryons::OptPriceHandler* optHand;
Baryons::SpotPriceHandler* spotHand;
Baryons::GreekHandler* greekHand;
Baryons::PriceMap* priceHand;
Baryons::TimerHandler* timeHand;
Baryons::StrategyTimerHandler* stratHand;
Baryons::RiskCheckTimer* riskHand;
Baryons::StatsTimeHandler* statsHand;
Baryons::IndicatorHandler* indHand;


std::vector<Tim::RespData> resp_vector;
std::map<std::string, Tim::execution_data> executed_position;
std::map<std::string, Tim::TradeVals> tradeBook;
std::map<uint32_t, Tim::order_data> order_map;
std::map<std::string, double> lot_size_map;

Tim::Paramloader* params_pointer;

void createGraph(int date, int mode){
    CBM = new Baryons::CallBackManager();
    CBM->setInstance(CBM);

    mdHand = new Baryons::MDHandler(params_pointer->global_params->start_hour, 
        params_pointer->global_params->start_min,
        params_pointer->global_params->end_hour, 
        params_pointer->global_params->end_min, date);
    optHand = new Baryons::OptPriceHandler(unHand);
    spotHand = new Baryons::SpotPriceHandler(spot_obj);
    greekHand = new Baryons::GreekHandler(100.0, 0.06);
    priceHand = new Baryons::PriceMap();

    pnl_calc = new Tim::PNL(resp_vector, priceHand->price_map->market_data_map, "", 0.01);
    auto it = lot_size_map.begin();
    stats = new Tim::StatsGen(unHand, pnl_calc, margin_calc, &tradeBook, spot_obj, it->second);

    timeHand = new Baryons::TimerHandler(params_pointer->global_params->start_hour, 
        params_pointer->global_params->start_min,
        params_pointer->global_params->end_hour, 
        params_pointer->global_params->end_min, date);
    
    shock_ind_ = new Tim::ExpectedShock(unHand, spot_obj);
    indHand = new Baryons::IndicatorHandler(0, 1*1e9, rand_ind_); 
    stratHand = new Baryons::StrategyTimerHandler(date, 0, 0, unHand, 
        &tradeBook, &executed_position);
    int pid = 0;
    trader = new Tim::RandIntTrader(unHand, params_pointer, stats, margin_calc, &tradeBook,
        date, pid, shock_ind_, spot_obj);
    stratHand->ind_trader = trader;
    riskHand = new Baryons::RiskCheckTimer(0, 1*1e9, risk_checker, pnl_calc,
        unHand, &tradeBook);
    statsHand = new Baryons::StatsTimeHandler(stats, 0, 30*1000000000ull, pnl_calc);
    
    CBM->addEdge(mdHand, optHand);
    CBM->addEdge(mdHand, spotHand);
    CBM->addEdge(mdHand, priceHand);
    CBM->addEdge(optHand, greekHand);
    CBM->addEdge(spotHand, greekHand);

    CBM->addEdge(timeHand, stratHand);
    CBM->addEdge(timeHand, indHand);
    CBM->addEdge(indHand, stratHand);
    CBM->addEdge(timeHand, riskHand);
    CBM->addEdge(timeHand, statsHand);
    CBM->addEdge(riskHand, stratHand);

    mdHand->addMDListener(optHand);
    mdHand->addMDListener(spotHand);
    mdHand->addMDListener(priceHand);

    timeHand->addTimerListener(riskHand);
    timeHand->addTimerListener(stratHand);
    timeHand->addTimerListener(statsHand);
    timeHand->addTimerListener(indHand);
    indHand->addRandomListener(stratHand);
    riskHand->addRiskListener(stratHand);

    spotHand->addSpotPx_listener(greekHand);
    optHand->addOptPx_listener(greekHand);

    timeHand->ResolveMinTime(); 
}

bool Init_time = false;
int date_;

void PrintBook(){

    for(auto itr = unHand->TradeUniverse.begin(); itr != unHand->TradeUniverse.end(); itr++){
        if(itr->second->isOption){
            itr->second->opt->printOption();
        }
    }
}

static void md_cb(uint16_t symbolID, const illuminati::md::MarketUpdateNew* md){
    bool isOkay = false;
    mdHand->setMD_(md, &isOkay);
    CBM->notifyAll();
}


static void timer_cb(uint64_t timestamp){

    uint64_t newtime = ((timestamp / timeHand->unit_time_this) + 1) * timeHand->unit_time_this;
    //std::string ts = helper->getHumanReadableTime(timestamp);
    //std::string new_ts = helper->getHumanReadableTime(newtime);

    MyLogger::log(TRADEFILE, "Currts: ", timestamp);
    MyLogger::log(TRADEFILE, "New Ts: ", newtime);
    if(!Init_time){
        int hr_ = helper->getHourOfDay(timestamp);
        int min_ = helper->getMinuteOfDay(timestamp);
        int sec_ = helper->getSecondOfDay(timestamp);

        int time_adder = 1;
        if(sec_ > 55){
            time_adder = 2;
        }

        int this_min_ = min_;
        min_ = (min_ + time_adder)%60;
        int add_min_ = (this_min_ + time_adder)/60;
        hr_ += add_min_;

        struct tm t = {0};
        t.tm_year = helper->getYear(date_);
        t.tm_mon = helper->getMonth(date_);

        t.tm_mday = helper->getDay(date_);
        t.tm_hour = hr_;
        t.tm_min = min_;
        t.tm_sec = 0;
        std::time_t timeSinceEpoch = mktime(&t);
        newtime = long(timeSinceEpoch)*1000000000ull;
        Init_time = true;
        std::string ts = helper->getHumanReadableTime(newtime);
        std::string ts_curr = helper->getHumanReadableTime(timestamp);
        MyLogger::log(TRADEFILE, "Curr ts: ", ts_curr);
        MyLogger::log(TRADEFILE, "New round time set: ", ts);
        con->addTimer(newtime, 0);
        return;
    }
    timeHand->setTimer_(timestamp);
    //Update strat_params
    //Tim::PortfolioGreeks pg_ = stats->FetchPortfolioGreeks(true, false, false, false, true, false);
    //MyLogger::log(TRADEFILE, "Call IV: ", pg_.atm_call_iv);
    //MyLogger::log(TRADEFILE, "Put IV: ", pg_.atm_put_iv);
    //shock_ind_->setExpectedShock();
    //MyLogger::log(TRADEFILE, "Expected Shock Call: ", shock_ind_->call_expected_shock);
    //MyLogger::log(TRADEFILE, "Expected Shock Put: ", shock_ind_->put_expected_shock);
    //params.spot_mid = spot_obj->getSpotStruct().ask_spot_price;
    //auto it = lot_size_map.begin();
    //params.curr_port_delta = pg_.port_delta/it->second;
    //params.call_iv = pg_.atm_call_iv;
    //params.put_iv = pg_.atm_put_iv;
    //params.call_expected_shock = shock_ind_->call_expected_shock;
    //params.put_expected_shock = shock_ind_->put_expected_shock;

    CBM->notifyAll();
    if(timestamp >= trader->trade_starttime && timestamp <
        trader->trade_endtime){
        
        MyLogger::log(TRADEFILE, "Executing orders...");
        int percent_tol = 0;
        Tim::ModeExecution mode;
        mode = Tim::ModeExecution::PASSIVE;
        int no_of_lots = 6;
        exec_logic->manageExecution(tradeBook, order_map,
            priceHand->price_map->market_data_map, lot_size_map, OOM,
            0.75*1e9, timestamp, true, mode, 0,
            no_of_lots, 4, 25);
    }

    if(timestamp >= trader->trade_endtime){
        pnl_calc->DisplayPnl();
        stats->DisplayGreeks(true, true, true, true, true);
        std::vector<double> addToDumpLine;
        shock_ind_->setExpectedShock();
        double mean_expected_shock = (shock_ind_->call_expected_shock + shock_ind_->put_expected_shock)/2.0;
        addToDumpLine.push_back(mean_expected_shock);
        MyLogger::flush();
        MyLogger::Stop();
        exit(0);
    }

    con->addTimer(newtime, 0);

    return;
}

static void resp_cb(illuminati::infra::ResponseMsg* resp){
    std::string unl = helper->getUnderlyer(resp->Symbol);
    int lot_size = lot_size_map[unl];
    respHand->processResponse(resp, lot_size);
}

static void nomarketdata_callback(){

}


int main(int argc, char* argv[]){
    Tim::ConfigReader configReader = Tim::ConfigReader::GetInstance(argc, argv);
    std::string pos_file_path = "";
    std::string pnl_file_path = "";
    std::string shock_file_path = "";
    std::cout<<"Sym file path: "<<configReader.symbol_file_path<<std::endl;

    MyLogger::GetInstance(configReader.symbol_file_path, configReader.date, configReader.pid);
    date_ = configReader.date;

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

    int h = helper->getKRXExpiry(configReader.date);
    if(path_resolver.HasMember("pos_file_path")){
        const Baryons::rapidjson::Value& pos_path= path_resolver["pos_file_path"];
        std::string pos_path_suffix = pos_path.GetString();
        pos_file_path += pos_path_suffix + "SGposFile_" + std::to_string(configReader.pid) +
            "_" + std::to_string(configReader.date) + ".txt";
    }
    else{
        MyLogger::log(TRADEFILE, "Pos file path undefined!");
    }

    if(path_resolver.HasMember("pnl_file_path")){
        const Baryons::rapidjson::Value& pnl_path= path_resolver["pnl_file_path"];
        std::string pnl_path_suffix = pnl_path.GetString();
        pnl_file_path += pnl_path_suffix + "SGpnlFile_" + std::to_string(configReader.pid) +
            "_" + std::to_string(configReader.date) + ".txt";
    }
    else{
        MyLogger::log(TRADEFILE, "Pnl file path undefined!");
    }

    if(path_resolver.HasMember("shock_file_path")){
        const Baryons::rapidjson::Value& shock_path= path_resolver["shock_file_path"];
        std::string shock_path_suffix = shock_path.GetString();
        shock_file_path += shock_path_suffix + "SGShock_" + std::to_string(configReader.pid) +
            "_" + std::to_string(configReader.date) + ".txt";
    }
    else{
        MyLogger::log(TRADEFILE, "Shock file path undefined!");
    }

    std::string trade_info_path = "";
    if(configReader.mode == 1){
    trade_info_path = "/home/pjain/Hog_local/FileDumps/SGTradeInfo_" +
        std::to_string(configReader.date) + "_.txt";
    }

    MyLogger::log(TRADEFILE, "Resolved paths: ");
    MyLogger::log(TRADEFILE, "pos_file_path: ", pos_file_path);
    MyLogger::log(TRADEFILE, "pnl_file_path: ", pnl_file_path);
    MyLogger::log(TRADEFILE, "shock_file_path: ", shock_file_path);

    params_pointer = new Tim::Paramloader(configReader.param_file_path);
    params_pointer->loadParams();
    params_pointer->InitializeGlobalParams(); 
    //Specifying Instrument Type
    Tim::Helpers::date_ptr = &configReader.date; 
    Tim::Helpers::pid_ptr = &configReader.pid;
    int MIN_SYM_SIZE_NSE = 20;
    Tim::NSESymbolLoader symLoad(configReader.symbol_file_path,
        MIN_SYM_SIZE_NSE, configReader.date, "");
    symLoad.displayStratSymbols();

    std::map<std::string, std::vector<std::string>> mySyms;
    Tim::ExpiryTime expiry = Tim::ExpiryTime::EXPIRY_1;
    if(helper->getWeekday(configReader.date) == 4){
        expiry = Tim::ExpiryTime::EXPIRY_2;
    }

    symLoad.getStratSymbols(mySyms, expiry);

    std::string dir_file_path;
    if(configReader.mode == 2){
        dir_file_path += "/nasteamdata/pjain/metafiles/margin_files/";
        }
    else{
        dir_file_path += "/home/pjain/margin_files/";
    }

    std::vector<std::string> traded_ticker_list;
    std::string ticker_list="";
    std::string underlier_for_this_strat = "";
    std::string ticker10len_list = "";

    for (auto itr = symLoad.instrument_info_vector.begin(); itr != symLoad.instrument_info_vector.end(); itr++){
        std::string unl = (*itr).Underlier;
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
                underlier_for_this_strat = unl;
                break;
            }
        }

        else if(unl == "NIFTY"){
            std::string nft_file_path = "";
            const Baryons::rapidjson::Value& spot_constituents_path =
                path_resolver["spot_constituents_path"];
            if(spot_constituents_path.HasMember("NFT")){
                const Baryons::rapidjson::Value& nft_path = spot_constituents_path["NFT"];
                nft_file_path += nft_path.GetString();
            }
            spot_obj = new Tim::NSESpotPricing(nft_file_path, configReader.date);
            underlier_for_this_strat = unl;
            break;
            }

        else{
            std::cout<<"No known underlier found!"<<std::endl;
        }
    }

    for (auto itr = spot_obj->spot_constituents.begin();
            itr != spot_obj->spot_constituents.end(); itr++){

        std::string spot_sym = std::string(itr->first);
        std::string spot_sym_10len = helper->make10lenString(spot_sym);
        ticker10len_list += spot_sym_10len + ",";
    }

    for(auto x = mySyms.begin(); x != mySyms.end(); x++){
        for (auto y = x->second.begin(); y != x->second.end(); y++){
            ticker_list += std::string(*y) + std::string(",");
            traded_ticker_list.push_back(*y);
        }
    }

    exec_logic = new Tim::ExecutionManager(5);
    OOM = new Tim::OverallOrderManager();
    unHand = new Tim::UniverseHandler("NSE", traded_ticker_list, configReader.date);
    unHand->loadUniverse();

    MyLogger::log(TRADEFILE, "Trade Universe Test\n");

    MyLogger::log(TRADEFILE, "Setting up Margin percent and lot size...");
    Tim::MarginLotReader* ml_reader = new Tim::MarginLotReader(configReader.date,
        dir_file_path, lot_size_map, unHand);
    ml_reader->updateLotSizeMap();
    ml_reader->updateMarginPercent();

    risk_checker = new Tim::RiskChecker(params_pointer->global_params->max_pos, 
        params_pointer->global_params->global_max_pos, 1000000, 10000000, unHand);
    for (auto itr = unHand->TradeUniverse.begin(); itr != unHand->TradeUniverse.end(); itr++){
        if(itr->second->isOption){
            MyLogger::log(TRADEFILE, itr->first);
            std::string unl = helper->getUnderlyer(itr->first);
            itr->second->opt->updateLotSize(lot_size_map[unl]);
        }
    }

    for(auto itr = spot_obj->spot_constituents.begin();
            itr != spot_obj->spot_constituents.end(); itr++){

        std::string spot_sym = std::string(itr->first);
        std::string spot_sym_10len = helper->make10lenString(spot_sym);
        traded_ticker_list.push_back(spot_sym_10len);
    }
    margin_calc = new Tim::MarginStats(60000000, configReader.date);
    respHand = new Tim::RespHandler(&resp_vector, &tradeBook, &executed_position, &order_map, OOM);
    rand_ind_ = new Tim::RandomIntIndicator();
    createGraph(configReader.date, configReader.mode);

    Tim::ConnectorSetup myCon(connector_file_path, ticker_list, ticker10len_list,
        traded_ticker_list, configReader.date);
    std::vector<std::string> v;
    bool isRaw = false;
    if(connector_file_path.find("raw") != std::string::npos){
        isRaw = true;
    }
    con = new ISHM::NSEConnectorWrapper(&md_cb, &resp_cb, myCon.connectorCfg.INTERACTION_MODE,
        &(myCon.connectorCfg), isRaw? traded_ticker_list : v);

    //con->RegisterNoMDCallback(&(nomarketdata_callback));
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
    con->RegisterTimerCallback(&timer_cb);
    con->addTimer(mdHand->md_check->starttime, 0);
    OOM->MakeMap(traded_ticker_list, con, "NSE");
    con->StartSync();
    return 0;
}
