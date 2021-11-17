#include "DataDumper.hpp"
//#include "agshocklib.h"
#include "GreekCalculator.h"
#include <chrono>


namespace Baryons{
        DataDumper::DataDumper(int date_t, uint64_t timestamp_t, uint64_t unit_time_t, 
                Tim::UniverseHandler *unHand_t, Tim::Paramloader* params_t,Tim::DumpParams * dump_t,
                std::map<std::string, Tim::SpotPricing*>* spot_obj_map_t, 
                double delta_lower_cap_t, double delta_upper_cap_t, 
                Tim::ModelHandler *modelHandler_t, int priority):
        spot_obj_map(spot_obj_map_t),
        unHand(unHand_t),
        timestamp(timestamp_t),
        params(params_t),
        delta_lower_cap(delta_lower_cap_t),
        delta_upper_cap(delta_upper_cap_t),
        _on_timer_update(false),
        _on_greek_timer_update(false),
        modelHandler(modelHandler_t)
{
        // count_atm_separate = dump_params->count_atm_separate;
        model_starttime = timestamp;
        date = date_t;
        global_params = params->global_params;
        dump_params = dump_t;
        // time_ranges = dump_params->time_ranges;
        // def_cols = dump_params->def_columns;
        trade_starttime = Tim::Helpers::epochTimeInNanos(date, global_params->start_hour, global_params->start_min, 0, 0);
        trade_endtime = Tim::Helpers::epochTimeInNanos(date, global_params->end_hour, global_params->end_min, 0, 0);
        setUnitTime(unit_time_t);
        setTimeStampRanges();
        std::map<std::string, Tim::Instrument*> TU0, TU1, TU2, TU3;
        for (auto itr = unHand->TradeUniverse.begin(); itr != unHand->TradeUniverse.end(); itr++)
        {
                std::cout << itr->first << std::endl;
                if (itr->second->isOption && itr->second->expiry_name == Tim::ExpiryType::Weekly){
                        TU0[itr->first] = itr->second;
                }
                else if (itr->second->isOption && itr->second->expiry_name == Tim::ExpiryType::Fortnightly){
                        TU1[itr->first] = itr->second;
                }
                else if (itr->second->isOption && itr->second->expiry_name == Tim::ExpiryType::Premonthly){
                        TU2[itr->first] = itr->second;
                }
                else if (itr->second->isOption && itr->second->expiry_name == Tim::ExpiryType::Monthly){
                        TU3[itr->first] = itr->second;
                }
                std::cout << itr->first;
        }
        if (!TU0.empty())
                expiryTU[Tim::ExpiryType::Weekly] = TU0;
        if (!TU1.empty())
                expiryTU[Tim::ExpiryType::Fortnightly] = TU1;
        if (!TU2.empty())
                expiryTU[Tim::ExpiryType::Premonthly] = TU2;
        if (!TU3.empty())
                expiryTU[Tim::ExpiryType::Monthly] = TU3;
        
  
  }
        DataDumper::DataDumper(int date_t, uint64_t timestamp_t, uint64_t unit_time_t, 
                Tim::UniverseHandler *unHand_t, Tim::Paramloader* params_t,Tim::DumpParams * dump_t,
                std::map<std::string, Tim::SpotPricing*>* spot_obj_map_t, 
                double delta_lower_cap_t, double delta_upper_cap_t, 
                Tim::ModelHandler *modelHandler_t, std::vector<std::vector<std::string>> pos_csv, int priority):
        csv_data(pos_csv),
        spot_obj_map(spot_obj_map_t),
        unHand(unHand_t),
        timestamp(timestamp_t),
        params(params_t),
        delta_lower_cap(delta_lower_cap_t),
        delta_upper_cap(delta_upper_cap_t),
        _on_timer_update(false),
        _on_greek_timer_update(false),
        modelHandler(modelHandler_t)
{
        // count_atm_separate = dump_params->count_atm_separate;
        model_starttime = timestamp;
        date = date_t;
        global_params = params->global_params;
        dump_params = dump_t;
        // time_ranges = dump_params->time_ranges;
        // def_cols = dump_params->def_columns;
        trade_starttime = Tim::Helpers::epochTimeInNanos(date, global_params->start_hour, global_params->start_min, 0, 0);
        trade_endtime = Tim::Helpers::epochTimeInNanos(date, global_params->end_hour, global_params->end_min, 0, 0);
        setUnitTime(unit_time_t);
        setTimeStampRanges();
        std::map<std::string, Tim::Instrument*> TU0, TU1, TU2, TU3;
        // alphagrep::infra::logger::Logger<>::Init("/spare/local/pranka/output");

        alphagrep::shock::ShockConfig cfg2(d, exc);
        cfg2.setUnderlyingMovesList(uMoves);
        cfg2.setVolMovesList(vMoves);


        
        cfg2.setRiskFreeRate(0);

        for (auto itr = unHand->TradeUniverse.begin(); itr != unHand->TradeUniverse.end(); itr++)
        {
                std::cout << itr->first << std::endl;
                if (itr->second->isOption && itr->second->expiry_name == Tim::ExpiryType::Weekly){
                        TU0[itr->first] = itr->second;
                }
                else if (itr->second->isOption && itr->second->expiry_name == Tim::ExpiryType::Fortnightly){
                        TU1[itr->first] = itr->second;
                }
                else if (itr->second->isOption && itr->second->expiry_name == Tim::ExpiryType::Premonthly){
                        TU2[itr->first] = itr->second;
                }
                else if (itr->second->isOption && itr->second->expiry_name == Tim::ExpiryType::Monthly){
                        TU3[itr->first] = itr->second;
                }
                // auto &ags =  agShockMap[itr->first];
                // ags.init(cfg2);
                
        }

        if (!TU0.empty())
                expiryTU[Tim::ExpiryType::Weekly] = TU0;
        if (!TU1.empty())
                expiryTU[Tim::ExpiryType::Fortnightly] = TU1;
        if (!TU2.empty())
                expiryTU[Tim::ExpiryType::Premonthly] = TU2;
        if (!TU3.empty())
                expiryTU[Tim::ExpiryType::Monthly] = TU3;
        
  
  }

void DataDumper::shockDumper3(uint64_t timestamp, std::vector<std::string> spot_m, std::vector<std::string> iv_m, std::string und, bool isup, bool isLib, int exp_id){
        std::cout << "Timestamp: " << Tim::Helpers::getHumanReadableTime(timestamp) << std::endl;
        static int logger_i[4] = {0,0,0,0};
        if ((*Tim::Helpers::timestamp_ptr/(1000*1000*1000LL))%1 != 0) {
             return;
        }
        std::string fname = "";
        if(isup){
                if(isLib){
                        fname = "/home/pranka/work/Strat_Init/Strat/temp/data_lib_" + und + "_up_0508_.csv";
                }
                else{
                        fname = "/home/pranka/work/Strat_Init/Strat/temp/data_" + und + "_up_0508_.csv";
                }

        }
        else{
        	if(isLib){
                        fname = "/home/pranka/work/Strat_Init/Strat/temp/data_lib_" + und + "_down_0508_.csv";
                }
                else{
                        fname = "/home/pranka/work/Strat_Init/Strat/temp/data_" + und + "_down_0508_.csv";
                }
        }
        static std::ofstream output[4];
        output[exp_id].open(fname, std::ios_base::app); // append instead of overwrite
        std::string out = "";
        std::stringstream ss("");
        
        ss.precision(5);
        ss << std::fixed;
        if (logger_i[exp_id]==0) {
                logger_i[exp_id]++;
                out += "time,machine,submachine,symbol,pos,quantity,timestamp,spot,price,strike,tte,type,iv,delta,gamma,rho,vega,theta,spot_spot_0.02,price_spot_0.02,strike_spot_0.02,shock_spot_0.02,tte_spot_0.02,type_spot_0.02,iv_spot_0.02,delta_spot_0.02,gamma_spot_0.02,rho_spot_0.02,vega_spot_0.02,theta_spot_0.02,spot_spot_0.05,price_spot_0.05,strike_spot_0.05,shock_spot_0.05,tte_spot_0.05,type_spot_0.05,iv_spot_0.05,delta_spot_0.05,gamma_spot_0.05,rho_spot_0.05,vega_spot_0.05,theta_spot_0.05,spot_spot_0.1,price_spot_0.1,strike_spot_0.1,shock_spot_0.1,tte_spot_0.1,type_spot_0.1,iv_spot_0.1,delta_spot_0.1,gamma_spot_0.1,rho_spot_0.1,vega_spot_0.1,theta_spot_0.1,spot_spot_0.15,price_spot_0.15,strike_spot_0.15,shock_spot_0.15,tte_spot_0.15,type_spot_0.15,iv_spot_0.15,delta_spot_0.15,gamma_spot_0.15,rho_spot_0.15,vega_spot_0.15,theta_spot_0.15,spot_iv_0.02,price_iv_0.02,strike_iv_0.02,shock_iv_0.02,tte_iv_0.02,type_iv_0.02,iv_iv_0.02,delta_iv_0.02,gamma_iv_0.02,rho_iv_0.02,vega_iv_0.02,theta_iv_0.02,spot_iv_0.05,price_iv_0.05,strike_iv_0.05,shock_iv_0.05,tte_iv_0.05,type_iv_0.05,iv_iv_0.05,delta_iv_0.05,gamma_iv_0.05,rho_iv_0.05,vega_iv_0.05,theta_iv_0.05";
                out+="\n";
                std::cout << out << std::endl;
                output[exp_id] << out;
        }
        output[exp_id].precision(5);
        output[exp_id] << std::fixed;
        std::map<std::string, Tim::Instrument*> TU_s = expiryTU[Tim::ExpiryType::Weekly];
        struct tm t2 = {0}; 
        t2.tm_year = Tim::Helpers::getYear(20210805);
        t2.tm_mon = Tim::Helpers::getMonth(20210805);
        t2.tm_mday = Tim::Helpers::getDay(20210805);
        double s_pnl;

        for(int i = 0; i < csv_data.size(); i++){
        	// std::cout << csv_data[i][0] << " " << csv_data[i][1] << " " << csv_data[i][2] << " " << csv_data[i][3] << " " << csv_data[i][4] << " " << csv_data[i][5] << std::endl;
                t2.tm_hour = std::stoi(csv_data[i][0])/100;
                t2.tm_min = std::stoi(csv_data[i][0]) % 100;
                t2.tm_sec = 0;
                std::time_t tSE = mktime(&t2);
                uint64_t rowtime = long(tSE)*E9;
                if (rowtime==model_starttime){
                        rowtime+=60*long(E9);
                } 

                Tim::Options* opt_opp;

                std::string sym = csv_data[i][3];
//                std::cout << unHand->TradeUniverse.size() << "\n";
//                std::cout << "\nCSV_Data: i= " << i << " " << sym << std::endl  ;


                if (rowtime >= timestamp && rowtime < timestamp + long(E9)){
                		Tim::Options* opt = unHand->TradeUniverse[csv_data[i][3]]->opt;
                        std::string ss_str = "";
                        if (isLib){
                                agShockMap[sym].onMdUpdate(und, opt->underlying_price/100, timestamp/(1));
                                agShockMap[sym].onMdUpdate(sym, opt->mid_px, rowtime/(1));
                                agShockMap[sym].onTradeUpdate(sym, std::stoi(csv_data[i][5]) > 0 ?'B':'S', (opt->mid_px), std::abs(std::stoi(csv_data[i][5])) , timestamp);
                                
                        }
                        ss << csv_data[i][0] << "," << csv_data[i][1] << "," << csv_data[i][2] << "," <<csv_data[i][3] << "," << csv_data[i][4] << ","<< csv_data[i][5] << "," << rowtime << "," << opt->underlying_price << "," << opt->mid_px << "," << opt->strike << "," << opt->time_to_expiry << "," << opt->str_opt_type << "," << opt->volatility << "," << opt->delta << "," << opt->gamma << "," << opt->rho << "," << opt->vega << "," <<opt->theta<< ",";
                        for(int k = 0; k < spot_m.size(); k++){
                                // std::string uMoves = spot_moves[k], vMoves = "0";
                                double sm = std::stod(spot_m[k]);
                                double ivm = 0.0;
//                                s_pnl = s.getScenarioPnl((sm), (ivm))*100;
                                double newspot = (1+sm)* opt->underlying_price;
                                Tim::GreekCalculator G(0);
                                Tim::Options n_opt;
                                n_opt = *opt;
                                Tim::Options* new_opt = &n_opt;
                                new_opt->underlying_price = (newspot);
                                double new_mid_px = G.findBSPrice(new_opt, newspot, 0);
                                new_opt->mid_px = new_mid_px;
                                Tim::Greeks g_new = G.getGreeks(new_opt, newspot, 0);
                                new_opt->updateAllGreeks(g_new);
                                if (isLib){
                                	auto s = agShockMap[sym].getShockRisk();
                                	s_pnl = s.getScenarioPnl((sm), (ivm))*100;
                                }
                                else{
                                	s_pnl = (new_mid_px - opt->mid_px)* double(std::stoi(csv_data[i][4]));
                                }

                                ss << new_opt->underlying_price<< "," << new_opt->mid_px<< "," << new_opt->strike << "," << s_pnl << "," << new_opt->time_to_expiry<< "," << new_opt->str_opt_type<< "," << new_opt->volatility<< "," << new_opt->delta<< "," << new_opt->gamma<< "," << new_opt->rho<< "," << new_opt->vega<< "," << new_opt->theta << ",";
                        }
                        for(int k = 0; k < iv_m.size(); k++){
                                double sm = 0;
                                double ivm = std::stod(iv_m[k]);
                                double newiv = opt->volatility + ivm;
                                Tim::GreekCalculator G(0);
                                Tim::Options n_opt;
                                n_opt = *opt;
                                Tim::Options* new_opt = &n_opt;
                                double new_mid_px = findBSOptPrice(new_opt, opt->underlying_price, ivm);
                                new_opt->mid_px = new_mid_px;
                                
                                Tim::Greeks g_new = G.getGreeks(new_opt, opt->underlying_price, 0);
                                new_opt->updateAllGreeks(g_new);
                                if (isLib){
                                        auto s = agShockMap[sym].getShockRisk();
                                        s_pnl = s.getScenarioPnl((sm), (ivm))*100;
                                }
                                else{
                                        s_pnl = (new_mid_px - opt->mid_px)* double(std::stoi(csv_data[i][4]));
                                }
                                ss << new_opt->underlying_price<< "," << new_opt->mid_px<< "," << new_opt->strike << "," << s_pnl << "," << new_opt->time_to_expiry<< "," << new_opt->str_opt_type<< "," << new_opt->volatility<< "," << new_opt->delta<< "," << new_opt->gamma<< "," << new_opt->rho<< "," << new_opt->vega<< "," << new_opt->theta << ",";

                        }
                        ss_str = ss.str();
                        ss_str.pop_back();
                        ss.str(std::string());
                        output[exp_id] << ss_str+"\n";
                        std::cout << ss_str << std::endl;
                }
                else{
                	continue;
                }
                
        }
        output[exp_id].close();
}
double DataDumper::findBSOptPrice(Tim::Options* option, double spot_px, double iv_move){
    double divisor = 1.0/option->fields->feed_multiplier;
    double risk_free_rate = 0;
    double spot_price = (double)spot_px/divisor;
    double strike = (double)option->d_strike/divisor;
    double time_to_expiry=option->time_to_expiry;
    double min_iv = 0.0001, max_iv = 4;
    double iv_ = option->volatility;
    double iv = iv_ + iv_move;
     bool mispriced = option->mispriced;
     if (option->ask_px == 0 || option->bid_px == 0 || spot_price == 0 || mispriced || miscle(iv, min_iv) || miscge(iv, max_iv)) {
       return option->mid_px;
     }
    black_scholes::BlackScholesCall bsc(spot_price, strike, risk_free_rate, time_to_expiry);
    if(option->str_opt_type == 'c'){
     return divisor*bsc(iv);
    }
    else{
     double call_px = bsc(iv);
     return divisor*(call_px - spot_price + strike * exp(-risk_free_rate * time_to_expiry));
    }
  }

	
void DataDumper::onGreekTimerUpdate(uint64_t timestamp_new){
        if (timestamp_new >= model_starttime) {
                if(timestamp + unit_time <= timestamp_new){
                        timestamp += unit_time;
                        setDirty();
                        _on_greek_timer_update = true;
                }
                // else if(timestamp_new == model_starttime){
                //         timestamp +=long(E9);
                //          setDirty();
                //         _on_greek_timer_update = true;
                // }
        }
}

void DataDumper::onNotify(){
        
         MyLogger::log(TRADEFILE, "DatDumperHandler::onNotify |", "_on_timer_update:", _on_timer_update, " |_on_greek_time_update:", _on_greek_timer_update);
          if(_on_greek_timer_update ){
                  auto start = std::chrono::high_resolution_clock::now();
        	  static int ctr = 1;
              _on_timer_update = false;
              _on_greek_timer_update = false;
                bool getdata = false;
                // for (int j = 0; j<timestamp_ranges.size(); j++){
                //         std::vector<uint64_t> tt = timestamp_ranges[j];
                //         if (timestamp>=tt[0] && timestamp<=tt[1]){
                //                 getdata = true;
                //                 break;
                //         }       
                // }
                if (true){
                        std::cout << "Acceptable TimeStamp: " << Tim::Helpers::getHumanReadableTime(timestamp) << std::endl;
                        std::cout << "Entering dumper \n";
                //       DataDumper::shockDumper3(timestamp, spot_moves, iv_moves, "BANKNIFTY", true, true, 0);
                //       DataDumper::shockDumper3(timestamp, spot_moves2, iv_moves2, "BANKNIFTY", false, true, 1);
                //       DataDumper::shockDumper3(timestamp, spot_moves, iv_moves, "BANKNIFTY", true, false, 2);
                //       DataDumper::smileDataDumper(timestamp);
                       if (ctr==300){
                    	   DataDumper::alldataDumper(timestamp, true);
                    	   ctr = 0;
                       }
                       else{
                    	   DataDumper::alldataDumper(timestamp, false);
                       }
                //        DataDumper::shockDumper2(timestamp);
                //        DataDumper::shockDumper3(timestamp);
                //        DataDumper::shockDumper4(timestamp);
//                        DataDumper::shockDumper2(timestamp);
                        // DataDumper::alldataDumper(timestamp);
                }
                ctr++;
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        
                // To get the value of duration use the count()
                // member function on the duration object
                std::cout << "Duration: " << duration.count() << std::endl;
                        

         }
 }

void DataDumper::setUnitTime(uint64_t unit_time_t){
          unit_time = unit_time_t;
      }
  
uint64_t DataDumper::getUnitTime(){
          return unit_time;
}
  
void DataDumper::setTimeStampRanges(){
        for(auto i:time_ranges){
                uint64_t st_time = Tim::Helpers::epochTimeInNanos(date, i.start_hour, i.start_min, 0, 0);
                uint64_t e_time = Tim::Helpers::epochTimeInNanos(date, i.end_hour, i.end_min, 0, 0);
                std::cout << "StartTime: " << st_time << ", EndTime: " << e_time << std::endl;
                std::vector<uint64_t> t{st_time, e_time};
                timestamp_ranges.push_back(t);
        }
}    
void DataDumper::smileDataDumper(uint64_t timestamp){
        static int logger_i = 0;
        if ((*Tim::Helpers::timestamp_ptr/(1000*1000*1000LL))%1 != 0) {
             return;
        }
        std::string fname = *Tim::Helpers::folder_ptr + "/iv_smile_" + std::to_string(*Tim::Helpers::date_ptr) +"_" + std::to_string(*Tim::Helpers::pid_ptr) +  ".csv";
        static std::ofstream output;
        std::string out = "timestamp,symbol,spot,price,iv,delta,vega,";
        if (logger_i==0) {
                output.open(fname, std::ios_base::trunc); // append instead of overwrite
                ++logger_i;
                out.pop_back();
                out+="\n";
                std::cout << out << std::endl;
                output << out;
                output.close();
        }
        output.precision(10);
        output << std::fixed;
        std::stringstream ss("");
        ss.precision(4);
        ss << std::fixed;
        output.open(fname, std::ios_base::app);
        for(auto itr:unHand->TradeUniverse){
                ss << Tim::Helpers::getHumanReadableTime(timestamp) << "," << itr.second->opt->symbol << "," << 
                itr.second->opt->underlying_price << "," << itr.second->opt->mid_px << "," << itr.second->opt->volatility << "," << 
                itr.second->opt->delta << "," << itr.second->opt->vega << "\n";
                std::cout << ss.str();
        }
        std::string ss_str = ss.str();
        output << ss_str;
        output.close();
}


void DataDumper::alldataDumper(uint64_t timestamp, bool refresh){

        static int logger_i = 0;
        if ((*Tim::Helpers::timestamp_ptr/(1000*1000*1000LL))%1 != 0) {
             return;
        }

        std::vector<double> buckets{0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9};
        std::vector<std::string> cp{"_C_", "_P_"};
        std::string fname = *Tim::Helpers::folder_ptr + "/oi_data_new_" + std::to_string(*Tim::Helpers::date_ptr) +"_" + std::to_string(*Tim::Helpers::pid_ptr) +  ".csv";
        static std::ofstream output;
        
        std::string out = "timestamp,spot,"; //spot_AXIS,spot_HDFC,spot_ICICI,spot_KOTAK,spot_SBI,";
        std::vector<std::string> cols{"oi","ntp","volume"};
        if (logger_i==0) {
                output.open(fname, std::ios_base::trunc); // append instead of overwrite
                ++logger_i;
                for (int x = 0; x < cols.size(); x++){
                	for (int y = 0; y < cp.size(); y++){
                		for (int z = 0; z < buckets.size(); z++){
                			out+=(cols[x]+cp[y]+std::to_string(buckets[z])+",");
                		}
                	}
                }
//                out += "timestamp,symbol,delta,oi,ltp,ltq,ltp*ltq,ttq,";
                out.pop_back();
                out+="\n";
                std::cout << out << std::endl;
                if (output.is_open())
                        output << out;
                output.close();
        }
        output.precision(10);
        output << std::fixed;
        // for(int i = 0; i < 5; i++){
        // 	oi_data.push_back({0.0,0.0,0.0,0.0});
        // }
        static int32_t prev_vol = 0;
        static std::vector<double> oi(20,0), prev(20,0) ;
        static std::vector<int64_t> vlm(20,0);
        std::vector<uint64_t> ntp{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        std::map<std::string, Tim::Instrument*> TU_s = expiryTU[Tim::ExpiryType::Weekly];
        std::stringstream ss("");
        ss.precision(4);
        ss << std::fixed;
        double spot = 0;
        double spotA = 0,spotH = 0, spotI = 0,spotK = 0,spotS = 0;
        
        for(auto itr:unHand->TradeUniverse){
                if ((itr.second->opt->delta>=0.0 && itr.second->opt->delta<0.1) ){
                        oi[0] += itr.second->opt->OI;
                        vlm[0]+=itr.second->opt->volume;
                        itr.second->opt->ntp[0]+= itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta>=0.1 && itr.second->opt->delta<0.2) ){
                        oi[1] += itr.second->opt->OI;
                        vlm[1]+=itr.second->opt->volume;
                        itr.second->opt->ntp[1]+= itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta>=0.2 && itr.second->opt->delta<0.3) ){
                        oi[2] += itr.second->opt->OI;
                        vlm[2]+=itr.second->opt->volume;
                        itr.second->opt->ntp[2]+=  itr.second->opt->net_traded_premium;
                }
                else if ((itr.second->opt->delta>=0.3 && itr.second->opt->delta<0.4) ){
                        oi[3] += itr.second->opt->OI;
                        vlm[3]+=itr.second->opt->volume;
                        itr.second->opt->ntp[3]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta>=0.4 && itr.second->opt->delta<0.5) ){
                        oi[4] += itr.second->opt->OI;
                        vlm[4]+=itr.second->opt->volume;
                        itr.second->opt->ntp[4]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta>=0.5 && itr.second->opt->delta<0.6) ){
                        oi[5] += itr.second->opt->OI;
                        vlm[5]+=itr.second->opt->volume;
                        itr.second->opt->ntp[5]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta>=0.6 && itr.second->opt->delta<0.7) ){
                        oi[6] += itr.second->opt->OI;
                        vlm[6]+=itr.second->opt->volume;
                        itr.second->opt->ntp[6]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta>=0.7 && itr.second->opt->delta<0.8) ){
                        oi[7] += itr.second->opt->OI;
                        vlm[7]+=itr.second->opt->volume;
                        itr.second->opt->ntp[7]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta>=0.8 && itr.second->opt->delta<0.9) ){
                        oi[8] += itr.second->opt->OI;
                        vlm[8]+=itr.second->opt->volume;
                        itr.second->opt->ntp[8]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta>=0.9 && itr.second->opt->delta<=1) ){
                        oi[9] += itr.second->opt->OI;
                        vlm[9]+=itr.second->opt->volume;
                        itr.second->opt->ntp[9]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta<-1*0.0 && itr.second->opt->delta>=-1*0.1)){
                        oi[10] += itr.second->opt->OI;
                        vlm[10]+=itr.second->opt->volume;
                        itr.second->opt->ntp[10]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta<-1*0.1 && itr.second->opt->delta>=-1*0.2)){
                        oi[11] += itr.second->opt->OI;
                        vlm[11]+=itr.second->opt->volume;
                        itr.second->opt->ntp[11]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta<-1*0.2 && itr.second->opt->delta>=-1*0.3)){
                        oi[12] += itr.second->opt->OI;
                        vlm[12]+=itr.second->opt->volume;
                        itr.second->opt->ntp[12]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta<-1*0.3 && itr.second->opt->delta>=-1*0.4)){
                        oi[13] += itr.second->opt->OI;
                        vlm[13]+=itr.second->opt->volume;
                        itr.second->opt->ntp[13]+= itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta<-1*0.4 && itr.second->opt->delta>=-1*0.5)){
                        oi[14] += itr.second->opt->OI;
                        vlm[14]+=itr.second->opt->volume;
                        itr.second->opt->ntp[14]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta<-1*0.5 && itr.second->opt->delta>=-1*0.6)){
                        oi[15] += itr.second->opt->OI;
                        vlm[15]+=itr.second->opt->volume;
                        itr.second->opt->ntp[15]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta<-1*0.6 && itr.second->opt->delta>=-1*0.7)){
                        oi[16] += itr.second->opt->OI;
                        vlm[16]+=itr.second->opt->volume;
                        itr.second->opt->ntp[16]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta<-1*0.7 && itr.second->opt->delta>=-1*0.8)){
                        oi[17] += itr.second->opt->OI;
                        vlm[17]+=itr.second->opt->volume;
                        itr.second->opt->ntp[17]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta<-1*0.8 && itr.second->opt->delta>=-1*0.9)){
                        oi[18] += itr.second->opt->OI;
                        vlm[18]+=itr.second->opt->volume;
                        itr.second->opt->ntp[18]+=  itr.second->opt->net_traded_premium;

                }
                else if ((itr.second->opt->delta<-1*0.9 && itr.second->opt->delta>=-1)){
                        oi[19] += itr.second->opt->OI;
                        vlm[19]+=itr.second->opt->volume;
                        itr.second->opt->ntp[19]+=  itr.second->opt->net_traded_premium;

                }
                
                // std::cout << Tim::Helpers::getHumanReadableTime(timestamp) << " " << itr.second->opt->symbol << " " << "Delta: " << itr.second->opt->delta <<
                // " " << "OI: " << itr.second->opt->OI << "\n";
                /*
                for(int i = 0; i < itr.second->opt->ntp.size(); i++){
                        std::cout << i << ": " << itr.second->opt->ntp[i] << " " ;
                }
                std::cout << "\n";

                */
                // std::cout << itr.second->opt->symbol << " " << itr.second->opt->underlying << " " << itr.second->opt->underlying_price << std::endl;			
                itr.second->opt->prev_volume = itr.second->opt->volume;
       }
       if (refresh){
                output.open(fname, std::ios_base::app); // append instead of overwrite
        	ss << Tim::Helpers::getHumanReadableTime(timestamp)<< ",";
        	for(auto itr:unHand->TradeUniverse){
        		ntp[0]+= itr.second->opt->ntp[0];
        		ntp[1]+= itr.second->opt->ntp[1];
        		ntp[2]+= itr.second->opt->ntp[2];
        		ntp[3]+= itr.second->opt->ntp[3];
        		ntp[4]+= itr.second->opt->ntp[4];
        		ntp[5]+= itr.second->opt->ntp[5];
        		ntp[6]+= itr.second->opt->ntp[6];
        		ntp[7]+= itr.second->opt->ntp[7];
        		ntp[8]+= itr.second->opt->ntp[8];
        		ntp[9]+= itr.second->opt->ntp[9];
                        ntp[10]+= itr.second->opt->ntp[10];
                        ntp[11]+= itr.second->opt->ntp[11];
                        ntp[12]+= itr.second->opt->ntp[12];
                        ntp[13]+= itr.second->opt->ntp[13];
                        ntp[14]+= itr.second->opt->ntp[15];
                        ntp[15]+= itr.second->opt->ntp[15];
                        ntp[16]+= itr.second->opt->ntp[16];
                        ntp[17]+= itr.second->opt->ntp[17];
                        ntp[18]+= itr.second->opt->ntp[18];
                        ntp[19]+= itr.second->opt->ntp[19];
        		for(int x = 0; x < itr.second->opt->ntp.size(); x++){
        			itr.second->opt->ntp[x] = 0;
        		}
                        spot = itr.second->opt->underlying_price;
                        // if (itr.second->opt->underlying == "AXISBANK")
                        //         spotA = itr.second->opt->underlying_price;
                        // else if (itr.second->opt->underlying == "HDFCBANK")
                        //         spotH = itr.second->opt->underlying_price;
                        // else if (itr.second->opt->underlying == "ICICIBANK")
                        //         spotI = itr.second->opt->underlying_price;
                        // else if (itr.second->opt->underlying == "KOTAKBANK")
                        //         spotK = itr.second->opt->underlying_price;
                        // else if (itr.second->opt->underlying == "SBIN")
                        //         spotS = itr.second->opt->underlying_price;
                        std::cout << Tim::Helpers::getHumanReadableTime(timestamp) << " " << itr.second->opt->symbol << " " << "Delta: " << itr.second->opt->delta <<
                " " << "OI: " << itr.second->opt->OI << "          Spot " << itr.second->opt->underlying_price << "\n";
        	}
                ss << spot << ",";
                // ss << spotA << ","<< spotH << ","<< spotI << ","<< spotK << ","<< spotS << ",";
                for(int x = 0; x < 20; x++){
                        ss << oi[x] << ",";
                }
                for(int x = 0; x < 20; x++){
                        ss << ntp[x]<<",";
                }
                for(int x = 0; x < 20; x++){
                        ss << vlm[x] << ",";
                }
                std::string ss_str = ss.str();
                ss_str.pop_back();
                output << ss_str << "\n";
                output.close();
        }
        for(int x = 0; x < 20; x++){
                oi[x] = 0;
                vlm[x] = 0;
        }
        for(auto itr:unHand->TradeUniverse){
			itr.second->opt->net_traded_premium = 0;
			itr.second->opt->tot = 0;
        }
}
void DataDumper::onTimerUpdate(uint64_t timestamp_new){
//	MyLogger::log(TRADEFILE, "DataDumperHandler::onTimerUpdate |trade_starttime:" , trade_starttime, " |timestamp_new:", timestamp_new, " |trade_endtime:", trade_endtime);
 //       if(timestamp_new >= trade_starttime && timestamp_new <= trade_endtime){
 // 
 //       	if(timestamp + unit_time <= timestamp_new){
 //              		timestamp += unit_time;
 //                 	setDirty();
 //                 	_on_timer_update = true;
 //             }
 //         }
  
 //         else{
 //             MyLogger::log(TRADEFILE, "StrategyTime |Error: Time not in Trading time range!");
  //        }
}
}
// void DataDumper::optionStockLeadLag(uint64_t timestamp){

