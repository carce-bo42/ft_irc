#ifndef IRC42_SERVER_H
# define IRC42_SERVER_H

#include "Channel.hpp" // without this, it doesnt compile
#include "Types.hpp"

#include "Server/AIrcCommands.hpp"

namespace irc {

class Server : public AIrcCommands {

    typedef void (irc::AIrcCommands::*CommandFnx)(Command &cmd, int fd);
    typedef std::map<std::string, CommandFnx> CommandMap;

    public:
    Server(void);
    Server(std::string &password);
    Server(std::string &ip, std::string &port);
    Server(std::string &ip, std::string &port, std::string &password);
    Server(const Server &other);
    ~Server();
    
    private:

    bool serverHasPassword();
    void maybeRegisterUser(User &user);
    void registerUser(User &user);

    void DataFromUser(int fd);
    void DataToUser(int fd, std::string data, int type);
    
    int mainLoop(void);

    void pingLoop(void);
    void sendPingToUser(int fd);

    CommandMap cmd_map;
    void loadCommandMap();

    /* Buffer management */
    char srv_buff[BUFF_MAX_SIZE];
    int srv_buff_size;
    std::string processLeftovers(int fd);
    void parseCommandBuffer(std::string &cmd_content, int fd);
};

/**
 * Reglas propias servidor :
 * - El número mázimo de usuarios conectados a la vez será de 255 (MAX_FDS).
 * - Los usuarios se guardan, con comandos no finalizados en CRLF, un buffer
 *  interno, de donde reconstruir un comando que se haya enviado troceado.
 *  La suma de los bytes en este buffer y los del comando al que se añaden
 *  nunca podrá ser superior a 512.
 * e.g. Si tengo 200 bytes guardados de USer A (un mensaje muy largo), y 
 * me llega el final de éste ( <loquesea> CRLF), insertaré esos 200 bytes
 * al pricnipio de <loquesea>, y si 200 + len(<loquesea> CRLF) > 512, este
 * comando no será ejecutado y se le enviará al usuario un error de input
 * too long.
 * De forma silenciosa, cuando el buffer interno (que será de 512 bytes) 
 * se llene, cuando llegue un nuevo mensaje que añadir a lo que ya hay 
 * se enviará tambien error de input too long.
 * 
 * 
 * Funcionamiento interno de las estructuras del servidor:
 * 
 * Cuando un usuario se conecta, de él solo se conoce el
 * fd asociado, o la ENTRADA que ocupa en la matriz de fds
 * que usa poll. Ni nickname, ni username, ni nada.
 * Por este motivo, existe un FdUserMap que en función de su
 * índice de fd que ocupe en la matriz de poll, se corresponde
 * con una entrada de User. A su vez, existirá, para amenizar 
 * el envío de mensajes privados o mensajes a canal o peticiones
 * con necesidad de lookup de nombre de usuario, un mapa de
 * clave nickname y valor fd. De esta forma, cuando se tenga un nickname
 * en vez de un fd, se usará el mapa para encontrar el fd correpsondiente,
 * y luego se buscará al usuario dentro del FdUserMap.
 * 
 * 
 * 
 * 
 * Para gestionar mensajes de NICK / USER iniciales en
 * redes con banda ancha muy mala, o troleos con netcat más bien
 * ya que no existe a día de hoy una red que no pueda procesar por
 * tcp en un solo paquete 30 bytes, es necesario poder reconocer
 * a un usuario por su fd antes incluso de que nos haya proporcionado
 * un nickname. Y cuando s
 */
}

#endif /* IRC42_SERVER_H */
