#ifndef ENCODER_HPP
#define ENCODER_HPP

#include "assembler.hpp"
#include "utils.hpp"

unsigned int encodeRType(string rd, string rs1, string rs2, string f3, string f7, string op);
unsigned int encodeIType(string rd, string rs1, int imm, string f3, string op, const string& mnemonic);
unsigned int encodeSType(string rs1, string rs2, int imm, string f3, string op);
unsigned int encodeBType(string rs1, string rs2, int imm, string f3, string op);
unsigned int encodeJType(string rd, int imm, string op);
map<unsigned int, unsigned int> translateToOpcode(const vector<ParsedInstruction>& instructions);

#endif
