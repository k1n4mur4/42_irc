# ft_irc Development Documentation

## Project Structure

```
ft_irc/
├── Makefile
├── DEV_DOC.md
├── bin/                    # Output directory
│   └── ircserv
├── obj/                    # Build artifacts
│   ├── config.h            # Generated
│   └── *.o
└── src/                    # Source & Headers
    ├── config.h.in         # Config template
    ├── ft_irc.hpp          # Main header
    ├── ft_irc.cpp          # Main logic & Global::Version
    ├── main.cpp            # Entry point
    ├── irc_cli.cpp         # CLI parsing
    ├── server.hpp          # Server declarations
    ├── server.cpp          # Server socket handling
    ├── client.hpp          # Client declarations
    ├── client.cpp          # Client connection handling
    ├── channel.hpp         # Channel declarations (planned)
    ├── channel.cpp         # Channel management (planned)
    ├── command.hpp         # Command declarations (planned)
    ├── command.cpp         # IRC command processing (planned)
    ├── message.hpp         # Message parsing declarations (planned)
    └── message.cpp         # IRC protocol message parsing (planned)
```

---

## Module Descriptions

### Core Modules

| File | Namespace | Description |
|------|-----------|-------------|
| `ft_irc.cpp` | `Global` | Version定義, `ft_irc_main()` |
| `main.cpp` | - | `main()` エントリーポイント |
| `irc_cli.cpp` | `Cli` | コマンドライン引数解析 |

### Server Modules

| File | Namespace | Description |
|------|-----------|-------------|
| `server.cpp` | - | ソケット作成, bind, listen, accept, poll |
| `client.cpp` | - | クライアント接続管理, 切断処理 |
| `channel.cpp` | - | チャンネル作成, 参加, 退出, モード管理 (planned) |

### Protocol Modules

| File | Namespace | Description |
|------|-----------|-------------|
| `message.cpp` | - | IRCプロトコルメッセージのパース (planned) |
| `command.cpp` | - | IRCコマンド実行 (NICK, JOIN, etc.) (planned) |

---

## Namespace Design

```cpp
// ft_irc.hpp - Main header
namespace Global {
    const std::string Version = "...";
    const std::string ServerName = "ft_irc";
    // Banner definition, print_banner()
}

namespace Cli {
    struct Config {
        int         port;
        std::string password;
        bool        valid;
    };
    Config parse(int argc, char** argv);
}

int ft_irc_main(int argc, char** argv);

// server.hpp
class Server {
public:
    Server();
    Server(int port, std::string password);

    void        ServerInit();
    void        SerSocket();
    void        AcceptNewClient();
    void        ReceiveNewData(int fd);

    static void signalHandler(int signum);

    void        setPort(int port);
    int         getPort();
    void        setPassword(std::string password);
    std::string getPassword();
    void        setSignal(bool signal);
    bool        getSignal();
    void        closeFds();
    void        clearClients(int fd);
private:
    int                         port_;
    std::string                 password_;
    int                         serSocketFd_;
    static bool                 signal_;
    std::vector<Client>         clients_;
    std::vector<struct pollfd>  fds_;
};

// client.hpp
class Client {
public:
    Client();
    int     getFd();
    void    setFd(int fd);
    void    setIpAdd(std::string ipadd);
private:
    int         fd_;
    std::string ip_add_;
};

// channel.hpp (planned)
class Channel {
public:
    Channel(const std::string& name);
    void join(Client* client);
    void part(Client* client);
    void broadcast(const std::string& message, Client* sender);
private:
    std::string name_;
    std::string topic_;
    std::vector<Client*> members_;
    std::vector<Client*> operators_;
};

// message.hpp (planned)
struct IRCMessage {
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
};

IRCMessage parse(const std::string& raw);
std::string build(const IRCMessage& msg);

// command.hpp (planned)
void execute(Server& server, Client& client, const IRCMessage& msg);

// Individual command handlers
void cmd_pass(Server& server, Client& client, const std::vector<std::string>& params);
void cmd_nick(Server& server, Client& client, const std::vector<std::string>& params);
void cmd_user(Server& server, Client& client, const std::vector<std::string>& params);
void cmd_join(Server& server, Client& client, const std::vector<std::string>& params);
void cmd_part(Server& server, Client& client, const std::vector<std::string>& params);
void cmd_privmsg(Server& server, Client& client, const std::vector<std::string>& params);
void cmd_kick(Server& server, Client& client, const std::vector<std::string>& params);
void cmd_invite(Server& server, Client& client, const std::vector<std::string>& params);
void cmd_topic(Server& server, Client& client, const std::vector<std::string>& params);
void cmd_mode(Server& server, Client& client, const std::vector<std::string>& params);
void cmd_quit(Server& server, Client& client, const std::vector<std::string>& params);
```

---

## IRC Commands (42 ft_irc requirements)

| Command | Description | Implementation |
|---------|-------------|----------------|
| `PASS` | 接続パスワード | `cmd_pass()` |
| `NICK` | ニックネーム設定 | `cmd_nick()` |
| `USER` | ユーザー情報設定 | `cmd_user()` |
| `JOIN` | チャンネル参加 | `cmd_join()` |
| `PART` | チャンネル退出 | `cmd_part()` |
| `PRIVMSG` | メッセージ送信 | `cmd_privmsg()` |
| `KICK` | ユーザー追放 | `cmd_kick()` |
| `INVITE` | ユーザー招待 | `cmd_invite()` |
| `TOPIC` | トピック設定/取得 | `cmd_topic()` |
| `MODE` | モード設定 | `cmd_mode()` |
| `QUIT` | 切断 | `cmd_quit()` |

---

## Channel Modes (42 ft_irc requirements)

| Mode | Description |
|------|-------------|
| `i` | Invite-only |
| `t` | Topic restriction |
| `k` | Channel key (password) |
| `o` | Operator privilege |
| `l` | User limit |

---

## Implementation Order

1. **Phase 1: Server Foundation**
   - [x] `server.cpp` - Socket creation, bind, listen
   - [x] `client.cpp` - Client accept, fd management

2. **Phase 2: Protocol**
   - [ ] `message.cpp` - Message parsing
   - [ ] `command.cpp` - Command dispatcher

3. **Phase 3: Authentication**
   - [ ] `cmd_pass()` - Password verification
   - [ ] `cmd_nick()` - Nickname handling
   - [ ] `cmd_user()` - User registration

4. **Phase 4: Channels**
   - [ ] `channel.cpp` - Channel management
   - [ ] `cmd_join()`, `cmd_part()`
   - [ ] `cmd_privmsg()` - Message routing

5. **Phase 5: Operators**
   - [ ] `cmd_kick()`, `cmd_invite()`
   - [ ] `cmd_topic()`, `cmd_mode()`

---

## Color Constants

```cpp
namespace Color {
    const char* BOLD           = "\033[1m";
    const char* BOLD_UNDERLINE = "\033[1;4m";
    const char* BOLD_RED       = "\033[1;31m";
    const char* YELLOW         = "\033[33m";
    const char* RESET          = "\033[0m";
}
```

---

## Build Commands

```bash
make          # Build
make clean    # Remove objects
make fclean   # Remove all
make re       # Rebuild
make help     # Show targets
make info     # Show build info
```
