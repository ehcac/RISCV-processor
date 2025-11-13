#include "assembler.hpp"

//minimum instructions: LW, SW, SLT, SLL, SLLI, BEQ, BLT
map<string, InstructionInfo> INSTRUCTION_SET = {
    {"add",  {"R", "0110011", "000", "0000000"}},
    {"sub",  {"R", "0110011", "000", "0100000"}},
    {"sll",  {"R", "0110011", "001", "0000000"}},
    {"slt",  {"R", "0110011", "010", "0000000"}},
    {"and",  {"R", "0110011", "111", "0000000"}},

    {"addi", {"I", "0010011", "000"}},
    {"slli", {"I", "0010011", "001", "0000000"}},
    {"lw",   {"I", "0000011", "010"}},
    {"jalr", {"I", "1100111", "000"}},

    {"sw",   {"S", "0100011", "010"}},
    {"beq",  {"B", "1100011", "000"}},
    {"blt",  {"B", "1100011", "100"}},
    {"jal",  {"J", "1101111", "000"}},
};

map<unsigned int, unsigned int> INSTRUCTION_MEMORY;
map<string, unsigned int> SYMBOL_TABLE;
