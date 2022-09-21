#include "Server/IrcDataBase.hpp"
#include "User.hpp"
#include "Log.hpp"
#include "Channel.hpp"
#include "libft.h"

using std::string;

namespace irc {

IrcDataBase::IrcDataBase(void) {
}

IrcDataBase::~IrcDataBase() {
}

IrcDataBase::IrcDataBase(const IrcDataBase& other)
:
    channel_map(other.channel_map),
    nick_fd_map(other.nick_fd_map),
    fd_user_map(other.fd_user_map)
{}

void IrcDataBase::addNewUser(int new_fd, const char *ip_address) {
    /* case server is full of users */
    if (new_fd == -1
        || ip_address == NULL) {
        return ;
    }
    User user(new_fd, ip_address);
    addFdUserPair(new_fd, user);
}


void IrcDataBase::removeUser(int fd) {
    
    User &user = getUserFromFd(fd);
    LOG(INFO) << "User " << user << " removed";
    /* If the user had a nick registered, erase it */
    if (nick_fd_map.count(user.nick)) {
        removeNickFdPair(user.nick);
    }
    /* erase user from fd map (this entry is created after connection) */
    removeFdUserPair(fd);
}

void IrcDataBase::updateUserNick(int fd, string &new_nick,
                                 string &new_real_nick)
{
    User& user = getUserFromFd(fd);
    removeNickFdPair(user.nick);
    addNickFdPair(new_nick, fd);
    updateUserInChannels(user, new_nick);
    user.nick = new_nick;
    user.real_nick = new_real_nick;
}

void IrcDataBase::addFdUserPair(int fd, User& user) {
    fd_user_map.insert(std::pair<int, User>(fd, user));
}

void IrcDataBase::removeFdUserPair(int fd) {
    fd_user_map.erase(fd);
}

void IrcDataBase::addNickFdPair(string &nick, int fd) {
    nick_fd_map.insert(std::pair<string, int>(nick, fd));
}

void IrcDataBase::removeNickFdPair(string &nick) {
    nick_fd_map.erase(nick);
}

void IrcDataBase::addNewChannel(Channel& new_channel) {
    channel_map.insert(std::pair<string, Channel>(new_channel.name, new_channel));
}

void IrcDataBase::maybeRemoveChannel(Channel& channel) {
    if (channel.users.empty()) {
        channel_map.erase(channel_map.find(channel.name));
    }
}

bool IrcDataBase::fdExists(int fd) {
    return fd_user_map.count(fd);
}

bool IrcDataBase::nickExists(string &nick) {
    return nick_fd_map.count(nick);
}

bool IrcDataBase::channelExists(string &channel_name) {
    return channel_map.count(channel_name);
}

/* See 
 * https://forums.mirc.com/ubbthreads.php/topics/186181/nickname-valid-characters */
bool IrcDataBase::nickFormatOk(string &nickname) {

    if (nickname.empty()
        || nickname.length() > NAME_MAX_SIZE)
    {
        return false;
    }
    if (!ft_isalpha(nickname[0])) {
        return false;
    }
    for (string::iterator it = nickname.begin(); it != nickname.end(); it++) {
        if (*it != '`' && *it != '|' && *it != '^' && *it != '_'
            && *it != '-' && *it != '{' && *it != '}' && *it != '['
            && *it != ']' && *it != '\\' && *it != '@' && *it != '!')
        {
            continue ;
        }
        else {
            return false;
        }
    }
    return true;
}

User& IrcDataBase::getUserFromFd(int fd) {
    FdUserMap::iterator it = fd_user_map.find(fd);
    return it->second;
}

int IrcDataBase::getFdFromNick(string& nickname) {
    NickFdMap::iterator it = nick_fd_map.find(nickname);
    return it->second;
}

User& IrcDataBase::getUserFromNick(string& nickname) {
    int fd = getFdFromNick(nickname);
    FdUserMap::iterator it = fd_user_map.find(fd);
    return it->second;
}

Channel& IrcDataBase::getChannelFromName(string& name) {
    ChannelMap::iterator it = channel_map.find(name);
    return it->second;
}

void IrcDataBase::updateUserInChannels(irc::User &user, string new_nick) {
    for (std::map<string, unsigned char>::iterator
            it = user.ch_name_mask_map.begin();
            it != user.ch_name_mask_map.end(); it++)
    {
        string ch_name = it->first;
        Channel &channel = getChannelFromName(ch_name);
        channel.updateUserNick(user.nick, new_nick);
    }
}

void IrcDataBase::debugFdUserMap(void) {
    for(FdUserMap::const_iterator it = fd_user_map.begin();
        it != fd_user_map.end(); ++it)
    {
        LOG(DEBUG) << "[FD USER MAP] fd : " << it->first << ", &user : " << &(it->second);
    }
}

void IrcDataBase::debugNickFdMap(void) {
    for(NickFdMap::const_iterator it = nick_fd_map.begin();
        it != nick_fd_map.end(); ++it)
    {
        LOG(DEBUG) << "[NICK FD MAP] nick : " << it->first << ", fd : " << (it->second);
    }
}

void IrcDataBase::removeUserFromChannels(int fd) {
    
    User &user = getUserFromFd(fd);

    for (std::map<string, unsigned char>::iterator
                 it = user.ch_name_mask_map.begin();
         it != user.ch_name_mask_map.end(); it++)
    {
        string ch_name = it->first;
        Channel &channel = getChannelFromName(ch_name);
        channel.deleteUser(user);
        maybeRemoveChannel(channel);
    }
}

} // namespace
