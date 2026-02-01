#include "ft_irc.hpp"
#include <cstring>

int main(int argc, char **argv) {
	if (argc == 2)
	{
		enum Option { OPT_HELP, OPT_VERSION, OPT_UNKNOWN };
		Option opt = OPT_UNKNOWN;

		if (std::strcmp(argv[1], "-h") == 0 || std::strcmp(argv[1], "--help") == 0)
			opt = OPT_HELP;
		else if (std::strcmp(argv[1], "-v") == 0 || std::strcmp(argv[1], "--version") == 0)
			opt = OPT_VERSION;

		switch (opt)
		{
			case OPT_HELP:
				add_option();
				break;
			case OPT_VERSION:
				add_version();
				break;
			case OPT_UNKNOWN:
				add_application();
				add_usage(argv[0]);
				add_option();
				break;
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
