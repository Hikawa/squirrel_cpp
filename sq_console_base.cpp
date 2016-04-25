#include "sq_console_base.h"

#include <boost/algorithm/string.hpp>

namespace sq {

std::string ConsoleBase::interpretCommand() {
  boost::trim(currentCommand);
  if (retVal) {
    while (!currentCommand.empty() && currentCommand.back() == ';') {
      currentCommand.erase(currentCommand.size() - 1, 1);
      boost::trim(currentCommand);
    }
    currentCommand = (boost::format("return (%1%)") % currentCommand).str();
  }

  vm->printHandler = this;
  struct ExecGuard {
    ExecGuard(ConsoleBase* p): parent(p) {
      parent->isExecuting = true;
      top = parent->vm->getTop();
    }
    ~ExecGuard() {
      parent->isExecuting = false;
      parent->vm->setTop(top);
      parent->currentCommand.clear();
    }

    int top;
    ConsoleBase* parent;
  } execGuard(this);

  vm->pushRootTable();
  vm->compile(currentCommand);
  vm->pushRootTable();
  vm->call(1, retVal);
  if (retVal) {
    return vm->toString();
  } else return "";
}

bool ConsoleBase::isCommandComplete(const std::string& command) {
  std::string part = command;
  boost::trim(part);
  if (currentCommand.empty()) {
    blocks = 0;
    inString = '\0';
    retVal = false;
    if (part.front() == '=') {
      part.erase(0, 1);
      boost::trim(part);
      retVal = true;
    }
  }
  if (part.empty()) return false;

  for (int i = 0; i < part.size(); ++i) {
    const char c = part[i];
    if (inString == '@') {
      if (c == '"') {
        if ((i + 1 < part.size()) && (part[i + 1] == '"'))
          ++i;
        else
          inString = '\0';
      }
    } else if (inString != '\0') {
      if (c == inString) inString = '\0';
      else if (c == '\\') ++i;
    } else if ((c == '\'') || (c == '"'))
      inString = c;
    else if ((c == '@') && (i + 1 < part.size()) && (part[i + 1] == '"')) {
      inString = '@';
      ++i;
    }
    else if (c == '}') blocks--;
    else if (c == '{') blocks++;
  }

  bool doIt = (blocks <= 0) && (inString != '@');

  if (part.back() == '\\') {
    part.erase(part.size() - 1, 1);
    if (!inString) part.push_back('\n');
    doIt = false;
  } else part.push_back('\n');

  currentCommand += part;
  return doIt;
}

}

