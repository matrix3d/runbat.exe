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
#define MY_PIPE_BUFFER_SIZE 4096
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


typedef struct _ASTAT_
{
	ADAPTER_STATUS adapt;
	NAME_BUFFER NameBuff[30];
} ASTAT, * PASTAT;

/*void getMac(char* mac)
{
	ASTAT Adapter;
	NCB Ncb;
	UCHAR uRetCode;
	LANA_ENUM lenum;
	int i = 0;

	memset(&Ncb, 0, sizeof(Ncb));
	Ncb.ncb_command = NCBENUM;
	Ncb.ncb_buffer = (UCHAR*)&lenum;
	Ncb.ncb_length = sizeof(lenum);

	uRetCode = Netbios(&Ncb);
	printf("The NCBENUM return adapter number is: %d \n ", lenum.length);
	for (i = 0; i < lenum.length; i++)
	{
		memset(&Ncb, 0, sizeof(Ncb));
		Ncb.ncb_command = NCBRESET;
		Ncb.ncb_lana_num = lenum.lana[i];
		uRetCode = Netbios(&Ncb);

		memset(&Ncb, 0, sizeof(Ncb));
		Ncb.ncb_command = NCBASTAT;
		Ncb.ncb_lana_num = lenum.lana[i];
		const char* str1 = "* ";
		strcpy_s((char*)Ncb.ncb_callname,strlen(str1)+1,str1);
		Ncb.ncb_buffer = (unsigned char*)&Adapter;
		Ncb.ncb_length = sizeof(Adapter);
		uRetCode = Netbios(&Ncb);

		if (uRetCode == 0)
		{
			sprintf_s(mac,sizeof(mac), "%02x-%02x-%02x-%02x-%02x-%02x ",
			//sprintf_s(mac, "%02X%02X%02X%02X%02X%02X ",
				Adapter.adapt.adapter_address[0],
				Adapter.adapt.adapter_address[1],
				Adapter.adapt.adapter_address[2],
				Adapter.adapt.adapter_address[3],
				Adapter.adapt.adapter_address[4],
				Adapter.adapt.adapter_address[5]
			);
			//printf( "The Ethernet Number on LANA %d is: %s\n ", lenum.lana[i], mac);
		}
	}
}*/

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
	//WIN32_FILE_ATTRIBUTE_DATA    attr;     //文件属性结构体
	//TCHAR file[20] = L"C:\\";     //文件名
	//GetFileAttributesEx(file, GetFileExInfoStandard, &attr);        //获取文件属性
	//FILETIME createTime = attr.ftCreationTime;                    //获取文件时间
	//FILETIME accessTime = attr.ftLastAccessTime;
	//FILETIME modifyTime = attr.ftLastWriteTime;
	//SYSTEMTIME time;                                                     //系统时间结构体
	//FileTimeToSystemTime(&createTime, &time);             //将文件事件转换为系统时间
	//printf("注册码\n");
	//std::string thetime = DwordToString(createTime.dwHighDateTime);
	//printf(thetime.c_str());
	//printf("\n");
	//std::ifstream timefile("注册码.txt");
	//if (timefile.is_open()) {
	//}
	//else
	//{
		//printf("放入注册码.txt\n");
		//char name[50];
		//std::cin >> name;
	//}

	//char* mac = new char[32];
	//getMac(mac);
	//printf("%s\n ", mac);
    //std::cout << "Hello World!\n"; 
	//system("run.bat");
	//run py
	//WinExec("cmd /c run.bat", SW_HIDE);
	//run bat

	//run air
	//读写文件
	
	const char* to_search = "*.xml";
	long handle;                                                //用于查找的句柄
	struct _finddata_t fileinfo;                          //文件信息的结构体
	handle = _findfirst(to_search, &fileinfo);         //第一次查找
	if (-1 == handle)
		return -1;
	printf("%s\n", fileinfo.name);                         //打印出找到的文件的文件名
	//while (!_findnext(handle, &fileinfo))               //循环查找其他符合的文件，知道找不到其他的为止
	//{
	//	printf("%s\n", fileinfo.name);
	//}
	_findclose(handle);

	_mkdir("xml");

	int i = 0;
	while (true) {
		printf("true\n");
		std::ifstream myfile(fileinfo.name);
		std::string fileinfoname =fileinfo.name;
		fileinfoname = "xml\\" + fileinfoname;
		std::string outfilename =fileinfoname.replace(fileinfoname.end()-4, fileinfoname.end(),std::to_string(i));
		i++;
		std::ifstream myoutfile(outfilename);
		//if (myoutfile.is_open()) {
		//	myoutfile.close();
		//}else {
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

			
			std::string cmd = "AIRSDK\\bin\\adl64 -screensize 1559x720:720x1559 " + outfilename + " .";
			//std::string cmd = "AIRSDK\\bin\\adl64 -screensize 1559x720:720x1559 xml\\132 .";
			printf(cmd.c_str());
			wchar_t* CommandLine = StringToWchar(cmd); //const_cast<wchar_t*>(L"AIRSDK\\bin\\adl64.exe 1.xml"+outfilename);
			printf("aaaa");
			if (StartProcess(TEXT("AIRSDK\\bin\\adl64.exe"), CommandLine)) {
				break;
			}

			//return 0;
			//初始化管道
			/*HANDLE hPipeRead;
			HANDLE hPipeWrite;
			SECURITY_ATTRIBUTES saOutPipe;
			::ZeroMemory(&saOutPipe, sizeof(saOutPipe));
			saOutPipe.nLength = sizeof(SECURITY_ATTRIBUTES);
			saOutPipe.lpSecurityDescriptor = NULL;
			saOutPipe.bInheritHandle = TRUE;
			if (CreatePipe(&hPipeRead, &hPipeWrite, &saOutPipe, MY_PIPE_BUFFER_SIZE))
			{
				//WinExec(cmd.c_str(), SW_HIDE);
				STARTUPINFO si;
				memset(&si, 0, sizeof(STARTUPINFO));//初始化si在内存块中的值（详见memset函数）
				si.cb = sizeof(STARTUPINFO);
				si.dwFlags = STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_HIDE;
				si.hStdOutput = hPipeWrite;
				si.hStdError = hPipeWrite;
				//si.hStdInput = hPipeRead;
				PROCESS_INFORMATION pi;//必备参数设置结束

				std::string cmd = "AIRSDK\\bin\\adl64 -screensize 1559x720:720x1559 " + outfilename + "111 .";
				printf(cmd.c_str());
				wchar_t* CommandLine = StringToWchar(cmd); //const_cast<wchar_t*>(L"AIRSDK\\bin\\adl64.exe 1.xml"+outfilename);
				if (CreateProcess(
					TEXT("AIRSDK\\bin\\adl64.exe"),
					CommandLine,
					NULL,
					NULL,
					TRUE,
					0,
					NULL,
					NULL,
					&si,
					&pi
				)) {
					if (WAIT_TIMEOUT != WaitForSingleObject(pi.hProcess, 3000))
					{
						printf("aaaa");
						DWORD dwReadLen = 0;
						DWORD dwStdLen = 0;
						if (PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwReadLen, NULL) && dwReadLen > 0)
						{
							printf("bbbb");
							char szPipeOut[MY_PIPE_BUFFER_SIZE];
							::ZeroMemory(szPipeOut, sizeof(szPipeOut));
							if (ReadFile(hPipeRead, szPipeOut, dwReadLen, &dwStdLen, NULL))
							{
								printf("1213213213");
								printf(szPipeOut);
								// 输出值
								int k = 0;
							}
							int a = 1;
						}
					}
				}
				//不使用的句柄最好关掉
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
			}
			CloseHandle(hPipeRead);
			CloseHandle(hPipeWrite);*/
			//break;
		//}
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