// 	static int logger_i = 0;
// 	Tim::Options* this_opt = nullptr;
//         static std::ofstream output(*Tim::Helpers::folder_ptr + "/data_option_stock" + std::to_string(*Tim::Helpers::date_ptr) +"_" + std::to_string(*Tim::Helpers::pid_ptr) +  ".txt");	
// 	if ((*Tim::Helpers::timestamp_ptr/(1000*1000*1000LL))%1 != 0) {
//              return;
//          }
//         if (logger_i==0) {
//              ++logger_i;
//         output << "date_time,date,time,symbol,Option_type,strike_price,ask_px, bid_px, trade_px, mid_price,  oi,spot_px,volume,moneyness, weight\n";

//          }

// 	output.precision(4);
//         output << std::fixed;
//         MyLogger::log(TRADEFILE, "\n-------\nDataDumper: Time: ",Tim::Helpers::getHumanReadableTime(timestamp),"\n");

// 	for(auto it=unHand->TradeUniverseIDs.begin(); it!=unHand->TradeUniverseIDs.end(); ++it) {
//                  if(it->second->isOption){
// 			this_opt = it->second->opt;
// 			if(this_opt->underlying =="BANKNIFTY" && (this_opt->opt_type == Tim::OptionType::Call)){
				
// 				std::stringstream ss("");
//         			ss.precision(4);
//         			ss << std::fixed;
//         			std::string dateTime = std::to_string(date) +" "+ Tim::Helpers::getHumanReadableTime(timestamp);
//         			output<< dateTime;
// 				ss <<","<<date<<","<<Tim::Helpers::getHumanReadableTime(timestamp) <<","<<this_opt->symbol<<","<< this_opt->str_opt_type<<","<< this_opt->strike <<"," << this_opt->ask_px << "," << this_opt->bid_px << "," << this_opt->trade_px << ","<<this_opt->mid_px<<","<< this_opt->OI<<","<<this_opt->underlying_price <<","<<this_opt->volume<<","<<this_opt->strike/this_opt->underlying_price<<","<<this_opt->volume*this_opt->mid_px<<"\n";
// 				output <<ss.str();
// 			}
// 		}
// 	}
	

