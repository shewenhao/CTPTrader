#ifndef ORDER_TRADERSPI_H_
#define ORDER_TRADERSPI_H_


#include "ThostFtdcTraderApi.h"
#include <vector>
#include <map>
#include "StructFunction.h"
#include "ThostFtdcMdApi.h"


class CtpMdSpi;


class CtpTraderSpi : public CThostFtdcTraderSpi
{
public:
	CtpTraderSpi(CThostFtdcTraderApi* api, CThostFtdcMdApi* pUserApi_md, CtpMdSpi* MDSpi):m_pUserApi_td(api), m_pMDUserApi_td(pUserApi_md), m_MDSpi(MDSpi)
	{		
		m_closeProfit = 0.0;//平仓盈亏
		m_OpenProfit = 0.0;//浮动盈亏

		first_inquiry_order = true;//是否首次查询报单
		first_inquiry_trade = true;//是否首次查询成交
		firs_inquiry_Detail = true;//是否首次查询持仓明细
		firs_inquiry_TradingAccount = true;//是否首次查询资金账号
		firs_inquiry_Position = true;//是否首次查询投资者持仓
		first_inquiry_Instrument = true;//是否首次查询合约
			
	};

	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected();

	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///投资者结算结果确认响应
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询合约响应
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询资金账户响应
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询投资者持仓响应
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///报单录入请求响应
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///报单操作请求响应
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	virtual void OnFrontDisconnected(int nReason);

	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	virtual void OnHeartBeatWarning(int nTimeLapse);

	///报单通知
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

	///成交通知
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

	///请求查询报单响应
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询成交响应
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询投资者持仓明细响应
	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//市场行情通知
	void CtpTraderSpi::Set_CThostFtdcDepthMarketDataField(CThostFtdcDepthMarketDataField *pDepthMarketData);

	///判断订单类型
	virtual void SendOrderDecoration(TThostFtdcInstrumentIDType    instId, string order_type, TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType * kpp, TThostFtdcPriceType price, TThostFtdcVolumeType vol, vector<int> orderType, CThostFtdcDepthMarketDataField *pDepthMarketData);

	//判断下单类型
	virtual void CheckOrderPosition(TThostFtdcInstrumentIDType instId, TThostFtdcDirectionType dir, TThostFtdcPriceType *price, vector<int> orderType, CThostFtdcDepthMarketDataField *pDepthMarketData);
public:
	///用户登录请求
	void ReqUserLogin(TThostFtdcBrokerIDType	appId,
		TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd);
	
	///投资者结算结果确认
	void ReqSettlementInfoConfirm();
	
	///请求查询合约
	void ReqQryInstrument(TThostFtdcInstrumentIDType instId);
	
	///请求查询合约，所有合约
	void ReqQryInstrument_all();

	///请求查询资金账户
	void ReqQryTradingAccount();
	
	///请求查询投资者持仓
	void ReqQryInvestorPosition(TThostFtdcInstrumentIDType instId);

	///请求查询投资者持仓,所有合约
	void ReqQryInvestorPosition_all();
	
	///报单录入请求
	void ReqOrderInsert(TThostFtdcInstrumentIDType instId,
		TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
		TThostFtdcPriceType price,   TThostFtdcVolumeType vol);
	
	///报单操作请求
	void ReqOrderAction(TThostFtdcSequenceNoType orderSeq);

	////错误响应
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

	//打印报单记录
	void PrintOrders();
	
	//打印成交记录
	void PrintTrades();

	//请求查询报单
	void ReqQryOrder();

	//请求查询成交
	void ReqQryTrade();

	//请求查询投资者持仓明细
	void ReqQryInvestorPositionDetail();

	//账号设置
	void setAccount(TThostFtdcBrokerIDType	appId,	TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd);

	//撤单，如需追单，可在报单回报里面等撤单成功后再进行
	void MaintainOrder(const string& MDtime, double MDprice);

	//设置交易的合约代码
	void setInstId(string instId);

	//强制账户平仓
	void ForceClose();

	//计算平仓盈亏
	double sendCloseProfit();

	//计算账户的浮动盈亏，以开仓价算	
	double sendOpenProfit_account(string instId, double lastPrice);

	//计算持仓盈亏，以昨结计算，若是日内，与浮动盈亏相同（实际交易中应用比较少）
	double sendPositionProfit();

