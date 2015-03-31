

#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <string>

#include <iostream>

#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <wininet.h>
#pragma comment(lib, "WinInet.lib")


#ifdef _WIN32 || _WIN64

using namespace std;

struct http_response {

	int error_code = 0;

	std::string headers;
	std::string body;
};

struct Url {

	std::string scheme, host, path;
	int port;
};



inline bool parse_url(const std::string &url, struct Url *url_ptr);

inline bool http_request_get(const string &_url, struct http_response* response){

	Url url;

	if (!parse_url(_url, &url)){
		return false;
	}

	HINTERNET hIntSession = NULL;
	HINTERNET hHttpSession = NULL;
	HINTERNET hHttpRequest = NULL;

	TCHAR* szHeaders = _T("User-Agent: Mozilla/5.0 (Windows NT 6.3; WOW64; rv:36.0) Gecko/20100101 Firefox/36.0\r\nAccept-Language: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");

	// agent, access_type, proxy_name, proxy_flags, flags
	hIntSession = ::InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

	if (hIntSession){

		LPCWSTR host = wstring(url.host.begin(), url.host.end()).c_str();

		int port = url.scheme == "https" ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

		// hInternet, server_name, port, username, password, service, flags, context
		hHttpSession = InternetConnect(hIntSession, host, port, 0, 0, INTERNET_SERVICE_HTTP, 0, NULL);

		if (hHttpSession){

			DWORD dwTimeOut = 10 * 1000; // In milliseconds

			InternetSetOption(hHttpSession, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeOut, sizeof(dwTimeOut));
			InternetSetOption(hHttpSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &dwTimeOut, sizeof(dwTimeOut));
			InternetSetOption(hHttpSession, INTERNET_OPTION_SEND_TIMEOUT, &dwTimeOut, sizeof(dwTimeOut));

			//LPCWSTR path = wstring(url.path.begin(), url.path.end()).c_str();

			int flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES;

			if (url.scheme == "https"){
				flags |= INTERNET_FLAG_SECURE;
			}

			wstring path = wstring(url.path.begin(), url.path.end());

			// hInternet, http_verb, http_target, http_version, referer, accept_types, flags, context
			hHttpRequest = HttpOpenRequest(hHttpSession, _T("GET"), path.c_str(), 0, 0, 0, flags, 0);

			if (hHttpRequest){

				CHAR data[4096];

				// hInternet, headers, headers_length, optional data after headers - post/put, optional_length
				if (HttpSendRequest(hHttpRequest, szHeaders, _tcslen(szHeaders), data, strlen(data))) {

					CHAR buffer[65536];
					DWORD length = 65536;

					/// fuck why has no one told me to put this AFTER httpSendRequest... wasted whole day
					if (HttpQueryInfoA(hHttpRequest, HTTP_QUERY_RAW_HEADERS_CRLF, buffer, &length, NULL)){

						// probably need this
						buffer[length] = '\0';

						response->headers = buffer;
					}

					std::string body;
					const int size = 1024;

					CHAR szBuffer[size];
					DWORD dwRead = 0;

					while (InternetReadFile(hHttpRequest, (LPVOID)szBuffer, size - 1, &dwRead) && dwRead) {

						szBuffer[dwRead] = '\0';

						body.append(szBuffer, dwRead);

						//printf("bytes read: %d, body size: %d\r\n", dwRead, content.length());

						dwRead = 0;
					}

					response->body = body;
				}
			}
		}
	}

	// error has happened?
	response->error_code = GetLastError();

	::InternetCloseHandle(hHttpRequest);
	::InternetCloseHandle(hHttpSession);
	::InternetCloseHandle(hIntSession);

	return !response->error_code;
}



#endif


inline bool parse_url(const std::string &url, struct Url *url_ptr){

	int temp;
	int cursor = 0;

	if ((temp = url.find("://")) > -1){

		// set protocol
		url_ptr->scheme = url.substr(0, temp);

		cursor = temp + 3;
	}

	int port = url.find(":", cursor);
	int path = url.find("/", cursor);

	if (port > -1){

		url_ptr->host = url.substr(cursor, port - cursor);

		string pp;

		if (path > -1){
			// stop at slash
			pp = url.substr(port + 1, path - (port + 1));
		}
		else {
			// read until the end
			pp = url.substr(port + 1);
		}

		//this->port = atoi(pp.c_str());

		url_ptr->port = sprintf("%d", pp.c_str());

	}

	// port not found - we're still trying to extract hostname
	if (url_ptr->host.empty()){
		url_ptr->host = url.substr(cursor, path - cursor);
	}

	// path must exist then
	if (path > -1){

		int hash = url.find("#", cursor);

		url_ptr->path = url.substr(path);
	}

	return true;
}


#endif