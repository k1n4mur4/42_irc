# ft_irc Development Documentation

## Project Structure

```
ft_irc/
├── Makefile
├── DEV_DOC.md
├── FEAT_IRC.md
├── bin/                    # Output directory
│   └── ircserv
├── obj/                    # Build artifacts
│   ├── config.h            # Generated
│   └── *.o
└── src/                    # Source & Headers
    ├── config.h.in         # Config template
    ├── ft_irc.hpp          # Main header (includes all modules)
    ├── ft_irc.cpp          # Main logic & Global::Version
    ├── main.cpp            # Entry point
    ├── irc_cli.cpp         # CLI parsing
    ├── server.hpp          # Server declarations
    ├── server.cpp          # Server socket, poll loop, client/channel management
    ├── client.hpp          # Client declarations
    ├── client.cpp          # Client buffering, auth state
    ├── channel.hpp         # Channel declarations
    ├── channel.cpp         # Channel member/operator/invite management
    ├── command.hpp         # Command dispatcher declarations
    ├── command.cpp         # IRC command handlers
    ├── message.hpp         # Message parsing declarations
    └── message.cpp         # IRC protocol message parsing/building
└── tests/                 # Automated integration tests
    ├── test.sh            # Test runner
    ├── helpers.sh         # Shared helpers & config
    ├── test_registration.sh
    ├── test_ping.sh
    ├── test_channel.sh
    ├── test_operator.sh
    ├── test_errors.sh
    └── README.md
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

| File | Class | Description |
|------|-------|-------------|
| `server.cpp` | `Server` | ソケット作成, bind, listen, accept, poll, POLLOUT処理, クライアント/チャンネル管理 |
| `client.cpp` | `Client` | recv/sendバッファリング, 認証状態管理, nick!user@host prefix生成 |
| `channel.cpp` | `Channel` | メンバー/オペレーター/招待リスト管理, ブロードキャスト |

### Protocol Modules

| File | Namespace | Description |
|------|-----------|-------------|
| `message.cpp` | `Message` | RFC 1459形式のIRCメッセージパース/ビルド |
| `command.cpp` | `Command` | IRCコマンドディスパッチャー & 全コマンドハンドラー |

---

## Class Design

### Server

```cpp
class Server {
public:
    Server();
    Server(int port, std::string password);

    // Poll loop
    void        ServerInit();
    void        SerSocket();
    void        AcceptNewClient();
    void        ReceiveNewData(int fd);
    void        HandlePollout(int fd);

    static void signalHandler(int signum);

    // Getters/Setters
    void        setPort(int port);
    int         getPort();
    void        setPassword(std::string password);
    std::string getPassword();
    void        closeFds();
    void        clearClients(int fd);

    // Client operations
    void        SendToClient(int fd, const std::string& message);
    void        SendReply(Client& client, const std::string& numeric,
                          const std::string& params);
    Client*     FindClientByFd(int fd);
    Client*     FindClientByNick(const std::string& nick);
    void        DisconnectClient(int fd);

    // Channel operations
    Channel*    FindChannel(const std::string& name);
    Channel*    FindOrCreateChannel(const std::string& name);
    void        RemoveChannel(const std::string& name);
    void        RemoveClientFromAllChannels(Client* client, const std::string& quit_msg);
    void        BroadcastNickChange(Client* client, const std::string& oldPrefix,
                                    const std::string& newNick);

private:
    int                            port_;
    std::string                    password_;
    int                            serSocketFd_;
    static bool                    signal_;
    std::map<int, Client>          clients_;    // key=fd (pointer-stable)
    std::vector<struct pollfd>     fds_;
    std::map<std::string, Channel> channels_;   // key=channel name
};
```

### Client

```cpp
class Client {
public:
    Client();

    // FD / IP
    int         getFd();
    void        setFd(int fd);
    std::string getIpAdd();
    void        setIpAdd(std::string ipadd);

    // Recv/Send buffering
    void        appendRecvBuffer(const std::string& data);
    bool        hasCompleteLine() const;
    std::string extractLine();           // extracts one \r\n or \n delimited line
    void        appendSendBuffer(const std::string& data);
    const std::string& getSendBuffer() const;
    void        eraseSendBuffer(size_t n);
    bool        hasPendingData() const;

    // Authentication
    std::string getNickname() const;
    void        setNickname(const std::string& nick);
    std::string getUsername() const;
    void        setUsername(const std::string& user);
    std::string getRealname() const;
    void        setRealname(const std::string& real);
    bool        isPassReceived() const;
    void        setPassReceived(bool v);
    bool        isRegistered() const;
    void        setRegistered(bool v);
    std::string getPrefix() const;       // nick!user@host

private:
    int         fd_;
    std::string ip_add_;
    std::string recv_buffer_;
    std::string send_buffer_;
    std::string nickname_;
    std::string username_;
    std::string realname_;
    bool        pass_received_;
    bool        registered_;
};
```

### Channel

```cpp
class Channel {
public:
    Channel();
    Channel(const std::string& name);

