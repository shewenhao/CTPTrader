////////////////////////////////////////////////////
/////参数设定区域                             //////
////////////////////////////////////////////////////
// KindleLengthOnSecond:1;
// WindowFrameLength:200;
// StandardDeviationTimes:1.0;
//Quote: get `:C:/q/Quote;
isTable:{if[98h=type x;:1b];if[99h=type x;:98h=type key x];0b};isTable2: {@[{isTable value x}; x; 0b]};
PairFormula:{[x;y;z;u] (x*y) - (z*u)};
h:hopen `:127.0.0.1:5004;
PairLeg1Symbol:`zn3M;PairLeg2Symbol:`ZS3M;PairLeg3Symbol:`UC1M;PairLeg4Symbol:`USDCNH;
////////////////////////////////////////////////////
/////多腿数据合并                             //////
////////////////////////////////////////////////////
PairLeg1:   h"-500#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = `",string PairLeg1Symbol;
PairLeg2:   h"-500#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = `",string PairLeg2Symbol;
PairLeg3:   h"-500#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = `",string PairLeg3Symbol;
PairLeg4:   h"-500#select  Date: Date, ReceiveDate:ReceiveDate, Symbol:Symbol, BidPrice1: BidPrice1, BidVol1:BidVol1, AskPrice1:AskPrice1, AskVol1: AskVol1 from Quote where Symbol = `",string PairLeg4Symbol;
//应该加个时间过滤

