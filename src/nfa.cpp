// Copyright (c) 2010, Ram Bhamidipaty
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above
//       copyright notice, this list of conditions and the
//       following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials
//       provided with the distribution.
// 
//     * Neither the name of Ram Bhamidipaty nor the names of its
//       contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <memory>
#include <limits>
#include <list>
#include <vector>
#include <iostream>
using namespace std;

#include "cpptoken.h"
#include "cpptoken_private.h"
using namespace cpptoken;

/********************************/
static void *
NFA::operator new(size_t sz)
{
  void *ptr = ::operator new(sz);
  return ptr;
}

static void *
NFA::operator new(size_t sz, MemoryControl *mc)
{
  void *ptr = mc->allocate(sz);
  return ptr;
}

static void
NFA::operator delete(void *ptr, size_t sz, MemoryControl *mc)
{
  mc->deallocate(ptr, sz);
}

/********************************/

Builder::Builder(MemoryControl *mc)
{
  this->m_mc = mc;
  this->m_pats = NULL;
}

Builder::~Builder()
{
  this->m_mc = NULL;
  if (this->m_pats != NULL) {
    delete this->m_pats;
  }
}

/********************************/

static void *
Builder::operator new(size_t sz)
{
  void *ptr = ::operator new(sz);
  return ptr;
}

static void *
Builder::operator new(size_t sz, MemoryControl *mc)
{
  void *ptr = mc->allocate(sz);
  return ptr;
}

static void
Builder::operator delete(void *ptr, size_t sz, MemoryControl *mc)
{
  mc->deallocate(ptr, sz);
}

/********************************/

void
Builder::addRegEx(const char *ptr, action_func fp, void *arg)
{
  if (this->m_pats == NULL) {
    Alloc<PatternAction *> allocObj;
    allocObj.setMC(this->m_mc);
    this->m_pats = new list<PatternAction *, Alloc<PatternAction *> >(allocObj);
  }
}

NFA *
Builder::BuildNFA(MemoryControl *nfaMC, BuilderLimits *NFALim)
{
  NFA *res = new (nfaMC) NFA();
  return res;
}

/********************************/
stateNum
NFA::getNumStates() const
{
  return 0;
}
