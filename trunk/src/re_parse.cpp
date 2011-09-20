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

#include <cstring>

#include <memory>
#include <limits>
#include <list>
#include <vector>
#include <iostream>
using namespace std;

#include "cpptoken.h"
#include "cpptoken_private.h"
using namespace cpptoken;

/********************************************************/

REToken::REToken(TokenList *tlist, TokType tt, uchar c)
  : m_ttype(tt)
{
  switch (tt) {
  case TT_SELF_CHAR:
    this->u.m_ch = c;
    break;

  case TT_CCAT:
  case TT_PIPE:
  case TT_STAR:
  case TT_DOT:
  case TT_QMARK:
  case TT_LPAREN:
  case TT_RPAREN:
    break;

  case TT_CHAR_CLASS:
    this->u.m_charClass = NULL;
    break;

  case TT_QUANTIFIER:
    this->u.m_quant.m_v1Valid = false;
    this->u.m_quant.m_v2Valid = false;
    this->u.m_quant.m_v1 = 0;
    this->u.m_quant.m_v2 = 0;
    break;

  case TT_num:
    break;
  }

  this->m_next = tlist->m_allREToks;
  tlist->m_allREToks = this;
}

REToken::REToken(TokenList2 *tlist, TokType tt, uchar c)
  : m_ttype(tt)
{
  switch (tt) {
  case TT_SELF_CHAR:
    this->u.m_ch = c;
    break;

  case TT_CCAT:
  case TT_PIPE:
  case TT_STAR:
  case TT_DOT:
  case TT_QMARK:
  case TT_LPAREN:
  case TT_RPAREN:
    break;

  case TT_CHAR_CLASS:
    this->u.m_charClass = NULL;
    break;

  case TT_QUANTIFIER:
    this->u.m_quant.m_v1Valid = false;
    this->u.m_quant.m_v2Valid = false;
    this->u.m_quant.m_v1 = 0;
    this->u.m_quant.m_v2 = 0;
    break;

  case TT_num:
    break;
  }

  this->m_next = tlist->m_allREToks;
  tlist->m_allREToks = this;
}

REToken::REToken(TokenList2 *tlist, const REToken *other)
{
  this->m_ttype = other->m_ttype;

  switch (other->m_ttype) {
  case TT_SELF_CHAR:
    this->u.m_ch = other->u.m_ch;
    break;

  case TT_CCAT:
  case TT_PIPE:
  case TT_STAR:
  case TT_DOT:
  case TT_QMARK:
  case TT_LPAREN:
  case TT_RPAREN:
    break;

  case TT_CHAR_CLASS:
    {
      this->u.m_charClass = new UCharList(tlist->m_toks.get_allocator());
      UCharList::const_iterator iter = other->u.m_charClass->begin();
    }
    break;

  case TT_QUANTIFIER:
    if (other->u.m_quant.m_v1Valid) {
      this->u.m_quant.m_v1Valid = true;
      this->u.m_quant.m_v1 = other->u.m_quant.m_v1;
    }
    else {
      this->u.m_quant.m_v1Valid = false;
      this->u.m_quant.m_v1 = 0;
    }

    if (other->u.m_quant.m_v2Valid) {
      this->u.m_quant.m_v2Valid = true;
      this->u.m_quant.m_v2 = other->u.m_quant.m_v2;
    }
    else {
      this->u.m_quant.m_v2Valid = false;
      this->u.m_quant.m_v2 = 0;
    }
    break;

  case TT_num:
    break;
  }

  this->m_next = tlist->m_allREToks;
  tlist->m_allREToks = this;
}

static void *
REToken::operator new(size_t sz, MemoryControl *mc)
{
  void *ret = mc->allocate(sz);
  return ret;
}

static void
REToken::operator delete(void *ptr, MemoryControl *mc)
{
  mc->deallocate(ptr, 1);
}

/********************************************************/
TokenList::TokenList(MemoryControl *mc,
		     Alloc<REToken *> obj,
		     const char *regex)
  : m_mc(mc),
    m_toks(obj),
    m_allREToks(NULL),
    m_tmpCharList(NULL),
    m_tmpInvCharList(NULL)
{
  this->m_toks.clear();
  size_t len = strlen(regex);
  try {
    this->build(regex, 0, len);
  }
  catch (const SyntaxError &e) {
    this->undoContructor(this->m_allREToks);
    throw;
  }
  catch (const bad_alloc &e) {
    this->undoContructor(this->m_allREToks);
    throw;
  }
}

