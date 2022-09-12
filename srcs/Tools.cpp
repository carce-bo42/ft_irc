#include "Tools.hpp"
#include "Exceptions.hpp"
#include "Types.hpp"

#include <vector>
#include <string>
#include <iostream>
#include <ctime>

#include "libft.h"

extern "C" {
#include "string.h"
}

using std::string;
using std::vector;

namespace irc {
namespace tools {

vector<string>& split(vector<string> &to_fill, string &str, string del) {
    int start = 0;
    int end = str.find(del);
    while (end != -1) {
        to_fill.push_back(str.substr(start, end - start));
        start = end + del.size();
        end = str.find(del, start);
    }
    string last = str.substr(start, end - start);
    if (!last.empty()) {
        to_fill.push_back(str.substr(start, end - start));
    }
    return to_fill;
}

string& trimRepeatedChar(string& str, char c) {
    string clean_str;
    size_t src_size = str.size();
    clean_str.reserve(src_size); // max possible
    
    for (size_t i = 0; i < src_size; i++) {
        clean_str.push_back(str[i]);
        if (str[i] == c) {
            while (str[i] == c) {
                i++;
            }
            i--; // counter increment
        }
    }
    str = clean_str;
    return str;
}

/*
 * Replaces all <from> occurrences in string str for <to>.
 * From https://stackoverflow.com/questions/2896600/
 */
void ReplaceAll(string& str, const string& from, const string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
}

/*
 * Each lowercase letter is 32 + uppercase equivalent. This means simply
 * flipping the bit at position 5 (counting from least significant bit
 * at position 0) inverts the case of a letter.
 */
void ToUpperCase(string &str) {
    for (string::iterator it = str.begin(); it < str.end(); it++) {
        if (*it >= 'a' && *it <= 'z') {
            *it ^= 0x20;
        }
    }
}

/* checks if a string is equal to another, ignoring case
 * differences (HellO = hELlo) */
bool isEqual(const string &str1, const string &str2) {
    if (str1.length() != str2.length()) {
        return false;
    }
    size_t len = str1.length();
    for (size_t i = 0; i < len; i++) {
        if (std::tolower(str1[i]) != std::tolower(str2[i])) {
            return false;
        }
    }
    return true;
}

/* Check if str ends with suffix */
bool endsWith(string const &str, string const &suffix) {
    if (str.length() < suffix.length()) {
        return false;
    }
    return (str.compare(str.length() - suffix.length(),
                        suffix.length(), suffix) == 0);
}

/* Check if str starts with suffix */
bool starts_with_mask(string const str) {
    return (str[0] == '!' || str[0] == '#' || str[0] == '+' || str[0] == '&');
}

bool hasUnknownChannelFlag(const string &mode) {
    return ((mode.find("-") == string::npos
        && mode.find("+") == string::npos)
       || (mode.find("o") == string::npos
       && mode.find("i") == string::npos
       && mode.find("k") == string::npos
       && mode.find("m") == string::npos
       && mode.find("b") == string::npos
       && mode.find("v") == string::npos)
       || (mode.find("-") != string::npos
       && mode.find("+") != string::npos));
}

/* Check if a char is inside a string */
bool charIsInString(const string &str, const char c) {
    return (str.find(c) != string::npos);
}

/*
 * from https://stackoverflow.com/questions/440133
 * See https://cplusplus.com/reference/cstdlib/rand/
 */
string rngString(int len) {
    srand((unsigned)time(NULL) * getpid());
    static const unsigned char printables[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "!$/%()=?^_;\\|@#~[]{},-'+";
    string tmp_s;
    tmp_s.reserve(len);
    for (int i = 0; i < len; ++i) {
        unsigned char c = printables[rand() % (sizeof(printables) - 1)];
        tmp_s.push_back(c);
    }
    return tmp_s;
}

/*
 * returns the index corresponding to the last crlf, e.g. :
 * from "Hello CRLF lol", returns the position 5.
 * This is done to then call substr(0, pos),
 * which will be the contents of the string before
 * the last CRLF.
 */
size_t findLastCRLF(string& haystack) {
    size_t CR_pos = haystack.find_last_of(CR);
    size_t LF_pos = haystack.find_last_of(LF);

    if (CR_pos != string::npos
        && LF_pos != string::npos)
    {
        if (LF_pos == CR_pos + 1) {
            return CR_pos - 1;
        }
    }
    return string::npos;
}


void cleanBuffer(char *buff, size_t size) {
    if (size > 0 && buff != NULL) {
        ft_memset(buff, '\0', size);
    }
}

void printError(string error_str) {
    std::cerr << error_str << std::endl;
}

bool anyRepeatedChar(string &s) {
    int len = s.length();

    for (int i = 0; i <= len; ++i) {
        for (int j = i+1; j <= len; ++j) {
            if (s[j] == s[i]) {
                return true;
            }
        }
    }
    return false;
}

} // tools 
} // irc
