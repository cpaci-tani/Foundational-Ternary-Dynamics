// app/app_util.cpp — portable string/format helpers (see app/app_util.h).

#include "app/app_util.h"

#include <cctype>
#include <cstdio>

namespace ftd::native::app {

std::vector<std::string> split_csv(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == ',') {
            if (!cur.empty()) out.push_back(cur);
            cur.clear();
        } else if (c != ' ') {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

std::string to_lower(std::string s) {
    for (char& c : s) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string upper(std::string s) {
    for (char& c : s) c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    return s;
}

std::string fmt(const char* f, double v) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), f, v);
    return buf;
}

std::string fmt3(const char* f, double a, double b, double c) {
    char buf[96];
    std::snprintf(buf, sizeof(buf), f, a, b, c);
    return buf;
}

}  // namespace ftd::native::app