TokenList::TokenList(MemoryControl *mc,
		     Alloc<REToken *> obj,
		     const char *regex,
		     size_t start, size_t len)
  : m_mc(mc),
    m_toks(obj),
    m_allREToks(NULL),
    m_tmpCharList(NULL),
    m_tmpInvCharList(NULL)
{
  this->m_toks.clear();
  try {
    this->build(regex, start, len);
  }
  catch (const SyntaxError &e) {
    this->undoContructor(this->m_allREToks);
    throw;
  }
  catch (const bad_alloc &e) {
    this->undoContructor(this->m_allREToks);
    throw;
  }
}

static void *
TokenList::operator new(size_t sz, MemoryControl *mc)
{
  void *ret = mc->allocate(sz);
  return ret;
}

static void
TokenList::operator delete(void *ptr, MemoryControl *mc)
{
  mc->deallocate(ptr, 1);
}

TokenList::~TokenList()
{
  this->undoContructor(this->m_allREToks);
  return;
}

void
TokenList::undoContructor(REToken *ptr)
{
  while (ptr != NULL) {
    REToken *tmp = ptr->m_next;
    if (ptr->m_ttype == TT_CHAR_CLASS && ptr->u.m_charClass)
      delete ptr->u.m_charClass;
    ptr->~REToken();
    this->m_mc->deallocate(ptr, sizeof(*ptr));
    ptr = tmp;
  }

  if (this->m_tmpCharList) {
    delete this->m_tmpCharList;
    this->m_tmpCharList = NULL;
  }

  if (this->m_tmpInvCharList) {
    delete this->m_tmpInvCharList;
    this->m_tmpInvCharList = NULL;
  }
}

/*******************************************************/
void
TokenList::build(const char *regex, size_t start, size_t len)
{
  const uchar *ptr, *last_valid;
  uchar ch;
  
  ptr = (uchar *)regex + start;
  last_valid = (uchar *)regex + start + len - 1;
  while (ptr <= last_valid) {
    ch = *ptr;
    
    switch (ch) {
    case '*':
      this->simpleAddToken(TT_STAR);
      break;
    case '|':
      this->simpleAddToken(TT_PIPE);
      break;
    case '(':
      this->addTokenAndMaybeCcat(TT_LPAREN);
      break;
    case ')':
      this->addTokenAndMaybeCcat(TT_RPAREN, ch);
      break;
    case '[':
      ptr = this->buildCharClass((const uchar *)regex, ptr, last_valid);
      break;
    case '{':
      ptr = this->buildQuantifier((const uchar *)regex, ptr, last_valid);
      break;
    default:
      this->addTokenAndMaybeCcat(TT_SELF_CHAR, ch);
      break;
    }

    ptr++;
  }

  return;
}

const uchar *
TokenList::buildQuantifier(const uchar *start, const uchar *ptr,
			   const uchar *last_valid)
{
  size_t v1, v2, tmp;
  uchar ch;
  bool v1_found, v2_found;
  REToken *tok;

  v1_found = false;
  v1 = 0;
  v2_found = false;
  v2 = 0;

  ptr++;

  while (ptr <= last_valid) {
    ch = *ptr;
    if (ch == ' ' || ch == '\t') {
      ptr++;
      continue;
    }
    break;
  }

  while (ptr <= last_valid) {
    ch = *ptr;
    if (ch < '0' || ch > '9')
      break;
    v1_found = true;
    tmp = v1 * 10;
    if (tmp / 10 != v1) {
      size_t idx = ptr - start;
      throw SyntaxError(idx, "Quantifier too large");
    }
    v1 = tmp + (ch - '0');
    ptr++;
  }

  while (ptr <= last_valid) {
    ch = *ptr;
    if (ch == ' ' || ch == '\t') {
      ptr++;
      continue;
    }
    break;
  }

  if (ch == ',')
    ptr++;
  else if (ch != '}') {
    size_t idx = ptr - start;
    throw SyntaxError(idx, "Bad quantifier");
  }

  while (ptr <= last_valid) {
    ch = *ptr;
    if (ch == ' ' || ch == '\t') {
      ptr++;
      continue;
    }
    break;
  }

  if (ch == '}') {
    tok = new (this->m_mc) REToken(this, TT_QUANTIFIER);
    tok->u.m_quant.m_v1 = v1;
    tok->u.m_quant.m_v2 = 0;
    tok->u.m_quant.m_v1Valid = true;
    tok->u.m_quant.m_v2Valid = false;
    this->m_toks.push_back(tok);
    ptr++;
    return ptr;
  }

  while (ptr <= last_valid) {
    ch = *ptr;
    if (ch < '0' || ch > '9')
      break;
    v2_found = true;
    tmp = v2 * 10;
    if ( tmp / 10 != v2) {
      size_t idx = ptr - start;
      throw SyntaxError(idx, "Quantifier too large");
    }
    v2 = tmp + (ch - '0');
    ptr++;
  }

  while (ptr <= last_valid) {
    ch = *ptr;
    if (ch == ' ' || ch == '\t') {
      ptr++;
      continue;
    }
    break;
  }

  if (ch == '}') {
    tok = new (this->m_mc) REToken(this, TT_QUANTIFIER);
    tok->u.m_quant.m_v1 = v1;
    tok->u.m_quant.m_v2 = v2;
    tok->u.m_quant.m_v1Valid = v1_found;
    tok->u.m_quant.m_v2Valid = v2_found;
    this->m_toks.push_back(tok);
    ptr++;
    return ptr;
  }

  return ptr;
}

