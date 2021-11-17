/*
 *  ParamsBase.hpp
 *  Author: Pranay
 *      Created on: 05-July-2021
 *
 */
#ifndef PARAMS_DERIVED_HPP_
#define PARAMS_DERIVED_HPP_

#include "Params/Paramsbase.hpp"
/*
 *  - Loads and handles Params in a polymorphic manner.
 *  - Definitions are present in the child classes.
 *  - This ensures that all the dump params are not 
 *    exposed in a global struct.
 */
namespace Tim{
    struct TimeRange{
        int start_min;
        int start_hour;
        int end_hour;
        int end_min;
};
    struct DumpFrequency{
        char dump_unit;
        int frequency;
    }
    struct DumpParams{
        std::vector<Tim::TimeRange> time_range_vec;
        Tim::DumpFrequency;
        std::vector<std::string> columns;
        int hundred_plus_minus;
        
    }

}
class ParamloaderDerived : public Paramloader{
public:
    Tim::DumpParams* dump_params;    
    ParamloaderDerived(std::string param_file_path_t):
        Paramloader(param_file_path_t){
        
        strat_params = new Tim::DumpParams;
        initializeDumpParams(); 
    }
    
    void loadParams(){
        MyLogger::log(TEMPFILE, "Paramloader | ", "Loading Params..."); 
        FILE* fp = fopen(param_file_path.c_str(), "r");
        Baryons::rapidjson::FileStream is(fp);
        params_doc.ParseStream<0>(is);
        fclose(fp);
    }
  
    void reInitializeParams() override{
        loadParams();
        initializeGlobalParams();
        initializeDumpParams();
    }
 
