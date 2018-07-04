/ Welcome to qStudio,
/ 
/ First you should click on the menu: Server->"Add Server"  to add your KDB server.
/ Then below you will find example queries to generate each chart type:
/ 
/ - Simply press control+enter to run the single line time series charts 
/     Then on the chart tab/panel select "Time Series" chart type to draw the first example.
/ 
/ - For multi-line charts highlight the table query and press control+e to execute the highlighted text
/     and again select the appropriate chart type in the chart panels drop down.    
/
/ If you want help there are guides located at http://www.timestored.com/qstudio/help/
/ Any feature requests etc feel free to contact us at:  contact@timestored.com


//### Example Time Series Charts
([] dt:2013.01.01+til 21; cosineWave:cos a; sineWave:sin a:0.6*til 21)

([] time:10:00t+60000*til 99; Position:0.4*a-mod[a;8]; Cost:a:100*sin 0.015*til 99)


//### Example Line / Bar / Area Chart

/ Multiple Series with Time X-Axis
([Month:2000.01m + til 12]  
	 Costs:30.0 40.0 45.0 55.0 58.0 63.0 55.0 65.0 78.0 80.0 75.0 90.0 ; 
	 Sales:10.0 12.0 14.0 18.0 26.0 42.0 74.0 90.0 110.0 130.0 155.0 167.0 )

/ Multiple Series
([] Continent:`NorthAmerica`Asia`Asia`Europe`Europe`Africa`Asia`Africa`Asia;
	 Country:`US`China`japan`Germany`UK`Zimbabwe`Bangladesh`Nigeria`Vietnam; 
	 Population:313847.0 1343239.0 127938.0 81308.0 63047.0 13010.0 152518.0 166629.0 87840.0 ;
	 GDP:15080.0 11300.0 4444.0 3114.0 2228.0 9.9 113.0 196.0 104.0 ; 
	GDPperCapita:48300.0 8400.0 34700.0 38100.0 36500.0 413.0 1788.0 732.0 3359.0 ;  
	LifeExpectancy:77.14 72.22 80.93 78.42 78.16 39.01 61.33 51.01 70.05 )


//### Bubble Chart / Scatter Plot
update GDPperCapita%20 from ([] Continent:`NorthAmerica`Asia`Asia`Europe`Europe`Africa`Asia`Africa`Asia;
	 Country:`US`China`japan`Germany`UK`Zimbabwe`Bangladesh`Nigeria`Vietnam; 
	 Population:313847.0 1343239.0 127938.0 81308.0 63047.0 13010.0 152518.0 166629.0 87840.0 ;
	 GDP:15080.0 11300.0 4444.0 3114.0 2228.0 9.9 113.0 196.0 104.0 ; 
	GDPperCapita:48300.0 8400.0 34700.0 38100.0 36500.0 413.0 1788.0 732.0 3359.0 ;  
	LifeExpectancy:77.14 72.22 80.93 78.42 78.16 39.01 61.33 51.01 70.05 )


