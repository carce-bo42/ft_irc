#ifndef IRC42_COMMAND_H
# define IRC42_COMMAND_H

#include <list>
#include <string>
#include <vector>
#include "Types.hpp"


namespace irc {

/* Managea cada comando que llegue. Está dedicado a resolver
 * un comando en exclusive, i.e. [CMDNAME <args ... > : ... ]
 */
class Command {

    public:
    Command();
    ~Command();

    typedef enum PARSE_RESULT {
        OK = 0,
        ERR_NO_COMMAND,
        ERR_COLONS,
        ERR_NEWLINES
    } PARSE_RESULT;

    /* construye el vector de argumentos. Divide las strings segun
     * los espacios, hasta llegar a un ':'. Lo que venga después de ':'
     * va en un solo argumento, junto a ':'.
     * Si un comando es válido sólo una string contendrá el valor ':'.
     */
    int Parse(std::string &cmd);
    std::string &Name();

    /* Esto tendrá que tener en su momento, una especie de mapa s.t.:
     * [USER] -> [function que managea user]
     * [NICK] -> [funcion que managea nick]
     * .
     * .
     * .
     */
    std::vector<std::string> args;

    private:
    void debugCommand() const;
    bool colon_placed_incorrectly(std::string &str);
    bool newlines_left(std::string &str);
};

} // namespace

#endif /*IRC42_COMMAND_H*/
