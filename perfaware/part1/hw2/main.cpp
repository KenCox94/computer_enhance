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




enum MOV{ 
	REG_IMM = 0xB0,
	MEM_ACCUM = 0xA8,
	REG_MEM  = 0x88,
};

typedef std::map<int, std::string> opcode_map;
typedef std::map<int, std::string> Dest_State;

struct NFA{
	int inital_state; //first six bytes  
	bool is_final; // false on upper_nibble
	std::map<int, Dest_State> transition_map; // 
};


//NFA inital_state => first byte
// is_final we can map to an instruction => false 
//  transition_map => MOD (00) =>  

struct Instruct{
	unsigned char op: 6,
				  d:  1,
	 			  w:  1;

	unsigned char mod: 2,
				  reg: 3,
				  r_m: 3; 
};

struct Instruction{ 
	uint8_t op;
	union{ 
		uint8_t _register_8;
		uint16_t _register_16;
	} _register;
	void* _rmi;
};

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

int 
main(int argc, char** argv){
	if (argc < 2){
		std::cout << "ASM file must be provided" << std::endl;
		exit(1);
	}
	std::cout << "bits 16" << std::endl;
	std::cout << std::endl;

	std::ifstream input(argv[1], std::ios::binary);
	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
	const std::vector<std::string> w_registers{"ax", "cx", "dx","bx", "sp", "bp", "si", "di"};
	const std::vector<std::string> b_registers{"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

	
	const std::map<unsigned char, const char*> hash_map{
		{ 0x22, "MOV"},

	};


	auto it = buffer.begin();

	auto registers = &w_registers;
	while(it != buffer.end()){
		unsigned char op_code = (0xFC & *it) >> 2;
		auto next_byte = *(++it); 

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

		switch ((next_byte & 0xC0) >> 6){
			case Mod::MEM_MEM:
				break;
			case Mod::MEM_MOD_8:
				break;
			case Mod::MEM_MOD_16:
				break;
			case Mod::REGISTER_MOD: {
				auto mnuemonic = hash_map[op_code];
				break;
			}
			default:
				break;
		};
		

		it++;
		
	}

}



