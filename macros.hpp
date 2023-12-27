#pragma once

#define HTTP_VERSION                    "HTTP/1.1 "

#define CRLF                            "\r\n"
#define LOCATION                        "Location: "
#define CONTENT_TYPE                    "Content-Type: "
#define CONTENT_LENGTH                  "Content-Length: "

#define RESPONSE_OK                      "200 OK"
#define RESPONSE_CREATED                 "201 Created"
#define RESPONSE_NO_CONTENT              "204 No Content"
#define RESPONSE_MOVED_PERMANETLY        "301 Moved Permanently"
#define RESPONSE_BAD_REQUEST             "400 Bad Request"
#define RESPONSE_FORBIDDEN               "403 Forbidden"
#define RESPONSE_NOT_FOUND               "404 Not Found"
#define RESPONSE_METHOD_NOT_ALLOWED      "405 Method Not Allowed"
#define RESPONSE_CONFLICT                "409 Conflict"
#define RESPONSE_REQUEST_TOO_LARGE       "413 Payload Too Large"
#define RESPONSE_URI_TOO_LONG            "414 URI Too Long"
#define RESPONSE_INTERNAL_SERVER_ERROR   "500 Internal Server Error"
#define RESPONSE_NOT_IMPLEMENTED         "501 Not Implemented"