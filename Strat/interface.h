#pragma once
#include "shockrisk.h"
#include "configreader.h"

namespace alphagrep {
namespace shock {

class AgShockLib
{
public:
  AgShockLib() = default;
  ~AgShockLib();

  bool init(illuminati::Configfile& config);
  bool init(ShockConfig &cfg);
  void onMdUpdate(std::string symbol, double price, uint64_t timestamp);
  void onTradeUpdate(std::string symbol, double price, int32_t quantity, uint64_t timestamp);

  ShockRisk getShockRisk();

};/* AgShockLib */
}/* namespace shock */
}/* namespace alphagrep */