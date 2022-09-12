#include "Command.hpp"
#include <string>
#include <vector>
#include "Tools.hpp"
#include <iostream>


using std::string;
using std::vector;

namespace irc {

Command::Command(void)
{}

Command::~Command() 
{}

/* Le llega el comando sin CRLF. Será el servidor el
 * encargado de splitear los buffer en CRLF.
 * Esto es necesario ya que cada commando responderá a un
 * nombre, y cada nombre a una función. Entonces, la mecánica
 * es ir pasando, dado un buffer, todas las líneas separadas
 * por CRLF de una en una, y que el servidor vaya llamando
 * a su CommandMap[command.name], hasta que se acabe el
 * tamaño del buffer o se de un error.
 * De esta forma también evitamos tener que gestionar CRLF a secas
 * dentro de Parse, ya que según el RFC los comandos vacíos 
 * deben ignorarse.
 */
int Command::Parse(string &cmd) {

    if (newlines_left(cmd)) {
        return ERR_NEWLINES;
    }
    if (colon_placed_incorrectly(cmd)) {
        return ERR_COLONS;
    }

    cmd = tools::trimRepeatedChar(cmd, ' ');
    if (cmd[0] == ':') {
        int prefix_end = cmd.find(" ");
        // eliminate prefix in case specified
        if (prefix_end != -1) {
            cmd = cmd.substr(prefix_end);
        }
    }
    
    vector<string> colon_split;
    vector<string> space_split;

    colon_split = tools::split(colon_split, cmd, ":");
    /* If there was a colon split, only split in spaces first arg. */
    if (colon_split.size() == 2) {
        /* Never loose the semicolon ! */
        colon_split[1].insert(0, ":");
        space_split = tools::split(space_split, colon_split[0], " ");
    /* else just split in spaces original command */
    } else {
        space_split = tools::split(space_split, cmd, " ");
    }
    /* from irc-hispano does this */
    tools::ToUpperCase(space_split[0]);
    
    args = space_split;
    if (colon_split.size() == 2) {
        args.push_back(colon_split[1]);
    }
    debugCommand();
    return OK;
}

string &Command::Name() {
    return args[0];
}

/* Checks that the buffer recieved, in case it has a colon, 
 * it is places separating two pieces of text.
 * (1) msg = * : * OK 
 * (2) msg = :* * :* OK
 * (3) msg = :* * OK
 * (4) msg = *
 * anything else throws an error. Second example corresponds
 * to some clients that send :user_info as a prefix to all
 * messages (must be ignored, but still).
 */
bool Command::colon_placed_incorrectly(string &str) {
    
    vector<string> result;
    string del(":");
    tools::split(result, str, del);
    size_t size = result.size();
    if (size < 1 || size > 2) {
        return true;
    }
    if (result[0].empty()) {
        return true;
    }
    /* si no viene despues de un espacio, tampoco vale. Basta con
     * ver si la penúltima string acaba en espacio. Esto
     * es funcdamental de cara a hacer primero un split de ':' y
     * luego uno de ' ' */
    if (size == 2) {
        if ((result[0])[result[0].size() - 1] != ' ') {
            return true;
        }
    }
    return false;
}

bool Command::newlines_left(string &str) {
    for (string::iterator it = str.begin(); it < str.end(); it++) {
        if (*it == '\n' || *it == '\r')
            return true;
    }
    return false;
}


void Command::debugCommand() const {
    std::cout << std::endl << "COMMAND RESULT : " << std::endl;
    size_t size = args.size();
    std::cout << "Command vector size : " << args.size();
    std::cout << ", content :" << std::endl;
    for (size_t it = 0; it < size; it++) {
        std::cout << it << ": [" << args[it] << "]" << std::endl;
    }
    std::cout << std::endl;
}


} // namespace
