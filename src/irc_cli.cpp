// SPDX-License-Identifier: MIT

#include "ft_irc.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace Color {
	const char* BOLD = "\033[1m";
	const char* BOLD_UNDERLINE = "\033[1;4m";
	const char* BOLD_RED = "\033[1;31m";
	const char* YELLOW = "\033[33m";
	const char* RESET = "\033[0m";
}

namespace Cli {
	static void error(const std::string& msg) {
		std::cerr << Color::BOLD_RED << "error:" << Color::RESET << " "
			<< msg << "\n" << std::endl;
	}

	static void version() {
		std::cout << "ft_irc version: " << Color::BOLD << Global::Version
			<< Color::RESET << std::endl;
	}

	static void usage() {
		std::cout << Color::BOLD_UNDERLINE << "Usage:" << Color::RESET << " "
			<< Color::BOLD << "ircserv" << Color::RESET
			<< " <port> <password>\n" << std::endl;
	}

	static void help() {
		std::cout << Color::BOLD_UNDERLINE << "Arguments:" << Color::RESET << "\n"
			<< "  " << Color::BOLD << "<port>" << Color::RESET
			<< "       Port number (1-65535)\n"
			<< "  " << Color::BOLD << "<password>" << Color::RESET
			<< "   Connection password\n\n"
			<< Color::BOLD_UNDERLINE << "Options:" << Color::RESET << "\n"
			<< "  " << Color::BOLD << "-h, --help" << Color::RESET
			<< "     Show this help message\n"
			<< "  " << Color::BOLD << "-v, --version" << Color::RESET
			<< "  Show version\n" << std::endl;
	}

	Config parse(int argc, char** argv) {
		Config config;
		config.port = 0;
		config.password = "";
		config.valid = false;

		// Option parsing
		for (int i = 1; i < argc; ++i) {
			std::string arg(argv[i]);
			if (arg == "-h" || arg == "--help") {
				usage();
				help();
				return config;
			}
			if (arg == "-v" || arg == "--version") {
				version();
				return config;
			}
		}

		// Argument count check
		if (argc != 3) {
			error("expected 2 arguments: <port> <password>");
			usage();
			return config;
		}

		// Port parsing
		char* endptr;
		long port = std::strtol(argv[1], &endptr, 10);
		if (*endptr != '\0' || port < 1 || port > 65535) {
			error("port must be a number between 1 and 65535");
			return config;
		}

		config.port = static_cast<int>(port);
		config.password = argv[2];
		config.valid = true;
		return config;
	}
}
