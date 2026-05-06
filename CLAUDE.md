# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

```bash
make                         # build → ./bin/ircserv
make re                      # full rebuild
make clean / fclean          # remove objects / objects + binary
./bin/ircserv <port> <password>   # e.g. ./bin/ircserv 6667 secret
```

Standard: **C++98**, flags `-Wall -Wextra -Werror -pedantic`. The Makefile auto-generates `src/config.h` from `src/config.h.in` (git commit, compiler info).

## Tests

```bash
make && bash tests/test.sh            # run all test suites
bash tests/test_registration.sh       # run a single suite
```

Tests are Bash scripts that connect via `nc` and assert server replies.

## Architecture

### Data flow

```
TCP recv() → Client::recv_buffer_ → extractLine()
→ Message::parse() → Command::execute() → handler
→ Client::send_buffer_ → poll(POLLOUT) → TCP send()
```

### Core classes

| Class / namespace | File | Role |
|---|---|---|
| `Server` | server.cpp | Owns the poll loop, listening socket, `map<int,Client>` (by FD), `map<string,Channel>` |
| `Client` | client.cpp | Per-connection state: recv/send buffers, auth flags (`pass_received_`, `registered_`), nick/user/realname |
| `Channel` | channel.cpp | Members, operators, invite list, modes (`+i +t +k +o +l`), `Broadcast()` |
| `Message` (namespace) | message.cpp | `parse(line)` → RFC 1459 struct; `build(prefix, cmd, params)` → wire string |
| `Command` (namespace) | command.cpp | Dispatcher + 12 handler functions |
| `ft_irc` | ft_irc.cpp | Entry point, CLI arg parsing, version/banner |

### poll() loop (Server::ServerInit)

- `POLLIN` is always set for every FD.
- `POLLOUT` is set dynamically only when `Client::hasPendingData()` is true.
- New connection on the listening FD → `AcceptNewClient()`.
- Data on a client FD → `ReceiveNewData()` → `extractLine()` loop → `Command::execute()`.
- `POLLOUT` ready → `HandlePollout()` drains `send_buffer_`.
- `SIGINT`/`SIGQUIT` sets a static flag; the loop exits cleanly.

### Authentication flow

1. `PASS` — validates against server password.
2. `NICK` — validates uniqueness, stores nickname.
3. `USER` — stores username/realname; server sends RPL 001–004 and marks client registered.

Commands that require registration (`JOIN`, `PRIVMSG`, operator commands) check `client.registered_` and return `ERR_NOTREGISTERED` otherwise.

### Implemented commands (12)

Pre-registration: `PASS`, `NICK`, `USER`, `QUIT`, `PING`  
Channel: `JOIN`, `PART`, `PRIVMSG`  
Operator-only: `KICK`, `INVITE`, `TOPIC`, `MODE` (`+i +t +k +o +l`)

### Channel modes

| Mode | Meaning |
|---|---|
| `+i` | Invite-only |
| `+t` | Only operators can change topic |
| `+k` | Channel key (password) |
| `+o` | Grant/revoke operator privilege |
| `+l` | User limit |

## Key conventions

- All code is **C++98** — no C++11 features.
- No external libraries; only POSIX sockets and standard C++ headers.
- `Message::build()` is the single place to construct wire-format IRC replies; always go through it rather than constructing strings inline.
- Channel names are looked up case-sensitively in `Server::channels_`.
- Nicknames are compared with `client.getNickname()` which returns the stored string as-is; collision checks iterate the client map.
