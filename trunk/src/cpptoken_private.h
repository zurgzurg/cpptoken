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

struct REToken {
  TokType m_ttype;
  union {
    uchar m_ch;
    list<uchar> *m_charClass;
    RETokQuantifier quant;
  } u;

  REToken(TokType tt, uchar c='\0');
};
  
/********************************/

struct TokenList {
  list<REToken *>  m_toks;
  list<REToken *>::iterator m_iter;

  TokenList(const char *);
  TokenList(const char *, size_t idx, size_t len);
  ~TokenList();

  bool equals(list<REToken *>::iterator, TokType, uchar = '\0');

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

  const uchar *buildCharClass(const uchar *, const uchar *);
  void addRange(bool invert, list<uchar> *);
  void addToCharClass(list<uchar>  *, uchar, uchar);
  list<uchar> *computeInverseRange(const list<uchar> *src);

  void simpleAddToken(TokType, uchar = '\0');
  void addTokenAndMaybeCcat(TokType, uchar = '\0');
  void maybeAddCcat(TokType);
};

}

#endif
