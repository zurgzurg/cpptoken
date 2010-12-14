#ifndef _CPPTOKEN_H_
#define _CPPTOKEN_H_

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

namespace cpptoken {

/*******************************************************/

class MemoryControl {
 public:
  virtual void *allocate(size_t);
  virtual void deallocate(void *, size_t);
};

/*******************************************************/

class SyntaxError : public std::exception {
  private:
    size_t m_errIdx;
    const char *m_reason;

  public:
    SyntaxError(size_t idx, const char *msg)
      : std::exception(),
        m_errIdx(idx),
        m_reason(msg) {;};

    size_t getErrorIndex() const throw();
};

/*******************************************************/

class Buffer {
 private:
  
 public:
  Buffer(MemoryControl *);
};

/*******************************************************/
class Builder {
 private:

 public:
  Builder(MemoryControl *);
  ~Builder();
  static void *operator new(size_t);
  static void *operator new(size_t, MemoryControl *);
  static void operator delete(void *, MemoryControl *);
};

}

#endif
