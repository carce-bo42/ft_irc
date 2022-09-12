#include <iostream>
#include <utility>

#include <string.h>
#include <errno.h>
#include <time.h>

#include "Server/Server.hpp"
#include "User.hpp"
#include "Exceptions.hpp"
#include "Command.hpp"
#include "Tools.hpp"
#include "NumericReplies.hpp"
#include "Log.hpp"
#include "libft.h"

using std::string;
using std::vector;

/* ref https://www.youtube.com/watch?v=dEHZb9JsmOU
 * 
 * IRC 'handshake'
 * http://chi.cs.uchicago.edu/chirc/irc_examples.html
 * 
 * install Weechat (Hell) on Ubuntu
 * https://weechat.org/files/doc/stable/weechat_user.en.html#dependencies
 * https://weechat.org/files/doc/stable/weechat_user.en.html
 * 
 * Userguide to Weechat:
 * https://www.linode.com/docs/guides/using-weechat-for-irc/
 * 
 * Pointers to member functions :
 * https://cplusplus.com/forum/beginner/47833/
 * https://stackoverflow.com/questions/6265851/
 */

namespace irc {

Server::Server(void)
:
    AIrcCommands()
{
    ft_memset(srv_buff, '\0', BUFF_MAX_SIZE);
    srv_buff_size = 0;
    loadCommandMap();
    mainLoop();
}

Server::Server(string& password)
:
    AIrcCommands(password)
{
    ft_memset(srv_buff, '\0', BUFF_MAX_SIZE);
    srv_buff_size = 0;
    loadCommandMap();
    mainLoop();
}

Server::Server(string &hostname, string &port)
:
    AIrcCommands(hostname, port)
{
    ft_memset(srv_buff, '\0', BUFF_MAX_SIZE);
    srv_buff_size = 0;
    loadCommandMap();
    mainLoop();
}

Server::Server(string &hostname, string &port, string &password)
:
    AIrcCommands(hostname, port, password)
{
    ft_memset(srv_buff, '\0', BUFF_MAX_SIZE);
    srv_buff_size = 0;
    loadCommandMap();
    mainLoop();
}

Server::Server(const Server& other)
:
    AIrcCommands(other),
    cmd_map(other.cmd_map),
    srv_buff_size(other.srv_buff_size)
{
    ft_memset(srv_buff, '\0', BUFF_MAX_SIZE);
    if (other.srv_buff_size > 0) {
        ft_memcpy(srv_buff, other.srv_buff, other.srv_buff_size);
    }
}

Server::~Server(void) {
}

void Server::loadCommandMap(void) {
    cmd_map.insert(std::pair<string, CommandFnx>(string("NICK"), &AIrcCommands::NICK));
    cmd_map.insert(std::pair<string, CommandFnx>(string("USER"), &AIrcCommands::USER));
    cmd_map.insert(std::pair<string, CommandFnx>(string("PING"), &AIrcCommands::PING));
    cmd_map.insert(std::pair<string, CommandFnx>(string("PONG"), &AIrcCommands::PONG));
    cmd_map.insert(std::pair<string, CommandFnx>(string("JOIN"), &AIrcCommands::JOIN));
    cmd_map.insert(std::pair<string, CommandFnx>(string("PART"), &AIrcCommands::PART));
    cmd_map.insert(std::pair<string, CommandFnx>(string("KICK"), &AIrcCommands::KICK));
    cmd_map.insert(std::pair<string, CommandFnx>(string("TOPIC"), &AIrcCommands::TOPIC));
    cmd_map.insert(std::pair<string, CommandFnx>(string("INVITE"), &AIrcCommands::INVITE));
    cmd_map.insert(std::pair<string, CommandFnx>(string("MODE"), &AIrcCommands::MODE));
    cmd_map.insert(std::pair<string, CommandFnx>(string("PASS"), &AIrcCommands::PASS));
    cmd_map.insert(std::pair<string, CommandFnx>(string("QUIT"), &AIrcCommands::QUIT));
    cmd_map.insert(std::pair<string, CommandFnx>(string("NAMES"), &AIrcCommands::NAMES));
    cmd_map.insert(std::pair<string, CommandFnx>(string("LIST"), &AIrcCommands::LIST));
    cmd_map.insert(std::pair<string, CommandFnx>(string("PRIVMSG"), &AIrcCommands::PRIVMSG));
    cmd_map.insert(std::pair<string, CommandFnx>(string("WHOIS"), &AIrcCommands::WHOIS));
}

// this might have to manage signals at some point ?? 
int Server::mainLoop(void) {

    setUpPoll();
    while (42) {
        Poll();
        for (int fd_idx = 0; fd_idx < fds_size; fd_idx++) {
            if (skipFd(fd_idx)
                || !hasDataToRead(fd_idx))
            {
                continue;
            }
            /* listener is always at first entry */
            if (fd_idx == 0) {
                int new_fd = acceptConnection();
                const char* ip_address = getSocketAddress(new_fd);
                addNewUser(new_fd, ip_address);
                continue;
            }
            int fd = getFdFromIndex(fd_idx);
            DataFromUser(fd);
        }
        pingLoop();
    }
}

/* 
 * When a user is more than SERVER_PONG_TIME_SEC without sending anything,
 * the server sends a PING <random_10_byte_string> that the user has to
 * reply within SERVER_PONG_TIME_SEC with PONG <random_10_byte_string>.
 * If the user does not send the PONG message in time, the user is 
 * removed. The bytes correspond to printable chars.
 * 
 * See
 * https://stackoverflow.com/questions/14315497/ 
 * 
 */
void Server::pingLoop(void) {
    for (int fd_idx = 0; fd_idx < fds_size; fd_idx++) {
        if (fd_idx == 0 || skipFd(fd_idx)) {
            continue;
        }
        int fd = getFdFromIndex(fd_idx);
        User &user = getUserFromFd(fd);
        if (user.isOnPongHold()) {
            time_t since_ping = time(NULL) - user.getPingTime();
            if (since_ping >= PING_TIMEOUT_S) {
                string reason = "Ping timeout: " PING_TIMEOUT_S_STR " seconds";
                removeUserFromServer(fd, reason);
            }
            continue ;
        }
        time_t since_last_msg = time(NULL) - user.getLastMsgTime();
        if (since_last_msg >= PING_TIMEOUT_S) {
            sendPingToUser(fd);
        }
    }
}

void Server::sendPingToUser(int fd) {

    User &user = getUserFromFd(fd);
    /* send ping message */
    string random = ":" + tools::rngString(10);
    string ping_msg("PING " + random);
    DataToUser(fd, ping_msg, NO_NUMERIC_REPLY);
    user.updatePingStatus(random);
}

void Server::DataFromUser(int fd) {

    srv_buff_size = recv(fd, srv_buff, sizeof(srv_buff), 0);
    if (srv_buff_size == -1) {
        if (socketErrorIsNotFatal(fd)) {
            LOG(WARNING) << "DataFromUser closing fd " << fd
                         << " from user " << getUserFromFd(fd)
                         << " non fatal error";
                string reason = "Internal server error";
                return removeUserFromServer(fd, reason);
        }
        throw irc::exc::FatalError("recv -1");
    }
    if (srv_buff_size == 0) {
        string reason = "Client closed connection";
        return removeUserFromServer(fd, reason);
    }
    /* Update when a user sends a command ! */
    User& user = getUserFromFd(fd);
    if (!user.isOnPongHold()) {
        user.last_received = time(NULL);
    }

    LOG(INFO) << "DataFromUser user " << user
              << ", bytes " << srv_buff_size
              << " content [" << srv_buff << "]";

    string cmd_string = processLeftovers(fd);
    /* cmd_string can be empty here in case there have been buffering 
     * problems with the user */
    if (!cmd_string.empty()) {
        parseCommandBuffer(cmd_string, fd);
    }
}

/* 
 * sends [:<hostname> <msg>CRLF] to user with fd asociated.
 * 
 * Why send() function is controlled as follows : 
 * https://stackoverflow.com/questions/33053507/econnreset-in-send-linux-c
 * This way b_sent = 0 does not have to be controlled, because ECONNRESET
 * will be returned by send in case we try to send to a closed connection
 * twice.
 */
void Server::DataToUser(int fd, string msg, int type) {

    if (type == NUMERIC_REPLY) {
        msg.insert(0, ":" + hostname);
    }
    msg.insert(msg.size(), CRLF);
    
    User& user = getUserFromFd(fd);

    LOG(INFO) << "DataToUser user " << user
              << ", bytes " << msg.size()
              << ", content [" << msg << "]"; 

    int b_sent = 0;
    int total_b_sent = 0;

    do {
        b_sent = send(fd, &msg[b_sent], msg.size() - total_b_sent, 0);
        if (b_sent == -1) {
            if (socketErrorIsNotFatal(fd)) {
                LOG(WARNING) << "DataToUser closing fd " << fd
                             << " from user " << user
                             << " non fatal error";
                string reason = "Internal server error";
                return removeUserFromServer(fd, reason);
            }
            throw irc::exc::FatalError("send = -1");
        }
        /* add bytes sent to total */
        total_b_sent += b_sent;
    /* keep looping until full message is sent */
    } while (total_b_sent != (int)msg.size());
}

/* 
 * Gestiona toda la logica para recoger el comando con buffering (guardar
 * leftovers, agregarlos al comando recibido, eliminarlos si excede la
 * longitud maxima, etc.)
 * El comando más largo que podrá tener en memoria el servidor será de
 * 512 * 2 bytes, que se corresponde con el caso en que un usuario envía
 * un primer comando incompleto (sin CRLF). En el caso en que un comando
 * sumado a los leftovers exceda los 512 bytes, se enviará ERR_INPUTTOOLONG 
 * y el comando será vaciado.
 * Cuando un comando más sus leftovers superan los 512 bytes y la string total
 * no contenga CRLF, se vaciará el comando y no se enviará nada.
 */
string Server::processLeftovers(int fd) {

    User& user = getUserFromFd(fd);

    string cmd_string(srv_buff, srv_buff_size);
    tools::cleanBuffer(srv_buff, srv_buff_size);
    srv_buff_size = 0;

    if (tools::endsWith(cmd_string, CRLF)) {
        /* add leftovers at start of buffer recieved */
        if (user.hasLeftovers()) {
            cmd_string.insert(0, user.BufferToString());
            user.resetBuffer();
        }
        /* total buffer is too big */
        if (cmd_string.length() > BUFF_MAX_SIZE) {
            string reply(ERR_INPUTTOOLONG+user.nick+STR_INPUTTOOLONG);
            LOG(WARNING) << "Buffer from User [" << user.nick << "] too long";
            DataToUser(fd, reply, NUMERIC_REPLY);
            return "";
        }
    // cmd_string stays as it is.
    } else {
        size_t pos = tools::findLastCRLF(cmd_string);
        // no CRLF found
        if (pos == string::npos) {
            // ill-formated long comand
            if (cmd_string.length() + user.buffer_size > BUFF_MAX_SIZE) {
                user.resetBuffer();
                LOG(WARNING) << "Ill formatted buffer from user " << user;
                return "";
            }
            /* buffer has space left : save and return empty command */
            user.addLeftovers(cmd_string);
            return "";
        }
        /* if CRLF is somewhere, construct comand until last CRLF
         * and save leftovers */
        string leftovers = cmd_string.substr(pos);
        user.addLeftovers(leftovers);
        cmd_string = cmd_string.substr(0, pos);
    }
    return cmd_string;
}

/*
 * Recieves full buffer from user, wether it contains or not leftovers,
 * then splits it in CRLF. Each different command is then processed 
 * matching the first word (or second in case user prefix is first)
 * with a command name, sequentially. 
 * - Empty commands are ignored (CMD1 CRLFCRLFCRLF CMD2) will call 
 * CMD1 and CMD2, without raising an error.
 * - Commands name DO NOT have to be in upper case letters, this is
 * done internally. joIN &channel is the same as JOIN &channel.
 * - If a command name does not match any on the command map, an
 * error is raised. This is a prior check to user registration.
 *
 */
void Server::parseCommandBuffer(string &cmd_content, int fd) {
    
    vector<string> cmd_vector;
    User& user = getUserFromFd(fd);

    tools::split(cmd_vector, cmd_content, CRLF);
    int cmd_vector_size = cmd_vector.size();
    for (int i = 0; i < cmd_vector_size; i++) {
        Command command;
        if (command.Parse(cmd_vector[i]) != command.OK) {
            continue ;
        }
        /* command does not exist / ill formatted command */
        if (!cmd_map.count(command.Name())) {
            string msg(ERR_UNKNOWNCOMMAND+command.Name()+STR_UNKNOWNCOMMAND);
            DataToUser(fd, msg, NUMERIC_REPLY);
            continue ;
        }
        if (user.isOnPongHold() && command.Name().compare("PONG")) {
            continue ;
        }
        CommandMap::iterator it = cmd_map.find(command.Name());
        (*this.*it->second)(command, fd);
    }
}

bool Server::serverHasPassword(void) {
    return !password.empty();
}

void Server::maybeRegisterUser(User &user) {
    if (user.isReadyForRegistration(serverHasPassword())) {
        registerUser(user);
    }
}

/* This function is an entry point for a user which has all
 * necessary fields for registration.
 */
void Server::registerUser(User &user) {

    if (user.last_password == password) { // always true if password not set
        user.setPrefixFromHost(hostname);
        user.registered = true;
        user.addServerMask(OP);
        return sendWelcome(user.name, user.prefix, user.fd);
    } else {
        sendPasswordMismatch(user.real_nick, user.fd);
        string reason = "Password missmatch";
        removeUserFromServer(user.fd, reason);
    }
}

} /* namespace irc */

