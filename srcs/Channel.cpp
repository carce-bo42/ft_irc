#include "Channel.hpp"
#include <algorithm>
#include "Log.hpp"
#include "User.hpp"
#include "Tools.hpp"

using std::string;
using std::list;

namespace irc {

/* 
 * Al crearse el canal se setea al usuario creador el rol 'o' 
 * el canal al principio no tiene ningún modo. Se setea después 
 */
Channel::Channel(string name, User& user) : name(name), mode(0) {
    all_banned = false;
    users.push_back(user.nick);
    addMode(CH_TOP);
}

/**
 * Un canal se borra cuando se queda sin usuarios dentro, 
 * por lo que en principio el destructor estaría vacío 
 */
Channel::~Channel() {
}

/* CLASS FUNCTIONS */

/**
 * Añadir un usuario a un canal:
 * 1. Se comprueba la disponibilidad del canal a nivel de
 *    comando, antes de llamar esta funcion
 * 2. Se añade el usuario a la lista de usuarios
 */
void Channel::addUser(User &user) {
    users.push_back(user.nick);
}

/*
 * el nick dentro del canal cuando se lo actualicen
 *
 *  funcion que gestione asignar un nuevo operador :
 *  busque primero algun otro usuario con 'o', y en caso de
 *  que no que itere la lista hasta que de con el heredero correcto. 
 * 
 * Borrar un usuario del canal:
 * 1. Se busca dicho usuario
 * 2. Se borra el canal de la lista de canales del usuario
 * 
 */
void Channel::deleteUser(User &user) {
    NickList::iterator end = users.end();
    for (NickList::iterator u = users.begin(); u != end; u++) {
        if (!u->compare(user.nick)) {
            users.erase(u);
            break ;
        }
    }
}

/**
 * Banea a un usuario
 */
void Channel::banUser(string &user, int fd) {
    if (!user.compare("*!*@*")) {
        all_banned = true;
    }
    black_list.insert(std::pair<string, int>(user, fd));
}

/**
 * Desbanea a un usuario
 */
bool Channel::unbanUser(string &user) {
    if (black_list.count(user)) {
        std::map<string, int>::iterator it = black_list.find(user);
        if (!user.compare("*!*@*")) {
            all_banned = false;
        }
        black_list.erase(it->first);
        if (black_list.size() == 0) {
            deleteMode(CH_BAN);
        }
        return true;
    }
    return false;
}

/**
 * Comprueba si el usuario está en la lista de baneados
 */
bool Channel::userInBlackList(string nick, string ip_address) {
    for (std::map<string, int>::iterator it = black_list.begin();
         it != black_list.end(); it++)
    {
        string banned_user = it->first.substr(0, it->first.find("!"));
        string banned_ip = it->first.substr(it->first.find("@") + 1);
            // If *!*@* is found in the blackList
        if ((!banned_ip.compare("*") && !banned_user.compare("*"))
            // If nick!*@* is found
            || (!nick.compare(banned_user) && !banned_ip.compare("*"))
            // If *!*@ip_address is found
            || (!ip_address.compare(banned_ip) && !banned_user.compare("*"))
            // If user!*@ip_address is found
            || (!nick.compare(banned_user) && !ip_address.compare(banned_ip)))
        {
            return true;
        }
    }
    return false;
}

/**
 * Comprueba si el canal está en modo invitación
 */
bool Channel::inviteModeOn() {
    return ((mode & 0x08) >> CH_INV);
}

/**
 * Comprueba si el canal está en modo contraseña
 */
bool Channel::keyModeOn() {
        return (((mode & 0x10) >> CH_PAS));
}

/**
 * Comprueba si el canal está en modo baneos
 */
bool Channel::banModeOn() {
    return (((mode & 0x20) >> CH_BAN));
}

/**
 * Comprueba si el canal está en modo topic
 */
bool Channel::topicModeOn() {
    return (((mode & 0x02) >> CH_TOP));
}


/**
 * Comprueba si el canal está en modo moderado
 */
    bool Channel::moderatedModeOn() {
        return (((mode & 0x04) >> CH_MOD));
    }

/**
 * Check if user has operator channel_mode
 */
bool Channel::isUserOperator(User &user) {
    char mask = user.ch_name_mask_map.find(name)->second;
    return (((mask & 0x80) >> 7));
}

/**
 * Añade un nuevo usuario a la whitelist 
 */
void Channel::addToWhitelist(string &nick) {
    white_list.push_back(nick);
}

/**
 * Devuelve true si el usuario está en la white_list del canal 
 */
bool Channel::isInvited(string &nick) {
    return (std::find(white_list.begin(), white_list.end(), nick)
                      != white_list.end());
}

bool Channel::userIsInChannel(string& nick) {
    return (std::find(users.begin(), users.end(), nick) != users.end());
}

void Channel::addMode(int bits) {
    mode |= (0x01 << bits);
}

void Channel::deleteMode(int bits) {
    mode &= ~(0x01 << bits);
}

string Channel::getModeStr() {
    string mode = keyModeOn() ? "k" : "";
    mode += inviteModeOn() ? "i" : "";
    mode += moderatedModeOn() ? "m" : "";
    mode += banModeOn() ? "b" : "";
    return mode;
}

void Channel::updateUserNick(string &old_nick, string &new_nick) {
    list<string>::iterator it = std::find(users.begin(), users.end(), old_nick);
    if (it != users.end()) {
        *it = new_nick;
    }
}

} // namespace
