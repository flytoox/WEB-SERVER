#include "webserve.hpp"

//TODO: Change the response data structure is a vector container

// HTTP/1.1 200 OK
// Content-Type: text/plain
// Transfer-Encoding: chunked

// 7\r\n
// Mozilla\r\n
// 11\r\n
// Developer Network\r\n
// 0\r\n


//7\r\nMozilla\r\n11\r\nDeveloper Network\r\n
std::vector<std::string> splitString(const std::string& input, const std::string& delimiter) {
    std::vector<std::string> result;
    std::size_t start = 0;
    std::size_t end = input.find(delimiter);

    while (end != std::string::npos) {
        result.push_back(input.substr(start, end - start));
        start = end + delimiter.length();
        end = input.find(delimiter, start);
    }

    result.push_back(input.substr(start));

    return result;
}

static int hexaToDec(std::string &res) {

    std::stringstream ss(res);
    int decimal;

    ss >> std::hex >> decimal;
    return (decimal);
}

static void chunkedRequest(Request &request) {


    std::string requestBody = request.getRequestBody();

    //std::cout << requestBody;
    
    std::vector<std::string> res = splitString(requestBody, "\\r\\n\r\n");
    std::vector<std::string> output; unsigned long preSize ;

    // for (auto i : res) {
    //     std::cout << "|" << i << "|" << std::endl;
    // }
    

    //* Original
    for (size_t i = 0; i < res.size() - 1; i++) {

        preSize = hexaToDec(res[i]);

        if (res[i + 1].length() == preSize) {

            // output.push_back(res[i]);
            output.push_back(res[++i]);

        } else if ( preSize != 0) {
            std::cout << "501 Bad Gateway" << std::endl;
            output.clear(); break ;
        }
    }
    // for (auto i : output) {
    //     std::cout << "|" << i << "|";
    // }

    request.setChunkedResponse(output);
}



static void textContentType(Request &request) {

    std::string ret = request.getRequestBody();

    request.setTextResponse(ret);
}

static void multipartContentType(Request &request) {

    //std::string response = request.getRequestBody();

    std::string boundary = request.getBoundary();

    std::vector<std::string> split = splitString(request.getRequestBody(), boundary);

    split.erase(split.begin());  split.erase(split.end() - 1);

    request.setMultipartResponse(split);
    // for (auto i : split) {
    //     std::cout << "\n--------------             START  -----------------------------\n";
    //     std::cout  << i ;
    //     std::cout << "\n--------------             END  -----------------------------\n";
    // }
    // exit (0);

}

static void urlencodedContentType(Request &request) {

    std::string res = request.getRequestBody();
    std::map<std::string, std::string> mapTopush;

    size_t dividerPos = res.find('&');

    std::string firstKeyValue = res.substr(0, dividerPos);
    size_t equalSignPos = firstKeyValue.find('=');
    pair firstPair = std::make_pair(firstKeyValue.substr(0, equalSignPos), firstKeyValue.substr(equalSignPos + 1));

    std::string secondKeyValue = res.substr(dividerPos + 1, res.length() - 1);
    equalSignPos = secondKeyValue.find('=');
    pair secondPair = std::make_pair(secondKeyValue.substr(0, equalSignPos), secondKeyValue.substr(equalSignPos + 1));
    
    mapTopush.insert(firstPair); mapTopush.insert(secondPair);
    request.setUrlencodedResponse(mapTopush);

}

void parseRequestBody(Request &request) {


    //TODO : Check Transfer-Encoding && multipart

    std::map<std::string, std::string>::const_iterator itTransferEncoding;
    std::map<std::string, std::string>::const_iterator itContentType;
    itTransferEncoding = (request.getHttpRequestHeaders()).find("Transfer-Encoding:");

    if ( itTransferEncoding != (request.getHttpRequestHeaders()).end() ) {
        if (itTransferEncoding->second != "chunked") {
            std::string res = "502 Bad Gateway" ; 
            request.setrequestOutputTest(res);
            throw "502";
        }
        chunkedRequest(request);
    }

    itContentType = (request.getHttpRequestHeaders()).find("Content-Type:");
    if ( itContentType != (request.getHttpRequestHeaders()).end()) {

        std::string value = itContentType->second;

        if (value == "text/plain") {
            textContentType(request);
        } else if ((itContentType->second).find("multipart/form-data") != std::string::npos ) {
            multipartContentType(request);
        } else if (value == "application/x-www-form-urlencoded") {
            urlencodedContentType(request);
        } else {
            throw "502 Content-type";
        }
    }

}
