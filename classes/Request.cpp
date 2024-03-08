#include "../includes/webserve.hpp"
#include <fstream>

Request::Request() :
    httpVerb (""),
    uri(""),
    httpVersion(""),
    boundary(""),
    requestHeader(""),
    requestBody(""),
    requestBodyChunk(false),
    requestOutputTest("20000000000000000000"),
    saveLastBS(false),
    timeout(time(0)),
    realContentLength(0),
    reachedBodyLength(0),
    dup(false),
    reCheck(false)
    {}

//* GETTERS


const std::map<std::string, std::string> &Request::getDirectives() const {
    return (this->directives);
}
const std::map<std::string, std::map<int, std::string> > &Request::getPages() const {
    return (this->pages);
}
const std::vector<std::map<std::string, std::string> > &Request::getLocationsBlock() const {
    return (this->locationsBlock);
}

const std::string &Request::getHttpVerb(void) const {
    return ( this->httpVerb );
}

const std::string &Request::getUri(void) const {
    return ( this->uri );
}

const std::string &Request::getHTTPVersion(void) const {
    return (this->httpVersion );
}

// std::map<std::string, std::string> &Request::getRequestHeader() const {
//     return (this->requestHeader);
// }
// std::vector<std::map<std::string, std::string> > &Request::getRequestBody() const {
//     return (this->requestBody);
// }

const std::string &Request::getRequestHeader() const  {
    return (this->requestHeader);
}

const std::string &Request::getRequestBody() const  {
    return (this->requestBody);
}


const std::map<std::string, std::string> &Request::getHttpRequestHeaders() const {
    return (this->httpRequestHeaders);
}

const std::map<std::string, std::string> &Request::getLocationBlockWillBeUsed() const {
    return (this->locationBlockWillBeUsed);
}

// bool Request::getAllowRequestBodyChunk(void) const {
//     return (this->allowRequestBodyChunk);
// }

bool Request::getRequestBodyChunk(void) {
    return (this->requestBodyChunk);
}

const std::string &Request::getBoundary(void) const {
    return (this->boundary);
}

const std::vector<std::string>& Request::getChunkedResponse() const {
    return (this->chunkedResponse);
}

const std::string& Request::getTextResponse() const {
    return (this->textResponse);
}

const std::vector<std::string>& Request::getMultipartReponse() const  {
    return (this->multipartReponse);
}

const std::map<std::string, std::string> &Request::getUrlencodedResponse() const {
    return (this->urlencodedResponse);
}


bool Request::getSaveLastBS(void) const {
    return (this->saveLastBS);
}

const std::vector<std::string>& Request::getResponseVector() const {
    return (this->responseVector);
}


const std::string& Request::getRoot() const {
    return (this->root);
}


const std::map<std::string, std::string> &Request::getCgiDirectives() const  {
    return (this->cgiDirectives);
}

const std::string &Request::getQueryString() const {
    return (this->queryString);
}
//* SETTERS


// void Request::setJoiningTheWholeRequest(std::string &request) {
//     this->httpRequest += request;
// }
void Request::setTimeout() {
    this->timeout = time(0);
}

time_t Request::getTimeout() const {
    return this->timeout;
}
void Request::setBoundary(std::string &setter) {
    this->boundary = setter;
}

void Request::setRequestBody(std::string setter) {
    for (size_t i = 0; i != static_cast<size_t>(setter.length()); i++)
        this->requestBody.push_back(setter[i]);
    //this->requestBody.append(setter);

    //this->requestBody += setter;
    // std::cerr << this->requestBody.length() << std::endl;
}

void Request::setRequestHeader(std::string setter) {
    this->requestHeader += setter;
}


void Request::setDirectivesAndPages(std::map<std::string, std::string> directives, std::map<std::string, std::map<int, std::string> > pages) {
    this->directives = directives;
    this->pages = pages;
}

void Request::setLocationsBlock(std::vector<std::map<std::string, std::string> > other) {
    this->locationsBlock = other;
}


void Request::setHttpVerb(std::string &setter) {
    this->httpVerb = setter;
}

void Request::setUri(std::string &setter) {
    this->uri = setter;
}

void Request::setHTTPVersion(std::string &setter) {
    this->httpVersion = setter;
}


void Request::setLocationBlockWillBeUsed(std::map<std::string, std::string> &other) {
    this->locationBlockWillBeUsed = other;
}


// void Request::setAllowRequestBodyChunk(bool chunk) {
//     this->allowRequestBodyChunk = chunk;
// }

void Request::setRequestBodyChunk(bool chunk) {
    this->requestBodyChunk = chunk;
}

void Request::setHttpRequestHeaders(pair &setPair) {

    httpRequestHeaders[setPair.first] = setPair.second;
}

void Request::setChunkedResponse(std::vector<std::string> &setter) {
    this->chunkedResponse = setter;
}

void Request::setTextResponse(std::string &setter) {
    this->textResponse = setter;
}

void Request::setMultipartResponse(std::vector<std::string> &setter) {
    this->multipartReponse = setter;
}

void Request::setUrlencodedResponse(std::map<std::string, std::string> &setter) {
    this->urlencodedResponse = setter;
}


void Request::setSaveLastBS(bool chunk) {
    this->saveLastBS = chunk;
}

void Request::setResponseVector(std::string &setter) {
    responseVector.push_back(setter);
}

void Request::setRoot(std::string &setter) {
    this->root = setter;
}

void Request::setCgiDirectives(std::map<std::string, std::string> &setter) {
    this->cgiDirectives = setter;
}

void Request::setQueryString(std::string &setter) {
    this->queryString = setter;
}

Request::~Request() {}



//test
const std::string Request::getPageStatus(int status) const {
    if (!getLocationBlockWillBeUsed().count("location")) {
        return (getPages().at(".").at(status));
    }
    std::string LocationName = getLocationBlockWillBeUsed().at("location");
    std::string page = getPages().at(LocationName).at(status);
    return (page);
}

void Request::setrequestOutputTest(std::string &setter) {
    this->requestOutputTest = setter;
}


const std::string& Request::getrequestOutputTest(void) const {
    return ( this->requestOutputTest);
}
