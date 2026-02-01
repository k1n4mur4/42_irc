NAME		=	ircserv
CXX			=	c++
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98
RM			=	rm -rf
MAKEFLAGS	+=	--no-print-directory

# OS detection
UNAME_S		=	$(shell uname -s)
ifeq ($(UNAME_S), Darwin)
	OS_DIR	=	./macos
else
	OS_DIR	=	./linux
endif

######################### Directories #########################
SRCS_DIR	=	./srcs

######################### Sources #########################
OS_SRCS		=	$(OS_DIR)/os.cpp

### Common
SRCS		=	$(SRCS_DIR)/main.cpp
SRCS		+=	$(SRCS_DIR)/help.cpp
SRCS		+=	$(OS_SRCS)

######################### Objects #########################
OBJS_DIR	=	./objs

# Convert both SRCS_DIR and OS_DIR paths to OBJS_DIR
OBJS		=	$(patsubst $(SRCS_DIR)/%.cpp,$(OBJS_DIR)/%.o,$(filter $(SRCS_DIR)/%.cpp,$(SRCS))) \
				$(patsubst $(OS_DIR)/%.cpp,$(OBJS_DIR)/os_%.o,$(filter $(OS_DIR)/%.cpp,$(SRCS)))

######################### Includes #########################
INCLUDES	=	-I ./includes

######################### UI/UX #########################
### Progress Bar
TOTAL		:=	$(words $(SRCS))
CURRENT		:=	0
define show_progress
	$(eval CURRENT=$(shell echo $$(($(CURRENT)+1))))
	@if [ $(CURRENT) -eq 1 ]; then \
		printf "Compiling sources for $(NAME)...\n"; \
	fi
	@WIDTH=$$(if [ "$${COLUMNS:-0}" -gt 0 ]; then echo $$COLUMNS; else tput cols 2>/dev/null || echo 80; fi); \
	BAR_WIDTH=$$((WIDTH - 20)); \
	if [ $$BAR_WIDTH -lt 10 ]; then BAR_WIDTH=10; fi; \
	FILLED=$$(($(CURRENT) * BAR_WIDTH / $(TOTAL))); \
	BAR=""; \
	for i in $$(seq 1 $$BAR_WIDTH); do \
		if [ $$i -le $$FILLED ]; then \
			BAR="$${BAR}="; \
		else \
			BAR="$${BAR} "; \
		fi; \
	done; \
	printf "\r[%s] %d/%d (%d%%)  " "$$BAR" $(CURRENT) $(TOTAL) $$(($(CURRENT) * 100 / $(TOTAL)))
endef

######################### Targets #########################
all: $(NAME)

$(NAME): $(OBJS)
	@echo "\nLinking..."
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@echo "Build complete! Executable: $(NAME)"

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(call show_progress)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS_DIR)/os_%.o: $(OS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(call show_progress)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "Cleaning up..."
	@$(RM) $(OBJS_DIR)
	@echo "Clean objects complete."

fclean: clean
	@$(RM) $(NAME)
	@echo "Clean all complete."

re: fclean all

.PHONY: all clean fclean re
