#ifndef IRC42_CHANNEL_H
#define IRC42_CHANNEL_H

#include <list>
#include <map>
#include <string>

namespace irc {

class User;

class Channel {

    typedef std::list<std::string> NickList;
    typedef std::map<std::string, int> BlackListOpMap;


    public:
    Channel(std::string name, User& user);
    ~Channel();

    /* Class functions */
    void addUser(User& user);
    void deleteUser(User& user);
    void banUser(std::string &user, int fd);
    bool unbanUser(std::string &user);
    bool userInBlackList(std::string nick, std::string ip_address);
    bool inviteModeOn();
    bool keyModeOn();
    bool topicModeOn();
    bool banModeOn();
    bool moderatedModeOn();
    bool isInvited(std::string &nick);
    void addToWhitelist(std::string &nick);
    bool userIsInChannel(std::string& nick);
    bool isUserOperator(User& user);
    void addMode(int bits);
    void deleteMode(int bits);
    std::string getModeStr();
    void updateUserNick(std::string &old_nick, std::string &new_nick);

    /* ATTRIBUTES */
    NickList users;
    NickList white_list;
    BlackListOpMap black_list;

    std::string name;
    unsigned char mode;
    std::string key;
    bool all_banned;
    /**
     * Channel topic
     */
    std::string topic;

};

} // namespace

#endif
