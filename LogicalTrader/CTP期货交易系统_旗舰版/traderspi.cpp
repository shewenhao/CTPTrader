#include "ThostFtdcTraderApi.h"
#include "traderspi.h"
#include "windows.h"
#include "mdspi.h"
#include "StructFunction.h"
#include "strategy.h"
#include <fstream>

#pragma warning(disable :4996)

extern int requestId; //请求编号
extern HANDLE g_hEvent;//多线程用到的句柄
extern Strategy* g_strategy;//策略类指针


int	 frontId;	//前置编号
int	 sessionId;	//会话编号
char orderRef[13];//报单引用

char MapDirection(char src, bool toOrig);
char MapOffset(char src, bool toOrig);

int IndexFuturePositionLimit = 20;

CThostFtdcDepthMarketDataField *pDepthMarketDataTD;






void CtpTraderSpi::OnFrontConnected()
{
	cerr<<"Trader Init()调用成功"<<"Trader 连接交易前置...成功"<<endl;

	//配置品种开平规则
	define_TThostFtdcCombOffsetFlagType();

	//登录期货账号
	ReqUserLogin(m_appId, m_userId, m_passwd);

	SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqUserLogin(TThostFtdcBrokerIDType	vAppId,
	TThostFtdcUserIDType	vUserId,	TThostFtdcPasswordType	vPasswd)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, vAppId); 
	strcpy(req.UserID, vUserId);  
	strcpy(req.Password, vPasswd);
	int ret = m_pUserApi_td->ReqUserLogin(&req, ++requestId);
	cerr<<"Trader 请求 | 发送登录..."<<((ret == 0) ? "成功" :"失败") << endl;	
}


void CtpTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if ( !IsErrorRspInfo(pRspInfo) && pRspUserLogin ) {  
		// 保存会话参数
		frontId = pRspUserLogin->FrontID;
		sessionId = pRspUserLogin->SessionID;
		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		sprintf(orderRef, "%d", ++nextOrderRef);

		cerr<<"TD_frontId:"<<frontId<<" sessionId:"<<sessionId<<" orderRef:"<<orderRef<<endl;

		cerr<<"交易模块登录成功"<<endl<<endl;

		cerr<<"Trader 响应 | 登录成功...当前交易日:"
			<<pRspUserLogin->TradingDay<<endl;

		m_tradeDate = pRspUserLogin->TradingDay;

		ReqSettlementInfoConfirm();//结算单确认

	}

	if(bIsLast) SetEvent(g_hEvent);
}


void CtpTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_appId);
	strcpy(req.InvestorID, m_userId);
	int ret = m_pUserApi_td->ReqSettlementInfoConfirm(&req, ++requestId);

	cerr<<" 请求 | 发送结算单确认..."<<((ret == 0)?"成功":"失败")<<endl;
}


void CtpTraderSpi::OnRspSettlementInfoConfirm(
	CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){

		cerr<<" 响应 | 结算单..."<<pSettlementInfoConfirm->InvestorID
			<<"...<"<<pSettlementInfoConfirm->ConfirmDate
			<<" "<<pSettlementInfoConfirm->ConfirmTime<<">...确认"<<endl<<endl;


		cerr<<"结算单确认正常，首次查询报单:"<<endl;
		Sleep(3000);//可适当增加暂停时间，让CTP柜台有充足的响应时间，然后才开始进行查询操作		
		ReqQryOrder();
	}
	if(bIsLast) SetEvent(g_hEvent);
}


///请求查询合约信息
void CtpTraderSpi::ReqQryInstrument(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.InstrumentID, instId);//为空表示查询所有合约
	int ret = m_pUserApi_td->ReqQryInstrument(&req, ++requestId);
	cerr<<" 请求 | 发送合约查询..."<<((ret == 0)?"成功":"失败")<<endl;
}


///请求查询合约信息，所有合约
void CtpTraderSpi::ReqQryInstrument_all()
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));

	int ret = m_pUserApi_td->ReqQryInstrument(&req, ++requestId);
	cerr<<" 请求 | 发送合约查询..."<<((ret == 0)?"成功":"失败")<<endl;
	cerr<<" ret:"<<ret<<endl;
}




//请求查询合约响应，查询合约结束后，TD初始化结束
void CtpTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 	
	if ( !IsErrorRspInfo(pRspInfo) &&  pInstrument)
	{
		if(first_inquiry_Instrument == true)
		{
			m_Instrument_all = m_Instrument_all + pInstrument->InstrumentID + ",";

			//保存所有合约的信息到map
			CThostFtdcInstrumentField* instField = new CThostFtdcInstrumentField();
			memcpy(instField,  pInstrument, sizeof(CThostFtdcInstrumentField));
			m_instMessage_map.insert(pair<string, CThostFtdcInstrumentField*> (instField->InstrumentID, instField));


			//策略交易的合约
			if(strcmp(m_instId, pInstrument->InstrumentID) == 0)
			{
				cerr<<" 响应 | 合约:"<<pInstrument->InstrumentID
					<<" 合约名称:"<<pInstrument->InstrumentName
					<<" 合约在交易所的代码:"<<pInstrument->ExchangeInstID
					<<" 产品代码:"<<pInstrument->ProductID
					<<" 产品类型:"<<pInstrument->ProductClass
					<<" 交割月:"<<pInstrument->DeliveryMonth
					<<" 多头保证金率:"<<pInstrument->LongMarginRatio
					<<" 空头保证金率:"<<pInstrument->ShortMarginRatio
					<<" 合约数量乘数:"<<pInstrument->VolumeMultiple
					<<" 最小变动价位:"<<pInstrument->PriceTick
					<<" 交易所代码:"<<pInstrument->ExchangeID
					<<" 交割年份:"<<pInstrument->DeliveryYear
					<<" 交割月:"<<pInstrument->DeliveryMonth
					<<" 创建日:"<<pInstrument->CreateDate
					<<" 上市日:"<<pInstrument->OpenDate
					<<" 到期日:"<<pInstrument->ExpireDate
					<<" 开始交割日:"<<pInstrument->StartDelivDate
					<<" 结束交割日:"<<pInstrument->EndDelivDate
					<<" 合约生命周期状态:"<<pInstrument->InstLifePhase
					<<" 当前是否交易:"<<pInstrument->IsTrading
					<<endl;

			}


			if(bIsLast)
			{
				first_inquiry_Instrument = false;

				m_Instrument_all = m_Instrument_all.substr(0,m_Instrument_all.length()-1);

				cerr<<"m_Instrument_all大小："<<m_Instrument_all.length()<<","<<m_Instrument_all<<endl;

				cerr<<"map大小(合约个数）："<<m_instMessage_map.size()<<endl;

				//将合约-合约信息结构体的map复制到策略类
				g_strategy->set_instMessage_map_stgy(m_instMessage_map);


				cerr<<"----------------------------输出合约信息map的内容----------------------------"<<endl;
				//showInstMessage();

				//保存全市场合约，在TD进行，需要订阅全市场合约行情时再运行
				//m_MDSpi->set_instIdList_all(m_Instrument_all);


				cerr<<"TD初始化完成，启动MD:"<<endl;
				m_pMDUserApi_td->Init();
				
				SetEvent(g_hEvent);
			}

		}



	}


}



void CtpTraderSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_appId);
	strcpy(req.InvestorID, m_userId);
	int ret = m_pUserApi_td->ReqQryTradingAccount(&req, ++requestId);
	cerr<<" 请求 | 发送资金查询..."<<((ret == 0)?"成功":"失败")<<endl;

}



//请求查询资金响应
void CtpTraderSpi::OnRspQryTradingAccount(
	CThostFtdcTradingAccountField *pTradingAccount,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr<<"请求查询资金响应:OnRspQryTradingAccount"<<" pTradingAccount:"<<pTradingAccount<<endl;

	if (!IsErrorRspInfo(pRspInfo) &&  pTradingAccount)
	{
		cerr<<"响应 | 投资者帐号:"<<pTradingAccount->AccountID
			<<" 上次结算准备金(静态权益，期初权益):"<<pTradingAccount->PreBalance
			<<" 期货结算准备金(动态权益):"<<pTradingAccount->Balance
			<<" 可用资金:"<<pTradingAccount->Available
			<<" 可取资金:"<<pTradingAccount->WithdrawQuota
			<<" 当前保证金总额:"<<pTradingAccount->CurrMargin
			<<" 平仓盈亏:"<<pTradingAccount->CloseProfit
			<<" 持仓盈亏"<<pTradingAccount->PositionProfit
			<<" 手续费:"<<pTradingAccount->Commission
			<<" 冻结保证金:"<<pTradingAccount->FrozenMargin
			<< endl;



		if(firs_inquiry_TradingAccount == true)
		{
			firs_inquiry_TradingAccount = false;
			Sleep(1000);

			cerr<<"资金查询正常，查询投资者持仓:"<<endl;		
			
			ReqQryInvestorPosition_all();//查询所有合约持仓信息
			
			///ReqQryInvestorPosition(m_instId);//查询单合约持仓信息
		}


	}

	else
	{
		if(firs_inquiry_TradingAccount == true)
		{
			firs_inquiry_TradingAccount = false;
			Sleep(1000);
			
			cerr<<"资金查询出错,查询投资者持仓:"<<endl;			
			
			ReqQryInvestorPosition_all();//查询所有合约持仓信息
			
			///ReqQryInvestorPosition(m_instId);//查询单合约持仓信息
		}

	}
	if(bIsLast) SetEvent(g_hEvent);

	cerr<<"-----------------------------------------------请求查询资金响应结束"<<endl;
}

///请求查询投资者持仓
void CtpTraderSpi::ReqQryInvestorPosition(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_appId);
	strcpy(req.InvestorID, m_userId);
	strcpy(req.InstrumentID, instId);	
	int ret = m_pUserApi_td->ReqQryInvestorPosition(&req, ++requestId);
	cerr<<" 请求 | 发送持仓查询..."<<((ret == 0)?"成功":"失败")<<endl;
}




///请求查询投资者持仓,所有合约的持仓
void CtpTraderSpi::ReqQryInvestorPosition_all()
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	//strcpy(req.BrokerID, appId);
	//strcpy(req.InvestorID, userId);
	//strcpy(req.InstrumentID, instId);
	int ret = m_pUserApi_td->ReqQryInvestorPosition(&req, ++requestId);
	cerr<<" 请求 | 发送持仓查询..."<<((ret == 0)?"成功":"失败")<<endl;
}




