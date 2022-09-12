#include "Server/AIrcCommands.hpp"
#include "NumericReplies.hpp"
#include "Channel.hpp"
#include "User.hpp"
#include "Command.hpp"
#include "Tools.hpp"

using std::string;

namespace irc {

void AIrcCommands::createNewChannel(const Command &cmd, int size,
                                    User &user, int fd)
{
    Channel channel(cmd.args[1], user);
    user.ch_name_mask_map.insert(
            std::pair<string, unsigned char>(channel.name, 0x80));
    if (size >= 3) {
        channel.key = cmd.args[2];
        channel.addMode(CH_PAS);
    }
    addNewChannel(channel);
    sendJoinReply(fd, user, channel, false);
}

void AIrcCommands::sendWhoisReply(const Command &cmd, int fd,
                                  User &user, string &nick)
{
    User &whois = getUserFromNick(nick);
    string info_rpl = RPL_WHOISUSER
                           + user.real_nick + " "
                           + whois.real_nick + " "
                           + whois.name + " "
                           + whois.ip_address
                           + STR_WHOISUSER
                           + whois.full_name;
    DataToUser(fd, info_rpl, NUMERIC_REPLY);
    if (!whois.ch_name_mask_map.empty()) {
        string channel_rpl = constructWhoisChannelRpl(whois, user.real_nick);
        DataToUser(fd, channel_rpl, NUMERIC_REPLY);
    }
    string mid_rpl = RPL_WHOISSERVER
                     + user.real_nick + " "
                     + whois.real_nick + " "
                     + hostname
                     + " :WhatsApp 2";
    DataToUser(fd, mid_rpl, NUMERIC_REPLY);
    string end_rpl = RPL_ENDOFWHOIS
                     + user.real_nick + " "
                     + cmd.args[1]
                     + STR_ENDOFWHOIS;
    DataToUser(fd, end_rpl, NUMERIC_REPLY);
}

void AIrcCommands::joinExistingChannel(int fd, User &user,
                                       Channel &channel)
{
    channel.addUser(user);
    user.ch_name_mask_map.insert(
        std::pair<string, unsigned char>(channel.name, 0x00));
    if (channel.topicModeOn()
        && !channel.topic.empty())
    {
        string reply = RPL_TOPIC
                       + user.nick + " "
                       + channel.name + " :"
                       + channel.topic;
        DataToUser(fd, reply, NUMERIC_REPLY);
    }
    sendJoinReply(fd, user, channel, true);
}

void AIrcCommands::sendBlackListReply(int fd, const User &user,
                                      Channel &channel)
{
    if (!channel.black_list.empty()) {
        for (std::map<string, int>::iterator it = channel.black_list.begin();
             it != channel.black_list.end(); it++)
        {
            string blacklist_rpl = RPL_BANLIST
                                   + user.real_nick + " "
                                   + channel.name + " "
                                   + it->first + " "
                                   + getUserFromFd(it->second).real_nick;
            DataToUser(fd, blacklist_rpl, NUMERIC_REPLY);
        }
    }
    string blacklist_end_rpl = RPL_ENDOFBANLIST
                               + user.real_nick + " "
                               + channel.name
                               + STR_ENDOFBANLIST;
    return DataToUser(fd, blacklist_end_rpl, NUMERIC_REPLY);
}

string AIrcCommands::checkAndGetVoiceRpl(const Command &cmd, const User &user,
                                         Channel &channel, const string &mode,
                                         User &other) const
{
    string mode_rpl;
    if (tools::charIsInString(mode, '+')) {
        other.addChannelMask(channel.name, CH_MOD);
        mode_rpl = ":" + user.prefix
                    + " MODE "
                    + cmd.args[1]
                    + " +v :"
                    + cmd.args[3];
    }
    if (tools::charIsInString(mode, '-')) {
        other.deleteChannelMask(channel.name, CH_MOD);
        mode_rpl = ":" + user.prefix
                    + " MODE "
                    + cmd.args[1]
                    + " -v :"
                    + cmd.args[3];
    }
    return mode_rpl;
}

void AIrcCommands::checkModeToAddOrDelete(const Command &cmd, Channel &channel,
                                          User &user, char m, int mode)
{
    string mode_rpl;

    if (!tools::charIsInString(cmd.args[2], m)) {
        return ;
    }
    if (tools::charIsInString(cmd.args[2], '+')) {
        channel.addMode(mode);
        mode_rpl = ":" + user.prefix
                    + " MODE "
                    + cmd.args[1]
                    + " :+"
                    + m;
    } else if (tools::charIsInString(cmd.args[2], '-')) {
        channel.deleteMode(mode);
        mode_rpl = ":" + user.prefix
                    + " MODE "
                    + cmd.args[1]
                    + " :-"
                    + m;
    }
    sendMessageToChannel(channel, mode_rpl, user.nick);
    return DataToUser(user.fd, mode_rpl, NO_NUMERIC_REPLY);
}

void AIrcCommands::checkKeyMode(const irc::Command &cmd, irc::Channel &channel,
                                User &user)
{
    string mode_rpl;
    string key = cmd.args[3][0] == ':'
                 ? cmd.args[3].substr(1)
                 : cmd.args[3];
    if (tools::charIsInString(cmd.args[2], '+')) {
        if (!channel.key.empty()) {
            return ;
        }
        channel.addMode(CH_PAS);
        channel.key = key;
        mode_rpl = ":" + user.prefix
                    + " MODE "
                    + channel.name
                    + " +k :"
                    + key;
    } else if (tools::charIsInString(cmd.args[2],'-')) {
        if (channel.key.empty()) {
            return ;
        }
        channel.deleteMode(CH_PAS);
        channel.key = "";
        mode_rpl = ":" + user.prefix
                    + " MODE "
                    + channel.name
                    + " -k :"
                    + key;
    }
    sendMessageToChannel(channel, mode_rpl, user.nick);
    return DataToUser(user.fd, mode_rpl, NO_NUMERIC_REPLY);
}

void AIrcCommands::checkOpMode(const irc::Command &cmd, string nick,
                               User &user, irc::Channel &channel,
                               int fd)
{
    User &other = getUserFromNick(nick);
    string op_rpl;
    if (tools::charIsInString(cmd.args[2], '+')) {
        if (!nick.compare(user.nick)) {
            return ;
        }
        other.addChannelMask(channel.name, OP);
        op_rpl = " +o :";
    } else if (tools::charIsInString(cmd.args[2],'-')) {
        other.deleteChannelMask(channel.name, OP);
        op_rpl = " -o :";
    }
    string mode_rpl = ":" + user.prefix
                       + " MODE "
                       + cmd.args[1]
                       + op_rpl
                       + cmd.args[3];
    DataToUser(fd, mode_rpl, NO_NUMERIC_REPLY);
    if (other.fd != fd) {
        DataToUser(other.fd, mode_rpl, NO_NUMERIC_REPLY);
    }
}

void AIrcCommands::sendQuitToAllChannels(int fd, string &msg) {

    User &user = getUserFromFd(fd);

    for (std::map<string, unsigned char>::iterator
                 it = user.ch_name_mask_map.begin();
         it != user.ch_name_mask_map.end(); it++)
    {
        string ch_name = it->first;
        Channel &channel = getChannelFromName(ch_name);
        string quit_msg = ":" + user.prefix
                          + " QUIT :"
                          + msg; //Client Closed connection";
        sendMessageToChannel(channel, quit_msg, user.nick);
    }
}

void AIrcCommands::sendClosingLink(int fd, string &reason) {

    User &user = getUserFromFd(fd);

    /* irc hispano, en función del servidor, escribe una palabra que
     * parece ser random appendeada a @<ip>, en concreto, 113AADPV1@<ip>,
     * cuando el usuario no está registrado y se le ha de cerrar el link.
     * En este servidor, se envía unregistered@<ip>.
     */
    string prefix = user.prefix.empty() ?
                    "unregistered@" + user.ip_address
                    : user.prefix;
    string msg = "ERROR :Closing link : ("
                 + prefix + ") ["
                 + reason + "]";
    DataToUser(fd, msg, NO_NUMERIC_REPLY);
}

void AIrcCommands::removeUserFromServer(int fd, string &reason) {
    sendQuitToAllChannels(fd, reason);
    removeUserFromChannels(fd);
    sendClosingLink(fd, reason);
    removeUser(fd);
    return closeConnection(fd);
}

}

