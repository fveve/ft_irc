NAME = irc

CC = c++

CFLAGS = -g -Wall -Wextra -Werror -I includes -std=c++98

HEADER = Server User Channel

SRCS = Server User main HandleRequest Channel channel_commands send_msg server_setup channel_mod

INCLUDES = $(addsuffix .hpp, $(addprefix includes/, $(HEADER)))

SRC = $(addsuffix .cpp, $(addprefix srcs/, $(SRCS)))

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	@echo "\033[0;32m$(NAME) Compilé !"
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJ) -I.
	@echo "\n\033[0m./$(NAME) pour exécuter le programme !"

%.o: %.cpp $(INCLUDES)
	@printf "\033[0;33mGénération des objets $(NAME)... %-33.33s\r\n" $@
	@$(CC) $(CFLAGS) -c $< -o $@

debug:
	@echo "INCLUDES: $(INCLUDES)"
	@echo "SRC: $(SRC)"
	@echo "OBJ: $(OBJ)"


clean:
	@echo "\nSuppression des binaires..."
	@rm -f $(OBJ)
	@echo "\033[0m"

fclean: clean
	@echo "\nSuppression de l'exécutable..."
	@rm -f $(NAME)
	@echo "\033[0m"

re: fclean all

.PHONY: all clean fclean re