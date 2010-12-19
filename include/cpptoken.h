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

class FABase;
class NFA;
struct PatternAction;

/*******************************************************/
class MemoryControl {
 public:
  virtual void *allocate(size_t);
  virtual void deallocate(void *, size_t);
};

/*******************************************************/
template <class T>
class Alloc  {
  MemoryControl *mc;

 public:
  typedef T                value_type;
  typedef T*               pointer;
  typedef const T*         const_pointer;
  typedef T&               reference;
  typedef const T&         const_reference;
  typedef std::size_t      size_type;
  typedef std::ptrdiff_t   difference_type;

  template <class U>
  struct rebind {
    typedef Alloc<U> other;
  };

  pointer address (reference value) const {
    return &value;
  }

  const_pointer address (const_reference value) const {
    return &value;
  }

  Alloc() throw() {
    this->mc = NULL;
  }

  Alloc(const Alloc&other) throw() {
    this->mc = other.mc;
  }

  template <class U>
  Alloc(const Alloc<U>&other) throw() {
    this->mc = other.getMC();
  }

  ~Alloc() throw() {
  }

  size_type max_size () const throw() {
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
  }

  pointer allocate (size_type num, const void* = 0) {
    pointer ret = (pointer)this->mc->allocate(num * sizeof(T));
    return ret;
  }

  void construct (pointer p, const T& value) {
    new((void*)p)T(value);
  }

  void destroy (pointer p) {
    p->~T();
  }

  void deallocate (pointer p, size_type num) {
    this->mc->deallocate(p, num * sizeof(T));
  }

  ////////

  MemoryControl *getMC() const throw() {return this->mc;}
  void setMC(MemoryControl *obj) throw() { this->mc = obj; }
};

template <class T1, class T2>
bool operator== (const Alloc<T1>&, const Alloc<T2>&) throw() {
  return true;
}

template <class T1, class T2>
bool operator!= (const Alloc<T1>&, const Alloc<T2>&) throw() {
  return false;
}

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
typedef void *(*action_func)(void *userArg, const char *str, size_t len);

class BuilderLimits {
  size_t m_maxTimeInSeconds;
  size_t m_maxStates;

 public:
  BuilderLimits() : m_maxTimeInSeconds(0), m_maxStates(0) {;};
  BuilderLimits(size_t tm, size_t st)
    : m_maxTimeInSeconds(0),
      m_maxStates(0) {;};
};

class Builder {
 private:
  MemoryControl *m_mc;
  list<PatternAction *, Alloc<PatternAction *> > *m_pats;

 public:
  Builder(MemoryControl *);
  ~Builder();

  static void *operator new(size_t sz);
  static void *operator new(size_t sz, MemoryControl *mc);
  static void operator delete(void *ptr, size_t sz, MemoryControl *mc);

  /* public api */
  void addRegEx(const char *, action_func, void *userArg);
  
  /* not for external use */
  NFA *BuildNFA(MemoryControl *, BuilderLimits *);
};


}

#endif
