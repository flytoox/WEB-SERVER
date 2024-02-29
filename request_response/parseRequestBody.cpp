#include "../includes/webserve.hpp"

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


static std::vector<std::string> splitChunkedBody(const std::string& input, const std::string& delimiter) {
    std::vector<std::string> result;

    size_t start = 0;
    size_t pos = input.find(delimiter);

    while (pos != std::string::npos) {
        result.push_back(input.substr(start, pos - start));
        start = pos + delimiter.length();
        pos = input.find(delimiter, start);
    }

    result.push_back(input.substr(start));

    return result;
}

static bool characterNotAllowed(std::string &hexadecimal) {

    std::string allowedCharacters = "ABCDEFabcdef0123456789";

    for (size_t i = 0; i < hexadecimal.length(); i++) {
        if (allowedCharacters.find(hexadecimal[i]) == std::string::npos )
            return true;
    }
    return false;
}

int hexaToDec(Request &request, std::string &res) {

    if ( characterNotAllowed(res) ) {
        request.response = responseBuilder()
            .addStatusLine("400")
            .addContentType("text/html")
            .addResponseBody("<html><body><h1>400 Bad Request</h1></body></html>");
            throw "400 CHUNKED";
    }

    std::stringstream ss(res);
    int decimal;

    ss >> std::hex >> decimal;
    return (decimal);
}

void chunkedRequest(Request &request) {


    std::string requestBody = request.getRequestBody();

    //std::cout << requestBody;
    
    std::vector<std::string> res = splitChunkedBody(requestBody, "\r\n");
    std::vector<std::string> output; unsigned long preSize ;

    //* Original
    for (size_t i = 0; i < res.size() - 1; i++) {

        preSize = hexaToDec(request, res[i]);

        //std::cout << "WHAT |" << res[i] << "|\n";
        if (!preSize && i == res.size() - 1)
            break ;
        if (i + 1 <= res.size() && res[i + 1].length() == preSize) {
            // output.push_back(res[i]);
            output.push_back(res[++i]);

        } else {
            request.response = responseBuilder()
                .addStatusLine("400")
                .addContentType("text/html")
                .addResponseBody("<html><body><h1>400 Bad Request</h1></body></html>");
                throw "400 CHUNKED";
        }
    }


    std::string ret;
    for (auto i : output) {
        ret += i;
    }

    request.setRequestBody(ret);

    // request.setChunkedResponse(output);
}



void textContentType(Request &request) {

    std::string ret = request.getRequestBody();

    request.response = responseBuilder()
    .addResponseBody(ret);

}

void pureBinary(Request &request, std::string &image, std::string &destination) {


    // std::cout << "ERROR|" << image <<"|\n";

    std::size_t pos = image.find("filename=\"");

//*PROTECT pos

    image.erase(0, pos + 10);
    std::string filename = image.substr(0, image.find("\""));
    pos = image.find("Content-Type: ");
    image.erase(0, pos + 14);
    pos = image.find("\r\n\r\n");
    image.erase(0, pos);
    image.erase(image.end() - 2, image.end());


    // std::string type = image.substr('/');
    // std::cout << "|" << type << "|\n"; exit (0);


    std::string absolutePath = destination + '/' + filename ; 

    std::ofstream outputFile(absolutePath);

    std::cout << "FILENAME|" << filename << "|\n";
    //std::cerr << request.getRequestBody() << "|\n";

    if (!outputFile.is_open()) {
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody("<html><body><h1>400 Bad Request1</h1></body></html>");
        throw "403";
    }

    outputFile << image.substr(image.find("\r\n\r\n") + 4);

    if (outputFile.fail()) {
        request.response = responseBuilder()
        .addStatusLine("400")
        .addContentType("text/html")
        .addResponseBody("<html><body><h1>400 Bad Request2</h1></body></html>");
        throw "403";
    }

    outputFile.close();
    return ;


}

void multipartContentType(Request &request) {

    //std::string response = request.getRequestBody();

    std::string boundary = request.getBoundary();

    // std::cout << "REQUEST|" << request.getRequestBody() << "|\n";

    // std::cerr << request.getRequestBody() << std::endl;
    std::vector<std::string> split = splitString(request.getRequestBody(), boundary);

    if (split.size() > 2) {
        split.erase(split.begin());
        split.erase(split.end() - 1);
    }
    else {
        // std::cout << "PROBLEEEEEEEMMMMMMM |" << request.getRequestHeader() << std::endl;
        // std::cout << "PROBLEEEEEEEMMMMMMM |" << request.getRequestBody().size() << std::endl;
        std::cerr << "ERROR: what are u doing kid!!" << std::endl;
    }
    // for (auto it : split) {
    //     std::cout << "|" << it << "|\n";
    //     std::cout << "**********************\n";
    // }


    std::map<std::string, std::string> locations = request.getLocationBlockWillBeUsed();
    //TODO: get back the if else statement -> [upload_enable"] == "on")
    // if (locations["upload_enable"] == "on") {
        std::string destination = locations["upload_store"];
        for (size_t i = 0; i < split.size(); i++) {
            pureBinary(request, split[i], destination);
        }
    // } 
    // else if (locations["upload_enable"] == "off") {
    //     request.response = responseBuilder()
    //     .addStatusLine("403")
    //     .addContentType("text/html")
    //     .addResponseBody("<html><head><title>403 Forbidden</title></head><body><h1>Forbidden</h1><p>You don't have permission to access the requested resource. File uploads are not allowed.</p></body></html>");
    //     throw "403";
    // }


    // request.setMultipartResponse(split);
    // for (auto i : split) {
    //     std::cout << "\n--------------             START  -----------------------------\n";
    //     std::cerr << i << std::endl;
    //     std::cout << "\n--------------             END  -----------------------------\n";
    // }
    // exit (0);

}

