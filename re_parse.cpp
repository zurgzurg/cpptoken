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
  const char *ptr, *last_valid;
  char ch;
  
  ptr = regex + start;
  last_valid = regex + start + len - 1;
  while (ptr <= last_valid) {
    ch = *ptr;
    
    switch (ch) {
    case '*':
      this->simpleAddToken(STAR);
      break;
    case '|':
      this->simpleAddToken(PIPE);
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
TokenList::simpleAddToken(TokType tp, char ch)
{
  REToken *tok = new REToken(tp, ch);
  this->toks.push_back(tok);
  return;
}

void
TokenList::addTokenAndMaybeCcat(TokType tp, char ch)
{
  this->maybeAddCcat();
  REToken *tok = new REToken(tp, ch);
  this->toks.push_back(tok);
  return;
}

void
TokenList::maybeAddCcat()
{
  if (this->toks.empty())
    return;

  TokIter iter = this->toks.end();
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
TokenList::equals(TokIter iter, TokType tp, char ch)
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
