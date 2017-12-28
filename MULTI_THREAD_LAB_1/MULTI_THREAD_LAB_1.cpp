// MULTI_THREAD_LAB_1.cpp: определ€ет точку входа дл€ консольного приложени€.
//

#include "stdafx.h"
#include <iostream>		//вводы-ывод
#include <regex>		//регул€рные выражени€
#include <Windows.h>    //WINAPI
#include <deque>		//ƒвухсторонн€ очередь
#include <vector>		//векторы
#include <string>		//строки
#include <fstream>		//файловые потоки


using namespace std;

CRITICAL_SECTION cs;
CRITICAL_SECTION hs;
CRITICAL_SECTION rflag;
CRITICAL_SECTION hflag;
bool rf_end = false;
bool hd_end = false;

struct ResultHandle {
	long int sum;
	long int mul;
	vector<int> number;
};

deque<string> STRINGS;
deque<ResultHandle> RESULT;


DWORD WINAPI ReadThreadProc(LPVOID t)
{
	ifstream ifs;
	ifs.open("Source/data.txt", std::ifstream::in);
	if (ifs.is_open())
	{
		int read_str = 0;
		string _str;
		while (!ifs.eof())
		{
			getline(ifs, _str);
			EnterCriticalSection(&cs);
			STRINGS.push_back(_str);
			LeaveCriticalSection(&cs);
			read_str++;
		}
	}
	else
		cout<<"FILE NOT FOUND";
	EnterCriticalSection(&rflag);
	rf_end = true;
	LeaveCriticalSection(&rflag);
	ifs.close();
	return 0;
  }
DWORD WINAPI HandleStrings(LPVOID p)
{
	regex dig("\\d+");
	smatch match;
	ResultHandle res;
	string local;
M:
	EnterCriticalSection(&cs);
	bool st_flag = STRINGS.empty();
	LeaveCriticalSection(&cs);
	while (!st_flag)
	{
		EnterCriticalSection(&cs);
		local = STRINGS.front();
 		STRINGS.pop_front();
		LeaveCriticalSection(&cs);
		ResultHandle temp;
		while (regex_search(local, match, dig))
		{
			//cout << "El: " << match[0] << "\n";
			temp.number.push_back(stoi(match[0]));
			local = match.suffix().str();
		}
		temp.sum = 0;
		temp.mul = 1;
		for (int i = 0; i < temp.number.size(); i++)
		{
			
			temp.sum += temp.number[i];
			temp.mul *= temp.number[i];
		}
		EnterCriticalSection(&hs);
		RESULT.push_back(temp);
		LeaveCriticalSection(&hs);
		temp.number.clear();
		EnterCriticalSection(&cs);
		st_flag = STRINGS.empty();
		LeaveCriticalSection(&cs);

	}
	EnterCriticalSection(&rflag);
	if (rf_end == false) 
	{ 
		LeaveCriticalSection(&rflag);
		goto M;
	}
	else LeaveCriticalSection(&rflag);	
	EnterCriticalSection(&hflag);
	hd_end = true;
	LeaveCriticalSection(&hflag);
	return 0;
}

DWORD WINAPI WriteToFile(LPVOID p)
{
	ofstream ofs;
	ResultHandle res;
	ofs.open("Source/result.txt",ofstream::out);
N:	
	EnterCriticalSection(&hs);
	bool res_flag = RESULT.empty();
	LeaveCriticalSection(&hs);
	while (!res_flag)
	{
		EnterCriticalSection(&hs);
		res = RESULT.front();
		RESULT.pop_front();
		LeaveCriticalSection(&hs);
		string outst = "";
		for (int i = 0; i < res.number.size(); i++)
		{
			outst.append(to_string(res.number[i])).append(" ");
		}
		outst.append("MUL: ").append(to_string(res.mul)).append(" ");
		outst.append("SUM: ").append(to_string(res.sum)).append("\n");
		cout << outst;
		ofs << outst;
		EnterCriticalSection(&hs);
		res_flag = RESULT.empty();
		LeaveCriticalSection(&hs);
	}
	EnterCriticalSection(&hflag);
	if (hd_end == false)
	{
		LeaveCriticalSection(&hflag);
		goto N;
	}
	else LeaveCriticalSection(&hflag);
	ofs.close();
	return 0;
}

int main()
{
	InitializeCriticalSection(&cs);
	InitializeCriticalSection(&hs);
	InitializeCriticalSection(&rflag);
	InitializeCriticalSection(&hflag);
	HANDLE th  = CreateThread(NULL, 0, ReadThreadProc, NULL, 0, NULL);
	HANDLE th2 = CreateThread(NULL, 0, HandleStrings, NULL, 0, NULL);
	HANDLE th3 = CreateThread(NULL, 0, WriteToFile, NULL, 0, NULL);
	WaitForSingleObject(th, INFINITE);
	WaitForSingleObject(th2, INFINITE);
	WaitForSingleObject(th3, INFINITE);
	getchar();
    return 0;
}