// void urlencodedContentType(Request &request) {

//     std::string res = request.getRequestBody();
//     std::map<std::string, std::string> mapTopush;

//     size_t dividerPos = res.find('&');

//     std::string firstKeyValue = res.substr(0, dividerPos);
//     size_t equalSignPos = firstKeyValue.find('=');
//     pair firstPair = std::make_pair(firstKeyValue.substr(0, equalSignPos), firstKeyValue.substr(equalSignPos + 1));

//     std::string secondKeyValue = res.substr(dividerPos + 1, res.length() - 1);
//     equalSignPos = secondKeyValue.find('=');
//     pair secondPair = std::make_pair(secondKeyValue.substr(0, equalSignPos), secondKeyValue.substr(equalSignPos + 1));
    
//     mapTopush.insert(firstPair); mapTopush.insert(secondPair);
//     request.setUrlencodedResponse(mapTopush);

// }

void urlencodedContentType(Request &request) {

    std::cout << "GOTTEN WITHIN urlencodedContentType()\n";

    std::string res = request.getRequestBody();
    std::map<std::string, std::string> mapTopush;

    std::vector<std::string> keyValueVector = splitString(res, "&");

    for (const_vector_it it = keyValueVector.begin(); it != keyValueVector.end(); it++) {
        std::string keyValue = (*it);
        size_t signPos = keyValue.find('=');
        if (signPos != std::string::npos) {
            pair pair = std::make_pair(keyValue.substr(0, signPos), keyValue.substr(signPos + 1));
            mapTopush.insert(pair);
        } else {
            request.response = responseBuilder()
            .addStatusLine("400")
            .addContentType("text/html")
            .addResponseBody("<html><body><h1>400 Bad Request</h1></body></html>");
            throw "400";
        }
    }
    request.setUrlencodedResponse(mapTopush);

    // for (auto it : mapTopush) {
    //     std::cout << "URLENCODED |" << it.first << "|\t|" << it.second << "\n";
    // }


}


static int getTheMaxsize(Request &request) {

    std::map<std::string, std::string> location = request.getLocationBlockWillBeUsed();
    int size = 0;

    for ( mapConstIterator it =  location.begin() ; it != location.end() ; ++it ) {
        if (it->first == "client_max_body_size") {
            size = std::atoi((it->second).c_str());
            return (size);
        }
    }

    std::map<std::string, std::string> directives = request.getDirectives();

    for ( mapConstIterator it =  directives.begin() ; it != directives.end() ; ++it ) {
        if (it->first == "client_max_body_size") {
            size = std::atoi((it->second).c_str());
            break ;
        }
    }

    return (size);
}

void parseRequestBody(Request &request) {


    //*Check Length of the Body

    unsigned long sizeMax = getTheMaxsize(request);
    if ( sizeMax && request.getRequestBody().length() > sizeMax ) {
        
        request.response = responseBuilder()
        .addStatusLine("413")
        .addContentType("text/html")
        .addResponseBody("<html><h1> 413 Content Too Large </h1></html>");
        throw "413";

    }

    // if ( request.getRequestBody().length() > 4000000000 ) {

    //     request.response = responseBuilder()
    //     .addStatusLine("413")
    //     .addContentType("text/html")
    //     .addResponseBody("<html><h1> 413 Content Too Large </h1></html>");
    //     throw "413";

    // }

    //TODO : Check Transfer-Encoding && multipart

    std::map<std::string, std::string>::const_iterator itTransferEncoding;
    std::map<std::string, std::string>::const_iterator itContentType;
    itTransferEncoding = (request.getHttpRequestHeaders()).find("Transfer-Encoding:");

    if ( itTransferEncoding != (request.getHttpRequestHeaders()).end() ) {
        // if (itTransferEncoding->second != "chunked") {
        
        //     request.response = responseBuilder()
        //     .addStatusLine("500")
        //     .addContentType("txt");

        //     throw "500";

        // }
        std::cout << "Transfer-Encoding\n";
        chunkedRequest(request);

    }



    // itContentType = (request.getHttpRequestHeaders()).find("Content-Type:");
    // if ( itContentType != (request.getHttpRequestHeaders()).end()) {

    //     std::string value = itContentType->second;

    //     if (value == "text/plain") {

    //         request.response = responseBuilder()
    //         .addStatusLine("200")
    //         .addContentType("txt");

    //         textContentType(request);
    //         throw "200L";
    //     } else if ((itContentType->second).find("multipart/form-data") != std::string::npos ) {

    //         multipartContentType(request);
    //         request.response = responseBuilder()
    //         .addStatusLine("200")
    //         .addContentType("text/html");

    //         request.response = responseBuilder()
    //         .addResponseBody("<html><h1> Successfully Uploaded </h1></html>");
    //         throw "200L";
    //     } else if (value == "application/x-www-form-urlencoded") {
    //         std::cout << "111\n";
    //         urlencodedContentType(request);
    //         throw "200L";
    //     } else {
    //         throw "502 Content-type";
    //     }
    // }

}
