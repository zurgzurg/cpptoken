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

#include <list>
using namespace std;

#include "cpptoken.h"
#include "cpptoken_private.h"
using namespace cpptoken;

/********************************/
REToken::REToken(TokType tt, uchar c)
{
  this->m_ttype = tt;
  this->u.m_ch = c;
}

/********************************/

TokenList::TokenList(const char *regex)
{
  this->m_toks.clear();
  size_t len = strlen(regex);
  try {
    this->build(regex, 0, len);
  }
  catch (const SyntaxError &e) {
    if (!this->m_toks.empty()) {
      list<REToken *>::iterator iter;
      iter = this->m_toks.begin();
      while (iter != this->m_toks.end()) {
	if ((*iter)->m_ttype == TT_CHAR_CLASS)
	  delete (*iter)->u.m_charClass;
	delete *iter;
	iter++;
      }
    }
    throw;
  }
}

TokenList::TokenList(const char *regex, size_t start, size_t len)
{
  this->m_toks.clear();
  try {
    this->build(regex, start, len);
  }
  catch (const SyntaxError &e) {
    if (!this->m_toks.empty()) {
      list<REToken *>::iterator iter;
      iter = this->m_toks.begin();
      while (iter != this->m_toks.end()) {
	if ((*iter)->m_ttype == TT_CHAR_CLASS)
	  delete (*iter)->u.m_charClass;
	delete *iter;
	iter++;
      }
    }
    throw;
  }
}


TokenList::~TokenList()
{
  if (!this->m_toks.empty()) {
    list<REToken *>::iterator iter;
    iter = this->m_toks.begin();
    while (iter != this->m_toks.end()) {
      if ((*iter)->m_ttype == TT_CHAR_CLASS)
	delete (*iter)->u.m_charClass;
      delete *iter;
      iter++;
    }
  }
  return;
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
    tok = new REToken(TT_QUANTIFIER);
    tok->u.quant.m_v1 = v1;
    tok->u.quant.m_v2 = 0;
    tok->u.quant.m_v1Valid = true;
    tok->u.quant.m_v2Valid = false;
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
    tok = new REToken(TT_QUANTIFIER);
    tok->u.quant.m_v1 = v1;
    tok->u.quant.m_v2 = v2;
    tok->u.quant.m_v1Valid = v1_found;
    tok->u.quant.m_v2Valid = v2_found;
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
  list<uchar> *tmp = new list<uchar>;
  uchar prev, cur;
  const uchar *char_class_start;
  int state;
  bool is_invert = false, close_found = false;

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
	tmp->push_back(cur);
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
	tmp->push_back(prev);
	ptr++;
	break;
      }
      else {
	tmp->push_back(prev);
	ptr++;
	prev = cur;
      }
      continue;
    }

    else if (state == 2) {
      /* maybe got a real range */
      if (cur == ']') {
	close_found = true;
	tmp->push_back(prev);
	tmp->push_back('-');
	ptr++;
	break;
      }
      else {
	this->addToCharClass(tmp, prev, cur);
	ptr++;
	state = 0;
      }
      continue;
    }
  }

  if (!close_found) {
    delete tmp;
    size_t idx = char_class_start - start;
    throw SyntaxError(idx, "Unterminated char class");
  }

  this->addRange(is_invert, tmp);

  return ptr;
}

void
TokenList::addToCharClass(list<uchar>  *c_class, uchar v1, uchar v2)
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
TokenList::addRange(bool invert, list<uchar> *chars)
{
  list<uchar> *clist;

  REToken *tok = new REToken(TT_CHAR_CLASS);

  if (invert) {
    clist = this->computeInverseRange(chars);
    tok->u.m_charClass = clist;
    delete chars;
  }
  else {
    tok->u.m_charClass = chars;
  }

  this->m_toks.push_back(tok);

  return;
}

list<uchar> *
TokenList::computeInverseRange(const list<uchar> *src)
{
  unsigned char buf[256];
  
  for (int i = 0; i < 256; i++)
    buf[i] = 0;

  list<uchar>::const_iterator src_pos = src->begin();
  while (src_pos != src->end()) {
    uchar ch = *src_pos;
    buf[ (unsigned int)ch ] = 1;
    src_pos++;
  }

  list<uchar> *result = new list<uchar>;

  for (int i = 0; i < 256; i++) {
    if (buf[i] == 0)
      result->push_back( (uchar) i );
  }

  return result;
}

void
TokenList::simpleAddToken(TokType tp, uchar ch)
{
  REToken *tok = new REToken(tp, ch);
  this->m_toks.push_back(tok);
  return;
}

void
TokenList::addTokenAndMaybeCcat(TokType tp, uchar ch)
{
  this->maybeAddCcat(tp);
  REToken *tok = new REToken(tp, ch);
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

  list<REToken *>::iterator iter = this->m_toks.end();
  iter--;

  REToken *tok = *iter;

  switch (tok->m_ttype) {
  case TT_SELF_CHAR:
    tok = new REToken(TT_CCAT);
    this->m_toks.push_back(tok);
    break;
  default:
    break;
  }

  return;
}

/*******************************************************/
bool
TokenList::equals(list<REToken *>::iterator iter, TokType tp, uchar ch)
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
  list<uchar>::iterator iter = tok->u.m_charClass->begin();
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
    list<uchar>::const_iterator ch_iter = tok->u.m_charClass->begin();
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
  
  if (v1v != tok->u.quant.m_v1Valid)
    return false;

  if (v2v != tok->u.quant.m_v2Valid)
    return false;
  
  if (v1v == true && tok->u.quant.m_v1 != v1)
    return false;

  if (v2v == true && tok->u.quant.m_v2 != v2)
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
