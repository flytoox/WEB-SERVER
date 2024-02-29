NAME = webserv

SRCS = $(wildcard *.cpp */*.cpp)

CPP = c++

HEADERS = $(wildcard *.hpp */*.hpp)
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
