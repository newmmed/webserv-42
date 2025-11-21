NAME := webserv

CXX := c++

CXXFLAGS := -Wall -Wextra -Werror -std=c++98

SRCDIR := ./srcs ./srcs/config-parser ./srcs/server ./srcs/http

vpath %.cpp $(SRCDIR)

SRCFILES := $(foreach dir,$(SRCDIR),$(wildcard $(dir)/*.cpp))

OBJDIR := obj

OBJFILES := $(addprefix $(OBJDIR)/,$(notdir $(SRCFILES:.cpp=.o)))

.PHONY: all clean 

all: $(NAME)

$(NAME): $(OBJFILES)
	@$(CXX) $(CXXFLAGS) $(OBJFILES) -o $(NAME)
	@echo Created executable \'$(NAME)\'

clean:
	@$(RM) -rf $(OBJDIR)
	@echo Deleted \'obj/\'

fclean: clean
	@$(RM) -f $(NAME)
	@echo Deleted executable \'$(NAME)\'

re: fclean all

DEPENDS := $(patsubst %.o,%.d,$(OBJFILES))

-include $(DEPENDS)

$(OBJDIR)/%.o : %.cpp Makefile
	@mkdir -p $(OBJDIR)
	@$(call tidy_compilation,$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@)

define tidy_compilation
	@printf "%s\e[K\n" "$(1)"
	@$(1)
	@printf "\e[A\e[K"
endef
