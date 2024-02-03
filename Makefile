NAME = webserv

SRCS = main.cpp configFile.cpp Server.cpp multiplexing.cpp Request.cpp\
	parseRequestHeader.cpp parseRequestBody.cpp validateRequest.cpp\
	checkHttpMethod.cpp getMethod.cpp postMethod.cpp deleteMethod.cpp config.cpp\
	responseBuilder.cpp cgi.cpp cgi_get.cpp \

CPP = c++

HEADERS = webserve.hpp configFile.hpp Server.hpp Request.hpp multiplexing.hpp macros.hpp responseBuilder.hpp
CPPFLAGS = -Wall -Wextra -Werror -g -std=c++11 #-fsanitize=address

OBJS = $(SRCS:.cpp=.o)

all : $(NAME)

$(NAME) : $(OBJS)
	$(CPP) $(CPPFLAGS) $^ -o $@

%.o : %.cpp $(HEADERS)
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean :
	rm -rf $(OBJS)

fclean : clean
	rm -rf $(NAME)

re : fclean all