    void initializeDumpParams() override{
        if (params_doc.HasMember("SGIParams")) {

            const Baryons::rapidjson::Value& paramDoc = params_doc["SGIParams"];
            assert(paramDoc.IsObject());
            if(paramDoc.HasMember("trader")){
                const Baryons::rapidjson::Value& trader= paramDoc["trader"];
                (*strat_params).trader = trader.GetString();
            }
            else{
                (*strat_params).trader = "";
            }
            if(paramDoc.HasMember("delta_lower_cap")){
                const Baryons::rapidjson::Value& d_l_cap= paramDoc["delta_lower_cap"];
                (*strat_params).delta_lower_cap = d_l_cap.GetDouble();
            }
            else{
                (*strat_params).delta_lower_cap = 0;
            }
            if(paramDoc.HasMember("delta_upper_cap")){
                const Baryons::rapidjson::Value& d_u_cap = paramDoc["delta_upper_cap"];
                (*strat_params).delta_upper_cap = d_u_cap.GetDouble();
            }
            else{
                (*strat_params).delta_upper_cap = 1;
            }
            if(paramDoc.HasMember("delta_tolerance")){
                const Baryons::rapidjson::Value& delta_tol = paramDoc["delta_tolerance"];
                (*strat_params).delta_tolerance = delta_tol.GetInt();
            }
            else{
                (*strat_params).delta_tolerance = 100;
            }
            if(paramDoc.HasMember("lower_delta_slope")){
                const Baryons::rapidjson::Value& lower_d_slope = paramDoc["lower_delta_slope"];
                (*strat_params).lower_delta_slope = lower_d_slope.GetDouble();
            }
            else{
                (*strat_params).lower_delta_slope = -1;
            }
            if(paramDoc.HasMember("upper_delta_slope")){
                const Baryons::rapidjson::Value& upper_d_slope = paramDoc["upper_delta_slope"];
                (*strat_params).upper_delta_slope = upper_d_slope.GetDouble();
            }
            else{
                (*strat_params).upper_delta_slope = 1;
            }
            if(paramDoc.HasMember("risk_growth_rate")){
                const Baryons::rapidjson::Value& risk_growth_rate = paramDoc["risk_growth_rate"];
                (*strat_params).risk_growth_rate = risk_growth_rate.GetDouble();
            }
            else{
                (*strat_params).risk_growth_rate = 0;
            }
            if(paramDoc.HasMember("no_of_lots")){
                const Baryons::rapidjson::Value& no_of_lots = paramDoc["no_of_lots"];
                (*strat_params).no_of_lots = no_of_lots.GetDouble();
            }
            else{
                (*strat_params).no_of_lots = 1;
            }
            if(paramDoc.HasMember("margin")){
                const Baryons::rapidjson::Value& margin = paramDoc["margin"];
                (*strat_params).margin = margin.GetDouble();
            }
            else{
                (*strat_params).margin = 0;
            }
            if(paramDoc.HasMember("trade_start_hour")){
                // std::cout<<"Trade Start hour"<<std::endl;
                const Baryons::rapidjson::Value& trade_start_hour = paramDoc["trade_start_hour"];
                if(trade_start_hour.IsInt()){
                    (*strat_params).trade_start_hour = trade_start_hour.GetInt();
                }
                else{
                    std::cout<<"Not Int"<<std::endl;
                }
            }
            else{
                (*strat_params).trade_start_hour = 9;
            }
            if(paramDoc.HasMember("trade_start_min")){
                const Baryons::rapidjson::Value& trade_start_min = paramDoc["trade_start_min"];
                (*strat_params).trade_start_min = trade_start_min.GetInt();
            }
            else{
                (*strat_params).trade_start_min = 15;
            }
            if(paramDoc.HasMember("trade_end_hour")){
                const Baryons::rapidjson::Value& trade_end_hour = paramDoc["trade_end_hour"];
                (*strat_params).trade_end_hour = trade_end_hour.GetInt();
            }
            else{
                (*strat_params).trade_end_hour = 15;
            }
            if(paramDoc.HasMember("trade_end_min")){
                const Baryons::rapidjson::Value& trade_end_min = paramDoc["trade_end_min"];
                (*strat_params).trade_end_min = trade_end_min.GetInt();
            }
            else{
                (*strat_params).trade_end_min = 29;
            }
            if(paramDoc.HasMember("timer_unit")){
                const Baryons::rapidjson::Value& timer_unit_t = paramDoc["timer_unit"];
                (*strat_params).timer_unit = timer_unit_t.GetInt()*(1000*1000*1000LL);
            }
            else{
                (*strat_params).timer_unit = 30 * (1000*1000*1000LL);
            }
            if(paramDoc.HasMember("aggress_start_time")){
                const Baryons::rapidjson::Value& aggress_start_time_t = paramDoc["aggress_start_time"];
                std::string aggress_start_time = aggress_start_time_t.GetString();
                (*strat_params).aggress_start_hour = std::stoi(aggress_start_time.substr(0,2));
                (*strat_params).aggress_start_min = std::stoi(aggress_start_time.substr(3,2));

            }
            else{
                (*strat_params).aggress_start_hour = 15;
                (*strat_params).aggress_start_min = 10;
            }
            if(paramDoc.HasMember("trade_timer")){
                const Baryons::rapidjson::Value& trade_timer = paramDoc["trade_timer"];
                (*strat_params).trade_timer = trade_timer.GetDouble()*(1000*1000*1000LL);
            }
            else{
                (*strat_params).trade_timer = (1000*1000*1000LL);
            }
            if(paramDoc.HasMember("improved_order_level_timeout")){
                const Baryons::rapidjson::Value& imp_level_timeout = paramDoc["improved_order_level_timeout"];
                (*strat_params).improved_order_level_timeout = imp_level_timeout.GetDouble()*(1000*1000*1000LL);
            }
            else{
                (*strat_params).improved_order_level_timeout = (1000*1000*1000LL);
            }
            if(paramDoc.HasMember("no_of_levels_to_improve")){
                const Baryons::rapidjson::Value& no_of_levels= paramDoc["no_of_levels_to_improve"];
                (*strat_params).no_of_levels_to_improve = no_of_levels.GetInt();
            }
            else{
                (*strat_params).no_of_levels_to_improve = 2;
            }
            if(paramDoc.HasMember("percent_spread_improve")){
                const Baryons::rapidjson::Value& percent_spread_improve = paramDoc["percent_spread_improve"];
                (*strat_params).percent_spread_improve = percent_spread_improve.GetInt();
            }
            else{
                (*strat_params).percent_spread_improve = 50;
            }
            if(paramDoc.HasMember("sample_interval")){
                const Baryons::rapidjson::Value& sample_interval = paramDoc["sample_interval"];
                (*strat_params).sample_interval = sample_interval.GetInt();
            }
            else{
                (*strat_params).sample_interval = 1;
            }
            if(paramDoc.HasMember("trade_sample_interval")){
                const Baryons::rapidjson::Value& trade_sample_interval = paramDoc["trade_sample_interval"];
                (*strat_params).trade_sample_interval = trade_sample_interval.GetInt();
            }
            else{
                (*strat_params).trade_sample_interval = 1;
            }
            if(paramDoc.HasMember("max_pos")){
                const Baryons::rapidjson::Value& max_pos = paramDoc["max_pos"];
                (*strat_params).max_pos = max_pos.GetInt();
            }
            else{
                (*strat_params).max_pos = 20;
            }
            if(paramDoc.HasMember("max_theo_pos")){
                const Baryons::rapidjson::Value& max_theo_pos = paramDoc["max_theo_pos"];
                (*strat_params).max_theo_pos = max_theo_pos.GetInt();
            }
            else{
                (*strat_params).max_theo_pos = 3;
            }
            if(paramDoc.HasMember("freeze_on_reject")){
                const Baryons::rapidjson::Value& freeze_on_reject = paramDoc["freeze_on_reject"];
                (*strat_params).freeze_on_reject = freeze_on_reject.GetBool();
            }
            else{
                (*strat_params).freeze_on_reject = true;
            }
            if(paramDoc.HasMember("exec_mode")){
                const Baryons::rapidjson::Value& exec_mode = paramDoc["exec_mode"];
                (*strat_params).exec_mode = exec_mode.GetString();
            }
            else{
                (*strat_params).exec_mode = "PASSIVE";
            }
            if(paramDoc.HasMember("spread")){
                const Baryons::rapidjson::Value& spread = paramDoc["spread"];
                (*strat_params).spread = spread.GetDouble();
            }
            else{
                (*strat_params).spread = 100;
            }
            if(paramDoc.HasMember("exit_condition")){
                const Baryons::rapidjson::Value& exit_condition = paramDoc["exit_condition"];
                (*strat_params).exit_condition = exit_condition.GetInt();
            }
            else{
                (*strat_params).exit_condition = 0;
            }
            if(paramDoc.HasMember("exit_with_timer")){
                const Baryons::rapidjson::Value& exit_with_timer = paramDoc["exit_with_timer"];
                (*strat_params).exit_with_timer = exit_with_timer.GetBool();
            }
            else{
                (*strat_params).exit_with_timer = false;
            }
            if(paramDoc.HasMember("exit_with_spot")){
                const Baryons::rapidjson::Value& exit_with_spot = paramDoc["exit_with_spot"];
                (*strat_params).exit_with_spot = exit_with_spot.GetBool();
            }
            else{
                (*strat_params).exit_with_spot = false;
            }
            if(paramDoc.HasMember("entry_with_timer")){
                const Baryons::rapidjson::Value& entry_with_timer = paramDoc["entry_with_timer"];
                (*strat_params).entry_with_timer = entry_with_timer.GetBool();
            }
            else{
                (*strat_params).entry_with_timer = false;
            }
            if(paramDoc.HasMember("entry_with_spot")){
                const Baryons::rapidjson::Value& entry_with_spot = paramDoc["entry_with_spot"];
                (*strat_params).entry_with_spot = entry_with_spot.GetBool();
            }
            else{
                (*strat_params).entry_with_spot = false;
            }
            if(paramDoc.HasMember("aggress_ticks")){
                const Baryons::rapidjson::Value& aggress_ticks = paramDoc["aggress_ticks"];
                (*strat_params).aggress_ticks = aggress_ticks.GetInt();
            }
            else{
                (*strat_params).aggress_ticks = 0;
            }
        }
        else {
            MyLogger::log(TRADEFILE,"Error:SGIParams missing");
        }

    };
};
#endif
