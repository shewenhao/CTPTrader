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

	char m_mdFront[50];//行情服务器地址
	char m_tradeFront[50];//交易服务器地址

	string m_read_contract;//合约代码

	
};



kdb::Connector kdbConnectorSetGet;
string kdbDataSetPath;
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

string replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return str;
}

string replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return str;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
	return str;
}

void kdbSetData()
{
	/*kdbConnectorSetGet.sync("Quote:-500000#select from Quote;");
	kdbConnectorSetGet.sync("delete from `Quote where Date.minute > 02:30:00, Date.minute <= 09:00:00;delete from `Quote where Date.minute > 11:30 : 00, Date.minute < 13 : 00 : 00;delete from `Quote where Date.minute > 15 : 15 : 00, Date.minute < 21 : 00 : 00; ");
	kdbConnectorSetGet.sync(kdbDataSetPath.c_str());*/
}

void kdbGetData()
{
	//kdbConnectorSetGet.sync(kdbDataGetPath.c_str());
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

//void SetMessage(ReadMessage& readMessage, int kdbPort)//要用引用
//{
//	//-------------------------------读取账号模块-------------------------------
//	CString read_brokerID;
//	//string read_brokerID;
//	GetPrivateProfileString("Account","brokerID","brokerID_error",read_brokerID.GetBuffer(MAX_PATH),MAX_PATH,"./input/AccountParam.ini");
//	
//	CString read_userId;
//	GetPrivateProfileString("Account","userId","userId_error",read_userId.GetBuffer(MAX_PATH),MAX_PATH,"./input/AccountParam.ini");
//
//	CString read_passwd;
//	GetPrivateProfileString("Account","passwd","passwd_error",read_passwd.GetBuffer(MAX_PATH),MAX_PATH,"./input/AccountParam.ini");
//
//	strcpy_s(readMessage.m_appId, read_brokerID);
//	strcpy_s(readMessage.m_userId, read_userId);
//	strcpy_s(readMessage.m_passwd, read_passwd);
//
//
//
//	//-------------------------------读取地址模块-------------------------------
//	CString read_MDAddress;
//	GetPrivateProfileString("FrontAddress","MDAddress","MDAddress_error",read_MDAddress.GetBuffer(MAX_PATH),MAX_PATH,"./input/AccountParam.ini");
//	
//	CString read_TDAddress;
//	GetPrivateProfileString("FrontAddress","TDAddress","TDAddress_error",read_TDAddress.GetBuffer(MAX_PATH),MAX_PATH,"./input/AccountParam.ini");
//	
//	strcpy_s(readMessage.m_mdFront, read_MDAddress);
//	strcpy_s(readMessage.m_tradeFront, read_TDAddress);
//	
//
//	//-------------------------------设置合约模块-------------------------------
//	CString read_contract;
//	GetPrivateProfileString("Contract","contract","contract_error",read_contract.GetBuffer(MAX_PATH),MAX_PATH,"./input/AccountParam.ini");
//	
//	readMessage.m_read_contract = (LPCTSTR)read_contract;
//	kdbConnectorSetGet.connect("localhost", kdbPort);
//	kdbDataSetPath = ExePath(); kdbDataSetPath = kdbDataSetPath.substr(0, kdbDataSetPath.find("CTPTrader")); kdbDataSetPath = replaceAll(kdbDataSetPath, "\\", "/"); kdbScriptExePath = "\\l " + kdbDataSetPath + specificFolderPath; kdbDataGetPath = "Quote: get `:" + kdbDataSetPath + "CTPTrader/Quote;"; kdbDataSetPath = "`:" + kdbDataSetPath + "CTPTrader/Quote set Quote;";
//	kdbGetData();
//}

void SetMessage(ReadMessage& readMessage, int kdbPort)
{
	kdbConnectorSetGet.connect("localhost", kdbPort);
	kdbDataSetPath = ExePath();
	acountParampath = kdbDataSetPath + "/input/AccountParam.csv";
	acountParampath = replaceAll(acountParampath, "\\", "/");
	mdflowPath = kdbDataSetPath + "\\MDflow\\";
	tdflowPath = kdbDataSetPath + "\\TDflow\\";
	//kdbDataSetPath = kdbDataSetPath.substr(0, kdbDataSetPath.find("CTPTrader")); 
	kdbDataSetPath = replaceAll(kdbDataSetPath, "\\", "/"); 
	kdbScriptExePath = "\\l " + kdbDataSetPath + specificFolderPath; 
	kdbDataGetPath = "Quote: get `:" + kdbDataSetPath + "/Quote;"; 
	kdbDataSetPath = "`:" + kdbDataSetPath + "/Quote set Quote;";
	kdbGetData();

	string infor_line;
	string infor_cell;
	cout << "check AccountParam.csv Right" << endl;
	ifstream Config_Stream(acountParampath);
	int count_infor = 0;
	
	if (Config_Stream.is_open())
	{
		getline(Config_Stream, infor_line);
		stringstream   lineStream(infor_line);

		while (count_infor < 6)
		{			
			getline(lineStream, infor_cell, '|');
			cout << infor_cell << endl;
			if (count_infor == 0)
			{	
				strcpy_s(readMessage.m_appId, infor_cell.c_str());
			}
			if (count_infor == 1)
			{
				strcpy_s(readMessage.m_userId, infor_cell.c_str());
			}
			if (count_infor == 2)
			{
				strcpy_s(readMessage.m_passwd, infor_cell.c_str());
			}
			if (count_infor == 3)
			{
				strcpy_s(readMessage.m_mdFront, infor_cell.c_str());
			}
			if (count_infor == 4)
			{
				strcpy_s(readMessage.m_tradeFront, infor_cell.c_str());
			}
			if (count_infor == 5)
			{
				readMessage.m_read_contract = infor_cell;
			}

			count_infor++;
		}
		Config_Stream.close();
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