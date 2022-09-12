# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pmedina- <3 carce-bo	                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/03/23 22:21:34 by pmedina-          #+#    #+#              #
#    Updated: 2022/04/21 21:32:41 by pmedina-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# 'dir' is used in here to increase readibity, the current usage has no other effect.

NAME		=	ircserv
SRCS		=	srcs/main.cpp\
				srcs/Server/Server.cpp \
				srcs/Server/AIrcCommands.cpp \
				srcs/Server/CommonReplies.cpp \
				srcs/Server/CommandUtils.cpp \
				srcs/Server/FdManager.cpp \
				srcs/Server/IrcDataBase.cpp \
				srcs/User.cpp \
				srcs/Channel.cpp \
				srcs/Tools.cpp \
				srcs/Command.cpp \
				srcs/Log.cpp 
CXX			=	g++ 
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98 -pedantic -g3 -Wno-c++0x-compat
RM			=	rm -f
OBJS		=	$(SRCS:.cpp=.o)

LIBFT_DIR = libft/
LIBFT_LINK = -L $(dir $(LIBFT_DIR)) -lft
LIBFT = libft.a

INC_DIR = includes/

UNAME := $(shell uname)
# Compiles using threads (make re doesnt work with this, so fuck it). 
# See https://stackoverflow.com/questions/4778389/automatically-setting-jobs-j-flag-for-a-multicore-machine (Linux)
# See https://stackoverflow.com/questions/1715580/how-to-discover-number-of-logical-cores-on-mac-os-x (MacosX)
ifeq ($(UNAME), Linux)
#export MAKEFLAGS="-j $(nproc --all)"
endif
ifeq ($(UNAME), Darwin)
#export MAKEFLAGS="-j $(sysctl -n hw.ncpu)"
endif

all: 		$(NAME)

%.o: 		%.cpp
			$(CXX) $(CXXFLAGS) -I $(dir $(LIBFT_DIR)) -I $(dir $(INC_DIR)) -c $< -o $@

$(LIBFT_DIR)$(LIBFT): \
			$(LIBFT_DIR)
			make -C $(dir $(LIBFT_DIR))

# See https://stackoverflow.com/questions/42586080/gcc-linking-object-files-with-warning-optimization-flags
$(NAME): 	$(OBJS) $(dir $(LIBFT_DIR))$(LIBFT)
			$(CXX) $(OBJS) $(CXXFLAGS) $(LIBFT_LINK) -o  $@

clean:
			$(RM) $(OBJS)
			make -C $(dir $(LIBFT_DIR)) clean

fclean:		clean
			make -C $(dir $(LIBFT_DIR)) fclean
			$(RM) $(NAME)

re:			fclean all

.PHONY:		all clean fclean re