const uchar *
TokenList::buildCharClass(const uchar *start, const uchar *ptr,
			  const uchar *last_valid)
{
  uchar prev, cur;
  const uchar *char_class_start;
  int state;
  bool is_invert = false, close_found = false;

  this->m_tmpCharList = new REToken::UCharList(this->m_toks.get_allocator());

  char_class_start = ptr;
  ptr++;

  if (*ptr == '^') {
    is_invert = true;
    ptr++;
  }

  state = 0;
  while (ptr <= last_valid) {
    cur = *ptr;

    if (state == 0) {
      /* zero state - go nothing */
      if (cur == '-') {
	this->m_tmpCharList->push_back(cur);
	ptr++;
      }
      else if (cur == ']') {
	close_found = true;
	ptr++;
	break;
      }
      else {
	state = 1;
	prev = cur;
	ptr++;
      }
      continue;
    }

    else if (state == 1) {
      /* got a previous char - maybe the start of a range x-y */
      if (cur == '-') {
	state = 2;
	ptr++;
      }
      else if (cur == ']') {
	close_found = true;
	this->m_tmpCharList->push_back(prev);
	ptr++;
	break;
      }
      else {
	this->m_tmpCharList->push_back(prev);
	ptr++;
	prev = cur;
      }
      continue;
    }

    else if (state == 2) {
      /* maybe got a real range */
      if (cur == ']') {
	close_found = true;
	this->m_tmpCharList->push_back(prev);
	this->m_tmpCharList->push_back('-');
	ptr++;
	break;
      }
      else {
	this->addToCharClass(this->m_tmpCharList, prev, cur);
	ptr++;
	state = 0;
      }
      continue;
    }
  }

  if (!close_found) {
    delete this->m_tmpCharList;
    this->m_tmpCharList = NULL;
    size_t idx = char_class_start - start;
    throw SyntaxError(idx, "Unterminated char class");
  }

  this->addRange(is_invert);

  return ptr;
}

void
TokenList::addToCharClass(REToken::UCharList  *c_class, uchar v1, uchar v2)
{
  uchar lim1, lim2, code;

  if (v1 < v2) {
    lim1 = v1;
    lim2 = v2;
  }
  else {
    lim1 = v2;
    lim2 = v1;
  }

  for (code = lim1; code <= lim2; code++)
    c_class->push_back(code);

  return;
}

void
TokenList::addRange(bool invert)
{
  REToken *tok = new (this->m_mc) REToken(this, TT_CHAR_CLASS);

  if (invert) {
    this->createInverseRange();
    this->m_toks.push_back(tok);
    tok->u.m_charClass = this->m_tmpInvCharList;
    this->m_tmpInvCharList = NULL;
  }
  else {
    this->m_toks.push_back(tok);
    tok->u.m_charClass = this->m_tmpCharList;
    this->m_tmpCharList = NULL;
  }

  return;
}

void
TokenList::createInverseRange()
{
  unsigned char buf[256];
  
  for (int i = 0; i < 256; i++)
    buf[i] = 0;

  REToken::UCharList::const_iterator src_pos;
  src_pos = this->m_tmpCharList->begin();
  while (src_pos != this->m_tmpCharList->end()) {
    uchar ch = *src_pos;
    buf[ (unsigned int)ch ] = 1;
    src_pos++;
  }

  delete this->m_tmpCharList;
  this->m_tmpCharList = NULL;

  this->m_tmpInvCharList =
    new REToken::UCharList(this->m_toks.get_allocator());

  for (int i = 0; i < 256; i++) {
    if (buf[i] == 0)
      this->m_tmpInvCharList->push_back( (uchar) i );
  }

  return;
}

