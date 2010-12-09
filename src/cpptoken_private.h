#ifndef _CPPTOKEN_PRIVATE_H_
#define _CPPTOKEN_PRIVATE_H_

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

/********************************/

enum TokType {
  TT_SELF_CHAR,

  TT_CCAT,
  TT_PIPE,
  TT_STAR,

  TT_DOT,

  TT_QMARK,
  TT_LBRACE,
  TT_RBRACE,
  TT_LPAREN,
  TT_RPAREN,

  TT_CHAR_CLASS,
  TT_QUANTIFIER,

  TT_num      /* not an actual type */
};

typedef unsigned char uchar;

/********************************/

struct RETokQuantifier {
  bool m_v1Valid;
  bool m_v2Valid;
  size_t m_v1;
  size_t m_v2;
};

struct TokenList;

struct REToken {
  typedef list<uchar, Alloc<uchar> > UCharList;

  REToken *m_next; // chain of all objs
  TokType m_ttype;
  union {
    uchar m_ch;
    UCharList *m_charClass;
    RETokQuantifier quant;
  } u;
    
  REToken(TokenList *, TokType tt, uchar c='\0');
};

/********************************/

struct TokenList {
  typedef list<REToken *, Alloc<REToken *> > TokList;

  TokList  m_toks;
  TokList::iterator m_iter;
  REToken *m_allREToks;
  REToken::UCharList *m_tmpCharList;
  REToken::UCharList *m_tmpInvCharList;

  TokenList(Alloc<REToken *>, const char *);
  TokenList(Alloc<REToken *>, const char *, size_t idx, size_t len);
  ~TokenList();

  bool equals(TokList::iterator, TokType, uchar = '\0');

  bool verifyCharClassLength(size_t);
  bool verifyCharClassMember(uchar);

  void beginIteration();
  void incrementIterator();
  bool verifyNext(TokType, uchar = '\0');
  bool verifyNextCharClass(const char *exp, size_t n_exp);
  bool verifyNextQuantifier(bool, size_t v1, bool, size_t v2);
  bool verifyEnd();

private:
  void build(const char *, size_t idx, size_t len);

  const uchar *buildQuantifier(const uchar *, const uchar *, const uchar *);

  const uchar *buildCharClass(const uchar *, const uchar *, const uchar *);
  void addRange(bool invert);
  void addToCharClass(REToken::UCharList  *, uchar, uchar);
  void createInverseRange();

  void simpleAddToken(TokType, uchar = '\0');
  void addTokenAndMaybeCcat(TokType, uchar = '\0');
  void maybeAddCcat(TokType);

  void undoContructor(REToken *);
};


/********************************/

class FABase {
};

class NFA : public FABase {
};

}

#endif
