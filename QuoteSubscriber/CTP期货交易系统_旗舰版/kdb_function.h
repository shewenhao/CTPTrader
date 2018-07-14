#include <iostream>
#include <string>
#include <cstdio>

#ifdef _MSC_VER
#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif
#ifndef KXVER
#define KXVER 3
#endif
#include "k.h"


namespace kdb {
	class Connector;
	class Result;
	std::ostream &operator<<(std::ostream &os, const Result &result);

	enum class Type {
		MixedList = 0,
		Boolean = 1,
		GUID = 2,
		Byte = 4,
		Short = 5,
		Int = 6,
		Long = 7,
		Real = 8,
		Float = 9,
		Char = 10,
		Symbol = 11,
		Timestamp = 12,
		Month = 13,
		Date = 14,
		Datetime = 15,
		Timespan = 16,
		Minute = 17,
		Second = 18,
		Time = 19,
		Table = 98,
		Dict = 99,
		Error = -128
	};
}
class kdb::Connector {
public:
	~Connector();
	bool connect(const char* host, int port, const char* usr_pwd = nullptr, int timeout = 1000);
	void disconnect();
	Result sync(const char* msg);
	void async(const char* msg);
	Result receive(int timeout = 1000);

private:
	std::string host_;
	std::string usr_pwd_;
	int port_ = 0;
	int hdl_ = 0;
};
class kdb::Result {
public:
	Result() = delete;
	Result(K res);
	Result(const Result &r);
	~Result();
	Type type();
	Result & operator = (const Result &r);
	friend std::ostream &operator<<(std::ostream &os, const Result &result);

public:
	K res_;
};
