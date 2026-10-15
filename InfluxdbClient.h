#pragma once
#include <string>

class InfluxdbClient
{
public:
	InfluxdbClient(void);
	~InfluxdbClient(void);

	int CreateDB(const std::string & strUrl, const std::string & strDBName, std::string & strResponse);
	int Write(const std::string & strUrl, const std::string & strDBName,const std::string & strData, std::string & strResponse);
	int query(const std::string & strUrl, const std::string & strDBName,const std::string & strQuery, std::string & strResponse,int iTimeOut);
	int DropDB(const std::string & strUrl, const std::string & strDBName, std::string & strResponse);
	int Write(const std::string & strUrl, const std::string & strDBName,const char* szData, std::string & strResponse);
	int aggregation(const std::string & strUrl, const std::string & strDBName,const std::string & strQuery, std::string & strResponse,int iTimeOut);
public:
	void SetDebug(bool bDebug);

private:
	bool m_bDebug;
};

