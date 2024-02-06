#include <functional>
#include <iostream>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>
#include <cstdint>
#include <ranges>


#define D 0x0
#define W 0x1

class Instruction { 
public:
	Instruction(uint8_t opcode, std::vector<bool> fields) : opcode{opcode}, fields{fields}{ 
		if (this->fields[W]) { 
			regs = &this->w_registers;
		}else {
			regs = &this->b_registers;
		}
	}

	 void get_reg_reg(size_t reg, size_t imm, void (*fn)(std::string, std::string)){
		if (this->fields[D]){
			fn((*this->regs)[imm], (*this->regs)[reg]);
		}else {
			fn((*this->regs)[reg], (*this->regs)[imm]);
		}
	}


	std::string get_register(size_t reg){
		return (*this->regs)[reg];
	}

	

private:
	uint8_t opcode; 
	std::vector<bool> fields;
	uint16_t _register;
	uint16_t _rmi;
	const std::vector<std::string>* regs;
	const std::vector<std::string> w_registers{"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
	const std::vector<std::string> b_registers{"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
};

enum Mod{ 
	MEM_MEM,
	MEM_MOD_8,
	MEM_MOD_16,
	REGISTER_MOD,
};

typedef void (*fptr)(std::string, std::string);

void reg_to_reg(std::string dst, std::string src){ 
	std::cout << dst << ", " << src << std::endl; 
}

fptr mem_mod(uint8_t reg, uint8_t imm) {

}


int main(int argc, char** argv){
	if (argc < 2){
		std::cout << "ASM file must be provided" << std::endl;
		exit(1);
	}
	std::cout << "bits 16" << std::endl;

	std::ifstream input(argv[1], std::ios::binary);
	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});


	auto it = buffer.begin();
	while(it != buffer.end()){
		uint8_t opcode = (*it & 0xFC) >> 2;
		bool field_d = (0x02 & *it);
		bool field_w = (0x01 & *it);
		std::vector<bool> fields{field_d, field_w};
		it++;

		Instruction inst{opcode, fields};
		size_t reg = (*it & 0x38) >> 3;
		size_t imm = (*it & 0x07);
		switch((*it & 0xC0) >> 6) {
			case MEM_MEM: { 
				break;
			};
			case MEM_MOD_8: { 
				std::cout << "mov "; 
				it++;
				break;
			};
			case MEM_MOD_16: {
				it++, it++;
				break;
			};
			case REGISTER_MOD:{
				std::cout << "mov ";
				inst.get_reg_reg(imm, reg, reg_to_reg);
				it++;
				break;
			};
			default: 
				break;
		}

	}
		
		
}



