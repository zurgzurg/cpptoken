#include <cstring>

#include <list>
using namespace std;

#include "cpptoken.h"
#include "cpptoken_private.h"
using namespace cpptoken;

list<REToken *> *
RETokenizer::tokenize(const char *regex)
{
  if (regex == NULL)
    return NULL;
  size_t idx = 0;
  size_t l = strlen(regex);
  list<REToken *> *result = RETokenizer::tokenize(regex, idx, l);
  return result;
}

list<REToken *> *
RETokenizer::tokenize(const char *, size_t start, size_t n)
{
  return NULL;
}

void
RETokenizer::freeTokList(list<REToken *> *toks)
{
  if (toks) {
    list<REToken *>::iterator iter;
    iter = toks->begin();
    while (iter != toks->end()) {
      delete *iter;
      iter++;
    }
  }
  return;
}
