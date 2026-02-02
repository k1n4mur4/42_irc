#include "ft_irc.hpp"

namespace Global {
	void print_banner() {
		const char* reset = "\033[0m";
		const char* dim = "\033[2;3;38;5;240m";

		for (int i = 0; Banner[i].text; ++i) {
			std::cout << Banner[i].color << Banner[i].text << reset << std::endl;
		}
		std::cout << dim << "                      v" << Version << reset << std::endl;
		std::cout << std::endl;
	}
}

int	ft_irc_main(int argc, char** argv) {
	Cli::Config config = Cli::parse(argc, argv);
	if (!config.valid) {
		return (EXIT_FAILURE);
	}
	Global::print_banner();
	// TODO: Start IRC server with config.port and config.password
	return (EXIT_SUCCESS);
}