//持仓查询回调函数,已经平仓的单子，持仓量为0了还会返回
void CtpTraderSpi::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField *pInvestorPosition, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//cout<<"持仓查询回调函数OnRspQryInvestorPosition()"<<endl;

	if( !IsErrorRspInfo(pRspInfo) &&  pInvestorPosition)
	{
		//账号下所有合约
		if(firs_inquiry_Position == true)
		{
			cerr<<"请求查询持仓响应:OnRspQryInvestorPosition"<<" pInvestorPosition:"<<pInvestorPosition<<endl;//会包括已经平仓而没有持仓了的持仓记录，按理应该过滤掉

			cerr<<" 响应 | 合约:"<<pInvestorPosition->InstrumentID<<endl
				<<" 持仓多空方向:"<<pInvestorPosition->PosiDirection<<endl//2多3空
				<<" 映射后的方向:"<<MapDirection(pInvestorPosition->PosiDirection-2,false)<<endl
				<<" 总持仓(今日持仓,错误名字):"<<pInvestorPosition->Position<<endl
				<<" 上日持仓:"<<pInvestorPosition->YdPosition<<endl
				<<" 今日持仓:"<<pInvestorPosition->TodayPosition<<endl
				<<" 保证金:"<<pInvestorPosition->UseMargin<<endl
				<<" 持仓成本:"<<pInvestorPosition->PositionCost<<endl
				<<" 开仓量:"<<pInvestorPosition->OpenVolume<<endl
				<<" 平仓量:"<<pInvestorPosition->CloseVolume<<endl
				<<" 持仓日期:"<<pInvestorPosition->PositionDate<<endl//返回1表示今仓，2表示昨仓
				<<" 平仓盈亏(按昨结):"<<pInvestorPosition->CloseProfit<<endl
				<<" 持仓盈亏:"<<pInvestorPosition->PositionProfit<<endl
				<<" 逐日盯市平仓盈亏(按昨结):"<<pInvestorPosition->CloseProfitByDate<<endl//快期显示的是这个字段的值
				<<" 逐笔对冲平仓盈亏(按开平仓价):"<<pInvestorPosition->CloseProfitByTrade<<endl//此字段在交易中比较有意义
				<<endl;


			//构造合约对应交易信息的结构体的map

			bool find_trade_message_map = false;

			for(map<string, trade_message*>::iterator iter = m_trade_message_map.begin(); iter!= m_trade_message_map.end();iter++)
			{
				if(strcmp( (iter->first).c_str(), pInvestorPosition->InstrumentID)==0)//合约已存在
				{
					find_trade_message_map = true;
					break;
				}
			}

			if(!find_trade_message_map)
			{
				cerr<<"--------------------------------------------没有这个合约，构造交易信息结构体"<<endl;

				trade_message* trade_message_p = new trade_message();


				trade_message_p->instId = pInvestorPosition->InstrumentID;

				m_trade_message_map.insert(pair<string, trade_message*> (pInvestorPosition->InstrumentID, trade_message_p));


				m_trade_message_map[pInvestorPosition->InstrumentID]->holding_long = 0;

				//多单今仓
				m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_long = 0;

				//多单昨仓 = 多单持仓量 - 多单今仓
				m_trade_message_map[pInvestorPosition->InstrumentID]->YdPosition_long = 0;

				//多单平仓盈亏
				m_trade_message_map[pInvestorPosition->InstrumentID]->closeProfit_long = 0;

				//多单浮动盈亏(其实是持仓盈亏，按昨结算的)
				m_trade_message_map[pInvestorPosition->InstrumentID]->OpenProfit_long = 0;

				m_trade_message_map[pInvestorPosition->InstrumentID]->holding_short = 0;

				//空单今仓
				m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_short = 0;

				//空单昨仓 = 空单持仓量 - 空单今仓
				m_trade_message_map[pInvestorPosition->InstrumentID]->YdPosition_short = 0;

				//空单平仓盈亏
				m_trade_message_map[pInvestorPosition->InstrumentID]->closeProfit_short = 0;

				//空单浮动盈亏
				m_trade_message_map[pInvestorPosition->InstrumentID]->OpenProfit_short = 0;
			}


			if(pInvestorPosition->PosiDirection == '2')//多单
			{
				//多单持仓量
				m_trade_message_map[pInvestorPosition->InstrumentID]->holding_long += pInvestorPosition->Position;

				//多单今仓
				m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_long += pInvestorPosition->TodayPosition;

				//多单昨仓 = 多单持仓量 - 多单今仓
				//m_trade_message_map[pInvestorPosition->InstrumentID]->YdPosition_long = pInvestorPosition->Position - m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_long;//也可以
				m_trade_message_map[pInvestorPosition->InstrumentID]->YdPosition_long += pInvestorPosition->Position - pInvestorPosition->TodayPosition;

				//多单平仓盈亏
				m_trade_message_map[pInvestorPosition->InstrumentID]->closeProfit_long +=  pInvestorPosition->CloseProfit;

				//多单浮动盈亏(其实是持仓盈亏，按昨结算的)
				m_trade_message_map[pInvestorPosition->InstrumentID]->OpenProfit_long += pInvestorPosition->PositionProfit;

			}
			else if(pInvestorPosition->PosiDirection == '3')//空单
			{
				//空单持仓量
				m_trade_message_map[pInvestorPosition->InstrumentID]->holding_short += pInvestorPosition->Position;

				//空单今仓
				m_trade_message_map[pInvestorPosition->InstrumentID]->TodayPosition_short += pInvestorPosition->TodayPosition;

				//空单昨仓 = 空单持仓量 - 空单今仓
				m_trade_message_map[pInvestorPosition->InstrumentID]->YdPosition_short += pInvestorPosition->Position - pInvestorPosition->TodayPosition;

				//空单平仓盈亏
				m_trade_message_map[pInvestorPosition->InstrumentID]->closeProfit_short += pInvestorPosition->CloseProfit;

				//空单浮动盈亏
				m_trade_message_map[pInvestorPosition->InstrumentID]->OpenProfit_short += pInvestorPosition->PositionProfit;
			}

			if (bIsLast)
			{
				firs_inquiry_Position = false;


				for(map<string, trade_message*>::iterator iter = m_trade_message_map.begin(); iter != m_trade_message_map.end(); iter++)
				{
					cerr<<"合约代码:"<< iter->first<<","<<iter->second->instId<<endl
						<<" 多单持仓量:"<<iter->second->holding_long<<endl
						<<" 空单持仓量:"<<iter->second->holding_short<<endl
						<<" 多单今日持仓:"<<iter->second->TodayPosition_long<<endl
						<<" 多单上日持仓:"<<iter->second->YdPosition_long<<endl
						<<" 空单今日持仓:"<<iter->second->TodayPosition_short<<endl
						<<" 空单上日持仓:"<<iter->second->YdPosition_short<<endl
						<<" 多单平仓盈亏:"<<iter->second->closeProfit_long<<endl
						<<" 多单浮动盈亏:"<<iter->second->OpenProfit_long<<endl
						<<" 空单平仓盈亏:"<<iter->second->closeProfit_short<<endl
						<<" 空单浮动盈亏:"<<iter->second->OpenProfit_short<<endl
						<<endl;

					//cerr<<"map元素个数:"<<m_trade_message_map.size()<<","<<"多单平仓盈亏:"<<iter->second->closeProfit_long<<","<<"空单平仓盈亏:"<<iter->second->closeProfit_short<<",账户平仓盈亏"<<m_closeProfit<<endl;
					
					
					//账户平仓盈亏
					m_closeProfit = m_closeProfit + iter->second->closeProfit_long + iter->second->closeProfit_short;
					
					//账户浮动盈亏
					m_OpenProfit = m_OpenProfit + iter->second->OpenProfit_long + iter->second->OpenProfit_short;


				}

				cerr<<"查询持仓返回结束:"<<endl;
				cerr<<"账户平仓盈亏:"<<m_closeProfit<<endl
					<<"账户浮动盈亏:"<<m_OpenProfit<<endl
					<<endl;

				Sleep(1000);
				cerr<<"查询持仓正常，首次查询合约（所有合约）:"<<endl;		
				ReqQryInstrument_all();

			}

		}



	}
	else
	{
		if(firs_inquiry_Position == true)
		{
			firs_inquiry_Position = false;

			cerr<<"查询持仓出错，或没有持仓，首次查询合约（所有合约）:"<<endl;
			Sleep(1000);
			ReqQryInstrument_all();
		}

	}

	cerr<<"-----------------------------------------------查询持仓返回单次结束"<<endl;
}





void CtpTraderSpi::ReqOrderInsert(TThostFtdcInstrumentIDType instId,
	TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
	TThostFtdcPriceType price,   TThostFtdcVolumeType vol)
{
	CThostFtdcInputOrderField req;
	string sentFailedStrOrderRef = orderRef;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, m_appId);  //应用单元代码	
	strcpy(req.InvestorID, m_userId); //投资者代码	
	strcpy(req.InstrumentID, instId); //合约代码
	strcpy(req.OrderRef, orderRef);  //报单引用
	int nextOrderRef = atoi(orderRef);
	sprintf(orderRef, "%d", ++nextOrderRef);
	
	 
	req.LimitPrice = price;	//价格
	if(0==req.LimitPrice){
		req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;//价格类型=市价
		req.TimeCondition = THOST_FTDC_TC_IOC;//有效期类型:立即完成，否则撤销
	}else{
		req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;//价格类型=限价	
		req.TimeCondition = THOST_FTDC_TC_GFD;  //有效期类型:当日有效
	}

	req.Direction = MapDirection(dir,true);  //买卖方向	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //组合开平标志:开仓

	//这样也可以
	/*
	req.Direction = dir;
	req.CombOffsetFlag[0] = kpp[0];
	*/

	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	  //组合投机套保标志	
	req.VolumeTotalOriginal = vol;	///数量		
	req.VolumeCondition = THOST_FTDC_VC_AV; //成交量类型:任何数量
	req.MinVolume = 1;	//最小成交量:1	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  //触发条件:立即

	//TThostFtdcPriceType	StopPrice;  //止损价
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//强平原因:非强平	
	req.IsAutoSuspend = 0;  //自动挂起标志:否	
	req.UserForceClose = 0;   //用户强评标志:否

	int ret = m_pUserApi_td->ReqOrderInsert(&req, ++requestId);


	m_orderRef_orderreq.insert(std::pair<int, CThostFtdcInputOrderField>(nextOrderRef - 1 , req));


	cerr<<" 请求 | 发送报单..."<<((ret == 0)?"成功":"失败")<< endl;
	cerr << " ReqOrderInsert函数内部:" << "instId:" << instId << " dir:" << dir << " kpp:" << kpp << " price:" << price << " vol:" << vol << endl;

	
}

void CtpTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInputOrder ){
		cerr<<"响应 | 报单提交成功...报单引用:"<<pInputOrder->OrderRef<<endl; 
	}
	else
	{
		if (pRspInfo->ErrorID == 30 || pRspInfo->ErrorID == 50 || pRspInfo->ErrorID == 51)
		{
			int errorOrderRef = atoi(pInputOrder->OrderRef);
			int nextOrderRef = atoi(orderRef);

			m_frontsessionref_ordertype.insert(std::pair<string, vector<int>>(to_string(frontId) + to_string(sessionId) + to_string(nextOrderRef), m_frontsessionref_ordertype[to_string(frontId) + to_string(sessionId) + to_string(errorOrderRef)]));
			m_frontsessionref_order_type.insert(std::pair<string, string>(to_string(frontId) + to_string(sessionId) + to_string(nextOrderRef),"ALL_OPEN"));

			TThostFtdcCombOffsetFlagType openkpp = "0";
			(m_orderRef_orderreq[errorOrderRef]).CombOffsetFlag[0] = MapOffset(openkpp[0], ++requestId);

			/*ReqOrderInsert((m_orderRef_orderreq[errorOrderRef]).InstrumentID, (m_orderRef_orderreq[errorOrderRef]).Direction, (m_orderRef_orderreq[errorOrderRef]).CombOffsetFlag, (m_orderRef_orderreq[errorOrderRef]).LimitPrice, (m_orderRef_orderreq[errorOrderRef]).VolumeTotalOriginal);*/


			SendOrderDecoration((m_orderRef_orderreq[errorOrderRef]).InstrumentID, "ALL_OPEN", (m_orderRef_orderreq[errorOrderRef]).Direction, &(m_orderRef_orderreq[errorOrderRef]).CombOffsetFlag, pDepthMarketDataTD->AskPrice1, pDepthMarketDataTD->BidPrice1, (m_orderRef_orderreq[errorOrderRef]).LimitPrice, (m_orderRef_orderreq[errorOrderRef]).VolumeTotalOriginal, m_frontsessionref_ordertype[to_string(frontId) + to_string(sessionId) + to_string(errorOrderRef)], pDepthMarketDataTD);

		}
	}

	if(bIsLast) SetEvent(g_hEvent);	
}

void CtpTraderSpi::ReqOrderAction(TThostFtdcSequenceNoType orderSeq)//经纪公司报单编号,int
{
	bool found=false; unsigned int i=0;
	for(i=0;i<orderList.size();i++){
		if(orderList[i]->BrokerOrderSeq == orderSeq){ found = true; break;}
	}
	if(!found){cerr<<" 请求 | 报单不存在."<<endl; return;} 

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_appId);   //经纪公司代码
	strcpy(req.InvestorID, m_userId); //投资者代码
	//strcpy(req.OrderRef, pOrderRef); //报单引用
	//req.FrontID = frontId;           //前置编号
	//req.SessionID = sessionId;       //会话编号
	strcpy(req.ExchangeID, orderList[i]->ExchangeID);//交易所代码
	strcpy(req.OrderSysID, orderList[i]->OrderSysID);//报单编号
	req.ActionFlag = THOST_FTDC_AF_Delete;  //操作标志,撤单

	int ret = m_pUserApi_td->ReqOrderAction(&req, ++requestId);
	cerr<< " 请求 | 发送撤单..." <<((ret == 0)?"成功":"失败") << endl;
}

void CtpTraderSpi::OnRspOrderAction(
	CThostFtdcInputOrderActionField *pInputOrderAction, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	cerr<<"OnRspOrderAction:要撤单有误才执行"<<endl;
	if (!IsErrorRspInfo(pRspInfo) && pInputOrderAction){
		cerr<< " 响应 | 撤单成功..."
			<< "交易所:"<<pInputOrderAction->ExchangeID
			<<" 报单编号:"<<pInputOrderAction->OrderSysID
			<<" 报单操作引用:"<<pInputOrderAction->OrderActionRef
			<<" 报单引用:"<<pInputOrderAction->OrderRef
			<<" 请求编号:"<<pInputOrderAction->RequestID
			<<" 前置编号:"<<pInputOrderAction->FrontID
			<<" 会话编号:"<<pInputOrderAction->SessionID
			<<" 操作标志:"<<pInputOrderAction->ActionFlag
			<<" 价格:"<<pInputOrderAction->LimitPrice
			<<" 数量变化:"<<pInputOrderAction->VolumeChange
			<<" 合约代码:"<<pInputOrderAction->InstrumentID
			<<endl;
	}


	//撤单成功后重新发委托

	if(bIsLast) SetEvent(g_hEvent);	
}

///报单回报
void CtpTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{	

	cerr<<"报单回报:前置编号FrontID:"<<pOrder->FrontID<<" 会话编号SessionID:"<<pOrder->SessionID<<" OrderRef:"<<pOrder->OrderRef<<endl;


	//所有合约
	CThostFtdcOrderField* order = new CThostFtdcOrderField();
	memcpy(order,  pOrder, sizeof(CThostFtdcOrderField));
	bool founded=false;    unsigned int i=0;
	for(i=0; i<orderList.size(); i++){
		if(orderList[i]->BrokerOrderSeq == order->BrokerOrderSeq) {//经纪公司报单编号
			founded=true;    break;
		}
	}
	if(founded) orderList[i]= order;
	else  orderList.push_back(order);

	cerr<<"---------------------------------------------------------"<<endl;
	cerr<<" 回报 | 报单已提交...序号(经纪公司报单编号):"<<order->BrokerOrderSeq<<endl//经纪公司报单编号，撤单时可以找到单子
		<<" 报单编号(同成交回报):"<<order->OrderSysID<<endl//与成交回报同
		<<" 报单状态："<<order->OrderStatus<<endl
		<<" 状态信息:"<<order->StatusMsg<<endl
		<<" 数量:"<<order->VolumeTotalOriginal<<endl
		<<" 剩余数量:"<<order->VolumeTotal<<endl
		<<" 合约代码:"<<order->InstrumentID<<endl
		<<" 报单价格条件:"<<order->OrderPriceType<<endl
		<<" 买卖方向:"<<order->Direction<<endl
		<<" 组合开平标志:"<<order->CombOffsetFlag<<endl
		//<<" 组合投机套保标志:"<<order->CombHedgeFlag<<endl
		<<" 价格:"<<order->LimitPrice<<endl
		//<<" 报单提交状态:"<<order->OrderSubmitStatus<<endl
		//<<" 报单类型:"<<order->OrderType<<endl
		//<<" 用户强评标志:"<<order->UserForceClose<<endl
		<<endl;
	cerr<<"---------------------------------------------------------"<<endl;




	//已撤单成功，可在这边追单
	if(order->OrderStatus == '5')
	{
		string strOrderRef = order->OrderRef;
		TThostFtdcInstrumentIDType    instId;//合约,合约代码在结构体里已经有了
		TThostFtdcDirectionType       dir;//方向,'0'买，'1'卖
		TThostFtdcCombOffsetFlagType  kpp;//开平，"0"开，"1"平,"3"平今
		TThostFtdcPriceType           price = 0;//价格，0是市价,上期所不支持
		TThostFtdcVolumeType          vol;//数量
		strcpy(instId, order->InstrumentID);
		string instIdStr(instId);
		dir = order->Direction;
		strcpy(kpp, order->CombOffsetFlag);
		vol = order->VolumeTotal;//要用剩余数量
		if (order->FrontID == frontId && order->SessionID == sessionId)
		{
			if (m_frontsessionref_frontsessionref.find(to_string(frontId) + to_string(sessionId) + strOrderRef) == m_frontsessionref_frontsessionref.end())
			{
				if (m_frontsessionref_ordertype.find(to_string(frontId) + to_string(sessionId) + strOrderRef) == m_frontsessionref_ordertype.end())
				{

				}
				else if (m_frontsessionref_ordertype[to_string(frontId) + to_string(sessionId) + strOrderRef][2] < 86400)
				{
					SendOrderDecoration(instId, m_frontsessionref_order_type[to_string(frontId) + to_string(sessionId) + strOrderRef], dir, &kpp, pDepthMarketDataTD->AskPrice1, pDepthMarketDataTD->BidPrice1,  price, vol, m_frontsessionref_ordertype[to_string(frontId) + to_string(sessionId) + strOrderRef], pDepthMarketDataTD);
					m_frontsessionref_frontsessionref.insert(pair<string, CThostFtdcOrderField*>(to_string(frontId) + to_string(sessionId) + strOrderRef, pOrder));
				}

			}
			else
			{

			}
		}

	}
	SetEvent(g_hEvent);	

}


