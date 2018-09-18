////////////////////////////////////////////////////
/////策略端口说明                             //////
////////////////////////////////////////////////////
//此类策略集中在80xx端口


////////////////////////////////////////////////////
/////参数设定区域                             //////
////////////////////////////////////////////////////
KindleLengthOnSecond:60;
VolatilityRange:50;
BreakRange:50;
PositionTimes:4;
NEntryParameter:0.5;
NStoplossPositionParameter:1.5;
////////////////////////////////////////////////////
/////多腿数据合并                             //////
////////////////////////////////////////////////////
PairLegOne:   h"-180#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = `IC1807";
PairLegTwo:   h"-180#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = `IH1807";
PairLegThree: h"-180#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = `IH1807";
PairLegOne:  `Date`ReceiveDate`Symbol`LegOneBP1`LegOneBV1`LegOneSP1`LegOneSV1 xcol PairLegOne;
PairLegTwo:  `Date`ReceiveDate`Symbol`LegTwoBP1`LegTwoBV1`LegTwoSP1`LegTwoSV1 xcol PairLegTwo;
PairLegThree:`Date`ReceiveDate`Symbol`LegThreeBP1`LegThreeBV1`LegThreeSP1`LegThreeSV1 xcol PairLegThree;
PairLegOne:   update LegTwoBP1:0Nf,LegTwoBV1:0N,LegTwoSP1:0Nf,LegTwoSV1:0N,LegThreeBP1:0Nf,LegThreeBV1:0N,LegThreeSP1:0Nf,LegThreeSV1:0N from PairLegOne;
PairLegTwo:   update LegOneBP1:0Nf,LegOneBV1:0N,LegOneSP1:0Nf,LegOneSV1:0N,LegThreeBP1:0Nf,LegThreeBV1:0N,LegThreeSP1:0Nf,LegThreeSV1:0N from PairLegTwo;
PairLegThree: update LegOneBP1:0Nf,LegOneBV1:0N,LegOneSP1:0Nf,LegOneSV1:0N,LegTwoBP1:0Nf,LegTwoBV1:0N,LegTwoSP1:0Nf,LegTwoSV1:0N from PairLegThree;
PairLegOne:  `Date`ReceiveDate`Symbol`LegOneBP1`LegOneBV1`LegOneSP1`LegOneSV1`LegTwoBP1`LegTwoBV1`LegTwoSP1`LegTwoSV1`LegThreeBP1`LegThreeBV1`LegThreeSP1`LegThreeSV1 xcols PairLegOne;
PairLegTwo:  `Date`ReceiveDate`Symbol`LegOneBP1`LegOneBV1`LegOneSP1`LegOneSV1`LegTwoBP1`LegTwoBV1`LegTwoSP1`LegTwoSV1`LegThreeBP1`LegThreeBV1`LegThreeSP1`LegThreeSV1 xcols PairLegTwo;
PairLegThree:`Date`ReceiveDate`Symbol`LegOneBP1`LegOneBV1`LegOneSP1`LegOneSV1`LegTwoBP1`LegTwoBV1`LegTwoSP1`LegTwoSV1`LegThreeBP1`LegThreeBV1`LegThreeSP1`LegThreeSV1 xcols PairLegThree;
PairLegThree: select from PairLegThree where ReceiveDate = min ReceiveDate;
tempMultiplePair:`ReceiveDate xasc PairLegOne,PairLegTwo, PairLegThree;
MultiplePair: update LegOneBP1:fills  LegOneBP1,LegOneBV1:fills  LegOneBV1,LegOneSP1:fills  LegOneSP1,LegOneSV1:fills  LegOneSV1,LegTwoBP1:fills  LegTwoBP1,LegTwoBV1:fills  LegTwoBV1,LegTwoSP1:fills  LegTwoSP1,LegTwoSV1:fills  LegTwoSV1,LegThreeBP1:fills  LegThreeBP1,LegThreeBV1:fills  LegThreeBV1,LegThreeSP1:fills  LegThreeSP1,LegThreeSV1:fills  LegThreeSV1  from tempMultiplePair;
MultiplePair: `Date`ReceiveDate`Symbol`LegOneBidPrice1`LegOneBidVol1`LegOneAskPrice1`LegOneAskVol1`LegTwoBidPrice1`LegTwoBidVol1`LegTwoAskPrice1`LegTwoAskVol1`LegThreeBidPrice1`LegThreeBidVol1`LegThreeAskPrice1`LegThreeAskVol1 xcol MultiplePair;
update BidPrice1:PairFormulaDValue[LegOneBidPrice1;200;LegTwoAskPrice1;600], BidVol1:1,AskPrice1:PairFormulaDValue[LegOneAskPrice1;200;LegTwoBidPrice1;600], AskVol1:1 from `MultiplePair;
MultiplePair: select from MultiplePair where LegOneBidPrice1<>0N;
MultiplePair: select from MultiplePair where LegTwoBidPrice1<>0N;
MultiplePair: select from MultiplePair where LegThreeBidPrice1<>0N;
////////////////////////////////////////////////////
/////叠加更新数据                             //////
////////////////////////////////////////////////////
TrailQuoteDataKindle: select Date:last Date,BidPrice1Open:first BidPrice1,BidPrice1High:max BidPrice1,BidPrice1Low:min BidPrice1, BidPrice1Close:last BidPrice1,BidVol1:100,AskPrice1Open:first AskPrice1,AskPrice1High:max AskPrice1,AskPrice1Low:min AskPrice1, AskPrice1Close:last AskPrice1,AskVol1:last 100,LegOneBidPrice1:last LegOneBidPrice1, LegOneAskPrice1:last LegOneAskPrice1, LegTwoBidPrice1:last LegTwoBidPrice1, LegTwoAskPrice1: last LegTwoAskPrice1 by KindleLengthOnSecond xbar Date.second from MultiplePair;
delete second from `TrailQuoteDataKindle;
QuoteDataKindle:QuoteDataKindle, TrailQuoteDataKindle;
QuoteDataKindle:select Date:last Date,BidPrice1Open:first BidPrice1Open,BidPrice1High:max BidPrice1High,BidPrice1Low:min BidPrice1Low, BidPrice1Close:last BidPrice1Close,BidVol1:100,AskPrice1Open:first AskPrice1Open,AskPrice1High:max AskPrice1High,AskPrice1Low:min AskPrice1Low, AskPrice1Close:last AskPrice1Close,AskVol1:last 100,LegOneBidPrice1:last LegOneBidPrice1, LegOneAskPrice1:last LegOneAskPrice1, LegTwoBidPrice1:last LegTwoBidPrice1, LegTwoAskPrice1: last LegTwoAskPrice1 by KindleLengthOnSecond xbar Date.second from QuoteDataKindle;
delete second from `QuoteDataKindle;QuoteDataKindle:`Date xasc QuoteDataKindle;
////////////////////////////////////////////////////
/////信号计算准备                             //////
////////////////////////////////////////////////////
StrategyData:update TrueRange: {max(x;y;z)}'[(AskPrice1High - BidPrice1Low);(AskPrice1High - (prev BidPrice1Close));((prev AskPrice1High) - BidPrice1Low)]  from QuoteDataKindle;
update N: mavg[VolatilityRange;TrueRange] from `StrategyData;
update ShortEntry: prev (BreakRange mmin BidPrice1Low), LongEntry: prev (BreakRange mmax AskPrice1High) from `StrategyData;
BidPrice1:exec last BidPrice1 from MultiplePair where Date = max Date;
AskPrice1:exec last AskPrice1 from MultiplePair where Date = max Date;
ParStrategyData:-2#select  from StrategyData;
LongEntry:exec first LongEntry from ParStrategyData;
N:exec first N from ParStrategyData;
DateTimeInMinute:exec last Date.second from MultiplePair;
WhetherInCoverTime:((DateTimeInMinute - 14:59:00) > 0);
if[(AskPrice1 > LocalHigh) and (PositionAddedTimes > 0);LocalHigh:AskPrice1];
if[((LocalHigh - AskPrice1) > 0) and (PositionAddedTimes > 0);(DrawBack:(LocalHigh - AskPrice1))];
////////////////////////////////////////////////////
/////三大逻辑判断                             //////
////////////////////////////////////////////////////
//根据时间判断是否平仓
if[(WhetherInCoverTime) and (PositionAddedTimes > 0);PositionAddedTimes:0;TrailShortLong:-1#select from QuoteDataKindle;update Signal:0 from `TrailShortLong;ShortLong: ShortLong, TrailShortLong;];
//回撤幅度判断是否平仓LocalHigh:
if[(DrawBack > (NStoplossPositionParameter * 1 * N)) and (PositionAddedTimes > 0);PositionAddedTimes:0;LocalHigh:-99999999;LocalLow:99999999;DrawBack: 0.0;;TrailShortLong:-1#select from QuoteDataKindle;update Signal:0 from `TrailShortLong;ShortLong: ShortLong, TrailShortLong;];
//根据仓位判断是否开仓
if[(not WhetherInCoverTime) and ((PositionAddedTimes = 3) and (PositionTimes > 0)) and (AskPrice1 > (LongEntry + (NEntryParameter * 3 * N)));PositionAddedTimes:4;TrailShortLong:-1#select from QuoteDataKindle;update Signal:4 from `TrailShortLong;ShortLong: ShortLong, TrailShortLong;];
if[(not WhetherInCoverTime) and ((PositionAddedTimes = 2) and (PositionTimes > 0)) and (AskPrice1 > (LongEntry + (NEntryParameter * 2 * N)));PositionAddedTimes:3;TrailShortLong:-1#select from QuoteDataKindle;update Signal:3 from `TrailShortLong;ShortLong: ShortLong, TrailShortLong;];
if[(not WhetherInCoverTime) and ((PositionAddedTimes = 1) and (PositionTimes > 0)) and (AskPrice1 > (LongEntry + (NEntryParameter * 1 * N)));PositionAddedTimes:2;TrailShortLong:-1#select from QuoteDataKindle;update Signal:2 from `TrailShortLong;ShortLong: ShortLong, TrailShortLong;];
if[(not WhetherInCoverTime) and ((PositionAddedTimes = 0) and (PositionTimes > 0)) and (AskPrice1 > (LongEntry + (NEntryParameter * 0 * N)));PositionAddedTimes:1;TrailShortLong:-1#select from QuoteDataKindle;update Signal:1 from `TrailShortLong;ShortLong: ShortLong, TrailShortLong;];
