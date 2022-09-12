#ifndef IRC42_TOOLS_H
# define IRC42_TOOLS_H

#include <vector>
#include <string>

namespace irc {
namespace tools {

std::vector<std::string>& split(std::vector<std::string> &to_fill,
                                std::string &str, std::string del);

bool starts_with_mask(std::string const);
void ToUpperCase(std::string &str);
bool isEqual(const std::string &str1, const std::string &str2);
bool endsWith(std::string const &str, std::string const &suffix);
bool hasUnknownChannelFlag(const std::string &mode);
bool charIsInString(const std::string &str, const char c);
bool anyRepeatedChar(std::string &s);

std::string& trimRepeatedChar(std::string& str, char c);
void ReplaceAll(std::string& str, const std::string& from,
                                  const std::string& to);
size_t findLastCRLF(std::string& haystack);

void cleanBuffer(char *buff, size_t size);
void printError(std::string error_str);

std::string rngString(int len);

} // tools
} // irc

#endif /* IRC42_TOOLS_H */