    // Properties
    const std::string& getName() const;
    const std::string& getTopic() const;
    void        setTopic(const std::string& topic);
    const std::string& getKey() const;
    void        setKey(const std::string& key);
    int         getUserLimit() const;
    void        setUserLimit(int limit);
    bool        isInviteOnly() const;
    void        setInviteOnly(bool v);
    bool        isTopicRestricted() const;
    void        setTopicRestricted(bool v);

    // Member management
    void        AddMember(Client* client);
    void        RemoveMember(Client* client);
    bool        IsMember(Client* client) const;
    int         MemberCount() const;
    const std::vector<Client*>& getMembers() const;

    // Operator management
    void        AddOperator(Client* client);
    void        RemoveOperator(Client* client);
    bool        IsOperator(Client* client) const;

    // Invite management
    void        AddInvited(const std::string& nick);
    bool        IsInvited(const std::string& nick) const;
    void        RemoveInvited(const std::string& nick);

    // Messaging
    void        Broadcast(const std::string& message, Client* exclude);

private:
    std::string              name_;
    std::string              topic_;
    std::string              key_;
    int                      user_limit_;
    bool                     invite_only_;
    bool                     topic_restricted_;
    std::vector<Client*>     members_;
    std::vector<Client*>     operators_;
    std::vector<std::string> invited_;
};
```

### IRCMessage & Namespaces

```cpp
// message.hpp
struct IRCMessage {
    std::string              prefix;
    std::string              command;
    std::vector<std::string> params;
};

namespace Message {
    IRCMessage  parse(const std::string& line);
    std::string build(const std::string& prefix,
                      const std::string& command,
                      const std::vector<std::string>& params);
}

// command.hpp
namespace Command {
    void execute(Server& server, Client& client, const IRCMessage& msg);
}
```

---

## IRC Commands

| Command | Description | Phase | Status |
|---------|-------------|-------|--------|
| `PING` | サーバー接続維持 | 2 | Done |
| `PASS` | 接続パスワード | 3 | Done |
| `NICK` | ニックネーム設定 | 3 | Done |
| `USER` | ユーザー情報設定 | 3 | Done |
| `QUIT` | 切断 | 3 | Done |
| `JOIN` | チャンネル参加 | 4 | Done |
| `PART` | チャンネル退出 | 4 | Done |
| `PRIVMSG` | メッセージ送信 | 4 | Done |
| `KICK` | ユーザー追放 | 5 | Done |
| `INVITE` | ユーザー招待 | 5 | Done |
| `TOPIC` | トピック設定/取得 | 5 | Done |
| `MODE` | モード設定 | 5 | Done |

---

## Registration Flow

1. クライアントが接続 → `AcceptNewClient()` で fd/IP を記録
2. `PASS <password>` → パスワード照合、`pass_received_ = true`
3. `NICK <nickname>` → バリデーション + 重複チェック
4. `USER <username> 0 * :<realname>` → username/realname 設定
5. 3つ全て完了 → `registered_ = true`、001-004 ウェルカムメッセージ送信
6. 未登録クライアントは PASS/NICK/USER/QUIT/PING 以外を 451 (ERR_NOTREGISTERED) で拒否

---

## Channel Modes

| Mode | Description | Parameter |
|------|-------------|-----------|
| `i` | Invite-only | - |
| `t` | Topic restriction (operator only) | - |
| `k` | Channel key (password) | `<key>` (set) / - (unset) |
| `o` | Operator privilege | `<nick>` |
| `l` | User limit | `<limit>` (set) / - (unset) |

MODE supports multiple flags in one command: `MODE #ch +itk-o key nick`

---

## Numeric Replies

| Numeric | Name | Used In |
|---------|------|---------|
| 001 | RPL_WELCOME | Registration |
| 002 | RPL_YOURHOST | Registration |
| 003 | RPL_CREATED | Registration |
| 004 | RPL_MYINFO | Registration |
| 324 | RPL_CHANNELMODEIS | MODE query |
| 331 | RPL_NOTOPIC | JOIN, TOPIC |
| 332 | RPL_TOPIC | JOIN, TOPIC |
| 341 | RPL_INVITING | INVITE |
| 353 | RPL_NAMREPLY | JOIN |
| 366 | RPL_ENDOFNAMES | JOIN |
| 401 | ERR_NOSUCHNICK | PRIVMSG, INVITE |
| 403 | ERR_NOSUCHCHANNEL | JOIN, PART, PRIVMSG, KICK, INVITE, TOPIC, MODE |
| 404 | ERR_CANNOTSENDTOCHAN | PRIVMSG |
| 411 | ERR_NORECIPIENT | PRIVMSG |
| 412 | ERR_NOTEXTTOSEND | PRIVMSG |
| 431 | ERR_NONICKNAMEGIVEN | NICK |
| 432 | ERR_ERRONEUSNICKNAME | NICK |
| 433 | ERR_NICKNAMEINUSE | NICK |
| 441 | ERR_USERNOTINCHANNEL | KICK, MODE +o/-o |
| 442 | ERR_NOTONCHANNEL | PART, KICK, INVITE, TOPIC, MODE |
| 443 | ERR_USERONCHANNEL | INVITE |
| 451 | ERR_NOTREGISTERED | Pre-registration guard |
| 461 | ERR_NEEDMOREPARAMS | PASS, USER, JOIN, PART, KICK, INVITE, TOPIC, MODE |
| 462 | ERR_ALREADYREGISTRED | PASS, USER |
| 464 | ERR_PASSWDMISMATCH | PASS |
| 471 | ERR_CHANNELISFULL | JOIN (+l) |
| 472 | ERR_UNKNOWNMODE | MODE |
| 473 | ERR_INVITEONLYCHAN | JOIN (+i) |
| 475 | ERR_BADCHANNELKEY | JOIN (+k) |
| 482 | ERR_CHANOPRIVSNEEDED | KICK, INVITE (+i), TOPIC (+t), MODE |

