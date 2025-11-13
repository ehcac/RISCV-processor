#ifndef UTILS_HPP
#define UTILS_HPP

#include "assembler.hpp"

unsigned int binToUint(const string& bin);
int getRegisterNumber(const string& reg);
int getImmediateValue(const string& immStr);
vector<string> split(const string& s, char delimiter);

#endif
