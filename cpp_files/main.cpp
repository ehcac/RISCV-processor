#include "../hpp_files/assembler.hpp"
#include "../hpp_files/parser.hpp"
#include "../hpp_files/encoder.hpp"
#include "../hpp_files/simulator.hpp"
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#include <sstream>

using namespace emscripten;

// Global simulator instance
RISCV_Simulator* globalSim = nullptr;
vector<ParsedInstruction> globalInstructions;
bool isInitialized = false;

// Structure to hold pipeline state for JS
struct PipelineStateJS {
    // IF/ID
    uint32_t if_id_pc;
    uint32_t if_id_ir;
    uint32_t if_id_npc;
    
    // ID/EX
    uint32_t id_ex_ir;
    int32_t id_ex_a;
    int32_t id_ex_b;
    int32_t id_ex_imm;
    uint32_t id_ex_npc;
    
    // EX/MEM
    uint32_t ex_mem_ir;
    int32_t ex_mem_aluoutput;
    uint32_t ex_mem_b;
    bool ex_mem_cond;
    
    // MEM/WB
    uint32_t mem_wb_ir;
    int32_t mem_wb_aluoutput;
    int32_t mem_wb_lmd;
    uint8_t mem_wb_rd;
    bool mem_wb_regwrite;
};

// Initialize the simulator with assembly code
std::string initializeSimulator(std::string assemblyCode) {
    try {
        // Clean up existing simulator
        if (globalSim != nullptr) {
            delete globalSim;
            globalSim = nullptr;
        }
        
        // Clear global state
        INSTRUCTION_MEMORY.clear();
        SYMBOL_TABLE.clear();
        DATA_SEGMENT.clear();
        globalInstructions.clear();
        
        // Write assembly to temporary processing
        std::istringstream stream(assemblyCode);
        vector<string> lines;
        string line;
        
        while (getline(stream, line)) {
            // Remove comments
            size_t commentPos = line.find('#');
            if (commentPos != string::npos) {
                line = line.substr(0, commentPos);
            }
            
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        
        if (lines.empty()) {
            return "ERROR: No valid assembly code provided";
        }
        
        // Build symbol table and parse
        SYMBOL_TABLE = buildSymbolTable(lines);
        parseDataSection(lines);
        globalInstructions = parseInstructions(lines);
        
        // Translate to opcodes
        INSTRUCTION_MEMORY = translateToOpcode(globalInstructions);
        
        // Create simulator
        globalSim = new RISCV_Simulator(INSTRUCTION_MEMORY);
        
        // Load data segment
        if (!DATA_SEGMENT.empty()) {
            for (auto const& [addr, val] : DATA_SEGMENT) {
                globalSim->set_memory(addr,     val & 0xFF);
                globalSim->set_memory(addr + 1, (val >> 8) & 0xFF);
                globalSim->set_memory(addr + 2, (val >> 16) & 0xFF);
                globalSim->set_memory(addr + 3, (val >> 24) & 0xFF);
            }
        }
        
        isInitialized = true;
        return "SUCCESS: Simulator initialized with " + std::to_string(globalInstructions.size()) + " instructions";
        
    } catch (const std::exception& e) {
        return std::string("ERROR: ") + e.what();
    }
}

// Execute one cycle
std::string stepSimulator() {
    if (!isInitialized || globalSim == nullptr) {
        return "ERROR: Simulator not initialized";
    }
    
    try {
        globalSim->step();
        return "SUCCESS: Executed 1 cycle";
    } catch (const std::exception& e) {
        return std::string("ERROR: ") + e.what();
    }
}

// Run until completion (max 10000 cycles for safety)
std::string runSimulator() {
    if (!isInitialized || globalSim == nullptr) {
        return "ERROR: Simulator not initialized";
    }
    
    try {
        int maxCycles = 10000;
        int cyclesRun = 0;
        uint32_t lastAddr = globalInstructions.back().address + 4;
        
        while (globalSim->get_pc() <= lastAddr && cyclesRun < maxCycles) {
            globalSim->step();
            cyclesRun++;
        }
        
        return "SUCCESS: Executed " + std::to_string(cyclesRun) + " cycles";
    } catch (const std::exception& e) {
        return std::string("ERROR: ") + e.what();
    }
}

// Reset simulator
std::string resetSimulator() {
    if (!isInitialized || globalSim == nullptr) {
        return "ERROR: Simulator not initialized";
    }
    
    try {
        delete globalSim;
        globalSim = new RISCV_Simulator(INSTRUCTION_MEMORY);
        
        // Reload data segment
        if (!DATA_SEGMENT.empty()) {
            for (auto const& [addr, val] : DATA_SEGMENT) {
                globalSim->set_memory(addr,     val & 0xFF);
                globalSim->set_memory(addr + 1, (val >> 8) & 0xFF);
                globalSim->set_memory(addr + 2, (val >> 16) & 0xFF);
                globalSim->set_memory(addr + 3, (val >> 24) & 0xFF);
            }
        }
        
        return "SUCCESS: Simulator reset";
    } catch (const std::exception& e) {
        return std::string("ERROR: ") + e.what();
    }
}

// Get current PC
uint32_t getPC() {
    if (!isInitialized || globalSim == nullptr) return 0;
    return globalSim->get_pc();
}

// Get register value
int32_t getRegister(int idx) {
    if (!isInitialized || globalSim == nullptr) return 0;
    if (idx < 0 || idx > 31) return 0;
    return globalSim->get_reg(idx);
}

// Set register value
std::string setRegister(int idx, int32_t value) {
    if (!isInitialized || globalSim == nullptr) {
        return "ERROR: Simulator not initialized";
    }
    if (idx < 1 || idx > 31) {
        return "ERROR: Invalid register index (must be 1-31)";
    }
    
    globalSim->set_reg(idx, value);
    return "SUCCESS: Register x" + std::to_string(idx) + " set to " + std::to_string(value);
}

// Get memory byte
uint8_t getMemoryByte(int addr) {
    if (!isInitialized || globalSim == nullptr) return 0;
    if (addr < 0 || addr > 127) return 0;
    return globalSim->get_mem(addr);
}

// Get memory word (32-bit)
int32_t getMemoryWord(int addr) {
    if (!isInitialized || globalSim == nullptr) return 0;
    if (addr < 0 || addr > 124) return 0;
    
    return globalSim->get_mem(addr) | 
           (globalSim->get_mem(addr+1) << 8) | 
           (globalSim->get_mem(addr+2) << 16) | 
           (globalSim->get_mem(addr+3) << 24);
}

// Set memory byte
std::string setMemoryByte(int addr, uint8_t value) {
    if (!isInitialized || globalSim == nullptr) {
        return "ERROR: Simulator not initialized";
    }
    if (addr < 0 || addr > 127) {
        return "ERROR: Invalid memory address (must be 0-127)";
    }
    
    globalSim->set_memory(addr, value);
    return "SUCCESS: Memory[" + std::to_string(addr) + "] set to " + std::to_string(value);
}

// Set memory word (32-bit)
std::string setMemoryWord(int addr, int32_t value) {
    if (!isInitialized || globalSim == nullptr) {
        return "ERROR: Simulator not initialized";
    }
    if (addr < 0 || addr > 124) {
        return "ERROR: Invalid memory address (must be 0-124)";
    }
    
    globalSim->set_memory(addr,     value & 0xFF);
    globalSim->set_memory(addr + 1, (value >> 8) & 0xFF);
    globalSim->set_memory(addr + 2, (value >> 16) & 0xFF);
    globalSim->set_memory(addr + 3, (value >> 24) & 0xFF);
    
    return "SUCCESS: Memory[" + std::to_string(addr) + "] (word) set to " + std::to_string(value);
}

// Get pipeline state
PipelineStateJS getPipelineState() {
    PipelineStateJS state;
    
    if (!isInitialized || globalSim == nullptr) {
        memset(&state, 0, sizeof(state));
        return state;
    }
    
    IF_ID if_id = globalSim->get_if_id();
    ID_EX id_ex = globalSim->get_id_ex();
    EX_MEM ex_mem = globalSim->get_ex_mem();
    MEM_WB mem_wb = globalSim->get_mem_wb();
    
    state.if_id_pc = if_id.PC;
    state.if_id_ir = if_id.IR;
    state.if_id_npc = if_id.NPC;
    
    state.id_ex_ir = id_ex.IR;
    state.id_ex_a = id_ex.A;
    state.id_ex_b = id_ex.B;
    state.id_ex_imm = id_ex.IMM;
    state.id_ex_npc = id_ex.NPC;
    
    state.ex_mem_ir = ex_mem.IR;
    state.ex_mem_aluoutput = ex_mem.ALUOutput;
    state.ex_mem_b = ex_mem.B;
    state.ex_mem_cond = ex_mem.cond;
    
    state.mem_wb_ir = mem_wb.IR;
    state.mem_wb_aluoutput = mem_wb.ALUOutput;
    state.mem_wb_lmd = mem_wb.LMD;
    state.mem_wb_rd = mem_wb.rd;
    state.mem_wb_regwrite = mem_wb.RegWrite;
    
    return state;
}

// Get assembly listing
std::string getAssemblyListing() {
    if (!isInitialized || globalInstructions.empty()) {
        return "";
    }
    
    std::stringstream ss;
    for (const ParsedInstruction& inst : globalInstructions) {
        unsigned int opcode = INSTRUCTION_MEMORY.at(inst.address);
        ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << inst.address
           << " | 0x" << std::hex << std::setw(8) << std::setfill('0') << opcode
           << " | " << inst.originalLine << "\n";
    }
    
    return ss.str();
}

// Emscripten bindings
EMSCRIPTEN_BINDINGS(riscv_simulator) {
    emscripten::function("initializeSimulator", &initializeSimulator);
    emscripten::function("stepSimulator", &stepSimulator);
    emscripten::function("runSimulator", &runSimulator);
    emscripten::function("resetSimulator", &resetSimulator);
    emscripten::function("getPC", &getPC);
    emscripten::function("getRegister", &getRegister);
    emscripten::function("setRegister", &setRegister);
    emscripten::function("getMemoryByte", &getMemoryByte);
    emscripten::function("getMemoryWord", &getMemoryWord);
    emscripten::function("setMemoryByte", &setMemoryByte);
    emscripten::function("setMemoryWord", &setMemoryWord);
    emscripten::function("getPipelineState", &getPipelineState);
    emscripten::function("getAssemblyListing", &getAssemblyListing);
    
    value_object<PipelineStateJS>("PipelineStateJS")
        .field("if_id_pc", &PipelineStateJS::if_id_pc)
        .field("if_id_ir", &PipelineStateJS::if_id_ir)
        .field("if_id_npc", &PipelineStateJS::if_id_npc)
        .field("id_ex_ir", &PipelineStateJS::id_ex_ir)
        .field("id_ex_a", &PipelineStateJS::id_ex_a)
        .field("id_ex_b", &PipelineStateJS::id_ex_b)
        .field("id_ex_imm", &PipelineStateJS::id_ex_imm)
        .field("id_ex_npc", &PipelineStateJS::id_ex_npc)
        .field("ex_mem_ir", &PipelineStateJS::ex_mem_ir)
        .field("ex_mem_aluoutput", &PipelineStateJS::ex_mem_aluoutput)
        .field("ex_mem_b", &PipelineStateJS::ex_mem_b)
        .field("ex_mem_cond", &PipelineStateJS::ex_mem_cond)
        .field("mem_wb_ir", &PipelineStateJS::mem_wb_ir)
        .field("mem_wb_aluoutput", &PipelineStateJS::mem_wb_aluoutput)
        .field("mem_wb_lmd", &PipelineStateJS::mem_wb_lmd)
        .field("mem_wb_rd", &PipelineStateJS::mem_wb_rd)
        .field("mem_wb_regwrite", &PipelineStateJS::mem_wb_regwrite);
}