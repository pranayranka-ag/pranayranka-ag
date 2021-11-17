#include "ShockHandler.hpp"
//#include "agshocklib.h"
#include "GreekCalculator.h"
#include <chrono>


namespace Baryons{
  ShockHandler::ShockHandler(int date_t, uint64_t timestamp_t, uint64_t unit_time_t, std::map<int, Tim::TradeVals> *tradeBook_t,
      Tim::UniverseHandler *unHand_t, Tim::Paramloader* params_t,
      std::map<std::string, Tim::SpotPricing*>* spot_obj_map_t, Tim::MarginStats *margin_stats_t, std::string filepath_t,
      int priority):
    spot_obj_map(spot_obj_map_t),
    tradeBook(tradeBook_t),
    unHand(unHand_t),
    timestamp(timestamp_t),
    params(params_t),
    _on_timer_update(false),
    _on_greek_timer_update(false),
    margin_stats(margin_stats_t),
    filepath(filepath_t)
  {

    // count_atm_separate = dump_params->count_atm_separate;
    model_starttime = timestamp;
    date = date_t;
    global_params = params->global_params;
    trade_starttime = Tim::Helpers::epochTimeInNanos(date, global_params->start_hour, global_params->start_min, 0, 0);
    trade_endtime = Tim::Helpers::epochTimeInNanos(date, global_params->end_hour, global_params->end_min, 0, 0);
    setUnitTime(unit_time_t);
    for (int i = 0; i < spot_moves.size(); i++) {
		for (int j = 0; j < iv_moves.size(); j++) {
			all_moves.push_back(std::make_pair(spot_moves[i],iv_moves[j]));
		}
	}

  }
  
  void ShockHandler::findMarginShock(uint64_t timestamp){
	  std::vector<double> a(int(spot_moves.size())*int(iv_moves.size()),0);
	  std::vector<std::string> all_unds;
	  for (auto itr = tradeBook->begin(); itr != tradeBook->end(); itr++) {
		  std::string und = unHand->TradeUniverseIDs[itr->first]->underlyer;
		  if (find(all_unds.begin(), all_unds.end(), und) == all_unds.end())
			  all_unds.push_back(und);
	  }
	  for(int j = 0; j < all_unds.size();j++){
		  std::vector<double> b = margin_stats->theo_margin_api->getShocks(all_unds[j], all_moves);
		  std::transform (a.begin(), a.end(), b.begin(), a.begin(), std::plus<double>());
	  }
	  std::string fname = filepath + "shock_risk_data_marginlib_" + std::to_string(*Tim::Helpers::date_ptr) +"_" + std::to_string(*Tim::Helpers::pid_ptr) +  ".csv";
	  static std::ofstream output2;

	  static int logger_i2 = 0;
	  std::string out = "timestamp,";
	  if (logger_i2==0) {
		output2.open(fname, std::ios_base::trunc); // append instead of overwrite
		++logger_i2;
		out+="spot_0_iv_0,spot_0_iv_5,spot_0_iv_-5,spot_2_iv_0,spot_2_iv_5,spot_2_iv_-5,spot_-2_iv_0,spot_-2_iv_5,spot_-2_iv_-5,";
		out.pop_back();
		out+="\n";
		std::cout << out << std::endl;
		if (output2.is_open())
		  output2 << out;
		output2.close();
	  }
	  output2.open(fname, std::ios_base::app); // append instead of overwrite
	  std::stringstream ss("");
	  ss.precision(5);
	  ss << std::fixed;
	  ss << Tim::Helpers::getHumanReadableTime(timestamp) << ",";
	  for(int z = 0; z < int(spot_moves.size())*int(iv_moves.size()); z++){
		ss << a[z] << ",";
		a[z] = 0;
	  }
	  std::string ss_str = "";
	  ss_str = ss.str();
	  ss_str.pop_back();
	  output2 << ss_str << "\n";
	  output2.close();
  }

