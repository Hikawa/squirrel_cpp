#pragma once

#include "sq_vm.h"

namespace sq {

class ConsoleBase: public VM::PrintHandler {
public:
  inline void setVm(VM* v) {
    vm = v;
    vm->printHandler = this;
  }

  VM* vm;
};

}