///成交通知
void CtpTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	cerr<<"成交通知:OnRtnTrade:"<<pTrade->InstrumentID<<" 报单引用OrderRef:"<<pTrade->OrderRef<<" 报单编号(同成交回报):"<<pTrade->OrderSysID<<" 成交编号:"<<pTrade->TradeID<<" 本地报单编号:"<<pTrade->OrderLocalID<<" 经纪公司报单编号:"<<pTrade->BrokerOrderSeq<<endl;


	//账户下的所有合约

	//新成交的合约，要订阅行情，才能准确计算账户盈亏信息

	bool find_instId_Trade = false;

	for(unsigned int i = 0; i< subscribe_inst_vec.size(); i++)
	{
		if(strcmp(subscribe_inst_vec[i].c_str(), pTrade->InstrumentID) == 0)//合约已存在，已订阅过行情
		{
			find_instId_Trade = true;
			break;
		}
	}

	if(!find_instId_Trade)
	{
		cerr<<"-------------------------------------------------------------OnRtnTrade,新成交的合约，订阅行情："<<endl;
		m_MDSpi->SubscribeMarketData(pTrade->InstrumentID);
		subscribe_inst_vec.push_back(pTrade->InstrumentID);
	}




	//新成交的合约，要构造合约对应交易信息的结构体的map,用count就可以判断

	bool find_trade_message_map_onTrade = false;

	for(map<string, trade_message*>::iterator iter = m_trade_message_map.begin(); iter!= m_trade_message_map.end();iter++)
	{
		if(strcmp( (iter->first).c_str(), pTrade->InstrumentID)==0)//合约已存在
		{
			find_trade_message_map_onTrade = true;
			break;
		}
	}

	if(!find_trade_message_map_onTrade)
	{
		cerr<<"-------------------------------------------------------------OnRtnTrade,新成交的合约，建立交易信息map："<<endl;

		trade_message* trade_message_p = new trade_message();

		trade_message_p->instId = pTrade->InstrumentID;

		m_trade_message_map.insert(pair<string, trade_message*> (pTrade->InstrumentID, trade_message_p));
	}





	CThostFtdcTradeField* trade_account = new CThostFtdcTradeField();
	memcpy(trade_account,  pTrade, sizeof(CThostFtdcTradeField));
	bool founded=false;     unsigned int i=0;
	for(i=0; i<tradeList.size(); i++){
		if(tradeList[i]->TradeID == trade_account->TradeID) {
			founded=true;   break;
		}
	}
	if (founded)
	{
		tradeList[i] = trade_account;//多手数分多次成交会有多次的成交通知，但每次成交编号都不一样
	}		 
	else
	{
		tradeList.push_back(trade_account);
	}

	cerr << endl
		<< " 回报 | 报单已成交...成交编号:" << trade_account->TradeID
		<< " 报单编号:" << trade_account->OrderSysID
		<< " 合约代码:" << trade_account->InstrumentID
		<< " 买卖方向:" << trade_account->Direction
		<< " 开平标志:" << trade_account->OffsetFlag
		<< " 投机套保标志:" << trade_account->HedgeFlag
		<< " 价格:" << trade_account->Price
		<< " 数量:" << trade_account->Volume
		<< " 成交时期:" << trade_account->TradeDate
		<< " 成交时间:" << trade_account->TradeTime
		<< " 经纪公司代码:" << trade_account->BrokerID
		<< " 投资者代码:" << trade_account->InvestorID//也是userId
		<< endl;






	int close_num_account_long = 0;//平仓的多单手数，如果有的话
	int close_num_account_short = 0;//平仓的空单手数，如果有的话


	//若是开仓单，则保存到tradeList_notClosed_account_long和tradeList_notClosed_account_short

	if (trade_account->OffsetFlag == '0')//开仓
	{
		if (trade_account->Direction == '0')//多单
		{
			//多单持仓表增加记录
			tradeList_notClosed_account_long.push_back(trade_account);
			//多单持仓量
			m_trade_message_map[trade_account->InstrumentID]->holding_long = m_trade_message_map[trade_account->InstrumentID]->holding_long + trade_account->Volume;
			//多单今日持仓
			m_trade_message_map[trade_account->InstrumentID]->TodayPosition_long = m_trade_message_map[trade_account->InstrumentID]->TodayPosition_long + trade_account->Volume;
		}
		else if (trade_account->Direction == '1')//空单
		{
			//空单持仓表增加记录
			tradeList_notClosed_account_short.push_back(trade_account);
			//空单持仓量
			m_trade_message_map[trade_account->InstrumentID]->holding_short = m_trade_message_map[trade_account->InstrumentID]->holding_short + trade_account->Volume;
			//空单今日持仓
			m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short = m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short + trade_account->Volume;
		}
	}
	//平仓单，删除记录
	else if (trade_account->OffsetFlag == '1' || trade_account->OffsetFlag == '3' || trade_account->OffsetFlag == '4')//1是平仓，3是平今
	{
		if (trade_account->Direction == '1')//卖，表示平多,有昨仓和今仓时，按时间顺序，先平昨仓
		{
			close_num_account_long = trade_account->Volume;

			for (vector<CThostFtdcTradeField*>::iterator iter = tradeList_notClosed_account_long.begin(); iter != tradeList_notClosed_account_long.end(); iter++)
			{
				if (strcmp(trade_account->InstrumentID, (*iter)->InstrumentID) == 0)
				{
					if ((*iter)->Volume < close_num_account_long)//没有忽略掉tradeList_notClosed中数量为0的单子，但不影响计算结果
					{
						close_num_account_long -= (*iter)->Volume;
						//平仓盈亏
						m_trade_message_map[trade_account->InstrumentID]->closeProfit_long = m_trade_message_map[trade_account->InstrumentID]->closeProfit_long + (trade_account->Price - (*iter)->Price) * (*iter)->Volume * m_instMessage_map[trade_account->InstrumentID]->VolumeMultiple;
						(*iter)->Volume = 0;
					}
					else if ((*iter)->Volume == close_num_account_long)
					{
						(*iter)->Volume = 0;
						//平仓盈亏
						m_trade_message_map[trade_account->InstrumentID]->closeProfit_long = m_trade_message_map[trade_account->InstrumentID]->closeProfit_long + (trade_account->Price - (*iter)->Price) * close_num_account_long * m_instMessage_map[trade_account->InstrumentID]->VolumeMultiple;
						break;
					}
					else if ((*iter)->Volume > close_num_account_long)
					{
						(*iter)->Volume = (*iter)->Volume - close_num_account_long;
						//平仓盈亏
						m_trade_message_map[trade_account->InstrumentID]->closeProfit_long = m_trade_message_map[trade_account->InstrumentID]->closeProfit_long + (trade_account->Price - (*iter)->Price) * close_num_account_long * m_instMessage_map[trade_account->InstrumentID]->VolumeMultiple;
						break;
					}

				}
			}

			//多单持仓量
			m_trade_message_map[trade_account->InstrumentID]->holding_long = m_trade_message_map[trade_account->InstrumentID]->holding_long - trade_account->Volume;
			//今仓持仓量和昨仓量，要分上期所和非上期所


			//今仓量和昨仓量，只对上期所有效
			if (trade_account->OffsetFlag == '1' || trade_account->OffsetFlag == '4')
				m_trade_message_map[trade_account->InstrumentID]->YdPosition_long = m_trade_message_map[trade_account->InstrumentID]->YdPosition_long - trade_account->Volume;//昨仓
			else if (trade_account->OffsetFlag == '3')
				m_trade_message_map[trade_account->InstrumentID]->TodayPosition_long = m_trade_message_map[trade_account->InstrumentID]->TodayPosition_long - trade_account->Volume;//今仓


			//假设今仓5手，昨仓1，平仓都是发'1'，假设平仓2手，导致昨仓是-1，今仓还是5手，实际应该是今仓5-1，昨仓0
			//3手昨仓，5手今仓，，'1'平仓了4手,导致昨仓是-1，今仓还是5手，实际应该是今仓5-1，昨仓0

			if (m_trade_message_map[trade_account->InstrumentID]->YdPosition_long < 0)
			{
				m_trade_message_map[trade_account->InstrumentID]->TodayPosition_long = m_trade_message_map[trade_account->InstrumentID]->TodayPosition_long + m_trade_message_map[trade_account->InstrumentID]->YdPosition_long;
				m_trade_message_map[trade_account->InstrumentID]->YdPosition_long = 0;

			}

		}
		else if (trade_account->Direction == '0')//平空
		{
			close_num_account_short = trade_account->Volume;

			for (vector<CThostFtdcTradeField*>::iterator iter = tradeList_notClosed_account_short.begin(); iter != tradeList_notClosed_account_short.end(); iter++)
			{
				if (strcmp(trade_account->InstrumentID, (*iter)->InstrumentID) == 0)
				{
					if ((*iter)->Volume < close_num_account_short)//没有忽略掉tradeList_notClosed中数量为0的单子，但不影响计算结果
					{
						close_num_account_short -= (*iter)->Volume;
						//平仓盈亏
						m_trade_message_map[trade_account->InstrumentID]->closeProfit_short = m_trade_message_map[trade_account->InstrumentID]->closeProfit_short + ((*iter)->Price - trade_account->Price) * (*iter)->Volume * m_instMessage_map[trade_account->InstrumentID]->VolumeMultiple;
						(*iter)->Volume = 0;
					}

					else if ((*iter)->Volume == close_num_account_short)
					{
						(*iter)->Volume = 0;
						//平仓盈亏
						m_trade_message_map[trade_account->InstrumentID]->closeProfit_short = m_trade_message_map[trade_account->InstrumentID]->closeProfit_short + ((*iter)->Price - trade_account->Price) * close_num_account_short * m_instMessage_map[trade_account->InstrumentID]->VolumeMultiple;
						break;
					}

					else if ((*iter)->Volume > close_num_account_short)
					{
						(*iter)->Volume = (*iter)->Volume - close_num_account_short;
						//平仓盈亏
						m_trade_message_map[trade_account->InstrumentID]->closeProfit_short = m_trade_message_map[trade_account->InstrumentID]->closeProfit_short + ((*iter)->Price - trade_account->Price) * close_num_account_short * m_instMessage_map[trade_account->InstrumentID]->VolumeMultiple;
						break;
					}

				}

			}

			//空单持仓量
			m_trade_message_map[trade_account->InstrumentID]->holding_short = m_trade_message_map[trade_account->InstrumentID]->holding_short - trade_account->Volume;

			//空单今日持仓
			//m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short = m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short - trade_account->Volume;



			//今仓量和昨仓量，只对上期所有效
			if (trade_account->OffsetFlag == '1' || trade_account->OffsetFlag == '4')
				m_trade_message_map[trade_account->InstrumentID]->YdPosition_short = m_trade_message_map[trade_account->InstrumentID]->YdPosition_short - trade_account->Volume;//昨仓
			else if (trade_account->OffsetFlag == '3')
				m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short = m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short - trade_account->Volume;//今仓


			if (m_trade_message_map[trade_account->InstrumentID]->YdPosition_short < 0)
			{
				m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short = m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short + m_trade_message_map[trade_account->InstrumentID]->YdPosition_short;
				m_trade_message_map[trade_account->InstrumentID]->YdPosition_short = 0;

			}
		}


	}

}


void CtpTraderSpi::OnFrontDisconnected(int nReason)
{
	cerr<<" 响应 | 连接中断..." 
		<< " reason=" << nReason << endl;
}


void CtpTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr<<" 响应 | 心跳超时警告..." 
		<< " TimerLapse = " << nTimeLapse << endl;
}


void CtpTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << " 请求 |error" << endl;
	IsErrorRspInfo(pRspInfo);
	
}


bool CtpTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (ret)
	{
		cerr << " 响应 | " << pRspInfo->ErrorMsg << endl;
	}
	return ret;
}


void CtpTraderSpi::PrintOrders(){
	CThostFtdcOrderField* pOrder; 
	for(unsigned int i=0; i<orderList.size(); i++){
		pOrder = orderList[i];
		cerr<<" 报单 | 合约:"<<pOrder->InstrumentID
			<<" 方向:"<<MapDirection(pOrder->Direction,false)
			<<" 开平:"<<MapOffset(pOrder->CombOffsetFlag[0],false)
			<<" 价格:"<<pOrder->LimitPrice
			<<" 数量:"<<pOrder->VolumeTotalOriginal
			<<" 序号:"<<pOrder->BrokerOrderSeq 
			<<" 报单编号:"<<pOrder->OrderSysID
			<<" 状态:"<<pOrder->StatusMsg<<endl;
	}
	SetEvent(g_hEvent);
}


void CtpTraderSpi::PrintTrades(){
	CThostFtdcTradeField* pTrade;
	for(unsigned int i=0; i<tradeList.size(); i++){
		pTrade = tradeList[i];
		cerr<<" 成交 | 合约:"<< pTrade->InstrumentID 
			<<" 方向:"<<MapDirection(pTrade->Direction,false)
			<<" 开平:"<<MapOffset(pTrade->OffsetFlag,false) 
			<<" 价格:"<<pTrade->Price
			<<" 数量:"<<pTrade->Volume
			<<" 报单编号:"<<pTrade->OrderSysID
			<<" 成交编号:"<<pTrade->TradeID<<endl;
	}
	SetEvent(g_hEvent);
}