// }


// for (auto itr = unHand->TradeUniverseIDs.begin(); itr != unHand->TradeUniverseIDs.end();itr++){
//         int k = 0;
//       for (int i = 0; i < spot_moves.size(); i++) {
//         for (int j = 0; j < iv_moves.size(); j++) {
//           GS->addShockParam(Tim::ShockParam::SPOT, spot_moves[i]);
//           GS->addShockParam(Tim::ShockParam::IV, iv_moves[j]);
//           Tim::Options* oldopt = unHand->TradeUniverseIDs[itr->first]->opt;
//           auto this_greeks = GS->getGreeksForShock(oldopt->underlying_price, oldopt, 0.06);
//           std::cout << "For spot shock of " << spot_moves[i] << " and iv shock of " << 
//             iv_moves[j] << ", Symbol: " << oldopt->symbol<< ":\n" << "Old Opt Price: " << oldopt->mid_px << 
//             ", New Opt Price: " << oldopt->greek_shock_price<< ", Opt Spot: " << oldopt->underlying_price 
//             << " Opt tte: " << oldopt->time_to_expiry << "< Opt IV: " << oldopt->volatility <<"\n"; 
//         //     << itr->second.actual_pos << "\n" << "Net risk: " << 
//         //     itr->second.actual_pos * (oldopt->greek_shock_price - oldopt->trade_px) << "\n";

//         //   portfolio_risk[k]+= itr->second.actual_pos * (oldopt->greek_shock_price - 
//         //       oldopt->trade_px);
//           GS->resetShockParam(Tim::ShockParam::SPOT);
//           GS->resetShockParam(Tim::ShockParam::IV);
//           k++;
//         }
//       }
//           }


// std::tuple<Tim::Options*,Tim::Options*, int,int> DataDumper::findATMOption(std::map<std::string, Tim::Instrument*> T_U){
//         double max_delta = 0.5;
//         double min_delta = -0.5;
//         Tim::Options* ATMCallOption = nullptr;
//         Tim::Options* ATMPutOption = nullptr;
//         Tim::Options* this_opt = nullptr;
//         int this_call_id = -1;
//          int this_put_id = -1;
//         double callDiff = 1;
//         double putDiff = 1;
        
//         for (auto it=T_U.begin(); it!=T_U.end(); ++it) {
//                 if(it->second->isOption){
//                         this_opt = it->second->opt;

//                         if( this_opt->opt_type == Tim::OptionType::Call && this_opt->delta < max_delta && (max_delta - this_opt->delta) < callDiff){
//                                 callDiff = max_delta - this_opt->delta;
//                                 ATMCallOption = this_opt;
//                                 this_call_id = unHand->symbol_id_map[it->first];
//                         }
//                         if( this_opt->opt_type == Tim::OptionType::Put && this_opt->delta > min_delta && (this_opt->delta - min_delta) < putDiff){
//                                 putDiff = this_opt->delta - min_delta;
//                                 ATMPutOption = this_opt;
//                                 this_put_id = unHand->symbol_id_map[it->first];                     
//                         }
//                 }
//         }
//         std::cout<<"ATM Call Selected "<<ATMCallOption->symbol_id<<" ** "<<ATMCallOption->delta<<" ** "<<ATMCallOption->strike<<"\n";
//         std::cout<<"ATM Put Selected "<<ATMPutOption->symbol_id<<" ** "<<ATMPutOption->delta<<" ** "<<ATMPutOption->strike<<"\n";

//         return std::make_tuple(ATMCallOption, ATMPutOption,this_call_id, this_put_id);
// }

// std::tuple<Tim::Options*, Tim::Options*, Tim::Options*, Tim::Options*, double, double> DataDumper::atmHighLowIV(std::map<std::string, Tim::Instrument*> T_U, double underlyingSpot){

// 	int callHighDiff = INT_MAX;
// 	int callLowDiff = INT_MIN;
// 	int putHighDiff = INT_MAX;
// 	int putLowDiff = INT_MIN;

