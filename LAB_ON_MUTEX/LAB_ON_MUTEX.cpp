// LAB_ON_MUTEX.cpp: определ€ет точку входа дл€ консольного приложени€.
//

#include "stdafx.h"
#include <iostream>		//вводы-вывод
#include <regex>		//регул€рные выражени€
#include <Windows.h>    //WINAPI
#include <deque>		//ƒвухсторонн€ очередь
#include <vector>		//векторы
#include <string>		//строки
#include <fstream>		//файловые потоки
using namespace std;

HANDLE th_rd_mutex;
HANDLE str_mutex;
HANDLE th_hd_mutex;
HANDLE res_mutex;

bool rf_end = false;
bool hd_end = false;

struct ResultHandle {
	long int sum;
	long int mul;
	vector<int> number;
};

deque<string> Strings;
deque<ResultHandle> Results;

string PrintResult(ResultHandle resulthandle)
{
	string result = "";
	for (int i = 0; i < resulthandle.number.size(); i++)
	{
		result.append(to_string(resulthandle.number[i])).append(" ");
	}
	result.append("MUL: ").append(to_string(resulthandle.mul)).append(" ");
	result.append("SUM: ").append(to_string(resulthandle.sum)).append("\n");
	return result;
}

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
			WaitForSingleObject(str_mutex,INFINITE);
			Strings.push_back(_str);
			ReleaseMutex(str_mutex);
			read_str++;
		}
	}
	else
		cout << "FILE NOT FOUND";
	WaitForSingleObject(th_rd_mutex, INFINITE);
	rf_end = true;
	ReleaseMutex(th_rd_mutex);
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
	WaitForSingleObject(str_mutex, INFINITE);
	bool st_flag = Strings.empty();
	ReleaseMutex(str_mutex);
	while (!st_flag)
	{
		WaitForSingleObject(str_mutex, INFINITE);
		local = Strings.front();
		Strings.pop_front();
		ReleaseMutex(str_mutex);
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
		WaitForSingleObject(res_mutex, INFINITE);
		Results.push_back(temp);
		ReleaseMutex(res_mutex);
		temp.number.clear();
		WaitForSingleObject(str_mutex, INFINITE);;
		st_flag = Strings.empty();
		ReleaseMutex(str_mutex);

	}
	WaitForSingleObject(th_rd_mutex, INFINITE);
	if (rf_end == false)
	{
		ReleaseMutex(th_rd_mutex);
		goto M;
	}
	else ReleaseMutex(th_rd_mutex);

	WaitForSingleObject(th_hd_mutex, INFINITE);
	hd_end = true;
	ReleaseMutex(th_hd_mutex);
	return 0;
}

DWORD WINAPI WriteToFile(LPVOID p)
{
	ofstream ofs;
	ResultHandle res;
	ofs.open("Source/result.txt", ofstream::out);
N:
	WaitForSingleObject(res_mutex,INFINITE);
	bool res_flag = Results.empty();
	ReleaseMutex(res_mutex);
	while (!res_flag)
	{
		WaitForSingleObject(res_mutex, INFINITE);
		res = Results.front();
		Results.pop_front();
		ReleaseMutex(res_mutex);
		string outst = PrintResult(res);
		cout << outst;
		ofs << outst;
		WaitForSingleObject(res_mutex,INFINITE);
		res_flag = Results.empty();
		ReleaseMutex(res_mutex);
	}
	WaitForSingleObject(th_hd_mutex,INFINITE);
	if (hd_end == false)
	{
		ReleaseMutex(th_hd_mutex);
		goto N;
	}
	else ReleaseMutex(th_hd_mutex);
	ofs.close();
	return 0;
}


int main()
{
 	th_rd_mutex = CreateMutex(NULL, FALSE, NULL);
	str_mutex = CreateMutex(NULL, FALSE, NULL);
	th_hd_mutex = CreateMutex(NULL, FALSE, NULL);
	res_mutex = CreateMutex(NULL, FALSE, NULL);
	HANDLE th = CreateThread(NULL, 0, ReadThreadProc, NULL, 0, NULL);
	HANDLE th2 = CreateThread(NULL, 0, HandleStrings, NULL, 0, NULL);
	HANDLE th3 = CreateThread(NULL, 0, WriteToFile, NULL, 0, NULL);
	WaitForSingleObject(th,INFINITE);
	WaitForSingleObject(th2, INFINITE);
	WaitForSingleObject(th3, INFINITE);
	getchar();
	return 0;
}