char MapDirection(char src, bool toOrig=true){
	if(toOrig){
		if('b'==src||'B'==src){src='0';}else if('s'==src||'S'==src){src='1';}
	}else{
		if('0'==src){src='B';}else if('1'==src){src='S';}
	}
	return src;
}


char MapOffset(char src, bool toOrig=true){
	if(toOrig){
		if('o'==src||'O'==src){src='0';}
		else if('c'==src||'C'==src){src='1';}
		else if('j'==src||'J'==src){src='3';}
	}else{
		if('0'==src){src='O';}
		else if('1'==src){src='C';}
		else if('3'==src){src='J';}
	}
	return src;
}


//请求查询报单
void CtpTraderSpi::ReqQryOrder()
{
	CThostFtdcQryOrderField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.InvestorID, m_userId);//投资者代码,也是userId

	int ret = m_pUserApi_td->ReqQryOrder(&req, ++requestId);


	cerr<<" 请求 | 发送查询报单..."<<((ret == 0)?"成功":"失败")<<" ret:"<<ret<<endl;//ret值为-3表示每秒发送请求数超过许可数

}


///请求查询报单响应,要分程序启动时第一次查询跟之后的查询
void CtpTraderSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr<<"请求查询报单响应:OnRspQryOrder"<<" pOrder:"<<pOrder<<endl;


	if (!IsErrorRspInfo(pRspInfo) && pOrder )
	{
		cerr<<"请求查询报单响应:前置编号FrontID:"<<pOrder->FrontID<<" 会话编号SessionID:"<<pOrder->SessionID<<" OrderRef:"<<pOrder->OrderRef<<endl;

		//所有合约
		if(first_inquiry_order == true)
		{
			CThostFtdcOrderField* order = new CThostFtdcOrderField();
			memcpy(order,  pOrder, sizeof(CThostFtdcOrderField));
			orderList.push_back(order);


			if(bIsLast) 
			{
				first_inquiry_order = false;

				cerr<<"所有合约报单次数："<<orderList.size()<<endl;

				cerr<<"--------------------------------------------------------------------报单start"<<endl;
				for(vector<CThostFtdcOrderField*>::iterator iter = orderList.begin(); iter != orderList.end(); iter++)
					cerr<<"经纪公司代码:"<<(*iter)->BrokerID<<endl<<" 投资者代码:"<<(*iter)->InvestorID<<endl<<" 用户代码:"<<(*iter)->UserID<<endl<<" 合约代码:"<<(*iter)->InstrumentID<<endl<<" 买卖方向:"<<(*iter)->Direction<<endl
					<<" 组合开平标志:"<<(*iter)->CombOffsetFlag<<endl<<" 价格:"<<(*iter)->LimitPrice<<endl<<" 数量:"<<(*iter)->VolumeTotalOriginal<<endl<<" 报单引用:"<< (*iter)->OrderRef<<endl<<" 客户代码:"<<(*iter)->ClientID<<endl
					<<" 报单状态:"<<(*iter)->OrderStatus<<endl<<" 委托时间:"<<(*iter)->InsertTime<<endl<<" 报单编号:"<<(*iter)->OrderSysID<<endl<<" GTD日期:"<<(*iter)->GTDDate<<endl<<" 交易日:"<<(*iter)->TradingDay<<endl
					<<" 报单日期:"<<(*iter)->InsertDate<<endl
					<<endl;

				cerr<<"--------------------------------------------------------------------报单end"<<endl;


				Sleep(1000);
				cerr<<"查询报单正常，首次查询成交:"<<endl;
				ReqQryTrade();

				SetEvent(g_hEvent);

			}
		}


	}
	else
	{
		if(first_inquiry_order == true ) 
		{
			first_inquiry_order = false;
			Sleep(1000);
			cerr<<"查询报单出错，或没有报单，首次查询成交:"<<endl;
			ReqQryTrade();
		}

	}

	cerr<<"-----------------------------------------------请求查询报单响应单次结束"<<endl;

}



void CtpTraderSpi::setAccount(TThostFtdcBrokerIDType	appId,	TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd)
{
	strcpy(m_appId, appId);
	strcpy(m_userId, userId);
	strcpy(m_passwd, passwd);

}




//撤单，如需追单，可在报单回报里面等撤单成功后再进行
void CtpTraderSpi::MaintainOrder(const string& MDtime, double MDprice, string maintainMode, int *tradedVolume)
{
	string InsertTime_str;
	double TargetPrice;
	double SetPrice;
	int MDtime_last2;

	if (orderList.size() > 0)
	{
		cerr<<"orderList.size():"<<orderList.size()<<endl;
	}

	if (maintainMode == "CheckOnTick")
	{
		for (vector<CThostFtdcOrderField*>::iterator iter = orderList.begin(); iter != orderList.end(); iter++)//倒序遍历，break
		{
			if ((*iter)->OrderStatus == '3' || (*iter)->OrderStatus == '1')//未成交还在队列中,部分成交还在队列中
			{
				if ((*iter)->FrontID == frontId && (*iter)->SessionID == sessionId)
				{
					string strOrderRef = (*iter)->OrderRef;
					SetPrice = (*iter)->LimitPrice;
					if (m_frontsessionref_frontsessionref.find(to_string(frontId) + to_string(sessionId) + strOrderRef) == m_frontsessionref_frontsessionref.end())
					{
						if (m_frontsessionref_ordertype.find(to_string(frontId) + to_string(sessionId) + strOrderRef) == m_frontsessionref_ordertype.end())
						{

						}
						else
						{
							CheckOrderPosition((*iter)->InstrumentID, (*iter)->Direction, pDepthMarketDataTD->AskPrice1, pDepthMarketDataTD->BidPrice1, &TargetPrice, m_frontsessionref_ordertype[to_string(frontId) + to_string(sessionId) + strOrderRef], pDepthMarketDataTD);
							InsertTime_str = (*iter)->InsertTime;//委托时间"21:47:01"

							MDtime_last2 = atoi(MDtime.substr(6, 2).c_str());

							if (MDtime_last2 < atoi(InsertTime_str.substr(6, 2).c_str()))//行情时间最后两位小于委托时间的最后两位
								MDtime_last2 += 60;//行情时间加60秒

							int OrderAdjustMode = m_frontsessionref_ordertype[to_string(frontId) + to_string(sessionId) + strOrderRef][2];
							if (MDtime_last2 - atoi(InsertTime_str.substr(6, 2).c_str()) >= OrderAdjustMode && SetPrice != TargetPrice)//委托大于该订单规定的秒数未成交
							{
								cerr << "撤单:" << endl;
								//撤单
								ReqOrderAction((*iter)->BrokerOrderSeq);

							}
						}
					}
				}
			}
		}
	}
	else if (maintainMode == "CancelOppositeOrder")
	{
		for (vector<CThostFtdcOrderField*>::iterator iter = orderList.begin(); iter != orderList.end(); iter++)//倒序遍历，break
		{
			if ((*iter)->OrderStatus == '3' || (*iter)->OrderStatus == '1')//未成交还在队列中,部分成交还在队列中
			{
				if ((*iter)->FrontID == frontId && (*iter)->SessionID == sessionId)
				{
					string strOrderRef = (*iter)->OrderRef;
					SetPrice = (*iter)->LimitPrice;
					if (m_frontsessionref_frontsessionref.find(to_string(frontId) + to_string(sessionId) + strOrderRef) == m_frontsessionref_frontsessionref.end())
					{
						if (m_frontsessionref_ordertype.find(to_string(frontId) + to_string(sessionId) + strOrderRef) == m_frontsessionref_ordertype.end())
						{

						}
						else
						{
							*tradedVolume = (*iter)->VolumeTotal;
							ReqOrderAction((*iter)->BrokerOrderSeq);
						}
					}
				}
			}
		}
	}
	




}


//请求查询成交
void CtpTraderSpi::ReqQryTrade()
{
	CThostFtdcQryTradeField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.InvestorID, m_userId);//投资者代码,也是userId

	int ret = m_pUserApi_td->ReqQryTrade(&req, ++requestId);

	cerr<<" 请求 | 发送成交查询..."<<((ret == 0)?"成功":"失败")<<" ret:"<<ret<<endl;//ret值为-3表示每秒发送请求数超过许可数

}




//请求查询成交响应,要分程序启动时第一次查询跟之后的查询
//只能查询当天的，昨仓不能，所以还要查询持仓明细OnRspQryInvestorPositionDetail()
void CtpTraderSpi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr<<"请求查询成交响应:OnRspQryTrade"<<" pTrade:"<<pTrade<<endl;

	if(!IsErrorRspInfo(pRspInfo) &&  pTrade)
	{
		//cerr<<"请求查询成交响应：OnRspQryTrade"<<" pTrade:"<<pTrade<<endl;

		//所有合约
		if(first_inquiry_trade == true)
		{
			CThostFtdcTradeField* trade = new CThostFtdcTradeField();
			memcpy(trade,  pTrade, sizeof(CThostFtdcTradeField));
			tradeList.push_back(trade);


			if(bIsLast)
			{
				first_inquiry_trade = false;

				cerr<<"成交次数:"<<tradeList.size()<<endl;

				cerr<<"--------------------------------------------------------------------成交start"<<endl;
				for(vector<CThostFtdcTradeField*>::iterator iter = tradeList.begin(); iter != tradeList.end(); iter++)
				{	
					cerr<<"合约代码:"<<(*iter)->InstrumentID<<endl<<" 用户代码:"<<(*iter)->UserID<<endl<<" 成交编号:"<<(*iter)->TradeID<<endl<<" 买卖方向:"<<(*iter)->Direction<<endl<<" 开平标志:"<<(*iter)->OffsetFlag<<endl
						<<" 投机套保标志:"<<(*iter)->HedgeFlag<<endl<<" 价格:"<<(*iter)->Price<<endl<<" 数量:"<<(*iter)->Volume<<endl<<" 成交时间:"<<(*iter)->TradeTime<<endl
						<<" 成交类型:"<<(*iter)->TradeType<<endl<<" 报单编号:"<<(*iter)->OrderSysID<<endl<<" 报单引用:"<<(*iter)->OrderRef<<endl<<" 本地报单编号:"<<(*iter)->OrderLocalID<<endl
						<<" 业务单元:"<<(*iter)->BusinessUnit<<endl<<" 序号:"<<(*iter)->SequenceNo<<endl<<" 经纪公司报单编号:"<<(*iter)->BrokerOrderSeq<<endl
						<<" 成交时期:"<<(*iter)->TradeDate<<endl<<" 交易日:"<<(*iter)->TradingDay

						<<endl<<endl;	
				}
				cerr<<"--------------------------------------------------------------------成交end"<<endl;


				Sleep(1000);
				//请求查询投资者持仓明细
				cerr<<"查询成交正常，首次查询投资者持仓明细:"<<endl;
				ReqQryInvestorPositionDetail();

				SetEvent(g_hEvent);
			}
		}



	}
	else
	{
		if(first_inquiry_trade == true)
		{
			first_inquiry_trade = false;
			Sleep(1000);
			//请求查询投资者持仓明细
			cerr<<"查询成交出错，或没有成交，首次查询投资者持仓明细:"<<endl;
			ReqQryInvestorPositionDetail();
		}

	}

	cerr<<"-----------------------------------------------请求查询成交响应单次结束"<<endl;

}



