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
#include "bscalc.h"
#include "bisection.h"
#include "agmarginlib.h"
#include "UniverseMaker/UniverseElement.hpp"
#include "UniverseMaker/GreekShocker.hpp"
namespace Baryons
{
	class ShockHandler:  public TimerListener,
			   public GreekTimerListener,
	                   public onNotifyListener
    {
	public:
    std::map<Tim::ExpiryType, std::map<std::string, Tim::Instrument*>> expiryTU;
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
	int date;
	std::string filepath = "";
    Tim::GreekShocker *GS;
    double interest_rate = 0;
    Tim::UniverseHandler *unHand;
	Tim::MarginStats *margin_stats;
    std::map<Tim::ShockParam, double> *shockMap;
	std::map<int, Tim::TradeVals>* tradeBook;
    std::vector<long double> portfolio_risk{15,0};
	std::vector<double> spot_moves{0.02, 0.05,0.1,0.15,-0.02,-0.05,-0.1,-0.15};
	std::vector<double> iv_moves{0.1,0.15,0.2,-0.1,-0.15,-0.2};
	std::vector<std::pair<double, double>> all_moves;
	std::map<std::string, Tim::SpotPricing*>* spot_obj_map;
	ShockHandler(int date, uint64_t timestamp_t, uint64_t unit_time_t, std::map<int, Tim::TradeVals> *tradeBook_t,
	Tim::UniverseHandler *unHand_t, Tim::Paramloader* params_t, 
	std::map<std::string, Tim::SpotPricing*>* spot_obj_map_t, Tim::MarginStats *margin_stats_t, std::string filepath_t,
	int priority = CallBackManager::DEFAULT_PRIORITY);
	// ~DataDumper(){};
    void onNotify() override;
    void onGreekTimerUpdate(uint64_t) override;
	void onTimerUpdate(uint64_t timestamp) override;
    void setUnitTime(uint64_t unit_time_t) override;
    uint64_t getUnitTime() override;

	void findMarginShock(uint64_t timestamp);
    void findPortfolioRisk(uint64_t ts);
	// void setheader();
    // void greekDumper(uint64_t timestamp);
	// void alldataDumper(uint64_t timestamp, bool refresh);
	// void shockDumper(uint64_t timestamp);
	// void shockDumper2(uint64_t timestamp);
	// void shockDumper3(uint64_t timestamp, std::vector<std::string> spot_m, std::vector<std::string> iv_m, std::string und, bool isup,  bool isLib, int exp_id); //, std::vector<std::string> spot_m, std::vector<std::string iv_m, std::string und, bool isup, bool isLib, int exp_id);
	// void shockDumper4(uint64_t timestamp);
	// void setTimeStampRanges();
	// double findBSOptPrice(Tim::Options* option, double spot_px, double iv_move);
	// std::tuple<std::vector<std::pair<std::string, double>>, 
    //     std::vector<std::pair<std::string, double>>, 
    //     std::vector<std::pair<std::string, double>>, 
    //     std::vector<std::pair<std::string, double>>> rankStrikesbySpotDiff(Tim::ExpiryType exp);
	// std::string createColumnNames(Tim::ExpiryType et);
	// std::string createDefaultColumnNames(Tim::ExpiryType et, int strike_rng);
	// void optionStockLeadLag(uint64_t timestamp);
	// std::tuple<Tim::Options*,Tim::Options*, int,int> findATMOption();
	// std::string getATMRow(std::pair<Tim::ExpiryType, std::map<std::string, Tim::Instrument*>> TU);
	// std::tuple<Tim::Options*,Tim::Options*, int,int> findATMOption(std::map<std::string, Tim::Instrument*> TU);
	// void writeData(Tim::Options* opt, uint64_t timestamp, double atmWeightedIV);
	// std::tuple<double, double> findATMWeightedInfo(Tim::Options* ATMCall, Tim::Options* ATMPut, Tim::ExpiryType exp);
	// std::tuple<Tim::Options*, Tim::Options*, Tim::Options*, Tim::Options*, double , double > atmHighLowIV(std::map<std::string, Tim::Instrument*> T_U, double underlyingSpot);

	// void helloWorld();
	};
} 
