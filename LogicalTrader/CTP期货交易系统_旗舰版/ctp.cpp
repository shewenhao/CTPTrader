//交易所代码：SHFE/DCE/CZCE/CFFEX(上期所/大连/郑州/中金所)



/**********************************************************************
项目名称: CTP期货交易系统(旗舰版)
最后修改：20150508
程序作者：期盈量化交易团队，官方QQ群：202367118


**********************************************************************/

#include <afx.h>
#include "ctp.h"
#include "mdspi.h"
#include "traderspi.h"
#include "strategy.h"
#include <iostream>
#include <string>
#include <cstdio>


int strategyVolumeTarget;

string strategykdbscript;

double par1, par2, par3, par4, par5, par6;

int strategyOrderType1, strategyOrderType2, strategyOrderType3;

string strategyPairLeg1Symbol, strategyPairLeg2Symbol, strategyPairLeg3Symbol;

int kdbPort;

int requestId=0;//请求编号

HANDLE g_hEvent;//多线程用到的句柄

Strategy* g_strategy;//策略类指针

CtpTraderSpi* g_pUserSpi_tradeAll;//全局的TD回调处理类对象，人机交互函数需用到

//人机交互函数
DWORD WINAPI ThreadProc(LPVOID lpParameter);





int main(int argc, const char* argv[])
{
	
	//string strategyAccountParampath = argv[1];
	string strategyAccountParampath = "9010_AccountParam_SHE_SIM_al_5_300_2.ini";
	g_hEvent=CreateEvent(NULL, true, false, NULL); 

	//--------------读取配置文件，获取账户信息、服务器地址、交易的合约代码--------------
	ReadMessage readMessage;
	memset(&readMessage, 0, sizeof(readMessage));
	SetMessage(readMessage, &kdbPort, &strategyVolumeTarget, &strategykdbscript, &par1, &par2, &par3, &par4, &par5, &par6, &strategyOrderType1, &strategyOrderType2, &strategyOrderType3, strategyAccountParampath, &strategyPairLeg1Symbol, &strategyPairLeg2Symbol, &strategyPairLeg3Symbol);	

	SetConsoleTitle(("CTP" + (strategyAccountParampath)).c_str());

	
	

	//--------------初始化行情UserApi，创建行情API实例----------------------------------
	CThostFtdcMdApi* pUserApi_md = CThostFtdcMdApi::CreateFtdcMdApi(mdflowPath.c_str());
	CtpMdSpi* pUserSpi_md = new CtpMdSpi(pUserApi_md);//创建回调处理类对象MdSpi
	pUserApi_md->RegisterSpi(pUserSpi_md);// 回调对象注入接口类
	pUserApi_md->RegisterFront(readMessage.m_mdFront);// 注册行情前置地址	
	pUserSpi_md->setAccount(readMessage.m_appId, readMessage.m_userId, readMessage.m_passwd);//经纪公司编号，用户名，密码
	pUserSpi_md->setInstId(readMessage.m_read_contract);//MD所需订阅行情的合约，即策略交易的合约



	//--------------初始化交易UserApi，创建交易API实例----------------------------------
	CThostFtdcTraderApi* pUserApi_trade = CThostFtdcTraderApi::CreateFtdcTraderApi(tdflowPath.c_str());
	CtpTraderSpi* pUserSpi_trade = new CtpTraderSpi(pUserApi_trade, pUserApi_md, pUserSpi_md);//构造函数初始化三个变量
	pUserApi_trade->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi_trade);// 注册事件类
	pUserApi_trade->SubscribePublicTopic(THOST_TERT_RESTART);// 注册公有流
	pUserApi_trade->SubscribePrivateTopic(THOST_TERT_QUICK);// 注册私有流THOST_TERT_QUICK
	pUserApi_trade->RegisterFront(readMessage.m_tradeFront);// 注册交易前置地址
	pUserSpi_trade->setAccount(readMessage.m_appId, readMessage.m_userId, readMessage.m_passwd);//经纪公司编号，用户名，密码	
	pUserSpi_trade->setInstId(readMessage.m_read_contract);//策略交易的合约

	g_pUserSpi_tradeAll = pUserSpi_trade;//全局的TD回调处理类对象，人机交互函数需用到




	//--------------创建策略实例--------------------------------------------------------
	g_strategy = new Strategy(pUserSpi_trade);
	g_strategy->Init(readMessage.m_strategy_strategytype, readMessage.m_read_contract, kdbPort, kdbScriptExePath, strategyVolumeTarget, strategykdbscript, par1, par2, par3, par4, par5, par6, strategyOrderType1, strategyOrderType2, strategyOrderType3, strategyPairLeg1Symbol,strategyPairLeg2Symbol, strategyPairLeg2Symbol);
	



	//--------------TD线程先启动--------------------------------------------------------	
	pUserApi_trade->Init();//TD初始化完成后，再进行MD的初始化
	



	//--------------人机交互模块--------------------------------------------------------
	HANDLE hThread1=CreateThread(NULL,0,ThreadProc,NULL,0,NULL);
	CloseHandle(hThread1);
	WaitForSingleObject(hThread1, INFINITE);
	

	if (readMessage.m_strategy_strategytype == "Quote")
	{
		timer_start(kdbSetData, 60000);
		pUserApi_md->Join();//等待接口线程退出
		pUserApi_trade->Join();
		while (true);
	}
	else
	{
		timer_start(kdbSetShortLong, 60000);
		pUserApi_md->Join();//等待接口线程退出
		pUserApi_trade->Join();
		while (true);
	}
	
}



//人机交互函数
DWORD WINAPI ThreadProc(
	LPVOID lpParameter
	)
{
	string str;

	int a;
	cerr<<"--------------------------------------------------------人机交互功能已启动"<<endl;
	cerr<<endl<<"请输入指令(查看持仓:show,强平持仓:close,允许开仓:yxkc, 禁止开仓:jzkc)：";

	while(1)
	{
		cin>>str;
		if(str == "show")
			a = 0;	
		else if(str == "close")
			a = 1;
		else if(str == "yxkc")
			a = 2;
		else if(str == "jzkc")
			a = 3;
		else
			a = 4;

		switch(a){
		case 0:
			{		
				cerr<<"查看账户持仓:"<<endl;
				g_pUserSpi_tradeAll->printTrade_message_map();
				cerr<<"请输入指令(查看持仓:show,强平持仓:close,允许开仓:yxkc, 禁止开仓:jzkc)："<<endl;
				break;
			}
		case 1:
			{
				cerr<<"强平账户持仓:"<<endl;
				g_pUserSpi_tradeAll->ForceClose();
				cerr<<"请输入指令(查看持仓:show,强平持仓:close,允许开仓:yxkc, 禁止开仓:jzkc)："<<endl;
				break;
			}
		case 2:
			{
				cerr<<"允许开仓:"<<endl;
				g_strategy->set_allow_open(true);
				cerr<<"请输入指令(查看持仓:show,强平持仓:close,允许开仓:yxkc, 禁止开仓:jzkc)："<<endl;
				break;

			}
		case 3:
			{
				cerr<<"禁止开仓:"<<endl;
				g_strategy->set_allow_open(false);
				cerr<<"请输入指令(查看持仓:show,强平持仓:close,允许开仓:yxkc, 禁止开仓:jzkc)："<<endl;
				break;

			}
		case 4:
			{
				cerr<<endl<<"输入错误，请重新输入指令(查看持仓:show,强平持仓:close,允许开仓:yxkc, 禁止开仓:jzkc)："<<endl;
				break;
			}


		}

	}

	return 0;

}