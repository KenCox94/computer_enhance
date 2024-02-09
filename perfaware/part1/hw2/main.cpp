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
#define R 0x0

enum Mod{
	MEMORY, 
	BIT_8,
	BIT_16,
	REG,
	NONE
};


enum OpMode {
  NOT_FOUND = -1,
  MEM_MEM,
  IMM_REG_MOD,
  MEM_MOD_8,
  MEM_MOD_16,
  ACC,
  REG_MOD,
};

const std::map<unsigned char, OpMode> OP_TYPE_MAP { {0x8, REG_MOD }, {0xB, IMM_REG_MOD}, {0xA, ACC }};

void reg_to_reg(std::string dst, std::string src) {
	std::cout << "mov ";
	std::cout << dst << ", " << src << std::endl;
}

typedef union{
	int16_t word;
	int8_t byte; 
}Data;

class Instruction {
public:
  Instruction(uint8_t opcode, std::vector<uint8_t> fields)
      : opcode{opcode}, fields{fields}{
    if (this->fields[W]) {
      regs = &this->w_registers;
    } else {
      regs = &this->b_registers;
    }
  }

  template<typename F>
  size_t perform_instruction(F& get_data){ 
	  Data data;
	  size_t count{1};
	  count = get_data(data);	
	  return count;
  }

  void imm_reg(Data* data){ 
	  std::cout << "mov ";
	  std::cout << (*this->regs)[this->fields[R]] << ", ";
	  if (this->fields[W]){
		  std::cout << std::format("{}", data->word) << std::endl;
	  } else {
		  std::cout << std::format("{}", data->byte) << std::endl;
	  }
  }

  void reg_reg(Data* data, void (*fn)(std::string, std::string)) {
	size_t reg = (data->byte & 0x38) >> 3;
	size_t imm = (data->byte & 0x07);
	size_t mod = (data->byte & 0xC0) >> 6;
	switch(mod){
		case REG: { 
			if (this->fields[D]) {
			  fn((*this->regs)[reg], (*this->regs)[imm]);
			} else {
			  fn((*this->regs)[imm], (*this->regs)[reg]);
			}
			break;
		};
		case MEMORY: {
			std::cout << "mov " << (*this->regs)[reg] << " "; 
			auto value = this->reg_reg_map.find((*this->regs)[reg]);
			if ( value != this->reg_reg_map.end()){
				std::cout << std::format("[{} + {}]", value->second[0], value->second[1]) << std::endl; 
			}
			break;
		};
		case BIT_8: {
			if(this->fields[D]){ 
				auto _register = (*this->regs)[reg];
				std::cout << "mov " <<  _register << " ";
				auto value = this->reg_reg_map.find(_register);
				if ( value != this->reg_reg_map.end()){
					std::cout << std::format("[{}]", value->second[0]) << std::endl; 
				}
			}
			break;
		};
		case BIT_16: { 
			std::cout << "mov " << std::endl;
		};
		default:
			std::cout << "mov " << std::endl;
			break;
  	}
  }

  void accumulator(Data* data){ 
	  std::cout << "mov ";
	  void* imm;
	  if(this->fields[W]){
	  	  imm = (uint16_t*)(data->word);
	  } else {
	  	  imm = (uint8_t*)(data->byte);
	  }
	  if (this->fields[D]) {
		  std::cout << reg <<  ", " << std::format("{}", imm) << std::endl; 
	  } else {
		  std::cout << std::format("{}", imm) <<  ", " << reg << std::endl; 
	  }
  }

private:
  uint8_t opcode;
  std::vector<uint8_t> fields;
  const std::vector<std::string> *regs;
  const std::vector<std::string> w_registers{"ax", "cx", "dx", "bx",
                                             "sp", "bp", "si", "di"};
  const std::vector<std::string> b_registers{"al", "cl", "dl", "bl",
                                             "ah", "ch", "dh", "bh"};
  const std::map<std::string, std::vector<std::string>> reg_reg_map{{"al", {"bx", "si"}}, {"bx", {"bp", "di"}}, {"dx", {"bp", "si"}}};
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

    uint8_t opcode = ((*it & ~0x4) & 0xF0) >> 0x4;

    size_t advance_count{1};
	auto opcode_type = OP_TYPE_MAP.find(opcode);

	if (opcode_type == OP_TYPE_MAP.end()){
		exit(1);
	}
  	switch (opcode_type->second){ 
		case REG_MOD: {
			auto it_ref = &it;
    		bool field_w = (0x01 & *it);
    		bool field_d = (0x02 & *it);
    		Instruction inst{opcode, {field_d, field_w}};
			auto data_getter = [&](Data& data) {
				data.byte = *(++it);
				inst.reg_reg(&data, reg_to_reg); 	
				return 1;
			};
    		advance_count = inst.perform_instruction(data_getter);
	    	break;
	    };
	    case IMM_REG_MOD: {
	    	bool field_w = (0x08 & *it) >> 3;
    		bool field_reg = (0x7 & *it);
    		Instruction inst{opcode, {field_reg, field_w}};
	    	auto data_getter = [&](Data& data) {
	    		size_t count{0};
	    		if (field_w){
					data.word =  *(++it) | (*(++it) << 0x8) ;
					count++;
				} else {
					data.byte = *(++it);
				}
				inst.imm_reg(&data);
				return count;
			};
    		advance_count = inst.perform_instruction(data_getter);
    		break;
		};
		case ACC: {
			bool field_w = (0x1 & *it);
    		bool field_d = (0x2 & *it) >> 1;
    		Instruction inst{opcode, {field_d, field_w}};
			auto data_getter = [&](Data& data) {
	    		size_t count{0};
	    		if (field_w){
					data.word =  *(++it) | (*(++it) << 0x8) ;
					count++;
				} else {
					data.byte = *(++it);
				}
    			inst.accumulator(data);
				return count;
			};
		};
	    default:{
	    	exit(1);
	    }
	};
	it += 1;
  
  }
}
