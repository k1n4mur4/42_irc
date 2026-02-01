#include "ft_irc.hpp"
#include <cstring>

void	add_version(void) {
	std::cout << "ft_irc version " << __FT_IRC_VERSION__ << std::endl;
}

void	add_application(void) {
	std::cout << "FT_IRC Application\n" << std::endl;
}

void	add_usage(const char* program_name) {
	std::cout << "Usage: " << program_name << " <port> <password>\n" << std::endl;
}

void	add_arguments(void) {
	std::cout	<< "Arguments:\n"
		<< "  <port>         Port number to listen on (1-65535)\n"
		<< "  <password>     Connection password for clients\n"
	<< std::endl;
}

void	add_option(void) {
	std::cout	<< "Options:\n"
		<< "  -h, --help\n"
		<< "      Display this help message and exit.\n"
		<< "\n"
		<< "  -v, --version\n"
		<< "      Display version information and exit.\n" << std::endl;
}
