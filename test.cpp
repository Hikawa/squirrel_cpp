#include "sq_text_console.h"

sq::VM& operator << (sq::VM& vm, int data) {
  vm << SQInteger(data);
}

int main() {
  sq::TextConsole console;
  sq::VM vm(&console);
  console.vm = &vm;
  console.repl();
}
