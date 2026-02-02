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
    ├── irc_cli.hpp         # CLI declarations
    ├── irc_cli.cpp         # CLI parsing
    ├── irc_server.hpp      # Server declarations
    ├── irc_server.cpp      # Server socket handling
    ├── irc_client.hpp      # Client declarations
    ├── irc_client.cpp      # Client connection handling
    ├── irc_channel.hpp     # Channel declarations
    ├── irc_channel.cpp     # Channel management
    ├── irc_command.hpp     # Command declarations
    ├── irc_command.cpp     # IRC command processing
    ├── irc_message.hpp     # Message parsing declarations
    ├── irc_message.cpp     # IRC protocol message parsing
    ├── utils.hpp           # Utility declarations
    └── osx/                # Platform-specific
        └── os.cpp
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
| `irc_server.cpp` | `Server` | ソケット作成, bind, listen, accept, poll/select |
| `irc_client.cpp` | `Client` | クライアント接続管理, 認証, 切断処理 |
| `irc_channel.cpp` | `Channel` | チャンネル作成, 参加, 退出, モード管理 |

### Protocol Modules

| File | Namespace | Description |
|------|-----------|-------------|
| `irc_message.cpp` | `Message` | IRCプロトコルメッセージのパース |
| `irc_command.cpp` | `Command` | IRCコマンド実行 (NICK, JOIN, etc.) |

---

## Namespace Design

```cpp
// ft_irc.hpp - Main header
namespace Global {
    extern const std::string Version;
}

// irc_server.hpp
namespace Server {
    class IRCServer {
    public:
        IRCServer(int port, const std::string& password);
        void run();
        void stop();
    private:
        int _port;
        std::string _password;
        int _socket_fd;
        std::vector<Client*> _clients;
        std::map<std::string, Channel*> _channels;
    };
}

// irc_client.hpp
namespace Client {
    class IRCClient {
    public:
        IRCClient(int fd);
        bool authenticate(const std::string& password);
        void send(const std::string& message);
        std::string receive();
    private:
        int _fd;
        std::string _nickname;
        std::string _username;
        bool _authenticated;
        std::string _buffer;
    };
}

// irc_channel.hpp
namespace Channel {
    class IRCChannel {
    public:
        IRCChannel(const std::string& name);
        void join(Client::IRCClient* client);
        void part(Client::IRCClient* client);
        void broadcast(const std::string& message, Client::IRCClient* sender);
    private:
        std::string _name;
        std::string _topic;
        std::vector<Client::IRCClient*> _members;
        std::vector<Client::IRCClient*> _operators;
    };
}

// irc_message.hpp
namespace Message {
    struct IRCMessage {
        std::string prefix;
        std::string command;
        std::vector<std::string> params;
    };

    IRCMessage parse(const std::string& raw);
    std::string build(const IRCMessage& msg);
}

// irc_command.hpp
namespace Command {
    void execute(Server::IRCServer& server,
                 Client::IRCClient& client,
                 const Message::IRCMessage& msg);

    // Individual command handlers
    void cmd_pass(Server::IRCServer& server, Client::IRCClient& client, const std::vector<std::string>& params);
    void cmd_nick(Server::IRCServer& server, Client::IRCClient& client, const std::vector<std::string>& params);
    void cmd_user(Server::IRCServer& server, Client::IRCClient& client, const std::vector<std::string>& params);
    void cmd_join(Server::IRCServer& server, Client::IRCClient& client, const std::vector<std::string>& params);
    void cmd_part(Server::IRCServer& server, Client::IRCClient& client, const std::vector<std::string>& params);
    void cmd_privmsg(Server::IRCServer& server, Client::IRCClient& client, const std::vector<std::string>& params);
    void cmd_kick(Server::IRCServer& server, Client::IRCClient& client, const std::vector<std::string>& params);
    void cmd_invite(Server::IRCServer& server, Client::IRCClient& client, const std::vector<std::string>& params);
    void cmd_topic(Server::IRCServer& server, Client::IRCClient& client, const std::vector<std::string>& params);
    void cmd_mode(Server::IRCServer& server, Client::IRCClient& client, const std::vector<std::string>& params);
    void cmd_quit(Server::IRCServer& server, Client::IRCClient& client, const std::vector<std::string>& params);
}
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
   - [ ] `irc_server.cpp` - Socket creation, bind, listen
   - [ ] `irc_client.cpp` - Client accept, fd management

2. **Phase 2: Protocol**
   - [ ] `irc_message.cpp` - Message parsing
   - [ ] `irc_command.cpp` - Command dispatcher

3. **Phase 3: Authentication**
   - [ ] `cmd_pass()` - Password verification
   - [ ] `cmd_nick()` - Nickname handling
   - [ ] `cmd_user()` - User registration

4. **Phase 4: Channels**
   - [ ] `irc_channel.cpp` - Channel management
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
    const char* BOLD_GREEN     = "\033[1;32m";
    const char* BOLD_YELLOW    = "\033[1;33m";
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