// 	Tim::Options* atmHighCall;
// 	Tim::Options* atmLowCall;
// 	Tim::Options* atmHighPut;
// 	Tim::Options* atmLowPut;
// 	Tim::Options* this_opt;
// 	double atmCallHighStrike;
// 	double atmCallLowStrike;
// 	double atmPutHighStrike;
// 	double atmPutLowStrike;
// 	for (auto it = T_U.begin(); it!= T_U.end(); ++it)
// 	{
// 		if(it->second->isOption)
// 		{
// 			this_opt = it->second->opt;
// 			if(this_opt->underlying =="BANKNIFTY" && this_opt->opt_type == Tim::OptionType::Put && this_opt->strike < underlyingSpot){
// 				//MyLogger::log(TRADEFILE, "\n-------\n DataDumper: Delta check testing ", "Time ", Tim::Helpers::getHumanReadableTime(timestamp), " Symbol ",this_opt->symbol,"strike ", this_opt->strike, "Underlying ", underlyingSpot, "delta", this_opt->delta,"\n-----\n");
// 			}
// 		}
// 	}	
// 	for(auto it= T_U.begin(); it!= T_U.end(); ++it) {
//                    if(it->second->isOption){
//                            this_opt = it->second->opt;
// 			  // if(std::abs(this_opt->delta) > delta_lower_cap){		  
// 			  if(this_opt->underlying =="BANKNIFTY" && this_opt->opt_type == Tim::OptionType::Call && this_opt->strike > underlyingSpot && (this_opt->strike- underlyingSpot) < callHighDiff)				 {
// 				callHighDiff = this_opt->strike- underlyingSpot;
// 				atmHighCall = this_opt;
// 				atmCallHighStrike = this_opt->strike;
// 			   }
				
// 			  if(this_opt->underlying =="BANKNIFTY" && this_opt->opt_type == Tim::OptionType::Call && this_opt->strike < underlyingSpot && (this_opt->strike- underlyingSpot) > callLowDiff){
//                                  callLowDiff = this_opt->strike- underlyingSpot;
//                                  atmLowCall = this_opt;
// 				 atmCallLowStrike = this_opt->strike;
//                         }
			  	
// 			  if(this_opt->underlying =="BANKNIFTY" && this_opt->opt_type == Tim::OptionType::Put && this_opt->strike > underlyingSpot && (this_opt->strike- underlyingSpot) < putHighDiff){
//                                  putHighDiff = this_opt->strike- underlyingSpot;
//                                  atmHighPut = this_opt;
// 				 atmPutHighStrike = this_opt->strike;
//                             }
		          
// 			  if(this_opt->underlying =="BANKNIFTY" && this_opt->opt_type == Tim::OptionType::Put && this_opt->strike < underlyingSpot && (this_opt->strike- underlyingSpot) >  putLowDiff){
//                                  putLowDiff = this_opt->strike- underlyingSpot;
//                                  atmLowPut = this_opt;
// 				 atmPutLowStrike = this_opt->strike;
//                             }
// 			//}
// 			}
// 		}	

// 	std::cout<<"\n Data "<<atmCallHighStrike<<" ** "<<atmCallLowStrike<<" ** "<<atmPutHighStrike<<" ** "<<atmPutLowStrike<<"\n";

// return std::make_tuple(atmHighCall, atmLowCall, atmHighPut, atmLowPut, atmCallHighStrike, atmCallLowStrike);	
// }

// std::tuple<double, double> DataDumper::findATMWeightedInfo(Tim::Options* ATMCall, Tim::Options* ATMPut, Tim::ExpiryType exp){
//         std::map<std::string, Tim::Instrument*> TU = expiryTU[exp];
// 	double underlyingSpot = ATMCall->underlying_price;
// 	//double underlyingSpot2 = ATMPut->underlying_price;
// 	std::cout<<"\n underlying spot price "<<underlyingSpot<<"\n";;
// 	//double atmCallStrike = ATMCall->strike;
// 	//double atmPutStrike = ATMPut->strike;
// 	std::tuple<Tim::Options*, Tim::Options*, Tim::Options*, Tim::Options*, double, double> highLowATMOptions = atmHighLowIV(TU, underlyingSpot);
// 	Tim::Options* atmHighCall = std::get<0>(highLowATMOptions);
// 	Tim::Options* atmLowCall = std::get<1>(highLowATMOptions);
// 	Tim::Options* atmHighPut = std::get<2>(highLowATMOptions);
// 	Tim::Options* atmLowPut = std::get<3>(highLowATMOptions);
// 	double atmHighStrike = std::get<4>(highLowATMOptions);
// 	double atmLowStrike = std::get<5>(highLowATMOptions);
// 	MyLogger::log(TRADEFILE, "\n-------\nDataDumper: ATM Low Call IV",
//                atmLowCall->volatility, "  ATM Low Put IV ", atmLowPut->volatility, "  ATM High Call IV ", atmHighCall->volatility,  "ATM High Put IV", atmHighPut->volatility,"ATM High Strike", atmHighStrike,"ATM Low Strike", atmLowStrike,"underlying Spot", underlyingSpot," atmLowCallTradePrice ", atmLowCall->trade_px, " atmHighCallTradePrice ", atmHighCall->trade_px," atmLowPutTradePrice ", atmLowPut->trade_px," atmHighPutTrade Price ", atmHighPut->trade_px, " ATM low call symbol ", atmLowCall->symbol," ATm High Call Symbol ", atmHighCall->symbol, " ATM LOW Put Symbol ", atmLowPut->symbol," ATM Hih Put Symbol ", atmHighPut->symbol, " atm low call ask price ", atmLowCall->ask_px, " atm high call ask price ", atmHighCall->ask_px, "**** ", atmHighCall->LTT, "\n-------\n");
// 	double atmWeightedIV = 0.5*((atmLowCall->volatility + atmLowPut->volatility)*(atmHighStrike - underlyingSpot) + (atmHighCall->volatility + atmHighPut->volatility)*(underlyingSpot - atmLowStrike));
// 	atmWeightedIV /= std::abs(atmHighStrike - atmLowStrike);
// 	double atmWeightedVega = 0.5*((atmLowCall->vega + atmLowPut->vega)*(atmHighStrike - underlyingSpot) + (atmHighCall->vega + atmHighPut->vega)*(underlyingSpot - atmLowStrike));
// 	atmWeightedVega /= std::abs(atmHighStrike - atmLowStrike);
// 	MyLogger::log(TRADEFILE, "\n-------\n DataDumper: ATM weightd IV",atmWeightedIV,"\n-----\n");
// 	return std::make_tuple(atmWeightedIV, atmWeightedVega);
	
// }











// // void DataDumper::writeData(Tim::Options* opt, uint64_t timestamp, double atmWeightedIV)
// // {
// //      	std::stringstream ss("");
// //       	ss.precision(4);
// //       	ss << std::fixed;
//
// // 	std::string dateTime = std::to_string(date) +" "+ Tim::Helpers::getHumanReadableTime(timestamp);
// //      	 std::cout<<"*******Time = "<<Tim::Helpers::getHumanReadableTime(timestamp)<<"********";
// // 	std::cout<<"**********Dumping Greek Data for "<< opt->symbol<<" ************\n";
//     
// //         ss <<","<<date<<","<<Tim::Helpers::getHumanReadableTime(timestamp) <<","<< opt->symbol<< ","<<opt->str_opt_type<<"," << opt->strike <<"," << opt->ask_px << "," << opt->bid_px << "," << opt->trade_px << "," << opt->gamma<< "," << opt->theta << "," << opt->delta << "," << opt->intrinsic_value << "," << opt->vega << "," <<  opt->OI << "," <<opt->underlying_price <<"," <<opt->actual_iv <<","<<atmWeightedIV <<"," <<opt->volatility<< ","<<opt->expiry_date<<"\n";
//
//
//
// // }
// // std::string DataDumper::createColumnNames(Tim::ExpiryType et){
// //         std::string output = "";
// //         for(auto s:DataDumper::exp_cols){
// //                 output+=s + "_" + std::to_string(int(et)) + "_c" + ",";
// //         }
// //         for(auto s:DataDumper::exp_cols){
// //                 output+=s + "_" + std::to_string(int(et)) + "_p" + ",";
// //         }
// //         // output.pop_back();
// //         return output;
// // }
//
// // std::string DataDumper::getATMRow(std::pair<Tim::ExpiryType, std::map<std::string, Tim::Instrument*>> TU){
// //         std::tuple<Tim::Options*,Tim::Options*,int, int> ATMOptions = findATMOption(TU.second);
// //         Tim::Options* callATM = std::get<0>(ATMOptions);
// //         Tim::Options* putATM = std::get<1>(ATMOptions);
// //         std::tuple<double, double> atmWeightedInfo = findATMWeightedInfo(std::get<0>(ATMOptions), std::get<1>(ATMOptions));
// //         double atmWeightedIV = std::get<0>(atmWeightedInfo);
// //         double atmWeightedVega = std::get<1>(atmWeightedInfo);
// //         double atmIVatmVega = atmWeightedIV * atmWeightedVega;
// // }
// // std::string DataDumper::createDefaultColumnNames(Tim::ExpiryType exp, int strike_rng){
// //         std::string output = "", distinguisher = "";
// //         if (dump_params->isSeparateATM){
// //                 for(int i = 100-strike_rng; i <= 100+strike_rng; i++){
// //                         for(auto s:DataDumper::def_cols){
// //                                 if(i==100){
// //                                         distinguisher = "ATM_100_c_" + std::to_string(int(et));
//                                        
// //                                 }
// //                                 else if (i <100){
// //                                         distinguisher = "ITM_0"+ std::to_string(i)+"_c_" + std::to_string(int(et));
// //                                 }
// //                                 else if (i>100){
// //                                         distinguisher = "OTM_"+ std::to_string(i)+"_c_" + std::to_string(int(et));
// //                                 }
// //                                 output += distinguisher + s + ",";
// //                         }
// //                         for(auto s:DataDumper::def_cols){
// //                                 if(i==100){
// //                                         distinguisher = "ATM_100_p_" + std::to_string(int(et));
//                                        
// //                                 }
// //                                 else if (i <100){
// //                                         distinguisher = "ITM_0"+ std::to_string(i)+"_p_" + std::to_string(int(et));
// //                                 }
// //                                 else if (i>100){
// //                                         distinguisher = "OTM_"+ std::to_string(i)+"_p_" + std::to_string(int(et));
// //                                 }
// //                                 output += distinguisher + s + ",";
// //                         }
// //                 }
// //         }
// //         else{
// //                 for(int i = 100-strike_rng; i!=100 && i <= 100+strike_rng; i++){
// //                         for(auto s:DataDumper::def_cols){
// //                                 if (i <100){
// //                                         distinguisher = "ITM_0"+ std::to_string(i)+"_c_" + std::to_string(int(et));
// //                                 }
// //                                 else if (i>100){
// //                                         distinguisher = "OTM_"+ std::to_string(i)+"_c_" + std::to_string(int(et));
// //                                 }
// //                                 output += distinguisher + s + ",";
// //                         }
// //                         for(auto s:DataDumper::def_cols){
// //                                 if (i <100){
// //                                         distinguisher = "ITM_0"+ std::to_string(i)+"_p_" + std::to_string(int(et));
// //                                 }
// //                                 else if (i>100){
// //                                         distinguisher = "OTM_"+ std::to_string(i)+"_p_" + std::to_string(int(et));
// //                                 }
// //                                 output += distinguisher + s + ",";
// //                         }
// //                 }
// //         }
//        
// //         // output.pop_back();
// //         return output;
// // }
//
//
// // void DataDumper::setheader()
// //   {
//
// // 	std::cout<<"**********Setting Header***********\n";
// //     std::stringstream ss("");
// //     ss.precision(3);
// //     ss << std::fixed;
// //     //std::cout<<"Test STring "<<ss.str()<<"\n";
// //     ss<<"";
//
// //     //for (auto itr = unHand->TradeUniverse.begin(); itr != unHand->TradeUniverse.end(); itr++)
// //     //{
//      
// //       //if (itr->second->isOption and itr->second->opt->underlying == "BANKNIFTY")
// //       //{
// //         //std::string symbol = itr->second->opt->symbol;
//        
// //         std::string list[] = {"date_time","date","time","symbol","Option_type","strike_price","ask_px", "bid_px", "trade_px", "gamma", "theta", "delta", "intrinsic_value", "vega", "oi","spot_px","actual_iv","atm_weighted_iv","volatility, expiry_date"};
// //         for (auto add : list)
// //         {
// //           //ss << "," <<  itr->second->opt->symbol+ "_" + std::to_string(int(itr->second->opt->strike/100)) + "_" + add;
// //           ss << ","<<add;
// //         }
//
// // 	//}
// // 	//break;
// //     //}
// // 	std::cout<<ss.str()<<"\n";
// //     MyLogger::log(STATSFILE, ss.str());
// //   }
//
// // std::tuple<Tim::Options*,Tim::Options*, int,int> DataDumper::findATMOption(){
// //          double max_delta = 0.5;
// // 	  double min_delta = -0.5;
// //          Tim::Options* ATMCallOption = nullptr;
// //          Tim::Options* ATMPutOption = nullptr;
// //          Tim::Options* this_opt = nullptr;
// //          int this_call_id = -1;
// //          int this_put_id = -1;
// //          double callDiff = 1;
// //          double putDiff = 1;
//        
// // 	for (auto it=unHand->TradeUniverseIDs.begin(); it!=unHand->TradeUniverseIDs.end(); ++it) {
// // 		if(it->second->isOption){
// // 			this_opt = it->second->opt;
// // 			//MyLogger::log(TRADEFILE, "Data Dumper time ", timestamp ,"\n");
// // 			//MyLogger::log(TRADEFILE, "Symbol ", this_opt->symbol, " Last traded time ", this_opt->LTT,"\n" );
// // 		}
// // 	}
// //          for(auto it=unHand->TradeUniverseIDs.begin(); it!=unHand->TradeUniverseIDs.end(); ++it) {
// //                  if(it->second->isOption){
// //                          this_opt = it->second->opt;
// // 			//std::cout<<"Data Dumper : Symbol"<< this_opt->symbol<<" *** "<<"current time "<< timestamp<< " ** "<<this_opt->timestamp<<" ***** "<<this_opt->LTT<<"\n";
// // 			// if(timestamp - this_opt->LTT <= (30*60)*(1000*1000*1000LL)){
// // 				//std::cout<<"TimeStamp diff: "<< (timestamp - this_opt->LTT)/1000*1000*1000LL<<"\n";
// //                          if( this_opt->opt_type == Tim::OptionType::Call && this_opt->delta < max_delta && (max_delta - this_opt->delta) < callDiff){
// //                                  callDiff = max_delta - this_opt->delta;
// //                                  ATMCallOption = this_opt;
// //                                  this_call_id = it->first;
// //                          }
// //                          if( this_opt->opt_type == Tim::OptionType::Put && this_opt->delta > min_delta && (this_opt->delta - min_delta) < putDiff){
// //                                   putDiff = this_opt->delta - min_delta;
// //                                   ATMPutOption = this_opt;
// //                                   this_put_id = it->first;                         
// // 			}
// // 			//}
//
 
