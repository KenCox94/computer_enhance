

auto mov_reg_reg =
    inital_state.add_state(0x8, nullptr) -> add_state(0x2, nullptr);
mov_reg_reg->add_state(0x3, new Instruction{"mov"});
