#ifndef TEST_H_
#define TEST_H_

#include "ThostFtdcUserApiStruct.h"
#include <string>
#include <iostream>
#ifndef KXVER
#define KXVER 3
#include "k.h"
#include "kdb_function.h"
#endif

using namespace std;


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



void SetMessage(ReadMessage& readMessage)//要用引用
{
	//-------------------------------读取账号模块-------------------------------
	CString read_brokerID;
	GetPrivateProfileString("Account","brokerID","brokerID_error",read_brokerID.GetBuffer(MAX_PATH),MAX_PATH,"./input/AccountParam.ini");
	
	CString read_userId;
	GetPrivateProfileString("Account","userId","userId_error",read_userId.GetBuffer(MAX_PATH),MAX_PATH,"./input/AccountParam.ini");

	CString read_passwd;
	GetPrivateProfileString("Account","passwd","passwd_error",read_passwd.GetBuffer(MAX_PATH),MAX_PATH,"./input/AccountParam.ini");

	strcpy_s(readMessage.m_appId, read_brokerID);
	strcpy_s(readMessage.m_userId, read_userId);
	strcpy_s(readMessage.m_passwd, read_passwd);



	//-------------------------------读取地址模块-------------------------------
	CString read_MDAddress;
	GetPrivateProfileString("FrontAddress","MDAddress","MDAddress_error",read_MDAddress.GetBuffer(MAX_PATH),MAX_PATH,"./input/AccountParam.ini");
	
	CString read_TDAddress;
	GetPrivateProfileString("FrontAddress","TDAddress","TDAddress_error",read_TDAddress.GetBuffer(MAX_PATH),MAX_PATH,"./input/AccountParam.ini");
		
	strcpy_s(readMessage.m_mdFront, read_MDAddress);
	strcpy_s(readMessage.m_tradeFront, read_TDAddress);
	

	//-------------------------------设置合约模块-------------------------------
	CString read_contract;
	GetPrivateProfileString("Contract","contract","contract_error",read_contract.GetBuffer(MAX_PATH),MAX_PATH,"./input/AccountParam.ini");
	
	readMessage.m_read_contract = (LPCTSTR)read_contract;
	
}



#endif