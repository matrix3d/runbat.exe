// runbat.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include <fstream>
#include <Windows.h>
#include <string>
#include <direct.h>
#include <stdio.h>
#include <io.h>
#include <regex>
#include "CMyINI.h"
#define MY_PIPE_BUFFER_SIZE 409600

//https://www.jb51.net/article/37627.htm
wchar_t* CharToWchar(const char* c)
{
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
			if (WAIT_TIMEOUT != WaitForSingleObject(processInfo.hProcess, 15000))
			{
				DWORD dwReadLen = 0;
				DWORD dwStdLen = 0;
				if (PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwReadLen, NULL) && dwReadLen > 0)
				{
					char szPipeOut[MY_PIPE_BUFFER_SIZE];
					::ZeroMemory(szPipeOut, sizeof(szPipeOut),MY_PIPE_BUFFER_SIZE);
					if (ReadFile(hPipeRead, szPipeOut, dwReadLen, &dwStdLen, NULL))
					{
						szPipeOut[MY_PIPE_BUFFER_SIZE - 1] = 0;
						std::string errtxt = "invocation forwarded to primary instance";
						std::string szPipeOutStr = szPipeOut;
						printf("%s:%d\n", errtxt.c_str(),errtxt.size());
						printf("%s:%d\n", szPipeOutStr.c_str(),szPipeOutStr.size());
						if (szPipeOutStr.find(errtxt)!=std::string::npos) {
							ret = false;
							printf("相等?\n");
						}
						else
						{
							printf("不相等?\n");
						}
					}
				}
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
	printf("多开工具v0.02\n");
	CMyINI* p = new CMyINI();
	if (p->ReadINI("Setting.ini")==0) {
		//-profile mobileDevice
		p->SetValue("config", "width", "1024");
		p->SetValue("config", "height", "768");
		p->SetValue("config", "profile1", "mobileDevice");
		p->SetValue("config", "profile2", "desktop");
		p->WriteINI("Setting.ini");
	}
	std::string w= p->GetValue("config","width");
	std::string h= p->GetValue("config","height");
	std::string pf= p->GetValue("config","profile");
	if (pf.size()==0) {
		pf = "mobileDevice";
	}
	std::string wh = w + "x" + h + ":" + h + "x" + w;

	struct _finddata_t fileinfo;                          //文件信息的结构体
	
	const char* to_search = "*.xml";
	long handle;                                                //用于查找的句柄
	handle = _findfirst(to_search, &fileinfo);         //第一次查找
	std::string fileinfoname = fileinfo.name;
	if (-1 == handle) {//没找到，找swf
		const char* to_searchswf = "*.swf";
		long handleswf;                                                //用于查找的句柄
		struct _finddata_t fileinfoswf;                          //文件信息的结构体
		handleswf = _findfirst(to_searchswf, &fileinfoswf);         //第一次查找
		if (-1 != handleswf) {//swf找到了
			const char* xmltext = R"V0G0N(
            <application xmlns="http://ns.adobe.com/air/application/33.1">
			<id>{name}</id>
			<filename>{name}</filename>
			<name>{name}</name>
			<versionNumber>1.0</versionNumber>
			<initialWindow>
			<title>{name}</title>
			<content>{name}</content>
			<visible>true</visible>
			<fullScreen>true</fullScreen>
			<resizable>false</resizable>
			<aspectRatio>landscape</aspectRatio>
			<renderMode>direct</renderMode>
			<systemChrome>standard</systemChrome>
			<depthAndStencil>true</depthAndStencil>
			<autoOrients>true</autoOrients>
			</initialWindow>
			<supportedProfiles>mobileDevice</supportedProfiles>
		</application>
		)V0G0N";
			const std::regex pattern("\\{name\\}");
			std::string replaceswf = fileinfoswf.name; //$2表示匹配模式串的第二个字串，即以a,e,i,o,u开头的单词
			std::string xmltextstr = std::regex_replace(xmltext, pattern, replaceswf);
			printf("%s\n", xmltextstr.c_str());
			_findclose(handleswf);
			//写文件
			fileinfoname = "app.xml";
			std::ofstream outfilexml(fileinfoname, std::ios::out);
			outfilexml << xmltextstr;
			outfilexml << std::endl;
			outfilexml.close();
		}
		else {
			return -1;
		}
	}
	else
	{
		fileinfoname =fileinfo.name;
	}

	printf("%s\n", fileinfoname.c_str());
	_findclose(handle);

	int md= _mkdir("xml");
	int i = 0;
	while (true) {
		std::ifstream myfile(fileinfoname);
		std::string fileinfonamef = "xml\\" + fileinfoname;
		std::string outfilename = fileinfonamef.replace(fileinfonamef.end()-4, fileinfonamef.end(),std::to_string(i));
		printf("%s\n", outfilename.c_str());
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
		//printf("%s\n", pf.c_str());
		//pf = "mobileDevice";
		std::string cmd = "AIRSDK\\bin\\adl64 -screensize "+wh+" " + outfilename + " .";
		//if (pf.compare("mobileDevice")!=0) {
		//	cmd = "AIRSDK\\bin\\adl64 -profile " + pf + " " + outfilename + " .";
		//}
		printf("%s\n",cmd.c_str());
		if (StartProcess(TEXT("AIRSDK\\bin\\adl64.exe"), StringToWchar(cmd))) {
			break;
		}
	}
	return 0;
}
