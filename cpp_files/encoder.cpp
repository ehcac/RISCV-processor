#include "../hpp_files/encoder.hpp"

// Assembler Phase 3: Encoding Functions

/**
 * R-Type Instruction Format: [31:25 funct7] [24:20 rs2] [19:15 rs1] [14:12 funct3] [11:7 rd] [6:0 opcode]
 */
unsigned int encodeRType(string rd, string rs1, string rs2, string f3, string f7, string op) {
    unsigned int machineCode = 0;
    
    unsigned int u_rd = getRegisterNumber(rd);
    unsigned int u_rs1 = getRegisterNumber(rs1);
    unsigned int u_rs2 = getRegisterNumber(rs2);
    unsigned int u_f3 = binToUint(f3);
    unsigned int u_f7 = binToUint(f7);
    unsigned int u_op = binToUint(op);

    // Assembly: (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode
    machineCode |= (u_f7 << 25);
    machineCode |= (u_rs2 << 20);
    machineCode |= (u_rs1 << 15);
    machineCode |= (u_f3 << 12);
    machineCode |= (u_rd << 7);
    machineCode |= u_op;

    return machineCode;
}

/**
 * I-Type Instruction Format: [31:20 imm] [19:15 rs1] [14:12 funct3] [11:7 rd] [6:0 opcode]
 */
unsigned int encodeIType(string rd, string rs1, int imm, string f3, string op, const string& mnemonic) {
    unsigned int machineCode = 0;
    
    unsigned int u_rd = getRegisterNumber(rd);
    unsigned int u_rs1 = getRegisterNumber(rs1);
    unsigned int u_f3 = binToUint(f3);
    unsigned int u_op = binToUint(op);

    unsigned int immediateValue;
    if (mnemonic == "slli") {
        // SLLI: immediate (shamt) - [24:20]; funct7 - [31:25].
        // RISC-V pseudo-I-Type: [31:25 funct7] [24:20 shamt] [19:15 rs1] [14:12 funct3] [11:7 rd] [6:0 opcode]
        unsigned int shamt = imm & 0b11111; // 5-bit immediate
        unsigned int u_f7 = binToUint(INSTRUCTION_SET.at(mnemonic).f7);

        machineCode |= (u_f7 << 25);
        machineCode |= (shamt << 20);
    } else {
        // Standard I-Type: 12-bit immediate in [31:20]
        immediateValue = imm & 0xFFF; 
        machineCode |= (immediateValue << 20);
    }


    // Assembly: (imm/shamt/f7 << 20/25) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode
    machineCode |= (u_rs1 << 15);
    machineCode |= (u_f3 << 12);
    machineCode |= (u_rd << 7);
    machineCode |= u_op;

    return machineCode;
}

/**
 * S-Type Instruction Format: [31:25 imm[11:5]] [24:20 rs2] [19:15 rs1] [14:12 funct3] [11:7 imm[4:0]] [6:0 opcode]
 */
unsigned int encodeSType(string rs1, string rs2, int imm, string f3, string op) {
    unsigned int machineCode = 0;
    
    unsigned int u_rs1 = getRegisterNumber(rs1);
    unsigned int u_rs2 = getRegisterNumber(rs2);
    unsigned int u_f3 = binToUint(f3);
    unsigned int u_op = binToUint(op);

    // Immediate split
    unsigned int imm_11_5 = (imm >> 5) & 0b1111111; // Bits [11..5] 
    unsigned int imm_4_0 = imm & 0b11111; // Bits [4..0] 

    // Assembly: (imm[11:5] << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm[4:0] << 7) | opcode
    machineCode |= (imm_11_5 << 25);
    machineCode |= (u_rs2 << 20);
    machineCode |= (u_rs1 << 15);
    machineCode |= (u_f3 << 12);
    machineCode |= (imm_4_0 << 7);
    machineCode |= u_op;

    return machineCode;
}

/**
 * B-Type Instruction Format: [31 imm[12]] [30:25 imm[10:5]] [24:20 rs2] [19:15 rs1] [14:12 funct3] [11:8 imm[4:1]] [7 imm[11]] [6:0 opcode]
 */
unsigned int encodeBType(string rs1, string rs2, int imm, string f3, string op) {
    unsigned int machineCode = 0;
    
    unsigned int u_rs1 = getRegisterNumber(rs1);
    unsigned int u_rs2 = getRegisterNumber(rs2);
    unsigned int u_f3 = binToUint(f3);
    unsigned int u_op = binToUint(op);

    // imm: [12 | 10:5 | 4:1 | 11] (total 12 bits + implicit 0)
    unsigned int imm_12 = (imm >> 12) & 0b1; // [12]
    unsigned int imm_10_5 = (imm >> 5) & 0b111111; // [10..5]
    unsigned int imm_4_1 = (imm >> 1) & 0b1111; // [4..1]
    unsigned int imm_11 = (imm >> 11) & 0b1; // [11]

    // Assembly: 
    // (imm[12] << 31) | (imm[10:5] << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (imm[4:1] << 8) | (imm[11] << 7) | opcode
    machineCode |= (imm_12 << 31);
    machineCode |= (imm_10_5 << 25);
    machineCode |= (u_rs2 << 20);
    machineCode |= (u_rs1 << 15);
    machineCode |= (u_f3 << 12);
    machineCode |= (imm_4_1 << 8);
    machineCode |= (imm_11 << 7);
    machineCode |= u_op;

    return machineCode;
}