---

## Data Flow

```
recv() → Client::recv_buffer_ → extractLine() → Message::parse()
       → Command::execute() → handler → Client::send_buffer_
       → poll(POLLOUT) → send()
```

All I/O goes through `poll()` — no direct send/recv outside the poll loop (42 requirement).

Partial data handling: `recv()` appends to `recv_buffer_`; lines are extracted only when `\r\n` or `\n` is found. This satisfies the `nc` + `Ctrl+D` fragmentation test.

---

## Implementation Order

1. **Phase 1: Server Foundation** — Done
   - [x] `server.cpp` - Socket creation, bind, listen, accept, poll loop
   - [x] `client.cpp` - Client fd/IP management

2. **Phase 2: Protocol Foundation** — Done
   - [x] `clients_` を `std::map<int, Client>` に変更 (pointer stability)
   - [x] `client.cpp` - recv/send バッファリング
   - [x] `message.cpp` - RFC 1459 メッセージパーサー/ビルダー
   - [x] `command.cpp` - コマンドディスパッチャー, PING/PONG
   - [x] `server.cpp` - POLLOUT 対応, SendToClient, HandlePollout

3. **Phase 3: Authentication** — Done
   - [x] `client.cpp` - 認証メンバー (nickname, username, realname, pass_received, registered)
   - [x] `cmd_pass()` - パスワード照合
   - [x] `cmd_nick()` - ニックネームバリデーション + 重複チェック
   - [x] `cmd_user()` - ユーザー情報設定
   - [x] `cmd_quit()` - 切断処理
   - [x] `server.cpp` - SendReply, FindClientByNick, DisconnectClient

4. **Phase 4: Channels** — Done
   - [x] `channel.cpp` - Channel クラス (メンバー/オペレーター/招待管理)
   - [x] `cmd_join()` - チャンネル作成/参加 (+i/+k/+l チェック, NAMES応答)
   - [x] `cmd_part()` - チャンネル退出
   - [x] `cmd_privmsg()` - チャンネル/DM メッセージルーティング
   - [x] `server.cpp` - channels_ 管理, RemoveClientFromAllChannels, BroadcastNickChange

5. **Phase 5: Operators** — Done
   - [x] `cmd_kick()` - ユーザー追放
   - [x] `cmd_invite()` - ユーザー招待
   - [x] `cmd_topic()` - トピック設定/取得 (+t 制限)
   - [x] `cmd_mode()` - +i, +t, +k, +o, +l (複数モード同時変更対応)

---

## Testing

### Automated Test Suite (24 tests)

```bash
bash tests/test.sh                  # 全テスト実行
bash tests/test_registration.sh     # 個別実行も可
```

詳細は `tests/README.md` を参照。

### nc (netcat) テスト

```bash
# Build & run
make re && ./bin/ircserv 6667 pass

# Basic registration
nc -C localhost 6667
PASS pass
NICK testuser
USER testuser 0 * :Test User
# → 001-004 ウェルカムメッセージ

# Partial data (Ctrl+D fragmentation)
nc localhost 6667
com^Dman^Dd\n
# → バッファリングされて完全行として処理される

# Error cases
PASS wrong    # → 464
NICK @bad     # → 432
```

### irssi テスト

```bash
# Single client
irssi -c localhost -p 6667 -w pass
/join #test
/msg #test hello
/part #test
/msg othernick hello

# Two clients for operator commands
# Client 1 (operator):
/join #test
/mode #test +i
/invite user2 #test
/kick #test user2
/topic #test New Topic
/mode #test +k secret
/mode #test +l 5
/mode #test +o user2
```

---

## Color Constants

```cpp
// server.hpp macros
#define RED "\033[1;31m"
#define WHI "\033[0;37m"
#define GRE "\033[1;32m"
#define YEL "\033[1;33m"

// irc_cli.cpp
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
