#pragma once

#include <string>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <boost/format.hpp>
#include <cassert>

#include "squirrel.h"
#include "sqstdio.h"

#define SQVM_TOPG TopGuard g(this, true, __FILE__, __LINE__, __FUNCTION__)
#define SQVM_LTOPG TopGuard g(this, false, __FILE__, __LINE__, __FUNCTION__)
#define SQVM_CTOPG CTopGuard g(this, __FILE__, __LINE__, __FUNCTION__)
#define SQVM_ASS(expr) if (!SQ_SUCCEEDED(expr)) throw sq::VM::Error(this, 0, (boost::format("%1 failed in %2: %3: %4") % #expr % __FILE__ % __LINE__ % __FUNCTION__).str())

namespace sq {

namespace lit {
extern const char* CONSTRUCTOR;
extern const char* GET;
extern const char* SET;
extern const char* TO_STRING;
extern const char* TYPE_OF;
}

class VM {
public:
  class Error;
  
  struct ClosureInfo {
    SQUnsignedInteger params;
    SQUnsignedInteger freeVars;
  };
  
  class Any;
  
  enum State {
    IDLE = SQ_VMSTATE_IDLE,
    RUNNING = SQ_VMSTATE_RUNNING,
    SUSPENDED = SQ_VMSTATE_SUSPENDED
  };

  class PrintHandler {
  public:
    virtual void onSqPrint(VM* vm, const std::string& message) = 0;
    virtual void onSqError(VM* vm, const std::string& message) = 0;
    virtual void onSqCompileError(
                   VM* vm,
                   const std::string& desc,
                   const std::string& source,
                   SQInteger line,
                   SQInteger column) = 0;
  };
  
  VM(const VM&) = delete;
  VM(PrintHandler* handler = nullptr, SQInteger initialStackSize = 1024);
  virtual ~VM() { sq_close(vm); }
  
  State getState() const;
  
  // Simple API wrappers
  // Stack Operations
  int cmp();
  inline int getTop() const { return sq_gettop(vm); }
  void pop(SQInteger count = 1);
  void push(SQInteger idx);
  void remove(SQInteger idx);
  void reservestack(SQInteger n);
  void setTop(SQInteger top);
  
  // Object creation and handling
  void bindEnv(SQInteger idx);
  void pushInstance(SQInteger idx = -1);
  // sq_getbyhandle -
  ClosureInfo getClosureInfo(SQInteger idx = -1) const;
  std::string getClosureName(SQInteger idx = -1) const;
  // sq_gethash -
  template <typename T = void*>
  T getInstancePtr(SQInteger ind = -1, void* typeTag = nullptr) const;
  // sq_getmemberhandle -
  // sq_getscratchpad -
  inline int getValueSize(SQInteger idx = -1) const { return sq_getsize(vm, idx); }
  // sq_getthread -
  inline SQObjectType valueType(SQInteger idx = -1) const { return sq_gettype(vm, idx); }
  const char* valueTypeName(SQInteger idx = -1) const;
  // sq_gettypetag -
  template <typename T = void*>
  T getUserData(SQInteger idx = -1) const;
  template <typename T = void*>
  T getPtr(SQInteger idx = -1) const;
  void pushNewArray(SQInteger size = 0);
  void pushNewClass(bool base);
  void pushNewTable(SQInteger capacity = -1);
  template <typename T = void*>
  T pushUserData(SQInteger size);
  template <typename T, typename ... Ts>
  void pushUserValue(Ts ... args);
  void pushNull();
  void pushPtr(const void* ptr);
  // sq_setbyhandle -
  void setClassUDSize(SQInteger size, SQInteger idx = -1);
  void setInstancePtr(void* ptr, SQInteger idx = -1);
  void setReleaseHook(SQRELEASEHOOK f, SQInteger idx = -1);
  // sq_settypetag -
  void pushTypeOf(SQInteger idx = -1);
  
