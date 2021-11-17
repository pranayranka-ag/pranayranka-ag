#pragma once

#include <algorithm>
#include <fstream>
#include "Helpers/my_logger.hpp"
#include "UniverseMaker/UniverseHandler.hpp"
#include "CallBackManager/CallBacks.h"
#include "CallBackManager/TimerListener.hpp"
#include "CallBackManager/GreekTimerListener.hpp"
#include "Trade/TradeLogic.hpp"
#include "Params/Paramsbase.hpp"
#include "UniverseMaker/GreekCalculator.h"
#include "Models/model_handler.hpp"
#include "agshocklib.h"
#include "bscalc.h"
#include "bisection.h"
#include "UniverseMaker/UniverseElement.hpp"

namespace Baryons
{
	class DataDumper:  public TimerListener,
			   public GreekTimerListener,
	                   public onNotifyListener
    {
	public:
	std::vector<std::vector<std::string>> csv_data;
	std::vector<std::string> columns_in_csv{"date_time","date","time","symbol"};
	std::vector<std::string> def_cols;//{ "symbol","Option_type","strike_price","ask_px", "bid_px", "trade_px", "gamma", "theta", "delta", "intrinsic_value", "vega", "oi","spot_px","actual_iv","atm_weighted_iv","volatility", "expiry_date","atm_weighted_vega","atm_iv*atm_vega"};
	std::map<Tim::ExpiryType, std::map<std::string, Tim::Instrument*>> expiryTU;
	std::vector<std::vector<uint64_t>> timestamp_ranges;
	std::vector<Tim::TimeRange> time_ranges;
	bool count_atm_separate;
	uint64_t model_starttime=0; 
	uint64_t trade_starttime;
    uint64_t trade_endtime;
	//std::ofstream output;
	uint64_t unit_time;   
    uint64_t timestamp;     
	bool _on_timer_update = false;
    bool _on_greek_timer_update = false;
	Tim::Paramloader *params;
	Tim::GlobalParams *global_params;
	Tim::DumpParams *dump_params;
	int date;
    Tim::UniverseHandler *unHand;
	Tim::ModelHandler *modelHandler;
	double delta_lower_cap;
	double delta_upper_cap;
	std::vector<std::vector<double>> oi_data;
	std::vector<std::string> spot_moves{"0.02","0.05", "0.10", "0.15"};
	std::vector<std::string> spot_moves2{"-0.02","-0.05", "-0.10", "-0.15"};
	std::vector<std::string> iv_moves{"0.02","0.05"};
	std::vector<std::string> iv_moves2{"-0.02","-0.05"};
	alphagrep::shock::AgShockLib agShock;
	int d = 20210805;
	std::string exc = "NSE";
	std::map<std::string, alphagrep::shock::AgShockLib> agShockMap;
	std::vector<alphagrep::shock::AgShockLib> agvec;
	std::string uMoves = "0,0.02,0.05,0.1,0.15,-0.02,-0.05,-0.1,-0.15", vMoves = "0,0.02,0.05,-0.02,-0.05";
	std::map<std::string, std::string> dumpMap;
    std::map<std::string, Tim::SpotPricing*>* spot_obj_map;
    DataDumper(int date, uint64_t timestamp_t, uint64_t unit_time_t, 
		Tim::UniverseHandler *unHand_t, Tim::Paramloader* params_t, Tim::DumpParams * dump_t,
		std::map<std::string, Tim::SpotPricing*>* spot_obj_map_t,double delta_lower_cap, 
		double delta_upper_cap, Tim::ModelHandler *modelHandler_t, 
		int priority = CallBackManager::DEFAULT_PRIORITY);
	DataDumper(int date, uint64_t timestamp_t, uint64_t unit_time_t, 
		Tim::UniverseHandler *unHand_t, Tim::Paramloader* params_t, Tim::DumpParams * dump_t,
		std::map<std::string, Tim::SpotPricing*>* spot_obj_map_t,double delta_lower_cap, 
		double delta_upper_cap, Tim::ModelHandler *modelHandler_t, std::vector<std::vector<std::string>> pos_csv,
		int priority = CallBackManager::DEFAULT_PRIORITY);
	// ~DataDumper(){};
	void setheader();
    void greekDumper(uint64_t timestamp);
	void alldataDumper(uint64_t timestamp, bool refresh);
	void smileDataDumper(uint64_t timestamp);
	void shockDumper(uint64_t timestamp);
	void shockDumper2(uint64_t timestamp);
	void shockDumper3(uint64_t timestamp, std::vector<std::string> spot_m, std::vector<std::string> iv_m, std::string und, bool isup,  bool isLib, int exp_id); //, std::vector<std::string> spot_m, std::vector<std::string iv_m, std::string und, bool isup, bool isLib, int exp_id);
	void shockDumper4(uint64_t timestamp);
	void setTimeStampRanges();
	double findBSOptPrice(Tim::Options* option, double spot_px, double iv_move);
	std::tuple<std::vector<std::pair<std::string, double>>, 
        std::vector<std::pair<std::string, double>>, 
        std::vector<std::pair<std::string, double>>, 
        std::vector<std::pair<std::string, double>>> rankStrikesbySpotDiff(Tim::ExpiryType exp);
	std::string createColumnNames(Tim::ExpiryType et);
	std::string createDefaultColumnNames(Tim::ExpiryType et, int strike_rng);
	void optionStockLeadLag(uint64_t timestamp);
	std::tuple<Tim::Options*,Tim::Options*, int,int> findATMOption();
	std::string getATMRow(std::pair<Tim::ExpiryType, std::map<std::string, Tim::Instrument*>> TU);
	std::tuple<Tim::Options*,Tim::Options*, int,int> findATMOption(std::map<std::string, Tim::Instrument*> TU);
	void writeData(Tim::Options* opt, uint64_t timestamp, double atmWeightedIV);
	std::tuple<double, double> findATMWeightedInfo(Tim::Options* ATMCall, Tim::Options* ATMPut, Tim::ExpiryType exp);
	std::tuple<Tim::Options*, Tim::Options*, Tim::Options*, Tim::Options*, double , double > atmHighLowIV(std::map<std::string, Tim::Instrument*> T_U, double underlyingSpot);
	void onNotify() override;
    void onGreekTimerUpdate(uint64_t) override;
	void onTimerUpdate(uint64_t timestamp) override;
    void setUnitTime(uint64_t unit_time_t) override;
    uint64_t getUnitTime() override;
	void helloWorld();
	};
} 