void
TokenList::simpleAddToken(TokType tp, uchar ch)
{
  REToken *tok = new (this->m_mc) REToken(this, tp, ch);
  this->m_toks.push_back(tok);
  return;
}

void
TokenList::addTokenAndMaybeCcat(TokType tp, uchar ch)
{
  this->maybeAddCcat(tp);
  REToken *tok = new (this->m_mc) REToken(this, tp, ch);
  this->m_toks.push_back(tok);
  return;
}

void
TokenList::maybeAddCcat(TokType cur_tp)
{
  if (this->m_toks.empty())
    return;

  if (cur_tp == TT_LPAREN || cur_tp == TT_RPAREN)
    return;

  TokList::iterator iter = this->m_toks.end();
  iter--;

  REToken *tok = *iter;

  switch (tok->m_ttype) {
  case TT_SELF_CHAR:
    tok = new (this->m_mc) REToken(this, TT_CCAT);
    this->m_toks.push_back(tok);
    break;
  default:
    break;
  }

  return;
}

/*******************************************************/
bool
TokenList::equals(TokList::iterator iter, TokType tp, uchar ch)
{
  REToken *ptr;

  if (iter == this->m_toks.end())
    return false;

  ptr = *iter;
  if (ptr->m_ttype != tp)
    return false;
  if (tp == TT_SELF_CHAR) {
    if (ptr->u.m_ch != ch)
      return false;
  }

  return true;
}

bool
TokenList::verifyCharClassLength(size_t exp)
{
  if (this->m_iter == this->m_toks.end())
    return false;
  REToken *tok = *this->m_iter;
  if (tok->m_ttype != TT_CHAR_CLASS)
    return false;
  size_t act = tok->u.m_charClass->size();
  if (act != exp)
    return false;
  return true;
}

bool
TokenList::verifyCharClassMember(uchar exp)
{
  if (this->m_iter == this->m_toks.end())
    return false;
  REToken *tok = *this->m_iter;
  if (tok->m_ttype != TT_CHAR_CLASS)
    return false;
  REToken::UCharList::iterator iter = tok->u.m_charClass->begin();
  while (iter != tok->u.m_charClass->end()) {
    if (*iter == exp)
      return true;
    ++iter;
  }
  return false;
}

void
TokenList::beginIteration()
{
  this->m_iter = this->m_toks.begin();
}

void
TokenList::incrementIterator()
{
  this->m_iter++;
}

bool
TokenList::verifyNext(TokType tp, uchar ch)
{
  bool result = this->equals(this->m_iter, tp, ch);
  if (this->m_iter != this->m_toks.end())
    this->m_iter++;
  return result;
}

bool
TokenList::verifyNextCharClass(const char *exp, size_t n_exp)
{
  if (this->m_iter == this->m_toks.end())
    return false;

  REToken *tok = *this->m_iter;

  if (tok->m_ttype != TT_CHAR_CLASS)
    return false;
  
  if (n_exp > 0 && tok->u.m_charClass == NULL)
    return false;

  bool result = true;

  for (size_t i = 0; i < n_exp; i++) {
    char exp_ch = exp[i];
    bool found = false;
    REToken::UCharList::const_iterator ch_iter = tok->u.m_charClass->begin();
    while (ch_iter != tok->u.m_charClass->end()) {
      uchar act_ch = *ch_iter;
      if ((uchar)exp_ch == act_ch) {
	found = true;
	break;
      }
      ch_iter++;
    }
    if (!found) {
      result = false;
      break;
    }
  }

  this->m_iter++;

  return result;
}

bool
TokenList::verifyNextQuantifier(bool v1v, size_t v1, bool v2v, size_t v2)
{
  if (this->m_iter == this->m_toks.end())
    return false;

  REToken *tok = *this->m_iter;

  if (tok->m_ttype != TT_QUANTIFIER)
    return false;
  
  if (v1v != tok->u.m_quant.m_v1Valid)
    return false;

  if (v2v != tok->u.m_quant.m_v2Valid)
    return false;
  
  if (v1v == true && tok->u.m_quant.m_v1 != v1)
    return false;

  if (v2v == true && tok->u.m_quant.m_v2 != v2)
    return false;

  return true;
}

