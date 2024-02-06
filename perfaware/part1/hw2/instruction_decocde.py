from typing import List

class Instruction:
    def __init__(self, op, fields):
        self.op = op
        self.fields = fields

    def register(self, reg):
        if self.fields[1]:
            return self._reg_16(reg)
        return self._reg_8(reg) 

    def _reg_16(self, reg):
        return ["ax","cx","dx","bx", "sp", "bp", "si", "di"][reg]

    def _reg_8(self, reg):
        return ["al","cl","dl","bl", "ah", "ch", "dh", "bh"][reg]

def _imm_16(r_m):
    pass

def _imm_8(r_m):
    pass

def reg_reg(reg, r_m, instr):
    return f"{instr.register(reg)}, {instr.register(r_m)}"

op_map  = { 
           0b100010 : "mov" 
}

mod_map = { 
           0b00 : "",
           0b01 : "",
           0b10 : "",
           0b11 : reg_reg 
}

def decode_instr(binary: List[int]):
    count = 0
    while binary:
        opcode = (binary[count] & 0xFC) >> 2
        d,w = (binary[count] & 0x1, binary[count] & 0x2 >> 1)

        if (op := op_map.get(opcode, None)) == None:
            return
        
        print(op, end=' ')

        count += 1
        mod = (binary[count] & 0xC0) >> 6
        reg = (binary[count] & 0x38) >> 3
        r_m = (binary[count] & 0x03)

        if (mod := mod_map.get(mod, None)) == None:
            return 
        print(mod(reg, r_m, Instruction(opcode, [d,w])))


if __name__ == "__main__":
    decode_instr([0b10001000, 0b11000101])