//### Candlestick Chart
([] t:09:00t+600000*til 22; high:c+30; low:c-20; open:60+til 22; close:c:55+2*til 22; volume:22#3 9 6)


//### Heatmap
([] Continent:`NorthAmerica`Asia`Asia`Europe`Europe`Africa`Asia`Africa`Asia;
	 Country:`US`China`japan`Germany`UK`Zimbabwe`Bangladesh`Nigeria`Vietnam; 
	 Population:313847.0 1343239.0 127938.0 81308.0 63047.0 13010.0 152518.0 166629.0 87840.0 ;
	 GDP:15080.0 11300.0 4444.0 3114.0 2228.0 9.9 113.0 196.0 104.0 ; 
	GDPperCapita:48300.0 8400.0 34700.0 38100.0 36500.0 413.0 1788.0 732.0 3359.0 ;  
	LifeExpectancy:77.14 72.22 80.93 78.42 78.16 39.01 61.33 51.01 70.05 )


//### Histogram
([] Returns:cos 0.0015*til 500; Losses:cos 0.002*til 500)

//### PieChart
/ single pie
([] Country:`US`China`japan`Germany`UK`Zimbabwe`Bangladesh`Nigeria`Vietnam; 
	 GDP:15080.0 11300.0 4444.0 3114.0 2228.0 9.9 113.0 196.0 104.0 )

/ Many Pies
([] Continent:`NorthAmerica`Asia`Asia`Europe`Europe`Africa`Asia`Africa`Asia;
	 Country:`US`China`japan`Germany`UK`Zimbabwe`Bangladesh`Nigeria`Vietnam; 
	 Population:313847.0 1343239.0 127938.0 81308.0 63047.0 13010.0 152518.0 166629.0 87840.0 ;
	 GDP:15080.0 11300.0 4444.0 3114.0 2228.0 9.9 113.0 196.0 104.0 ; 
	GDPperCapita:48300.0 8400.0 34700.0 38100.0 36500.0 413.0 1788.0 732.0 3359.0 ;  
	LifeExpectancy:77.14 72.22 80.93 78.42 78.16 39.01 61.33 51.01 70.05 )


`Quote insert (2018.06.29D16:33:29;14985.000000;14995.000000;14985.000000;14995.000000)
DAQuote:([] Date:();`symbol$Symbol:(); `float$Leg1Bid1:(); `float$Leg1Ask1:(); `float$Leg2Bid1:();`float$Leg2Ask1:())
CTPQuote:([] Date:();`symbol$Symbol:(); `float$Leg1Bid1:(); `float$Leg1Ask1:(); `float$Leg2Bid1:();`float$Leg2Ask1:())

`DAQuote insert ("2018.06.30D18:10:33926;`NI3M;15020.000000;15030.000000;15020.000000;15030.000000)

select 
Coding_Applications/Codes/Github/CTPTrader/CTP期货交易系统_旗舰版/Debug

`:D:/Coding_Applications/Codes/Github/CTPTrader/CTP期货交易系统_旗舰版/Debug/CTPQuote set CTPQuote


`:D:/Coding_Applications/Codes/Github/ / CTPTrader / CTPQuote set CTPQuote;


`:D:/Coding_Applications/Codes/Github/CTPTrader/CTPQuote set CTPQuote;

delete from `CTPQuote where Date.minute > 02:30:00, Date.minute <= 09:00:00;
delete from `CTPQuote where Date.minute > 11:30:00, Date.minute < 13:00:00;
delete from `CTPQuote where Date.minute > 15:15:00, Date.minute < 21:00:00;
temp:DAQuote;
`temp insert (2018.06.30D18:17:033.560;`NI3M;15020.000000;15030.000000;15020.000000;15030.000000)

