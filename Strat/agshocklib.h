#pragma once
#include "shockrisk.h"
#include "shockmarket.h"
#include "configreader.h"
#include "symbolstore.h"
#include "util.h"
#include <agoptionslib/datastructs/option_chain.h>
#include <agoptionslib/calendars/whole_year.h>
#include <agoptionslib/pricers/black76.h>
#include <agoptionslib/pricers/black_scholes.h>

namespace alphagrep {
namespace shock {

class AgShockLib
{
    public:
    AgShockLib(): m_latestTimestamp(0) {}

    bool init(illuminati::Configfile& cfg){
        LLOG(INFO, "Initializing Shock Risk");
        double m_riskFreeRate = cfg.getDouble("RISK", "RiskFreeRate", double(0));
        std::string defaultMoves = "-0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40";
        std::vector<std::string> underlyingMovesList = illuminati::stringtokens(cfg.getString_s("RISK", "UnderlyingMovesList", defaultMoves.c_str()), ',');
        std::vector<std::string> volMovesList = illuminati::stringtokens(cfg.getString_s("RISK", "VolMovesList", defaultMoves.c_str()), ',');
        m_symbolStore.exchange(cfg.getStringStrict("RISK", "Exchange"));
        m_date = cfg.getIntStrict("RISK", "Date");
        m_shockRisk = new ShockRisk(underlyingMovesList, volMovesList);
        return initSymbols();
    };

    bool init(ShockConfig &cfg){
        LLOG(INFO, "Initializing Shock Risk");
        double m_riskFreeRate = cfg.getRiskFreeRate();
        m_symbolStore.exchange(cfg.getExchange());
        m_date = cfg.getDate();
        m_shockRisk = new ShockRisk(cfg.getUnderlyingMovesList(), cfg.getVolMovesList());
        return initSymbols();
    }

    void onMdUpdate(std::string symbol, double price, uint64_t timestamp){
        if (m_symbolMap.find(symbol) == m_symbolMap.end()){
            insertNewSymbol(symbol);
        }
        m_symbolMap.find(symbol)->second->mid(price);
        m_latestTimestamp = timestamp;
    };

    void onTradeUpdate(std::string symbol, unsigned char side, double price, int32_t quantity, uint64_t timestamp){
        if (m_symbolMap.find(symbol) == m_symbolMap.end()){
            insertNewSymbol(symbol);
        }
        int signedQty = (side == 'B')? std::abs(quantity): -1*std::abs(quantity);
        m_symbolMap.find(symbol)->second->position() += signedQty;
        m_latestTimestamp = timestamp;
    };

    const ShockRisk& getShockRisk(){ return getShockRiskInternal();};

    private:
    std::unordered_map<std::string, ShockMarket*> m_symbolMap;
    std::unordered_map<std::string, std::unique_ptr<alphagrep::infra::options::OptionChain>> m_optionChainMap;
    std::unordered_map<std::string, std::unique_ptr<infra::options::BlackScholesPricer>> m_pricerMap;
    std::unordered_map<std::string, std::string> m_symbolUnderlyingKeyMap;
    std::unordered_map<std::string, uint64_t> m_optionChainExpiryMap;
    std::unordered_map<int, std::vector<ShockMarket*> > m_underlyingOverlyingVectorMap; 
    ShockRisk* m_shockRisk;
    std::vector<double> m_origTheo;
    std::vector<double> m_origVol;
    double m_riskFreeRate;
    SymbolStore m_symbolStore;
    std::unordered_set<std::string> m_symbolUniverse;
    uint64_t m_latestTimestamp;
    int64_t m_timezoneOffset;
    int m_date;

