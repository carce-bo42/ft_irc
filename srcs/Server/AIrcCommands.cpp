#include "Server/AIrcCommands.hpp"
#include "Tools.hpp"
#include "Command.hpp"
#include "Types.hpp"
#include "User.hpp"
#include "NumericReplies.hpp"
#include "Log.hpp"
#include "Channel.hpp"

#include "libft.h"

#include <map>

using std::string;
using std::map;

namespace irc {

AIrcCommands::AIrcCommands(void)
:
    FdManager(),
    IrcDataBase()
{}

AIrcCommands::AIrcCommands(string &password)
:
    FdManager(),
    IrcDataBase(),
    password(password)
{}

AIrcCommands::AIrcCommands(string &hostname, string &port)
:
    FdManager(hostname, port),
    IrcDataBase()
{}

AIrcCommands::AIrcCommands(string &hostname, string &port, string &password)
:
    FdManager(hostname, port),
    IrcDataBase(),
    password(password)
{}

AIrcCommands::AIrcCommands(const AIrcCommands& other)
:
    FdManager(other),
    IrcDataBase(other),
    password(other.password)
{}

AIrcCommands::~AIrcCommands()
{}


/**
 * Command: NICK
 * Parameters: <nickname>
 */
void AIrcCommands::NICK(Command &cmd, int fd) {

    User &user = getUserFromFd(fd);
    int size = cmd.args.size();
    
    /* case too many params */
    if (size > 2) {
        return ;
    }
    /* case no nickname */
    if (size < 2) {
        string unknown_user = "*";
        string nick = nickExists(user.nick) ? user.real_nick : unknown_user;
        return sendNeedMoreParams(nick, cmd.Name(), fd);
    }
    string real_nick = cmd.args[1];
    if (real_nick[0] == ':') {
        real_nick = real_nick.substr(1); // ignore : start.
    }
    string nick = real_nick; // nick is always upper case to ease lookup
    tools::ToUpperCase(nick);
    /* case forbidden characters are found / incorrect length */
    if (nickFormatOk(real_nick) == false) {
        string reply(ERR_ERRONEUSNICKNAME
                     + real_nick
                     + STR_ERRONEUSNICKNAME);
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    /* case nickname is equal to some other in the server
     * (ignoring upper/lower case) */
    if (nickExists(nick)) {
        string reply(ERR_NICKNAMEINUSE
                     + real_nick
                     + STR_NICKNAMEINUSE);
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    /* case nickname change */
    if (user.isResgistered()) {
        // Notify channels of nickname change
        for (map<string, unsigned char>::iterator
             it = user.ch_name_mask_map.begin();
             it != user.ch_name_mask_map.end(); it++)
        {
            string reply = ":" + user.prefix + " "
                            + cmd.Name() + " :"
                            + real_nick;
            string ch_name = it->first;
            Channel &channel = getChannelFromName(ch_name);
            sendMessageToChannel(channel, reply, user.nick);
        }
        return updateUserNick(fd, nick, real_nick);
    }
    /* case the nickname is the first recieved from this user */
    user.nick = nick;
    user.real_nick = real_nick;
    addNickFdPair(nick, fd);
    /* case NICK is recieved before valid USER comand */
    return maybeRegisterUser(user);
}

/**
 * Command: USER
 * Parameters: <username> 0 * <realname>
 */
void AIrcCommands::USER(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);
    int size = cmd.args.size();

    /* case many params (from irc-hispano) */
    if (size > 5) {
        return ;
    }
    /* case arguments unsufficient */
    if (size < 5) {
        string unknown_user = "*";
        string nick = nickExists(user.nick) ? user.real_nick : unknown_user;
        return sendNeedMoreParams(nick, cmd.Name(), fd);
    }
    /* case user already sent a valid USER comand */
    if (user.isResgistered()) {
        return sendAlreadyRegistered(user.real_nick, fd);
    }
    /* wether nick exists or not, name and full name should be saved */
    user.name = cmd.args[1];
    string full_name = cmd.args[size - 1];
    if (full_name[0] == ':') {
        full_name = full_name.substr(1);
    }
    user.full_name = full_name;
    return maybeRegisterUser(user);
}

/**
 * Command: PASS
 * Parameters: <password>
 * If password is set on the server, users must provide it with
 * this command to be registered. The password used for registration is
 * the one correpsonding to the last valid PASS command recieved.
 */
void AIrcCommands::PASS(Command &cmd, int fd) {

    User &user = getUserFromFd(fd);
    int size = cmd.args.size();

    /* case sending PASS command to server without password set, ignore */
    if (!serverHasPassword()) {
        return ;
    }
    if (user.registered) {
        return sendAlreadyRegistered(user.real_nick, fd);
    }
    if (size < 2) {
        string unknown_user = "*";
        return sendNeedMoreParams(unknown_user, cmd.Name(), fd);
    }
    user.last_password = cmd.args[1];
    return maybeRegisterUser(user);
}


/*
 * Command: PING
 * Parameters: <token> 
 */
void AIrcCommands::PING(Command &cmd, int fd) {
    
    int size = cmd.args.size();
    User& user = getUserFromFd(fd);

    if (size < 2) {
        return sendNeedMoreParams(user.real_nick, cmd.Name(), fd);
    }
    if (!user.isResgistered()) {
        string nick = nickExists(user.nick) ? user.real_nick : "*";
        return sendNotRegistered(nick, cmd.Name(), fd);
    }
    string pong_reply("PONG " + cmd.args[1]);
    DataToUser(fd, pong_reply, NO_NUMERIC_REPLY);
}

/*
 * Command: PONG
 * Parameters: [<server>] <token>
 */
void AIrcCommands::PONG(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);
    int size = cmd.args.size();

    if (size < 2) {
        return sendNeedMoreParams(user.real_nick, cmd.Name(), fd);
    }
    /* case pong is recieved wihtout previous PING (random pong) */
    if (user.ping_str.empty()) {
        return ;
    }
    if (user.ping_str.compare(cmd.args[1]) == 0) {
        user.resetPingStatus();
        LOG(INFO) << "PING from user " << user << " correct";
    } else {
        LOG(INFO) << "PING from user " << user << " incorrect, sent "
                   << user.ping_str << " recieved " << cmd.args[1];
    }
    return ;
}

/**
 * Command: JOIN
 * Parameters: <channel> [<key>]
 * 1. If command JOIN has no arguments, error message is returned
 * 2. If channel mask is missing, bad mask error is returned
 * 3. If channel doesn't exist, create new channel and new User with
 *    operator channel_mode and add it to the channel_map
 * 4. If channel exists, check channel_mode
 * 4.1. If channel is in channel_mode 'i', check if user is invited
 *      to join it. If not, error message is returned
 * 4.2. If channel is in channel_mode 'K', check if user is providing
 *      the correct pw. If not, error message is returned
 * 5. Add user to channel
 * 6. Send replies
 */
void AIrcCommands::JOIN(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);
    int size = cmd.args.size();

