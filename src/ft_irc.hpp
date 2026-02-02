# ifndef FT_IRC_HPP
# define FT_IRC_HPP

#include "utils.hpp"

#include <iostream>
#include <string>

// Global namespace
namespace Global {
	extern const std::string Version;
}

// Main entry point
int	ft_irc_main(int argc, char** argv);

// CLI namespace
namespace Cli {
	struct Config {
		int port;
		std::string password;
		bool valid;
	};

	Config parse(int argc, char** argv);
}

#endif