    bool initSymbols(){
        const std::string refdataPath = Util::getCsvRefdataPath(m_symbolStore.exchange(), m_date);
        LLOG(INFO, "Using refdata path = ", refdataPath);
        alphagrep::referencedata::CsvLoader refdataLoader;
        if (!refdataLoader.load(refdataPath)){
            LLOG(FATAL, "Unable to load refdata: ", refdataPath);
            return false;
        }
        auto allSymbols = refdataLoader.getAllIllumSymbols();
        auto exch_timezone = refdataLoader.getExchangeTimezone();

        m_timezoneOffset = Util::getLocalTimeOffset(exch_timezone)*1e9;

        // Load all options, populate option chain and find underlyings
        for (auto symbol : allSymbols){
            auto  instrument = refdataLoader.getInstrumentFromIllumSymbol(symbol.c_str());
            m_symbolStore.insert(symbol, *instrument);
        }

        if (m_symbolStore.size() == 0){
            LLOG(FATAL, "No symbol registered for shock risk from refdata: ", refdataPath);
            return false;
        }
        return true;
    }

    void reloadOptionChain(std::string &chainKey, double &option_strike, std::string &symbol, infra::options::OptionType &option_type,
        std::string &opposite_option_symbol, infra::options::OptionType &opposite_option_type){
        std::unique_ptr<alphagrep::infra::options::OptionChain> oldChain = std::move(m_optionChainMap.find(chainKey)->second);
        auto newChain = std::unique_ptr<alphagrep::infra::options::OptionChain>{new infra::options::OptionChain()};
        for (auto it = oldChain->begin(); it != oldChain->end(); it++){
            newChain->addOption(it->strike, it->type, it->market);
        }
        newChain->addOption(option_strike, option_type, m_symbolMap.find(symbol)->second);
        newChain->addOption(option_strike, opposite_option_type, m_symbolMap.find(opposite_option_symbol)->second);

        // LLOG(INFO, "Reloaded chain for key: ", chainKey, " Size: ", newChain->size());
        oldChain.reset();
        m_optionChainMap.find(chainKey)->second = std::move(newChain);
        LLOG(INFO, "Check reloaded chain for key: ", chainKey, " Size: ", m_optionChainMap.find(chainKey)->second->size());
    }

    void insertNewUnderlyingSymbol(std::string &symbol){
        auto instr = m_symbolStore.get(symbol);
        std::string instrType = instr.getValue<referencedata::INSTRUMENT_TYPE>();
        m_symbolMap.emplace(symbol, new ShockMarket{symbol, instrType});
        LLOG(INFO, "InsertedNewUnderlyingSymbol:", symbol);
        auto contractMultiplier = instr.getValue<referencedata::CONTRACT_MULTIPLIER>();
        auto mdPriceMultiplier = instr.getValue<referencedata::MD_PRICE_MULTIPLIER>();
        m_symbolMap.find(symbol)->second->contractMultiplier(contractMultiplier);
        m_symbolMap.find(symbol)->second->mdPriceMultiplier(mdPriceMultiplier);
    }

