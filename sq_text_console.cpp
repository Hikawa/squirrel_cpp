#include "sq_text_console.h"

#include <iostream>

namespace sq {

void TextConsole::onSqPrint(VM* vm, const std::string& message) {
  std::cout << message;
}

void TextConsole::onSqError(VM* vm, const std::string& message) {
  std::cerr << message << std::endl;
}

void TextConsole::onSqCompileError(
                    VM* vm,
                    const std::string& desc,
                    const std::string& source,
                    SQInteger line,
                    SQInteger column) {
  std::cerr << desc << " in " << source << " on line " << line << " column " << column << std::endl;
}

bool TextConsole::reps() {
  std::string line;
  do {
    std::cout << '>';
    for (int i = 0; i < blocks; ++i)
      std::cout << '>';
    if (inString)
      std::cout << "...";
    std::cout << ' ';
    std::getline(std::cin, line);
    if (!std::cin) return false;
  } while (!isCommandComplete(line));
  try {
    std::cout << interpretCommand() << std::endl;
  } catch (VM::Error& e) {
    std::cerr << e.what() << std::endl;
  }
  return true;
}

void TextConsole::repl() {
  while (reps());
}


}