bool
TokenList::verifyEnd()
{
  if (this->m_iter == this->m_toks.end())
    return true;
  return false;
}




/********************************************************/
/********************************************************/
TokenList2::TokenList2(MemoryControl *mc,
		       Alloc<REToken *> obj)
  : m_mc(mc),
    m_toks(obj),
    m_allREToks(NULL),
    m_tmpCharList(NULL),
    m_tmpInvCharList(NULL)
{
  this->m_toks.clear();
}

static void *
TokenList2::operator new(size_t sz, MemoryControl *mc)
{
  void *ret = mc->allocate(sz);
  return ret;
}

static void
TokenList2::operator delete(void *ptr, MemoryControl *mc)
{
  mc->deallocate(ptr, 1);
}

TokenList2::~TokenList2()
{
  REToken *ptr = this->m_allREToks;
  while (ptr != NULL) {
    REToken *tmp = ptr->m_next;
    if (ptr->m_ttype == TT_CHAR_CLASS && ptr->u.m_charClass)
      delete ptr->u.m_charClass;
    ptr->~REToken();
    this->m_mc->deallocate(ptr, sizeof(*ptr));
    ptr = tmp;
  }

  if (this->m_tmpCharList) {
    delete this->m_tmpCharList;
    this->m_tmpCharList = NULL;
  }

  if (this->m_tmpInvCharList) {
    delete this->m_tmpInvCharList;
    this->m_tmpInvCharList = NULL;
  }
  return;
}

/*******************************************************/
void
TokenList2::build(const char *regex)
{
  size_t l = strlen(regex);
  this->build(regex, 0, l);
}

void
TokenList2::build(const char *regex, size_t start, size_t len)
{
  const uchar *ptr, *last_valid;
  uchar ch;
  
  ptr = (uchar *)regex + start;
  last_valid = (uchar *)regex + start + len - 1;
  while (ptr <= last_valid) {
    ch = *ptr;
    
    switch (ch) {
    case '\\':
      ptr++;
      if (ptr > last_valid)
	throw SyntaxError(0, "illegal backslash at end of regex");
      ch = *ptr;
      this->addTokenAndMaybeCcat(TT_SELF_CHAR, ch);
      break;

    case '*':
      this->simpleAddToken(TT_STAR);
      break;
    case '|':
      this->simpleAddToken(TT_PIPE);
      break;
    case '(':
      this->addTokenAndMaybeCcat(TT_LPAREN);
      break;
    case ')':
      this->addTokenAndMaybeCcat(TT_RPAREN, ch);
      break;
    case '[':
      ptr = this->buildCharClass((const uchar *)regex, ptr, last_valid);
      break;
    case '{':
      ptr = this->buildQuantifier((const uchar *)regex, ptr, last_valid);
      break;
    default:
      this->addTokenAndMaybeCcat(TT_SELF_CHAR, ch);
      break;
    }

    ptr++;
  }

  return;
}

const uchar *
TokenList2::buildQuantifier(const uchar *start, const uchar *ptr,
			   const uchar *last_valid)
{
  size_t v1, v2, tmp;
  uchar ch;
  bool v1_found, v2_found;
  REToken *tok;

  v1_found = false;
  v1 = 0;
  v2_found = false;
  v2 = 0;

  ptr++;

  while (ptr <= last_valid) {
    ch = *ptr;
    if (ch == ' ' || ch == '\t') {
      ptr++;
      continue;
    }
    break;
  }

  while (ptr <= last_valid) {
    ch = *ptr;
    if (ch < '0' || ch > '9')
      break;
    v1_found = true;
    tmp = v1 * 10;
    if (tmp / 10 != v1) {
      size_t idx = ptr - start;
      throw SyntaxError(idx, "Quantifier too large");
    }
    v1 = tmp + (ch - '0');
    ptr++;
  }

  while (ptr <= last_valid) {
    ch = *ptr;
    if (ch == ' ' || ch == '\t') {
      ptr++;
      continue;
    }
    break;
  }

  if (ch == ',')
    ptr++;
  else if (ch != '}') {
    size_t idx = ptr - start;
    throw SyntaxError(idx, "Bad quantifier");
  }

  while (ptr <= last_valid) {
    ch = *ptr;
    if (ch == ' ' || ch == '\t') {
      ptr++;
      continue;
    }
    break;
  }

  if (ch == '}') {
    tok = new (this->m_mc) REToken(this, TT_QUANTIFIER);
    tok->u.m_quant.m_v1 = v1;
    tok->u.m_quant.m_v2 = 0;
    tok->u.m_quant.m_v1Valid = true;
    tok->u.m_quant.m_v2Valid = false;
    this->m_toks.push_back(tok);
    ptr++;
    return ptr;
  }

  while (ptr <= last_valid) {
    ch = *ptr;
    if (ch < '0' || ch > '9')
      break;
    v2_found = true;
    tmp = v2 * 10;
    if ( tmp / 10 != v2) {
      size_t idx = ptr - start;
      throw SyntaxError(idx, "Quantifier too large");
    }
    v2 = tmp + (ch - '0');
    ptr++;
  }

  while (ptr <= last_valid) {
    ch = *ptr;
    if (ch == ' ' || ch == '\t') {
      ptr++;
      continue;
    }
    break;
  }

  if (ch == '}') {
    tok = new (this->m_mc) REToken(this, TT_QUANTIFIER);
    tok->u.m_quant.m_v1 = v1;
    tok->u.m_quant.m_v2 = v2;
    tok->u.m_quant.m_v1Valid = v1_found;
    tok->u.m_quant.m_v2Valid = v2_found;
    this->m_toks.push_back(tok);
    ptr++;
    return ptr;
  }

  return ptr;
}

