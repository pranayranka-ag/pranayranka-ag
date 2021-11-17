#include "CI_calendar.hpp"

namespace Tim{
        CI_calendar::CI_calendar(Tim::ComplexInstrumentType 
        complex_instrument_type,
        Tim::UniverseHandler* unHand_t, 
        std::map<int, Tim::TradeVals>* tradeBook_t,
        Tim::Paramloader* params_t,
        Tim::StatsGen* stats_t,
	//Tim::MarginStats* margin_calc_t,
        Tim::SpotPricing* spot_obj_t,
        bool take_only_long_pos_t, std::tuple<Tim::Options*,Tim::Options*,int, int> longATMOption_t, 
	std::tuple<Tim::Options*,Tim::Options*,int, int> shortATMOption_t):
        stats(stats_t),
 	longATMOption(longATMOption_t),
	shortATMOption(shortATMOption_t),
        spot_obj(spot_obj_t),
//	margin_calc(margin_calc_t),
        take_only_long_pos(take_only_long_pos_t),
        ComplexInstruments(complex_instrument_type, 
            unHand_t, params_t, tradeBook_t,1){
    
        //WHat is this for?
        ci_type = Tim::ComplexInstrumentType::IVLong;
        fetchParams(); 
        makeComplexInstrument();
    }   

        void CI_calendar::fetchParams(){

        if(ComplexInstruments::params->params_doc.HasMember("CICalendarParams")){
            const Baryons::rapidjson::Value& paramDoc =
            ComplexInstruments::params->params_doc["CICalendarParams"];
            assert(paramDoc.IsObject());


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

        spot_mid = spot_obj->getSpotStruct()->ask_spot_price;
    }


void CI_calendar::makeComplexInstrument(){


        Tim::SimpleInstruments* si_pn = SimpleInstruments::allocate();
        Tim::SimpleInstruments* si_cn = SimpleInstruments::allocate();
	Tim::SimpleInstruments* si_pf = SimpleInstruments::allocate();
        Tim::SimpleInstruments* si_cf = SimpleInstruments::allocate();
        int theo_pos = 0;
       //double price = 0;
        //std::tuple<Tim::Options*,Tim::Options*,int, int> nearATMOptions = findATMOption(Tim::NearWeekly);
	//std::tuple<Tim::Options*,Tim::Options*,int, int> farATMOptions = findATMOption(Tim::FarWeekly);
        Tim::Options* nearATMCall = std::get<0>(longATMOption);
        int nearAtmCallId = std::get<2>(longATMOption);
        Tim::Options* nearATMPut = std::get<1>(longATMOption);
        int nearAtmPutId = std::get<3>(longATMOption);

	MyLogger::log(TRADEFILE, "\n----------\n","IVCalendar Model Long Instrument construction CI_Calendar", "CI Calendar::ATM Call ID: ",
 nearATMCall->symbol_id, "ATM Call Symbol ", nearATMCall->symbol, "  ATM Call Delta ", nearATMCall->delta, "  ATM Call Strike ", nearATMCall->strike, "Spot ", nearATMCall->underlying_price, "\n-------\n");
	MyLogger::log(TRADEFILE, "\n----------\n","IVCalendar Model Long Instrument construction CI_Calendar ", "CI Calendar::ATM Put ID: ",
 nearATMPut->symbol_id, "ATM Put Symbol ", nearATMPut->symbol, "  ATM Put Delta ", nearATMPut->delta, "  ATM Put Strike ", nearATMPut->strike, "Spot ", nearATMPut->underlying_price, "\n-------\n");

        theo_pos = 1;
       
        si_pn->initialize(Tim::SimpleInstrumentType::SIPut,
          nearATMPut, nearATMPut, nearAtmPutId, theo_pos, nearATMPut->bid_px);
        si_pn->simple_instrument_status = Tim::SimpleInstrumentStatus::SIPendingEntry;
        constituent_simple_instruments.push_back(si_pn);
         

        si_cn->initialize(Tim::SimpleInstrumentType::SICall,
           nearATMCall, nearATMCall, nearAtmCallId, theo_pos, nearATMCall->bid_px);
                  
        si_cn->simple_instrument_status = Tim::SimpleInstrumentStatus::SIPendingEntry;
                         
        constituent_simple_instruments.push_back(si_cn);

	Tim::Options* farATMCall = std::get<0>(shortATMOption);
        int farAtmCallId = std::get<2>(shortATMOption);
        Tim::Options* farATMPut = std::get<1>(shortATMOption);
        int farAtmPutId = std::get<3>(shortATMOption);

	MyLogger::log(TRADEFILE, "\n----------\n","IVCalendar Model Short Instrument construction CI_Calendar ", "CI Calendar::ATM Call ID: ",
 farATMCall->symbol_id, "ATM Call Symbol ", farATMCall->symbol, "  ATM Call Delta ", farATMCall->delta, "  ATM Call Strike ", farATMCall->strike, "Spot ", farATMCall->underlying_price, "\n-------\n");
        MyLogger::log(TRADEFILE, "\n----------\n","IVCalendar Model Short Instrument construction CI_Calendar", "CI Calendar::ATM Put ID: ",
  farATMPut->symbol_id, "ATM Put Symbol ", farATMPut->symbol, "  ATM Put Delta ", farATMPut->delta, "  ATM Put Strike ", farATMPut->strike, "Spot ", farATMPut->underlying_price, "\n-------\n");
 
        theo_pos = -1;

        si_pf->initialize(Tim::SimpleInstrumentType::SIPut,
           farATMPut, farATMPut, farAtmPutId, theo_pos, farATMPut->ask_px);
        si_pf->simple_instrument_status = Tim::SimpleInstrumentStatus::SIPendingEntry;
        constituent_simple_instruments.push_back(si_pf);
 
        si_cf->initialize(Tim::SimpleInstrumentType::SICall,
            farATMCall, farATMCall, farAtmCallId, theo_pos, farATMCall->ask_px);
 
        si_cf->simple_instrument_status = Tim::SimpleInstrumentStatus::SIPendingEntry;
 
        constituent_simple_instruments.push_back(si_cf);
}
}