  void ShockHandler::findPortfolioRisk(uint64_t timestamp){
    for (auto itr = tradeBook->begin(); itr != tradeBook->end(); itr++) {
      int k = 0;
      for (int i = 0; i < spot_moves.size(); i++) {
        for (int j = 0; j < iv_moves.size(); j++) {
          GS->addShockParam(Tim::ShockParam::SPOT, spot_moves[i]);
          GS->addShockParam(Tim::ShockParam::IV, iv_moves[j]);
          Tim::Options* oldopt = unHand->TradeUniverseIDs[itr->first]->opt;
          auto this_greeks = GS->getGreeksForShock(oldopt->underlying_price, oldopt, 0.06);
          std::cout << "For spot shock of " << spot_moves[i] << " and iv shock of " << 
            iv_moves[j] << ":\n" << "Old Opt Price: " << oldopt->trade_px << 
            ", New Opt Price: " << oldopt->greek_shock_price<< ", Opt Spot: " << oldopt->underlying_price 
            << " Opt tte: " << oldopt->time_to_expiry << "< Opt IV: " << oldopt->volatility <<"\n" << "Position: " << 
            itr->second.actual_pos << "\n" << "Net risk: " << 
            itr->second.actual_pos * (oldopt->greek_shock_price - oldopt->trade_px) << "\n";

          portfolio_risk[k]+= itr->second.actual_pos * (oldopt->greek_shock_price - 
              oldopt->trade_px);
          GS->resetShockParam(Tim::ShockParam::SPOT);
          GS->resetShockParam(Tim::ShockParam::IV);
          k++;
        }
      }
    }
    std::string fname = *Tim::Helpers::folder_ptr + "shock_risk_data_" + std::to_string(*Tim::Helpers::date_ptr) +"_" + std::to_string(*Tim::Helpers::pid_ptr) +  ".csv";
    static std::ofstream output;
    
    static int logger_i = 0;
    std::string out = "timestamp,";
    if (logger_i==0) {
      output.open(fname, std::ios_base::trunc); // append instead of overwrite
      ++logger_i;
      out+="spot_0_iv_0,spot_0_iv_5,spot_0_iv_-5,spot_2_iv_0,spot_2_iv_5,spot_2_iv_-5,spot_-2_iv_0,spot_-2_iv_5,spot_-2_iv_-5,";
      out.pop_back();
      out+="\n";
      std::cout << out << std::endl;
      if (output.is_open())
        output << out;
      output.close();
    }
    output.open(fname, std::ios_base::app); // append instead of overwrite
    std::stringstream ss("");
    ss.precision(5);
    ss << std::fixed;
    ss << Tim::Helpers::getHumanReadableTime(timestamp) << ",";
    for(int z = 0; z < 9; z++){
      ss << portfolio_risk[z] << ",";
      portfolio_risk[z] = 0;
    }
    std::string ss_str = "";
    ss_str = ss.str();
    ss_str.pop_back();
    output << ss_str << "\n";
    output.close();
  }



  void ShockHandler::onNotify(){
    MyLogger::log(TRADEFILE, "ShockHandler::onNotify |", "_on_timer_update:", _on_timer_update, " |_on_greek_time_update:", _on_greek_timer_update);
    if(_on_greek_timer_update){
      _on_timer_update = false;
      _on_greek_timer_update = false;
      std::cout << "Acceptable TimeStamp: " << Tim::Helpers::getHumanReadableTime(timestamp) << std::endl;
        std::cout << "Entering dumper \n";
      ShockHandler::findMarginShock(timestamp);
       std::cout << "Acceptable TimeStamp: " << Tim::Helpers::getHumanReadableTime(timestamp) << std::endl;
        std::cout << "Exiting dumper \n";
    }
  }

  void ShockHandler::setUnitTime(uint64_t unit_time_t){
    unit_time = unit_time_t;
  }

  uint64_t ShockHandler::getUnitTime(){
    return unit_time;
  }

  void ShockHandler::onTimerUpdate(uint64_t timestamp_new){
//     if(timestamp_new > model_starttime) {
//       if(timestamp + unit_time <= timestamp_new){
//         timestamp += unit_time;
//         setDirty();
//         _on_timer_update = true;
//       }
//     }
  }

  void ShockHandler::onGreekTimerUpdate(uint64_t timestamp_new){
    if (timestamp_new >= model_starttime) {
      if(timestamp + unit_time <= timestamp_new){
        timestamp += unit_time;
        setDirty();
        _on_greek_timer_update = true;
      }
    }
  }
}