//请求查询投资者持仓明细
void CtpTraderSpi::ReqQryInvestorPositionDetail()
{
	CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));
	
	strcpy(req.InvestorID, m_userId);//投资者代码,也是userId
	
	//strcpy(req.InstrumentID, "IF1402");

	int ret = m_pUserApi_td->ReqQryInvestorPositionDetail(&req, ++requestId);

	cerr<<" 请求 | 发送投资者持仓明细查询..."<<((ret == 0)?"成功":"失败")<<" ret:"<<ret<<endl;//ret值为-3表示每秒发送请求数超过许可数

}



///请求查询投资者持仓明细响应,要分程序启动时第一次查询跟之后的查询
void CtpTraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr<<"请求查询投资者持仓明细响应：OnRspQryInvestorPositionDetail"<<" pInvestorPositionDetail:"<<pInvestorPositionDetail<<endl;

	if(!IsErrorRspInfo(pRspInfo) && pInvestorPositionDetail )
	{
		//按时间先后排序返回，按合约返回

		//所有合约
		if(firs_inquiry_Detail == true)
		{	
			//对于所有合约，不保存已平仓的，只保存未平仓的
			//将程序启动前的持仓记录保存到未平仓容器tradeList_notClosed_account_long和tradeList_notClosed_account_short
			//为什么不在查询成交回调函数OnRspQryTrade()来处理,因为只能查询当天的
			//使用结构体用CThostFtdcTradeField，因为含有时间字段，而CThostFtdcInvestorPositionDetailField没有时间字段

			CThostFtdcTradeField* trade = new CThostFtdcTradeField();

			strcpy(trade->InvestorID, pInvestorPositionDetail->InvestorID);///投资者代码
			strcpy(trade->InstrumentID, pInvestorPositionDetail->InstrumentID);///合约代码
			strcpy(trade->ExchangeID, pInvestorPositionDetail->ExchangeID);///交易所代码
			trade->Direction = pInvestorPositionDetail->Direction;///买卖方向
			trade->Price = pInvestorPositionDetail->OpenPrice;///价格
			trade->Volume = pInvestorPositionDetail->Volume;///数量
			strcpy(trade->TradeDate, pInvestorPositionDetail->OpenDate);///成交时期


			if(pInvestorPositionDetail->Volume > 0)//筛选未平仓的
			{
				if(trade->Direction == '0')
					tradeList_notClosed_account_long.push_back(trade);
				else if(trade->Direction == '1')
					tradeList_notClosed_account_short.push_back(trade);


				//收集持仓合约的合约代码

				bool find_instId = false;

				for(unsigned int i = 0; i< subscribe_inst_vec.size(); i++)
				{
					if(strcmp(subscribe_inst_vec[i].c_str(), trade->InstrumentID) == 0)//合约已存在，已订阅过行情
					{
						find_instId = true;
						break;
					}
				}

				if(!find_instId)
				{
					cerr<<"--------------------------------------该持仓合约没有订阅过行情，加进订阅列表："<<endl;
					subscribe_inst_vec.push_back(trade->InstrumentID);
				}




			}


			//输出所有合约的持仓明细，要在这边进行下一步的查询ReqQryTradingAccount();
			if(bIsLast)
			{
				firs_inquiry_Detail = false;

				
				string inst_holding;//保存持仓的合约列表

				for(unsigned int i = 0; i< subscribe_inst_vec.size(); i++)
					inst_holding = inst_holding + subscribe_inst_vec[i] + ",";

				inst_holding = inst_holding.substr(0,inst_holding.length()-1);//去掉最后面的逗号 

				cerr<<"程序启动前的持仓列表:"<<inst_holding<<","<<inst_holding.length()<<","<<inst_holding.size()<<endl;

				if(inst_holding.length() > 0)
					m_MDSpi->setInstIdList_holding_md(inst_holding);//设置程序启动前的留仓，即需要订阅行情的合约




				cerr<<"账户所有合约未平仓单笔数(不是手数,一笔可对应多手):多单"<<tradeList_notClosed_account_long.size()<<" 空单"<<tradeList_notClosed_account_short.size()<<endl;

				cerr<<"----------------------------------------------程序启动前未平仓单(先多后空)start"<<endl;

				for(vector<CThostFtdcTradeField*>::iterator iter = tradeList_notClosed_account_long.begin(); iter != tradeList_notClosed_account_long.end(); iter++)
				{
					cerr<<"投资者代码:"<<(*iter)->InvestorID<<endl
						<<" 合约代码:"<<(*iter)->InstrumentID<<endl
						<<" 交易所代码:"<<(*iter)->ExchangeID<<endl
						<<" 买卖方向:"<<(*iter)->Direction<<endl
						<<" 价格:"<<(*iter)->Price<<endl
						<<" 数量:"<<(*iter)->Volume<<endl
						<<" 成交时期:"<<(*iter)->TradeDate<<endl
						<<" 成交时间(持仓明细结构体无):"<<(*iter)->TradeTime<<endl<<endl;
				}

				for(vector<CThostFtdcTradeField*>::iterator iter = tradeList_notClosed_account_short.begin(); iter != tradeList_notClosed_account_short.end(); iter++)
				{
					cerr<<"投资者代码:"<<(*iter)->InvestorID<<endl
						<<" 合约代码:"<<(*iter)->InstrumentID<<endl
						<<" 交易所代码:"<<(*iter)->ExchangeID<<endl
						<<" 买卖方向:"<<(*iter)->Direction<<endl
						<<" 价格:"<<(*iter)->Price<<endl
						<<" 数量:"<<(*iter)->Volume<<endl
						<<" 成交时期:"<<(*iter)->TradeDate<<endl
						<<" 成交时间(持仓明细结构体无):"<<(*iter)->TradeTime<<endl<<endl;
				}

				cerr<<"------------------------------------------------程序启动前未平仓单(先多后空)end"<<endl;


				Sleep(1000);
				cerr<<"查询投资者持仓明细正常，请求查询资金账户:"<<endl;
				ReqQryTradingAccount();

			}
		}



	}
	else
	{
		if(firs_inquiry_Detail == true)
		{
			firs_inquiry_Detail = false;
			Sleep(1000);

			cerr<<"查询投资者持仓明细出错，或没有投资者持仓明细，查询资金账户:"<<endl;
			ReqQryTradingAccount();
		}

	}

	cerr<<"-----------------------------------------------请求查询投资者持仓明细响应结束"<<endl;

}





//设置交易的合约代码
void CtpTraderSpi::setInstId(string instId)
{
	strcpy(m_instId, instId.c_str());
}




//强制平仓，整个账户所有合约
void  CtpTraderSpi::ForceClose()
{
	TThostFtdcInstrumentIDType    instId;//合约
	TThostFtdcDirectionType       dir;//方向,'0'买，'1'卖
	TThostFtdcCombOffsetFlagType  kpp;//开平，"0"开，"1"平,"3"平今
	TThostFtdcPriceType           price;//价格，0是市价,上期所不支持
	TThostFtdcVolumeType          vol;//数量


	for(map<string, trade_message*>::iterator iter = m_trade_message_map.begin(); iter != m_trade_message_map.end(); iter++)
	{
		
		cerr<<"合约代码:"<< iter->first<<","<<iter->second->instId<<endl
		<<" 多单持仓量:"<<iter->second->holding_long<<endl
		<<" 空单持仓量:"<<iter->second->holding_short<<endl
		<<" 多单今日持仓:"<<iter->second->TodayPosition_long<<endl
		<<" 多单上日持仓:"<<iter->second->YdPosition_long<<endl
		<<" 空单今日持仓:"<<iter->second->TodayPosition_short<<endl
		<<" 空单上日持仓:"<<iter->second->YdPosition_short<<endl
		<<" 多单平仓盈亏:"<<iter->second->closeProfit_long<<endl
		<<" 多单浮动盈亏:"<<iter->second->OpenProfit_long<<endl
		<<" 空单平仓盈亏:"<<iter->second->closeProfit_short<<endl
		<<" 空单浮动盈亏:"<<iter->second->OpenProfit_short<<endl
		<<endl;



		//平多
		if(iter->second->holding_long > 0 )
		{
			strcpy_s(instId, iter->second->instId.c_str());//或strcpy(instId, iter->first.c_str());
			dir = '1';
			price = iter->second->lastPrice - 5 * m_instMessage_map[instId]->PriceTick;

			//上期所
			if(strcmp(m_instMessage_map[instId]->ExchangeID, "SHFE") == 0)
			{
				if(iter->second->YdPosition_long == 0)//没有昨仓
				{
					cerr<<"多单上期所全部平今："<<endl;

					strcpy(kpp, "3");//平今
					vol = iter->second->holding_long;
					ReqOrderInsert(instId, dir, kpp, price, vol);
					
				}
				else if(iter->second->TodayPosition_long == 0)//没有今仓
				{
					cerr<<"多单上期所全部平昨："<<endl;

					strcpy(kpp, "1");//平仓
					vol = iter->second->holding_long;
					ReqOrderInsert(instId, dir, kpp, price, vol);
					
				}
				//同时持有昨仓和今仓
				else if(iter->second->YdPosition_long > 0 && iter->second->TodayPosition_long > 0)
				{
					cerr<<"多单上期所同时平今平昨："<<endl;

					strcpy(kpp, "3");//平今
					vol = iter->second->TodayPosition_long;
					ReqOrderInsert(instId, dir, kpp, price, vol);

					strcpy(kpp, "1");//平仓
					vol = iter->second->YdPosition_long;
					ReqOrderInsert(instId, dir, kpp, price, vol);
					
				}

			}
			//非上期所
			else
			{
				cerr<<"非上期所多单平仓:"<<endl;

				strcpy(kpp, "1");
				vol = iter->second->holding_long;
				ReqOrderInsert(instId, dir, kpp, price, vol);
				
			}


		}


		//平空
		if(iter->second->holding_short > 0)
		{
			strcpy_s(instId, iter->second->instId.c_str());//或strcpy(instId, iter->first.c_str());
			dir = '0';
			price = iter->second->lastPrice + 5 * m_instMessage_map[instId]->PriceTick;

			//上期所
			if(strcmp(m_instMessage_map[instId]->ExchangeID, "SHFE") == 0)
			{
				if(iter->second->YdPosition_short == 0)//没有昨仓
				{
					cerr<<"空单上期所全部平今："<<endl;

					strcpy(kpp, "3");//平今
					vol = iter->second->holding_short;
					ReqOrderInsert(instId, dir, kpp, price, vol);
					
				}
				else if(iter->second->TodayPosition_short == 0)//没有今仓
				{
					cerr<<"空单上期所全部平昨："<<endl;

					strcpy(kpp, "1");//平仓
					vol = iter->second->holding_short;
					ReqOrderInsert(instId, dir, kpp, price, vol);
					
				}
				//同时持有昨仓和今仓
				else if(iter->second->YdPosition_short > 0 && iter->second->TodayPosition_short > 0)
				{
					cerr<<"空单上期所同时平今平昨："<<endl;

					strcpy(kpp, "3");//平今
					vol = iter->second->TodayPosition_short;
					ReqOrderInsert(instId, dir, kpp, price, vol);

					strcpy(kpp, "1");//平仓
					vol = iter->second->YdPosition_short;
					ReqOrderInsert(instId, dir, kpp, price, vol);
					
				}

			}
			//非上期所
			else
			{
				cerr<<"非上期所空单平仓:"<<endl;

				strcpy(kpp, "1");
				vol = iter->second->holding_short;
				ReqOrderInsert(instId, dir, kpp, price, vol);
				
			}



		}


		
	}

}






