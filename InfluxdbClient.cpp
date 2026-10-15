#include "StdAfx.h"
#include "InfluxdbClient.h"
#include "curl/curl.h"
#include <string>

InfluxdbClient::InfluxdbClient(void):
	m_bDebug(false)
{
}


InfluxdbClient::~InfluxdbClient(void)
{
}

static int OnDebug(CURL *, curl_infotype itype, char * pData, size_t size, void *)
{
	if(itype == CURLINFO_TEXT)
	{
		//printf("[TEXT]%s\n", pData);
	}
	else if(itype == CURLINFO_HEADER_IN)
	{
		printf("[HEADER_IN]%s\n", pData);
	}
	else if(itype == CURLINFO_HEADER_OUT)
	{
		printf("[HEADER_OUT]%s\n", pData);
	}
	else if(itype == CURLINFO_DATA_IN)
	{
		printf("[DATA_IN]%s\n", pData);
	}
	else if(itype == CURLINFO_DATA_OUT)
	{
		printf("[DATA_OUT]%s\n", pData);
	}
	return 0;
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
	if( NULL == str || NULL == buffer )
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
}

int InfluxdbClient::CreateDB(const std::string & strUrl, const std::string & strDBName, std::string & strResponse)
{
	std::string strQ = "q=CREATE+DATABASE+";
	strQ += strDBName;
	std::string strUrl1 = strUrl;
	strUrl1 += strQ;
	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}

	curl_easy_setopt(curl, CURLOPT_URL, strUrl1.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

int InfluxdbClient::Write(const std::string & strUrl, const std::string & strDBName,const std::string & strData, std::string & strResponse)
{
	std::string strUrl1 = strUrl;
	strUrl1 += "db=";
	strUrl1 += strDBName;

	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}

	curl_easy_setopt(curl, CURLOPT_URL, strUrl1.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strData.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

int InfluxdbClient::aggregation(const std::string & strUrl, const std::string & strDBName,const std::string & strQuery, std::string & strResponse,int iTimeOut)
{
	std::string strUrl1 = strUrl;
	strUrl1 += "db=";
	strUrl1 += strDBName;

	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}

	strUrl1 += "&";
	strUrl1 += "q=";
	char* szQuery = curl_easy_escape(curl,strQuery.c_str(),strQuery.size());
	strUrl1 += szQuery;
	curl_free(szQuery);


	curl_easy_setopt(curl, CURLOPT_URL, strUrl1.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, iTimeOut);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

int InfluxdbClient::query(const std::string & strUrl, const std::string & strDBName,const std::string & strQuery, std::string & strResponse,int iTimeOut)
{
	std::string strUrl1 = strUrl;
	strUrl1 += "db=";
	strUrl1 += strDBName;

	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}

	strUrl1 += "&";
	strUrl1 += "q=";
	char* szQuery = curl_easy_escape(curl,strQuery.c_str(),strQuery.size());
	strUrl1 += szQuery;
	curl_free(szQuery);


	curl_easy_setopt(curl, CURLOPT_URL, strUrl1.c_str());
	//curl_easy_setopt(curl, CURLOPT_POST, 1);

	//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, iTimeOut);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

int InfluxdbClient::DropDB(const std::string & strUrl, const std::string & strDBName, std::string & strResponse)
{
	std::string strQ = "q=DROP+DATABASE+";
	strQ += strDBName;
	std::string strUrl1 = strUrl;
	strUrl1 += strQ;
	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}

	curl_easy_setopt(curl, CURLOPT_URL, strUrl1.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

int InfluxdbClient::Write(const std::string & strUrl, const std::string & strDBName,const char* szData, std::string & strResponse)
{
	std::string strUrl1 = strUrl;
	strUrl1 += "db=";
	strUrl1 += strDBName;

	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}

	curl_easy_setopt(curl, CURLOPT_URL, strUrl1.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, szData);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}