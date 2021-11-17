#include "CI_straddle.hpp"

namespace Tim{
	CI_straddle::CI_straddle(Tim::ComplexInstrumentType 
        complex_instrument_type,
        Tim::UniverseHandler* unHand_t, 
        //std::map<std::string, Tim::TradeVals>* tradeBook_t,
        std::map<int, Tim::TradeVals>* tradeBook_t,
        Tim::Paramloader* params_t,
        Tim::StatsGen* stats_t,
        //Tim::ExpectedShock* shock_ind_t,
        Tim::SpotPricing* spot_obj_t,
        bool take_only_long_pos_t
        ):
        stats(stats_t),
        //shock_ind(shock_ind_t),
        spot_obj(spot_obj_t),
        take_only_long_pos(take_only_long_pos_t),
        ComplexInstruments(complex_instrument_type, 
            unHand_t, params_t, tradeBook_t,1){
        
        
        ci_type = Tim::ComplexInstrumentType::IVLong;
        fetchParams(); 
        //makeComplexInstrument();
	make_complex_instrument();
    }


	void CI_straddle::fetchParams(){
//	std::cout<<"Inside fetch params\n";
	// Fetch this from NewParams.json
        if(ComplexInstruments::params->params_doc.HasMember("CIStraddleParams")){
            const Baryons::rapidjson::Value& paramDoc =
            ComplexInstruments::params->params_doc["CIStraddleParams"];
            assert(paramDoc.IsObject());

	//check where are we using these params

	if(paramDoc.HasMember("delta_lower_cap")){
                const Baryons::rapidjson::Value& d_l_cap= 
                    paramDoc["delta_lower_cap"];
                delta_lower_cap = d_l_cap.GetDouble();
            }
            else{
                delta_lower_cap = 0;
            }
            if(paramDoc.HasMember("delta_upper_cap")){
                const Baryons::rapidjson::Value& d_u_cap = 
                    paramDoc["delta_upper_cap"];
                delta_upper_cap = d_u_cap.GetDouble();
            }
            else{
                delta_upper_cap = 1;
            }
            if(paramDoc.HasMember("no_of_lots")){
                const Baryons::rapidjson::Value& no_of_lots_= 
                    paramDoc["no_of_lots"];
                no_of_lots = no_of_lots_.GetInt();
            }
            else{
                no_of_lots = 0;
            }
            if(paramDoc.HasMember("shock_val")){
                const Baryons::rapidjson::Value& shock_val_= 
                    paramDoc["shock_val"];
                shock_val = shock_val_.GetDouble();
            }
            else{
                shock_val = 0;
            }
            if(paramDoc.HasMember("mean_shock_value")){
                const Baryons::rapidjson::Value& mean_shock_value_ = 
                    paramDoc["mean_shock_value"];
                mean_shock_value = mean_shock_value_.GetDouble();
            }
            else{
                mean_shock_value = 0;
            }
            if(paramDoc.HasMember("shock_scaler")){
                const Baryons::rapidjson::Value& shock_scaler_= 
                    paramDoc["shock_scaler"];
                shock_scaler = shock_scaler_.GetDouble();
            }
            else{
                shock_scaler = 0;
            }
        }

        Tim::PortfolioGreeks pg_ = 
            stats->FetchPortfolioGreeks(true, false, false, false, true);
        curr_port_delta = pg_.port_delta;
        call_iv = pg_.atm_call_iv;
        put_iv = pg_.atm_put_iv;

        //shock_ind->setExpectedShock();
       // call_expected_shock = shock_ind->call_expected_shock;
       // put_expected_shock = shock_ind->put_expected_shock;

        spot_mid = spot_obj->getSpotStruct()->ask_spot_price;
    }

void CI_straddle::make_complex_instrument(){

	//std::cout<<"New Complex Instrument\n";

	Tim::SimpleInstruments* si_p = SimpleInstruments::allocate();
	Tim::SimpleInstruments* si_c = SimpleInstruments::allocate();
	int theo_pos = 0;
	double price = 0;
	//Fix ATM logic
	std::tuple<Tim::Options*,Tim::Options*,int, int> ATMOptions = findATMOption();
	Tim::Options* ATMCall = std::get<0>(ATMOptions);
	int atmCallId = std::get<2>(ATMOptions);
	Tim::Options* ATMPut = std::get<1>(ATMOptions);
	int atmPutId = std::get<3>(ATMOptions);
	//Set these values correctly later when executing
	theo_pos = 1;
	//price = ATMOption.first->bid_px;
	//std::cout<<"ATM Option Found: "<<ATMOption->symbol<<"\n";
	//if(constituent_simple_instruments.size()<= 180){
        //         theo_pos = 1;
	//	price = ATMOption.first->bid_px;
        // }
        // else{
         //        theo_pos = -1;
	//	price = ATMOption.first->ask_px;
        // }

	//for(auto it=unHand->TradeUniverseIDs.begin(); it!=unHand->TradeUniverseIDs.end(); ++it) {
                       //std::string underlyer = (it->second->underlyer);
        //                  std::cout<<it->second->isOption<<"\n";
	//		if(it->second->isOption) {
 	//                 Tim::Options* this_opt = it->second->opt;
	//		int this_ID = it->first;
	//		std::cout<<"Symbol ID"<<this_opt->delta<<"\n";

	//Tim::Options* this_opt = ComplexInstruments::unHand->TradeUniverse[v[1].second]->opt 
	//std::cout<<"ATM Call Id "<<atmCallId<<" ATMCallDelta "<<ATMCall->delta<< " ATM Put Id "<<atmPutId<<" ATM Put Delta "<<ATMPut->delta<<"\n";
        	si_p->initialize(Tim::SimpleInstrumentType::SIPut,
       		 ATMPut, ATMPut, atmPutId, theo_pos, ATMPut->bid_px);
               		//si_p->theo_position = 1;
              		si_p->simple_instrument_status = Tim::SimpleInstrumentStatus::SIPendingEntry;
              	 	constituent_simple_instruments.push_back(si_p);
			//std::cout<<si_p->opt->delta<<"Put Delta\n";

	si_c->initialize(Tim::SimpleInstrumentType::SICall,
                  ATMCall, ATMCall, atmCallId, theo_pos, ATMCall->bid_px);
                         //si_c->theo_position = 1;
                         si_c->simple_instrument_status = Tim::SimpleInstrumentStatus::SIPendingEntry;
                         constituent_simple_instruments.push_back(si_c);

		//std::cout<<si_c->opt->delta<<"Call Delta\n";

//}	
//}
//	std::cout<<"Length Simple Instrument:"<<constituent_simple_instruments.size()<<"\n";
}

std::tuple<Tim::Options*,Tim::Options*, int,int> CI_straddle::findATMOption(){
	double max_delta = 0.5;
	double min_delta = -0.5;
	Tim::Options* ATMCallOption = nullptr;
	Tim::Options* ATMPutOption = nullptr;
	Tim::Options* this_opt = nullptr;
	int this_call_id = -1;
	int this_put_id = -1;
	double callDiff = 1;
	double putDiff = 1;
	for(auto it=unHand->TradeUniverseIDs.begin(); it!=unHand->TradeUniverseIDs.end(); ++it) {
		if(it->second->isOption){
			this_opt = it->second->opt;
			
			//std::cout<<"Delta Check "<<this_opt->delta<<"\n";
			//if(this_opt->opt_type == OptionType::Call){
			//	std::cout<<this_opt->symbol_id<<" ** "<<this_opt->delta<<" ** "<<this_opt->strike<<"\n";
			//}
			//if(this_opt->opt_type == OptionType::Call && std::abs(this_opt->delta -0.5) < callDiff){
			//	callDiff = std::abs(this_opt->delta -0.5);
			//	ATMCallOption = this_opt;
			//	this_call_id = it->first;
		//	}
		//	if(this_opt->opt_type == OptionType::Put && std::abs(this_opt->delta + 0.5) < putDiff){
                //                 putDiff = std::abs(this_opt->delta +0.5);
                //                 ATMPutOption = this_opt;
                //                 this_put_id = it->first;
                 //        }

			if( this_opt->opt_type == OptionType::Call && this_opt->delta < max_delta && (max_delta - this_opt->delta) < callDiff){
				callDiff = max_delta - this_opt->delta;
				ATMCallOption = this_opt;
				this_call_id = it->first;
			}
			if( this_opt->opt_type == OptionType::Put && this_opt->delta > min_delta && (this_opt->delta - min_delta) < putDiff){
                                 putDiff = this_opt->delta - min_delta;
                                 ATMPutOption = this_opt;
                                 this_put_id = it->first;                         }
					
		}
	 }
	
	//MyLogger::log(TRADEFILE, "\n----------\nCI STraddle::ATM Call ID: ",
         //    ATMCallOption->symbol_id, "  ATM Call Delta ", ATMCallOption->delta, "  ATM Call Strike ", ATMCallOption->strike, "\n-------\n");

	//MyLogger::log(TRADEFILE, "\n----------\nCI Straddle::ATM Put ID: ",
          //    ATMPutOption->symbol_id, "  ATM Put Delta ", ATMPutOption->delta, "  ATM Put Strike ", ATMPutOption->strike, "\n-------\n");
//	std::cout<<"ATM Call Selected "<<ATMCallOption->symbol_id<<" ** "<<ATMCallOption->delta<<" ** "<<ATMCallOption->strike<<"\n";
//	std::cout<<"ATM Put Selected "<<ATMPutOption->symbol_id<<" ** "<<ATMPutOption->delta<<" ** "<<ATMPutOption->strike<<"\n";
	return std::make_tuple(ATMCallOption, ATMPutOption,this_call_id, this_put_id); 
}

void CI_straddle::makeComplexInstrument(){
	std::cout<<"Make Complex Instruments\n";

        
        if(!take_only_long_pos){
           // findSurface();
        }

        std::pair<int, int> long_counts = getLongPositionsNumber();
	std::cout<<"Call"<<long_counts.first<<"Put"<<long_counts.second<<"\n";
        MyLogger::log(TRADEFILE, "\n----------\nCall_long pos: ",
            long_counts.first, "  Put_long pos: ", long_counts.second, "\n-------\n");

        Tim::ComplexInstrumentType long_pos_type = ci_type;
        double call_limit;
        double put_limit;

        std::map<double, std::vector<std::string>> search_map;
        std::vector<std::pair<double, std::string>> v;

        if(long_pos_type != Tim::ComplexInstrumentType::DeltaLong){
            search_map = ComplexInstruments::getSearchMap("Strike");
        }
        else{
            search_map = ComplexInstruments::getSearchMap("Delta");
        }

        switch (long_pos_type){
            case Tim::ComplexInstrumentType::IVLong:{
                double call_limit = getLongCutoff('c');
                double put_limit = getLongCutoff('p');
                double upper_spot = spot_mid*(1+call_limit);
                double lower_spot = spot_mid*(1-put_limit);

                MyLogger::log(TRADEFILE, 
                    "\nLimits obtained for IV Long positions:=\n",
                    "Upper_spot_limit = ", call_limit, "\n", 
                    "Lower_spot_limit = ", put_limit, "\n",
                    "Upper Spot = ", upper_spot, "\n",
                    "Lower Spot = ", lower_spot, "\n",
                    "Actual Spot = ", spot_mid, "\n");

                std::pair<double, std::string> pLong = 
                    std::make_pair(lower_spot, "");
                std::pair<double, std::string> cLong = 
                    std::make_pair(upper_spot, "");
                
                v.push_back(pLong);
                v.push_back(cLong);

                ComplexInstruments::findNearestStrike(search_map, v[0].first, v[0].second, 1);
                ComplexInstruments::findNearestStrike(search_map, v[1].first, v[1].second, 0);

                break;
            }

            case Tim::ComplexInstrumentType::DeltaLong:{
		//Later
		}
	default:
                break;
        }

        int temp_call = std::abs(long_counts.first);
        int sign_call = ((long_counts.first < 0) - (0 < long_counts.first));

	if(!v[1].second.empty() && long_counts.first < 0){
            while(temp_call--){
                Tim::Options* cl = ComplexInstruments::unHand->TradeUniverse[v[1].second]->opt;
		Tim::SimpleInstruments* si_call = new Tim::SimpleInstruments();
		si_call->simple_instrument_type = Tim::SimpleInstrumentType::SICall;
		si_call->opt = cl;
		si_call->opt_entry = cl;
		//check this
		si_call->symbol_id = v[1].first;
		//si_call->theo_position = sign_call;
		si_call->theo_position = 1;
		si_call->theo_fill_px = 0;
		std::cout<<"Filling Call SI\n";
                //Tim::SimpleInstruments* si_call = new Tim::SimpleInstruments(Tim::SimpleInstrumentType::SICall, cl, cl, v[1].second, sign_call, 0);
                constituent_simple_instruments.push_back(si_call);
            }
        }

        int temp_put = std::abs(long_counts.second);
        int sign_put = ((long_counts.second < 0) - (0 < long_counts.second));

	if(!v[0].second.empty() && long_counts.second < 0){
            while(temp_put--){
                Tim::Options* pl = ComplexInstruments::unHand->TradeUniverse[v[0].second]->opt;
		Tim::SimpleInstruments* si_put = new Tim::SimpleInstruments();
                 si_put->simple_instrument_type = Tim::SimpleInstrumentType::SIPut;
                 si_put->opt = pl;
                 si_put->opt_entry = pl;
                 //check this
                 si_put->symbol_id = v[0].first;
                //si_put->theo_position = sign_put;
                si_put->theo_position = 1;
                 si_put->theo_fill_px = 0;
		std::cout<<"Filling Put SI\n";
                //Tim::SimpleInstruments* si_put = new Tim::SimpleInstruments(Tim::SimpleInstrumentType::SIPut, pl, pl, v[0].second, sign_put, 0);
                constituent_simple_instruments.push_back(si_put);
            }
        }
	
	std::cout<<constituent_simple_instruments.size()<<"size\n";
    }

double CI_straddle::getLongCutoff(char type){
      	//double deviation = 0.0;
        //if(type == 'c'){
        //    deviation = shock_ind->call_expected_shock;
       // }       
       // else{
        //    deviation = shock_ind->put_expected_shock;
       // }
        //double scaler = shock_scaler*(1+deviation);
        //MyLogger::log(TRADEFILE, "Shock Value: ", shock_val, 
         //   "  Scaler: ", scaler, " mean_cut_off: ", mean_shock_value);
        //return shock_val*std::exp( - scaler*(std::max(0.0, deviation - mean_shock_value)));
        return 1.0;
    }

    std::pair<int, int> CI_straddle::getLongPositionsNumber(){
        int call_number = 0;
        int put_number = 0;
        for(auto i=ComplexInstruments::tradeBook->begin(); 
            i!=ComplexInstruments::tradeBook->end(); i++){
            if(unHand->TradeUniverseIDs[i->first]->opt->str_opt_type == 'c'){
		//std::cout<<"Increasing call count\n";
                call_number += i->second.theo_pos;
            }
            else{
		//std::cout<<"Increasing Put count\n";
                put_number += i->second.theo_pos;
            }
        }
        std::pair<int, int> num_pair = std::make_pair(call_number, put_number);
        return num_pair;
    }
}
