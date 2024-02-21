#include <iostream>
#include <string>

#include "jdbc/mysql_connection.h"
#include "jdbc/cppconn/driver.h"
#include "jdbc/cppconn/exception.h"
#include "jdbc/cppconn/resultset.h"
#include "jdbc/cppconn/statement.h"
#include "jdbc/cppconn/prepared_statement.h"

using namespace std;

#pragma comment(lib, "debug/mysqlcppconn")


#include <Windows.h>

#define SAFE_DELETE(p)	if(p)	{ delete (p);	(p)=0; } 

/* * @brief 웹에서 사용하는 utf-8 인코딩을 window에서 사용하는 ANSI 인코딩으로 변경 합니다. */
std::string Utf8ToMultiByte(std::string utf8_str)
{
	if (utf8_str.length() == 0)
	{
		return "";
	}
	std::string resultString; char* pszIn = new char[utf8_str.length() + 1];
	strncpy_s(pszIn, utf8_str.length() + 1, utf8_str.c_str(), utf8_str.length());
	int nLenOfUni = 0, nLenOfANSI = 0; wchar_t* uni_wchar = NULL;
	char* pszOut = NULL;

	// 1. utf8 Length
	if ((nLenOfUni = MultiByteToWideChar(CP_UTF8, 0, pszIn, (int)strlen(pszIn), NULL, 0)) <= 0)
		return nullptr;
	uni_wchar = new wchar_t[nLenOfUni + 1];
	memset(uni_wchar, 0x00, sizeof(wchar_t) * (nLenOfUni + 1));

	// 2. utf8 --> unicode
	nLenOfUni = MultiByteToWideChar(CP_UTF8, 0, pszIn, (int)strlen(pszIn), uni_wchar, nLenOfUni);

	// 3. ANSI(multibyte) Length
	if ((nLenOfANSI = WideCharToMultiByte(CP_ACP, 0, uni_wchar, nLenOfUni, NULL, 0, NULL, NULL)) <= 0)
	{
		delete[] uni_wchar; return 0;
	}
	pszOut = new char[nLenOfANSI + 1];
	memset(pszOut, 0x00, sizeof(char) * (nLenOfANSI + 1));

	// 4. unicode --> ANSI(multibyte)
	nLenOfANSI = WideCharToMultiByte(CP_ACP, 0, uni_wchar, nLenOfUni, pszOut, nLenOfANSI, NULL, NULL);
	pszOut[nLenOfANSI] = 0;
	resultString = pszOut;
	delete[] uni_wchar;
	delete[] pszOut;
	return resultString;
}

/*
* @brief window에서 사용하는 ANSI 인코딩을 웹에서 사용하는 utf-8 인코딩 인코딩으로 변경 합니다.
*/
std::string MultiByteToUtf8(std::string multibyte_str)
{
	if (multibyte_str.length() == 0)
	{
		return "";
	}

	char* pszIn = new char[multibyte_str.length() + 1];
	strncpy_s(pszIn, multibyte_str.length() + 1, multibyte_str.c_str(), multibyte_str.length());

	std::string resultString;

	int nLenOfUni = 0, nLenOfUTF = 0;
	wchar_t* uni_wchar = NULL;
	char* pszOut = NULL;

	// 1. ANSI(multibyte) Length
	if ((nLenOfUni = MultiByteToWideChar(CP_ACP, 0, pszIn, (int)strlen(pszIn), NULL, 0)) <= 0)
		return 0;

	uni_wchar = new wchar_t[nLenOfUni + 1];
	memset(uni_wchar, 0x00, sizeof(wchar_t) * (nLenOfUni + 1));

	// 2. ANSI(multibyte) ---> unicode
	nLenOfUni = MultiByteToWideChar(CP_ACP, 0, pszIn, (int)strlen(pszIn), uni_wchar, nLenOfUni);

	// 3. utf8 Length
	if ((nLenOfUTF = WideCharToMultiByte(CP_UTF8, 0, uni_wchar, nLenOfUni, NULL, 0, NULL, NULL)) <= 0)
	{
		delete[] uni_wchar;
		return 0;
	}

	pszOut = new char[nLenOfUTF + 1];
	memset(pszOut, 0, sizeof(char) * (nLenOfUTF + 1));

	// 4. unicode ---> utf8
	nLenOfUTF = WideCharToMultiByte(CP_UTF8, 0, uni_wchar, nLenOfUni, pszOut, nLenOfUTF, NULL, NULL);
	pszOut[nLenOfUTF] = 0;
	resultString = pszOut;

	delete[] uni_wchar;
	delete[] pszOut;

	return resultString;
}


int main()
{
	//workbench
	sql::Driver* driver = nullptr;
	sql::Connection* connection = nullptr; //주소
	sql::Statement* statement = nullptr; //SQL //하드코딩
	sql::ResultSet* resultSet = nullptr ; //결과
	sql::PreparedStatement* preparedStatement = nullptr; //sql + 값 추가

	try
	{
		driver = get_driver_instance();

		connection = driver->connect("tcp://127.0.0.1", "root", "1234");

		//use world
		connection->setSchema("guestbook");

		statement = connection->createStatement();

		preparedStatement = connection->prepareStatement("insert into guestbook (`writer`, `memo`) value (?, ?)");
		preparedStatement->setString(1, MultiByteToUtf8("이희주"));
		preparedStatement->setString(2, MultiByteToUtf8("언제 끝?"));
		preparedStatement->execute();

		resultSet = statement->executeQuery("select * from guestbook order by idx desc;");


		//statement->execute("insert into guestbook (`writer`, `memo`) value ('-=', 'dinner dinner chicken')");


		for (; resultSet->next();)
		{
			cout << resultSet->getInt("idx") << " : "
				<< Utf8ToMultiByte(resultSet->getString("writer")) << " : "
				<< Utf8ToMultiByte(resultSet->getString("memo")) << " : "
				<< Utf8ToMultiByte(resultSet->getString("reg_date")) << endl;
		}
	}
	catch (sql::SQLException e)
	{
		cout << e.what() << endl;
	}
	catch (exception e)
	{
		cout << e.what() << endl;
	}



	SAFE_DELETE(connection);
	SAFE_DELETE(statement);
	SAFE_DELETE(resultSet);
	SAFE_DELETE(preparedStatement);



	return 0;
}

