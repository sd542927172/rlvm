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
// Copyright (c) 2004-2006  Kazunori "jagarl" Ueno
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_LITTLEBUSTERSEF00DLL_HPP_
#define SRC_SYSTEMS_BASE_LITTLEBUSTERSEF00DLL_HPP_

#include "MachineBase/RealLiveDLL.hpp"

#include <string>
#include <boost/scoped_array.hpp>

class RLMachine;

// A port of Jagarl's EF00.DLL emulation from xclannad to rlvm. This is
// basically all Jagarl's work; I (Elliot) have only changed the inputs and
// output formats.
//
// For the record, I have no idea how this works.
class LittleBustersEF00DLL : public RealLiveDLL {
 public:
  // Overridden from RealLiveDLL:
  virtual int callDLL(RLMachine& machine, int func, int arg1, int arg2,
                      int arg3, int arg4);
  virtual const std::string& name() const;

 private:
  void configureEffect(int arg1, int arg2, int arg3, int arg4);
  void performCalculations(RLMachine& machine, int arg1);

  boost::scoped_array<double> lb_ef_param;
};

#endif  // SRC_SYSTEMS_BASE_LITTLEBUSTERSEF00DLL_HPP_
