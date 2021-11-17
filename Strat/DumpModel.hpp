#include "Models/model_handler.hpp"

namespace Tim{
class DumpModel: public ModelHandler{
public:
	//std::map<std::string, std::tuple<Tim::Options*,Tim::Options*, int,int>> ATMOptions;
	//std::tuple<Tim::Options*,Tim::Options*, int,int> farATMOptions;
	//bool IVCalendarSignal;
	//std::tuple<double, double> atmWeightedIVs;
	uint64_t timestamp;
	DumpModel(uint64_t timestamp_t,  Tim::UniverseHandler *unHand_t,
		Tim::Paramloader* params_t, std::map<int, Tim::TradeVals>* tradeBook_t, StatsGen *stats_t):
		ModelHandler(unHand_t, params_t, tradeBook_t, stats_t)
		{timestamp = timestamp_t;};
};
}
