#include <stdarg.h>

#include <cstring>

#include <list>
using namespace std;

#include "cpptoken.h"
#include "cpptoken_private.h"
using namespace cpptoken;

TokenList::TokenList(const char *regex)
{
  this->toks.clear();
  size_t len = strlen(regex);
  this->build(regex, 0, len);
}

TokenList::TokenList(const char *regex, size_t start, size_t len)
{
  this->toks.clear();
  this->build(regex, start, len);
}


TokenList::~TokenList()
{
  if (!this->toks.empty()) {
    list<REToken *>::iterator iter;
    iter = this->toks.begin();
    while (iter != this->toks.end()) {
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
      this->simpleAddToken(STAR);
      break;
    case '|':
      this->simpleAddToken(PIPE);
      break;
    case '(':
      this->addTokenAndMaybeCcat(LPAREN);
      break;
    case ')':
      this->addTokenAndMaybeCcat(RPAREN, ch);
      break;
    case '[':
      {
	list<uchar> tmp;

	ptr++;
	while (ptr <= last_valid && *ptr != ']') {
	  ch = *ptr;
	  tmp.push_back(ch);
	  ptr++;
	}
	this->addRange(false, tmp);
      }
      break;
    default:
      this->addTokenAndMaybeCcat(SELF_CHAR, ch);
      break;
    }

    ptr++;
  }

  return;
}

void
TokenList::addRange(bool invert, const list<uchar> &chars)
{
  list<uchar> inverted;
  const list<uchar> *clist;

  if (invert) {
    this->computeInverseRange(inverted, chars);
    clist = &inverted;
  }
  else {
    clist = &chars;
  }
  
  this->addTokenAndMaybeCcat(LPAREN);

  list<uchar>::const_iterator iter = clist->begin();
  while (iter != clist->end()) {
    this->simpleAddToken(SELF_CHAR, *iter);
    iter++;
  }

  this->simpleAddToken(RPAREN);

  return;
}

void
TokenList::computeInverseRange(list<uchar> &result, const list<uchar> &src)
{
  unsigned char buf[256];
  
  for (int i = 0; i < 256; i++)
    buf[i] = 0;

  list<uchar>::const_iterator src_pos = src.begin();
  while (src_pos != src.end()) {
    uchar ch = *src_pos;
    buf[ (unsigned int)ch ] = 1;
    src_pos++;
  }

  result.clear();
  for (int i = 0; i < 256; i++) {
    if (buf[i] == 1)
      result.push_back( (uchar) i );
  }

  return;
}

void
TokenList::simpleAddToken(TokType tp, uchar ch)
{
  REToken *tok = new REToken(tp, ch);
  this->toks.push_back(tok);
  return;
}

void
TokenList::addTokenAndMaybeCcat(TokType tp, uchar ch)
{
  this->maybeAddCcat(tp);
  REToken *tok = new REToken(tp, ch);
  this->toks.push_back(tok);
  return;
}

void
TokenList::maybeAddCcat(TokType cur_tp)
{
  if (this->toks.empty())
    return;

  if (cur_tp == LPAREN || cur_tp == RPAREN)
    return;

  list<REToken *>::iterator iter = this->toks.end();
  iter--;

  REToken *tok = *iter;

  switch (tok->ttype) {
  case SELF_CHAR:
    tok = new REToken(CCAT);
    this->toks.push_back(tok);
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

  if (iter == this->toks.end())
    return false;

  ptr = *iter;
  if (ptr->ttype != tp)
    return false;
  if (tp == SELF_CHAR) {
    if (ptr->ch != ch)
      return false;
  }

  return true;
}

void
TokenList::beginIteration()
{
  this->iter = this->toks.begin();
}

bool
TokenList::verifyNext(TokType tp, uchar ch)
{
  bool result = this->equals(this->iter, tp, ch);
  if (this->iter != this->toks.end())
    this->iter++;
  return result;
}

bool
TokenList::verifyEnd()
{
  if (this->iter == this->toks.end())
    return true;
  return false;
}
