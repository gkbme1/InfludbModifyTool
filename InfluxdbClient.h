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
	// 写入数据并返回 HTTP 状态码（InfluxDB 写入成功返回 204）
	int WriteEx(const std::string & strUrl, const std::string & strDBName,const std::string & strData, std::string & strResponse,int iTimeOut,long* pHttpCode);
	int aggregation(const std::string & strUrl, const std::string & strDBName,const std::string & strQuery, std::string & strResponse,int iTimeOut);
public:
	void SetDebug(bool bDebug);

private:
	bool m_bDebug;
};

