#ifndef IRC42_USER_H
# define IRC42_USER_H

#include "Types.hpp"
#include <iostream>

namespace irc {

/*
 * Los usuarios tendrán 2 nicks : El que dicen que tienen (real),
 * y el que se guardará en los mapas. Esto es para poder hacer de
 * forma directa la busqueda de usuarios cuando se les envía un 
 * mensaje, sin tener que estar haciendo comparaciones con tolower.
 * Se aprovecha la funcion en tools de ToUpperCase para crear nick 
 * a partir de real_nick (el recibido).
 * El prefijo, de todas formas, seguirá reflejando el real_nick,
 * ya me jodería que si me quiero llamar ChRiStIAn el servidor me
 * llame CHRISTIAN. El control interno tiene que quedar como es, interno.
 */


/*
 * Christian 10/08/2022 : 
 * Toda esta info esta sacada de https://modern.ircdocs.horse/#user-modes
 * Complementada con http://www.geekshed.net/commands/user-modes/
 *  y http://www.geekshed.net/commands/channel-modes/
 * 
 * User modes que nos son utiles : 
 * 
 * +i / +o : Invisibilidad / operador == OK
 * 
 * Por qué no usar el rresto :
 * 
 * +0 : indica que el operador es local, de este servidor only. Nosotros
 *      no hacemos conexion servidor - servidor, por lo que no hay diferencia
 *      entre 'o' y '0'.
 * +r : Esto es para servidores que utilicen IRCv3, que guardan en base de datos
 *      usuarios, credenciales, contraseñas de inicio de sesión, y toda la
 *      parafernalia que hace que te puedas desconectar y el servidor luego siga
 *      sabiendo quien eres con darle unas credenciales. Se va mucho más allá 
 *      del objetivo del proyecto.
 * +w : WALLOPS user mode. A no ser que se quiera investigar que mierda significa
 *      esto, mejor ni meterlo.
 * 
 * 
 * Channel modes que nos son utiles :
 * 
 * (Por cierto no he encontrado dónde un chanop setea otro chanop.
 * En plan el comando de MODE que lo hace.
 * Supongo que MODE &channelname +o <new_operator_nickname>)
 * 
 * +b / +i / +k : ban list, inivitation list, key protection. == OK
 * 
 * El resto de channel modes me parecen una patata. Porque son tonterías como
 * 
 *  +t : el topic solo puede cambiarlo el operator
 *  +s : el channel es secreto y no se ve en list
 *  +n : controla si los usuarios pueden escribir al canal sin estar dentro  
 *  +m : controla si puede hablar gente que no sea moderadora. 
 *  +l : controla si el canal tiene limite de clientes o no.
 *  +e : controla que pueda haber excepciones a los baneados.
 *  +I : invite-exception channel mode. No quiero ni saber lo que es.
 * 
 * Razones por las que no son necesarias estas anteriores mascaras, una por una:
 * 
 * +t : Se fijan unas normas. El topic lo cambia solo el operator desde siempre.
 *      Y nunca podrá hacerlo alguien que no sea operator. Fasil rapido sencillo.
 * +s : Se puede hacer que si un channel tiene key o invitación o las dos, no sea
 *      visible. O pasar completamente del feature. Sobre este tengo dudas porque es 
 *      una chorrada implementarlo. Pero cuanto menos metamos mejor.
 * +n : Qué cojones. Se setea que para hablar en un canal has de estar dentro y punto.
 *      Quieres decir solo una cosa a un canal ? Entras, lo dices, y te piras. Nada de
 *      tener un modo para esto, que solo trae dolores de cabeza.
 * +m : Si los operadores quieren hablar solo ellos con el resto de público, que se jodan.
 *      Si han de hablar de algo importante, lo mismo da porque el resto de no-operadores les
 *      leerían y aquello que es tan importante lo verían todos. Este modo, en mi opinión, 
 *      no tiene sentido ni en el protocolo real.
 * +l : Esto se setea en el server y será absoluto. Numero de clientes por canal ? Los que 
 *      nos salgan de los huevos. Pero no se deja abierta la posibilidad de cambiarlo a un
 *      usuario cualquiera.
 * +e : excepciones a baneos ? Qué tal desbanear ? XD. Flag de principio a fin inútil.
 * +I : solo el nombre ya da palo. 
 * 
 */

class User {

    typedef std::map<std::string, unsigned char> ChannelMaskMap;

    public:
    User(int fd, const char* ip_address);
    User(const User &other);
    ~User();

    User& operator=(const User &other);
    bool operator==(User const &other) const;

    void setPrefixFromHost(std::string &host);
    /* ATTRIBUTES */
    int fd;
    std::string ip_address;
    std::string real_nick; // caRCe-b042 
    std::string nick;      // CARCE-B042 (for lookups)
    std::string name;
    std::string full_name;
    std::string prefix;
    std::string mask;
    unsigned char server_mode;
    std::string afk_msg;
    std::string last_password; // only for password servers

    /* Channel Things */
    ChannelMaskMap ch_name_mask_map;

    char buffer[BUFF_MAX_SIZE];
    int buffer_size;

    bool isReadyForRegistration(bool server_password_on);
    bool registered;

    bool hasLeftovers(void) const;
    void resetBuffer(void);
    void addLeftovers(std::string &leftovers);
    std::string BufferToString(void) const;

    /* PING PONG things */
    time_t getLastMsgTime(void);
    time_t getPingTime(void);
    bool isOnPongHold(void);
    void resetPingStatus(void);
    void updatePingStatus(std::string &random);

    bool isResgistered(void);
    bool isAway(void);
    bool isOperator(void);
    bool isChannelModerator(std::string name);
    bool isChannelOperator(std::string name);
    bool isInChannel(std::string &channel); // nueva, carcebo
    void addChannelMask(std::string &channel, int bits);
    void deleteChannelMask(std::string &channel, int bits);
    void addServerMask(int bits);
    void deleteServerMask(int bits);

    bool on_pong_hold;
    time_t last_received;
    time_t ping_send_time;
    std::string ping_str;

};

}

std::ostream& operator<<(std::ostream &o, const irc::User &rhs);

#endif