/**
 * J-Type Instruction Format: [31 imm[20]] [30:21 imm[10:1]] [20 imm[11]] [19:12 imm[19:12]] [11:7 rd] [6:0 opcode]
 */
unsigned int encodeJType(string rd, int imm, string op) {
    unsigned int machineCode = 0;
    
    unsigned int u_rd = getRegisterNumber(rd);
    unsigned int u_op = binToUint(op);

    // Imm: [20 | 10:1 | 11 | 19:12] (total 20 bits + implicit 0)
    unsigned int imm_20 = (imm >> 20) & 0b1; // [20]
    unsigned int imm_10_1 = (imm >> 1) & 0b1111111111; // [10..1]
    unsigned int imm_11 = (imm >> 11) & 0b1; // [11]
    unsigned int imm_19_12 = (imm >> 12) & 0b11111111; // [19..12]

    // Assembly: 
    // (imm[20] << 31) | (imm[10:1] << 21) | (imm[11] << 20) | (imm[19:12] << 12) | (rd << 7) | opcode
    machineCode |= (imm_20 << 31);
    machineCode |= (imm_10_1 << 21);
    machineCode |= (imm_11 << 20);
    machineCode |= (imm_19_12 << 12);
    machineCode |= (u_rd << 7);
    machineCode |= u_op;

    return machineCode;
}

map<unsigned int, unsigned int> translateToOpcode(const vector<ParsedInstruction>& instructions) {
    map<unsigned int, unsigned int> opcodeMap;

    for (const ParsedInstruction& inst : instructions) {
        const string& mnemonic = inst.mnemonic;
        const vector<string>& ops = inst.operands;
        const unsigned int address = inst.address;

        const InstructionInfo& info = INSTRUCTION_SET.at(mnemonic);
        unsigned int opcode = 0;

        if (info.type == "R") {
            // R-Type: rd, rs1, rs2 (e.g., add x1, x2, x3)
            opcode = encodeRType(ops[0], ops[1], ops[2], info.f3, info.f7, info.op);

        } else if (info.type == "I" && mnemonic != "lw" && mnemonic != "jalr") {
            // Standard I-Type: rd, rs1, imm (e.g., addi x1, x2, 100)
            int imm = getImmediateValue(ops[2]);
            if (imm == 999999999) { } 
            
            opcode = encodeIType(ops[0], ops[1], imm, info.f3, info.op, mnemonic);
            
        } else if (mnemonic == "lw" || mnemonic == "jalr") {
            // Load I-Type: rd, imm(rs1) -> ops: rd, rs1, imm
            // JALR I-Type: rd, imm(rs1) -> ops: rd, rs1, imm (often rd, rs1, 0)
            int imm = getImmediateValue(ops[2]);
            opcode = encodeIType(ops[0], ops[1], imm, info.f3, info.op, mnemonic);
            
        } else if (info.type == "S") {
            // S-Type: rs2, imm(rs1) -> ops: rs2, rs1, imm
            int imm = getImmediateValue(ops[2]);
            opcode = encodeSType(ops[1], ops[0], imm, info.f3, info.op); // Note: rs1/rs2 swap for S-type register order
            
        } else if (info.type == "B") {
            // B-Type: rs1, rs2, label -> ops: rs1, rs2, label
            string label = ops[2];
            unsigned int targetAddress = SYMBOL_TABLE.at(label);
            // PC-relative immediate calculation: imm = Target - Current PC
            int imm = (int)targetAddress - (int)address; 
            opcode = encodeBType(ops[0], ops[1], imm, info.f3, info.op);

        } else if (info.type == "J") {
            // J-Type: rd, label -> ops: rd, label
            string label = ops[1];
            unsigned int targetAddress = SYMBOL_TABLE.at(label);
            // PC-relative immediate calculation: imm = Target - Current PC
            int imm = (int)targetAddress - (int)address; 
            opcode = encodeJType(ops[0], imm, info.op);

        } else {
            cerr << "FATAL ERROR: Unhandled instruction type for " << mnemonic << " at 0x" << hex << address << endl;
            exit(1);
        }

        opcodeMap[address] = opcode;
    }
    return opcodeMap;
}