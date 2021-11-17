#pragma once
#include "referencedata/csvLoader.h"

namespace alphagrep {
namespace shock {
class SymbolStore
{
    public:
    SymbolStore(){}

    void exchange(const std::string &exchange){
        m_exchange = exchange;
        std::transform(m_exchange.begin(), m_exchange.end(),m_exchange.begin(), ::toupper);
    }

    std::string exchange(){
        return m_exchange;
    }

    void insert(const std::string &symbol, const alphagrep::referencedata::Instrument &instr){
        m_symbolInstrMap[symbol] = instr;
        if (instr.getValue<referencedata::INSTRUMENT_TYPE>() == "FUTURE"){
            auto ag_id = instr.getValue<referencedata::AG_ID>();
            auto underlying_ag_id = instr.getValue<referencedata::UNDERLYING_AG_ID>();
            auto expiry_date = instr.getValue<referencedata::EXPIRY_DATE>();
            m_futureChainMap[underlying_ag_id[0]].push_back(std::make_pair(expiry_date, ag_id));
        }
    }

    bool has(const std::string &symbol){
        return (m_symbolInstrMap.find(symbol) == m_symbolInstrMap.end())? false : true;
    }

    alphagrep::referencedata::Instrument get(const std::string &symbol){
        return m_symbolInstrMap[symbol];
    }

    std::string getOppositeOption(const std::string &symbol){
        auto ourOption = m_symbolInstrMap[symbol];
        std::string oppositeOption;
        if (ourOption.getValue<referencedata::INSTRUMENT_TYPE>() != "OPTION"){
            LLOG(FATAL, "Tried to fetch opposite option for ", symbol);
            return oppositeOption;
        }
        auto expiry_mnemonic = ourOption.getValue<referencedata::EXPIRY_MNEMONIC>();
        auto underlying_ag_id = ourOption.getValue<referencedata::UNDERLYING_AG_ID>();
        auto option_strike = ourOption.getValue<referencedata::OPTION_STRIKE>();
        auto opposite_option_type = (ourOption.getValue<referencedata::OPTION_TYPE>() == "PUT")? "CALL" : "PUT";

        for(auto it: m_symbolInstrMap){
            auto newInstr = it.second;
            if (newInstr.getValue<referencedata::INSTRUMENT_TYPE>() == "OPTION" &&
                newInstr.getValue<referencedata::UNDERLYING_AG_ID>() == underlying_ag_id &&
                newInstr.getValue<referencedata::OPTION_STRIKE>() == option_strike &&
                newInstr.getValue<referencedata::EXPIRY_MNEMONIC>() == expiry_mnemonic &&
                newInstr.getValue<referencedata::OPTION_TYPE>() == opposite_option_type){
                    return it.first;
            }
            
        }
        LLOG(FATAL, "No opposite type option found for ", symbol);
        return oppositeOption;
    }

    std::string getIlluminatiSymbolFromAgId(uint64_t& ag_id){
        for(auto it: m_symbolInstrMap){
            auto newInstr = it.second;
            if (newInstr.getValue<referencedata::AG_ID>() == ag_id){
                return it.first;
            }
        }
        LLOG(FATAL, "No symbol found with ag_id=", ag_id);
        return "";
    }

    struct CompareExpiryDate
    {
        template< typename T >
        bool operator()( const uint64_t &exp1, const T &expPair2 ) const
        {
            // Equality added to catch correct expiry if match exists in upper_bound
            return exp1 <= expPair2.first;
        }

        template< typename T >
        bool operator()( const T &expPair1, const T &expPair2 ) const
        {
            return expPair1.first < expPair2.first;
        }
    };

    std::string getOptionUnderlyingSymbol(uint64_t &underlying_ag_id, int &expiry_date, std::string &expiry_mnemonic, std::string &underlying_class){
        
        if (underlying_class != "INDEX"){
            return getIlluminatiSymbolFromAgId(underlying_ag_id);
        }
        else{
            if (m_exchange == "NSE" || m_exchange == "CME"){
                // underlying_ag_id gives the correct underlying ticker
                return getIlluminatiSymbolFromAgId(underlying_ag_id);

            }
            else{
                // underlying_ag_id would give spot ticker which is invalid. Pick nearest expiry here.
                if (m_futureChainMap.find(underlying_ag_id) == m_futureChainMap.end()){
                    LLOG(FATAL, "Refdata has no index futures having underlying id = ", underlying_ag_id);
                    return "";
                }
                auto &futureChain = m_futureChainMap.find(underlying_ag_id)->second;
                std::sort(futureChain.begin(), futureChain.end(), CompareExpiryDate());
                auto nearestFuture = std::upper_bound (futureChain.begin(), futureChain.end(), expiry_date, CompareExpiryDate());
                if (nearestFuture == futureChain.end()){
                    LLOG(FATAL, "For underlying_ag_id=", underlying_ag_id, " no futures exist having expiry beyond ", expiry_date);
                    return "";
                }
                auto future_ag_id = nearestFuture->second;
                return getIlluminatiSymbolFromAgId(future_ag_id);
            }
        }

        LLOG(FATAL, "No underlying found for underlying_ag_id=", underlying_ag_id, "|expiry_date=", expiry_date, "|underlying_class=", underlying_class);
        return "";
    }

    void clear(){
        m_symbolInstrMap.clear();
    }

    uint64_t size(){
        return m_symbolInstrMap.size();
    }

    private:
    std::string m_exchange;
    std::unordered_map<std::string, alphagrep::referencedata::Instrument> m_symbolInstrMap;
    std::unordered_map<uint64_t , std::vector<std::pair<int, uint64_t> > > m_futureChainMap;
};
}
}