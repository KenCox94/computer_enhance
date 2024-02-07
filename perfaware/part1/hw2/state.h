#pragma once
typedef unsigned char _State;

struct State {

  State() : is_valid{false} {}

  State(Instruction *instruction) : is_valid{true}, instruction{instruction} {}

  ~State() {
    for (int i = 0; i < 16; i++) {
      if (this->_next_state_values[i]) {
        delete this->_next_state_values[i];
      }
    }
  }

  State *add_state(_State value, Instruction *instruction) {
    this->_next_state_values[value] = new State{instruction};
    return this->_next_state_values[value];
  }

  State *get_next_state(State value) { return this->_next_state_values[value]; }

  Instruction *instruction;
  bool is_valid;
  State *_next_state_values[16];
};