//计算平仓盈亏，平仓盈亏在成交回报有更新
double CtpTraderSpi::sendCloseProfit()
{
	double closeProfit_account = 0;

	for(map<string, trade_message*>::iterator iter = m_trade_message_map.begin(); iter != m_trade_message_map.end(); iter++)
	{		
		//平仓盈亏
		closeProfit_account = closeProfit_account + iter->second->closeProfit_long + iter->second->closeProfit_short;		
	}

	m_closeProfit = closeProfit_account;
	
	return closeProfit_account;

}




//计算账户的浮动盈亏，以开仓价算,可以限定持仓量大于0的，浮动盈亏计算完保存到m_trade_message_map
//如果要用到整个账户的浮动盈亏，是不是要等到所有合约都更新后才行
double CtpTraderSpi::sendOpenProfit_account(string instId, double lastPrice)
{
	double openProfit_account_long = 0, openProfit_account_short = 0, openProfit_account_all = 0;

	//多单的浮动盈亏
	for(vector<CThostFtdcTradeField*>::iterator iter = tradeList_notClosed_account_long.begin(); iter != tradeList_notClosed_account_long.end(); iter++)
	{
		if(strcmp(instId.c_str(), (*iter)->InstrumentID)==0)
			openProfit_account_long = openProfit_account_long + (lastPrice - (*iter)->Price) * (*iter)->Volume * m_instMessage_map[(*iter)->InstrumentID]->VolumeMultiple;

	}

	if(m_trade_message_map.count(instId) > 0)//该合约有持仓
		m_trade_message_map[instId]->OpenProfit_long = openProfit_account_long;//txt中的合约没有持仓，则行情回调不能调用


	//空单的浮动盈亏
	for(vector<CThostFtdcTradeField*>::iterator iter = tradeList_notClosed_account_short.begin(); iter != tradeList_notClosed_account_short.end(); iter++)
	{
		if(strcmp(instId.c_str(), (*iter)->InstrumentID)==0)
			openProfit_account_short = openProfit_account_short + ((*iter)->Price - lastPrice) * (*iter)->Volume * m_instMessage_map[(*iter)->InstrumentID]->VolumeMultiple;
	}

	if(m_trade_message_map.count(instId) > 0)//该合约有持仓
		m_trade_message_map[instId]->OpenProfit_short = openProfit_account_short;

	for(map<string, trade_message*>::iterator iter = m_trade_message_map.begin(); iter != m_trade_message_map.end(); iter++)
	{
		//浮动盈亏
		openProfit_account_all = openProfit_account_all + iter->second->OpenProfit_long + iter->second->OpenProfit_short;

	}

	m_OpenProfit = openProfit_account_all;

	return openProfit_account_all;


}



//打印所有合约信息
void CtpTraderSpi::showInstMessage()
{
	for(map<string, CThostFtdcInstrumentField*>::iterator iter = m_instMessage_map.begin(); iter != m_instMessage_map.end(); iter++)
	{
		cerr<<iter->first<<","<<iter->second->InstrumentID<<","<<iter->second->PriceTick<<","<<iter->second->VolumeMultiple<<endl;

	}

	cerr<<"m_instMessage_map.size():"<<m_instMessage_map.size()<<endl;


}




//打印持仓信息m_trade_message_map
void CtpTraderSpi::printTrade_message_map()
{
	cerr<<"------------------------------------------------printTrade_message_map开始"<<endl;
	for(map<string, trade_message*>::iterator iter = m_trade_message_map.begin(); iter != m_trade_message_map.end(); iter++)
	{
		if(iter->second->holding_long > 0 || iter->second->holding_short > 0)
		{
			cerr<<"合约代码:"<< iter->first<<","<<iter->second->instId<<endl
				<<" 多单持仓量:"<<iter->second->holding_long<<endl
				<<" 空单持仓量:"<<iter->second->holding_short<<endl
				<<" 多单今日持仓:"<<iter->second->TodayPosition_long<<endl
				<<" 多单上日持仓:"<<iter->second->YdPosition_long<<endl
				<<" 空单今日持仓:"<<iter->second->TodayPosition_short<<endl
				<<" 空单上日持仓:"<<iter->second->YdPosition_short<<endl
				<<" 多单平仓盈亏:"<<iter->second->closeProfit_long<<endl
				<<" 多单浮动盈亏:"<<iter->second->OpenProfit_long<<endl
				<<" 空单平仓盈亏:"<<iter->second->closeProfit_short<<endl
				<<" 空单浮动盈亏:"<<iter->second->OpenProfit_short<<endl
				<<endl;
		}
	}
	cerr<<"------------------------------------------------printTrade_message_map结束"<<endl;

}



//更新合约的最新价
void CtpTraderSpi::setLastPrice(string instID, double price)
{
	m_trade_message_map[instID]->lastPrice = price;

}


//合约交易信息结构体的map，KEY的个数，为0表示没有
int CtpTraderSpi::send_trade_message_map_KeyNum(string instID)
{
	return m_trade_message_map.count(instID);

}



int CtpTraderSpi::SendHolding_long(string instID)
{
	//cerr<<"m_trade_message_map.count(instID):"<<m_trade_message_map.count(instID)<<endl;

	if(m_trade_message_map.count(instID) == 0)
		return 0;
	else
		return m_trade_message_map[instID]->holding_long;

}


int CtpTraderSpi::SendHolding_short(string instID)
{
	if(m_trade_message_map.count(instID) == 0)
		return 0;
	else
		return m_trade_message_map[instID]->holding_short;

}

int CtpTraderSpi::SendTodayHolding_long(string instID)
{
	//cerr<<"m_trade_message_map.count(instID):"<<m_trade_message_map.count(instID)<<endl;

	if (m_trade_message_map.count(instID) == 0)
		return 0;
	else
		return m_trade_message_map[instID]->TodayPosition_long;

}

int CtpTraderSpi::SendTodayHolding_short(string instID)
{
	if (m_trade_message_map.count(instID) == 0)
		return 0;
	else
		return m_trade_message_map[instID]->TodayPosition_short;

}

int CtpTraderSpi::SendYdHolding_long(string instID)
{
	//cerr<<"m_trade_message_map.count(instID):"<<m_trade_message_map.count(instID)<<endl;

	if (m_trade_message_map.count(instID) == 0)
		return 0;
	else
		return m_trade_message_map[instID]->YdPosition_long;

}

int CtpTraderSpi::SendYdHolding_short(string instID)
{
	if (m_trade_message_map.count(instID) == 0)
		return 0;
	else
		return m_trade_message_map[instID]->YdPosition_short;

}