const uchar *
TokenList2::buildCharClass(const uchar *start, const uchar *ptr,
			   const uchar *last_valid)
{
  uchar prev, cur;
  const uchar *char_class_start;
  int state;
  bool is_invert = false, close_found = false;

  this->m_tmpCharList = new REToken::UCharList(this->m_toks.get_allocator());

  char_class_start = ptr;
  ptr++;

  if (*ptr == '^') {
    is_invert = true;
    ptr++;
  }

  state = 0;
  while (ptr <= last_valid) {
    cur = *ptr;

    if (state == 0) {
      /* zero state - go nothing */
      if (cur == '-') {
	this->m_tmpCharList->push_back(cur);
	ptr++;
      }
      else if (cur == ']') {
	close_found = true;
	ptr++;
	break;
      }
      else {
	state = 1;
	prev = cur;
	ptr++;
      }
      continue;
    }

    else if (state == 1) {
      /* got a previous char - maybe the start of a range x-y */
      if (cur == '-') {
	state = 2;
	ptr++;
      }
      else if (cur == ']') {
	close_found = true;
	this->m_tmpCharList->push_back(prev);
	ptr++;
	break;
      }
      else {
	this->m_tmpCharList->push_back(prev);
	ptr++;
	prev = cur;
      }
      continue;
    }

    else if (state == 2) {
      /* maybe got a real range */
      if (cur == ']') {
	close_found = true;
	this->m_tmpCharList->push_back(prev);
	this->m_tmpCharList->push_back('-');
	ptr++;
	break;
      }
      else {
	this->addToCharClass(this->m_tmpCharList, prev, cur);
	ptr++;
	state = 0;
      }
      continue;
    }
  }

  if (!close_found) {
    delete this->m_tmpCharList;
    this->m_tmpCharList = NULL;
    size_t idx = char_class_start - start;
    throw SyntaxError(idx, "Unterminated char class");
  }

  this->addRange(is_invert);

  return ptr;
}

void
TokenList2::addToCharClass(REToken::UCharList  *c_class, uchar v1, uchar v2)
{
  uchar lim1, lim2, code;

  if (v1 < v2) {
    lim1 = v1;
    lim2 = v2;
  }
  else {
    lim1 = v2;
    lim2 = v1;
  }

  for (code = lim1; code <= lim2; code++)
    c_class->push_back(code);

  return;
}

void
TokenList2::addRange(bool invert)
{
  REToken *tok = new (this->m_mc) REToken(this, TT_CHAR_CLASS);

  if (invert) {
    this->createInverseRange();
    this->m_toks.push_back(tok);
    tok->u.m_charClass = this->m_tmpInvCharList;
    this->m_tmpInvCharList = NULL;
  }
  else {
    this->m_toks.push_back(tok);
    tok->u.m_charClass = this->m_tmpCharList;
    this->m_tmpCharList = NULL;
  }

  return;
}

