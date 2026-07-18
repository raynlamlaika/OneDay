NAME=OneDay
FILES=main.cpp OneDay.cpp Sandbox.cpp Pro_child.cpp

OBJS=$(FILES:.cpp=.o)
CXX=c++
FLAGS=-std=c++20 -Wall -Wextra #-Werror -g
HEADER=OneDay.hpp Sandbox.hpp



all: $(NAME) 
	mkdir -p /tmp/sandbox_mount
$(NAME): $(OBJS) 
	$(CXX) $(FLAGS) -o $(NAME) $(OBJS)


%.o: %.cpp $(HEADER)
	$(CXX) $(FLAGS) -c $< -o $@

re: fclean all

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)
	rm -rf /tmp/sandbox_mount