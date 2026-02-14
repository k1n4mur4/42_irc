# ft_irc

An IRC server implemented in C++98 as part of the [42 School](https://42.fr/) curriculum.
Handles multiple concurrent clients over TCP using `poll()` for multiplexed non-blocking I/O, with RFC 1459 compliant message parsing.

## Features

- **12 IRC commands** — PASS, NICK, USER, QUIT, PING, JOIN, PART, PRIVMSG, KICK, INVITE, TOPIC, MODE
- **5 channel modes** — invite-only (`+i`), topic restriction (`+t`), channel key (`+k`), operator privilege (`+o`), user limit (`+l`)
- **RFC 1459 message parsing** — prefix, command, and parameter extraction with trailing parameter support
- **`poll()`-based non-blocking I/O** — all network I/O goes through the poll loop; no direct send/recv outside it
- **Authentication flow** — three-step registration: PASS → NICK → USER
- **Send/receive buffering** — partial data handling for fragmented TCP streams (supports `nc` + Ctrl+D)
- **Graceful shutdown** — signal handling for SIGINT and SIGQUIT

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
make help       # Show available targets
```

## Usage

```bash
./bin/ircserv <port> <password>
```

### Connecting with an IRC client

```bash
# irssi
irssi -c localhost -p 6667 -w mypassword

# WeeChat
/server add local localhost/6667 -password=mypassword
/connect local

# netcat (manual testing)
nc -C localhost 6667
PASS mypassword
NICK mynick
USER mynick 0 * :My Real Name
```

## Supported Commands

### Registration

| Command | Syntax | Description |
|---------|--------|-------------|
| `PASS` | `PASS <password>` | Authenticate with the server password |
| `NICK` | `NICK <nickname>` | Set or change nickname |
| `USER` | `USER <username> 0 * :<realname>` | Set username and real name |
| `QUIT` | `QUIT [:<message>]` | Disconnect from the server |
| `PING` | `PING <token>` | Keep-alive; server responds with PONG |

### Channels

| Command | Syntax | Description |
|---------|--------|-------------|
| `JOIN` | `JOIN <channel>[,<channel>] [<key>[,<key>]]` | Join one or more channels |
| `PART` | `PART <channel> [:<message>]` | Leave a channel |
| `PRIVMSG` | `PRIVMSG <target> :<message>` | Send a message to a channel or user |

### Operator

| Command | Syntax | Description |
|---------|--------|-------------|
| `KICK` | `KICK <channel> <nick> [:<reason>]` | Remove a user from a channel |
| `INVITE` | `INVITE <nick> <channel>` | Invite a user to a channel |
| `TOPIC` | `TOPIC <channel> [:<topic>]` | Set or query the channel topic |
| `MODE` | `MODE <channel> <modes> [<params>]` | Set or query channel modes |

## Channel Modes

| Mode | Description | Parameter |
|------|-------------|-----------|
| `+i` | Invite-only — only invited users can join | — |
| `+t` | Topic restricted — only operators can change the topic | — |
| `+k` | Channel key — requires a password to join | `<key>` (set) |
| `+o` | Operator privilege — grant or revoke operator status | `<nick>` |
| `+l` | User limit — restrict the maximum number of members | `<limit>` (set) |

Multiple modes can be set in a single command:

```
MODE #channel +itk-o secretkey nick
MODE #channel +kl secret 10
```

## Authentication Flow

```
Client                          Server
  │                               │
  │  PASS <password>              │
  │ ───────────────────────────>  │  Validate password
  │                               │
  │  NICK <nickname>              │
  │ ───────────────────────────>  │  Validate + check duplicates
  │                               │
  │  USER <user> 0 * :<real>      │
  │ ───────────────────────────>  │  Set user info
  │                               │
  │  001 RPL_WELCOME              │
  │  002 RPL_YOURHOST             │
  │  003 RPL_CREATED              │
  │  004 RPL_MYINFO               │
  │ <───────────────────────────  │  Registration complete
  │                               │
```

Unregistered clients can only use PASS, NICK, USER, QUIT, and PING. All other commands return `451 ERR_NOTREGISTERED`.

## Architecture

### Data Flow

```
recv() → Client::recv_buffer_ → extractLine()
       → Message::parse() → Command::execute() → handler
       → Client::send_buffer_ → poll(POLLOUT) → send()
```

### Class Overview

| Class | Responsibility |
|-------|---------------|
| **Server** | Owns the listening socket, `poll()` loop, and client/channel maps. Manages connections, dispatches received data, and handles outgoing writes via POLLOUT. |
| **Client** | Represents a connected user. Manages recv/send buffers, authentication state (password, nickname, username), and generates the `nick!user@host` prefix. |
| **Channel** | Represents an IRC channel. Manages members, operators, invite lists, modes, and broadcasts messages to members. |
| **Message** | Namespace for RFC 1459 message parsing (`parse()`) and building (`build()`). |
| **Command** | Namespace containing the command dispatcher (`execute()`) and all 12 command handlers. |

## Project Structure

```
ft_irc/
├── Makefile
├── DEV_DOC.md            # Development documentation
├── FEAT_IRC.md           # IRC feature requirements
├── LICENSE
├── src/
│   ├── main.cpp          # Entry point
│   ├── ft_irc.hpp        # Main header (Global, Cli namespaces)
│   ├── ft_irc.cpp        # ft_irc_main(), version, banner
│   ├── irc_cli.cpp       # CLI argument parsing
│   ├── config.h.in       # Build config template
│   ├── server.hpp        # Server class declaration
│   ├── server.cpp        # Socket, poll loop, client/channel management
│   ├── client.hpp        # Client class declaration
│   ├── client.cpp        # Client buffering, auth state
│   ├── channel.hpp       # Channel class declaration
│   ├── channel.cpp       # Channel member/operator/invite management
│   ├── command.hpp       # Command dispatcher declaration
│   ├── command.cpp       # IRC command handlers
│   ├── message.hpp       # Message parsing declaration
│   ├── message.cpp       # IRC protocol message parsing/building
│   └── osx/
│       └── os.cpp        # macOS platform-specific code
├── tests/
│   ├── test.sh           # Test runner
│   ├── helpers.sh        # Shared test helpers
│   ├── test_registration.sh
│   ├── test_ping.sh
│   ├── test_channel.sh
│   ├── test_operator.sh
│   └── test_errors.sh
├── bin/
│   └── ircserv           # Output binary
└── obj/                  # Build artifacts
```

## Testing

### Automated Tests (24 tests)

```bash
make && bash tests/test.sh
```

| Test Suite | Tests | Coverage |
|------------|-------|----------|
| `test_registration.sh` | 7 | PASS, NICK, USER validation and error cases |
| `test_ping.sh` | 1 | PING/PONG keep-alive |
| `test_channel.sh` | 5 | JOIN, PART, PRIVMSG, first-user-is-operator |
| `test_operator.sh` | 7 | KICK, INVITE, TOPIC, MODE (+i, +k, +l) |
| `test_errors.sh` | 4 | Channel errors, permission errors |

Individual test suites can be run separately:

```bash
bash tests/test_registration.sh
bash tests/test_channel.sh
```

### Manual Testing with netcat

```bash
# Terminal 1: Start server
make && ./bin/ircserv 6667 pass

# Terminal 2: Connect and register
nc -C localhost 6667
PASS pass
NICK testuser
USER testuser 0 * :Test User
JOIN #test
PRIVMSG #test :Hello, world!
PART #test :Goodbye
QUIT :Bye
```

See [DEV_DOC.md](DEV_DOC.md) for the full development documentation.

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
