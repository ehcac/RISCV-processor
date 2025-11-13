#include "utils.hpp"

unsigned int binToUint(const string& bin) {
    return stoul(bin, nullptr, 2);
}

int getRegisterNumber(const string& reg) {
    if (reg.length() > 1 && reg[0] == 'x') {
        try {
            int num = stoi(reg.substr(1));
            return (num >= 0 && num <= 31) ? num : -1;
        } catch (...) { return -1; }
    }
    return -1;
}

int getImmediateValue(const string& immStr) {
    try {
        if (immStr.size() > 2 && immStr.substr(0, 2) == "0x")
            return stoul(immStr, nullptr, 16);
        else return stoi(immStr);
    } catch (...) { return 999999999; }
}

vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        token.erase(remove_if(token.begin(), token.end(), ::isspace), token.end());
        if (!token.empty()) tokens.push_back(token);
    }
    return tokens;
}
