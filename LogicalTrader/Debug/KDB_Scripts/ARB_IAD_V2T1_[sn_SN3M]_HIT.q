////////////////////////////////////////////////////
/////策略端口说明                             //////
////////////////////////////////////////////////////
//此类策略集中在90xx端口


////////////////////////////////////////////////////
/////参数设定区域                             //////
////////////////////////////////////////////////////
// StandardDeviationTimes:1.0;
// WindowFrameLength:200;
// KindleLengthOnSecond:1;
h "PairLeg1Symbol:`sn1809l;PairLeg2Symbol:`SN3M;PairLeg3Symbol:`SN3M;";
////////////////////////////////////////////////////
/////多腿数据合并                             //////
////////////////////////////////////////////////////
PairLegOne:   h"-5000#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = PairLeg1Symbol";
PairLegTwo:   h"-5000#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = PairLeg2Symbol";
PairLegThree: h"-5000#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = PairLeg3Symbol";
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
MultiplePair:update LegOneBP1:fills  LegOneBP1,LegOneBV1:fills  LegOneBV1,LegOneSP1:fills  LegOneSP1,LegOneSV1:fills  LegOneSV1,LegTwoBP1:fills  LegTwoBP1,LegTwoBV1:fills  LegTwoBV1,LegTwoSP1:fills  LegTwoSP1,LegTwoSV1:fills  LegTwoSV1,LegThreeBP1:fills  LegThreeBP1,LegThreeBV1:fills  LegThreeBV1,LegThreeSP1:fills  LegThreeSP1,LegThreeSV1:fills  LegThreeSV1  from tempMultiplePair;
MultiplePair:`Date`ReceiveDate`Symbol`LegOneBidPrice1`LegOneBidVol1`LegOneAskPrice1`LegOneAskVol1`LegTwoBidPrice1`LegTwoBidVol1`LegTwoAskPrice1`LegTwoAskVol1`LegThreeBidPrice1`LegThreeBidVol1`LegThreeAskPrice1`LegThreeAskVol1 xcol MultiplePair;
update BidPrice1:PairFormula[LegOneBidPrice1;LegTwoAskPrice1], BidVol1:1,AskPrice1:PairFormula[LegOneAskPrice1;LegTwoBidPrice1], AskVol1:1 from `MultiplePair;
MultiplePair:select from MultiplePair where LegOneBidPrice1<>0N;
MultiplePair:select from MultiplePair where LegTwoBidPrice1<>0N;
MultiplePair:select from MultiplePair where LegThreeBidPrice1<>0N;
////////////////////////////////////////////////////
/////信号计算准备                             //////
////////////////////////////////////////////////////
strategyData:delete date,second from select by ReceiveDate.date, KindleLengthOnSecond xbar ReceiveDate.second from MultiplePair;
//MultiplePair:-300#select from MultiplePair;
update LowerBand:bollingerBands[StandardDeviationTimes;WindowFrameLength;BidPrice1][0], HigherBand:bollingerBands[StandardDeviationTimes;WindowFrameLength;AskPrice1][2]  from `strategyData;
Signal: strategyData;update Signal:1 from `Signal where AskPrice1 < LowerBand;update Signal:-1 from `Signal where BidPrice1 > HigherBand;
//Signal:select from Signal where ((Signal = 1) or  (Signal = -1));
//SignalCount: exec count Signal from Signal;
//if[(SignalCount>0);SignalLast:-1#select from Signal;FinalSignal:FinalSignal,SignalLast];
////////////////////////////////////////////////////
/////策略逻辑判断                             //////
////////////////////////////////////////////////////
SignalLast:-1#select from Signal;delete from `SignalLast where Signal = 0N; FinalSignal:FinalSignal,SignalLast;
delete from `FinalSignal where Date.minute > 00:59:55, Date.minute < 09:00:05;
delete from `FinalSignal where Date.minute > 10:14:55, Date.minute < 10:30:05;
delete from `FinalSignal where Date.minute > 11:29:55, Date.minute < 13:30:05;
delete from `FinalSignal where Date.minute > 14:59:55, Date.minute < 21:00:05;
//ShortLong:select from FinalSignal  where (Signal<>(prev Signal));
//FinalSignal:ShortLong;