#pragma once

#include "configFile.hpp"
#include "Server.hpp"
#include "responseBuilder.hpp"
#include <sstream>

typedef std::pair<std::string, std::string> pair;

class Request {
public:
    std::string stringUnparsed;
    
private:
    std::map<std::string, std::string> directives;
    std::map<std::string, std::string> locationBlockWillBeUsed;
    std::map<std::string, std::map<int, std::string> > pages;

    std::map<std::string, std::string> cgiDirectives;

    std::vector<std::map<std::string, std::string> > locationsBlock;

    std::string httpVerb;
    std::string uri;
    std::string httpVersion;
    std::string boundary;

    std::map<std::string, std::string> httpRequestHeaders;

    std::string requestHeader;
    std::string requestBody;

    std::string queryString;

    // bool allowRequestBodyChunk;
    bool requestBodyChunk;
    // bool responseChunk;

    //test
    std::string requestOutputTest;

    //Response

    std::vector<std::string> chunkedResponse;
    std::string textResponse;
    std::vector<std::string> multipartReponse;
    std::map<std::string, std::string> urlencodedResponse;

    bool saveLastBS;

    std::vector<std::string> responseVector;
    std::string root;
    time_t timeout;


public:

    Request();

    responseBuilder response;
    size_t realContentLength;
    size_t reachedBodyLength;
    std::string extension;

    std::string RePort;
    std::string ReHost;

    bool dup;
	std::string result;
    bool reCheck;
    //* GETTERS
    time_t getTimeout(void) const;
    const std::map<std::string, std::map<int, std::string> > &getPages() const;
    const std::map<std::string, std::string> &getDirectives() const;
    const std::vector<std::map<std::string, std::string> > &getLocationsBlock() const;

    const std::string &getHttpVerb(void) const;
    const std::string &getUri(void) const;
    const std::string &getHTTPVersion(void) const;
    const std::string &getBoundary(void) const;


    const std::string &getRequestHeader() const ;
    const std::string &getRequestBody() const ;

    std::map<std::string, std::string> &getHttpRequestHeaders();
    const std::map<std::string, std::string> &getLocationBlockWillBeUsed() const ;
    const std::string getPageStatus(int status) const ;

    bool getRequestBodyChunk(void);
   //  bool getResponseChunk(void) const;

   const std::string &getQueryString(void) const;

    //! Responses

    const std::vector<std::string> &getChunkedResponse() const ;
    const std::string &getTextResponse() const ;
    const std::vector<std::string> &getMultipartReponse() const ;
    const std::map<std::string, std::string> &getUrlencodedResponse() const ;

    bool getSaveLastBS(void) const;

    const std::vector<std::string>& getResponseVector() const ;
    const std::string& getRoot() const;
    const std::map<std::string, std::string> &getCgiDirectives() const ;


    //* SETTERS

    void setTimeout();
    void setRequestHeader(std::string &setter);
    void setRequestBody(std::string &setter);


    void setHttpRequestHeaders(pair setPair);

    void setHttpVerb(std::string &setter);
    void setUri(std::string &setter);
    void setHTTPVersion(std::string &setter);
    void setBoundary(std::string &setter);

    void setDirectivesAndPages(std::map<std::string, std::string> directives, std::map<std::string, std::map<int, std::string> > Pages);
    void setLocationsBlock(std::vector<std::map<std::string, std::string> > other);

    void setLocationBlockWillBeUsed(std::map<std::string, std::string> &other);
    void setRequestBodyChunk(bool chunk) ;

    void setQueryString(std::string &setter);

    //test


    void setrequestOutputTest(std::string &setter);
    const std::string &getrequestOutputTest(void) const;

    //! Responses

    void setChunkedResponse(std::vector<std::string> &setter);
    void setTextResponse(std::string &setter);
    void setMultipartResponse(std::vector<std::string> &setter);
    void setUrlencodedResponse(std::map<std::string, std::string> &setter);

    void setSaveLastBS(bool chunk) ;
    void setResponseVector(std::string &setter);
    void setRoot(std::string &setter);
    void setCgiDirectives(std::map<std::string, std::string> &setter);


    ~Request() ;
    std::string hey;

};


