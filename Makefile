NAME := webserv
CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98

# OPTIONAL BUILD FLAGS (FEATURE TOGGLES)
# These allow you to enable features by setting environment variables
# Usage: make FEATURE=1

# Optimize for speed: make O3=1
# -Ofast: Maximum optimization (can be aggressive)
ifdef O3
	CXXFLAGS += -Ofast
endif

# Add debug symbols: make DEBUG=1
# -g3: Maximum debug information (for gdb debugging)
ifdef DEBUG
	CXXFLAGS += -g3
endif

# Enable AddressSanitizer: make SAN=1
# -fsanitize=address: Detects memory errors at runtime
# Examples of what it catches:
# - Buffer overflows: accessing array beyond its size
# - Use-after-free: using memory after it's freed
# - Memory leaks: forgetting to free allocated memory
# - Stack overflow: excessive stack usage
ifdef SAN
	CXXFLAGS += -fsanitize=address
endif

# Find all .cpp files in the current directory automatically
# Example: if current dir has main.cpp, HumanA.cpp, HumanB.cpp
# then SRCS becomes main.cpp HumanA.cpp HumanB.cpp
SRCDIR := ./srcs ./srcs/config-parser ./srcs/server ./srcs/http

# Tell make where to look for %.cpp prerequisites. This lets the pattern rule
# below use a plain "%.cpp" prerequisite and still find sources in any of the
# directories listed in $(SRCDIR).
vpath %.cpp $(SRCDIR)

SRCFILES := $(foreach dir,$(SRCDIR),$(wildcard $(dir)/*.cpp))

# Directory where object files will be stored (keeps source directory clean)
OBJDIR := obj

# Convert .cpp files to .o files in the obj directory
# This does several transformations:
# 1. $(SRCS:.cpp=.o): Change .cpp extension to .o → main.cpp becomes main.o
# 2. $(notdir ...): Remove directory path → main.o becomes main.o (no change for current dir)
# 3. $(addprefix $(OBJDIR)/, ...): Add obj/ prefix → main.o becomes obj/main.o
#
# Example transformation:
# main.cpp → obj/main.o
# HumanA.cpp → obj/HumanA.o
# HumanB.cpp → obj/HumanB.o
OBJFILES := $(addprefix $(OBJDIR)/,$(notdir $(SRCFILES:.cpp=.o)))

.PHONY: all clean 

all: $(NAME)

# Main target: links all object files into final executable
$(NAME): $(OBJFILES)
	@$(CXX) $(CXXFLAGS) $(OBJFILES) -o $(NAME)
	@echo Created executable \'$(NAME)\'

clean:
	@$(RM) -rf $(OBJDIR)
	@echo Deleted \'obj/\'

fclean: clean
	@$(RM) -f $(NAME)
	@echo Deleted executable \'$(NAME)\'

# Rebuild
re: fclean all

# DEPENDENCY HANDLING (ADVANCED BUT ESSENTIAL)
# Generate .d dependency files from .o files
# patsubst pattern: %.o → %.d
# Example: obj/main.o → obj/main.d
DEPENDS := $(patsubst %.o,%.d,$(OBJFILES))

# Include all dependency files if they exist
# The dash (-) means "don't error if files don't exist"
# This ensures Make knows about header file dependencies
-include $(DEPENDS)

# Rule to compile .cpp files into .o files. With the vpath above we can use a
# plain "%.cpp" prerequisite and make will search the directories in
# $(SRCDIR) for the matching source file.
$(OBJDIR)/%.o : %.cpp Makefile
# Create output directory if it doesn't exist
	@mkdir -p $(OBJDIR)
# Call the tidy_compilation function with the compile command
# The command includes:
# -MMD: Generate dependency files (.d)
# -MP: Add phony targets for headers (prevents errors if headers are deleted)
# -c: Compile only (don't link)
# $<: Input file (the .cpp file)
# -o $@: Output file (the .o file)
	@$(call tidy_compilation,$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@)

# TIDY COMPILATION FUNCTION
# Custom function to display clean compilation messages
define tidy_compilation
	@printf "%s\e[K\n" "$(1)"
	# Execute the actual compilation command (passed as $1)
	@$(1)
	# Use ANSI escape codes to clear the previous line
	# \e[A: Move cursor up one line
	# \e[K: Clear the entire line
	# This creates clean output without showing the long compiler command
	@printf "\e[A\e[K"
endef

# USAGE EXAMPLES:
# make              → Normal build
# make DEBUG=1      → Build with debug symbols
# make O3=1         → Build with maximum optimization  
# make SAN=1        → Build with memory error detection
# make clean        → Remove object files
# make fclean       → Remove everything
# make re           → Rebuild from scratch
# make re SAN=1     → Clean rebuild with sanitizer
# Combined flags example:
# make DEBUG=1 SAN=1 → Debug build with memory checking

# Inspired from: Ajelloul's blog
# Reference: aboubakr-jelloulat.github.io/posts/makefile-complete-guide/