  // Calls
  void call(SQInteger params, bool ret);
  void pushCallee();
  void pushLastError();
  // sq_getlocal
  void resetError();
  // sq_resume
  SQInteger throwError(const std::string& msg);
  SQInteger throwError(const char* msg);
  SQInteger throwObject();
  inline SQInteger throwNull() {
    pushNull();
    return throwObject();
  }
  
  // Objects manipulation
  void arrayAppend(SQInteger idx = -2);
  void arrayInsert(SQInteger pos, SQInteger idx = -1);
  void arrayPop(SQInteger idx = -1, bool doPush = true);
  void arrayRemove(SQInteger pos, SQInteger idx = -1);
  void arrayResize(SQInteger size, SQInteger idx = -1);
  void arrayReverse(SQInteger idx = -1);
  void valueClear(SQInteger idx = -1);
  void valueClone(SQInteger idx = -1);
  void deleteSlot(SQInteger idx = -1, bool doPush = false);
  void pushSlotValue(SQInteger idx = -1);
  // sq_getattributes -
  void pushBaseOf(SQInteger idx = -1);
  void pushClassOf(SQInteger idx = -1);
  // sq_getdelegate -
  // sq_getfreevariable -
  // sq_getweakrefval -
  bool instanceOf();
  // sq_newmember -
  void newSlot(SQInteger idx = -3, bool isStatic = false);
  template <typename Key, typename Value>
  void makeSlot(Key key, Value value, SQInteger idx = -1, bool isStatic = false);
  bool next(SQInteger idx = -2);
  // sq_rawdeleteslot -
  // sq_rawget -
  // sq_rawnewmember -
  // sq_rawset -
  void setSlot(SQInteger idx = -3);
  template <typename Key, typename Value>
  void setSlot(Key key, Value value, SQInteger idx = -1);
  // sq_setattributes -
  void setDelegate(SQInteger idx = -2);
  // sq_setfreevariable -
  // sq_weakref -
  
  // Bytecode serialization
  // sq_readclosure -
  // sq_writeclosure -
  
  // Raw object handling
  
  // Garbage Collector
  // sq_collectgarbage -
  // sq_resurrectunreachable
  
  // Additional API

  // Datatypes:

  // int  
  SQInteger getInt(SQInteger idx = -1) const;
  template <typename Key>
  SQInteger getIntField(Key key, SQInteger idx = -1) const;
  VM& operator >> (SQInteger& data);
  VM& operator << (SQInteger data);

  // float
  SQFloat getFloat(SQInteger idx = -1) const;
  template <typename Key>
  SQFloat getFloatField(Key key, SQInteger idx = -1) const;
  VM& operator >> (SQFloat& data);
  VM& operator << (SQFloat data);

  // string
  std::string getString(SQInteger idx = -1) const;
  std::string getAsString(SQInteger idx = -1) const;
  template <typename Key>
  std::string getStringField(Key key, SQInteger idx = -1) const;
  VM& operator >> (std::string& data);
  VM& operator << (const std::string& data);

  // bool
  bool getBool(SQInteger idx = -1) const;
  bool getAsBool(SQInteger idx = -1) const;
  template <typename Key>
  bool getBoolField(Key key, SQInteger idx = -1) const;
  VM& operator >> (bool& data);
  VM& operator << (bool data);
/*
  // user data
  std::string getString(SQInteger idx = -1) const;
  template <typename Key>
  std::string getStringField(Key key, SQInteger idx = -1) const;
  VM& operator >> (std::string& data);
  VM& operator << (const std::string& data);

  // user pointer
  std::string getString(SQInteger idx = -1) const;
  template <typename Key>
  std::string getStringField(Key key, SQInteger idx = -1) const;
  VM& operator >> (std::string& data);
  VM& operator << (const std::string& data);
*/

  // any
  Any get(SQInteger idx = -1) const;
  template <typename Key>
  Any getField(Key key, SQInteger idx = -1) const;
  VM& operator >> (Any& data);
  VM& operator << (const Any& data);

  
  template <typename Key>
  void pushField(Key field, int idx = -1);
  