void  CtpTraderSpi::define_TThostFtdcCombOffsetFlagType()
{
	m_symbol_order_type_map.insert(pair<string, string>("IF", "CLOSE_IF_TODAYPOS_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("IH", "CLOSE_IF_TODAYPOS_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("IC", "CLOSE_IF_TODAYPOS_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("TF", "CLOSE_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("T1", "CLOSE_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("au", "CLOSE_TODAY_YD_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("ag", "CLOSE_TODAY_YD_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("cu", "CLOSE_TODAY_YD_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("al", "CLOSE_TODAY_YD_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("zn", "CLOSE_TODAY_YD_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("pb", "CLOSE_TODAY_YD_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("ni", "CLOSE_TODAY_YD_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("sn", "CLOSE_TODAY_YD_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("jm", "CLOSE_IF_TODAYPOS_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("j1", "CLOSE_IF_TODAYPOS_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("ZC", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("rb", "CLOSE_TODAY_YD_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("i1", "CLOSE_IF_TODAYPOS_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("hc", "CLOSE_TODAY_YD_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("SF", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("SM", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("FG", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("fu", "CLOSE_TODAY_YD_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("sc", "CLOSE_TODAY_YD_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("ru", "CLOSE_TODAY_YD_OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("l1", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("TA", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("v1", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("MA", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("pp", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("bu", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("a1", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("b1", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("WH", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("c1", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("RI", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("JR", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("LR", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("RS", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("m1", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("RM", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("y1", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("OI", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("p1", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("CF", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("SR", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("CY", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("jd", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("cs", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("AP", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("fb", "CLOSE__OPEN"));
	m_symbol_order_type_map.insert(pair<string, string>("bb", "CLOSE__OPEN"));
}


string CtpTraderSpi::Send_TThostFtdcCombOffsetFlagType(string instID)
{
	return m_symbol_order_type_map[instID];
}

void CtpTraderSpi::Set_CThostFtdcDepthMarketDataField(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	pDepthMarketDataTD = pDepthMarketData;
	/*m_pDepthMarketData_type.insert(std::pair<string, CThostFtdcDepthMarketDataField*>(to_string(frontId) + to_string(sessionId), pDepthMarketData));*/
}

void CtpTraderSpi::SendOrderDecoration(TThostFtdcInstrumentIDType instId, string order_type, TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType * kpp, TThostFtdcPriceType askprice, TThostFtdcPriceType bidprice, TThostFtdcPriceType price, TThostFtdcVolumeType vol, vector<int> orderType, CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	string strOrderRef = orderRef;
	m_frontsessionref_ordertype.insert (std::pair<string, vector<int>>(to_string(frontId) + to_string(sessionId) + strOrderRef, orderType));
	m_frontsessionref_order_type.insert(std::pair<string, string>(to_string(frontId) + to_string(sessionId) + strOrderRef, order_type));
	CheckOrderPosition(instId, dir, askprice, bidprice, &price, orderType, pDepthMarketData);

	#pragma region CLOSE_TODAY_YD_OPEN
		if (order_type == "CLOSE_TODAY_YD_OPEN")
		{
			if (dir == '0')
			{
				if (vol <= this->SendTodayHolding_short(instId))
				{
					this->ReqOrderInsert(instId, dir, "3", price, vol);
				}
				else if (vol <= this->SendYdHolding_short(instId))
				{
					this->ReqOrderInsert(instId, dir, "1", price, vol);
				}
				else if (vol > this->SendYdHolding_short(instId) && vol > this->SendTodayHolding_short(instId) && vol <= (this->SendTodayHolding_short(instId) + this->SendYdHolding_short(instId)))
				{
					this->ReqOrderInsert(instId, dir, "3", price, this->SendTodayHolding_short(instId));
					
					strOrderRef = orderRef;
					m_frontsessionref_ordertype.insert(std::pair<string, vector<int>>(to_string(frontId) + to_string(sessionId) + strOrderRef, orderType));
					m_frontsessionref_order_type.insert(std::pair<string, string>(to_string(frontId) + to_string(sessionId) + strOrderRef, order_type));
					CheckOrderPosition(instId, dir, askprice, bidprice, &price, orderType, pDepthMarketData);
					this->ReqOrderInsert(instId, dir, "1", price, this->SendYdHolding_short(instId));
				}
				else if (vol > this->SendYdHolding_short(instId) && vol > this->SendTodayHolding_short(instId) && vol > (this->SendTodayHolding_short(instId) + this->SendYdHolding_short(instId)))
				{
					this->ReqOrderInsert(instId, dir, "3", price, this->SendTodayHolding_short(instId));

					strOrderRef = orderRef;
					m_frontsessionref_ordertype.insert(std::pair<string, vector<int>>(to_string(frontId) + to_string(sessionId) + strOrderRef, orderType));
					m_frontsessionref_order_type.insert(std::pair<string, string>(to_string(frontId) + to_string(sessionId) + strOrderRef, order_type));
					CheckOrderPosition(instId, dir, askprice, bidprice, &price, orderType, pDepthMarketData);
					this->ReqOrderInsert(instId, dir, "1", price, this->SendYdHolding_short(instId));
					
					strOrderRef = orderRef;
					m_frontsessionref_ordertype.insert(std::pair<string, vector<int>>(to_string(frontId) + to_string(sessionId) + strOrderRef, orderType));
					m_frontsessionref_order_type.insert(std::pair<string, string>(to_string(frontId) + to_string(sessionId) + strOrderRef, order_type));
					CheckOrderPosition(instId, dir, askprice, bidprice, &price, orderType, pDepthMarketData);
					this->ReqOrderInsert(instId, dir, "0", price, vol - this->SendYdHolding_short(instId) - this->SendTodayHolding_short(instId));
				}
			}
			else if (dir == '1')
			{
				if (vol <= this->SendTodayHolding_long(instId))
				{
					this->ReqOrderInsert(instId, dir, "3", price, vol);
				}
				else if (vol <= this->SendYdHolding_long(instId))
				{
					this->ReqOrderInsert(instId, dir, "1", price, vol);
				}
				else if (vol > this->SendYdHolding_long(instId) && vol > this->SendTodayHolding_long(instId) && vol <= (this->SendTodayHolding_long(instId) + this->SendYdHolding_long(instId)))
				{
					this->ReqOrderInsert(instId, dir, "3", price, this->SendYdHolding_long(instId));

					strOrderRef = orderRef;
					m_frontsessionref_ordertype.insert(std::pair<string, vector<int>>(to_string(frontId) + to_string(sessionId) + strOrderRef, orderType));
					m_frontsessionref_order_type.insert(std::pair<string, string>(to_string(frontId) + to_string(sessionId) + strOrderRef, order_type));
					CheckOrderPosition(instId, dir, askprice, bidprice, &price, orderType, pDepthMarketData);
					this->ReqOrderInsert(instId, dir, "1", price, this->SendYdHolding_short(instId));
				}
				else if (vol > this->SendYdHolding_long(instId) && vol > this->SendTodayHolding_long(instId) && vol > (this->SendTodayHolding_long(instId) + this->SendYdHolding_long(instId)))
				{
					this->ReqOrderInsert(instId, dir, "3", price, this->SendTodayHolding_short(instId));

					strOrderRef = orderRef;
					m_frontsessionref_ordertype.insert(std::pair<string, vector<int>>(to_string(frontId) + to_string(sessionId) + strOrderRef, orderType));
					m_frontsessionref_order_type.insert(std::pair<string, string>(to_string(frontId) + to_string(sessionId) + strOrderRef, order_type));
					CheckOrderPosition(instId, dir, askprice, bidprice, &price, orderType, pDepthMarketData);
					this->ReqOrderInsert(instId, dir, "1", price, this->SendYdHolding_short(instId));

					strOrderRef = orderRef;
					m_frontsessionref_ordertype.insert(std::pair<string, vector<int>>(to_string(frontId) + to_string(sessionId) + strOrderRef, orderType));
					m_frontsessionref_order_type.insert(std::pair<string, string>(to_string(frontId) + to_string(sessionId) + strOrderRef, order_type));
					CheckOrderPosition(instId, dir, askprice, bidprice, &price, orderType, pDepthMarketData);
					this->ReqOrderInsert(instId, dir, "0", price, vol - this->SendYdHolding_short(instId) - this->SendTodayHolding_short(instId));

				}
			}
		}
	#pragma endregion CLOSE_TODAY_YD_OPEN

	#pragma region CLOSE_TODAY_YD_OPEN
		if (order_type == "CLOSE_IF_TODAYPOS_OPEN")
		{
			if (dir == '0')
			{
				if (this->SendTodayHolding_short(instId) > 0 && IndexFuturePositionLimit - this->SendTodayHolding_short(instId) - this->SendTodayHolding_long(instId) >= vol)
				{
					this->ReqOrderInsert(instId, dir, "0", price, vol);
				}
				else if (this->SendTodayHolding_short(instId) == 0 && IndexFuturePositionLimit - this->SendTodayHolding_short(instId) - this->SendTodayHolding_long(instId) >= vol)
				{
					this->ReqOrderInsert(instId, dir, "1", price, vol);
				}
				else if (this->SendTodayHolding_short(instId) == 0 && this->SendYdHolding_short(instId) >= vol)
				{
					this->ReqOrderInsert(instId, dir, "1", price, vol);
				}
			}
			else if (dir == '1')
			{
				if (this->SendTodayHolding_long(instId) > 0 && IndexFuturePositionLimit - this->SendTodayHolding_short(instId) - this->SendTodayHolding_long(instId) >= vol)
				{
					this->ReqOrderInsert(instId, dir, "0", price, vol);
				}
				else if (this->SendTodayHolding_long(instId) == 0 && IndexFuturePositionLimit - this->SendTodayHolding_short(instId) - this->SendTodayHolding_long(instId) >= vol)
				{
					this->ReqOrderInsert(instId, dir, "1", price, vol);
				}
				else if (this->SendTodayHolding_long(instId) == 0 && this->SendYdHolding_long(instId) >= vol)
				{
					this->ReqOrderInsert(instId, dir, "1", price, vol);
				}
			}
		}
	#pragma endregion CLOSE_TODAY_YD_OPEN

	#pragma region ALL_OPEN
			if (order_type == "ALL_OPEN")
			{
				if (dir == '0')
				{
					if (this->SendTodayHolding_short(instId) > 0 && IndexFuturePositionLimit - this->SendTodayHolding_short(instId) - this->SendTodayHolding_long(instId) >= vol)
					{
						this->ReqOrderInsert(instId, dir, "0", price, vol);
					}
					else if (this->SendTodayHolding_short(instId) == 0 && IndexFuturePositionLimit - this->SendTodayHolding_short(instId) - this->SendTodayHolding_long(instId) >= vol)
					{
						this->ReqOrderInsert(instId, dir, "0", price, vol);
					}
					else if (this->SendTodayHolding_short(instId) == 0 && this->SendYdHolding_short(instId) >= vol)
					{
						this->ReqOrderInsert(instId, dir, "0", price, vol);
					}
				}
				else if (dir == '1')
				{
					if (this->SendTodayHolding_long(instId) > 0 && IndexFuturePositionLimit - this->SendTodayHolding_short(instId) - this->SendTodayHolding_long(instId) >= vol)
					{
						this->ReqOrderInsert(instId, dir, "0", price, vol);
					}
					else if (this->SendTodayHolding_long(instId) == 0 && IndexFuturePositionLimit - this->SendTodayHolding_short(instId) - this->SendTodayHolding_long(instId) >= vol)
					{
						this->ReqOrderInsert(instId, dir, "0", price, vol);
					}
					else if (this->SendTodayHolding_long(instId) == 0 && this->SendYdHolding_long(instId) >= vol)
					{
						this->ReqOrderInsert(instId, dir, "0", price, vol);
					}
				}
			}
	#pragma endregion ALL_OPEN

		
}
void CtpTraderSpi::CheckOrderPosition(TThostFtdcInstrumentIDType instId, TThostFtdcDirectionType dir, TThostFtdcPriceType askprice, TThostFtdcPriceType bidprice, TThostFtdcPriceType *price, vector<int> orderType, CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	int OrderHitPutMode = orderType[0];
	int OrderPriceMover = orderType[1];
	int OrderAdjustMode = orderType[2];
	double OrderPriceTick = m_instMessage_map[instId]->VolumeMultiple;
	double SettlePrice = 0.0;


	if (dir == '0')
	{
		if (OrderHitPutMode == 1)
		{
			SettlePrice = askprice + OrderPriceMover * m_instMessage_map[instId]->PriceTick;
		}
		else if (OrderHitPutMode == -1)
		{
			SettlePrice = bidprice + OrderPriceMover * m_instMessage_map[instId]->PriceTick;
		}
	}
	else if (dir == '1')
	{
		if (OrderHitPutMode == 1)
		{
			SettlePrice = bidprice - OrderPriceMover * m_instMessage_map[instId]->PriceTick;
		}
		else if (OrderHitPutMode == -1)
		{
			SettlePrice = askprice - OrderPriceMover * m_instMessage_map[instId]->PriceTick;
		}
	}
	*price = SettlePrice;
}

void CtpTraderSpi::Testkkk(string order_type, vector<int> orderType)
{
	string strOrderRef = orderRef;
	m_frontsessionref_ordertype.insert(std::pair<string, vector<int>>(to_string(frontId) + to_string(sessionId) + strOrderRef, orderType));
	m_frontsessionref_order_type.insert(std::pair<string, string>(to_string(frontId) + to_string(sessionId) + strOrderRef, order_type));
}
