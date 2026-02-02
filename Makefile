#* ft_irc Makefile v1.6

MAKEFILE_VERSION := 1.6
BANNER  = \n \033[38;5;51m██\033[38;5;240m╗ \033[38;5;51m██████\033[38;5;240m╗  \033[38;5;51m██████\033[38;5;240m╗\n \033[38;5;45m██\033[38;5;239m║ \033[38;5;45m██\033[38;5;239m╔══\033[38;5;45m██\033[38;5;239m╗\033[38;5;45m██\033[38;5;239m╔════╝\n \033[38;5;39m██\033[38;5;238m║ \033[38;5;39m██████\033[38;5;238m╔╝\033[38;5;39m██\033[38;5;238m║     \n \033[38;5;33m██\033[38;5;237m║ \033[38;5;33m██\033[38;5;237m╔══\033[38;5;33m██\033[38;5;237m╗\033[38;5;33m██\033[38;5;237m║     \n \033[38;5;27m██\033[38;5;236m║ \033[38;5;27m██\033[38;5;236m║  \033[38;5;27m██\033[38;5;236m║╚\033[38;5;27m██████\033[38;5;236m╗\n \033[38;5;235m╚═╝ ╚═╝  ╚═╝ ╚═════╝ \033[1;3;38;5;240mMakefile v$(MAKEFILE_VERSION)\033[0m\n

override VERSION := $(shell head -n100 src/ft_irc.cpp 2>/dev/null | grep "Version =" | cut -f2 -d'"' || echo "unknown")
override TIMESTAMP := $(shell date +%s 2>/dev/null || echo "0")

ifneq ($(QUIET),true)
	override QUIET := false
endif

#? Compiler and Linker
NAME		:= ircserv
CXX			:= c++
RM			:= rm -rf

#? Compiler Flags (btop-style split)
REQFLAGS	:= -std=c++98
WARNFLAGS	:= -Wall -Wextra -Werror -pedantic
OPTFLAGS	:=
LDFLAGS		:=
CXXFLAGS	:= $(REQFLAGS) $(WARNFLAGS) $(OPTFLAGS)

#? Git and compiler info for config.h
GIT_COMMIT			:= $(shell git rev-parse --short HEAD 2>/dev/null || echo "")
CONFIGURE_COMMAND	:= $(MAKE)
override CXX_VERSION := $(shell $(CXX) -dumpfullversion -dumpversion 2>/dev/null || echo "unknown")

#? Detect PLATFORM and ARCH
ARCH		?= $(shell uname -m || echo unknown)

ifeq ($(shell uname -s),Darwin)
	PLATFORM		:= macOS
	PLATFORM_DIR	:= osx
	THREADS			:= $(shell sysctl -n hw.ncpu || echo 1)
else
	PLATFORM		:= $(shell uname -s || echo unknown)
	PLATFORM_DIR	:= linux
	THREADS			:= $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)
endif

ifeq ($(shell command -v gdate >/dev/null; echo $$?),0)
	DATE_CMD := gdate
else
	DATE_CMD := date
endif

#? Use all CPU cores
MAKEFLAGS	+= --jobs=$(THREADS) --no-print-directory

ifeq ($(VERBOSE),true)
	override VERBOSE := false
else
	override VERBOSE := true
endif

#? The Directories, Source, Objects
SRCDIR		:= src
BUILDDIR	:= obj
TARGETDIR	:= bin
SRCEXT		:= cpp
OBJEXT		:= o

#? Sources and Objects
SOURCES		:= $(sort $(wildcard $(SRCDIR)/*.$(SRCEXT)))
SOURCES		+= $(sort $(wildcard $(SRCDIR)/$(PLATFORM_DIR)/*.$(SRCEXT)))

OBJECTS		:= $(patsubst $(SRCDIR)/%.$(SRCEXT),$(BUILDDIR)/%.$(OBJEXT),$(SOURCES))

#? Includes (btop-style: src/ for headers, obj/ for generated config.h)
INC			:= -I $(SRCDIR) -I $(BUILDDIR)

#? Setup percentage progress
SOURCE_COUNT := $(words $(SOURCES))
P := %%

ifeq ($(VERBOSE),true)
	override SUPPRESS := > /dev/null 2> /dev/null
else
	override SUPPRESS :=
endif

#? Default Make
.ONESHELL:
all: | info directories config.h $(TARGETDIR)/$(NAME)
	@printf "\n\033[1;92mBuild complete in \033[92m(\033[97m$$($(DATE_CMD) -d @$$(expr $$(date +%s 2>/dev/null || echo "0") - $(TIMESTAMP) 2>/dev/null) -u +%Mm:%Ss 2>/dev/null | sed 's/^00m://' || echo "unknown")\033[92m)\033[0m\n"

ifneq ($(QUIET),true)
info:
	@printf " $(BANNER)\n"
	@printf "\033[1;92mPLATFORM     \033[1;93m?| \033[0m$(PLATFORM)\n"
	@printf "\033[1;96mARCH         \033[1;93m?| \033[0m$(ARCH)\n"
	@printf "\033[1;93mCXX          \033[1;93m?| \033[0m$(CXX) \033[1;93m(\033[97m$(CXX_VERSION)\033[93m)\n"
	@printf "\033[1;94mTHREADS      \033[1;94m:| \033[0m$(THREADS)\n"
	@printf "\033[1;91mREQFLAGS     \033[1;91m!| \033[0m$(REQFLAGS)\n"
	@printf "\033[1;93mWARNFLAGS    \033[1;94m:| \033[0m$(WARNFLAGS)\n"
	@printf "\033[1;95mOPTFLAGS     \033[1;94m:| \033[0m$(if $(OPTFLAGS),$(OPTFLAGS),(none))\n"
	@printf "\033[1;96mLDFLAGS      \033[1;94m:| \033[0m$(if $(LDFLAGS),$(LDFLAGS),(none))\n"
	@printf '\033[1;97mCXXFLAGS     \033[1;92m+| \033[0m$$(REQFLAGS) $$(WARNFLAGS) $$(OPTFLAGS)\n'
	@printf "\033[1;95mINCLUDES     \033[1;92m+| \033[0m$(INC)\n"
	@printf "\n\033[1;92mBuilding $(NAME) \033[91m(\033[97mv$(VERSION)\033[91m) \033[93m$(PLATFORM) \033[96m$(ARCH)\033[0m\n\n"
else
info:
	@true
endif

help:
	@printf " $(BANNER)\n"
	@printf "\033[1;97mft_irc makefile\033[0m\n"
	@printf "usage: make [target]\n\n"
	@printf "\033[1;4mTargets:\033[0m\n"
	@printf "  \033[1mall\033[0m          Compile $(NAME) (default)\n"
	@printf "  \033[1mclean\033[0m        Remove built objects\n"
	@printf "  \033[1mfclean\033[0m       Remove built objects and binary\n"
	@printf "  \033[1mre\033[0m           Rebuild from scratch\n"
	@printf "  \033[1minfo\033[0m         Display build information\n"
	@printf "  \033[1mhelp\033[0m         Show this help message\n"

#? Make the Directories
directories:
	@$(VERBOSE) || printf "mkdir -p $(TARGETDIR)\n"
	@mkdir -p $(TARGETDIR)
	@$(VERBOSE) || printf "mkdir -p $(BUILDDIR)/$(PLATFORM_DIR)\n"
	@mkdir -p $(BUILDDIR)/$(PLATFORM_DIR)
	@echo 0 > $(BUILDDIR)/.progress_count

#? Generate config.h from template
config.h: $(BUILDDIR)/config.h

$(BUILDDIR)/config.h: $(SRCDIR)/config.h.in | directories
	@$(QUIET) || printf "\033[1mConfiguring $(BUILDDIR)/config.h\033[0m\n"
	@sed -e 's|@GIT_COMMIT@|$(GIT_COMMIT)|g' \
	     -e 's|@CONFIGURE_COMMAND@|$(CONFIGURE_COMMAND)|g' \
	     -e 's|@COMPILER@|$(CXX)|g' \
	     -e 's|@COMPILER_VERSION@|$(CXX_VERSION)|g' \
	     $< > $@

#? Link
.ONESHELL:
$(TARGETDIR)/$(NAME): $(OBJECTS) | directories
	@TSTAMP=$$(date +%s 2>/dev/null || echo "0")
	@$(QUIET) || printf "\033[1;92mLinking and creating binary\033[37m...\033[0m\n"
	@$(VERBOSE) || printf "$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^\n"
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ || exit 1
	@printf "\033[1;92m100$(P) -> \033[1;37m$@ \033[1;93m(\033[1;97m$$(du -h $@ | cut -f1)\033[1;93m)\033[0m\n"

#? Compile
.ONESHELL:
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT) | directories config.h
	@TSTAMP=$$(date +%s 2>/dev/null || echo "0")
	@$(QUIET) || printf "\033[1;97mCompiling $<\033[0m\n"
	@$(VERBOSE) || printf "$(CXX) $(CXXFLAGS) $(INC) -c -o $@ $<\n"
	@$(CXX) $(CXXFLAGS) $(INC) -c -o $@ $< || exit 1
	@while ! mkdir $(BUILDDIR)/.progress_lock 2>/dev/null; do sleep 0.01; done; \
	PROGRESS=$$(( $$(cat $(BUILDDIR)/.progress_count) + 1 )); \
	echo $$PROGRESS > $(BUILDDIR)/.progress_count; \
	rmdir $(BUILDDIR)/.progress_lock; \
	PERCENT=$$((PROGRESS * 100 / $(SOURCE_COUNT))); \
	printf "\033[1;92m%3d$(P) -> \033[1;37m$@\033[0m\n" $$PERCENT

#? Clean only Objects
clean:
	@printf "\033[1;91mRemoving: \033[1;97mbuilt objects...\033[0m\n"
	@$(RM) $(BUILDDIR)
	@printf "\033[1;92mClean complete.\033[0m\n"

#? Clean Objects and Binary
fclean: clean
	@printf "\033[1;91mRemoving: \033[1;97m$(TARGETDIR)/...\033[0m\n"
	@$(RM) $(TARGETDIR)
	@printf "\033[1;92mFull clean complete.\033[0m\n"

#? Rebuild
re: fclean all

#? Non-File Targets
.PHONY: all clean fclean re info help directories config.h
