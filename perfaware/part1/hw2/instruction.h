struct Instruction {

  Instruction(std::string nmuemonic) : nmuemonic{nmuemonic} {}

  std::string nmuemonic;
  bool is_destination_reg;
  union {
    uint8_t _register_8;
    uint16_t _register_16;
  } _register;
  union {
    uint8_t _rmi_8;
    uint16_t _rmi_16;
  } _rmi;
};
