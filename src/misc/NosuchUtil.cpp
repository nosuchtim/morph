#ifdef _WIN32
#include <winsock.h>
#endif

#include <stdint.h>
#include "NosuchUtil.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <locale>
#include <sstream>
#include <string>

#include "string.h"

using namespace std;

#define NOSUCHMAXSTRING 8096

std::vector<std::string> NosuchSplitOnAnyChar(std::string s, std::string sepchars)
{
	std::vector<std::string> result;
	const char *seps = sepchars.c_str();
	const char *str = s.c_str();

    while(1) {
        const char *begin = str;
		while( strchr(seps,*str) == NULL && *str) {
                str++;
		}
        result.push_back(std::string(begin, str));
		if(0 == *str) {
			break;
		}
		// skip one or more of the sepchars
		while( strchr(seps,*str) != NULL && *str ) {
			str++;
		}
    }
    return result;
}

std::vector<std::string> NosuchSplitOnString(const std::string& s, const std::string& delim, const bool keep_empty) {
	std::vector<std::string> result;
    if (delim.empty()) {
        result.push_back(s);
        return result;
    }
    std::string::const_iterator substart = s.begin(), subend;
    while (true) {
        subend = search(substart, s.end(), delim.begin(), delim.end());
		std::string temp(substart, subend);
        if (keep_empty || !temp.empty()) {
            result.push_back(temp);
        }
        if (subend == s.end()) {
            break;
        }
        substart = subend + delim.size();
    }
    return result;
}

