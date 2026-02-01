# ifndef FT_IRC_HPP
# define FT_IRC_HPP

#include "utils.hpp"

#include <iostream>
#include <string>

# define __FT_IRC_VERSION__ "1.0.0"

// message
void	add_version(void);
void	add_usage(const char* program_name);
void	add_application(void);
void	add_arguments(void);
void	add_option(void);

#endif