    void insertNewSymbol(std::string &symbol){
        if (!m_symbolStore.has(symbol)){
            LLOG(FATAL, "Symbol: ", symbol, " was not found in refdata");
            return;
        }
        auto instr = m_symbolStore.get(symbol);
        std::string instrType = instr.getValue<referencedata::INSTRUMENT_TYPE>();
        if (instrType == "OPTION"){
            m_symbolMap.emplace(symbol, new ShockMarket{symbol, instrType});
            LLOG(INFO, "InsertedNewSymbol:", symbol);
            auto contractMultiplier = instr.getValue<referencedata::CONTRACT_MULTIPLIER>();
            auto mdPriceMultiplier = instr.getValue<referencedata::MD_PRICE_MULTIPLIER>();
            auto expiry_mnemonic = instr.getValue<referencedata::EXPIRY_MNEMONIC>();
            auto underlying_ag_id = instr.getValue<referencedata::UNDERLYING_AG_ID>();
            auto underlying_class = instr.getValue<referencedata::UNDERLYING_CLASS>();
            auto expiry_date = instr.getValue<referencedata::EXPIRY_DATE>();
            auto expiry_time = instr.getValue<referencedata::EXPIRY_TIME>();
            auto option_strike = instr.getValue<referencedata::OPTION_STRIKE>();
            auto option_type = (instr.getValue<referencedata::OPTION_TYPE>() == "PUT")? infra::options::OptionType::Put : infra::options::OptionType::Call;
            m_symbolMap.find(symbol)->second->contractMultiplier(contractMultiplier);
            m_symbolMap.find(symbol)->second->mdPriceMultiplier(mdPriceMultiplier);

            std::string chainKey = std::to_string(underlying_ag_id[0])+expiry_mnemonic;
            m_optionChainExpiryMap[chainKey] = illuminati::ITime_Convert::YYYYMMDDTHHMMSSMicros_to_ns(Util::getFormattedExpiryTime(expiry_date, expiry_time)) - m_timezoneOffset;
            std::cout << "Epoch time " << m_optionChainExpiryMap[chainKey] << std::endl;
            
            auto opposite_option_symbol = m_symbolStore.getOppositeOption(symbol);
            auto opposite_option_type = (option_type == infra::options::OptionType::Call)? infra::options::OptionType::Put : infra::options::OptionType::Call;
            if (m_symbolMap.find(opposite_option_symbol) != m_symbolMap.end()){
                LLOG(FATAL, "Opposite option was already added for symbol: ", symbol, " Opposite option: ", opposite_option_symbol);
            }

            m_symbolMap.emplace(opposite_option_symbol, new ShockMarket{opposite_option_symbol, instrType});
            LLOG(INFO, "InsertedNewSymbol:", opposite_option_symbol);
            m_symbolMap.find(opposite_option_symbol)->second->contractMultiplier(contractMultiplier);
            m_symbolMap.find(opposite_option_symbol)->second->mdPriceMultiplier(mdPriceMultiplier);

            auto underlyingSymbol = m_symbolStore.getOptionUnderlyingSymbol(underlying_ag_id[0], expiry_date, expiry_mnemonic, underlying_class);
            if (underlyingSymbol == ""){
                LLOG(FATAL, "Could not find underlying symbol for option: ", symbol, ". Tried searching for given underlying_ag_id: ", underlying_ag_id[0], " having underlying_class: ", underlying_class);
                return;
            }

            if (m_symbolMap.find(underlyingSymbol) == m_symbolMap.end()){
                insertNewUnderlyingSymbol(underlyingSymbol);
            }

            m_symbolMap.find(symbol)->second->underlying(m_symbolMap.find(underlyingSymbol)->second);
            m_symbolMap.find(opposite_option_symbol)->second->underlying(m_symbolMap.find(underlyingSymbol)->second);
            LLOG(INFO, "Set underlying: ", underlyingSymbol, " for option pair: ", symbol, ",", opposite_option_symbol);


            // First instance of option chain
            if (m_optionChainMap.find(chainKey) == m_optionChainMap.end()){
                m_optionChainMap.emplace(chainKey, std::unique_ptr<alphagrep::infra::options::OptionChain>{new infra::options::OptionChain()});
                m_optionChainMap.find(chainKey)->second->addOption(option_strike, option_type, m_symbolMap.find(symbol)->second);
                m_optionChainMap.find(chainKey)->second->addOption(option_strike, opposite_option_type, m_symbolMap.find(opposite_option_symbol)->second);
            }
            else{
                reloadOptionChain(chainKey, option_strike, symbol, option_type, opposite_option_symbol, opposite_option_type);
            }
            
            m_optionChainMap.find(chainKey)->second->init(m_optionChainExpiryMap[chainKey], std::unique_ptr<infra::options::WholeYearCalendar>(new infra::options::WholeYearCalendar{}));
            if (m_pricerMap.find(chainKey) != m_pricerMap.end()){
                m_pricerMap.find(chainKey)->second.reset();
                m_pricerMap.find(chainKey)->second = std::unique_ptr<infra::options::BlackScholesPricer>(new infra::options::BlackScholesPricer(*(m_optionChainMap.find(chainKey)->second)));
            }
            else{
                m_pricerMap.emplace(chainKey, std::unique_ptr<alphagrep::infra::options::BlackScholesPricer>{new infra::options::BlackScholesPricer(*(m_optionChainMap.find(chainKey)->second))});
            }

            LLOG(INFO, "Tracking Risk for Option: ", symbol, " Strike: ", option_strike ," Type: " , (int)option_type , " Expiry: ", m_optionChainExpiryMap[chainKey] );
            LLOG(INFO, "Tracking Risk for Option: ", opposite_option_symbol, " Strike: ", option_strike ," Type: " , (int)opposite_option_type , " Expiry: ", m_optionChainExpiryMap[chainKey] );

        }
        else{
            insertNewUnderlyingSymbol(symbol);
        }

    }

