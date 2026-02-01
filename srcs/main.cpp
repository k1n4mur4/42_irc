#include "ft_irc.hpp"

int main(int argc, char **argv) {
	if (argc == 2)
	{
		if (std::strcmp(argv[1], "-h") == 0 || std::strcmp(argv[1], "--help") == 0)
			add_option();
		else if (std::strcmp(argv[1], "-v") == 0 || std::strcmp(argv[1], "--version") == 0)
			add_version();
		else {
			add_application();
			add_usage(argv[0]);
			add_option();
		}
		return (EXIT_FAILURE);
	}
	if (argc != 3)
	{
		add_usage(argv[0]);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
