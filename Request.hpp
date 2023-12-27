#pragma once

#include "configFile.hpp"
#include "Server.hpp"
#include <sstream>

typedef std::pair<std::string, std::string> pair;

class Request {

private:

    
    std::map<std::string, std::string> directives;
    std::map<std::string, std::string> locationBlockWillBeUsed;

    std::map<std::string, std::string> cgiDirectives;

    std::vector<std::map<std::string, std::string> > locationsBlock;

    std::string httpVerb;
    std::string uri;
    std::string httpVersion;
    std::string boundary;

    std::map<std::string, std::string> httpRequestHeaders;

    std::string requestHeader;
    std::string requestBody;

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


public:

    Request();

    responseBuilder response;
    std::string extension;

    std::string RePort;
    std::string ReHost;

    bool dup = false;
	std::string result;
    //* GETTERS

    const std::map<std::string, std::string> &getDirectives() const;
    const std::vector<std::map<std::string, std::string> > &getLocationsBlock() const;

    const std::string &getHttpVerb(void) const;
    const std::string &getUri(void) const;
    const std::string &getHTTPVersion(void) const;
    const std::string &getBoundary(void) const;



    // const std::string &getHost(void) const;
    // const int         &getPort(void) const;
    // const std::string &getContentType(void) const;
    // const int         &getContentLength(void) const;
    // const std::string &getTransferEncoding(void) const;
    // const std::string &getConnection(void) const;



    // void getRequestHeader(std::map<std::string, std::string> &other);
    // void getRequestBody(std::vector<std::map<std::string, std::string> > &other);


    const std::string &getRequestHeader() const ;
    const std::string &getRequestBody() const ;

    const std::map<std::string, std::string> &getHttpRequestHeaders() const;
    const std::map<std::string, std::string> &getLocationBlockWillBeUsed() const ;

    // bool getAllowRequestBodyChunk(void) const;
    bool getRequestBodyChunk(void) const;
   //  bool getResponseChunk(void) const;

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

    //void setJoiningTheWholeRequest(std::string &request);
    void setRequestHeader(std::string &setter);
    void setRequestBody(std::string &setter);


    void setHttpRequestHeaders(pair &setPair);    

    void setHttpVerb(std::string &setter);
    void setUri(std::string &setter);
    void setHTTPVersion(std::string &setter);
    void setBoundary(std::string &setter);
    // void setHost(std::string &setter);
    // void setPort(int &setter);
    // void setContentType(std::string &setter);
    // void setContentLength(int &setter);
    // void setTransferEncoding(std::string &setter);
    // void setConnection(std::string &setter);


    void setDirectives(std::map<std::string, std::string> &other);
    void setLocationsBlock(std::vector<std::map<std::string, std::string> > &other);

    void setLocationBlockWillBeUsed(std::map<std::string, std::string> &other);

    // void setAllowRequestBodyChunk(bool chunk) ;
    void setRequestBodyChunk(bool chunk) ;
    // void setResponseChunk(bool chunk) ;

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


// for (auto it = request.getHttpRequestHeaders().begin(); it != request.getHttpRequestHeaders().end(); it++) {
//     std::cout << it->first << " - " << it->second << std::endl;
// }
