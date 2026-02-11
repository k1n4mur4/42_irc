# ft_irc

An IRC server implemented in C++98 as part of the [42 School](https://42.fr/) curriculum. Uses `poll()` for multiplexed non-blocking I/O to handle multiple concurrent clients over TCP.

## Features

- Multiple simultaneous client connections over TCP
- `poll()`-based event loop for non-blocking I/O
- Graceful shutdown via signal handling (SIGINT, SIGQUIT)
- CLI argument parsing (port and password)
- Auto-generated build configuration (git commit hash, compiler info)

## Requirements

- C++ compiler with C++98 support
- POSIX environment (macOS / Linux)
- Make

## Build

```bash
make            # Build (parallel, uses all CPU cores)
make clean      # Remove object files
make fclean     # Remove objects and binary
make re         # Full rebuild
make info       # Show build configuration
```

## Usage

```bash
./bin/ircserv <port> <password>
```

Connect with any IRC-compatible client:

```bash
nc localhost 6667
```

Or use a graphical IRC client such as [irssi](https://irssi.org/), [WeeChat](https://weechat.org/), or [HexChat](https://hexchat.github.io/).

## Project Structure

```
ft_irc/
├── Makefile
├── DEV_DOC.md
├── src/
│   ├── main.cpp          # Entry point
│   ├── ft_irc.hpp        # Main header (Global, Cli namespaces)
│   ├── ft_irc.cpp        # ft_irc_main(), Global::Version
│   ├── irc_cli.cpp       # CLI argument parsing
│   ├── config.h.in       # Build config template
│   ├── server.hpp        # Server class declaration
│   ├── server.cpp        # Socket, poll loop, accept, receive
│   ├── client.hpp        # Client class declaration
│   └── client.cpp        # Client fd/IP management
├── bin/
│   └── ircserv           # Output binary
└── obj/                  # Build artifacts
```

## Roadmap

| Phase | Description | Status |
|-------|-------------|--------|
| 1 | Server Foundation — socket, bind, listen, accept, poll loop | Done |
| 2 | Protocol — IRC message parsing, command dispatcher | Planned |
| 3 | Authentication — PASS, NICK, USER | Planned |
| 4 | Channels — JOIN, PART, PRIVMSG, channel management | Planned |
| 5 | Operators — KICK, INVITE, TOPIC, MODE | Planned |

See [DEV_DOC.md](DEV_DOC.md) for the full development documentation.

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