  void pushRootTable();
  void compile(const std::string& code, const std::string& fileName = "repl");
  void exec(const std::string& code, const std::string& fileName = "repl");
  void doFile(const std::string& fileName);
  
  std::string toString(int idx = -1) const;
  
  class TopGuard;
  class CTopGuard;
  
  void setParameterCheck(SQInteger paramCount, const std::string& params);

  void pushRawClosure(SQFUNCTION func, SQInteger freeVars);
  
  static VM* inst(HSQUIRRELVM vm) {
    return reinterpret_cast<VM*>(sq_getforeignptr(vm));
  }

  PrintHandler* printHandler;

private:
  
  HSQUIRRELVM vm;
  bool noTopGuard;
};

class VM::Error: public std::runtime_error {
public:
  Error(const VM* vm, SQInteger idx, const std::string& message)
    : std::runtime_error("Squirrel VM error: " + message), vm(vm), idx(idx) {
  }

  const VM* vm;
  const SQInteger idx;
};

#ifdef SQVM_STACK_TRACE
class VM::TopGuard {
public:
  TopGuard(VM* vm, bool prevent, const char* file, int line, const char* function)
      : vm(vm), file(file), line(line), function(function), checked(false)
  {
    oldNt = vm->noTopGuard;
    if (prevent)
      this->vm->noTopGuard = true;
      oldTop = vm->getTop();
  }

  virtual void check(int delta = 0) {
    checked = true;
    if ((delta != -1) && (vm->getTop() - oldTop != delta))
      throw Error(vm, vm->getTop(), (boost::format(
              "SQ VM stack integrity failed: Expexted delta = %1 but got %2 "
              " in %3: %4 on line %5") % delta % (vm->getTop() - oldTop)
              % file % function % line).str());
  }

  template <typename T>
  inline T check(T data, int delta) {
    check(delta);
    return data;
  }

  ~TopGuard() {
    vm->noTopGuard = oldNt;
    // TODO: if (!vm->noTopGuard && (vm->getTop() != oldTop))
    if (!checked) check();
  }

protected:
  VM* vm;
  const char* file;
  int line;
  const char* function;
  int oldTop;
  bool oldNt;
  bool checked;
};

#else
class VM::TopGuard {
public:
  TopGuard(VM* vm, bool prevent, const char* file, int line, const char* function) {}

  virtual void check(int delta = 0) {}

  template <typename T>
  inline T check(T data, int delta) {
    return data;
  }
};

#endif

class VM::CTopGuard: public VM::TopGuard {
public:
  CTopGuard(const VM* vm, const char* file, int line, const char* function)
    : TopGuard(const_cast<VM*>(vm), true, file, line, function) {}

  void check(int delta = 0) override {
    if (delta) throw std::logic_error("Expecting changing stack by constant operation");
  }
};

class VM::Any {
public:
  Any() {
    sq_resetobject(&obj);
  }

  Any(VM* v, SQInteger idx = -1) {
    reset(v, idx);
  }

  void reset(VM* v, SQInteger idx = -1) {
    if (vm) sq_release(vm->vm, &obj);
    vm = v;
    sq_resetobject(&obj);
    if (!vm) return;
    if (!SQ_SUCCEEDED(sq_getstackobj(vm->vm, idx, &obj)))
      throw Error(vm, idx, "Can't get stack object");
    sq_addref(vm->vm, &obj);
  }

  Any(const Any& other): vm(other.vm), obj(other.obj)  {
    if (vm) sq_addref(vm->vm, &obj);
  }

  ~Any() {
    if (vm) sq_release(vm->vm, &obj);
  }

  Any& operator = (const Any& other) {
    if (this == &other) return *this;
    if (!other.vm) {
      if (vm) sq_release(vm->vm, &obj);
      vm = nullptr;
      return *this;
    }
    assert(!vm || (vm == other.vm));
    vm = other.vm;
    sq_release(vm->vm, &obj);
    obj = other.obj;
    sq_addref(vm->vm, &obj);
    return *this;
  }