// //                  }
// //          }
// //         // MyLogger::log(TRADEFILE, "\n----------\nData Dumper::ATM Call ID: ",
// //         //      ATMCallOption->symbol_id, "  ATM Call Delta ", ATMCallOption->delta, "  ATM Call Strike ", ATMCallOption->strike,"******",ATMCallOption->trade_px, "Exp type " ,ATMCallOption->exp_type, "last traded time: ", ATMCallOption->LTT," time ",timestamp, "\n-------\n");
 
// //          //MyLogger::log(TRADEFILE, "\n----------\nData Dumper::ATM Put ID: ",
// //           //     ATMPutOption->symbol_id, "  ATM Put Delta ", ATMPutOption->delta, "  ATM Put Strike ", ATMPutOption->strike, "Exp type ", ATMPutOption->exp_type, "last traded time: ", ATMPutOption->LTT, "\n-------\n");
// //          //std::cout<<"ATM Call Selected "<<ATMCallOption->symbol_id<<" ** "<<ATMCallOption->delta<<" ** "<<ATMCallOption->strike<<"\n";
// //          //std::cout<<"ATM Put Selected "<<ATMPutOption->symbol_id<<" ** "<<ATMPutOption->delta<<" ** "<<ATMPutOption->strike<<"\n";
// //          return std::make_tuple(ATMCallOption, ATMPutOption,this_call_id, this_put_id);
// //  }


// /*
// void DataDumper::greekDumper(uint64_t timestamp)
//   {
// 	static int logger_i = 0;
//         static std::ofstream output(*Tim::Helpers::folder_ptr + "/pranaydata_" + std::to_string(*Tim::Helpers::date_ptr) +"_" + std::to_string(*Tim::Helpers::pid_ptr) +  ".txt");
// 	//output.open(*Tim::Helpers::folder_ptr + "/data_" + std::to_string(*Tim::Helpers::date_ptr) +"_" + std::to_string(*Tim::Helpers::pid_ptr) +  ".txt", std::ofstream::out);
 
//          if ((*Tim::Helpers::timestamp_ptr/(1000*1000*1000LL))%1 != 0) {
//              return;
//          }
// 	if (logger_i==0) {
//              ++logger_i;
//         output << "date_time,date,time,symbol,Option_type,strike_price,ask_px, bid_px, trade_px, gamma, theta, delta, intrinsic_value, vega, oi,spot_px,actual_iv,atm_weighted_iv,volatility, expiry_date,atm_weighted_vega,atm_iv*atm_vega\n";
		
//          }
// 	output.precision(4);
//         output << std::fixed;
// 	MyLogger::log(TRADEFILE, "\n-------\nDataDumper: Time: ",Tim::Helpers::getHumanReadableTime(timestamp),"\n");
// 	std::tuple<Tim::Options*,Tim::Options*,int, int> ATMOptions = findATMOption();
// 	Tim::Options* callATM = std::get<0>(ATMOptions);
// 	std::tuple<double, double> atmWeightedInfo = findATMWeightedInfo(std::get<0>(ATMOptions), std::get<1>(ATMOptions));
// 	double atmWeightedIV = std::get<0>(atmWeightedInfo);
// 	double atmWeightedVega = std::get<1>(atmWeightedInfo);
// 	double atmIVatmVega = atmWeightedIV * atmWeightedVega;
// 	std::stringstream ss_call("");
//          ss_call.precision(4);
//          ss_call << std::fixed;
//          std::string dateTime = std::to_string(date) +" "+ Tim::Helpers::getHumanReadableTime(timestamp);
//          output<< dateTime;
//           std::cout<<"*******Time = "<<Tim::Helpers::getHumanReadableTime(timestamp)<<"********";
//          std::cout<<"**********Dumping Greek Data for "<< callATM->symbol<<" ************\n";
//          ss_call <<","<<date<<","<<Tim::Helpers::getHumanReadableTime(timestamp) <<","<< callATM->symbol<< ","<<callATM->str_opt_type<<"," << callATM->strike <<"," << callATM->ask_px << "," << callATM->bid_px << "," << callATM->trade_px     << "," << callATM->gamma<< "," << callATM->theta << "," << callATM->delta << "," << callATM->intrinsic_value << "," << callATM->vega << "," <<  callATM->OI << "," <<callATM->underlying_price <<"," <<callATM->actual_iv <<","<<atmWeightedIV<<"," <<callATM->volatility<< ","<<callATM->expiry_date<<","<<atmWeightedVega<<","<<atmIVatmVega<<"\n";
//          output <<ss_call.str();
//         std::cout <<","<<date<<","<<Tim::Helpers::getHumanReadableTime(timestamp) <<","<< callATM->symbol<< ","<<callATM->str_opt_type<<"," << callATM->strike <<"," << callATM->ask_px << "," << callATM->bid_px << "," << callATM->trade_px     << "," << callATM->gamma<< "," << callATM->theta << "," << callATM->delta << "," << callATM->intrinsic_value << "," << callATM->vega << "," <<  callATM->OI << "," <<callATM->underlying_price <<"," <<callATM->actual_iv <<","<<atmWeightedIV<<"," <<callATM->volatility<< ","<<callATM->expiry_date<<","<<atmWeightedVega<<","<<atmIVatmVega<<"\n";
// 	std::stringstream ss_put("");
//           ss_put.precision(4);
//           ss_put << std::fixed;
// 	 Tim::Options* putATM = std::get<1>(ATMOptions);
//           //std::string dateTime = std::to_string(date) +" "+ Tim::Helpers::getHumanReadableTime(timestamp);
//           output<< dateTime;
//            std::cout<<"*******Time = "<<Tim::Helpers::getHumanReadableTime(timestamp)<<"********";
//           std::cout<<"**********Dumping Greek Data for "<< putATM->symbol<<" ************\n";
//          ss_put <<","<<date<<","<<Tim::Helpers::getHumanReadableTime(timestamp) <<","<< putATM->symbol<< ","<<putATM->str_opt_type<<"," << putATM->strike <<"," << putATM->ask_px << "," << putATM->bid_px << "," << putATM->trade_px         << "," << putATM->gamma<< "," << putATM->theta << "," << putATM->delta << "," << putATM->intrinsic_value << "," << putATM->vega << "," <<  putATM->OI << "," <<putATM->underlying_price <<"," <<putATM->actual_iv <<","<<atmWeightedIV     <<"," <<putATM->volatility<< ","<<putATM->expiry_date<<","<<atmWeightedVega<<","<<atmIVatmVega<<"\n";
//           output <<ss_put.str();

//     	//writeData(std::get<0>(ATMOptions), timestamp, atmWeightedIV);
// 	//writeData(std::get<1>(ATMOptions),  timestamp, atmWeightedIV);
// 	//output.close();

// }


// */

// }

// // void DataDumper::alldataDumper(uint64_t timestamp){
// //         static int logger_i = 0;
// //         if ((*Tim::Helpers::timestamp_ptr/(1000*1000*1000LL))%1 != 0) {
// //              return;
// //         }
// //         std::string fname = *Tim::Helpers::folder_ptr + "/sahaj_wtd_ivdata_" + std::to_string(*Tim::Helpers::date_ptr) +"_" + std::to_string(*Tim::Helpers::pid_ptr) +  ".csv";
// //         static std::ofstream output;
// //         output.open(fname, std::ios_base::app); // append instead of overwrite
// //         std::string out = "";
// //         if (logger_i==0) {
// //                 ++logger_i;
// //                 out += "date_time,date,time,symbol,type,vega,iv,denom,vega_weighted_iv,";

// //                 for(auto it:expiryTU){
// //                         // out+=createDefaultColumnNames(it.first, strike_range);
// //                 }
// //                 // out+="symbol,Option_type,strike_price,ask_px, bid_px, trade_px, gamma, theta, delta, intrinsic_value, vega, oi,spot_px,actual_iv,atm_weighted_iv,volatility, expiry_date,atm_weighted_vega,atm_iv*atm_vega,";

// //                 out.pop_back();
// //                 out+="\n";
// //                 std::cout << out << std::endl;
// //                 if (output.is_open())
// //                         output << out;
// //         }
// //         output.precision(4);
// //         output << std::fixed;

// //         std::string date_str = std::to_string(date);
// //         std::string dateTime = date_str +" "+ Tim::Helpers::getHumanReadableTime(timestamp);
        
// //         // std::cout << output.str() << std::endl;
// //         MyLogger::log(TRADEFILE, "\n-------\nDataDumper: Time: ",Tim::Helpers::getHumanReadableTime(timestamp),"\n");
// //         for(auto it:expiryTU){
// //                 double vc_sum = 0, vp_sum = 0;
// //                 Tim::Options* opt;
// //                 for(auto itr:it.second){
// //                         opt = itr.second->opt;
// //                         if (opt->delta_participation_ratio > 0.3){
                                
// //                                 if (opt->str_opt_type == 'c'){
// //                                         vc_sum+= opt->vega;
// //                                 }
// //                                 if (opt->str_opt_type == 'p'){
// //                                         vp_sum+=opt->vega;
// //                                 }
// //                         }
// //                 }
// //                 for(auto itr:it.second){
// //                         std::stringstream ss("");
// //                         ss.precision(4);
// //                         ss << std::fixed;
// //                         opt = itr.second->opt;
// //                         double wt = 0;
// //                         if (opt->delta_participation_ratio > 0.3){
                                
// //                                 if (opt->str_opt_type == 'c'){
// //                                         wt = opt->vega/vc_sum;
// //                                         ss << dateTime << "," << date << "," << Tim::Helpers::getHumanReadableTime(timestamp) << "," << opt->symbol << "," << opt->str_opt_type<< "," << opt->vega<< "," <<opt->volatility << ","<< vc_sum << "," << opt->volatility * wt << "\n";

// //                                 }
// //                                 if (opt->str_opt_type == 'p'){
// //                                         wt = opt->vega/vp_sum;
// //                                         ss << dateTime << "," << date << "," << Tim::Helpers::getHumanReadableTime(timestamp) << "," << opt->symbol << "," << opt->str_opt_type<< "," << opt->vega<< "," <<opt->volatility << ","<< vp_sum << "," << opt->volatility * wt << "\n";

// //                                 }
// //                                 output << ss.str();
// //                         }
                        
