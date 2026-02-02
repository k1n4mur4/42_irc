#include "ft_irc.hpp"

namespace Global {
	const std::string Version = "1.0.0";
}

int	ft_irc_main(int argc, char** argv) {
	Cli::Config config = Cli::parse(argc, argv);
	if (!config.valid) {
		return (EXIT_FAILURE);
	}
	// TODO: Start IRC server with config.port and config.password
	return (EXIT_SUCCESS);
}
