#include <cstddef>
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
  size_t imm_reg(F& get_data){ 
  	  Data data{get_data()};
	  size_t count{0};
	  std::cout << "mov ";
	  std::cout << (*this->regs)[this->fields[R]] << ", ";
	  if (this->fields[W]){
		  std::cout << std::format("{}", data.word) << std::endl;
	  } else {
		  std::cout << std::format("{}", data.byte) << std::endl;
	  }
	  return count;
  }

  template<typename F>
  size_t reg_reg(F& get_data) {
  	/*
  	 * Get second byte of instruction
  	 * */
  	Data data{get_data(false)};
	size_t reg = (data.byte & 0x38) >> 3;
	size_t imm = (data.byte & 0x07);
	size_t mod = (data.byte & 0xC0) >> 6;
	size_t count{0};
	switch(mod){
		case MEMORY: {
			std::cout << "mov ";
			auto value = this->reg_reg_map.find((*this->regs)[reg]);
			if ( value != this->reg_reg_map.end()){
				if (this->fields[D]) {
			        std::cout << (*this->regs)[reg] << ", "; 
					std::cout << std::format("[{} + {}]", value->second[0], value->second[1]) << std::endl; 
				} else {
					 std::cout << std::format("[{} + {}], ", value->second[0], value->second[1]); 
			         std::cout << (*this->regs)[reg]  << std::endl; 
				}
			} 
			break;
		};
		case REG: { 
			if (this->fields[D]) {
			  reg_to_reg((*this->regs)[reg], (*this->regs)[imm]);
			} else {
			  reg_to_reg((*this->regs)[imm], (*this->regs)[reg]);
			}
			break;
		};
		case BIT_8: {
			auto _register = (*this->regs)[reg];
			auto value = this->reg_reg_map.find((*this->regs)[imm]);
			std::cout << "mov ";
			if(value != this->reg_reg_map.end()){
				data = get_data(false);
				if (this->fields[D]) {
					std::cout << _register << ", ";
					if( value->second.size() < 2) {
						std::cout << std::format("[{} + {}]", value->second[0], data.byte) << std::endl; 
					}else {
						std::cout << std::format("[{} + {} + {}]", value->second[0], value->second[1], data.byte) << std::endl; 
					}
				}
				else {
					if(value->second.size() < 2){
						std::cout << std::format("[{} + {}], ", value->second[0], data.byte); 
					} else {
						std::cout << std::format("[{} + {} + {}], ", value->second[0], value->second[1], data.byte); 
					}

					std::cout <<  _register << std::endl;
				}
			}
			break;
		};
		case BIT_16: {
			auto _register = (*this->regs)[reg];
			std::cout << "mov " <<  _register << ", ";
			auto value = this->reg_reg_map.find((*this->regs)[reg]);
			if(value != this->reg_reg_map.end()){
				data = get_data(true);
				if(value->second.size() < 2){
					std::cout << std::format("[{} + {}]", value->second[0], data.word) << std::endl; 
				}else {
					std::cout << std::format("[{} + {} + {}]", value->second[0], value->second[1], data.word) << std::endl; 
				}
			}

			break;
		};
		default:
			break;
  	}
  	return count;
  }

  size_t accumulator(Data* data){ 
	  std::cout << "mov ";
	  void* imm;
	  if(this->fields[W]){
	  	  imm = (uint16_t*)(data->word);
	  } else {
	  	  imm = (uint8_t*)(data->byte);
	  }
	  if (this->fields[D]) {
		  std::cout << "" <<  ", " << std::format("{}", imm) << std::endl; 
	  } else {
		  std::cout << std::format("{}", imm) <<  ", " << "" << std::endl; 
	  }
	  return 0;
  }

private:
  uint8_t opcode;
  std::vector<uint8_t> fields;
  const std::vector<std::string> *regs;
  const std::vector<std::string> w_registers{"ax", "cx", "dx", "bx",
                                             "sp", "bp", "si", "di"};
  const std::vector<std::string> b_registers{"al", "cl", "dl", "bl",
                                             "ah", "ch", "dh", "bh"};
  const std::map<std::string, std::vector<std::string>> reg_reg_map{
  	{"al", {"bx", "si"}}, 
  	{"bx", {"bp", "di"}}, 
  	{"dx", {"bp", "si"}}, 
  	{"ah", {"bh", "si"}},
	{"dh", {"bp"}},
	{"si", {"bp"}},
	{"cx", {"bx", "di"}},
	{"cl", {"bp", "si"}}

  };
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
    uint8_t opcode = (*it & 0xF0) >> 0x4;
    size_t advance_count{1};
	auto opcode_type = OP_TYPE_MAP.find(opcode);

	if (opcode_type == OP_TYPE_MAP.end()){
		std::cout << "could not find opcode " << std::format("{}", opcode) << std::endl;
		exit(1);
	}
	switch (opcode_type->second){ 
		case REG_MOD: {
    		bool field_w = (0x01 & *it); 
    		bool field_d = (0x02 & *it);
    		Instruction inst{opcode, {field_d, field_w}};
			auto data_getter = [&](bool flag) {
				Data data;
				if (flag) {
					data.word =  *(++it) | (*(++it) << 0x8);
				} else {
					data.byte = *(++it);
				}
				return data; 	
			};
    		advance_count += inst.reg_reg(data_getter);
	    	break;
	    };
	    case IMM_REG_MOD: {
	    	uint8_t field_w = (0x08 & *it) >> 3;
    		uint8_t field_reg = (0x7 & *it);
    		Instruction inst{opcode, {field_reg, field_w}};
	    	auto data_getter = [&]() {
				Data data;
	    		if (field_w){
					data.word =  *(++it) | (*(++it) << 0x8);
				} else {
					data.byte = *(++it);
				}
				return data;
			};
    		advance_count += inst.imm_reg(data_getter);
    		break;
		};
		case ACC: {
			bool field_w = (0x1 & *it);
    		bool field_d = (0x2 & *it) >> 1;
    		Instruction inst{opcode, {field_d, field_w}};
			auto data_getter = [&]() {
				Data data;
	    		if (field_w){
					data.byte = *(++it);
				}
				return data;
			};
    		advance_count += 1;
    		break;
		};
	    default:{
	    	std::cout << "cannot process opcode " << std::endl;
	    	exit(1);
	    }
	};

	std::advance(it, advance_count);
  
  }
}