.Q.w[]
//把所有数据合并在Quote表中，这样减少后期定义烦恼
temp: select Date:Date, ReceiveDate:Date, Symbol, BidPrice1:Leg1Bid1, BidVol1:1,AskPrice1:Leg1Ask1, AskVol1:1 from CTPQuote;
Quote:temp;
temp: select Date:Date, ReceiveDate:Date, Symbol, BidPrice1:Leg1Bid1, BidVol1:1,AskPrice1:Leg1Ask1, AskVol1:1 from DAQuote;
Quote:Quote, temp;
Quote:`ReceiveDate xasc Quote;
//测试项目
`Quote insert (2018.07.02D10:15:00;2018.07.02D10:23:19.814;`ni1809;116730.000000;1;116740.000000;1)
`Quote insert (2018.07.02D10:31:45;2018.07.02D10:31:45.978;`NI3M;14870.000000;1;14885.000000;1)



////////////////////////////////////////////////////
//从主KDB中获取数据到次主kdb合并数据并计算信号//////
////////////////////////////////////////////////////
h:hopen `::5000;
PairFormula:{(x%y)};
f:{x%y};bollingerBands: {[k;n;data]      movingAvg: mavg[n;data];      md: sqrt mavg[n;data*data]-movingAvg*movingAvg;      movingAvg+/:(k*-1 0 1)*\:md};ReceiveTimeToDate:{(""z"" $ 1970.01.01+ floor x %86400000000 )+ 08:00:00.000 +""j""$ 0.001* x mod   86400000000}; 
PairLegOne:   h"-2000#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = `sn1809";
PairLegTwo:   h"-2000#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = `SN3M";
PairLegThree:   h"-2000#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = `SN3M";

PairLegOne:  `Date`ReceiveDate`Symbol`LegOneBP1`LegOneBV1`LegOneSP1`LegOneSV1 xcol PairLegOne;
PairLegTwo:  `Date`ReceiveDate`Symbol`LegTwoBP1`LegTwoBV1`LegTwoSP1`LegTwoSV1 xcol PairLegTwo;
PairLegThree:`Date`ReceiveDate`Symbol`LegThreeBP1`LegThreeBV1`LegThreeSP1`LegThreeSV1 xcol PairLegThree;
PairLegOne: update LegTwoBP1:0Nf,LegTwoBV1:0N,LegTwoSP1:0Nf,LegTwoSV1:0N,LegThreeBP1:0Nf,LegThreeBV1:0N,LegThreeSP1:0Nf,LegThreeSV1:0N from PairLegOne;
PairLegTwo: update LegOneBP1:0Nf,LegOneBV1:0N,LegOneSP1:0Nf,LegOneSV1:0N,LegThreeBP1:0Nf,LegThreeBV1:0N,LegThreeSP1:0Nf,LegThreeSV1:0N from PairLegTwo;
PairLegThree:update LegOneBP1:0Nf,LegOneBV1:0N,LegOneSP1:0Nf,LegOneSV1:0N,LegTwoBP1:0Nf,LegTwoBV1:0N,LegTwoSP1:0Nf,LegTwoSV1:0N from PairLegThree;
PairLegOne:`Date`ReceiveDate`Symbol`LegOneBP1`LegOneBV1`LegOneSP1`LegOneSV1`LegTwoBP1`LegTwoBV1`LegTwoSP1`LegTwoSV1`LegThreeBP1`LegThreeBV1`LegThreeSP1`LegThreeSV1 xcols PairLegOne;
PairLegTwo:`Date`ReceiveDate`Symbol`LegOneBP1`LegOneBV1`LegOneSP1`LegOneSV1`LegTwoBP1`LegTwoBV1`LegTwoSP1`LegTwoSV1`LegThreeBP1`LegThreeBV1`LegThreeSP1`LegThreeSV1 xcols PairLegTwo;
PairLegThree:`Date`ReceiveDate`Symbol`LegOneBP1`LegOneBV1`LegOneSP1`LegOneSV1`LegTwoBP1`LegTwoBV1`LegTwoSP1`LegTwoSV1`LegThreeBP1`LegThreeBV1`LegThreeSP1`LegThreeSV1 xcols PairLegThree;
PairLegThree: select from PairLegThree where ReceiveDate = min ReceiveDate;
tempMultiplePair:`ReceiveDate xasc PairLegOne,PairLegTwo, PairLegThree;
MultiplePair:update LegOneBP1:fills  LegOneBP1,
LegOneBV1:fills  LegOneBV1,
LegOneSP1:fills  LegOneSP1,
LegOneSV1:fills  LegOneSV1,
LegTwoBP1:fills  LegTwoBP1,
LegTwoBV1:fills  LegTwoBV1,
LegTwoSP1:fills  LegTwoSP1,
LegTwoSV1:fills  LegTwoSV1,
LegThreeBP1:fills  LegThreeBP1,
LegThreeBV1:fills  LegThreeBV1,
LegThreeSP1:fills  LegThreeSP1,
LegThreeSV1:fills  LegThreeSV1  from tempMultiplePair;
MultiplePair:`Date`ReceiveDate`Symbol`LegOneBidPrice1`LegOneBidVol1`LegOneAskPrice1`LegOneAskVol1`LegTwoBidPrice1`LegTwoBidVol1`LegTwoAskPrice1`LegTwoAskVol1`LegThreeBidPrice1`LegThreeBidVol1`LegThreeAskPrice1`LegThreeAskVol1 xcol MultiplePair;
update BidPrice1:PairFormula[LegOneBidPrice1;LegTwoAskPrice1], BidVol1:1,AskPrice1:PairFormula[LegOneAskPrice1;LegTwoBidPrice1], AskVol1:1 from `MultiplePair;
MultiplePair:select from MultiplePair where LegOneBidPrice1<>0N;
MultiplePair:select from MultiplePair where LegTwoBidPrice1<>0N;
MultiplePair:select from MultiplePair where LegThreeBidPrice1<>0N;
strategyData:delete date,second from select by ReceiveDate.date, 1 xbar ReceiveDate.second from MultiplePair;
update LowerBand:bollingerBands[1;200;BidPrice1][0], HigherBand:bollingerBands[1;200;AskPrice1][2]  from `strategyData;
Signal: strategyData;update Signal:`Long from `Signal where AskPrice1 < LowerBand;update Signal:`Short from `Signal where BidPrice1 > HigherBand;
Signal:select from Signal where ((Signal = `Long) or  (Signal = `Short));
ShortLong:select from Signal  where (Signal<>(prev Signal));
///////////////////Profit///////////////////
tempShortLong: ShortLong2;
update Profit: (((prev LegOneBidPrice1) - (LegOneAskPrice1))) from `tempShortLong;
LongProfit: select  from tempShortLong where Signal = `Long;
update Profit: (((LegOneBidPrice1) - (prev LegOneAskPrice1)))  from `tempShortLong;
ShortProfit: select  from tempShortLong where Signal = `Short;
Profit: ShortProfit, LongProfit;`ReceiveTimeDate xasc `Profit;delete from `Profit where Profit > 10000;update SumsProfit:sums Profit from `Profit;delete from `Profit where Profit > 15000; ";

select count Signal from ShortLong

////////////////////////////////////////////////////
/////盘后数据Insert错误//////
///////////////////////////////////////////////////
Quote:-500000#select from Quote;
delete from `Quote where Date.minute > 02:30:00, Date.minute <= 09:00:00;
delete from `Quote where Date.minute > 11:30:00, Date.minute < 13:00:00;
delete from `Quote where Date.minute > 15 : 15 : 00, Date.minute < 21 : 00 : 00;
////////////////////////////////////////////////////
/////C++ String的字符消失问题//////
///////////////////////////////////////////////////
std::string p = "f:{x%y};bollingerBands: {[k;n;data]      movingAvg: mavg[n;data];    md: sqrt mavg[n;data*data]-movingAvg*movingAvg;      movingAvg+/:(k*-1 0 1)*\\:md};ReceiveTimeToDate:{(\"\"z\"\" $ 1970.01.01+ floor x %86400000000 )+ 08:00:00.000 +\"\"j\"\"$ 0.001* x mod  86400000000};";
printf("%s", p.c_str());
return 0;
////////////////////////////////////////////////////
/////KDB 加载硬盘上的Q文件                   //////
///////////////////////////////////////////////////
\l d:/Coding_Applications/Codes/Github/CTPTrader/LogicalTrader/KDB_Scripts/ARB_IAD_V2T1_[sn_SN3M]_1_200_HIT.q
\l c:/q/ARB_IAD_V2T1_[sn_SN3M]_1_200_HIT.q
////////////////////////////////////////////////////
/////第一次启动需要手动加载表                  /////
////////////////////////////////////////////////////
FinalSignal::();
////////////////////////////////////////////////////
/////最后一行监控                              /////
////////////////////////////////////////////////////
select from MultiplePair where ReceiveDate = min ReceiveDate
select from MultiplePair where ReceiveDate = max ReceiveDate