// //                 }
// //         }
// //         //         std::tuple<std::vector<std::pair<std::string, double>>, 
// //         // std::vector<std::pair<std::string, double>>, 
// //         // std::vector<std::pair<std::string, double>>, 
// //         // std::vector<std::pair<std::string, double>>> output = rankStrikesbySpotDiff(it.first);
// //         //         for(auto itr:it.second){
// //         //                 std::stringstream ss("");
// //         //                 ss.precision(4);
// //         //                 ss << std::fixed;
// //         //                 std::string row = "";
// //         //                 Tim::Options* opt = itr.second->opt;
// //         //                 std::string otmitm = "";
// //         //                 if (opt->str_opt_type == 'c'){
// //         //                         if (opt->strike <= opt->underlying_price)
// //         //                                 otmitm = "itm";
// //         //                         else
// //         //                                 otmitm = "otm";
// //         //                 }
// //         //                 if (opt->str_opt_type == 'p'){
// //         //                         if (opt->strike >= opt->underlying_price)
// //         //                                 otmitm = "itm";
// //         //                         else
// //         //                                 otmitm = "otm";
// //         //                 }
// //         //                 std::cout << "Rolling " << opt->delta_participation_ratio << "\n";
// //         //                 ss  << opt->symbol << "," << opt->strike << "," << opt->underlying_price << "," << opt->str_opt_type << "," << otmitm << "," << opt->gamma << "," << opt->theta << "," << opt->delta << "," << opt->vega << "," << opt->volatility << "\n";
// //         //                 output << dateTime << "," << date << "," << Tim::Helpers::getHumanReadableTime(timestamp) << "," << ss.str();
// //         //         } 
// //         // }
// //         // std::stringstream ss_call("");
// //         // ss_call.precision(4);
// //         // ss_call << std::fixed;

// //         // std::stringstream ss_put("");
// //         // ss_put.precision(4);
// //         // ss_put << std::fixed;
// //         // for(auto it:expiryTU){
// //         //         // rankStrikesbySpotDiff(it.first);
// //         //         // if((*dump_params.strike_range==0){
// //         //         //         std::string ATMRow = getATMRow(it);
// //         //         //         output << ATMRow;
// //         //         // }
// //         //         std::tuple<Tim::Options*,Tim::Options*,int, int> ATMOptions = findATMOption(it.second);
// //         //         Tim::Options* callATM = std::get<0>(ATMOptions);
// //         //         Tim::Options* putATM = std::get<1>(ATMOptions);
// //         //         std::tuple<double, double> atmWeightedInfo = findATMWeightedInfo(std::get<0>(ATMOptions), std::get<1>(ATMOptions), it.first);
// //         //         double atmWeightedIV = std::get<0>(atmWeightedInfo);
// //         //         double atmWeightedVega = std::get<1>(atmWeightedInfo);
// //         //         double atmIVatmVega = atmWeightedIV * atmWeightedVega;
// //         //         std::cout<<"*******Time = "<<Tim::Helpers::getHumanReadableTime(timestamp)<<"********";
// //         //         std::cout<<"**********Dumping Greek Data for "<< callATM->symbol<<" ************\n";
// //         //         ss_call << callATM->symbol<< "," << callATM->str_opt_type << "," 
// //         //         << callATM->strike << "," 
// //         //         << callATM->ask_px << "," << callATM->bid_px << "," 
// //         //         << callATM->trade_px << "," 
// //         //         // << callATM->gamma<< "," << callATM->theta << "," 
// //         //         // << callATM->delta << "," << callATM->intrinsic_value << "," << callATM->vega << "," 
// //         //         // <<  callATM->OI << "," 
// //         //         // << callATM->underlying_price << "," << callATM->actual_iv << ","
// //         //         << atmWeightedIV << "," ;
// //         //         // << callATM->volatility << "," << callATM->expiry_date << "," 
// //         //         // << atmWeightedVega << "," << atmIVatmVega << ",";
// //         //         std::cout<<"*******Time = "<<Tim::Helpers::getHumanReadableTime(timestamp)<<"********";
// //         //         std::cout<<"**********Dumping Greek Data for "<< putATM->symbol<<" ************\n";
// //         //         ss_put  << putATM->symbol << "," << putATM->str_opt_type  << "," 
// //         //         << putATM->strike  << "," 
// //         //         << putATM->ask_px  << "," << putATM->bid_px  << "," 
// //         //         << putATM->trade_px  << "," 
// //         //         // << putATM->gamma << "," << putATM->theta  << "," 
// //         //         // << putATM->delta  << "," << putATM->intrinsic_value  << "," << putATM->vega  << "," 
// //         //         // <<  putATM->OI  << "," 
// //         //         // << putATM->underlying_price  << "," << putATM->actual_iv  << ","
// //         //         << atmWeightedIV 
// //         //         << ","; 
// //         //         // << putATM->volatility  << "," << putATM->expiry_date  << "," 
// //         //         // << atmWeightedVega << "," << atmIVatmVega << ",";
// //         //         // row_cp += ss_call.str() + ss_put.str();

// //         //         std::cout << row_cp << "\n";
// //         //         std::cout << "Rolling shit \n" << callATM->delta_participation_ratio << ",," << callATM->rolling_ttq << "\n";
// //         // }
// //         output.close();
// // }

// // void DataDumper::alldataDumper(uint64_t timestamp){
// //         static int logger_i = 0;
// //         if ((*Tim::Helpers::timestamp_ptr/(1000*1000*1000LL))%1 != 0) {
// //              return;
// //         }
// //         std::string fname = *Tim::Helpers::folder_ptr + "/sahaj_data_iv1_" + std::to_string(*Tim::Helpers::date_ptr) +"_" + std::to_string(*Tim::Helpers::pid_ptr) +  ".csv";
// //         static std::ofstream output;
// //         output.open(fname, std::ios_base::app); // append instead of overwrite
// //         std::string out = "";
// //         if (logger_i==0) {
// //                 ++logger_i;
// //                 out += "date_time,date,time,";

// //                 // for(auto it:expiryTU){
// //                 //         // out+=createDefaultColumnNames(it.first, strike_range);
// //                 // }
// //                 out+="symbol,Option_type,strike_price,ask_px, bid_px, trade_px, gamma, theta, delta, intrinsic_value, vega, oi,spot_px,actual_iv,atm_weighted_iv,volatility, expiry_date,atm_weighted_vega,atm_iv*atm_vega,tte";
// //                 out+="symbol,Option_type,strike_price,ask_px, bid_px, trade_px, gamma, theta, delta, intrinsic_value, vega, oi,spot_px,actual_iv,atm_weighted_iv,volatility, expiry_date,atm_weighted_vega,atm_iv*atm_vega,tte";

// //                 // out+="symbol,Option_type,strike_price,ask_px, bid_px, trade_px,atm_weighted_iv,";
// //                 // out+="symbol,Option_type,strike_price,ask_px, bid_px, trade_px,atm_weighted_iv,";
// //                 out.pop_back();
// //                 out+="\n";
// //                 std::cout << out << std::endl;
// //                 // if (output.is_open())
// //                         // output << out;
// //         }
// //         // output.precision(4);
// //         // output << std::fixed;
        

// //         std::stringstream ss_call("");
// //         ss_call.precision(4);
// //         ss_call << std::fixed;

// //         std::stringstream ss_put("");
// //         ss_put.precision(4);
// //         ss_put << std::fixed;

// //         std::string date_str = std::to_string(date);
// //         std::string dateTime = date_str +" "+ Tim::Helpers::getHumanReadableTime(timestamp);

       
// //         // output<< dateTime << "," << date << "," << Tim::Helpers::getHumanReadableTime(timestamp) << ",";
// //         std::string row_cp = "";
// //         // std::cout << output.str() << std::endl;
// //         MyLogger::log(TRADEFILE, "\n-------\nDataDumper: Time: ",Tim::Helpers::getHumanReadableTime(timestamp),"\n");
// //         for(auto it:expiryTU){
// //                 // rankStrikesbySpotDiff(it.first);
// //                 // if((*dump_params.strike_range==0){
// //                 //         std::string ATMRow = getATMRow(it);
// //                 //         output << ATMRow;
// //                 // }
// //                 std::tuple<Tim::Options*,Tim::Options*,int, int> ATMOptions = findATMOption(it.second);
// //                 Tim::Options* callATM = std::get<0>(ATMOptions);
// //                 Tim::Options* putATM = std::get<1>(ATMOptions);
// //                 std::tuple<double, double> atmWeightedInfo = findATMWeightedInfo(std::get<0>(ATMOptions), std::get<1>(ATMOptions), it.first);
// //                 double atmWeightedIV = std::get<0>(atmWeightedInfo);
// //                 double atmWeightedVega = std::get<1>(atmWeightedInfo);
// //                 double atmIVatmVega = atmWeightedIV * atmWeightedVega;
// //                 std::cout<<"*******Time = "<<Tim::Helpers::getHumanReadableTime(timestamp)<<"********";
// //                 std::cout<<"**********Dumping Greek Data for "<< callATM->symbol<<" ************\n";
// //                 ss_call << callATM->symbol<< "," << callATM->str_opt_type << "," 
// //                 << callATM->strike << "," 
// //                 << callATM->ask_px << "," << callATM->bid_px << "," 
// //                 << callATM->trade_px << "," 
// //                 << callATM->gamma<< "," << callATM->theta << "," 
// //                 << callATM->delta << "," << callATM->intrinsic_value << "," << callATM->vega << "," 
// //                 <<  callATM->OI << "," 
// //                 << callATM->underlying_price << "," << callATM->actual_iv << ","
// //                 << atmWeightedIV << ","
// //                 << callATM->volatility << "," << callATM->expiry_date << "," 
// //                 << atmWeightedVega << "," << atmIVatmVega << ","<< callATM->time_to_expiry << ",";
// //                 std::cout<<"*******Time = "<<Tim::Helpers::getHumanReadableTime(timestamp)<<"********";
// //                 std::cout<<"**********Dumping Greek Data for "<< putATM->symbol<<" ************\n";
// //                 ss_put  << putATM->symbol << "," << putATM->str_opt_type  << "," 
// //                 << putATM->strike  << "," 
// //                 << putATM->ask_px  << "," << putATM->bid_px  << "," 
// //                 << putATM->trade_px  << "," 
// //                 << putATM->gamma << "," << putATM->theta  << "," 
// //                 << putATM->delta  << "," << putATM->intrinsic_value  << "," << putATM->vega  << "," 
// //                 <<  putATM->OI  << "," 
// //                 << putATM->underlying_price  << "," << putATM->actual_iv  << ","
// //                 << atmWeightedIV 
// //                 << ","
// //                 << putATM->volatility  << "," << putATM->expiry_date  << "," 
// //                 << atmWeightedVega << "," << atmIVatmVega << "," << callATM->time_to_expiry << ",";
// //                 row_cp += ss_call.str() + ss_put.str();

// //                 std::cout << row_cp << "\n";
// //                 std::cout << "Rolling shit \n" << callATM->delta_participation_ratio << "    " << callATM->rolling_ttq << "\n";
// //         }
// //         row_cp.pop_back();
        
// //         output << row_cp << "\n";
// //         output.close();
// // }


// // std::tuple<std::vector<std::pair<std::string, double>>, 
// //         std::vector<std::pair<std::string, double>>, 
// //         std::vector<std::pair<std::string, double>>, 
// //         std::vector<std::pair<std::string, double>>>  DataDumper::rankStrikesbySpotDiff(Tim::ExpiryType exp)
// // {

// //         std::map<std::string, Tim::Instrument*> TU = expiryTU[exp];
// //         std::vector<std::pair<Tim::Options*, double>> ITMCallSymbols, ITMPutSymbols, OTMCallSymbols, OTMPutSymbols;
// //         for(auto itr:TU){
// //                 double diff = double(itr.second->opt->strike) - (itr.second->opt->underlying_price);
// //                 std::pair<Tim::Options*, double> sympair;
// //                 sympair.first = itr.second->opt;
// //                 sympair.second = diff;
// //                 if (diff<=0){
// //                         if(itr.second->opt->opt_type==Tim::OptionType::Call)
// //                                 ITMCallSymbols.push_back(sympair);
// //                         else
// //                                 ITMPutSymbols.push_back(sympair);
// //                 }
                          
// //                 else{
// //                         if(itr.second->opt->opt_type==Tim::OptionType::Call)
// //                                 OTMCallSymbols.push_back(sympair);
// //                         else
// //                                 OTMPutSymbols.push_back(sympair);
// //                 }
                        
// //         }
// //         auto cmp = [] (std::pair<std::string, double> & p1, std::pair<std::string, double>& p2) -> bool
// //         {
// //                 return p1.second > p2.second;
// //         };
// //         std::sort(ITMCallSymbols.begin(), ITMCallSymbols.end(), cmp);
// //         std::sort(ITMPutSymbols.begin(), ITMPutSymbols.end(), cmp);
// //         std::sort(OTMCallSymbols.begin(), OTMCallSymbols.end(), cmp);
// //         std::sort(OTMPutSymbols.begin(), OTMPutSymbols.end(), cmp);

// //         std::cout << "Printing ITM Symbols and diffs\n";
// //         for(auto pr:ITMCallSymbols){
// //                 std::cout << pr << ": " << std::endl;
// //         }
// //         return make_tuple(ITMCallSymbols, ITMPutSymbols, OTMCallSymbols, OTMPutSymbols);

