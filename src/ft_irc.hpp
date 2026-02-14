# ifndef FT_IRC_HPP
# define FT_IRC_HPP

#include <iostream>
#include <string>
#include "server.hpp"
#include "client.hpp"
#include "message.hpp"
#include "command.hpp"

// Global namespace
namespace Global {
	const std::string Version = "26.2.15";
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

	inline void print_banner() {
		const char* reset = "\033[0m";
		const char* dim = "\033[2;3;38;5;240m";

		for (int i = 0; Banner[i].text; ++i) {
			std::cout << Banner[i].color << Banner[i].text << reset << std::endl;
		}
		std::cout << dim << "                      v" << Version << reset << std::endl;
		std::cout << std::endl;
	}
}

// CLI namespace
namespace Cli {
	struct Config {
		int			port;
		std::string	password;
		bool		valid;
	};

	Config parse(int argc, char** argv);
}

// Main entry point
int	ft_irc_main(int argc, char** argv);

#endif