PairLeg1:  `Date`ReceiveDate`Symbol`Leg1BP1`Leg1BV1`Leg1SP1`Leg1SV1 xcol PairLeg1;
PairLeg2:  `Date`ReceiveDate`Symbol`Leg2BP1`Leg2BV1`Leg2SP1`Leg2SV1 xcol PairLeg2;
PairLeg3:`Date`ReceiveDate`Symbol`Leg3BP1`Leg3BV1`Leg3SP1`Leg3SV1 xcol PairLeg3;
PairLeg4:`Date`ReceiveDate`Symbol`Leg4BP1`Leg4BV1`Leg4SP1`Leg4SV1 xcol PairLeg4;
PairLeg1: update Leg2BP1:0Nf,Leg2BV1:0N,Leg2SP1:0Nf,Leg2SV1:0N,Leg3BP1:0Nf,Leg3BV1:0N,Leg3SP1:0Nf,Leg3SV1:0N,Leg4BP1:0Nf,Leg4BV1:0N ,Leg4SP1:0Nf,Leg4SV1:0N from PairLeg1;
PairLeg2: update Leg1BP1:0Nf,Leg1BV1:0N,Leg1SP1:0Nf,Leg1SV1:0N,Leg3BP1:0Nf,Leg3BV1:0N,Leg3SP1:0Nf,Leg3SV1:0N,Leg4BP1:0Nf,Leg4BV1:0N ,Leg4SP1:0Nf,Leg4SV1:0N from PairLeg2;
PairLeg3:update Leg1BP1:0Nf,Leg1BV1:0N,Leg1SP1:0Nf,Leg1SV1:0N,Leg2BP1:0Nf,Leg2BV1:0N,Leg2SP1:0Nf,Leg2SV1:0N,Leg4BP1:0Nf,Leg4BV1:0N ,Leg4SP1:0Nf,Leg4SV1:0N from PairLeg3;
PairLeg4:update Leg1BP1:0Nf,Leg1BV1:0N,Leg1SP1:0Nf,Leg1SV1:0N,Leg2BP1:0Nf,Leg2BV1:0N,Leg2SP1:0Nf,Leg2SV1:0N,Leg3BP1:0Nf,Leg3BV1:0N ,Leg3SP1:0Nf,Leg3SV1:0N from PairLeg4;
PairLeg1:`Date`ReceiveDate`Symbol`Leg1BP1`Leg1BV1`Leg1SP1`Leg1SV1`Leg2BP1`Leg2BV1`Leg2SP1`Leg2SV1`Leg3BP1`Leg3BV1`Leg3SP1`Leg3SV1`Leg4BP1`Leg4BV1`Leg4SP1`Leg4SV1 xcols PairLeg1;
PairLeg2:`Date`ReceiveDate`Symbol`Leg1BP1`Leg1BV1`Leg1SP1`Leg1SV1`Leg2BP1`Leg2BV1`Leg2SP1`Leg2SV1`Leg3BP1`Leg3BV1`Leg3SP1`Leg3SV1`Leg4BP1`Leg4BV1`Leg4SP1`Leg4SV1 xcols PairLeg2;
PairLeg3:`Date`ReceiveDate`Symbol`Leg1BP1`Leg1BV1`Leg1SP1`Leg1SV1`Leg2BP1`Leg2BV1`Leg2SP1`Leg2SV1`Leg3BP1`Leg3BV1`Leg3SP1`Leg3SV1`Leg4BP1`Leg4BV1`Leg4SP1`Leg4SV1 xcols PairLeg3;
PairLeg4:`Date`ReceiveDate`Symbol`Leg1BP1`Leg1BV1`Leg1SP1`Leg1SV1`Leg2BP1`Leg2BV1`Leg2SP1`Leg2SV1`Leg3BP1`Leg3BV1`Leg3SP1`Leg3SV1`Leg4BP1`Leg4BV1`Leg4SP1`Leg4SV1 xcols PairLeg4;
tempMultiplePair:`ReceiveDate xasc PairLeg1,PairLeg2, PairLeg3, PairLeg4;
MultiplePair:update Leg1BP1:fills  Leg1BP1,Leg1BV1:fills  Leg1BV1,Leg1SP1:fills  Leg1SP1,Leg1SV1:fills  Leg1SV1,Leg2BP1:fills  Leg2BP1,Leg2BV1:fills  Leg2BV1,Leg2SP1:fills  Leg2SP1,Leg2SV1:fills  Leg2SV1,Leg3BP1:fills  Leg3BP1,Leg3BV1:fills  Leg3BV1,Leg3SP1:fills  Leg3SP1,Leg3SV1:fills  Leg3SV1,Leg4BP1:fills  Leg4BP1,Leg4BV1:fills  Leg4BV1,Leg4SP1:fills  Leg4SP1,Leg4SV1:fills  Leg4SV1  from tempMultiplePair;
MultiplePair:`Date`ReceiveDate`Symbol`Leg1BidPrice1`Leg1BidVol1`Leg1AskPrice1`Leg1AskVol1`Leg2BidPrice1`Leg2BidVol1`Leg2AskPrice1`Leg2AskVol1`Leg3BidPrice1`Leg3BidVol1`Leg3AskPrice1`Leg3AskVol1`Leg4BidPrice1`Leg4BidVol1`Leg4AskPrice1`Leg4AskVol1 xcol MultiplePair;
update BidPrice1:PairFormula[Leg1BidPrice1;0.85;Leg2AskPrice1;(Leg3AskPrice1 + Leg4AskPrice1)], BidVol1:1,AskPrice1:PairFormula[Leg1AskPrice1;0.85;Leg2BidPrice1;(Leg3BidPrice1 + Leg4BidPrice1)], AskVol1:1 from `MultiplePair;
MultiplePair:select from MultiplePair where Leg1BidPrice1<>0N;
MultiplePair:select from MultiplePair where Leg2BidPrice1<>0N;
MultiplePair:select from MultiplePair where Leg3BidPrice1<>0N;
MultiplePair:select from MultiplePair where Leg4BidPrice1<>0N;
MultiplePairBar:select by Date.date, 60 xbar Date.second from MultiplePair;
CombinedDataExisted:isTable2 `CombinedData;
if[CombinedDataExisted = 1;CombinedData:CombinedData,MultiplePairBar;]
if[CombinedDataExisted = 0;CombinedData:MultiplePairBar;];
CombinedData:select by Date.date, 60 xbar Date.second from CombinedData;
delete from `CombinedData where Date.second > 00:59:59, Date.second < 09:00:00;
delete from `CombinedData where Date.second > 10:14:59, Date.second < 10:30:00;
delete from `CombinedData where Date.second > 11:29:59, Date.second < 13:30:00;
delete from `CombinedData where Date.second > 14:59:59, Date.second < 21:00:00;
select Date, BidPrice1, AskPrice1 from CombinedData