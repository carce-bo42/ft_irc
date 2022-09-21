#ifndef IRC42_AIRCCOMMANDS_H
# define IRC42_AIRCCOMMANDS_H

#include "FdManager.hpp"
#include "IrcDataBase.hpp"

// A stands for Abstract

/* 
 * Esta clase sirve para abstraer las implementaciones de comandos de la 
 * interfaz del servidor.
 * Un Server, entonces, solo tendrá que heredar de AIrcCommands y llamar
 * a loadCommands() con los comandos que se quieran habilitar. 
 *
 */

namespace irc {

class Command;

class AIrcCommands : public FdManager,
                     public IrcDataBase
{
    public:
    AIrcCommands(void);
    AIrcCommands(std::string &password);
    AIrcCommands(std::string &ip, std::string &port);
    AIrcCommands(std::string &ip, std::string &port, std::string &password);
    AIrcCommands(const AIrcCommands &other);
    ~AIrcCommands();
    
    /* Provided by Server */
    std::string password; // = 0 is C++11 :(
    virtual bool serverHasPassword(void) = 0;
    virtual void maybeRegisterUser(User &user) = 0;
    virtual void registerUser(User &user) = 0;

    virtual void DataFromUser(int fd) = 0;
    virtual void DataToUser(int fd, std::string data, int type) = 0;
    virtual void loadCommandMap(void) = 0;

    /* Command implementations */
    void NICK(Command &cmd, int fd);
    void USER(Command &cmd, int fd);
    void PING(Command &cmd, int fd);
    void PONG(Command &cmd, int fd);
    void MODE(Command &cmd, int fd);
    void PASS(Command &cmd, int fd);
    void QUIT(Command &cmd, int fd);
    void JOIN(Command &cmd, int fd);
    void KICK(Command &cmd, int fd);
    void PART(Command &cmd, int fd);
    void TOPIC(Command &cmd, int fd);
    void INVITE(Command &cmd, int fd);
    void NAMES(Command &cmd, int fd);
    void LIST(Command &cmd, int fd);
    void PRIVMSG(Command &cmd, int fd);
    void WHOIS(Command &cmd, int fd);

    /* Common replies  ? todas privadas ?*/
    void sendWelcome(std::string& name, std::string &prefix, int fd);
    void sendNeedMoreParams(std::string &nick, std::string& cmd_name, int fd);
    void sendParamNeeded(std::string &nick, std::string &ch_name,
                         std::string mode, std::string mode_msg, int fd);
    void sendNotRegistered(std::string &nick, std::string &cmd_name, int fd);
    void sendNoSuchChannel(std::string &nick, std::string &ch_name, int fd);
    void sendNotOnChannel(std::string &nick, std::string &ch_name, int fd);
    void sendBadChannelMask(std::string &nick, std::string &ch_name, int fd);
    void sendNoChannelModes(std::string &cmd_name, int fd);
    void sendChannelOperatorNeeded(std::string &nick,
                                   std::string &ch_name, int fd);
    void sendAlreadyRegistered(std::string &nick, int fd);
    void sendPasswordMismatch(std::string &nick, int fd);
    void sendJoinReply(int fd, User &user, Channel &channel, bool send_all);
    void sendNamesReply(int fd, User &user, Channel &channel);
    void sendListReply(int fd, User &user, std::string ch_name);
    void sendPartMessage(std::string &extra_msg, int fd,
                         User &user, Channel &channel);
    void sendNoSuchNick(int fd, std::string nick, std::string notFoundNick);
    void sendChannelModes(int fd, std::string nick, Channel &channel);
    void sendKickMessage(int fd, User &user, Channel &channel,
                         std::string &kicked);
    void sendMessageToChannel(Channel &channel, std::string &message,
                              std::string &nick);
    std::string constructNamesReply(std::string nick, Channel &channel);
    std::string constructListReply(std::string nick, Channel &channel);
    std::string constructWhoisChannelRpl(User &user, std::string &real_nick);
    void createNewChannel(const Command &cmd, int size, User &user, int fd);
    void sendWhoisReply(const Command &cmd, int fd, User &user,
                        std::string &nick);
    void joinExistingChannel(int fd, User &user, Channel &channel);
    void sendBlackListReply(int fd, const User &user, Channel &channel);
    void checkModeToAddOrDelete(const Command &cmd, Channel &channel,
                                User &user, char m, int mode);
    void checkKeyMode(const Command &cmd, Channel &channel, User &user);
    void checkOpMode(const irc::Command &cmd, std::string nick,
                     User &user, Channel &channel, int fd);
    std::string checkAndGetVoiceRpl(const Command &cmd, const User &user,
                                    Channel &channel, const std::string &mode,
                                    User &other) const;
    void sendQuitToAllChannels(int fd, std::string &msg);
    void sendClosingLink(int fd, std::string &reason);
    void removeUserFromServer(int fd, std::string &reason);
};

} // namespace


#endif /* IRC42_AIRCCOMMANDS_H */
