#include <afx.h>
#include <fstream>
#include <string>
#include <sstream>

#include "traderspi.h"
#include "mdspi.h"
#include "windows.h"


#pragma warning(disable : 4996)

extern int requestId;  
extern HANDLE g_hEvent;
extern Strategy* g_strategy;


void CtpMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
	int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
}

void CtpMdSpi::OnFrontDisconnected(int nReason)
{
	cerr<<" 响应 | 连接中断..." 
		<< " reason=" << nReason << endl;
}

void CtpMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr<<" 响应 | 心跳超时警告..." 
		<< " TimerLapse = " << nTimeLapse << endl;
}

void CtpMdSpi::OnFrontConnected()
{
	cerr<<"MD 连接交易前置OnFrontConnected()...成功"<<endl;
	
	//登录期货账号
	ReqUserLogin(m_appId, m_userId, m_passwd);

	SetEvent(g_hEvent);
}

void CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType	appId,
	TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(req.UserID, userId);
	strcpy(req.Password, passwd);
	int ret = m_pUserApi_md->ReqUserLogin(&req, ++requestId);
	cerr<<"MD 请求 | 发送登录..."<<((ret == 0) ? "成功" :"失败") << endl;	
	SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{

		cerr<<"行情模块登录成功"<<endl;	

		//cerr<<"---"<<"错误代码为0表示成功："<<pRspInfo->ErrorID<<",错误信息:"<<pRspInfo->ErrorMsg<<endl;

		//SubscribeMarketData_all();//订阅全市场行情

		strcpy(m_instId, m_instId_string.c_str());

		SubscribeMarketData(m_instId);//订阅交易合约的行情

		setInstIdList_holding_md(m_charNewIdList_holding_md_string);
		//订阅持仓合约的行情
		if(m_charNewIdList_holding_md)
		{	
			cerr<<"m_charNewIdList_holding_md大小:"<<strlen(m_charNewIdList_holding_md)<<","<<m_charNewIdList_holding_md<<endl;

			cerr<<"有持仓，订阅行情："<<endl;
			SubscribeMarketData(m_charNewIdList_holding_md);//流控为6笔/秒,如果没有持仓，就不要订阅

			//delete []m_charNewIdList_holding_md;//订阅完成，释放内存
		}
		else
			cerr<<"当前没有持仓"<<endl;


		//策略启动后默认禁止开仓是个好的风控习惯
		cerr<<endl<<endl<<endl<<"策略默认禁止开仓，如需允许交易，请输入指令(允许开仓:yxkc, 禁止开仓:jzkc)："<<endl;
		

	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpMdSpi::SubscribeMarketData(char* instIdList)
{
	vector<char*> list;
	char *token = strtok(instIdList, ",");
	while( token != NULL ){
		list.push_back(token); 
		token = strtok(NULL, ",");
	}
	unsigned int len = list.size();
	char** pInstId = new char* [len];  
	for(unsigned int i=0; i<len;i++)  pInstId[i]=list[i]; 
	int ret=m_pUserApi_md->SubscribeMarketData(pInstId, len);
	cerr<<" 请求 | 发送行情订阅... "<<((ret == 0) ? "成功" : "失败")<< endl;
	SetEvent(g_hEvent);
}



//订阅全市场行情
void CtpMdSpi::SubscribeMarketData_all()
{
	SubscribeMarketData(m_charNewIdList_all);
	delete []m_charNewIdList_all;
}




void CtpMdSpi::OnRspSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr<<" 响应 |  行情订阅...成功"<<endl;

	if(bIsLast)  SetEvent(g_hEvent);
}



void CtpMdSpi::OnRspUnSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr<<" 响应 |  行情取消订阅...成功"<<endl;
	if(bIsLast)  SetEvent(g_hEvent);
}



void CtpMdSpi::OnRtnDepthMarketData(
	CThostFtdcDepthMarketDataField *pDepthMarketData)
{
		g_strategy->OnTickData(pDepthMarketData);

}





bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{	
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (ret){
		cerr<<" 响应 | "<<pRspInfo->ErrorMsg<<endl;
	}
	return ret;
}






void CtpMdSpi::setAccount(TThostFtdcBrokerIDType	appId1,	TThostFtdcUserIDType	userId1,	TThostFtdcPasswordType	passwd1)
{
	strcpy(m_appId, appId1);
	strcpy(m_userId, userId1);
	strcpy(m_passwd, passwd1);
}





//设置交易的合约代码
void CtpMdSpi::setInstId(string instId)
{
	m_instId_string = instId;
	strcpy(m_instId, m_instId_string.c_str());
}





void CtpMdSpi::setInstIdList_holding_md(string instId)
{
	m_charNewIdList_holding_md_string = instId;
	//strcpy(m_instIdList_holding_md, instId.c_str());
	int sizeInstId = instId.size();

	m_charNewIdList_holding_md = new char[sizeInstId+1];

	memset(m_charNewIdList_holding_md,0,sizeof(char)*(sizeInstId+1));

	strcpy(m_charNewIdList_holding_md, instId.c_str());

	/*strcpy(m_instIdList_all, instIdList_all.c_str());*/

	cerr<<"有持仓的合约:"<<strlen(m_charNewIdList_holding_md)<<","<<sizeof(m_charNewIdList_holding_md)<<","<<_msize(m_charNewIdList_holding_md)<<endl<<m_charNewIdList_holding_md<<endl;

}



//保存全市场合约，在TD进行
void CtpMdSpi::set_instIdList_all(string instIdList_all)
{	
	int sizeIdList_all = instIdList_all.size();

	m_charNewIdList_all = new char[sizeIdList_all+1];

	memset(m_charNewIdList_all,0,sizeof(char)*(sizeIdList_all+1));

	strcpy(m_charNewIdList_all, instIdList_all.c_str());

	/*strcpy(m_instIdList_all, instIdList_all.c_str());*/

	if(!m_charNewIdList_all)//用strlen时m_charNewIdList_all不能为空
		cerr<<"收到的全市场合约:"<<strlen(m_charNewIdList_all)<<","<<sizeof(m_charNewIdList_all)<<","<<_msize(m_charNewIdList_all)<<endl<<m_charNewIdList_all<<endl;

}