// // }

//void DataDumper::shockDumper(uint64_t timestamp){
//        std::cout << "Timestamp: " << Tim::Helpers::getHumanReadableTime(timestamp) << std::endl;
//        static int logger_i = 0;
//        if ((*Tim::Helpers::timestamp_ptr/(1000*1000*1000LL))%1 != 0) {
//             return;
//        }
//        std::string fname = "/home/pranka/work/Strat_Init/Strat/temp/new_nf_up_lib_0508.csv";
//        static std::ofstream output;
//        output.open(fname, std::ios_base::app); // append instead of overwrite
//        std::string out = "";
//        std::stringstream ss("");
//
//        ss.precision(5);
//        ss << std::fixed;
//        if (logger_i==0) {
//                logger_i++;
//                out += "time,machine,submachine,symbol,pos,quantity,timestamp,spot,price,strike,tte,type,iv,delta,gamma,rho,vega,theta,spot_spot_0.02,price_spot_0.02,strike_spot_0.02,shock_spot_0.02,tte_spot_0.02,type_spot_0.02,iv_spot_0.02,delta_spot_0.02,gamma_spot_0.02,rho_spot_0.02,vega_spot_0.02,theta_spot_0.02,spot_spot_0.05,price_spot_0.05,strike_spot_0.05,shock_spot_0.05,tte_spot_0.05,type_spot_0.05,iv_spot_0.05,delta_spot_0.05,gamma_spot_0.05,rho_spot_0.05,vega_spot_0.05,theta_spot_0.05,spot_spot_0.1,price_spot_0.1,strike_spot_0.1,shock_spot_0.1,tte_spot_0.1,type_spot_0.1,iv_spot_0.1,delta_spot_0.1,gamma_spot_0.1,rho_spot_0.1,vega_spot_0.1,theta_spot_0.1,spot_spot_0.15,price_spot_0.15,strike_spot_0.15,shock_spot_0.15,tte_spot_0.15,type_spot_0.15,iv_spot_0.15,delta_spot_0.15,gamma_spot_0.15,rho_spot_0.15,vega_spot_0.15,theta_spot_0.15,spot_iv_0.02,price_iv_0.02,strike_iv_0.02,shock_iv_0.02,tte_iv_0.02,type_iv_0.02,iv_iv_0.02,delta_iv_0.02,gamma_iv_0.02,rho_iv_0.02,vega_iv_0.02,theta_iv_0.02,spot_iv_0.05,price_iv_0.05,strike_iv_0.05,shock_iv_0.05,tte_iv_0.05,type_iv_0.05,iv_iv_0.05,delta_iv_0.05,gamma_iv_0.05,rho_iv_0.05,vega_iv_0.05,theta_iv_0.05";
//                out+="\n";
//                std::cout << out << std::endl;
//                output << out;
//        }
//        output.precision(5);
//        output << std::fixed;
//        // std::map<std::string, Tim::Instrument*> TU_s = expiryTU[Tim::ExpiryType::Weekly];
//        struct tm t2 = {0};
//        t2.tm_year = Tim::Helpers::getYear(20210805);
//        t2.tm_mon = Tim::Helpers::getMonth(20210805);
//        t2.tm_mday = Tim::Helpers::getDay(20210805);
//
//        double s_pnl;
//        for(int i = 0; i < csv_data.size(); i++){
//                t2.tm_hour = std::stoi(csv_data[i][0])/100;
//                t2.tm_min = std::stoi(csv_data[i][0]) % 100;
//                t2.tm_sec = 0;
//                std::time_t tSE = mktime(&t2);
//                uint64_t rowtime = long(tSE)*E9;
//                Tim::Options* opt;
//                Tim::Options* opt_opp;
//                std::string sym = csv_data[i][3];
//                opt = unHand->TradeUniverse[sym]->opt;
//                double s_pnl;
//                if (rowtime==model_starttime){
//                        rowtime+=60*long(E9);
//                }
//                alphagrep::shock::ShockConfig cfg2(d, exc);
//                cfg2.setUnderlyingMovesList(uMoves);
//                cfg2.setVolMovesList(vMoves);
//                cfg2.setRiskFreeRate(0);
//                agShock.init(cfg2);
//                agShock.onMdUpdate("NIFTY50", opt->underlying_price/100, timestamp/(1));
//                // std::cout << ss.str() << std::endl;
//                if (rowtime >= timestamp && rowtime < timestamp + long(E9)){
//                        std::string ss_str = "";
//                        ss << csv_data[i][0] << "," << csv_data[i][1] << "," << csv_data[i][2] << "," <<csv_data[i][3] << "," << csv_data[i][4] << ","<< csv_data[i][5] << ","<< rowtime << "," << opt->underlying_price << "," << opt->mid_px << "," << opt->strike << "," << opt->time_to_expiry << "," << opt->str_opt_type << "," << opt->volatility << "," << opt->delta << "," << opt->gamma << "," << opt->rho << "," << opt->vega << "," <<opt->theta<< ",";
//                        agShock.onMdUpdate(sym, opt->mid_px, rowtime/(1));
//                        agShock.onTradeUpdate(sym, std::stoi(csv_data[i][5]) > 0 ?'B':'S', (opt->mid_px), std::abs(std::stoi(csv_data[i][5])) , rowtime);
//                        auto s = agShock.getShockRisk();
//                        s.printScenarios();
//                        for(int k = 0; k < spot_moves.size(); k++){
//                                // std::string uMoves = spot_moves[k], vMoves = "0";
//                                double sm = std::stod(spot_moves[k]);
//                                double ivm = 0.0;
//                                std::cout << "Spot Move: " << sm << " and VMove is " << ivm << std::endl;
//                                s_pnl = s.getScenarioPnl((sm), (ivm))*100;
//                                std::cout << s_pnl << std::endl;
//                                double newspot = (1+sm)* opt->underlying_price;
//                                Tim::GreekCalculator G(0);
//                                Tim::Options n_opt;
//                                n_opt = *opt;
//                                Tim::Options* new_opt = &n_opt;
//                                new_opt->underlying_price = (newspot);
//                                double new_mid_px = G.findBSPrice(new_opt, newspot, 0);
//                                new_opt->mid_px = new_mid_px;
//                                // auto start = std::chrono::high_resolution_clock::now();
//                                Tim::Greeks g_new = G.getGreeks(new_opt, newspot, 0);
//                                // auto stop = std::chrono::high_resolution_clock::now();
//                                // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
//                                // std::cout << "TIMEEEEEE: " << duration.count() << std::endl;
//                                new_opt->updateAllGreeks(g_new);
//                                // double s_pnl = (new_mid_px - opt->mid_px)* double(std::stoi(csv_data[i][4]));
//                                ss << new_opt->underlying_price<< "," << new_opt->mid_px<< "," << new_opt->strike << "," << s_pnl << "," << new_opt->time_to_expiry<< "," << new_opt->str_opt_type<< "," << new_opt->volatility<< "," << new_opt->delta<< "," << new_opt->gamma<< "," << new_opt->rho<< "," << new_opt->vega<< "," << new_opt->theta << ",";
//                        }
//                        for(int k = 0; k < iv_moves.size(); k++){
//                                double sm = 0;
//                                double ivm = std::stod(iv_moves[k]);
//                                // agShock.onMdUpdate("BANKNIFTY", opt->underlying_price/100, timestamp/(1));
//                                // agShockMap[sym].onMdUpdate(sym, opt->mid_px, timestamp/(1));
//                                // agShockMap[sym].onTradeUpdate(sym, std::stoi(csv_data[i][5]) > 0 ?'B':'S', (opt->mid_px), std::abs(std::stoi(csv_data[i][5])) , timestamp);
//                                // auto s = agShockMap[sym].getShockRisk();
//                                double newiv = (1+ivm)*opt->volatility;
//                                double iv_diff = (newiv - opt->volatility);
//                                std::cout << "IV_Diff : " << iv_diff << "\n";
//                                s_pnl = s.getScenarioPnl((sm), iv_diff)*100;
//
//                                Tim::GreekCalculator G(0);
//                                Tim::Options n_opt;
//                                n_opt = *opt;
//                                Tim::Options* new_opt = &n_opt;
//                                double new_mid_px = G.findBSPrice(new_opt, opt->underlying_price, ivm*100);
//                                new_opt->mid_px = new_mid_px;
//
//                                Tim::Greeks g_new = G.getGreeks(new_opt, opt->underlying_price, 0);
//                                new_opt->updateAllGreeks(g_new);
//                                // double s_pnl = (new_mid_px - opt->mid_px)* double(std::stoi(csv_data[i][4]));
//                                ss << new_opt->underlying_price<< "," << new_opt->mid_px<< "," << new_opt->strike << "," << s_pnl << "," << new_opt->time_to_expiry<< "," << new_opt->str_opt_type<< "," << new_opt->volatility<< "," << new_opt->delta<< "," << new_opt->gamma<< "," << new_opt->rho<< "," << new_opt->vega<< "," << new_opt->theta << ",";
//
//                        }
//                        ss_str = ss.str();
//                        ss_str.pop_back();
//                        ss.str(std::string());
//                        output << ss_str+"\n";
//                        std::cout << ss_str << std::endl;
//                }
//                else{
//                        continue;
//                }
//
//        }
//        output.close();
//}
//void DataDumper::shockDumper2(uint64_t timestamp){
////         static int logger_i2 = 0;
////         if ((*Tim::Helpers::timestamp_ptr/(1000*1000*1000LL))%1 != 0) {
////              return;
////         }
////         std::string fname = "/home/pranka/work/Strat_Init/Strat/temp/new_nf_down_lib_0508.csv";
////         static std::ofstream output2;
////         output2.open(fname, std::ios_base::app); // append instead of overwrite
////         std::string out = "";
////         std::stringstream ss("");
//
////         ss.precision(5);
////         ss << std::fixed;
////         if (logger_i2==0) {
////                 logger_i2++;
////                 out += "time,machine,submachine,symbol,pos,quantity,timestamp,spot,price,strike,tte,type,iv,delta,gamma,rho,vega,theta,spot_spot_0.02,price_spot_0.02,strike_spot_0.02,shock_spot_0.02,tte_spot_0.02,type_spot_0.02,iv_spot_0.02,delta_spot_0.02,gamma_spot_0.02,rho_spot_0.02,vega_spot_0.02,theta_spot_0.02,spot_spot_0.05,price_spot_0.05,strike_spot_0.05,shock_spot_0.05,tte_spot_0.05,type_spot_0.05,iv_spot_0.05,delta_spot_0.05,gamma_spot_0.05,rho_spot_0.05,vega_spot_0.05,theta_spot_0.05,spot_spot_0.1,price_spot_0.1,strike_spot_0.1,shock_spot_0.1,tte_spot_0.1,type_spot_0.1,iv_spot_0.1,delta_spot_0.1,gamma_spot_0.1,rho_spot_0.1,vega_spot_0.1,theta_spot_0.1,spot_spot_0.15,price_spot_0.15,strike_spot_0.15,shock_spot_0.15,tte_spot_0.15,type_spot_0.15,iv_spot_0.15,delta_spot_0.15,gamma_spot_0.15,rho_spot_0.15,vega_spot_0.15,theta_spot_0.15,spot_iv_0.02,price_iv_0.02,strike_iv_0.02,shock_iv_0.02,tte_iv_0.02,type_iv_0.02,iv_iv_0.02,delta_iv_0.02,gamma_iv_0.02,rho_iv_0.02,vega_iv_0.02,theta_iv_0.02,spot_iv_0.05,price_iv_0.05,strike_iv_0.05,shock_iv_0.05,tte_iv_0.05,type_iv_0.05,iv_iv_0.05,delta_iv_0.05,gamma_iv_0.05,rho_iv_0.05,vega_iv_0.05,theta_iv_0.05";
////                 out+="\n";
////                 std::cout << out << std::endl;
////                 output2 << out;
////         }
////         output2.precision(5);
////         output2 << std::fixed;
////         std::map<std::string, Tim::Instrument*> TU_s = expiryTU[Tim::ExpiryType::Weekly];
////         struct tm t2 = {0};
////         t2.tm_year = Tim::Helpers::getYear(20210805);
////         t2.tm_mon = Tim::Helpers::getMonth(20210805);
////         t2.tm_mday = Tim::Helpers::getDay(20210805);
//
//// double s_pnl;
////         for(int i = 0; i < csv_data.size(); i++){
////                 t2.tm_hour = std::stoi(csv_data[i][0])/100;
////                 t2.tm_min = std::stoi(csv_data[i][0]) % 100;
////                 t2.tm_sec = 0;
////                 std::time_t tSE = mktime(&t2);
////                 uint64_t rowtime = long(tSE)*E9;
////                 Tim::Options* opt;
////                 Tim::Options* opt_opp;
////                 std::string sym = csv_data[i][3];
////                 opt = unHand->TradeUniverse[sym]->opt;
////                 double s_pnl;
////                 if (rowtime==model_starttime){
////                         rowtime+=60*long(E9);
////                 }
////                 agShock.onMdUpdate("NIFTY", opt->underlying_price/100, timestamp/(1));
////                 // std::cout << ss.str() << std::endl;
////                 if (rowtime >= timestamp && rowtime < timestamp + long(E9)){
////                         std::string ss_str = "";
////                         ss << csv_data[i][0] << "," << csv_data[i][1] << "," << csv_data[i][2] << "," <<csv_data[i][3] << "," << csv_data[i][4] << ","<< csv_data[i][5] << ","<< rowtime << "," << opt->underlying_price << "," << opt->mid_px << "," << opt->strike << "," << opt->time_to_expiry << "," << opt->str_opt_type << "," << opt->volatility << "," << opt->delta << "," << opt->gamma << "," << opt->rho << "," << opt->vega << "," <<opt->theta<< ",";
////                         agShock.onMdUpdate(sym, opt->mid_px, timestamp/(1));
////                         agShock.onTradeUpdate(sym, std::stoi(csv_data[i][5]) > 0 ?'B':'S', (opt->mid_px), std::abs(std::stoi(csv_data[i][5])) , timestamp);
////                         auto s = agShock.getShockRisk();
////                         for(int k = 0; k < spot_moves2.size(); k++){
////                                 // std::string uMoves = spot_moves[k], vMoves = "0";
////                                 double sm = std::stod(spot_moves2[k]);
////                                 double ivm = 0.0;
////                                 s_pnl = s.getScenarioPnl((sm), (ivm))*100;
////                                 double newspot = (1+sm)* opt->underlying_price;
////                                 Tim::GreekCalculator G(0);
////                                 Tim::Options n_opt;
////                                 n_opt = *opt;
////                                 Tim::Options* new_opt = &n_opt;
////                                 new_opt->underlying_price = (newspot);
////                                 double new_mid_px = G.findBSPrice(new_opt, newspot, 0);
////                                 new_opt->mid_px = new_mid_px;
////                                 // auto start = std::chrono::high_resolution_clock::now();
////                                 Tim::Greeks g_new = G.getGreeks(new_opt, newspot, 0);
////                                 // auto stop = std::chrono::high_resolution_clock::now();
////                                 // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
////                                 // std::cout << "TIMEEEEEE: " << duration.count() << std::endl;
////                                 new_opt->updateAllGreeks(g_new);
////                                 // double s_pnl = (new_mid_px - opt->mid_px)* double(std::stoi(csv_data[i][4]));
////                                 ss << new_opt->underlying_price<< "," << new_opt->mid_px<< "," << new_opt->strike << "," << s_pnl << "," << new_opt->time_to_expiry<< "," << new_opt->str_opt_type<< "," << new_opt->volatility<< "," << new_opt->delta<< "," << new_opt->gamma<< "," << new_opt->rho<< "," << new_opt->vega<< "," << new_opt->theta << ",";
////                         }
////                         for(int k = 0; k < iv_moves.size(); k++){
////                                 double sm = 0;
////                                 double ivm = std::stod(iv_moves[k]);
////                                 // agShock.onMdUpdate("BANKNIFTY", opt->underlying_price/100, timestamp/(1));
////                                 // agShockMap[sym].onMdUpdate(sym, opt->mid_px, timestamp/(1));
////                                 // agShockMap[sym].onTradeUpdate(sym, std::stoi(csv_data[i][5]) > 0 ?'B':'S', (opt->mid_px), std::abs(std::stoi(csv_data[i][5])) , timestamp);
////                                 // auto s = agShockMap[sym].getShockRisk();
////                                 double newiv = (1+ivm)*opt->volatility;
////                                 double iv_diff = (newiv - opt->volatility);
////                                 std::cout << "IV_Diff : " << iv_diff << "\n";
////                                 s_pnl = s.getScenarioPnl((sm), iv_diff)*100;
//
////                                 Tim::GreekCalculator G(0);
////                                 Tim::Options n_opt;
////                                 n_opt = *opt;
////                                 Tim::Options* new_opt = &n_opt;
////                                 double new_mid_px = G.findBSPrice(new_opt, opt->underlying_price, ivm*100);
////                                 new_opt->mid_px = new_mid_px;
//
////                                 Tim::Greeks g_new = G.getGreeks(new_opt, opt->underlying_price, 0);
////                                 new_opt->updateAllGreeks(g_new);
////                                 // double s_pnl = (new_mid_px - opt->mid_px)* double(std::stoi(csv_data[i][4]));
////                                 ss << new_opt->underlying_price<< "," << new_opt->mid_px<< "," << new_opt->strike << "," << s_pnl << "," << new_opt->time_to_expiry<< "," << new_opt->str_opt_type<< "," << new_opt->volatility<< "," << new_opt->delta<< "," << new_opt->gamma<< "," << new_opt->rho<< "," << new_opt->vega<< "," << new_opt->theta << ",";
//
////                         }
////                         ss_str = ss.str();
////                         ss_str.pop_back();
////                         ss.str(std::string());
////                         output2 << ss_str+"\n";
////                         std::cout << ss_str << std::endl;
////                 }
////                 else{
////                         continue;
////                 }
//
////         }
//
////         output2.close();
//}

