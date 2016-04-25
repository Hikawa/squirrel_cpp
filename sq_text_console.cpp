#include "sq_text_console.h"

#include <iostream>

namespace sq {

void TextConsole::onSqPrint(VM* vm, const std::string& message) {
  std::cout << message;
}

void TextConsole::onSqError(VM* vm, const std::string& message) {
  std::cerr << message;
}

void TextConsole::onSqCompileError(
                    VM* vm,
                    const std::string& desc,
                    const std::string& source,
                    SQInteger line,
                    SQInteger column) {
  std::cerr << desc << " in " << source << " on line " << line << " column " << column;
}

bool TextConsole::reps() {
  std::string line;
  std::getline(std::cin, line);
  if (!std::cin) return false;
  try {
    vm->exec(line);
  } catch (VM::Error& e) {
    std::cerr << e.what();
  }
  return true;
}

void TextConsole::repl() {
  while (reps());
}


}

