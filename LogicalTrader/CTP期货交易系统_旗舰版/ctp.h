#ifndef TEST_H_
#define TEST_H_

#include "ThostFtdcUserApiStruct.h"
#include <string>
#include <iostream>
#include <thread>
#include <functional>
#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip>
#include <vector>
#include <fstream>
#include "StructFunction.h"
#ifndef KXVER
#define KXVER 3
#include "k.h"
#include "kdb_function.h"
#endif

using namespace std;
using namespace kdb;


//保存读取的信息的结构体
struct ReadMessage
{
	TThostFtdcBrokerIDType	m_appId;//经纪公司代码
	TThostFtdcUserIDType	m_userId;//用户名
	TThostFtdcPasswordType	m_passwd;//密码
	TThostFtdcProductInfoType	m_userProductInfo;
	TThostFtdcAuthCodeType	m_authCode;

	char m_mdFront[50];//行情服务器地址
	char m_tradeFront[50];//交易服务器地址

	string m_read_contract;//合约代码

	string m_read_contractdecoration;//合约代码

	int m_quote_kdbPort;
	int m_kdbPort;

	string m_strategy_strategytype = "";

	int m_strategy_volumetarget = 1;

	string m_strategy_kdbscript = "";

	double m_strategy_par1 = 0.0;

	double m_strategy_par2 = 0.0;

	double m_strategy_par3 = 0.0;

	double m_strategy_par4 = 0.0;

	double m_strategy_par5 = 0.0;
	
	double m_strategy_par6 = 0.0;

	int m_strategy_orderType1 = 0;

	int m_strategy_orderType2 = 0;

	int m_strategy_orderType3 = 0;

	int m_strategy_orderType4 = 0;

	int m_strategy_orderType5 = 0;

	int m_strategy_orderType6 = 0;
};

kdb::Connector kdbConnectorSetGet;
string kdbShortLongSetPath;
string kdbShortSetPath;
string kdbDataSetPath;
string kdbDataQuoteSetPath;
string kdbDataGetPath;
string specificFolderPath = "/KDB_Scripts/";
string kdbScriptExePath;
string acountParampath;
string mdflowPath;
string tdflowPath;


string ExePath() {
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos);
}

//string replace(std::string& str, const std::string& from, const std::string& to) {
//	size_t start_pos = str.find(from);
//	if (start_pos == std::string::npos)
//		return false;
//	str.replace(start_pos, from.length(), to);
//	return str;
//}

//string replaceAll(std::string& str, const std::string& from, const std::string& to) {
//	if (from.empty())
//		return str;
//	size_t start_pos = 0;
//	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
//		str.replace(start_pos, from.length(), to);
//		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
//	}
//	return str;
//}

void kdbSetData()
{
	kdbConnectorSetGet.sync("Quote:-500000#select from Quote;"); ////tempQuote:select by ReceiveDate.date, 1 xbar ReceiveDate.second, Symbol from Quote;delete date, second from `tempQuote;Quote: select Date, ReceiveDate, Symbol, BidPrice1, BidVol1, AskPrice1, AskVol1 from tempQuote; 
	kdbConnectorSetGet.sync("delete from `Quote where Date.minute > 02:30:00, Date.minute <= 09:00:00;delete from `Quote where Date.minute > 11:30:00, Date.minute < 13:00:00;delete from `Quote where Date.minute > 15:15:00, Date.minute < 21:00:00;");
	kdbConnectorSetGet.sync(kdbDataQuoteSetPath.c_str());
	//g_strategy->DataRebootDADataSource();
}


void kdbSetShortLong()
{
	//kdbConnectorSetGet.sync(kdbShortLongSetPath.c_str());
}

void kdbGetData()
{
	kdbConnectorSetGet.sync(kdbDataGetPath.c_str());
} 

string return_current_time_and_date1()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y.%m.%dD%X");
	auto duration = now.time_since_epoch();

	typedef std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<8>
	>::type> Days; /* UTC: +8:00 */
	Days days = std::chrono::duration_cast<Days>(duration);
	duration -= days;
	auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
	duration -= hours;
	auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
	duration -= minutes;
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
	duration -= seconds;
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
	duration -= milliseconds;
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
	duration -= microseconds;
	auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
	cout << (ss.str()).append(".").append(to_string(milliseconds.count())) << endl;
	return (ss.str()).append(".").append(to_string(milliseconds.count()));
}

