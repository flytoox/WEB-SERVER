NAME = webserv

SRCS = main.cpp configFile.cpp Server.cpp multiplexing.cpp Request.cpp\
	parseServer1Block.cpp parseServer2Block.cpp parseServer3Block.cpp\
	parseRequestHeader.cpp parseRequestBody.cpp validateRequest.cpp\
	checkHttpMethod.cpp getMethod.cpp postMethod.cpp deleteMethod.cpp config.cpp\
	cgiHandling.cpp\

CPP = c++

HEADERS = webserve.hpp configFile.hpp Server.hpp Request.hpp multiplexing.hpp
CPPFLAGS = -Wall -Wextra -Werror -g -std=c++11

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
