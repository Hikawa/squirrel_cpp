#pragma once

#include "sq_console_base.h"

namespace sq {

class TextConsole: public ConsoleBase {
public:
  virtual void onSqPrint(VM* vm, const std::string& message) override;
  virtual void onSqError(VM* vm, const std::string& message) override;
  virtual void onSqCompileError(
                 VM* vm,
                 const std::string& desc,
                 const std::string& source,
                 SQInteger line,
                 SQInteger column) override;

  bool reps();
  void repl();

};

}
