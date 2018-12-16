#include <afx.h>
#include<fstream>
#include<io.h>
#include<algorithm>
#include "StructFunction.h"

//读取历史K线
void ReadDatas(string fileName, vector<History_data> &history_data_vec)
{
	History_data stu;
	char dataline[512];//行数变量
	ifstream file1(fileName,ios::in);	//以只读方式读入,读取原始数据
	if(!file1){
		cout<<"no such file!"<<endl;
		abort();
	}	
	while(file1.getline(dataline,1024,'\n'))//while开始，读取一行1024够大？
	{
		int len=strlen(dataline);
		char tmp[20];
		for(int i=0,j=0,count=0;i<=len;i++){
			if(dataline[i]!= ',' && dataline[i]!='\0'){
				tmp[j++]=dataline[i];
			}
			else{
				tmp[j]='\0';
				count++;
				j=0;
				switch(count){
				case 1:
					stu.date=tmp;
					break;
				case 2:
					stu.time=tmp;
					break;
				case 3:
					stu.buy1price=atof(tmp);
					break;
				case 4:
					stu.sell1price=atof(tmp);
					break;
				case 5:
					stu.open=atof(tmp);
					break;
				case 6:
					stu.high=atof(tmp);
					break;
				case 7:
					stu.low=atof(tmp);
					break;

					break;
				case 8:
					stu.close=atof(tmp);
					break;

				}
			}
		}
		history_data_vec.push_back(stu);
	}

	file1.close();
}

int Store_fileName(string path, vector<string> &FileName)
{
	struct _finddata_t fileinfo;  
	string in_path;  
	string in_name;  

	//in_path="d:\\future_data";
	in_path = path;
	string curr = in_path+"\\*.txt"; 
	long handle;  

	if((handle=_findfirst(curr.c_str(),&fileinfo))==-1L)  
	{  
		cout<<"没有找到匹配文件!"<<endl;

		return 0;  
	}  

	else  
	{
		do
		{
			in_name=in_path + "\\" +fileinfo.name; 
			FileName.push_back(in_name);
		}while(!(_findnext(handle,&fileinfo)));

		_findclose(handle);

		return 0;

	}
}

string String_StripNum(string s)
{
	s.erase(remove_if(s.begin(), s.end(), [](char c) { return !isalpha(c); }), s.end());
	return s;
}

string String_StripChar(string s)
{
	s.erase(std::remove(s.begin(), s.end(), 'a'), s.end());
	return s;
}

int UpdateTime_Int(const string time)
{
	return atoi(time.substr(0, 2).c_str()) * 3600 + atoi(time.substr(3, 2).c_str()) * 60 + atoi(time.substr(6, 2).c_str());
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

vector<char*> StringSplit(char* splittingstring)
{
	vector<char*> list;
	char *token = strtok(splittingstring, ",");
	while (token != NULL){
		list.push_back(token);
		token = strtok(NULL, ",");
	}
	return list;
}

void SplitString(const string& s, vector<string>& v, const string& c)
{
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}