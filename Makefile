
NAME = ircserv

INCLUDES = include/

CPP_FLAGS = -I$(INCLUDES) -Wall -Werror -Wextra -std=c++98 -g
CC = c++

SRC_DIR		=	srcs/
SRC_NAME	=	main.cpp Server.cpp Client.cpp Channel.cpp \
				commands/invite.cpp \
				commands/kickpart.cpp \
				commands/mode.cpp \
				commands/topic.cpp \
				commands/join.cpp \
				commands/privmsg.cpp \
				commands/ppnu.cpp \
				utils/utils.cpp \
				utils/parser.cpp \
				utils/message.cpp

OBJ_DIR = obj/
OBJ_NAME = $(SRC_NAME:.cpp=.o)
OBJ = $(patsubst %, $(OBJ_DIR)%, $(OBJ_NAME))
$(OBJ_DIR)%.o:$(SRC_DIR)%.cpp
		@mkdir -p $(dir $@)
		@$(CC) $(CPP_FLAGS) $< -c -o $@
		@echo "\033[1;36m Compiled" $(*F)

%.o:%.cpp
		@$(CC) $(CPP_FLAGS) $< -c -o $@
		@echo "\033[1;36m Compiled" $<

all: $(NAME)

$(NAME): $(OBJ)
		@c++ $(CPP_FLAGS) -o $(NAME) $(OBJ)
		@echo "\033[1;32m Executable" $(NAME) "created"

clean:
		@rm -rf $(OBJ_DIR)
		@echo "\033[1;31m Deleted all object files"

fclean: clean
		@rm -f $(NAME)
		@echo "\033[1;31m Deleted $(NAME)"

all: $(NAME)

re: fclean all

.PHONY: all clean fclean re