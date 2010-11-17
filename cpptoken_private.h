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
  SELF_CHAR,

  CCAT,
  PIPE,
  STAR,

  DOT,

  QMARK,
  LBRACE,
  RBRACE,
  LPAREN,
  RPAREN,

  CHAR_CLASS,

  num_tokType      /* not an actual type */
};

typedef unsigned char uchar;

/********************************/

struct REToken {
  TokType ttype;
  uchar ch;

  REToken(TokType tt, uchar c='\0') : ttype(tt), ch(c) {;};
};
  
/********************************/

struct TokenList {
  list<REToken *>  toks;
  list<REToken *>::iterator iter;

  TokenList(const char *);
  TokenList(const char *, size_t idx, size_t len);
  ~TokenList();

  bool equals(list<REToken *>::iterator, TokType, uchar = '\0');

  void beginIteration();
  bool verifyNext(TokType, uchar = '\0');
  bool verifyEnd();

private:
  void build(const char *, size_t idx, size_t len);
  void addRange(bool invert, const list<uchar> &);
  void computeInverseRange(list<uchar> &result, const list<uchar> &src);
  void simpleAddToken(TokType, uchar = '\0');
  void addTokenAndMaybeCcat(TokType, uchar = '\0');
  void maybeAddCcat(TokType);
};

}

#endif