void
TokenList2::createInverseRange()
{
  unsigned char buf[256];
  
  for (int i = 0; i < 256; i++)
    buf[i] = 0;

  REToken::UCharList::const_iterator src_pos;
  src_pos = this->m_tmpCharList->begin();
  while (src_pos != this->m_tmpCharList->end()) {
    uchar ch = *src_pos;
    buf[ (unsigned int)ch ] = 1;
    src_pos++;
  }

  delete this->m_tmpCharList;
  this->m_tmpCharList = NULL;

  this->m_tmpInvCharList =
    new REToken::UCharList(this->m_toks.get_allocator());

  for (int i = 0; i < 256; i++) {
    if (buf[i] == 0)
      this->m_tmpInvCharList->push_back( (uchar) i );
  }

  return;
}

void
TokenList2::simpleAddToken(TokType tp, uchar ch)
{
  REToken *tok = new (this->m_mc) REToken(this, tp, ch);
  this->m_toks.push_back(tok);
  return;
}

void
TokenList2::addTokenAndMaybeCcat(TokType tp, uchar ch)
{
  this->maybeAddCcat(tp);
  REToken *tok = new (this->m_mc) REToken(this, tp, ch);
  this->m_toks.push_back(tok);
  return;
}

void
TokenList2::maybeAddCcat(TokType cur_tp)
{
  if (this->m_toks.empty())
    return;

  if (cur_tp == TT_RPAREN)
    return;

  REToken *tok = this->m_toks.back();

  switch (tok->m_ttype) {
  case TT_RPAREN:
  case TT_SELF_CHAR:
    tok = new (this->m_mc) REToken(this, TT_CCAT);
    this->m_toks.push_back(tok);
    break;
  default:
    break;
  }

  return;
}

/*******************************************************/


/* precidence table for token types   */
/* 0 means that precidence does not   */
/* apply to this token - actual chars */
/* for exaple - higher values have    */
/* higher precidence. Must be in same */
/* order as the TokType enums         */

int REToken::tokPrecidence[TT_num] = {
  0, /* TT_SELF_CHAR   */
  2, /* TT_CCAT        */
  1, /* TT_PIPE        */
  3, /* TT_STAR        */

  1, /* TT_DOT         */

  3, /* TT_QMARK       */
  4, /* TT_LPAREN      */
  4, /* TT_RPAREN      */

  4, /* TT_CHAR_CLASS */
  5  /* TT_QUANTIFIER */
};

/*
 Precidence

  low
     pipe
     ccat

     star
  high

  abc*
     --> a ccat b ccat c star
     --> (postfix) a b ccat c star ccat

 ab|cd
     --> a ccat b pipe c ccat d
     --> (postfix) a b ccat c d ccat pipe

  a|b*
    --> a pipe b star
    --> (postfix) a b star pipe

 */

const char *REToken::tokName[TT_num] = {
  "self_char",
  "CCAT",
  "PIPE",
  "STAR",

  "DOT",

  "QMARK",
  "LPAREN",
  "RPAREN",

  "CHAR_CLASS",
  "QUANTIFIER"
  
};

/********************************************************/

void
TokenList2::buildPostfix(TokenList2 *infix, tmpTokList *tmpOpList)
{
  REToken *cur, *cur2, *other_op;

  this->m_toks.clear();
  tmpOpList->clear();

  TokenList2::TokList::const_iterator iter = infix->m_toks.begin();
  while (iter != infix->m_toks.end()) {
    cur = *iter;

    switch (cur->m_ttype) {
    case TT_SELF_CHAR:
      cur2 = new (this->m_mc) REToken(this, cur);
      this->m_toks.push_back(cur2);
      break;

    case TT_LPAREN:
      // no need to create duplicate tokens for LPAREN / RPAREN
      // since they will not be in the final result
      tmpOpList->push_back(cur);
      break;

    case TT_RPAREN:
      {
	bool lparen_found = false;
	while (!tmpOpList->empty()) {
	  other_op = tmpOpList->back();
	  tmpOpList->pop_back();
	  if (other_op->m_ttype == TT_LPAREN) {
	    lparen_found = true;
	    break;
	  }
	  this->m_toks.push_back(other_op);
	}
	if (!lparen_found)
	  throw SyntaxError(0, "Unbalanced parenthesis");
      }
      break;

    case TT_CCAT:
    case TT_PIPE:
      while (! tmpOpList->empty()) {
	other_op = tmpOpList->back();
	if (other_op->m_ttype == TT_LPAREN)
	  break;
	if (REToken::tokPrecidence[cur->m_ttype]
	    > REToken::tokPrecidence[other_op->m_ttype])
	  break;
	tmpOpList->pop_back();
	this->m_toks.push_back(other_op);
      }
      cur2 = new (this->m_mc) REToken(this, cur);
      tmpOpList->push_back(cur2);
      break;

    case TT_STAR:
    case TT_QMARK:
    case TT_DOT:
    case TT_CHAR_CLASS:
    case TT_QUANTIFIER:
    case TT_num:
      break;
    }

    iter++;
  }

  while (! tmpOpList->empty()) {
    other_op = tmpOpList->back();
    tmpOpList->pop_back();
    this->m_toks.push_back(other_op);
  }

  return;
}

