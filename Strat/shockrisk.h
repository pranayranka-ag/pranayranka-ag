#pragma once
#include <vector>
namespace alphagrep {
namespace shock{

class ShockConfig{
    public:

    ShockConfig(const int &date, const std::string &exchange): m_date(date), m_exchange(exchange), m_riskFreeRate(0) {
        std::string defaultMoves = "-0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40";
        std::vector<std::string> underlyingMovesList = illuminati::stringtokens(defaultMoves, ',');
        std::vector<std::string> volMovesList = illuminati::stringtokens(defaultMoves, ',');
    }

    void setUnderlyingMovesList(std::string underlyingMovesStr){
        m_underlyingMovesList = illuminati::stringtokens(underlyingMovesStr, ',');
    }

    void setVolMovesList(std::string volMovesStr){
        m_volMovesList = illuminati::stringtokens(volMovesStr, ',');
    }

    void setRiskFreeRate(const double riskFreeRate) {m_riskFreeRate = riskFreeRate;}

    std::vector<std::string> getUnderlyingMovesList() const {return m_underlyingMovesList;}
    std::vector<std::string> getVolMovesList() const {return m_volMovesList;}
    double getRiskFreeRate() const {return m_riskFreeRate;}
    int getDate() const {return m_date;}
    std::string getExchange() const {return m_exchange;}


    private:
    std::string m_exchange;
    double m_riskFreeRate;
    std::vector<std::string> m_underlyingMovesList, m_volMovesList;
    int m_date;
};

class ShockRisk {
    public:
    std::vector<double> m_underlyingMove, m_volMove;
    std::unordered_map<int, int> m_underlyingMoveIndex, m_volMoveIndex;

    std::vector<std::vector<double>> m_scenarios;

    ShockRisk(const std::vector<std::string> &underlyingMovesList, const std::vector<std::string> &volMovesList){
        std::string uMoveLog, vMoveLog;
        for (auto uMove : underlyingMovesList){
            m_underlyingMove.push_back(std::stod(uMove));
            uMoveLog += uMove + ",";
        }
        for (auto vMove : volMovesList){
            m_volMove.push_back(std::stod(vMove));
            vMoveLog += vMove + ",";
        }
        LLOG(INFO, "Underlying Moves = ", uMoveLog);
        LLOG(INFO, "Vol Moves = ", vMoveLog);

        m_scenarios.resize(underlyingMovesList.size());
        for (int i = 0; i < m_scenarios.size(); i++){
            m_scenarios[i].resize(volMovesList.size());
        }

        for (int i = 0; i< m_underlyingMove.size(); i++)
        {
            m_underlyingMoveIndex[int(m_underlyingMove[i] * 100)] = i;
        }

        for (int i = 0; i< m_volMove.size(); i++)
        {
            m_volMoveIndex[int(m_volMove[i] * 100)] = i;
        }
    }

    void clearScenarios(){
        for (auto& volMoves : m_scenarios) {
            std::fill(volMoves.begin(), volMoves.end(), 0);
        }
    }

    void printScenarios(){
        std::stringstream ss; 
        ss << "\n **SLIDE** ";
        for (int j = 0; j < m_volMove.size(); j++){
            ss << std::to_string(m_volMove[j]) << " ** ";
        }
        ss << "\n";

        for (int i = 0; i < m_scenarios.size(); i++){
            ss << std::to_string(m_underlyingMove[i]) << " ** ";
            for (int j = 0; j < m_scenarios[i].size(); j++){
                ss <<  std::to_string(m_scenarios[i][j]) << "    ";
            }
            ss << "\n";
        }
        LLOG(INFO, ss.str());
    }

    double getScenarioPnl(const double &underlyingMove, const double &volMove){
        int underlyingMovePercent = int(underlyingMove * 100);
        int volMovePercent = int(volMove * 100);
        if (m_underlyingMoveIndex.find(underlyingMovePercent) == m_underlyingMoveIndex.end() ||
            m_volMoveIndex.find(volMovePercent) == m_volMoveIndex.end()){
            LLOG(FATAL, "No scenario exists with underlyingMove=", underlyingMove, " volMove=", volMove);
            return 0;
        }
        return m_scenarios[m_underlyingMoveIndex[underlyingMovePercent]][m_volMoveIndex[volMovePercent]];
    }
  };
}
}

