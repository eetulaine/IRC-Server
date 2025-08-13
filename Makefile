NAME 		:= ircserv

CC			:= c++
FLAGS		:= -Wall -Wextra -Werror -std=c++11

INCL 		:= includes/
SRC_PATH 	:= sources/
OBJ_PATH 	:= objects/

SRCS		:= main.cpp \
				Server.cpp

SRCS		:= $(addprefix $(SRC_PATH), $(SRCS))
OBJS		:= $(SRCS:$(SRC_PATH)%.cpp=$(OBJ_PATH)%.o)

RM			:= rm -rf

all: $(OBJ_PATH) $(NAME)

$(OBJ_PATH):
	mkdir -p $(OBJ_PATH)

$(OBJ_PATH)%.o: $(SRC_PATH)%.cpp $(INCL)
	$(CC) $(FLAGS) -I $(INCL) -c $< -o $@

$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)

clean:
	$(RM) $(OBJ_PATH)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
