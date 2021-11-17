#pragma once
#include "shockmarket.h"
#include "fileutils.h"
#include "aglogger.h"
#include <agoptionslib/datastructs/option_chain.h>

namespace alphagrep {
namespace shock {

class Util
{
    public:
    static std::string getCsvRefdataPath(std::string exchange, int date){
        std::transform(exchange.begin(), exchange.end(), exchange.begin(), ::tolower);
        std::string file = "/data/reference_data/"+exchange+"/csv/"+exchange+"." + std::to_string(date) + ".csv";
        if (!illuminati::fileutils::fileExists(file)){
            file = "/var/dumps/reference_data/"+exchange+"/csv/"+exchange+"."+ std::to_string(date) +".csv";
        }
        return file;
    }

    static std::string getFormattedExpiryTime(const int& expiry_date, const std::string &expiry_time){
        std::string expiry_date_str = std::to_string(expiry_date);
        std::string new_expiry = expiry_date_str.substr(0,4) + "-" + expiry_date_str.substr(4,2) + "-" + expiry_date_str.substr(6,2);
        new_expiry += "T" + expiry_time + "000000";
        std::cout << "Converted " <<  expiry_date << " " << expiry_time << " " << new_expiry << std::endl;
        return new_expiry;
    }

    static void printChain(std::unique_ptr<alphagrep::infra::options::OptionChain> &optionChain){
        // LLOG(INFO, "Size=", optionChain->size());
        for (auto it = optionChain->begin(); it != optionChain->end(); it++){
            std::string marketStr = static_cast<ShockMarket*>(it->market)->toString(); 
            std::string underlyingStr = static_cast<ShockMarket*>(static_cast<ShockMarket*>(it->market)->underlying())->toString();
            // LLOG(INFO, "Strike:", it->strike, "|Type:", (int)(it->type), "|", marketStr, "|Underlying:", underlyingStr);
        }
    }

    static int64_t getLocalTimeOffset(const std::string &timezone){
        setenv("TZ", ("/usr/share/zoneinfo/"+timezone).c_str(), 1);

        time_t t, t_gmt, t_local;
        struct tm *tmGmt;
        struct tm *tmLocal;

        time (&t);
        tmGmt = std::gmtime(&t);
        t_gmt = mktime(tmGmt);

        tmLocal = std::localtime(&t);
        t_local = mktime(tmLocal);

        // LLOG(INFO, "Using Timezone = ", timezone, " Timestamp offset = ", t_local - t_gmt);
        return t_local - t_gmt;
    }
};
}
}