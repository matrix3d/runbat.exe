// runbat.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include <iostream>
#include<Windows.h>
#include <string>
#include <fstream>
#include <atlstr.h>
#include <direct.h>
#include <CString>
#include <vector>
#include <stdio.h>
#include <io.h>
#include <regex>
#include "CMyINI.h"
#define MY_PIPE_BUFFER_SIZE 102400
LPCWSTR stringToLPCWSTR(std::string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

	return wcstring;
}
std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

std::wstring getMyDirectory()
{
	std::wstring wstr;
	unsigned long size = GetCurrentDirectory(0, NULL);
	wchar_t* path = new wchar_t[size];
	if (GetCurrentDirectory(size, path) != 0)
	{
		wstr = path;
	}
	delete[] path;
	return wstr;
}

//https://www.jb51.net/article/37627.htm
wchar_t* CharToWchar(const char* c)
{
	//Release();
	int len = MultiByteToWideChar(CP_ACP, 0, c, strlen(c), NULL, 0);
	wchar_t* m_wchar = new wchar_t[len + 1];
	MultiByteToWideChar(CP_ACP, 0, c, strlen(c), m_wchar, len);
	m_wchar[len] = '\0';
	return m_wchar;
}

wchar_t* StringToWchar(const std::string& s)
{
	const char* p = s.c_str();
	return CharToWchar(p);
}

bool StartProcess(LPCWSTR program, LPCWSTR args)
{
	bool ret = true;
	printf("1");
	//初始化管道
	HANDLE hPipeRead;
	HANDLE hPipeWrite;
	SECURITY_ATTRIBUTES saOutPipe;
	::ZeroMemory(&saOutPipe, sizeof(saOutPipe));
	saOutPipe.nLength = sizeof(SECURITY_ATTRIBUTES);
	saOutPipe.lpSecurityDescriptor = NULL;
	saOutPipe.bInheritHandle = TRUE;
	if (CreatePipe(&hPipeRead, &hPipeWrite, &saOutPipe, MY_PIPE_BUFFER_SIZE))
	{

		printf("2");
		PROCESS_INFORMATION processInfo;
		::ZeroMemory(&processInfo, sizeof(processInfo));
		STARTUPINFO startupInfo;
		::ZeroMemory(&startupInfo, sizeof(startupInfo));
		startupInfo.cb = sizeof(STARTUPINFO);
		startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		startupInfo.hStdOutput = hPipeWrite;
		startupInfo.hStdError = hPipeWrite;
		startupInfo.wShowWindow = SW_HIDE;

		if (::CreateProcessW(program, (LPWSTR)args,
			NULL,  // process security
			NULL,  // thread security
			TRUE,  //inheritance
			0,     //no startup flags
			NULL,  // no special environment
			NULL,  //default startup directory
			&startupInfo,
			&processInfo))
		{

			printf("3");
			if (WAIT_TIMEOUT != WaitForSingleObject(processInfo.hProcess, 3000))
			{
				printf("4");
				DWORD dwReadLen = 0;
				DWORD dwStdLen = 0;
				if (PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwReadLen, NULL) && dwReadLen > 0)
				{
					printf("5");
					char szPipeOut[MY_PIPE_BUFFER_SIZE];
					::ZeroMemory(szPipeOut, sizeof(szPipeOut),MY_PIPE_BUFFER_SIZE);
					if (ReadFile(hPipeRead, szPipeOut, dwReadLen, &dwStdLen, NULL))
					{
						szPipeOut[MY_PIPE_BUFFER_SIZE - 1] = 0;
						printf("%s",szPipeOut);
						std::string errtxt = "invocation forwarded to primary instance";
						if (errtxt.compare(szPipeOut)) {
							printf("相等");
							ret = false;
						}
						else {
							printf("不相等");
						}

						// 输出值
						int k = 0;
					}
					int a = 1;
				}
			}
			else {
				printf("6");
			}
		}
		if (processInfo.hProcess)
		{
			CloseHandle(processInfo.hProcess);
		}
		if (processInfo.hThread)
		{
			CloseHandle(processInfo.hThread);
		}
	}
	CloseHandle(hPipeRead);
	CloseHandle(hPipeWrite);
	return ret;
}

std::string DwordToString(DWORD val)
{
	std::string cur_str = std::to_string(long long(val));
	return cur_str;
}
int main()
{
	CMyINI* p = new CMyINI();
	if (p->ReadINI("Setting.ini")==0) {
		p->SetValue("config", "width", "1024");
		p->SetValue("config", "height", "768");
		p->WriteINI("Setting.ini");
	}
	std::string w= p->GetValue("config","width");
	std::string h= p->GetValue("config","height");
	printf("width=%s",w.c_str());
	std::string wh = w + "x" + h + ":" + h + "x" + w;
	

	const char* to_search = "*.xml";
	long handle;                                                //用于查找的句柄
	struct _finddata_t fileinfo;                          //文件信息的结构体
	handle = _findfirst(to_search, &fileinfo);         //第一次查找
	if (-1 == handle)
		return -1;
	printf("%s\n", fileinfo.name); 
	_findclose(handle);

	int md= _mkdir("xml");
	int i = 0;
	while (true) {
		printf("true\n");
		std::ifstream myfile(fileinfo.name);
		std::string fileinfoname =fileinfo.name;
		fileinfoname = "xml\\" + fileinfoname;
		std::string outfilename =fileinfoname.replace(fileinfoname.end()-4, fileinfoname.end(),std::to_string(i));
		i++;
		std::ifstream myoutfile(outfilename);
		std::ofstream outfile(outfilename, std::ios::out);
		std::string temp;
		while (std::getline(myfile, temp))
		{
			const std::regex pattern("(<id>(.+)?</id>)");// regular expression with two capture groups
			const std::regex pattern2("(<title>(.+)?</title>)");// regular expression with two capture groups
			if (std::regex_search(temp,pattern)) {
				std::string replace = "<id>$2."+std::to_string(i)+"</id>"; //$2表示匹配模式串的第二个字串，即以a,e,i,o,u开头的单词
				std::string newtext = std::regex_replace(temp, pattern, replace);
				outfile << newtext;
			}else if (std::regex_search(temp, pattern2)) {
				std::string replace = "<title>$2." + std::to_string(i) + "</title>"; //$2表示匹配模式串的第二个字串，即以a,e,i,o,u开头的单词
				std::string newtext = std::regex_replace(temp, pattern2, replace);
				outfile << newtext;
			}
			else
			{
				outfile << temp;
			}
			outfile << std::endl;
		}
		outfile.close();
		myfile.close();
			
		std::string cmd = "AIRSDK\\bin\\adl64 -screensize "+wh+" " + outfilename + " .";
		//std::string cmd = "AIRSDK\\bin\\adl64 -screensize 1559x720:720x1559 xml\\132 .";
		printf(cmd.c_str());
		wchar_t* CommandLine = StringToWchar(cmd); //const_cast<wchar_t*>(L"AIRSDK\\bin\\adl64.exe 1.xml"+outfilename);
		printf("aaaa");
		if (StartProcess(TEXT("AIRSDK\\bin\\adl64.exe"), CommandLine)) {
			break;
		}
	}
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