	//打印所有合约信息
	void showInstMessage();

	//打印持仓信息m_trade_message_map
	void printTrade_message_map();

	//更新合约的最新价
	void setLastPrice(string instID, double price);

	//合约交易信息结构体的map，KEY的个数，为0表示没有该合约的交易信息，即该合约没有持仓(开仓后已经平仓的，KEY不为0）
	int send_trade_message_map_KeyNum(string instID);

	//返回某合约多单持仓量
	int SendHolding_long(string instID);

	//返回某合约空单持仓量
	int SendHolding_short(string instID);

	//返回某合约多单持仓量
	int SendTodayHolding_long(string instID);

	//返回某合约空单持仓量
	int SendTodayHolding_short(string instID);

	//返回某合约多单持仓量
	int SendYdHolding_long(string instID);

	//返回某合约空单持仓量
	int SendYdHolding_short(string instID);

	//定义品种开平规则
	void define_TThostFtdcCombOffsetFlagType();

	//返回品种开平规则
	string Send_TThostFtdcCombOffsetFlagType(string instID);


	
private:
	
	CThostFtdcTraderApi* m_pUserApi_td;//交易API指针，构造函数里赋值
	
	CThostFtdcMdApi* m_pMDUserApi_td;//行情API指针，构造函数里赋值
	
	CtpMdSpi* m_MDSpi;//MD指针，构造函数里赋值

	TThostFtdcBrokerIDType	m_appId;// 应用单元
	TThostFtdcUserIDType	m_userId;// 投资者代码
	TThostFtdcPasswordType	m_passwd;//密码


	double m_closeProfit;//平仓盈亏，所有合约一起算后的值，另外在m_trade_message_map有单独计算每个合约的平仓盈亏值
	
	double m_OpenProfit;//浮动盈亏，所有合约一起算后的值，另外在m_trade_message_map有单独计算每个合约的浮动盈亏值
	
	map<string, trade_message*> m_trade_message_map;//合约交易信息结构体的map

	map<string, CThostFtdcInstrumentField*> m_instMessage_map;//保存合约信息的map

	map<string, string> m_symbol_order_type_map;//保存不同合约因交易所规定限制带来的有限平仓还是对开的map
	
	map<string, vector<int>> m_frontsessionref_ordertype; //保存本连接带来的订单类型，这样维护订单的时候如果订单需要撤单和重新挂单有所依据
	
	map<string, string> m_frontsessionref_order_type; //保存本连接所有带来的订单开平类型，这样维护订单的时候如果订单需要撤单和重新挂单有所开平依据

	map<string, CThostFtdcOrderField*> m_frontsessionref_frontsessionref;//保存被撤单的单子编号，防止反复根据撤单回报进行相关操作

	map<string, CThostFtdcOrderField*> m_CThostFtdcInputOrder_type;

	map<string, CThostFtdcDepthMarketDataField*> m_pDepthMarketData_type;//把和策略相关的行情存储进入到map中，防止其他策略的订阅行情干扰

	TThostFtdcInstrumentIDType m_instId;//合约代码
	
	string m_Instrument_all;//所有合约代码合在一起
	
	vector<string> subscribe_inst_vec;//需要订阅行情的合约，即程序启动前有持仓过的合约

	bool first_inquiry_order;//是否首次查询报单
	bool first_inquiry_trade;//是否首次查询成交
	bool firs_inquiry_Detail;//是否首次查询持仓明细
	bool firs_inquiry_TradingAccount;//是否首次查询资金账号
	bool firs_inquiry_Position;//是否首次查询投资者持仓
	bool first_inquiry_Instrument;//是否首次查询合约
	
	vector<CThostFtdcOrderField*> orderList;//委托记录，全部合约
    vector<CThostFtdcTradeField*> tradeList;//成交记录，全部合约
	
	vector<CThostFtdcTradeField*> tradeList_notClosed_account_long;//未平仓的多单成交记录,整个账户，全部合约
	vector<CThostFtdcTradeField*> tradeList_notClosed_account_short;//未平仓的空单成交记录,整个账户，全部合约

	string m_tradeDate;//当前交易日，持仓表中的开仓日期不相等，则是昨仓，可以直接赋值
	
};

#endif