#include <functional>
#include <iostream>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>
#include <cstdint>
#include <ranges>

/*
 *  "Machine instructions vary from one to six bytes in length"
 *  -------------------------
 *  | 1 | 2 | 3 | 4 | 5 | 6 |
 *  -------------------------
 *
 * --------
 * |  1   |
 * --------
 *
 * ------ [0:5] -> first six bits are the OPCODE that ids the basic instruction set
 * - [6] -> D field -- "direction" of the operation, 
 *   	+ 1 => REG in | 2 | is DES 
 *   	+ 0 => REG in | 2 | is SRC
 * - [7] -> Operation Size 
 *      + 1 => Word (2 Bytes) 
 *      + 0 => Byte
 *
 * --------
 * |  2   |
 * --------
 *
 * -- [0:1] ->  MOD, are the operands in MEM, REG or Both REG
 * --- [2:4] -> REG, register field (ids which register is being used) 
 * --- [5:7] -> R/M, depends on how the mode field is set
 *
 *
 *
 * MOV 
 * encoded as 0x8[9-C] => REG, REG/MEM
 * 			  0xA[0-3] => AL/AX/[MEM], MEM/[AL/AX]
 *            0xB[-]   => REG, IMM
 *
 * 0xA[0,1,2,3], 0000 0001 0010 0011 => mask? =  0xAC, 1010 1100 
 * 0x8[9-C], 1001 1010 
 * 0111 ^ 1111 => 1000
 * 
 * (1000), { mov, add, or, ...}  -> () [MOV] 
 *  
 *
 */



#define REG_TO_REG 0b100010

enum Width{
	Byte = 0,
	Word = 1,
};

enum Mod{ 
	MEM_MEM,
	MEM_MOD_8,
	MEM_MOD_16,
	REGISTER_MOD,
};

struct Instruction{ 

	Instruction(std::string nmuemonic) : nmuemonic{nmuemonic}{
	}

	std::string nmuemonic;
	union{ 
		uint8_t _register_8;
		uint16_t _register_16;
	} _register;
	union{ 
		uint8_t _rmi_8;
		uint8_t _rmi_16;
	}_rmi;
};



/*
 * 
 * inital thoughts
 * state 1 => vector of possible <string>, MOV, ADD, etc
 *       1 -> 0  Vector<states>   { 0, 1} 
 *       1 0 0 => Map<bool, Vector<ValidInstructions>>
 *
 * typedef bool State;
 * 
 * struct ValidInstruction {
 *    string mnuemonic;
 *    bool is_instruct;
 * };
 *
 *
 * typedef struct SM { 
 *    ValidInstruction is_instruct;
 *    State state_current;
 *    bool is_not_valid_instruction;
 *    Map<State, vector<ValidInstruction>>
 * } parser;
 * 
 *
 *  [0..3]            
 * (reset) ------> ( [MOV_IMM, MOV_....], (PUSH) ... ) --->  
 *   |
 *   |    TRANSITION 
 *    => 1000 0x8
 *   |            |
 *   .            |
 *   .            | 
 *   .            => [0..F]0x1  
 *   |
 *   |   ACCEPTED STATE {0xB_}
 *    => 1011 -> MOV  => NON_VALID_INSTR 
 * while( is_not_valid_instruction  ) {
 *    search(SM)
 * }
 *
 *
 */

typedef unsigned char State;


struct _State{ 

	_State() : is_valid{false} {

	}

	_State(std::string mnuemonic) : is_valid{true} {
	}


	~_State() {
		for(int i = 0; i < 16; i++ ){
			if (this->_next_state_values[i]){
				delete this->_next_state_values[i];
			}
		}
	}

	_State* add_state(State value, std::string mnuemonic){
		this->_next_state_values[value] = new _State{mnuemonic};
		return this->_next_state_values[value];
	}


	_State* get_next_state(State value) { 
		return this->_next_state_values[value];
	}


	Instruction* instruction;
	bool is_valid;
	_State* _next_state_values[16];
};


/*
 *  
 *  _State inital_state{false};
 *  inital_state.add_state(0xB);
 *
 *  inital_state.add_state(0x8).add_state(0x2);
 *  
 *
 *  
 *
 */


struct TransitionInstructionState {
	std::string	mnuemonic;
	bool accepted_state;
};


std::map<State, std::map<State, TransitionInstructionState>> _mnuemonic{ 
	 {0x8, {{0x2, {"mov", true}}, {0x1, {"mov", false}}}},
	 {0xB, {{0xF, {"mov", true}}}},
	 {0xC, {{0x3, {"mov", true}}}}, 
};



int 
main(int argc, char** argv){
	if (argc < 2){
		std::cout << "ASM file must be provided" << std::endl;
		exit(1);
	}
	std::cout << "bits 16" << std::endl;

	std::ifstream input(argv[1], std::ios::binary);
	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
	const std::vector<std::string> w_registers{"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
	const std::vector<std::string> b_registers{"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

	_State inital_state{};
	inital_state.add_state(0xB, nullptr)->add_state(0x2, "MOV");

	std::map<uint8_t, std::string> inst_map{
		{0x22, "mov"},
	};


	auto it = buffer.begin();
	auto registers{&w_registers};
	while(it != buffer.end()){
		unsigned char op_code = (0xFC & *it) >> 2;
		bool is_destination_reg = (0x02 & *it);
		auto nibble{*it >> 4};
		auto *state_map = &_mnuemonic;
		

		while(1) {
			auto lo_map = state_map->find(nibble);
		    if (lo_map == state_map->end()){ 
		    	break;
			}

			auto lo_nibble{*it & 0xF};
			TransitionInstructionState& state = lo_map->second[lo_nibble | 0xF];

			if (state.accepted_state){
				std::cout << state.mnuemonic;
				break;
			}
			
			break;
		}
		Instruction instr;
		instr.op = op_code;

		switch (*it & 0x01) {
			case Width::Byte : {
				registers = &b_registers;
				break;
			}
			case Width::Word : 
				registers = &w_registers;
				break;
			default: 
				std::cout << "invalid bit encoding" <<std::endl;
				exit(1);
		}

		auto next_byte = *(++it); 
		auto dest_idx = (0x38 & next_byte)>>3;
		auto src_idx = (0x7 & next_byte);
		if (!is_destination_reg) {
			src_idx ^= dest_idx;
			dest_idx ^= src_idx;
			src_idx ^= dest_idx;
		}
		
		auto mnuemonic = inst_map[instr.op];
		switch ((next_byte & 0xC0) >> 6){
			case Mod::MEM_MEM:
				break;
			case Mod::MEM_MOD_8:

				break;
			case Mod::MEM_MOD_16:
				break;
			case Mod::REGISTER_MOD: {
				auto d_reg = (*registers)[dest_idx];
				auto src_reg = (*registers)[src_idx];
				std::cout << mnuemonic << " " << d_reg << ", " << src_reg << std::endl;
				break;
			}
			default:
				break;
		};
		

		it++;
		
	}

}



