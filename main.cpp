#include<iostream>
#include <string.h>
#include <windows.h>
#include <winhttp.h>
#include <fstream>
#pragma comment(lib, "winhttp.lib")
using namespace std;

int main(){
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;

    hSession = WinHttpOpen(
        L"WinHTTP",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,
        0
    );

	if (!hSession){
        cout << "Error: Open session failed.";
        return -1;
    }

    LPCWSTR web = L"www.zhihu.com"; //输入需要爬取的网址
    hConnect = WinHttpConnect(
        hSession,
        web,
        INTERNET_DEFAULT_HTTPS_PORT,
        0
    );
    if (!hConnect){
        cout << "Error: Fail to connect.";
        return -1;
    }

    hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        NULL,
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE
    );

    if (!hRequest){
        cout << "Error: Request failed" << endl;
        cout << GetLastError() << endl;
        return -1;
    }

    string data = "Test";
    const void *ss = (const char *)data.c_str();

    BOOL bResults = 0;
    bResults = WinHttpSendRequest(hRequest,WINHTTP_NO_ADDITIONAL_HEADERS, 0, const_cast<void*>(ss), data.length(), data.length(), 0);

    if (!bResults){
		cout << "Error:SendRequest failed: " << GetLastError() << endl;
		return -1;
	}
	else{
		bResults = WinHttpReceiveResponse(hRequest, NULL);
	}

    LPVOID lpHeaderBuffer = NULL;
	DWORD dwSize = 0;
	if (bResults){
		//(1) 获取header的长度
		WinHttpQueryHeaders(hRequest,WINHTTP_QUERY_RAW_HEADERS_CRLF,WINHTTP_HEADER_NAME_BY_INDEX,NULL,&dwSize,WINHTTP_NO_HEADER_INDEX);

		//(2) 根据header的长度为buffer申请内存空间
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER){
			lpHeaderBuffer = new WCHAR[dwSize / sizeof(WCHAR)];

			//(3) 使用WinHttpQueryHeaders获取header信息
			bResults = WinHttpQueryHeaders(hRequest,WINHTTP_QUERY_RAW_HEADERS_CRLF,WINHTTP_HEADER_NAME_BY_INDEX,lpHeaderBuffer, &dwSize,WINHTTP_NO_HEADER_INDEX);
		}
	}
	printf("Header contents: \n%S", lpHeaderBuffer);

    PSTR pszOutBuffer = NULL;
	DWORD dwDownloaded = 0;//实际收取的字符数
	if (bResults){
		do{
			//(1)获取返回数据的大小（以字节为单位）
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize)){
				cout << "Error：WinHttpQueryDataAvailable failed：" << GetLastError() << endl;
				break;
			}
			if (!dwSize)
				break;//数据大小为0时跳出循环

			//(2)根据返回数据的长度为buffer来申请内存空间
			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer){
				cout<<"Out of memory."<<endl;
				dwSize = 0;
				break;
			}
			else{
				//(3)通过WinHttpReadData读取服务器的返回数据
				ZeroMemory(pszOutBuffer, dwSize + 1);//将buffer置0
				if (!WinHttpReadData(hRequest,pszOutBuffer, dwSize, &dwDownloaded)){
					cout << "Error：WinHttpQueryDataAvailable failed：" << GetLastError() << endl;
				}
				else
					cout << pszOutBuffer;

				//4.4. 将返回数据转换成UTF8
				wchar_t *pwText = NULL;
				DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, pszOutBuffer, -1, NULL, 0);//返回原始ASCII码的字符数目
				pwText = new wchar_t[dwNum];//根据ASCII码的字符数来分配UTF8的空间
				MultiByteToWideChar(CP_UTF8, 0, pszOutBuffer, -1, pwText, dwNum);//将ASCII码转换成UTF8
				printf("Received contents: \n%S", pwText);

				delete[] pszOutBuffer;
			}
		}while (dwSize > 0);
	}

	//5.依次关闭request，connect，session句柄
	if(hRequest)
		WinHttpCloseHandle(hRequest);
	if(hConnect)
		WinHttpCloseHandle(hConnect);
	if(hSession)
		WinHttpCloseHandle(hSession);

	return 0;

}