  SQInteger getInt() const;
  SQFloat getFloat() const;
  std::string getString() const;
  bool getBool() const;

  VM* vm = nullptr;
  HSQOBJECT obj;
};

inline VM::State VM::getState() const {
  SQVM_CTOPG;
  return static_cast<VM::State>(sq_getvmstate(vm));
}

// Simple API wrappers
// Stack Operations
inline int VM::cmp() {
  SQVM_TOPG; return g.check(sq_cmp(vm), -2);
}

inline void VM::pop(SQInteger count) {
  SQVM_TOPG; sq_pop(vm, count); g.check(-count);
}

inline void VM::push(SQInteger idx) {
  SQVM_TOPG; sq_push(vm, idx); g.check(1);
}

inline void VM::remove(SQInteger idx) {
  // TODO: emit beforeRemove(idx);
  sq_remove(vm, idx);
  // TODO: emit afterRemove(idx);
}

inline void VM::reservestack(SQInteger n) {
  SQVM_CTOPG; SQVM_ASS(sq_reservestack(vm, n));
}

inline void VM::setTop(SQInteger top) {
  sq_settop(vm, top);
  // TODO: event
}


// Object creation and handling
inline void VM::bindEnv(SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_bindenv(vm, idx));
}

inline void VM::pushInstance(SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_createinstance(vm, idx)); g.check(1);
}

inline VM::ClosureInfo VM::getClosureInfo(SQInteger idx) const {
  SQVM_CTOPG;
  ClosureInfo result;
  SQVM_ASS(sq_getclosureinfo(vm, idx, &result.params, &result.freeVars));
  return result;
}

inline std::string VM::getClosureName(SQInteger idx) const {
  SQVM_CTOPG;
  SQVM_ASS(sq_getclosurename(vm, idx));
  std::string result;
  const_cast<VM&>(*this) >> result;
  return result;
}

template <typename T>
inline T VM::getInstancePtr(SQInteger idx, void* typeTag) const {
  SQVM_CTOPG;
  SQUserPointer v;
  SQVM_ASS(sq_getinstanceup(vm, idx, &v, typeTag));
  return reinterpret_cast<T>(v);
}

template <typename T>
inline T VM::getUserData(SQInteger idx) const {
  SQVM_CTOPG;
  SQUserPointer v;
  SQVM_ASS(sq_getuserdata(vm, idx, &v, 0));
  return reinterpret_cast<T>(v);
}

template <typename T>
inline T VM::getPtr(SQInteger idx) const {
  SQVM_CTOPG;
  SQUserPointer v;
  SQVM_ASS(sq_getuserpointer(vm, idx, &v));
  return reinterpret_cast<T>(v);
}

inline void VM::pushNewArray(SQInteger size) {
  SQVM_TOPG;
  sq_newarray(vm, size);
  g.check(1);
}

inline void VM::pushNewClass(bool base) {
  SQVM_TOPG;
  sq_newclass(vm, base? SQTrue: SQFalse);
  g.check(base? 0: 1);
}

inline void VM::pushNewTable(SQInteger capacity) {
  SQVM_TOPG;
  if (capacity == -1)
    sq_newtable(vm);
  else
    sq_newtableex(vm, capacity);
  g.check(1);
}

template <typename T>
inline T VM::pushUserData(SQInteger size) {
  SQVM_TOPG; return g.check(reinterpret_cast<T>(sq_newuserdata(vm, size)), 1);
}

template <typename T, typename ... Ts>
inline void VM::pushUserValue(Ts ... args) {
  SQVM_TOPG;
  T* pointer = pushUserData<T*>(sizeof(T));
  new(pointer) T(args ...);
  struct Impl {
    static SQInteger release(SQUserPointer ptr, SQInteger) {
      reinterpret_cast<T*>(ptr)->~T();
      return 1;
    }
  };
  sq_setreleasehook(vm, -1, &Impl::release);
  g.check(1);
}

