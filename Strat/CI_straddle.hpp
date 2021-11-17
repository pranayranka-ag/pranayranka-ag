#pragma once

#include "ComplexInstruments.hpp"
#include "Stats/StatsGen.hpp"
#include "Indicators/ExpectedShock.hpp"

namespace Tim{

class CI_straddle: public ComplexInstruments{
public:
	double delta_lower_cap;
	double delta_upper_cap;
    	int no_of_lots;
    	bool take_only_long_pos;
    	Tim::ComplexInstrumentType ci_type;

    	// Check if these are needed currently
    	double shock_val;
    	double mean_shock_value;
    	double shock_scaler;
    	double curr_port_delta;
    	double spot_mid;
    	double call_iv;
    	double put_iv;
    	double call_expected_shock;
    	double put_expected_shock;

	Tim::StatsGen* stats;
    	Tim::ExpectedShock* shock_ind;
    	Tim::SpotPricing*  spot_obj;

	CI_straddle(Tim::ComplexInstrumentType complex_instrument_type,
        Tim::UniverseHandler* unHand_t, 
        std::map<int, Tim::TradeVals>* tradeBook_t,
        Tim::Paramloader* params_t,
        Tim::StatsGen* stats_t,
       // Tim::ExpectedShock* shock_ind_t,
         Tim::SpotPricing*  spot_obj_t,
        bool take_only_long_pos);

	void fetchParams() override;
	void makeComplexInstrument() override;
	void make_complex_instrument();
	std::tuple<Tim::Options*,Tim::Options*, int,int> findATMOption();
	double getLongCutoff(char type);
	std::pair<int, int> getLongPositionsNumber();
	~CI_straddle(){ std::cout<<"Calling CI Straddle deconstructor";}

};
}

	
