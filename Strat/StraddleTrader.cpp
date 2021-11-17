#pragma once // Avoids multiple inclusion of files

//create this new instrument class
//#include "CI_straddle.hpp"
#include "CI_straddle.cpp"
#include "Trade/TradeLogic.hpp"
#include "Indicators/ExpectedShock.hpp"

namespace Tim{
class StraddleTrader: public TradeLogic{
public:
	Tim::TradeParams* straddle_params;
	//Tim::ExpectedShock* shock_ind_;
	Tim::SpotPricing* spot_obj;
	//std::map<std::string, Tim::SpotPricing*> spot_obj_map;
	bool take_only_long_pos;
	uint64_t trade_starttime;
	uint64_t trade_endtime;

	//check if these params are needed and what is the functionality?
	StraddleTrader(Tim::UniverseHandler* unHand_t,
        Tim::Paramloader* params_t, Tim::StatsGen* stats, 
        Tim::MarginStats* margin_calc,
        std::map<int, Tim::TradeVals>* tradeBook_t,
        int date, int pid, Tim::SpotPricing* spot_obj_t):
        TradeLogic(unHand_t, params_t, stats, margin_calc, 
                tradeBook_t, date, pid){

	
	//shock_ind_ = shock_ind_t;
        spot_obj = spot_obj_t;
        straddle_params = new Tim::TradeParams();
	fetchParams();
        trade_starttime = Tim::Helpers::epochTimeInNanos(TradeLogic::date, 
            straddle_params->trade_start_hour, straddle_params->trade_start_min, 0, 0);
        trade_endtime = Tim::Helpers::epochTimeInNanos(TradeLogic::date, 
            straddle_params->trade_end_hour, straddle_params->trade_end_min, 0, 0);
        MyLogger::log(TRADEFILE, "Straddle Trade Starttime: ", trade_starttime);
        MyLogger::log(TRADEFILE, "Straddle Trade Endtime: ", trade_endtime);
	}

	//Fetching Parameters from NewParams.json file specific to Straddle Trade
	void fetchParams() override{

		if(TradeLogic::params->params_doc.HasMember("StraddleParams")){
            		const Baryons::rapidjson::Value& paramDoc =
                	TradeLogic::params->params_doc["StraddleParams"];
           		std::cout<<"Reading Trade Params..."<<std::endl;
           		 if(paramDoc.HasMember("trade_start_hour")){
                		const Baryons::rapidjson::Value& t_s_h= 
                    		paramDoc["trade_start_hour"];
                		straddle_params->trade_start_hour = t_s_h.GetInt();
            		}
            
            		if(paramDoc.HasMember("trade_start_min")){
                		const Baryons::rapidjson::Value& t_s_m= 
                   		paramDoc["trade_start_min"];
                		straddle_params->trade_start_min = t_s_m.GetInt();
            		}
            
            		if(paramDoc.HasMember("trade_end_hour")){
                		const Baryons::rapidjson::Value& t_e_h= 
                    		paramDoc["trade_end_hour"];
                		straddle_params->trade_end_hour = t_e_h.GetInt();
            		}
            
           		 if(paramDoc.HasMember("trade_end_min")){
                		const Baryons::rapidjson::Value& t_e_m= 
                    		paramDoc["trade_end_min"];
                		straddle_params->trade_end_min = t_e_m.GetInt();
            }
        }
    }


	void tradeDecisionMaker() override{
		//bool allClosed = true;
		//std::cout<<"Trade decision maker\n";
		for(auto ci = complex_instruments_book.begin(); ci!= complex_instruments_book.end(); ci++){
		//	std::cout<<"****Complex Ins********\n";
			for(auto si = (*ci)->constituent_simple_instruments.begin();
                          si != (*ci)->constituent_simple_instruments.end(); si++){
		//		std::cout<<"Symbol "<<(*si)->symbol_id <<" Status "<<(*si)->simple_instrument_status<<" Theo "<< (*si)->theo_position << " Actual "<< (*si)->actual_position<<"\n";
			}
		}

		if(complex_instruments_book.size() ==5){
		//	std::cout<<"Putting Status to Pending Exit\n";
			for(auto ci = complex_instruments_book.begin(); ci!= complex_instruments_book.end(); ci++){
                          for(auto si = (*ci)->constituent_simple_instruments.begin();
                            si != (*ci)->constituent_simple_instruments.end(); si++){                           
                                  if((*si)->simple_instrument_status == SimpleInstrumentStatus::SIOpen){
                                  		updateSIStates(*si, SimpleInstrumentStatus::SIPendingExit, (*si)->actual_fill_px);       
                                          
                                  }
		//		std::cout<<"Symbol "<<(*si)->symbol_id <<" Status "<<(*si)->simple_instrument_status<<" Theo "<< (*si)->theo_position << " Actual "<< (*si)->actual_position<<"\n";       
                         } 
			}
		}
		removeClosedCI();		

	

		if(complex_instruments_book.size() !=5){
		//std::cout<<"*********Creating New CI***********\n";
		Tim::ComplexInstrumentType ci_type =
			Tim::ComplexInstrumentType::Straddle;
	
		Tim::ComplexInstruments* ci = new Tim::CI_straddle(ci_type,
                          TradeLogic::unHand, TradeLogic::tradeBook, TradeLogic::params,
                          TradeLogic::stats, spot_obj, take_only_long_pos);

		ci->printComplexInstrument();

		TradeLogic::complex_instruments_book.push_back(ci);
		}
	}

	void exitChecks() override{
        	MyLogger::log(TEMPFILE, "StraddleTrader| exitChecks()");
    	}
};
}
