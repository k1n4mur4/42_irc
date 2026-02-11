#include "ft_irc.hpp"
#include <cstdlib>

bool Server::signal_ = false;

int	ft_irc_main(int argc, char** argv) {
	Cli::Config config = Cli::parse(argc, argv);
	if (!config.valid) {
		return (EXIT_FAILURE);
	}
	Global::print_banner();

	Server ser = Server(config.port, config.password);

	std::cout << "---- SERVER ----" << std::endl;

	try{
		signal(SIGINT, Server::signalHandler);
		signal(SIGQUIT, Server::signalHandler);
		ser.ServerInit();
	}
	catch(const std::exception& e){
		ser.closeFds();
		std::cerr << e.what() << std::endl;
	}

	std::cout << "The Server Closed!" << std::endl;

	return (EXIT_SUCCESS);
}
