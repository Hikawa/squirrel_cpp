#include "sq_vm.h"

#include <sstream>
#include <cstdarg>
#include <sqstdio.h>
#include <sqstdaux.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>
#include <sqstdblob.h>

#define SQVM_TOPG TopGuard g(this, true, __FILE__, __LINE__, __FUNCTION__)
#define SQVM_LTOPG TopGuard g(this, false, __FILE__, __LINE__, __FUNCTION__)
#define SQVM_CTOPG CTopGuard g(this, __FILE__, __LINE__, __FUNCTION__)
#define SQVM_ASS(expr) if (!SQ_SUCCEEDED(expr)) throw sq::VM::Error(this, 0, (boost::format("%1% failed in %2%: %3%: %4%") % #expr % __FILE__ % __LINE__ % __FUNCTION__).str())

namespace sq {

namespace lit {
const char* CONSTRUCTOR = "constructor";
const char* GET = "_get";
const char* SET = "_set";
const char* TO_STRING = "_tostring";
const char* TYPE_OF = "_typeof";
}

const char* VM::valueTypeName(SQInteger idx) const {
  SQVM_CTOPG;
  switch (sq_gettype(vm, idx)) {
  case OT_NULL:          return "null";
  case OT_INTEGER:       return "integer";
  case OT_FLOAT:         return "float";
  case OT_STRING:        return "string";
  case OT_TABLE:         return "table";
  case OT_ARRAY:         return "array";
  case OT_USERDATA:      return "userdata";
  case OT_CLOSURE:       return "closure(function)";
  case OT_NATIVECLOSURE: return "native closure(C function)";
  case OT_GENERATOR:     return "generator";
  case OT_USERPOINTER:   return "userpointer";
  case OT_BOOL:          return "bool";
  case OT_INSTANCE:      return "instance";
  case OT_CLASS:         return "class";
  case OT_WEAKREF:       return "weak reference";
  default:               return "unknown type";
  }
}

std::string VM::toString(int idx) const {
  SQVM_CTOPG;
  switch (valueType()) {
  case OT_ARRAY: {
    std::ostringstream result("[");
    const int count = getValueSize(idx);
    VM* mthis = const_cast<VM*>(this);
    for (SQInteger i = 0; i < count; ++i) {
      mthis->pushField(i);
      result << toString();
      mthis->pop();
      if (i < count - 1) result << ", ";
    }
    result << ']';
    return result.str();
  }
  case OT_TABLE: {
    if (idx < 0) --idx;
    std::ostringstream result("{");
    VM* mthis = const_cast<VM*>(this);
    mthis->pushNull();
    bool first = false;
    while (mthis->next(idx)) {
      if (!first) result << ", ";
      first = false;
      result << toString(-2);
      result << '=';
      result << toString(-1);
      mthis->pop(2);
    }
    mthis->pop();
    result << '}';
    return result.str();
  }
  default:
    return getAsString(idx);
  }
}

static void compileErrorFunc(
              HSQUIRRELVM v,
              const SQChar* desc,
              const SQChar* source,
              SQInteger line,
              SQInteger column) {
  VM* vm = VM::inst(v);
  if (vm->printHandler)
    vm->printHandler->onSqCompileError(vm, desc, source, line, column);
}

static void printFunc(HSQUIRRELVM v, const SQChar* s, ...) {
  va_list vl, vlCopy;
  va_start(vl, s);
  va_copy(vlCopy, vl);
  const size_t size = std::vsnprintf(nullptr, 0, s, vl) + 1;
  std::string message(size, ' ');
  std::vsnprintf(&message.front(), size, s, vlCopy);
  va_end(vl);
  va_end(vlCopy);

  VM* vm = VM::inst(v);
  if (vm->printHandler) vm->printHandler->onSqPrint(vm, message);
}

static void errorFunc(HSQUIRRELVM v, const SQChar* s, ...) {
  va_list vl, vlCopy;
  va_start(vl, s);
  va_copy(vlCopy, vl);
  const size_t size = std::vsnprintf(nullptr, 0, s, vl) + 1;
  std::string message(size, ' ');
  std::vsnprintf(&message.front(), size, s, vlCopy);
  va_end(vl);
  va_end(vlCopy);

  VM* vm = VM::inst(v);
  if (vm->printHandler) vm->printHandler->onSqError(vm, message);
}

VM::VM(PrintHandler* handler, SQInteger initialStackSize): noTopGuard(false), printHandler(handler) {
  vm = sq_open(initialStackSize);
  sq_setforeignptr(vm, this);

  sqstd_seterrorhandlers(vm); //registers the default error handlers
  sq_setprintfunc(vm, &printFunc, &errorFunc);
  sq_setcompilererrorhandler(vm, &compileErrorFunc);
  sq_enabledebuginfo(vm, SQTrue);

  pushRootTable();

  SQVM_ASS(sqstd_register_iolib(vm));
  SQVM_ASS(sqstd_register_mathlib(vm));
  SQVM_ASS(sqstd_register_stringlib(vm));
  SQVM_ASS(sqstd_register_systemlib(vm));
  SQVM_ASS(sqstd_register_bloblib(vm));
}

}

