#ifndef IRC42_IRCDATABASE_H
# define IRC42_IRCDATABASE_H

#include "Types.hpp"
#include <string>

namespace irc {

/* Esta clase guarda toda la información sobre usuarios y canales
 * del servidor. Sirve para consultar, extraer, o añadir información
 * sobre ambos. No se debe retraer un objeto User o Channel de ningun
 * otro lugar que no sea esta clase. Para hacerlo, es importante 
 * recuperarlos con referencias:
 * User& user = getUserFrom___();
 * Channel& channel = getChannelFrom___();
 * De esta forma, se cumple RAII (Resource Aquisition Is Initialization),
 * a la vez que se consigue una interfaz uniforme para trabajar con la 
 * información que se tiene.
 */

class IrcDataBase {

    typedef std::map<std::string, irc::Channel> ChannelMap;
    typedef std::map<int, irc::User> FdUserMap;
    typedef std::map<std::string, int> NickFdMap;

    public:
    IrcDataBase(void);
    IrcDataBase(const IrcDataBase& other);
    ~IrcDataBase();

    /* Data Bases */
    ChannelMap channel_map; // <string name, Channel> 
    NickFdMap nick_fd_map;  // <string nick, int fd>
    FdUserMap fd_user_map;  // <int fd, User>

    /* checkers */
    bool fdExists(int fd);
    bool nickExists(std::string& nick);
    bool nickFormatOk(std::string &nickname);
    bool channelExists(std::string &channel_name);
    
    /* accessors */
    User& getUserFromFd(int fd);
    User& getUserFromNick(std::string& nick);
    int getFdFromNick(std::string& nick);
    Channel& getChannelFromName(std::string& name);

    /* interactors */
    void addNewUser(int new_fd, const char *ip_address);
    void removeUser(int fd);

    void updateUserNick(int fd, std::string &new_nick,
                                std::string &new_real_nick);

    void addNickFdPair(std::string &nick, int fd);
    void removeNickFdPair(std::string &nick);

    void addFdUserPair(int fd, User& user);
    void removeFdUserPair(int fd);

    void addNewChannel(Channel& new_channel);
    void maybeRemoveChannel(Channel& channel);
    void removeUserFromChannels(int fd);

    void updateUserInChannels(User &user, std::string new_nick);

    void debugNickFdMap();
    void debugFdUserMap();
    //void debugChannelMap();
};

}

#endif /* IRC42_IRCDATABASE_H */
