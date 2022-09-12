#include "Server/AIrcCommands.hpp"
#include "NumericReplies.hpp"
#include "Channel.hpp"
#include "User.hpp"
#include "libft.h"
#include "Exceptions.hpp"

using std::string;

namespace irc {

void AIrcCommands::sendNeedMoreParams(string &nick, string& cmd_name, int fd) {
    string reply = ERR_NEEDMOREPARAMS
                   + nick + " "
                   + cmd_name
                   + STR_NEEDMOREPARAMS;
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendParamNeeded(string &nick, string &ch_name,
                                   string mode, string mode_msg, int fd)
{
    string reply = ERR_KEYNEEDED
                   + nick + " "
                   + ch_name    
                   + mode
                   + STR_KEYNEEDED
                   + mode_msg;
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendNotRegistered(string &nick, string &cmd_name, int fd) {
    string reply = ERR_NOTREGISTERED
                    + nick + " "
                    + cmd_name
                    + STR_NOTREGISTERED;
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendNoSuchChannel(string &nick, string &ch_name, int fd) {
    string reply = ERR_NOSUCHCHANNEL
                   + nick + " "
                   + ch_name
                   + STR_NOSUCHCHANNEL;
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendNotOnChannel(string &nick, string &ch_name, int fd) {
    string reply = ERR_NOTONCHANNEL
                   + nick + " "
                   + ch_name
                   + STR_NOTONCHANNEL;
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendBadChannelMask(string &nick, string &ch_name, int fd) {
    string reply = ERR_BADCHANMASK
                   + nick + " "
                   + ch_name
                   + STR_BADCHANMASK;
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendNoChannelModes(string &cmd_name, int fd) {
    string reply = ERR_NOCHANMODES
                   + cmd_name
                   + STR_NOCHANMODES;
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendChannelOperatorNeeded(string &nick, string &ch_name, int fd) {
    string reply = ERR_CHANOPRIVSNEEDED
                   + nick + " "
                   + ch_name
                   + STR_CHANOPRIVSNEEDED;
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendWelcome(string& nick, string &prefix, int fd) {
    string welcome_msg = RPL_WELCOME
                         + nick
                         + RPL_WELCOME_STR_1
                         + prefix;
    DataToUser(fd, welcome_msg, NUMERIC_REPLY);
}

void AIrcCommands::sendAlreadyRegistered(string &nick, int fd) {
    string reply = ERR_ALREADYREGISTERED
                   + nick
                   + STR_ALREADYREGISTERED;
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendPasswordMismatch(string &nick, int fd) {
    string reply = ERR_PASSWDMISMATCH
                   + nick
                   + STR_PASSWDMISMATCH;
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendNoSuchNick(int fd, string nick,
                                  string notFoundNick)
{
    string reply = ERR_NOSUCHNICK
                   + nick + " "
                   + notFoundNick + " "
                   + STR_NOSUCHNICK;
    DataToUser(fd, reply, NUMERIC_REPLY);
}

void AIrcCommands::sendJoinReply(int fd, User &user, Channel &channel,
                                 bool send_all)
{
    string join_rpl = ":" + user.prefix
                       + " JOIN :"
                       + channel.name;
    if (send_all) {
        sendMessageToChannel(channel, join_rpl, user.nick);
    }
    DataToUser(fd, join_rpl, NO_NUMERIC_REPLY);
    sendNamesReply(fd, user, channel);
}

void AIrcCommands::sendNamesReply(int fd, User &user, Channel &channel) {
    if (!channel.topic.empty()) {
        string top_rpl = RPL_TOPIC
                         + user.real_nick + " "
                         + channel.name + " :"
                         + channel.topic;
        DataToUser(fd, top_rpl, NUMERIC_REPLY);
    }
    string namesReply = constructNamesReply(user.real_nick, channel);
    DataToUser(fd, namesReply, NUMERIC_REPLY);
    namesReply = RPL_ENDOFNAMES
                 + user.real_nick + " "
                 + channel.name
                 + STR_ENDOFNAMES;
    DataToUser(fd, namesReply, NUMERIC_REPLY);
}

void AIrcCommands::sendListReply(int fd, User &user, string ch_name) {
    string start_rpl = RPL_LISTSTART
                       + user.real_nick
                       + STR_LISTSTART;
    DataToUser(fd, start_rpl, NUMERIC_REPLY);
    if (ch_name.compare("")) {
        Channel &channel = channel_map.find(ch_name)->second;
        string reply = constructListReply(user.real_nick, channel);
        DataToUser(fd, reply, NUMERIC_REPLY);
    } else if (channel_map.size() > 0) {
        for (std::map<string, Channel>::iterator it = channel_map.begin();
             it != channel_map.end(); it++)
        {
            string reply = constructListReply(user.real_nick, it->second);
            DataToUser(fd, reply, NUMERIC_REPLY);
        }
    }
    string end_reply = RPL_LISTEND
                       + user.real_nick
                       + STR_LISTEND;
    DataToUser(fd, end_reply, NUMERIC_REPLY);
}

void AIrcCommands::sendPartMessage(string &extra_msg, int fd, User &user,
                                   Channel &channel)
{
    bool msg = extra_msg.compare("");
    string part_message = msg 
                          ? " :\"" + extra_msg.substr(1) + "\""
                          : "";
    string part_rpl = ":" + user.prefix
                      + " PART " + (msg ? "" : ":")
                      + channel.name
                      + part_message;
    sendMessageToChannel(channel, part_rpl, user.nick);
    return (DataToUser(fd, part_rpl, NO_NUMERIC_REPLY));
}

void AIrcCommands::sendKickMessage(int fd, User &user, Channel &channel,
                                   string &kicked)
{
    string kick_rpl = ":" + user.prefix
                      + " KICK "
                      + channel.name + " "
                      + kicked + " :"
                      + kicked;
    sendMessageToChannel(channel, kick_rpl, user.nick);
    return (DataToUser(fd, kick_rpl, NO_NUMERIC_REPLY));
}

void AIrcCommands::sendChannelModes(int fd, string nick, Channel &channel) {
    string mode = channel.getModeStr();
    string mode_rpl = RPL_CHANNELMODEIS
                      + nick + " "
                      + channel.name + " :+"
                      + mode + "nt";
    DataToUser(fd, mode_rpl, NUMERIC_REPLY);
}

// PRIVATE METHODS
void AIrcCommands::sendMessageToChannel(Channel &channel, string &message,
                                        string &nick)
{
    for (std::list<string>::iterator it = channel.users.begin();
         it != channel.users.end(); it++)
    {
        if (!nickExists(*it)) {
            continue;
        }
        User &receiver = getUserFromNick(*it);
        if (receiver.nick.compare(nick)) {
            DataToUser(receiver.fd, message, NO_NUMERIC_REPLY);
        }
    }
}

string AIrcCommands::constructNamesReply(string nick, Channel &channel) {
    unsigned long i = 0;
    string reply = RPL_NAMREPLY
                   + nick + " = "
                   + channel.name + " " + ":";
    for (std::list<string>::iterator it = channel.users.begin();
         it != channel.users.end(); it++)
    {
        if (!nickExists(*it)) {
            continue;
        }
        User &user = getUserFromNick(*it);
        string at = user.isChannelModerator(channel.name) ? "+" : "";
        at = user.isChannelOperator(channel.name) ? "@" : at;
        reply += at;
        reply += user.real_nick;
        reply += (++i < channel.users.size()) ? " " : "";
    }
    return reply;
}

string AIrcCommands::constructListReply(string nick, Channel &channel) {
    string mode = channel.getModeStr();
    char* channel_size = ft_itoa((int)channel.users.size());
    if (!channel_size) {
        throw irc::exc::MallocError();
    }
    string reply = RPL_LIST
                   + nick + " "
                   + channel.name + " "
                   + string(channel_size) + " :[+"
                   + mode + "nt]";
    reply += channel.topicModeOn()
              ? (" " + channel.topic)
              : "";
    free(channel_size);
    return reply;
}

string AIrcCommands::constructWhoisChannelRpl(User &user, string &nick) {
    string rpl = RPL_WHOISCHANNELS
                 + nick + " "
                 + user.real_nick + " :";
    unsigned long i = 0;
    unsigned long size = user.ch_name_mask_map.size();
    for (std::map<string, unsigned char> ::iterator
         it = user.ch_name_mask_map.begin();
         i < size; i++)
    {
        string at = user.isChannelModerator(it->first) ? "+" : "";
        at = user.isChannelOperator(it->first) ? "@" : at;
        rpl += at + it->first;
        rpl += i < (size - 1) ? " " : "";
        it++;
    }
    return rpl;
}

} /* namespace irc */
