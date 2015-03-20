// Http.cpp : Defines the entry point for the console application.
//


#include "http_request.h";

#include <iostream>

using namespace std;


int main(int argc, char* argv[])
{
	string url = "http://azenv.net/gdfgdfg";

	http_response res;

	if (http_request_get(url, &res)){
		cout << "request success!" << endl;
	}
	else {
		cout << "request failed!" << endl;
	}

	cout << res.content << endl;


	return 0;
}