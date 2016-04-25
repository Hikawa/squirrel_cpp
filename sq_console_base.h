#pragma once

#include "sq_vm.h"

namespace sq {

class ConsoleBase: public VM::PrintHandler {
public:
  inline void setVm(VM* v) {
    vm = v;
    vm->printHandler = this;
  }

  std::string interpretCommand();
  bool isCommandComplete(const std::string& command);

  VM* vm;

  std::string currentCommand;
  int blocks = 0;
  char inString = '\0';
  bool retVal = false;
  bool isExecuting = false;
};

}