inline void VM::pushNull() {
  SQVM_TOPG; sq_pushnull(vm); g.check(1);
}

inline void VM::pushPtr(const void* ptr) {
  SQVM_TOPG; sq_pushuserpointer(vm, const_cast<SQUserPointer>(ptr)); g.check(1);
}

inline void VM::setClassUDSize(SQInteger size, SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_setclassudsize(vm, idx, size));
}

inline void VM::setInstancePtr(void* ptr, SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_setinstanceup(vm, idx, ptr));
}

inline void VM::setReleaseHook(SQRELEASEHOOK f, SQInteger idx) {
  SQVM_TOPG; sq_setreleasehook(vm, idx, f);
}

inline void VM::pushTypeOf(SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_typeof(vm, idx)); g.check(1);
}

inline void VM::call(SQInteger params, bool ret) {
  SQVM_LTOPG;
  if (!SQ_SUCCEEDED(sq_call(vm, params, ret? SQTrue: SQFalse, SQTrue))) {
    sq_getlasterror(vm);
    std::string errorString;
    (*this) >> errorString;
    throw Error(this, -1, errorString);
  }
  g.check(-params + (ret? 1: 0));
}

inline void VM::pushCallee() {
  SQVM_TOPG; SQVM_ASS(sq_getcallee(vm)); g.check(1);
}

inline void VM::pushLastError() {
  SQVM_TOPG; sq_getlasterror(vm); g.check(1);
}

inline void VM::resetError() {
  SQVM_TOPG; sq_reseterror(vm);
}

inline SQInteger VM::throwError(const std::string& msg) {
  SQVM_TOPG; return sq_throwerror(vm, msg.c_str());
}

inline SQInteger VM::throwError(const char* msg) {
  SQVM_TOPG; return sq_throwerror(vm, msg);
}

inline SQInteger VM::throwObject() {
  SQVM_TOPG; return g.check(sq_throwobject(vm), -1);
}


// Objects manipulation
inline void VM::arrayAppend(SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_arrayappend(vm, idx)); g.check(-1);
}

inline void VM::arrayInsert(SQInteger pos, SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_arrayinsert(vm, idx, pos)); g.check(-1);
}

inline void VM::arrayPop(SQInteger idx, bool doPush) {
  SQVM_TOPG; SQVM_ASS(sq_arraypop(vm, idx, doPush? SQTrue: SQFalse)); g.check(doPush? 1: 0);
}

inline void VM::arrayRemove(SQInteger pos, SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_arrayremove(vm, idx, pos));
}

inline void VM::arrayResize(SQInteger size, SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_arrayresize(vm, idx, size));
}

inline void VM::arrayReverse(SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_arrayreverse(vm, idx));
}

inline void VM::valueClear(SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_clear(vm, idx));
}

inline void VM::valueClone(SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_clone(vm, idx)); g.check(1);
}

inline void VM::deleteSlot(SQInteger idx, bool doPush) {
  SQVM_TOPG; SQVM_ASS(sq_deleteslot(vm, idx, doPush? SQTrue: SQFalse)); g.check(doPush? 1: 0);
}

inline void VM::pushSlotValue(SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_get(vm, idx));
}

inline void VM::pushBaseOf(SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_getbase(vm, idx)); g.check(1);
}

inline void VM::pushClassOf(SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_getclass(vm, idx)); g.check(1);
}

inline bool VM::instanceOf() {
  SQVM_TOPG; return g.check(sq_instanceof(vm), -2);
}

inline void VM::newSlot(SQInteger idx, bool isStatic) {
  SQVM_TOPG;
  if ((valueType(idx) != OT_CLASS) && (valueType(idx) != OT_TABLE))
    throw Error(this, idx, (boost::format("Can't create slot in value of type %1")
                % valueTypeName(idx)).str());
  SQVM_ASS(sq_newslot(vm, idx, isStatic? SQTrue: SQFalse));
  g.check(-2);
}

