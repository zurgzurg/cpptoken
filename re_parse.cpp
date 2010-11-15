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
  return;
}