//void DataDumper::shockDumper4(uint64_t timestamp){
//        std::cout << "Timestamp: " << Tim::Helpers::getHumanReadableTime(timestamp) << std::endl;
//        static int logger_i2 = 0;
//        if ((*Tim::Helpers::timestamp_ptr/(1000*1000*1000LL))%1 != 0) {
//             return;
//        }
//        std::string fname = "/home/pranka/work/Strat_Init/Strat/temp/new_nf_down_0508_.csv";
//        static std::ofstream output2;
//        output2.open(fname, std::ios_base::app); // append instead of overwrite
//        std::string out = "";
//        std::stringstream ss("");
//
//        ss.precision(5);
//        ss << std::fixed;
//        if (logger_i2==0) {
//                logger_i2++;
//                out += "time,machine,submachine,symbol,pos,quantity,timestamp,spot,price,strike,tte,type,iv,delta,gamma,rho,vega,theta,spot_spot_0.02,price_spot_0.02,strike_spot_0.02,shock_spot_0.02,tte_spot_0.02,type_spot_0.02,iv_spot_0.02,delta_spot_0.02,gamma_spot_0.02,rho_spot_0.02,vega_spot_0.02,theta_spot_0.02,spot_spot_0.05,price_spot_0.05,strike_spot_0.05,shock_spot_0.05,tte_spot_0.05,type_spot_0.05,iv_spot_0.05,delta_spot_0.05,gamma_spot_0.05,rho_spot_0.05,vega_spot_0.05,theta_spot_0.05,spot_spot_0.1,price_spot_0.1,strike_spot_0.1,shock_spot_0.1,tte_spot_0.1,type_spot_0.1,iv_spot_0.1,delta_spot_0.1,gamma_spot_0.1,rho_spot_0.1,vega_spot_0.1,theta_spot_0.1,spot_spot_0.15,price_spot_0.15,strike_spot_0.15,shock_spot_0.15,tte_spot_0.15,type_spot_0.15,iv_spot_0.15,delta_spot_0.15,gamma_spot_0.15,rho_spot_0.15,vega_spot_0.15,theta_spot_0.15,spot_iv_0.02,price_iv_0.02,strike_iv_0.02,shock_iv_0.02,tte_iv_0.02,type_iv_0.02,iv_iv_0.02,delta_iv_0.02,gamma_iv_0.02,rho_iv_0.02,vega_iv_0.02,theta_iv_0.02,spot_iv_0.05,price_iv_0.05,strike_iv_0.05,shock_iv_0.05,tte_iv_0.05,type_iv_0.05,iv_iv_0.05,delta_iv_0.05,gamma_iv_0.05,rho_iv_0.05,vega_iv_0.05,theta_iv_0.05";
//                out+="\n";
//                std::cout << out << std::endl;
//                output2 << out;
//        }
//        output2.precision(5);
//        output2 << std::fixed;
//        std::map<std::string, Tim::Instrument*> TU_s = expiryTU[Tim::ExpiryType::Weekly];
//        struct tm t2 = {0};
//        t2.tm_year = Tim::Helpers::getYear(20210805);
//        t2.tm_mon = Tim::Helpers::getMonth(20210805);
//        t2.tm_mday = Tim::Helpers::getDay(20210805);
//
//        double s_pnl;
//        for(int i = 0; i < csv_data.size(); i++){
//                t2.tm_hour = std::stoi(csv_data[i][0])/100;
//                t2.tm_min = std::stoi(csv_data[i][0]) % 100;
//                t2.tm_sec = 0;
//                std::time_t tSE = mktime(&t2);
//                uint64_t rowtime = long(tSE)*E9;
//                Tim::Options* opt;
//                Tim::Options* opt_opp;
//                std::string sym = csv_data[i][3];
//                opt = unHand->TradeUniverse[sym]->opt;
//                if (rowtime==model_starttime){
//                        rowtime+=60*long(E9);
//                }
//
//                // std::cout << ss.str() << std::endl;
//                if (rowtime >= timestamp && rowtime < timestamp + long(E9)){
//                        std::string ss_str = "";
//                        ss << csv_data[i][0] << "," << csv_data[i][1] << "," << csv_data[i][2] << "," <<csv_data[i][3] << "," << csv_data[i][4] << ","<< csv_data[i][5] << "," << rowtime << "," << opt->underlying_price << "," << opt->mid_px << "," << opt->strike << "," << opt->time_to_expiry << "," << opt->str_opt_type << "," << opt->volatility << "," << opt->delta << "," << opt->gamma << "," << opt->rho << "," << opt->vega << "," <<opt->theta<< ",";
//                        for(int k = 0; k < spot_moves2.size(); k++){
//                                // std::string uMoves = spot_moves[k], vMoves = "0";
//                                double sm = std::stod(spot_moves2[k]);
//                                double ivm = 0.0;
//                                // s_pnl = s.getScenarioPnl((sm), (ivm))*100;
//                                double newspot = (1+sm)* opt->underlying_price;
//                                Tim::GreekCalculator G(0);
//                                Tim::Options n_opt;
//                                n_opt = *opt;
//                                Tim::Options* new_opt = &n_opt;
//                                new_opt->underlying_price = (newspot);
//                                double new_mid_px = G.findBSPrice(new_opt, newspot, 0);
//                                new_opt->mid_px = new_mid_px;
//                                // auto start = std::chrono::high_resolution_clock::now();
//                                Tim::Greeks g_new = G.getGreeks(new_opt, newspot, 0);
//                                // auto stop = std::chrono::high_resolution_clock::now();
//                                // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
//                                // std::cout << "TIMEEEEEE: " << duration.count() << std::endl;
//                                new_opt->updateAllGreeks(g_new);
//                                s_pnl = (new_mid_px - opt->mid_px)* double(std::stoi(csv_data[i][5]));
//                                ss << new_opt->underlying_price<< "," << new_opt->mid_px<< "," << new_opt->strike << "," << s_pnl << "," << new_opt->time_to_expiry<< "," << new_opt->str_opt_type<< "," << new_opt->volatility<< "," << new_opt->delta<< "," << new_opt->gamma<< "," << new_opt->rho<< "," << new_opt->vega<< "," << new_opt->theta << ",";
//                        }
//                        for(int k = 0; k < iv_moves.size(); k++){
//                                double sm = 0;
//                                double ivm = std::stod(iv_moves[k]);
//                                double newiv = (1+ivm)*opt->volatility;
//                                Tim::GreekCalculator G(0);
//                                Tim::Options n_opt;
//                                n_opt = *opt;
//                                Tim::Options* new_opt = &n_opt;
//                                double new_mid_px = G.findBSPrice(new_opt, opt->underlying_price, ivm*100);
//                                new_opt->mid_px = new_mid_px;
//
//                                Tim::Greeks g_new = G.getGreeks(new_opt, opt->underlying_price, 0);
//                                new_opt->updateAllGreeks(g_new);
//                                s_pnl = (new_mid_px - opt->mid_px)* double(std::stoi(csv_data[i][5]));
//                                ss << new_opt->underlying_price<< "," << new_opt->mid_px<< "," << new_opt->strike << "," << s_pnl << "," << new_opt->time_to_expiry<< "," << new_opt->str_opt_type<< "," << new_opt->volatility<< "," << new_opt->delta<< "," << new_opt->gamma<< "," << new_opt->rho<< "," << new_opt->vega<< "," << new_opt->theta << ",";
//
//                        }
//                        ss_str = ss.str();
//                        ss_str.pop_back();
//                        ss.str(std::string());
//                        output2 << ss_str+"\n";
//                        std::cout << ss_str << std::endl;
//                }
//                else{
//                        continue;
//                }
//
//        }
//        output2.close();
//}

