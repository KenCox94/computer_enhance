#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <format>
#include <ranges>
#include <string>
#include <vector>

#define D 0x0
#define W 0x1

enum OpMode {
  NOT_FOUND = -1,
  MEM_MEM,
  IMM_REG_MOD,
  MEM_MOD_8,
  MEM_MOD_16,
  ACC,
  REG_MOD,
};

const std::map<unsigned char, OpMode> MODE_MAP { {0x88, REG_MOD }, {0x8A, MEM_MOD_8}, {0xB0, ACC}};

void reg_to_reg(std::string dst, std::string src) {
  std::cout << dst << ", " << src << std::endl;
}


typedef union{
	uint16_t word;
	uint8_t byte; 
}Data;


class Instruction {
public:
  Instruction(uint8_t opcode, std::vector<bool> fields)
      : opcode{opcode}, fields{fields} {
    if (this->fields[W]) {
      regs = &this->w_registers;
    } else {
      regs = &this->b_registers;
    }

	auto mode = MODE_MAP.find(opcode) ;
	if (mode != MODE_MAP.end()){ 
		this->mod = mode->second;
	} else {
		this->mod = NOT_FOUND;
	}
  }

  template<typename F>
  size_t perform_instruction(F& get_data ){ 
	  Data data;
	  size_t count;
  	  switch(this->mod) { 
		  case REG_MOD: {
		  	count = get_data(data); 
			size_t reg = (data.byte & 0x38) >> 3;
			size_t imm = (data.byte & 0x07);
			std::cout << "mov ";
  		    this->get_reg_reg(reg, imm, reg_to_reg); 	
  			break;
		  };
		  case NOT_FOUND: { 
		  	  std::cout << "Cannot process instruction" << std::endl;
		  	  exit(1);
		  	  break;
		  }
		  default:
		  	  break;
	  }
	  return count;
  }

  void get_reg_reg(size_t reg, size_t imm,
                   void (*fn)(std::string, std::string)) {
    if (this->fields[D]) {
      fn((*this->regs)[reg], (*this->regs)[imm]);
    } else {
      fn((*this->regs)[imm], (*this->regs)[reg]);
    }
  }

private:
  uint8_t opcode;
  std::vector<bool> fields;
  OpMode mod;
  const std::vector<std::string> *regs;
  const std::vector<std::string> w_registers{"ax", "cx", "dx", "bx",
                                             "sp", "bp", "si", "di"};
  const std::vector<std::string> b_registers{"al", "cl", "dl", "bl",
                                             "ah", "ch", "dh", "bh"};
};

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "ASM file must be provided" << std::endl;
    exit(1);
  }
  std::cout << "bits 16" << std::endl;

  std::ifstream input(argv[1], std::ios::binary);
  std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

  auto it = buffer.begin();
  while (it != buffer.end()) {
    uint8_t opcode = (*it & ~0x3);
    bool field_d = (0x02 & *it);
    bool field_w = (0x01 & *it);
    Instruction inst{opcode, {field_d, field_w}};
    auto it_ref = &it;
   	auto data_getter  = [&](Data& data) {
   		data.byte = *(++it);
   		return 1;
	};
    auto advance_count = inst.perform_instruction(data_getter);
	std::advance(it, advance_count);
  }
}
