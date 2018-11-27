////////////////////////////////////////////////////
/////参数设定区域                             //////
////////////////////////////////////////////////////
// KindleLengthOnSecond:1;
// WindowFrameLength:200;
// StandardDeviationTimes:1.0;
//PairLeg1Symbol:`sn1809;PairLeg2Symbol:`SN3M;PairLeg3Symbol:`SN3M;
////////////////////////////////////////////////////
/////多腿数据合并                             //////
////////////////////////////////////////////////////
PairLegOne:   h"-5000#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = `",string PairLeg1Symbol;
PairLegTwo:   h"-5000#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = `",string PairLeg2Symbol;
PairLegThree: h"-5000#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = `",string PairLeg3Symbol;
delete from `PairLegOne where Date.second > 01:00:00, Date.second < 09:00:00;
delete from `PairLegOne where Date.second > 10:15:00, Date.second < 10:30:00;
delete from `PairLegOne where Date.second > 11:30:00, Date.second < 13:30:00;
delete from `PairLegOne where Date.second > 15:00:00, Date.second < 21:00:00;
delete from `PairLegTwo where Date.second > 01:00:00, Date.second < 09:00:00;
delete from `PairLegTwo where Date.second > 10:15:00, Date.second < 10:30:00;
delete from `PairLegTwo where Date.second > 11:30:00, Date.second < 13:30:00;
delete from `PairLegTwo where Date.second > 15:00:00, Date.second < 21:00:00;
delete from `PairLegThree where Date.second > 01:00:00, Date.second < 09:00:00;
delete from `PairLegThree where Date.second > 10:15:00, Date.second < 10:30:00;
delete from `PairLegThree where Date.second > 11:30:00, Date.second < 13:30:00;
delete from `PairLegThree where Date.second > 15:00:00, Date.second < 21:00:00;
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
SecondLasttempMultiplePair:-6#tempMultiplePair;
Last_Time_Execution: exec first ReceiveDate from SecondLasttempMultiplePair;
Count_Control:5;
StackMultiplePair:select from tempMultiplePair where ReceiveDate <= Last_Time_Execution;
QueueMultiplePair:select from tempMultiplePair where ReceiveDate > Last_Time_Execution;
StepMultiplePair_Count:exec count ReceiveDate from QueueMultiplePair;
if[StepMultiplePair_Count > Count_Control;StepMultiplePair_Count:Count_Control;QueueMultiplePair:-5#select from tempMultiplePair where ReceiveDate > Last_Time_Execution;];
while[StepMultiplePair_Count-:1;
      StepMultiplePair:1#select from QueueMultiplePair;
      StackMultiplePair:StackMultiplePair,StepMultiplePair;
        MultiplePair:update LegOneBP1:fills  LegOneBP1,LegOneBV1:fills  LegOneBV1,LegOneSP1:fills  LegOneSP1,LegOneSV1:fills  LegOneSV1,LegTwoBP1:fills  LegTwoBP1,LegTwoBV1:fills  LegTwoBV1,LegTwoSP1:fills  LegTwoSP1,LegTwoSV1:fills  LegTwoSV1,LegThreeBP1:fills  LegThreeBP1,LegThreeBV1:fills  LegThreeBV1,LegThreeSP1:fills  LegThreeSP1,LegThreeSV1:fills  LegThreeSV1  from StackMultiplePair;
        MultiplePair:`Date`ReceiveDate`Symbol`LegOneBidPrice1`LegOneBidVol1`LegOneAskPrice1`LegOneAskVol1`LegTwoBidPrice1`LegTwoBidVol1`LegTwoAskPrice1`LegTwoAskVol1`LegThreeBidPrice1`LegThreeBidVol1`LegThreeAskPrice1`LegThreeAskVol1 xcol MultiplePair;
        update BidPrice1:PairFormula[LegOneBidPrice1;LegTwoAskPrice1], BidVol1:1,AskPrice1:PairFormula[LegOneAskPrice1;LegTwoBidPrice1], AskVol1:1 from `MultiplePair;
        MultiplePair:select from MultiplePair where LegOneBidPrice1<>0N;
        MultiplePair:select from MultiplePair where LegTwoBidPrice1<>0N;
        MultiplePair:select from MultiplePair where LegThreeBidPrice1<>0N;
        ////////////////////////////////////////////////////
        /////信号计算准备                             //////
        ////////////////////////////////////////////////////
        strategyData:delete date,second from select by ReceiveDate.date, KindleLengthOnSecond xbar ReceiveDate.second from MultiplePair;
        update LowerBand:bollingerBands[StandardDeviationTimes;WindowFrameLength;BidPrice1][0], HigherBand:bollingerBands[StandardDeviationTimes;WindowFrameLength;AskPrice1][2]  from `strategyData;
        Signal: strategyData;update Signal:1 from `Signal where AskPrice1 < LowerBand;update Signal:-1 from `Signal where BidPrice1 > HigherBand;
        ////////////////////////////////////////////////////
        /////策略逻辑判断                             //////
        ////////////////////////////////////////////////////
        SignalLast:-1#select from Signal;delete from `SignalLast where (Signal = 0N) or (LowerBand = 0N) or (HigherBand = 0N); FinalSignal:FinalSignal,SignalLast;
        delete from `FinalSignal where Date.second > 00:59:55, Date.second < 09:00:05;
        delete from `FinalSignal where Date.second > 10:14:55, Date.second < 10:30:05;
        delete from `FinalSignal where Date.second > 11:29:55, Date.second < 13:30:05;
        delete from `FinalSignal where Date.second > 14:59:55, Date.second < 21:00:05;
        ShortLong:select from FinalSignal  where (Signal<>(prev Signal));
        FinalSignal:ShortLong;
      ]