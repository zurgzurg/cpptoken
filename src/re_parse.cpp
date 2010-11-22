#include <cstring>

#include <list>
using namespace std;

#include "cpptoken.h"
#include "cpptoken_private.h"
using namespace cpptoken;

TokenList::TokenList(const char *regex)
{
  this->m_toks.clear();
  size_t len = strlen(regex);
  this->build(regex, 0, len);
}

TokenList::TokenList(const char *regex, size_t start, size_t len)
{
  this->m_toks.clear();
  this->build(regex, start, len);
}


TokenList::~TokenList()
{
  if (!this->m_toks.empty()) {
    list<REToken *>::iterator iter;
    iter = this->m_toks.begin();
    while (iter != this->m_toks.end()) {
      if ((*iter)->m_ttype == CHAR_CLASS)
	delete (*iter)->m_charClass;
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
	list<uchar> *tmp = new list<uchar>;
	uchar prev, cur;

	ptr++;

	prev = *ptr++;
	while (ptr <= last_valid) {
	  cur = *ptr;

	  if (cur == ']') {
	    tmp->push_back(prev);
	    ptr++;
	    break;
	  }

	  if (cur == '-') {
	    ptr++;
	    cur = *ptr++;
	    this->addToCharClass(tmp, prev, cur);
	    continue;
	  }

	  tmp->push_back(prev);
	  prev = cur;
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

  REToken *tok = new REToken(CHAR_CLASS);

  if (invert) {
    clist = this->computeInverseRange(chars);
    tok->m_charClass = clist;
    delete chars;
  }
  else {
    tok->m_charClass = chars;
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
    if (buf[i] == 1)
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

  if (cur_tp == LPAREN || cur_tp == RPAREN)
    return;

  list<REToken *>::iterator iter = this->m_toks.end();
  iter--;

  REToken *tok = *iter;

  switch (tok->m_ttype) {
  case SELF_CHAR:
    tok = new REToken(CCAT);
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
  if (tp == SELF_CHAR) {
    if (ptr->m_ch != ch)
      return false;
  }

  return true;
}

void
TokenList::beginIteration()
{
  this->m_iter = this->m_toks.begin();
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

  if (tok->m_ttype != CHAR_CLASS)
    return false;
  
  if (n_exp > 0 && tok->m_charClass == NULL)
    return false;

  bool result = true;

  for (size_t i = 0; i < n_exp; i++) {
    char exp_ch = exp[i];
    bool found = false;
    list<uchar>::const_iterator ch_iter = tok->m_charClass->begin();
    while (ch_iter != tok->m_charClass->end()) {
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
TokenList::verifyEnd()
{
  if (this->m_iter == this->m_toks.end())
    return true;
  return false;
}
