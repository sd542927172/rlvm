// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2010 Elliot Glaysher
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// -----------------------------------------------------------------------

#ifndef SRC_MACHINEBASE_GENERALOPERATIONS_IMPL_HPP_
#define SRC_MACHINEBASE_GENERALOPERATIONS_IMPL_HPP_

// Implementation details of the GeneralOperations functions. This internal
// interface is hidden and should only be accessed through the functions
// exposed in GeneralOperations.hpp.

// Contains details on how to get a certain object from an input
// RLMachine. Templates are specialized in the cpp file.
namespace getSystemObjImpl {

template<typename RETTYPE>
RETTYPE& getSystemObj(RLMachine& machine);

template<>
inline RLMachine& getSystemObj(RLMachine& machine) {
  return machine;
}

// Equivalent to machine.system().
template<>
System& getSystemObj(RLMachine& machine);

// Equivalent to machine.system().event().
template<>
EventSystem& getSystemObj(RLMachine& machine);

// Equivalent to machine.system().graphics().
template<>
GraphicsSystem& getSystemObj(RLMachine& machine);

// Equivalent to machine.system().text().
template<>
TextSystem& getSystemObj(RLMachine& machine);

// Equivalent to machine.system().sound().
template<>
SoundSystem& getSystemObj(RLMachine& machine);

// Equivalent to machine.system().graphics().cgTable().
template<>
CGMTable& getSystemObj(RLMachine& machine);

// Equivalent to machine.system().text().currentPage().
template<>
TextPage& getSystemObj(RLMachine& machine);

}  // namespace getSystemObjImpl

namespace binderImpl {

// Binds setting an internal variable to a passed in value in from a
// running Reallive script.
template<typename OBJTYPE>
class Op_CallWithInt : public RLOp_Void_1< IntConstant_T > {
 public:
  // The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(const int);

  explicit Op_CallWithInt(Setter s)
      : setter(s) {
  }

  void operator()(RLMachine& machine, int in) {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*setter)(in);
  }

 private:
  /// The setter function to call on Op_SetToIncoming::reference when
  /// called.
  Setter setter;
};

// Binds setting an internal variable to a passed in value in from a
// running Reallive script.
template<typename OBJTYPE>
class Op_CallWithMachineInt : public RLOp_Void_1< IntConstant_T > {
 public:
  // The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(RLMachine&, const int);

  explicit Op_CallWithMachineInt(Setter s)
      : setter(s) {
  }

  void operator()(RLMachine& machine, int in) {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*setter)(machine, in);
  }

 private:
  // The setter function to call on Op_SetToIncoming::reference when
  // called.
  Setter setter;
};

// Binds setting an internal variable to a passed in value in from a
// running Reallive script.
template<typename OBJTYPE>
class Op_CallWithMachineIntInt : public RLOp_Void_2< IntConstant_T,
                                                     IntConstant_T > {
 public:
  // The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(RLMachine&, const int, const int);

  explicit Op_CallWithMachineIntInt(Setter s)
      : setter(s) {
  }

  void operator()(RLMachine& machine, int one, int two) {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*setter)(
        machine, one, two);
  }

 private:
  // The setter function to call on Op_SetToIncoming::reference when
  // called.
  Setter setter;
};

// Binds setting an internal variable to a passed in value in from a
// running Reallive script.
template<typename OBJTYPE>
class Op_CallWithIntInt : public RLOp_Void_2< IntConstant_T, IntConstant_T > {
 public:
  // The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(const int, const int);

  explicit Op_CallWithIntInt(Setter s)
      : setter(s) {
  }

  void operator()(RLMachine& machine, int in1, int in2) {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*setter)(in1, in2);
  }

 private:
  // The setter function to call on Op_SetToIncoming::reference when
  // called.
  Setter setter;
};

// Binds setting an internal variable to a passed in value in from a
// running Reallive script.
template<typename OBJTYPE>
class Op_CallWithString : public RLOp_Void_1< StrConstant_T > {
 public:
  // The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(const std::string&);

  explicit Op_CallWithString(Setter s)
      : setter(s) {
  }

  void operator()(RLMachine& machine, std::string incoming) {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*setter)(incoming);
  }

 private:
  // The setter function to call on Op_CallWithString::reference
  // when called.
  Setter setter;
};

template<typename OBJTYPE>
class Op_CallMethod : public RLOp_Void_Void {
 public:
  // The string getter function to call
  typedef void(OBJTYPE::*FUNCTYPE)();

  explicit Op_CallMethod(FUNCTYPE f)
      : func(f) {
  }

  void operator()(RLMachine& machine) {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*func)();
  }

 private:
  FUNCTYPE func;
};

// Sets an internal variable to a specific value set at compile time,
// and exposes this as an operation to Reallive scripts.
template<typename OBJTYPE, typename VALTYPE>
class Op_CallWithConstant : public RLOp_Void_Void {
 public:
  typedef void(OBJTYPE::*Setter)(VALTYPE);

  Op_CallWithConstant(Setter s, VALTYPE in_val)
      : setter(s), value(in_val) {
  }

  void operator()(RLMachine& machine) {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*setter)(value);
  }

 private:
  Setter setter;
  VALTYPE value;
};

template<typename RETTYPE>
class Op_ReturnFunctionIntValue : public RLOp_Store_Void {
 public:
  typedef RETTYPE(*Getter)();

  explicit Op_ReturnFunctionIntValue(Getter g)
      : getter_(g) {
  }

  int operator()(RLMachine& machine) {
    return (*getter_)();
  }

 private:
  Getter getter_;
};

// Reads the value of an internal variable in a generic way using an
// arbitrary getter function and places it in the store register.
template<typename OBJTYPE, typename RETTYPE>
class Op_ReturnIntValue : public RLOp_Store_Void {
 public:
  typedef RETTYPE(OBJTYPE::*Getter)() const;

  explicit Op_ReturnIntValue(Getter g)
      : getter_(g) {
  }

  int operator()(RLMachine& machine) {
    return (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*getter_)();
  }

 private:
  Getter getter_;
};

template<typename OBJTYPE, typename RETTYPE>
class Op_ReturnIntValueWithInt : public RLOp_Store_1<IntConstant_T> {
 public:
  typedef RETTYPE(OBJTYPE::*Getter)(const int) const;

  explicit Op_ReturnIntValueWithInt(Getter g)
      : getter_(g) {
  }

  int operator()(RLMachine& machine, int one) {
    return (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*getter_)(one);
  }

 private:
  Getter getter_;
};

template<typename OBJTYPE, typename RETTYPE>
class Op_ReturnIntValueWithString : public RLOp_Store_1<StrConstant_T> {
 public:
  typedef RETTYPE(OBJTYPE::*Getter)(const std::string&) const;

  explicit Op_ReturnIntValueWithString(Getter g)
      : getter_(g) {
  }

  int operator()(RLMachine& machine, string one) {
    return (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*getter_)(one);
  }

 private:
  Getter getter_;
};

// Reads the value of an internal variable in a generic way using an
// arbitrary getter function and places it in a passed in reference.
template<typename OBJTYPE>
class Op_ReturnStringValue : public RLOp_Void_1< StrReference_T > {
 public:
  /// The signature of a string getter function
  typedef const std::string&(OBJTYPE::*Getter)() const;  // NOLINT

  explicit Op_ReturnStringValue(Getter g)
      : getter_(g) {
  }

  void operator()(RLMachine& machine, StringReferenceIterator dest) {
    *dest = (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*getter_)();
  }

 private:
  /// The string getter function to call
  Getter getter_;
};

}  // namespace binderImpl

#endif
