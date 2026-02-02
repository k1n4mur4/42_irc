# ifndef FT_IRC_HPP
# define FT_IRC_HPP

#include "utils.hpp"

#include <iostream>
#include <string>

// Global namespace
namespace Global {
	const std::string Version = "1.0.0";
	const std::string ServerName = "ft_irc";

	struct BannerLine {
		const char* color;
		const char* text;
	};

	const BannerLine Banner[] = {
		{"\033[38;5;51m",  " ██╗ ██████╗  ██████╗"},
		{"\033[38;5;45m",  " ██║ ██╔══██╗██╔════╝"},
		{"\033[38;5;39m",  " ██║ ██████╔╝██║     "},
		{"\033[38;5;33m",  " ██║ ██╔══██╗██║     "},
		{"\033[38;5;27m",  " ██║ ██║  ██║╚██████╗"},
		{"\033[38;5;21m",  " ╚═╝ ╚═╝  ╚═╝ ╚═════╝"},
		{0, 0}
	};

	void print_banner();
}

// Main entry point
int	ft_irc_main(int argc, char** argv);

// CLI namespace
namespace Cli {
	struct Config {
		int			port;
		std::string	password;
		bool		valid;
	};

	Config parse(int argc, char** argv);
}

#endif