template <typename Key, typename Value>
inline void VM::makeSlot(Key key, Value value, SQInteger idx, bool isStatic) {
  SQVM_TOPG;
  if (idx < 0) idx -= 2;
  (*this) << key << value;
  newSlot(idx, isStatic);
}

inline bool VM::next(SQInteger idx) {
  SQVM_TOPG;
  bool result = SQ_SUCCEEDED(sq_next(vm, idx));
  g.check(result? 2: 0);
  return result;
}

inline void VM::setSlot(SQInteger idx) {
  SQVM_TOPG; SQVM_ASS(sq_set(vm, idx)); g.check(-2);
}

template <typename Key, typename Value>
inline void VM::setSlot(Key key, Value value, SQInteger idx) {
  SQVM_TOPG;
  if (idx < 0) idx -= 2;
  (*this) << key << value;
  setSlot(idx);
}

inline void VM::setDelegate(SQInteger idx) {
  SQVM_TOPG; sq_setdelegate(vm, idx); g.check(-1);
}

template <typename Key>
inline void VM::pushField(Key field, int idx) {
  SQVM_TOPG;
  if (idx < 0) idx -= 1;
  (*this) << field;
  if (!SQ_SUCCEEDED(sq_get(vm, idx)))
    throw Error(this, idx, (boost::format("Can't get field: %1")
                % field).str());
  g.check(1);
}

inline void VM::pushRootTable() {
  SQVM_TOPG; sq_pushroottable(vm); g.check(1);
}

inline void VM::pushRawClosure(SQFUNCTION func, SQInteger freeVars) {
  SQVM_TOPG; sq_newclosure(vm, func, freeVars); g.check(1 - freeVars);
}

inline void VM::compile(const std::string& code, const std::string& fileName) {
  SQVM_TOPG;
  SQVM_ASS(sq_compilebuffer(vm, code.c_str(), code.size(), fileName.c_str(), SQTrue));
  g.check(1);
}

inline void VM::exec(const std::string& code, const std::string& fileName) {
  const int top = getTop();
  compile(code, fileName);
  pushRootTable();
  call(1, false);
  setTop(top);
}

inline void VM::doFile(const std::string& fileName) {
  const int top = getTop();
  pushRootTable();
  SQVM_ASS(sqstd_dofile(vm, fileName.c_str(), SQFalse, SQTrue));
  setTop(top);
}

// Data types:

// integer

inline SQInteger VM::getInt(SQInteger idx) const {
  SQInteger v;
  if (!SQ_SUCCEEDED(sq_getinteger(vm, idx, &v)))
    throw Error(this, idx, (boost::format("Expected integer but got value of type %1")
                % valueTypeName(idx)).str());
  return v;
}

template <typename Key>
inline SQInteger VM::getIntField(Key key, SQInteger idx) const {
  SQVM_CTOPG;
  const_cast<VM*>(this)->pushField(key, idx);
  SQInteger value;
  const_cast<VM&>(*this) >> value;
  return value;
}

inline VM& VM::operator >> (SQInteger& data) {
  SQVM_TOPG;
  data = getInt();
  pop();
  g.check(-1);
}

inline VM& VM::operator << (SQInteger data) {
  SQVM_TOPG; sq_pushinteger(vm, data); g.check(1);
  return *this;
}

inline SQInteger VM::Any::getInt() const {
  return sq_objtointeger(&obj);
}

// float

inline SQFloat VM::getFloat(SQInteger idx) const {
  SQFloat v;
  if (!SQ_SUCCEEDED(sq_getfloat(vm, idx, &v)))
    throw Error(this, idx, (boost::format("Expected float but got value of type %1")
                % valueTypeName(idx)).str());
  return v;
}