    if (!user.isResgistered()) {
        string nick = nickExists(user.nick) ? user.real_nick : "*";
        return sendNotRegistered(nick, cmd.Name(), fd);
    }
    if (size < 2) {
        return sendNeedMoreParams(user.real_nick, cmd.Name(), fd);
    }
    string ch_name = cmd.args[1];
    if (!tools::starts_with_mask(ch_name)) {
        return sendBadChannelMask(user.real_nick,ch_name, fd);
    }
    if (!channelExists(cmd.args[1])) {
        return (createNewChannel(cmd, size, user, fd));
    }
    Channel &channel = getChannelFromName(ch_name);
    if (channel.userIsInChannel(user.nick)) {
        return ;
    }
    if (channel.banModeOn()
        && (channel.userInBlackList(user.real_nick, user.ip_address)
            || channel.all_banned)
        && !channel.isUserOperator(user))
    {
        string reply = (ERR_BANNEDFROMCHAN
                        + user.real_nick + " "
                        + channel.name
                        + STR_BANNEDFROMCHAN);
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    if (channel.inviteModeOn()
        && !channel.isInvited(user.nick))
    {
        string reply(ERR_INVITEONLYCHAN
                     + user.real_nick + " "
                     + channel.name
                     + STR_INVITEONLYCHAN);
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    if (channel.keyModeOn()
        && (size == 2
            || channel.key.compare(cmd.args[2]) != 0))
    {
        string reply(ERR_BADCHANNELKEY
                     + user.real_nick + " "
                     + channel.name
                     + STR_BADCHANNELKEY);
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    joinExistingChannel(fd, user, channel);
}

/**
 * Command: PART
 * Parameters: <channel> [<partMessage>]
 * 1. If command PART has no arguments, error message is returned
 * 2. If channel mask is missing, bad mask error is returned
 * 3. In each channel, check if it exists and if user belongs to channel
 * 4. Delete user in channel
 * 5. If channel has no users left, destroy channel
 * 6. Send part or default message to channel
 */
void AIrcCommands::PART(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);
    int size = cmd.args.size();
    
    if (!user.isResgistered()) {
        string nick = nickExists(user.nick) ? user.real_nick : "*";
        return sendNotRegistered(nick, cmd.Name(), fd);
    }
    if (size < 2) {
        return sendNeedMoreParams(user.real_nick, cmd.Name(), fd);
    }
    if (!tools::starts_with_mask(cmd.args[1])) {
        return sendBadChannelMask(user.real_nick, cmd.args[1], fd);
    }
    if (!channelExists(cmd.args[1])) {
        return sendNoSuchChannel(user.real_nick, cmd.args[1], fd);
    }
    Channel &channel = channel_map.find(cmd.args[1])->second;
    if (!channel.userIsInChannel(user.nick)) {
        return sendNotOnChannel(user.real_nick, channel.name, fd);
    }
    channel.deleteUser(user);
    user.ch_name_mask_map.erase(channel.name);
    if (size == 3) {
        return sendPartMessage(cmd.args[2], fd, user, channel);
    }
    string no_message = "";
    sendPartMessage(no_message, fd, user, channel);
    maybeRemoveChannel(channel);
}

/**
 * Command: TOPIC
 * Parameters: <channel> [<topic>]
 * 1. If command TOPIC has no arguments, error message is returned
 * 2. If channel doesn't start with mask, bad mask error is returned
 * 3. If command TOPIC has one argument, channel topic is returned
 *    if exists (channel, user in channel & topic)
 * 4. If command TOPIC has two arguments, channel topic is set
 *    (if user has the requested permissions)
 * 5. If command TOPIC has two arguments, and topic is and empty
 *    string, the channel topic is removed
 * 6. Return RLP_TOPIC to client
 *  If not, error message is returned
 */
void AIrcCommands::TOPIC(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);
    int size = cmd.args.size();

    if (!user.isResgistered()) {
        string nick = nickExists(user.nick) ? user.real_nick : "*";
        return sendNotRegistered(nick, cmd.Name(), fd);
    }
    if (size < 2) {
        return sendNeedMoreParams(user.real_nick, cmd.Name(), fd);
    }
    if (!tools::starts_with_mask(cmd.args[1])) {
        return sendBadChannelMask(user.real_nick, cmd.args[1], fd);
    }
    if (!channelExists(cmd.args[1])) {
        return sendNoSuchChannel(user.real_nick, cmd.args[1], fd);
    }
    Channel &channel = channel_map.find(cmd.args[1])->second;
    if (!channel.userIsInChannel(user.nick)) {
        return sendNotOnChannel(user.real_nick, channel.name, fd);
    }
    if (!channel.topicModeOn()) {
        return sendNoChannelModes(cmd.Name(), fd);
    }
    if (size == 2) {
        if (channel.topic.empty()) {
            string reply(RPL_NOTOPIC
                         + user.real_nick + " "
                         + channel.name
                         + STR_NOTOPIC);
            return DataToUser(fd, reply, NUMERIC_REPLY);
        }
        string reply(RPL_TOPIC
                     + user.real_nick + " "
                     + channel.name + " :"
                     + channel.topic);
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    if (!channel.isUserOperator(user)) {
        return sendChannelOperatorNeeded(user.real_nick, channel.name, fd);
    }
    if (size == 3) {
        channel.topic = (cmd.args[2][0] == ':')
                        ? cmd.args[2].substr(1)
                        : cmd.args[2];
        string reply(user.prefix + " "
                     + cmd.Name()+ " "
                     + channel.name + " :"
                     + channel.topic);
        return DataToUser(fd, reply, NO_NUMERIC_REPLY);
    }
    for (int i = 2; i < size; i++) {
        channel.topic += cmd.args[i];
        channel.topic += i < size - 1 ? " " : "";
    }
    string reply(user.prefix + " "
                 + cmd.Name()+ " "
                 + channel.name + " :"
                 + channel.topic);
    DataToUser(fd, reply, NO_NUMERIC_REPLY);
}

/**
 * Command: KICK
 * Parameters: <channel> <user> [<comment>]
 * 1. Check correct parameter size & if user and channel exist
 * 1. Check if user has the correct permissions to do kick another user
 * 2. Find the user to kick in the channel, if it exists
 * 3. Reuse the PART method passing the comment as the part message
 */
void AIrcCommands::KICK(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);

    if (!user.isResgistered()) {
        string nick = nickExists(user.nick) ? user.real_nick : "*";
        return sendNotRegistered(nick, cmd.Name(), fd);
    }
    int size = cmd.args.size();
    if (size < 3) {
        return sendNeedMoreParams(user.real_nick, cmd.Name(), fd);
    }
    if (!tools::starts_with_mask(cmd.args[1])) {
        return sendBadChannelMask(user.real_nick, cmd.args[1], fd);
    }
    if (!channelExists(cmd.args[1])) {
        return sendNoSuchChannel(user.real_nick, cmd.args[1], fd);
    }
    Channel &channel = channel_map.find(cmd.args[1])->second;
    if (!channel.userIsInChannel(user.nick)) {
        return sendNotOnChannel(user.real_nick, channel.name, fd);
    }
    if (!channel.isUserOperator(user)) {
        return sendChannelOperatorNeeded(user.real_nick, channel.name, fd);
    }
    string nick = cmd.args[2];
    tools::ToUpperCase(nick);
    if (!channel.userIsInChannel(nick)) {
        string reply = ERR_USERNOTINCHANNEL
                       + user.real_nick + " "
                       + cmd.args[2] + " "
                       + channel.name
                       + STR_USERNOTINCHANNEL;
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    User& user_to_kick = getUserFromNick(nick);
    sendKickMessage(fd, user, channel, user_to_kick.real_nick);
    channel.deleteUser(user_to_kick);
    user_to_kick.ch_name_mask_map.erase(channel.name);
    maybeRemoveChannel(channel);
}

/**
 * Command: INVITE
 * Parameters: <nickname> <channel>
 * 1. Check correct parameter size & if user and channel exist
 * 1. Check if user has the correct permissions to do kick another user
 * 2. Find the user to kick in the channel, if it exists
 * 3. Reuse the PART method passing the comment as the part message
 */
void AIrcCommands::INVITE(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);
        int size = cmd.args.size();

    if (!user.isResgistered()) {
        string nick = nickExists(user.nick) ? user.real_nick : "*";
        return sendNotRegistered(nick, cmd.Name(), fd);
    }
    if (size < 3) {
        return sendNeedMoreParams(user.real_nick, cmd.Name(), fd);
    }
    if (!channelExists(cmd.args[2])) {
        return sendNoSuchChannel(user.real_nick, cmd.args[2], fd);
    }
    Channel &channel = channel_map.find(cmd.args[2])->second;
    if (!channel.userIsInChannel(user.nick)) {
        return sendNotOnChannel(user.real_nick, channel.name, fd);
    }
    if (!channel.isUserOperator(user)) {
        return sendChannelOperatorNeeded(user.real_nick, channel.name, fd);
    }
    string nick = cmd.args[1];
    tools::ToUpperCase(nick);
    if (!nick_fd_map.count(nick)) {
        return sendNoSuchNick(fd, user.real_nick, cmd.args[1]);
    }
    if (channel.userIsInChannel(nick)) {
        string reply = ERR_USERONCHANNEL
                       + user.real_nick + " "
                       + cmd.args[1] + " "
                       + channel.name
                       + STR_USERONCHANNEL;
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    channel.addToWhitelist(nick);
    string invite_msg = ":" + user.prefix + " INVITE "
                         + cmd.args[1] + " :"
                         + channel.name;
    DataToUser(getUserFromNick(nick).fd, invite_msg, NO_NUMERIC_REPLY);
    string invite_rpl = RPL_INVITING
                        + user.real_nick + " "
                        + cmd.args[1] + " :"
                        + channel.name;
    DataToUser(fd, invite_rpl, NUMERIC_REPLY);
}

/**
 * Command: MODE
 * Parameters: <channel> <channelMode> [<modeParams>]
 */
void AIrcCommands::MODE(Command &cmd, int fd) {

    User& user = getUserFromFd(fd);
    int size = cmd.args.size();
    
    if (!user.isResgistered()) {
        string nick = nickExists(user.nick) ? user.real_nick : "*";
        return sendNotRegistered(nick, cmd.Name(), fd);
    }
    if (size < 2) {
        return sendNeedMoreParams(user.real_nick, cmd.Name(), fd);
    }
    // CHANGE CHANNEL MODE
    // filter out user modes, we do not implement them outside channels.
    if (!tools::starts_with_mask(cmd.args[1])) {
        return ;
    }
    if (!channelExists(cmd.args[1])) {
        return sendNoSuchChannel(user.real_nick, cmd.args[1], fd);
    }
    Channel &channel = channel_map.find(cmd.args[1])->second;
    if (!channel.userIsInChannel(user.nick)) {
        return sendNotOnChannel(user.real_nick, channel.name, fd);
    }
    if (size == 2) {
        return sendChannelModes(fd, user.real_nick, channel);
    }
    if (!channel.isUserOperator(user)) {
        return sendChannelOperatorNeeded(user.real_nick, channel.name, fd);
    }
    string mode = cmd.args[2];
    if (tools::anyRepeatedChar(mode)
        || tools::hasUnknownChannelFlag(mode))
    {
        string reply = ERR_UNKNOWNMODE
                        + user.real_nick + " "
                        + mode
                        + STR_UNKNOWNMODE;
        return DataToUser(fd, reply, NUMERIC_REPLY);
    }
    checkModeToAddOrDelete(cmd, channel, user, 'i', CH_INV);
    checkModeToAddOrDelete(cmd, channel, user, 'm', CH_MOD);
    if (tools::charIsInString(mode, 'k')) {
        if (size < 4) {
            return sendParamNeeded(user.real_nick, channel.name, " k *",
                                    "key mode. Syntax <key>", fd);
        }
        if (tools::charIsInString(mode, '-')
            && cmd.args[3].compare(channel.key)
            && !channel.key.empty())
        {
            string reply = ERR_KEYSET
                            + user.real_nick + " "
                            + channel.name
                            + STR_KEYSET;
            return DataToUser(fd, reply, NUMERIC_REPLY);
        }
        checkKeyMode(cmd, channel, user);
    }
    if (tools::charIsInString(mode, 'o')) {
        if (size < 4) {
            return sendParamNeeded(user.real_nick, channel.name, " o *",
                                    "op mode. Syntax: <nick>", fd);
        }
        string nick = cmd.args[3];
        tools::ToUpperCase(nick);
        if (!nick_fd_map.count(nick)) {
            return sendNoSuchNick(fd, user.real_nick, cmd.args[3]);
        }
        if (channel.userIsInChannel(nick)) {
            checkOpMode(cmd, nick, user, channel, fd);
        }
    }
    if (tools::charIsInString(mode, 'b')) {
        if (tools::charIsInString(mode, '+')) {
            if (size == 3) {
                return sendBlackListReply(fd, user, channel);
            }
            if (size == 4) {
                string ban_nick = ft_isdigit(cmd.args[3][0])
                                  ? "*"
                                  : cmd.args[3];
                string ip = (ft_isdigit(cmd.args[3][0])
                             || cmd.args[3].find('@') != string::npos)
                             ? cmd.args[3]
                             : "*";
                if (!channel.banModeOn()) {
                    channel.addMode(CH_BAN);
                }
                // check if already banned
                if (!channel.userInBlackList(ban_nick, ip)) {
                    string ban_mask = (ban_nick.find("@") == string::npos)
                                       ? ban_nick + "!*@" + ip
                                       : ban_nick;
                    channel.banUser(ban_mask, user.fd);
                    string mode_rpl = ":" + user.prefix
                                      + " MODE "
                                      + cmd.args[1] + " +b "
                                      + cmd.args[3];
                    return sendMessageToChannel(channel, mode_rpl,
                                                user.real_nick);
                }
            }
        }
        if (tools::charIsInString(mode, '-')
            && size == 4)
        {
            string user_to_unban = cmd.args[3];
            if (!channel.unbanUser(user_to_unban)) {
                string ban_rpl = ERR_NOSUCHBAN
                                 + user.real_nick + " "
                                 + channel.name
                                 + STR_NOSUCHBAN
                                 + user_to_unban;
                return DataToUser(fd, ban_rpl, NUMERIC_REPLY);
            }
            if (channel.black_list.empty()) {
                channel.deleteMode(CH_BAN);
            }
            string mode_rpl = ":" + user.prefix
                              + " MODE "
                              + cmd.args[1] + " -b "
                              + user_to_unban;
            return sendMessageToChannel(channel, mode_rpl, user.real_nick);
        }
    }
    if (tools::charIsInString(mode, 'v')) {
        if (size < 4) {
            return sendParamNeeded(user.real_nick, channel.name, " v *",
                                   "voice mode. Syntax: <nick>", fd);
        }
        string nick = cmd.args[3];
        tools::ToUpperCase(nick);
        User &other = getUserFromNick(nick);
        if (!other.isResgistered()) {
            return sendNoSuchNick(fd, user.real_nick, cmd.args[3]);
        }
        string mode_rpl = checkAndGetVoiceRpl(cmd, user, channel, mode, other);
        sendMessageToChannel(channel, mode_rpl, user.nick);
        return DataToUser(fd, mode_rpl, NO_NUMERIC_REPLY);
    }
}

/**
 * Command: QUIT
 * Parameters: [<quitMessage>]
 * The server aknowledges this by sending an error client to the server
 */
void AIrcCommands::QUIT(Command &cmd, int fd) {

    User &user = getUserFromFd(fd);
    int size = cmd.args.size();
    
    if (!user.isResgistered()) {
        string nick = nickExists(user.nick) ? user.real_nick : "*";
        return sendNotRegistered(nick, cmd.Name(), fd);
    }
    string message = "Client exited";
    if (size == 2) {
        message = "Quit: ";
        message += cmd.args[1][0] == ':'
                   ? cmd.args[1].substr(1)
                   : cmd.args[1];
    }
    removeUserFromServer(fd, message);
}

/**
 * Command: NAMES
 * Parameters: [<channel>]
 * Lists users visible to client or users in a channel
 */
void AIrcCommands::NAMES(Command &cmd, int fd) {

    User &user = getUserFromFd(fd);
    int size = cmd.args.size();

    if (!user.isResgistered()) {
        string nick = nickExists(user.nick) ? user.real_nick : "*";
        return sendNotRegistered(nick, cmd.Name(), fd);
    }
    if (size == 2) {
        if (!channelExists(cmd.args[1])) {
            return sendNoSuchChannel(user.real_nick, cmd.args[1], fd);
        }
        Channel &channel = channel_map.find(cmd.args[1])->second;
        return sendNamesReply(fd, user, channel);
    }
    string names_reply = RPL_ENDOFNAMES
                         + user.real_nick + " *"
                         + STR_ENDOFNAMES;
    DataToUser(fd, names_reply, NUMERIC_REPLY);
}

/**
 * Command: LIST
 * Parameters: [<channel>]
 * Lists channel size & modes
 * */
void AIrcCommands::LIST(Command &cmd, int fd) {

    User &user = getUserFromFd(fd);
    int size = cmd.args.size();

    if (!user.isResgistered()) {
        string nick = nickExists(user.nick) ? user.real_nick : "*";
        return sendNotRegistered(nick, cmd.Name(), fd);
    }
    if (size == 1) {
        sendListReply(fd, user, "");
    }
    if (size == 2) {
        if (!channelExists(cmd.args[1])) {
            return sendNoSuchChannel(user.real_nick, cmd.args[1], fd);
        }
        sendListReply(fd, user, cmd.args[1]);
    }
}

/**
 * Command: PRIVMSG
 * Parameters: [<channel/user>] <message>
 * Lists channel size & modes
 */
void AIrcCommands::PRIVMSG(Command &cmd, int fd) {

    User &user = getUserFromFd(fd);
    int size = cmd.args.size();

    if (!user.isResgistered()) {
        string nick = nickExists(user.nick) ? user.real_nick : "*";
        return sendNotRegistered(nick, cmd.Name(), fd);
    }
    if (size < 3) {
        return sendNeedMoreParams(user.real_nick, cmd.Name(), fd);
    }
    string name = cmd.args[1];
    string message = cmd.args[2][0] == ':'
                     ? cmd.args[2].substr(1)
                     : cmd.args[2];
    if (!tools::starts_with_mask(name) && size == 3) {
        tools::ToUpperCase(name);
        if (!nick_fd_map.count(name)) {
            return sendNoSuchNick(fd, user.real_nick, cmd.args[1]);
        }
        User &receiver = getUserFromNick(name);
        if (!receiver.isResgistered()) {
            return sendNoSuchNick(fd, user.real_nick, cmd.args[1]);
        }
        string reply = ":" + user.prefix
                        + " PRIVMSG "
                        + cmd.args[1] + " :"
                        + message;
        return DataToUser(receiver.fd, reply, NO_NUMERIC_REPLY);
    }
    if (tools::starts_with_mask(name)
        && size == 3)
    {
        if (!channelExists(cmd.args[1])) {
            return sendNoSuchChannel(user.real_nick, cmd.args[1], fd);
        }
        Channel &channel = channel_map.find(name)->second;
        if (!channel.userIsInChannel(user.nick)) {
            string reply = ERR_CANNOTSENDTOCHAN
                           + user.real_nick + " "
                           + channel.name
                           + STR_CANNOTSENDTOCHAN
                           + "the +n (noextmsg) mode is set";
            return DataToUser(fd, reply, NUMERIC_REPLY);
        }
        if (channel.banModeOn()
            && (channel.userInBlackList(user.real_nick, user.ip_address)
                || channel.all_banned)
            && !channel.isUserOperator(user))
        {
            string reply = ERR_CANNOTSENDTOCHAN
                           + user.real_nick + " "
                           + channel.name
                           + STR_CANNOTSENDTOCHAN
                           + "banned";
            return DataToUser(fd, reply, NUMERIC_REPLY);
        }
        if (channel.moderatedModeOn()
            && !user.isChannelOperator(name)
            && !user.isChannelModerator(name))
        {
            string reply = ERR_CANNOTSENDTOCHAN
                           + user.real_nick + " "
                           + channel.name
                           + STR_CANNOTSENDTOCHAN
                           + "the +m (moderated) mode is set";
            return DataToUser(fd, reply, NUMERIC_REPLY);
        }
        string reply = ":" + user.prefix
                        + " PRIVMSG "
                        + channel.name + " :"
                        + message;
        sendMessageToChannel(channel, reply, user.nick);
    }
}

/**
 * Command: WHOIS
 * Parameters: <user>
 *
 */
void AIrcCommands::WHOIS(Command &cmd, int fd) {

    User &user = getUserFromFd(fd);
    int size = cmd.args.size();

    if (!user.isResgistered()) {
        string nick = nickExists(user.nick) ? user.real_nick : "*";
        return sendNotRegistered(nick, cmd.Name(), fd);
    }
    if (size < 2) {
        return sendNeedMoreParams(user.real_nick, cmd.Name(), fd);
    }
    string nick = cmd.args[1];
    tools::ToUpperCase(nick);
    if (!nickExists(nick)) {
        return sendNoSuchNick(fd, user.real_nick, cmd.args[1]);
    }
    sendWhoisReply(cmd, fd, user, nick);
}


} // namespace irc