    const ShockRisk& getShockRiskInternal(){
        m_shockRisk->clearScenarios();
        for (int i = 0; i < m_shockRisk->m_underlyingMove.size(); i++) {
            for (int j = 0; j < m_shockRisk->m_volMove.size(); j++) {
                double valueDelta = 0;

                for (auto it = m_optionChainMap.begin(); it != m_optionChainMap.end(); it++){
                    valueDelta = 0;
                    auto expiry = it->first;
                    auto &optionChain = it->second;
                    auto &pricer = m_pricerMap.find(expiry)->second;
                    // Util::printChain(optionChain);
                    double underlyingPrice = static_cast<ShockMarket*>(((*optionChain)[0]).market)->underlyingPrice(); 
                    // LLOG(INFO, "Expiry = " , expiry , " UnderlyingPx = " , underlyingPrice );

                    m_origTheo.reserve(optionChain->size());
                    m_origVol.reserve(optionChain->size());
                    
                    optionChain->setTime(m_latestTimestamp);
                    pricer->setOptionPrices();
                    pricer->calculateImpliedVol(underlyingPrice, m_riskFreeRate, 1e-6);

                    for (int i = 0; i < optionChain->size(); i++) {
                        pricer->setVol(i, std::max(0.01, std::min(0.99, (double)pricer->iv(i))));
                    }

                    pricer->price(underlyingPrice, m_riskFreeRate); 

                    for (int i = 0; i < optionChain->size(); i++) {
                        m_origTheo[i] = pricer->theoPrice(i);
                        m_origVol[i] = pricer->vol(i);
                        // LLOG(INFO,"Index:", i, "|Theo:", m_origTheo[i], "|Vol:", m_origVol[i], "|Delta:", pricer->delta(i));
                    }

                    for (int i = 0; i < optionChain->size(); i++){
                        pricer->setVol(i, std::max(0.01, std::min(0.99, m_origVol[i] + m_shockRisk->m_volMove[j])));
                    }

                    pricer->price(underlyingPrice * (1 + m_shockRisk->m_underlyingMove[i]), m_riskFreeRate);

                    for (int i = 0; i < optionChain->size(); i++){
                        auto contractMultiplier = static_cast<ShockMarket*>(((*optionChain)[i]).market)->contractMultiplier();
                        double incrementPnl = static_cast<ShockMarket*>(((*optionChain)[i]).market)->position() * (double)contractMultiplier * (pricer->theoPrice(i) - m_origTheo[i]);
                        valueDelta += incrementPnl;
                    }

                    for (int i = 0; i < optionChain->size(); i++) {
                        pricer->setVol(i, m_origVol[i]);
                    }
                    m_origTheo.clear(); m_origVol.clear();
                    m_shockRisk->m_scenarios[i][j] += valueDelta;
                }
            }

            // Shock slide for non-option instruments will only have underlying move impact and no vol impact
            double nonOptionShock = 0;
            for (auto &it : m_symbolMap){
                auto &symbol = it.second;
                if (symbol->instrType() != "OPTION"){
                    nonOptionShock += symbol->position() * (double)symbol->contractMultiplier() * symbol->mid() * m_shockRisk->m_underlyingMove[i];
                }
            }
            for (int j = 0; j < m_shockRisk->m_volMove.size(); j++){
                m_shockRisk->m_scenarios[i][j] += nonOptionShock;
            }


        }
        return *m_shockRisk;
    }

};/* AgShockLib */
}/* namespace shock */
}/* namespace alphagrep */