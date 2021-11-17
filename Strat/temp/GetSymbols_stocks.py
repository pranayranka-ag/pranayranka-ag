#!usr/bin/python
import calendar
import subprocess
import pandas as pd
from subprocess import Popen
import os
import sys
import re
from datetime import timedelta, date
import datetime

lst = ["AXISBANK","HDFCBANK", "ICICIBANK", "KOTAKBANK", "SBIN"]
# lst = ["AXISBANK", "BANDHANBNK", "FEDERALBNK", "HDFCBANK", "ICICIBANK", "IDFCFIRSTB", "INDUSINDBK", "KOTAKBANK", "PNB", "RBLBANK", "SBIN"]
def isvalidDateForNextMonth(date):
    date = datetime.date(int(date[0:4]), int(date[4:6]), int(date[6:]))
    nextDate = date
    cr = 0
    while cr < 2:
        nextDate += datetime.timedelta(1)
        if nextDate.month != date.month:
            break
        if nextDate.weekday() == 3:
            cr = cr + 1
        if cr >=2 :
            return False
    if cr == 1 or date.weekday() == 3:
        return True
    return False


def inclusive_range(start, stop, step):
    d1 = datetime.date(int(start[0:4]), int(start[4:6]), int(start[6:]))
    d2 = datetime.date(int(stop[0:4]), int(stop[4:6]), int(stop[6:]))
    days = [d1 + datetime.timedelta(days=x) for x in range((d2 - d1).days + 1)]
    myRange = []
    for day in days:
        if day.weekday() < 5:
            myRange.append(day.strftime('%Y%m%d'))
            print(myRange[-1])
    return myRange


def findDay(date):
    born = datetime.datetime.strptime(date, '%Y%m%d').weekday()
    print(born)
    return born


# if __name__ == "__main__":
#python3 Get_Symbol.py 20201001 20210228 BNFTWO C P 80 120
print("Program starts here")
start_date = int(sys.argv[1])
end_date = int(sys.argv[2])
sym = str(sys.argv[3])
types = []
types.append(str(sys.argv[4]))
types.append(str(sys.argv[5]))
strike_range = []
strike_range.append(int(sys.argv[6]))
strike_range.append(int(sys.argv[7]))

print("Good here", start_date, end_date, sym, types[0], types[1], strike_range[0], strike_range[1])
sys.stdout.flush()
date_range = inclusive_range(str(start_date), str(end_date), 1)
this_month = date_range
next_month = [i for i in date_range if isvalidDateForNextMonth(str(i))]

print(next_month)
def excute(dates, typ, s):
    block = 0
    while True:
        if block * 100 >= len(dates):
            break
        date_range = dates[block * 100:(block + 1) * 100]
        cmd = []
        for i in date_range:
            cmd.append(Popen('~/bin/getAxisSymStocks.sh ' + str(i) + " " + str(strike_range[0]) + " " + str(
                strike_range[1]) + " " + s + " " + typ, shell=True))
        for c in cmd:
            c.wait()
        for i in date_range:
            with open("/home/pranka/dated_symbols_" + s + "/" + str(i) + "/symbols", "r") as f:
                lines = f.readlines()
            with open("/home/pranka/dated_symbols_" + s + "/" + str(i) + "/symbols", "w") as f:
                for line in lines:
                    ll = line
                    symb = line.split(" ")[1]
                    print(symb)
                    digs = re.findall('\d+', symb)
                    # print(digs)
               	    if (len(digs[0]) > 10):
                        f.write(ll)
		        #f.write(ll)
        block = block + 1

excute(this_month, "0", sym)
# excute(next_month, "1")

# for i in date_range:
#     print("T")
#     subprocess.call(
#         ['/home/pkumar/bin/getAxisSym.sh ' + str(i) + " " + str(strike_range[0]) + " " + str(
#             strike_range[1]) + " " + sym + " 0"],
#         shell=True)
#     if isvalidDateForNextMonth(str(i)):
#         subprocess.call(['/home/pkumar/bin/getAxisSym.sh ' + str(i) + " " + str(strike_range[0]) + " " + str(
#             strike_range[1]) + " " + sym + " 0"], shell=True)
#     with open("/home/pkumar/dated_symbols/" + str(i) + "/symbols", "r") as f:
#         lines = f.readlines()
#     with open("/home/pkumar/dated_symbols/" + str(i) + "/symbols", "w") as f:
#         for line in lines:
#             ll = line
#             symb = line.split(" ")[1]
#             print(symb)
#             digs = re.findall('\d+', symb)
#             if (len(digs[0]) > 10):
#                 f.write(ll)
