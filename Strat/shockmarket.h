#pragma once
#include <agoptionslib/markets/market.h>

namespace alphagrep{
namespace shock{

class ShockMarket : public infra::options::Market{
    public:
    ShockMarket(const std::string &symbol, const std::string &instrType): 
        m_valid(false), m_symbol(symbol), m_instrType(instrType),
        m_mid(0), m_position(0), m_contractMultiplier(1), m_mdPriceMultiplier(1){}

    bool valid() const {return m_valid;}
    double mid() const {return m_mid/m_mdPriceMultiplier;}
    double spread() const {return 0.0;}
    int position() const {return m_position;}
    int& position() {return m_position;}
    int contractMultiplier() const {return m_contractMultiplier;}
    int mdPriceMultiplier() const {return m_mdPriceMultiplier;}
    std::string symbol() const {return m_symbol;}
    std::string instrType() const {return m_instrType;}
    ShockMarket* underlying() const {return m_underlying;}
    double underlyingPrice() const {return (m_underlying == nullptr)? 0 : m_underlying->mid();}
    std::string toString() const {return "Symbol:"+m_symbol+"|Mid:"+std::to_string(mid())+"|Pos:"+std::to_string(position())+"|ContMult:"+std::to_string(contractMultiplier())+"|MdMult:"+std::to_string(mdPriceMultiplier())+"|UnderlyingPx:"+std::to_string(underlyingPrice());}

    void valid(const bool &valid) {m_valid = valid;}
    void mid(const double &midPx) {valid(true); m_mid = midPx;}
    void position(const int &position) {m_position = position;}
    void contractMultiplier(const double &contractMultiplier) {m_contractMultiplier = contractMultiplier;}
    void mdPriceMultiplier(const double &mdPriceMultiplier) {m_mdPriceMultiplier = mdPriceMultiplier;}
    void symbol(const std::string &symbol) {m_symbol = symbol;}
    void underlying(ShockMarket* underlying) {m_underlying = underlying;}



    private:
    bool m_valid;
    double m_mid;
    int m_position;
    double m_contractMultiplier;
    double m_mdPriceMultiplier;
    std::string m_symbol, m_instrType;
    ShockMarket* m_underlying = nullptr;
};

}
}