/*******************************************************/
bool
TokenList2::equals(TokList::iterator iter, TokType tp, uchar ch)
{
  REToken *ptr;

  if (iter == this->m_toks.end())
    return false;

  ptr = *iter;
  if (ptr->m_ttype != tp)
    return false;
  if (tp == TT_SELF_CHAR) {
    if (ptr->u.m_ch != ch)
      return false;
  }

  return true;
}

bool
TokenList2::verifyCharClassLength(size_t exp)
{
  if (this->m_iter == this->m_toks.end())
    return false;
  REToken *tok = *this->m_iter;
  if (tok->m_ttype != TT_CHAR_CLASS)
    return false;
  size_t act = tok->u.m_charClass->size();
  if (act != exp)
    return false;
  return true;
}

bool
TokenList2::verifyCharClassMember(uchar exp)
{
  if (this->m_iter == this->m_toks.end())
    return false;
  REToken *tok = *this->m_iter;
  if (tok->m_ttype != TT_CHAR_CLASS)
    return false;
  REToken::UCharList::iterator iter = tok->u.m_charClass->begin();
  while (iter != tok->u.m_charClass->end()) {
    if (*iter == exp)
      return true;
    ++iter;
  }
  return false;
}

void
TokenList2::beginIteration()
{
  this->m_iter = this->m_toks.begin();
}

void
TokenList2::incrementIterator()
{
  this->m_iter++;
}

bool
TokenList2::verifyNext(TokType tp, uchar ch)
{
  bool result = this->equals(this->m_iter, tp, ch);
  if (this->m_iter != this->m_toks.end())
    this->m_iter++;
  return result;
}

bool
TokenList2::verifyNextCharClass(const char *exp, size_t n_exp)
{
  if (this->m_iter == this->m_toks.end())
    return false;

  REToken *tok = *this->m_iter;

  if (tok->m_ttype != TT_CHAR_CLASS)
    return false;
  
  if (n_exp > 0 && tok->u.m_charClass == NULL)
    return false;

  bool result = true;

  for (size_t i = 0; i < n_exp; i++) {
    char exp_ch = exp[i];
    bool found = false;
    REToken::UCharList::const_iterator ch_iter = tok->u.m_charClass->begin();
    while (ch_iter != tok->u.m_charClass->end()) {
      uchar act_ch = *ch_iter;
      if ((uchar)exp_ch == act_ch) {
	found = true;
	break;
      }
      ch_iter++;
    }
    if (!found) {
      result = false;
      break;
    }
  }

  this->m_iter++;

  return result;
}

bool
TokenList2::verifyNextQuantifier(bool v1v, size_t v1, bool v2v, size_t v2)
{
  if (this->m_iter == this->m_toks.end())
    return false;

  REToken *tok = *this->m_iter;

  if (tok->m_ttype != TT_QUANTIFIER)
    return false;
  
  if (v1v != tok->u.m_quant.m_v1Valid)
    return false;

  if (v2v != tok->u.m_quant.m_v2Valid)
    return false;
  
  if (v1v == true && tok->u.m_quant.m_v1 != v1)
    return false;

  if (v2v == true && tok->u.m_quant.m_v2 != v2)
    return false;

  return true;
}

bool
TokenList2::verifyEnd()
{
  if (this->m_iter == this->m_toks.end())
    return true;
  return false;
}

void
TokenList2::dumpTokens()
{
  if (this->m_toks.empty()) {
    cout << "Empty token list\n";
    return;
  }
  cout << "Tokens:";
  TokList::iterator iter = this->m_toks.begin();
  while (iter != this->m_toks.end()) {
    REToken *tok = *iter;
    if (tok->m_ttype == TT_SELF_CHAR)
      cout << " '" << tok->u.m_ch << "'";
    else
      cout << " " << REToken::tokName[ tok->m_ttype ];
    iter++;
  }
  cout << "\n";
}