void SetMessage(ReadMessage& readMessage, string *exePath, int *m_quote_kdbPort, int *kdbPort, int *strategyVolumeTarget, string *strategykdbscript, double *par1, double *par2, double *par3, double *par4, double *par5, double *par6, int *strategy_orderType1, int *strategy_orderType2, int *strategy_orderType3, int *strategy_orderType4, int *strategy_orderType5, int *strategy_orderType6, string strategyAccountParampath, string *strategyPairLeg1Symbol, string *strategyPairLeg2Symbol, string *strategyPairLeg3Symbol)//要用引用
{
	kdbDataSetPath = ExePath();
	*exePath = kdbDataSetPath;
	acountParampath = kdbDataSetPath + "/Input/" + strategyAccountParampath;
	acountParampath = replaceAll(acountParampath, "\\", "/");
	mdflowPath = kdbDataSetPath + "\\MDflow\\";
	tdflowPath = kdbDataSetPath + "\\TDflow\\";
	kdbDataSetPath = replaceAll(kdbDataSetPath, "\\", "/");
	kdbScriptExePath = "\\l " + kdbDataSetPath + specificFolderPath;
	kdbDataGetPath = "Quote: get `:" + kdbDataSetPath + "/Quote;";
	kdbDataSetPath = "`:" + kdbDataSetPath + "/Quote set Quote;";
	kdbDataQuoteSetPath = kdbDataSetPath;
	kdbShortLongSetPath = replaceAll(kdbDataSetPath, "Quote", "ShortLong");
	//kdbGetData();

	//-------------------------------读取账号模块-------------------------------
	CString read_brokerID;
	GetPrivateProfileString("Account", "brokerid", "brokerID_error", read_brokerID.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	
	CString read_userId;
	GetPrivateProfileString("Account", "userid", "userId_error", read_userId.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());

	CString read_passwd;
	GetPrivateProfileString("Account", "passwd", "passwd_error", read_passwd.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());

	CString read_userProductInfo;
	GetPrivateProfileString("Account", "userproductinfo", "", read_userProductInfo.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());

	CString read_authCode;
	GetPrivateProfileString("Account", "authcode", "", read_authCode.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());

	strcpy_s(readMessage.m_appId, read_brokerID);
	strcpy_s(readMessage.m_userId, read_userId);
	strcpy_s(readMessage.m_passwd, read_passwd);
	strcpy_s(readMessage.m_userProductInfo, read_userProductInfo);
	strcpy_s(readMessage.m_authCode, read_authCode);

	//-------------------------------读取地址模块-------------------------------
	CString read_MDAddress;
	GetPrivateProfileString("FrontAddress", "MDAddress", "MDAddress_error", read_MDAddress.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	
	CString read_TDAddress;
	GetPrivateProfileString("FrontAddress", "TDAddress", "TDAddress_error", read_TDAddress.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
		
	strcpy_s(readMessage.m_mdFront, read_MDAddress);
	strcpy_s(readMessage.m_tradeFront, read_TDAddress);
	
	//-------------------------------设置合约模块-------------------------------
	CString read_contractdecoration;
	GetPrivateProfileString("Contract", "contractdecoration", "contractdecoration_error", read_contractdecoration.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());

	readMessage.m_read_contractdecoration = (LPCTSTR)read_contractdecoration;

	CString read_contract;
	GetPrivateProfileString("Contract", "contract", "contract_error", read_contract.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	
	readMessage.m_read_contract = (LPCTSTR)read_contract;

	//-------------------------------读取kdb模块-------------------------------
	CString read_m_kdbPort;
	GetPrivateProfileString("DataBaseAddress", "kdbPort", "kdbPort_error", read_m_kdbPort.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_quote_kdbPort = atoi(read_m_kdbPort);
	*m_quote_kdbPort = atoi(read_m_kdbPort);
	//kdbConnectorSetGet.connect("localhost", *kdbPort);

	CString read_kdbPort;
	GetPrivateProfileString("DataBaseAddress", "kdbPort", "kdbPort_error", read_kdbPort.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_kdbPort = atoi(read_kdbPort);
	*kdbPort = atoi(read_kdbPort);
	kdbConnectorSetGet.connect("localhost", *kdbPort);

	//-------------------------------设置策略与否-------------------------------
	CString read_StrategyType;
	GetPrivateProfileString("StrategySetting", "StrategyType", "StrategySetting_error", read_StrategyType.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());

	readMessage.m_strategy_strategytype = read_StrategyType;
	//*strategyVolumeTarget = readMessage.m_strategy_strategyorquote;
	
	//-------------------------------设置策略仓位-------------------------------
	CString read_strategy_volumeTarget;
	GetPrivateProfileString("StrategySetting", "VolumeTarget", "StrategySetting_error", read_strategy_volumeTarget.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());

	readMessage.m_strategy_volumetarget = atoi(read_strategy_volumeTarget);
	*strategyVolumeTarget = readMessage.m_strategy_volumetarget;

	//-------------------------------设置策略卷轴-------------------------------
	CString read_strategy_kdbscript;
	GetPrivateProfileString("StrategySetting", "kdbScript", "StrategySetting_error", read_strategy_kdbscript.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());

	readMessage.m_strategy_kdbscript = read_strategy_kdbscript;
	cout << readMessage.m_strategy_kdbscript << endl;
	*strategykdbscript = readMessage.m_strategy_kdbscript;

	//-------------------------------设置策略参数-------------------------------
	CString read_par1;
	GetPrivateProfileString("StrategySetting", "par1", "StrategySetting_error", read_par1.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_strategy_par1 = atof(read_par1);
	*par1 = readMessage.m_strategy_par1;

	CString read_par2;
	GetPrivateProfileString("StrategySetting", "par2", "StrategySetting_error", read_par2.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_strategy_par2 = atof(read_par2);
	*par2 = readMessage.m_strategy_par2;

	CString read_par3;
	GetPrivateProfileString("StrategySetting", "par3", "StrategySetting_error", read_par3.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_strategy_par3 = atof(read_par3);
	*par3 = readMessage.m_strategy_par3;

	CString read_par4;
	GetPrivateProfileString("StrategySetting", "par4", "StrategySetting_error", read_par4.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_strategy_par4 = atof(read_par4);
	*par4 = readMessage.m_strategy_par4;

	CString read_par5;
	GetPrivateProfileString("StrategySetting", "par5", "StrategySetting_error", read_par5.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_strategy_par5 = atof(read_par5);
	*par5 = readMessage.m_strategy_par5;

	CString read_par6;
	GetPrivateProfileString("StrategySetting", "par6", "StrategySetting_error", read_par6.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_strategy_par6 = atof(read_par6);
	*par6 = readMessage.m_strategy_par6;

	//-------------------------------设置订单类型-------------------------------
	CString read_strategy_orderType1;
	GetPrivateProfileString("StrategySetting", "strategy_orderType1", "StrategySetting_error", read_strategy_orderType1.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_strategy_orderType1 = atoi(read_strategy_orderType1);
	*strategy_orderType1 = readMessage.m_strategy_orderType1;

	CString read_strategy_orderType2;
	GetPrivateProfileString("StrategySetting", "strategy_orderType2", "StrategySetting_error", read_strategy_orderType2.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_strategy_orderType2 = atoi(read_strategy_orderType2);
	*strategy_orderType2 = readMessage.m_strategy_orderType2;

	CString read_strategy_orderType3;
	GetPrivateProfileString("StrategySetting", "strategy_orderType3", "StrategySetting_error", read_strategy_orderType3.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_strategy_orderType3 = atoi(read_strategy_orderType3);
	*strategy_orderType3 = readMessage.m_strategy_orderType3;

	CString read_strategy_orderType4;
	GetPrivateProfileString("StrategySetting", "strategy_orderType4", "StrategySetting_error", read_strategy_orderType4.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_strategy_orderType4 = atoi(read_strategy_orderType4);
	*strategy_orderType4 = readMessage.m_strategy_orderType4;

	CString read_strategy_orderType5;
	GetPrivateProfileString("StrategySetting", "strategy_orderType5", "StrategySetting_error", read_strategy_orderType5.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_strategy_orderType5 = atoi(read_strategy_orderType5);
	*strategy_orderType5 = readMessage.m_strategy_orderType5;

	CString read_strategy_orderType6;
	GetPrivateProfileString("StrategySetting", "strategy_orderType6", "StrategySetting_error", read_strategy_orderType6.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	readMessage.m_strategy_orderType6 = atoi(read_strategy_orderType6);
	*strategy_orderType6 = readMessage.m_strategy_orderType6;

	//-------------------------------设置数据合约-------------------------------
	CString read_strategyPairLeg1Symbol;
	GetPrivateProfileString("StrategySetting", "strategyPairLeg1Symbol", "StrategySetting_error", read_strategyPairLeg1Symbol.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	*strategyPairLeg1Symbol = read_strategyPairLeg1Symbol;

	CString read_strategyPairLeg2Symbol;
	GetPrivateProfileString("StrategySetting", "strategyPairLeg2Symbol", "StrategySetting_error", read_strategyPairLeg2Symbol.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	*strategyPairLeg2Symbol = read_strategyPairLeg2Symbol;

	CString read_strategyPairLeg3Symbol;
	GetPrivateProfileString("StrategySetting", "strategyPairLeg3Symbol", "StrategySetting_error", read_strategyPairLeg3Symbol.GetBuffer(MAX_PATH), MAX_PATH, acountParampath.c_str());
	*strategyPairLeg3Symbol = read_strategyPairLeg3Symbol;

	//-------------------------------设置初始工作-------------------------------
	if (readMessage.m_strategy_strategytype == "Quote")
	{
		kdbGetData();
	}
}

void timer_start(std::function<void(void)> func, unsigned int interval)
{
	std::thread([func, interval]()
	{
		while (true)
		{
			auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
			func();
			std::this_thread::sleep_until(x);
		}
	}).detach();
}




#endif