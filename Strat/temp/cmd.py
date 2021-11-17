
from datetime import timedelta, date
#import datetime

startDate = date(2019, 1, 1) #20200101
endDate   = date(2021,11,10) #20210228

delta = endDate - startDate
weekdays  = [5,6]
fileObject = open("dump_cmd_oi.txt",'a')
for i in range(delta.days + 1):
	day = startDate + timedelta(days = i)
#	print(day.weekday())
	if day.weekday() not in weekdays:
		#print(type(day.strftime("%Y%m%d")))
		command = "nohup ./nasteamdata/pranka/HogBin/hog_oi --mode 1 --pid 0 --date " + str(day.strftime("%Y%m%d")) + " --param_file_path /nasteamdata/pranka/HogBin/NewParamsList.json  --symbol_file_path /nasteamdata/pranka/HogBin/SymSpecifierList_banknifty.json  --output_folder_path /nasteamdata/pranka/spare/oi/"
		fileObject.write(command)
		fileObject.write("\n")
		#command = "nohup ./nasteamdata/hpuria/HogBin/hog --mode 1 --pid 1 --date " + str(day.strftime("%Y%m%d")) + " --param_file_path /nasteamdata/hpuria/HogBin/NewParamsList.json  --symbol_file_path /nasteamdata/hpuria/HogBin/SymSpecifierList_bankniftyNWO.json  --output_folder_path /nasteamdata/hpuria/spare/"
		#fileObject.write(command)
		#fileObject.write("\n")

		#command = "nohup ./nasteamdata/hpuria/HogBin/hog --mode 1 --pid 2 --date " + str(day.strftime("%Y%m%d")) + " --param_file_path /nasteamdata/hpuria/HogBin/NewParamsList.json  --symbol_file_path /nasteamdata/hpuria/HogBin/SymSpecifierList_bankniftyO.json  --output_folder_path /nasteamdata/hpuria/spare/"
		#fileObject.write(command)
		#fileObject.write("\n")
fileObject.close()