template <typename Key>
inline SQFloat VM::getFloatField(Key key, SQInteger idx) const {
  SQVM_CTOPG;
  const_cast<VM*>(this)->pushField(key, idx);
  SQFloat value;
  const_cast<VM&>(*this) >> value;
  return value;
}

inline VM& VM::operator >> (SQFloat& data) {
  SQVM_TOPG;
  data = getFloat();
  pop();
  g.check(-1);
}

inline VM& VM::operator << (SQFloat data) {
  SQVM_TOPG; sq_pushfloat(vm, data); g.check(1);
  return *this;
}

inline SQFloat VM::Any::getFloat() const {
  return sq_objtofloat(&obj);
}

// string

inline std::string VM::getString(SQInteger idx) const {
  const SQChar* str;
  if (!SQ_SUCCEEDED(sq_getstring(vm, idx, &str)))
    throw Error(this, idx, (boost::format("Expected string but got value of type %1")
                % valueTypeName(idx)).str());
  return std::string(str);
}

inline std::string VM::getAsString(SQInteger idx) const {
  SQVM_CTOPG;
  if (!SQ_SUCCEEDED(sq_tostring(vm, idx)))
    throw Error(this, idx, (boost::format("Can't convert value of type %1 to string")
                % valueTypeName(idx)).str());
  std::string result;
  const_cast<VM&>(*this) >> result;
  return result;
}

template <typename Key>
inline std::string VM::getStringField(Key key, SQInteger idx) const {
  SQVM_CTOPG;
  const_cast<VM*>(this)->pushField(key, idx);
  std::string value;
  const_cast<VM&>(*this) >> value;
  return value;
}

inline VM& VM::operator >> (std::string& data) {
  SQVM_TOPG;
  data = getString();
  pop();
  g.check(-1);
}

inline VM& VM::operator << (const std::string& data) {
  SQVM_TOPG; sq_pushstring(vm, data.c_str(), data.size()); g.check(1);
  return *this;
}

inline std::string VM::Any::getString() const {
  return std::string(sq_objtostring(&obj));
}

// bool

inline bool VM::getBool(SQInteger idx) const {
  SQBool v;
  if (!SQ_SUCCEEDED(sq_getbool(vm, idx, &v)))
    throw Error(this, idx, (boost::format("Expected bool but got value of type %1")
                % valueTypeName(idx)).str());
  return v;
}

inline bool VM::getAsBool(SQInteger idx) const {
  SQBool v;
  sq_tobool(vm, idx, &v);
  return v;
}

template <typename Key>
inline bool VM::getBoolField(Key key, SQInteger idx) const {
  SQVM_CTOPG;
  const_cast<VM*>(this)->pushField(key, idx);
  bool value;
  const_cast<VM&>(*this) >> value;
  return value;
}

inline VM& VM::operator >> (bool& data) {
  SQVM_TOPG;
  data = getBool();
  pop();
  g.check(-1);
}

inline VM& VM::operator << (bool data) {
  SQVM_TOPG; sq_pushbool(vm, data? SQTrue: SQFalse); g.check(1);
  return *this;
}

inline bool VM::Any::getBool() const {
  return sq_objtobool(&obj);
}

// any

inline VM::Any VM::get(SQInteger idx) const {
  return VM::Any(const_cast<VM*>(this), idx);
}

template <typename Key>
inline VM::Any VM::getField(Key key, SQInteger idx) const {
  SQVM_CTOPG;
  const_cast<VM*>(this)->pushField(key, idx);
  VM::Any value;
  const_cast<VM&>(*this) >> value;
  return value;
}

inline VM& VM::operator >> (Any& data) {
  SQVM_TOPG;
  data.reset(this);
  pop();
  g.check(-1);
}

inline VM& VM::operator << (const Any& data) {
  SQVM_TOPG; sq_pushobject(vm, data.obj); g.check(1);
  return *this;
}

}

#undef SQVM_TOPG
#undef SQVM_LTOPG
#undef SQVM_CTOPG
#undef SQVM_ASS

