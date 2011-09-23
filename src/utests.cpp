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

#include <cstdarg>
#include <cstring>

#include <list>
#include <vector>
#include <limits>
#include <sstream>
#include <exception>
#include <iostream>
#include <new>

using namespace std;

#include "cpptoken.h"
#include "cpptoken_private.h"
using namespace cpptoken;

/****************************************************/
class TestFailure : public exception {
  string msg;

  // attempt to prevent two exceptions from being thrown at the 'same' time
  // TestFailure should only be created if it is going to be thrown, so bump
  // the counter on creation and decrment when it is handled
  static int numActive;

public:
  TestFailure(const char *file, int line, size_t numAsserts);
  ~TestFailure() throw() {;};

  const char *what() const throw();

  static void setUnwindDone() throw();
  static int getNumActive() throw();
};

int TestFailure::numActive = 0;

TestFailure::TestFailure(const char *file, int line, size_t assertNum)
{
  stringstream tmp;
  tmp << "TestFailure excecption: ";
  tmp << file;
  tmp << ":";
  tmp << line;
  if (assertNum > 0) {
    tmp << " assert number ";
    tmp << assertNum;
  }
  tmp << "\n";
  this->msg = tmp.str();
  TestFailure::numActive++;
}

const char *
TestFailure::what() const throw()
{
  const char *r;

  r = this->msg.c_str();

  return r;
}

// static member func
void
TestFailure::setUnwindDone() throw()
{
  TestFailure::numActive--;
}

// static member func
int
TestFailure::getNumActive() throw()
{
  return TestFailure::numActive;
}

/****************************************************/
class TestOption {
public:
  bool verbose;

  TestOption() : verbose(false) {;};
};

/****************************************************/
class TestResult {
public:
  size_t n_run;
  size_t n_pass;
  size_t n_fail;
  size_t n_exceptions;
  size_t n_asserts;

  list<string> tests_e;
  list<string> tests_f;

  TestResult() : n_run(0),
		 n_pass(0),
		 n_fail(0),
		 n_exceptions(0),
		 n_asserts(0) {;};

  void report();
};

void
TestResult::report()
{
  cout << "num_run=" << this->n_run
       << " num_pass=" << this->n_pass
       << " num_fail=" << this->n_fail
       << " num_exceptions=" << this->n_exceptions
       << " num_asserts=" << this->n_asserts
       << "\n";
  if (!this->tests_e.empty()) {
    list<string>::iterator iter = this->tests_e.begin();
    cout << "Tests failing with exceptions\n";
    while (iter != this->tests_e.end()) {
      cout << "    " << *iter << "\n";
      iter++;
    }
  }

  if (!this->tests_f.empty()) {
    list<string>::iterator iter = this->tests_f.begin();
    cout << "Failing tests\n";
    while (iter != this->tests_f.end()) {
      cout << "    " << *iter << "\n";
      iter++;
    }
  }

  return;
}

/****************************************************/
struct TestCase {
  TestResult *result;
  string name;
  bool isPass;
  bool statusSet;
  size_t m_refCount;

  virtual void setup();
  virtual void run();
  virtual void teardown();

  TestCase(const char *nm)
    : result(NULL),
      name(nm),
      isPass(false),
      statusSet(false),
      m_refCount(1)
  {;};

  void assertTrue(bool, const char *file, int line);
  void setStatus(bool);
  TestCase *makeReference();
  void releaseReference();
};

#define ASSERT_TRUE(c) this->assertTrue(c, __FILE__, __LINE__)

void
TestCase::assertTrue(bool c, const char *fname, int line)
{
  if (this->result)
    this->result->n_asserts++;
  if (c == true)
    return;
  if (TestFailure::getNumActive() > 0)
    return;

  size_t n_asserts = 0;
  if (this->result)
    n_asserts = this->result->n_asserts;
  throw TestFailure(fname, line, n_asserts);
}

void
TestCase::setStatus(bool s)
{
  this->statusSet = true;
  this->isPass = s;
  if (this->result) {
    if (s == true)
      this->result->n_pass++;
    else
      this->result->n_fail++;
  }
  return;
}

TestCase *
TestCase::makeReference()
{
  this->m_refCount++;
#if 0
  cout << "make ref for "
       << this->name
       << "count="
       << this->m_refCount
       << "\n";
#endif
  return this;
}

void
TestCase::releaseReference()
{
  this->m_refCount--;
#if 0
  cout << "release ref for "
       << this->name
       << "count="
       << this->m_refCount
       << "\n";
#endif
  if (this->m_refCount == 0)
    delete this;
}

void
TestCase::setup()
{
  ;
}

void
TestCase::run()
{
  ;
}

void
TestCase::teardown()
{
  ;
}

/****************************************************/
struct TestSuite {
  list<TestCase *> tests;

  ~TestSuite();

  void run(TestResult *, TestOption *);
  void addTestCase(TestCase *);
};

TestSuite::~TestSuite()
{
  list<TestCase *>::iterator iter;
  iter = this->tests.begin();
  while (iter != this->tests.end()) {
    TestCase *tc = *iter;
    tc->releaseReference();
    iter++;
  }
}

/************/

void
TestSuite::run(TestResult *result, TestOption *opt)
{
  list<TestCase *>::iterator iter;
  
  iter = this->tests.begin();
  while (iter != this->tests.end()) {
    TestCase *tc = *iter;

    if (opt->verbose)
      cout << "Starting test " << tc->name << "\n";

    tc->result = result;
    try {
      tc->setup();

      result->n_run++;
      tc->statusSet = false;
      tc->run();

      if (tc->statusSet == false) {
	result->n_fail++;
	result->tests_f.push_back( tc->name );
      }

    }
    catch (TestFailure e) {
      TestFailure::setUnwindDone();
      if (opt->verbose)
	cout << "    caught test fail exception\n";
      result->tests_e.push_back( tc->name + e.what() );
      result->n_exceptions++;
    }
    if (opt->verbose)
      cout << "    done\n";

    iter++;
  }

  return;
}

void
TestSuite::addTestCase(TestCase *tc)
{
  this->tests.push_back(tc);
  return;
}

/****************************************************/
/****************************************************/
/* basic tests                                      */
/****************************************************/
/****************************************************/
struct TC_Basic01 : public TestCase {
  TC_Basic01() : TestCase("TC_Basic01") {;};
  void run();
};

void
TC_Basic01::run()
{
  ASSERT_TRUE(true);
  this->setStatus(true);
}

/********************/

struct TC_Basic02 : public TestCase {
  TC_Basic02() : TestCase("TC_Basic02") {;};
  void run();
};

void
TC_Basic02::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  
  alloc.setMC(&mc);

  TokenList *toks = new (&mc) TokenList(&mc, alloc, "");
  toks->~TokenList();
  mc.deallocate(toks, 1);

  this->setStatus(true);
}

/****************************************************/
/****************************************************/
/* tokenization tests                               */
/****************************************************/
/****************************************************/
struct TC_Tokens01 : public TestCase {
  TC_Tokens01() : TestCase("TC_Tokens01") {;};
  void run();
};

void
TC_Tokens01::run()
{
  this->setStatus(true);
}

/********************/

struct TC_Tokens02 : public TestCase {
  TC_Tokens02() : TestCase("TC_Tokens02") {;};
  void run();
};

void
TC_Tokens02::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  TokenList tlist(&mc, alloc, "a");
  TokenList::TokList::iterator iter = tlist.m_toks.begin();

  ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'a'));
  this->setStatus(true);
}

/********************/

struct TC_Tokens03 : public TestCase {
  TC_Tokens03() : TestCase("TC_Tokens03") {;};
  void run();
};

void
TC_Tokens03::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  TokenList tlist(&mc, alloc, "a*");
  TokenList::TokList::iterator iter = tlist.m_toks.begin();

  ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'a'));
  iter++;
  ASSERT_TRUE(tlist.equals(iter, TT_STAR));
  this->setStatus(true);
}

/********************/

struct TC_Tokens04 : public TestCase {
  TC_Tokens04() : TestCase("TC_Tokens04") {;};
  void run();
};

void
TC_Tokens04::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  {
    TokenList tlist(&mc, alloc, "a|b");
    TokenList::TokList::iterator iter = tlist.m_toks.begin();
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'a'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_PIPE));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'b'));
    iter++;
    ASSERT_TRUE(iter == tlist.m_toks.end());
  }

  {
    TokenList tlist(&mc, alloc, "ab");
    TokenList::TokList::iterator iter = tlist.m_toks.begin();
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'a'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_CCAT));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'b'));
    iter++;
    ASSERT_TRUE(iter == tlist.m_toks.end());
  }
  
  {
    TokenList tlist(&mc, alloc, "abc");
    TokenList::TokList::iterator iter = tlist.m_toks.begin();
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'a'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_CCAT));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'b'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_CCAT, 'b'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'c'));
    iter++;
    ASSERT_TRUE(iter == tlist.m_toks.end());
  }

  {
    TokenList tlist(&mc, alloc, "(a)");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_LPAREN));
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNext(TT_RPAREN));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "[abc]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNextCharClass("abc", 3));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "[a-c]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNextCharClass("abc", 3));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "[-c]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNextCharClass("-c", 2));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "[a-cx-]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNextCharClass("abcx-", 5));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  this->setStatus(true);
}

/********************/

struct TC_Tokens05 : public TestCase {
  TC_Tokens05() : TestCase("TC_Tokens05") {;};
  void run();
};

void
TC_Tokens05::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  {
    TokenList tlist(&mc, alloc, "[^abc]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyCharClassLength(256 - 3));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "[^abc]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyCharClassMember('z'));
    ASSERT_TRUE( ! tlist.verifyCharClassMember('a'));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  this->setStatus(true);
}

/********************/

struct TC_Tokens06 : public TestCase {
  TC_Tokens06() : TestCase("TC_Tokens06") {;};
  void run();
};

void
TC_Tokens06::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  {
    TokenList tlist(&mc, alloc, "a{1,2}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 1, true, 2));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "a{10,20}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 10, true, 20));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "a{ 10,20}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 10, true, 20));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "a{ 10 , 20}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 10, true, 20));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "a{ 2 , 3 }");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 2, true, 3));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "a{2,}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 2, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "a{ 23 ,}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 23, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "a{ 23, }");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 23, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "a{2}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 2, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "a{ 2 }");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 2, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "a{ 22 }");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 22, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(&mc, alloc, "a{,3}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(false, 0, true, 3));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  this->setStatus(true);
}


/********************/

struct TC_Tokens07 : public TestCase {
  TC_Tokens07() : TestCase("TC_Tokens07") {;};
  void run();
};

void
TC_Tokens07::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  try {
    TokenList tlist(&mc, alloc, "{2 2}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(e.getErrorIndex() == 3);
  }

  try {
    TokenList tlist(&mc, alloc, "{2  2}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(e.getErrorIndex() == 4);
  }

  try {
    TokenList tlist(&mc, alloc, "{2-}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(e.getErrorIndex() == 2);
  }

  try {
    TokenList tlist(&mc, alloc, "{9999999999999999999999999999999999999999999}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(true);
  }

  try {
    TokenList tlist(&mc, alloc, "{9, 99999999999999999999999999999999999999999999}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(true);
  }

  this->setStatus(true);
}

/********************/

struct TC_Tokens08 : public TestCase {
  TC_Tokens08() : TestCase("TC_Tokens08") {;};
  void run();
};

void
TC_Tokens08::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  try {
    TokenList tlist(&mc, alloc, "a[b");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(e.getErrorIndex() == 1);
  }

  this->setStatus(true);
}

/********************/

struct TC_MemFail01 : public TestCase {
  TC_MemFail01() : TestCase("TC_MemFail01") {;};
  void run();
};

void
TC_MemFail01::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  try {
    TokenList tlist(&mc, alloc, "a[b");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(e.getErrorIndex() == 1);
  }

  this->setStatus(true);
}

/********************/

class MemoryControlWithFailure : public MemoryControl {
public:
  size_t  m_numAllocs;
  size_t  m_numDeallocs;
  bool    m_allowMismatch;

private:
  bool    m_useLimit;
  size_t  m_limit;

public:
  MemoryControlWithFailure();
  ~MemoryControlWithFailure();

  virtual void *allocate(size_t);
  virtual void deallocate(void *, size_t);

  void resetCounters();
  void disableLimit();
  void setLimit(size_t);
};

MemoryControlWithFailure::MemoryControlWithFailure()
{
  this->m_numAllocs = 0;
  this->m_numDeallocs = 0;
  this->m_allowMismatch = false;
  this->m_useLimit = false;
  this->m_limit = 0;
}

MemoryControlWithFailure::~MemoryControlWithFailure()
{
  if (this->m_allowMismatch == true)
    return;
  if (this->m_numAllocs != this->m_numDeallocs
      && TestFailure::getNumActive() == 0)
    throw TestFailure(__FILE__, __LINE__, 0);
}

void *
MemoryControlWithFailure::allocate(size_t sz)
{
  if (this->m_useLimit
      && this->m_numAllocs >= this->m_limit
      && TestFailure::getNumActive() == 0)
    throw bad_alloc();
  void *ptr = ::operator new(sz);
  this->m_numAllocs++;
  return ptr;
}

void
MemoryControlWithFailure::deallocate(void *ptr, size_t sz)
{
  ::operator delete(ptr);
  this->m_numDeallocs++;
}

void
MemoryControlWithFailure::resetCounters()
{
  this->m_numAllocs = 0;
  this->m_numDeallocs = 0;
}

void
MemoryControlWithFailure::disableLimit()
{
  this->m_useLimit = false;
  this->m_limit = 0;
}

void
MemoryControlWithFailure::setLimit(size_t l)
{
  this->m_useLimit = true;
  this->m_limit = l;
}

/********************/

struct TC_MemFail02 : public TestCase {
  TC_MemFail02() : TestCase("TC_MemFail02") {;};
  void run();
};

void
TC_MemFail02::run()
{
  MemoryControlWithFailure mc;
  mc.resetCounters();
  mc.disableLimit();
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  {
    TokenList tlist(&mc, alloc, "a");
  }

  size_t numAllocs = mc.m_numAllocs;
  for (size_t lim = 0; lim < numAllocs; lim++) {

    mc.resetCounters();
    mc.setLimit(lim);

    try {
      TokenList tlist(&mc, alloc, "a");
      ASSERT_TRUE(false);
    }
    catch (const bad_alloc &e) {
      ASSERT_TRUE(true);
    }
  }

  this->setStatus(true);
}

/********************/

struct TC_MemFail03 : public TestCase {
  TC_MemFail03() : TestCase("TC_MemFail03") {;};
  void run();
};

void
TC_MemFail03::run()
{
  MemoryControlWithFailure mc;
  mc.resetCounters();
  mc.disableLimit();
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  {
    TokenList tlist(&mc, alloc, "[a-z]");
  }

  size_t numAllocs = mc.m_numAllocs;
  for (size_t lim = 0; lim < numAllocs; lim++) {

    mc.resetCounters();
    mc.setLimit(lim);

    try {
      TokenList tlist(&mc, alloc, "[a-z]");
      ASSERT_TRUE(false);
    }
    catch (const bad_alloc &e) {
      ASSERT_TRUE(true);
    }
  }

  this->setStatus(true);
}

/********************/

struct TC_MemFail04 : public TestCase {
  TC_MemFail04() : TestCase("TC_MemFail04") {;};
  void checkOneRegex(MemoryControlWithFailure &,
		     Alloc<REToken *>& alloc,
		     const char *re);
  void run();
};

void
TC_MemFail04::checkOneRegex(MemoryControlWithFailure &mc,
			    Alloc<REToken *>& alloc,
			    const char *regex)
{
  {
    mc.resetCounters();
    mc.disableLimit();
    TokenList tlist(&mc, alloc, regex);
  }
  
  size_t numAllocs = mc.m_numAllocs;
  for (size_t lim = 0; lim < numAllocs; lim++) {

    mc.resetCounters();
    mc.setLimit(lim);

    try {
      TokenList tlist(&mc, alloc, regex);
      ASSERT_TRUE(false);
    }
    catch (const bad_alloc &e) {
      ASSERT_TRUE(true);
    }
  }
}

void
TC_MemFail04::run()
{
  MemoryControlWithFailure mc;
  mc.resetCounters();
  mc.disableLimit();
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  this->checkOneRegex(mc, alloc, "abc");
  this->checkOneRegex(mc, alloc, "a{2,10}");
  this->checkOneRegex(mc, alloc, "a");
  this->checkOneRegex(mc, alloc, "a{2}");

  this->setStatus(true);
}

/********************/

struct TC_MemFail05 : public TestCase {
  TC_MemFail05() : TestCase("TC_MemFail05") {;};
  void expectFailure(MemoryControlWithFailure &,
		     Alloc<REToken *>& alloc,
		     const char *re);
  void run();
};

void
TC_MemFail05::expectFailure(MemoryControlWithFailure &mc,
			    Alloc<REToken *>& alloc,
			    const char *regex)
{
  {
    mc.resetCounters();
    mc.disableLimit();
    try {
      TokenList tlist(&mc, alloc, regex);
      ASSERT_TRUE(false);
    }
    catch (const SyntaxError &e) {
      ASSERT_TRUE(true);
    }
  }
  
  size_t numAllocs = mc.m_numAllocs;
  for (size_t lim = 0; lim < numAllocs; lim++) {

    mc.resetCounters();
    mc.setLimit(lim);

    try {
      TokenList tlist(&mc, alloc, regex);
      ASSERT_TRUE(false);
    }
    catch (const bad_alloc &e) {
      ASSERT_TRUE(true);
    }
    catch (const SyntaxError &e) {
      ASSERT_TRUE(true);
    }
  }
}

void
TC_MemFail05::run()
{
  MemoryControlWithFailure mc;
  mc.resetCounters();
  mc.disableLimit();
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  this->expectFailure(mc, alloc, "{2 2}");
  this->expectFailure(mc, alloc, "{2 - }");
  this->expectFailure(mc, alloc, "{999999999999999999999999999999999");
  this->expectFailure(mc, alloc, "a{2,99999999999999999999999999999999999}");

  this->setStatus(true);
}

/****************************************************/
/****************************************************/
/* tokenization2 tests                              */
/****************************************************/
/****************************************************/
struct TC_Tokens201 : public TestCase {
  TC_Tokens201() : TestCase("TC_Tokens201") {;};
  void run();
};

void
TC_Tokens201::run()
{
  this->setStatus(true);
}

/********************/

struct TC_Tokens202 : public TestCase {
  TC_Tokens202() : TestCase("TC_Tokens202") {;};
  void run();
};

void
TC_Tokens202::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  TokenList2 tlist(&mc, alloc);
  tlist.build("a", 0, 1);
  TokenList2::TokList::iterator iter = tlist.m_toks.begin();

  ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'a'));
  this->setStatus(true);
}

/********************/

struct TC_Tokens203 : public TestCase {
  TC_Tokens203() : TestCase("TC_Tokens203") {;};
  void run();
};

void
TC_Tokens203::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  TokenList2 tlist(&mc, alloc);
  tlist.build("a*");
  TokenList2::TokList::iterator iter = tlist.m_toks.begin();

  ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'a'));
  iter++;
  ASSERT_TRUE(tlist.equals(iter, TT_STAR));
  this->setStatus(true);
}

/********************/

struct TC_Tokens204 : public TestCase {
  TC_Tokens204() : TestCase("TC_Tokens204") {;};
  void run();
};

void
TC_Tokens204::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a|b");
    TokenList2::TokList::iterator iter = tlist.m_toks.begin();
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'a'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_PIPE));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'b'));
    iter++;
    ASSERT_TRUE(iter == tlist.m_toks.end());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("ab");
    TokenList2::TokList::iterator iter = tlist.m_toks.begin();
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'a'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_CCAT));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'b'));
    iter++;
    ASSERT_TRUE(iter == tlist.m_toks.end());
  }
  
  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a(b)c");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNext(TT_CCAT));
    ASSERT_TRUE(tlist.verifyNext(TT_LPAREN));
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'b'));
    ASSERT_TRUE(tlist.verifyNext(TT_RPAREN));
    ASSERT_TRUE(tlist.verifyNext(TT_CCAT));
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'c'));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("\\a");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("abc");
    TokenList2::TokList::iterator iter = tlist.m_toks.begin();
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'a'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_CCAT));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'b'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_CCAT, 'b'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, TT_SELF_CHAR, 'c'));
    iter++;
    ASSERT_TRUE(iter == tlist.m_toks.end());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("(a)");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_LPAREN));
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNext(TT_RPAREN));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("[abc]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNextCharClass("abc", 3));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("[a-c]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNextCharClass("abc", 3));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("[-c]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNextCharClass("-c", 2));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("[a-cx-]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNextCharClass("abcx-", 5));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  this->setStatus(true);
}

/********************/

struct TC_Tokens205 : public TestCase {
  TC_Tokens205() : TestCase("TC_Tokens205") {;};
  void run();
};

void
TC_Tokens205::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("[^abc]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyCharClassLength(256 - 3));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("[^abc]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyCharClassMember('z'));
    ASSERT_TRUE( ! tlist.verifyCharClassMember('a'));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  this->setStatus(true);
}

/********************/

struct TC_Tokens206 : public TestCase {
  TC_Tokens206() : TestCase("TC_Tokens206") {;};
  void run();
};

void
TC_Tokens206::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a{1,2}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 1, true, 2));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a{10,20}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 10, true, 20));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a{ 10,20}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 10, true, 20));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a{ 10 , 20}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 10, true, 20));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a{ 2 , 3 }");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 2, true, 3));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a{2,}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 2, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a{ 23 ,}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 23, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a{ 23, }");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 23, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a{2}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 2, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a{ 2 }");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 2, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a{ 22 }");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 22, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a{,3}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(false, 0, true, 3));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  this->setStatus(true);
}


/********************/

struct TC_Tokens207 : public TestCase {
  TC_Tokens207() : TestCase("TC_Tokens207") {;};
  void run();
};

void
TC_Tokens207::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  try {
    TokenList2 tlist(&mc, alloc);
    tlist.build("{2 2}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(e.getErrorIndex() == 3);
  }

  try {
    TokenList2 tlist(&mc, alloc);
    tlist.build("{2  2}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(e.getErrorIndex() == 4);
  }

  try {
    TokenList2 tlist(&mc, alloc);
    tlist.build("{2-}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(e.getErrorIndex() == 2);
  }

  try {
    TokenList2 tlist(&mc, alloc);
    tlist.build("{9999999999999999999999999999999999999999999}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(true);
  }

  try {
    TokenList2 tlist(&mc, alloc);
    tlist.build("{9, 99999999999999999999999999999999999999999999}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(true);
  }

  this->setStatus(true);
}

/********************/

struct TC_Tokens208 : public TestCase {
  TC_Tokens208() : TestCase("TC_Tokens208") {;};
  void run();
};

void
TC_Tokens208::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  try {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a[b");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(e.getErrorIndex() == 1);
  }

  this->setStatus(true);
}

/********************/

struct TC_Tokens209 : public TestCase {
  TC_Tokens209() : TestCase("TC_Tokens209") {;};
  void run();
};

void
TC_Tokens209::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  for (char c1 = ' '; c1 <= '~'; c1++) {
    TokenList2 tlist(&mc, alloc);
    char buf[3];

    switch (c1) {
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case '\\':
    case '*':
    case '?':
    case '+':
    case '|':
      buf[0] = '\\';
      buf[1] = c1;
      buf[2] = '\0';
      break;
    default:
      buf[0] = c1;
      buf[1] = '\0';
      buf[2] = '\0';
      break;
    }

    tlist.build(buf);
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, c1));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  this->setStatus(true);
}

/********************/

struct TC_Tokens210 : public TestCase {
  TC_Tokens210() : TestCase("TC_Tokens210") {;};
  void run();
};

void
TC_Tokens210::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  for (char c1 = ' '; c1 <= '~'; c1++) {
    char buf[5];

    int idx = 0;
    switch (c1) {
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case '\\':
    case '*':
    case '?':
    case '+':
    case '|':
      buf[idx++] = '\\';
      buf[idx++] = c1;
      break;
    default:
      buf[idx++] = c1;
      break;
    }

    for (char c2 = ' '; c2 <= '~'; c2++) {
      TokenList2 tlist(&mc, alloc);

      int idx2 = idx;

      switch (c2) {
      case '(':
      case ')':
      case '[':
      case ']':
      case '{':
      case '}':
      case '\\':
      case '*':
      case '?':
      case '+':
      case '|':
	buf[idx2++] = '\\';
	buf[idx2++] = c2;
	break;
      default:
	buf[idx2++] = c2;
	break;
      }

      while (idx2 <= 4)
	buf[idx2++] = '\0';

      tlist.build(buf);

      tlist.beginIteration();
      ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, c1));
      ASSERT_TRUE(tlist.verifyNext(TT_CCAT));
      ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, c2));
      ASSERT_TRUE(tlist.verifyEnd());
    }
  }

  this->setStatus(true);
}

/********************/

struct TC_MemFail2_02 : public TestCase {
  TC_MemFail2_02() : TestCase("TC_MemFail2_02") {;};
  void run();
};

void
TC_MemFail2_02::run()
{
  MemoryControlWithFailure mc;
  mc.resetCounters();
  mc.disableLimit();
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("a");
  }

  size_t numAllocs = mc.m_numAllocs;
  for (size_t lim = 0; lim < numAllocs; lim++) {

    mc.resetCounters();
    mc.setLimit(lim);

    try {
      TokenList2 tlist2(&mc, alloc);
      tlist2.build("a");
      ASSERT_TRUE(false);
    }
    catch (const bad_alloc &e) {
      ASSERT_TRUE(true);
    }
  }

  this->setStatus(true);
}

/********************/

struct TC_MemFail2_03 : public TestCase {
  TC_MemFail2_03() : TestCase("TC_MemFail2_03") {;};
  void run();
};

void
TC_MemFail2_03::run()
{
  MemoryControlWithFailure mc;
  mc.resetCounters();
  mc.disableLimit();
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  {
    TokenList2 tlist(&mc, alloc);
    tlist.build("[a-z]");
  }

  size_t numAllocs = mc.m_numAllocs;
  for (size_t lim = 0; lim < numAllocs; lim++) {

    mc.resetCounters();
    mc.setLimit(lim);

    try {
      TokenList2 tlist(&mc, alloc);
      tlist.build("[a-z]");
      ASSERT_TRUE(false);
    }
    catch (const bad_alloc &e) {
      ASSERT_TRUE(true);
    }
  }
  this->setStatus(true);
}

/********************/

struct TC_MemFail2_04 : public TestCase {
  TC_MemFail2_04() : TestCase("TC_MemFail2_04") {;};
  void checkOneRegex(MemoryControlWithFailure &,
		     Alloc<REToken *>& alloc,
		     const char *re);
  void run();
};

void
TC_MemFail2_04::checkOneRegex(MemoryControlWithFailure &mc,
			    Alloc<REToken *>& alloc,
			    const char *regex)
{
  {
    mc.resetCounters();
    mc.disableLimit();
    TokenList2 tlist(&mc, alloc);
    tlist.build(regex);
  }
  
  size_t numAllocs = mc.m_numAllocs;
  for (size_t lim = 0; lim < numAllocs; lim++) {

    mc.resetCounters();
    mc.setLimit(lim);

    try {
      TokenList2 tlist(&mc, alloc);
      tlist.build(regex);
      ASSERT_TRUE(false);
    }
    catch (const bad_alloc &e) {
      ASSERT_TRUE(true);
    }
  }
}

void
TC_MemFail2_04::run()
{
  MemoryControlWithFailure mc;
  mc.resetCounters();
  mc.disableLimit();
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  this->checkOneRegex(mc, alloc, "abc");
  this->checkOneRegex(mc, alloc, "a{2,10}");
  this->checkOneRegex(mc, alloc, "a");
  this->checkOneRegex(mc, alloc, "a{2}");

  this->setStatus(true);
}

/********************/

struct TC_MemFail2_05 : public TestCase {
  TC_MemFail2_05() : TestCase("TC_MemFail2_05") {;};
  void expectFailure(MemoryControlWithFailure &,
		     Alloc<REToken *>& alloc,
		     const char *re);
  void run();
};

void
TC_MemFail2_05::expectFailure(MemoryControlWithFailure &mc,
			    Alloc<REToken *>& alloc,
			    const char *regex)
{
  {
    mc.resetCounters();
    mc.disableLimit();
    try {
      TokenList2 tlist(&mc, alloc);
      tlist.build(regex);
      ASSERT_TRUE(false);
    }
    catch (const SyntaxError &e) {
      ASSERT_TRUE(true);
    }
  }
  
  size_t numAllocs = mc.m_numAllocs;
  for (size_t lim = 0; lim < numAllocs; lim++) {

    mc.resetCounters();
    mc.setLimit(lim);

    try {
      TokenList2 tlist(&mc, alloc);
      tlist.build(regex);
      ASSERT_TRUE(false);
    }
    catch (const bad_alloc &e) {
      ASSERT_TRUE(true);
    }
    catch (const SyntaxError &e) {
      ASSERT_TRUE(true);
    }
  }
}

void
TC_MemFail2_05::run()
{
  MemoryControlWithFailure mc;
  mc.resetCounters();
  mc.disableLimit();
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  this->expectFailure(mc, alloc, "{2 2}");
  this->expectFailure(mc, alloc, "{2 - }");
  this->expectFailure(mc, alloc, "{999999999999999999999999999999999");
  this->expectFailure(mc, alloc, "a{2,99999999999999999999999999999999999}");

  this->setStatus(true);
}

/****************************************************/
/****************************************************/
/* postfix                                          */
/****************************************************/
/****************************************************/
struct TC_Postfix01 : public TestCase {
  TC_Postfix01() : TestCase("TC_Postfix01") {;};
  void run();
};


void
TC_Postfix01::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  TokenList2 tlist(&mc, alloc);
  tlist.build("a", 0, 1);
  TokenList2 tlist2(&mc, alloc);
  TokenList2::tmpTokList tmpList;
  tlist2.buildPostfix(&tlist, &tmpList);

  tlist2.beginIteration();
  ASSERT_TRUE(tlist2.verifyNext(TT_SELF_CHAR, 'a'));
  ASSERT_TRUE(tlist2.verifyEnd());

  this->setStatus(true);
}

/********************/
struct TC_Postfix02 : public TestCase {
  TC_Postfix02() : TestCase("TC_Postfix02") {;};
  void run();
};


void
TC_Postfix02::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  TokenList2 tlist(&mc, alloc);
  tlist.build("ab");

  TokenList2 tlist2(&mc, alloc);
  TokenList2::tmpTokList tmpList;
  tlist2.buildPostfix(&tlist, &tmpList);

  tlist2.beginIteration();
  ASSERT_TRUE(tlist2.verifyNext(TT_SELF_CHAR, 'a'));
  ASSERT_TRUE(tlist2.verifyNext(TT_SELF_CHAR, 'b'));
  ASSERT_TRUE(tlist2.verifyNext(TT_CCAT));
  ASSERT_TRUE(tlist2.verifyEnd());

  this->setStatus(true);
}

/********************/

struct TC_Postfix03 : public TestCase {
  TC_Postfix03() : TestCase("TC_Postfix03") {;};
  void run();
};


void
TC_Postfix03::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  TokenList2 tlist(&mc, alloc);
  tlist.build("ab", 0, 2);
  TokenList2 tlist2(&mc, alloc);
  TokenList2::tmpTokList tmpList;
  tlist2.buildPostfix(&tlist, &tmpList);

  tlist2.beginIteration();
  ASSERT_TRUE(tlist2.verifyNext(TT_SELF_CHAR, 'a'));
  ASSERT_TRUE(tlist2.verifyNext(TT_SELF_CHAR, 'b'));
  ASSERT_TRUE(tlist2.verifyNext(TT_CCAT));
  ASSERT_TRUE(tlist2.verifyEnd());

  this->setStatus(true);
}

/********************/

struct TC_Postfix04 : public TestCase {
  TC_Postfix04() : TestCase("TC_Postfix04") {;};
  void run();
};


void
TC_Postfix04::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  TokenList2 tlist(&mc, alloc);
  tlist.build("ab|cd");
  TokenList2 tlist2(&mc, alloc);
  TokenList2::tmpTokList tmpList;
  tlist2.buildPostfix(&tlist, &tmpList);

  tlist2.beginIteration();
  ASSERT_TRUE(tlist2.verifyNext(TT_SELF_CHAR, 'a'));
  ASSERT_TRUE(tlist2.verifyNext(TT_SELF_CHAR, 'b'));
  ASSERT_TRUE(tlist2.verifyNext(TT_CCAT));
  ASSERT_TRUE(tlist2.verifyNext(TT_SELF_CHAR, 'c'));
  ASSERT_TRUE(tlist2.verifyNext(TT_SELF_CHAR, 'd'));
  ASSERT_TRUE(tlist2.verifyNext(TT_CCAT));
  ASSERT_TRUE(tlist2.verifyNext(TT_PIPE));
  ASSERT_TRUE(tlist2.verifyEnd());

  this->setStatus(true);
}

/********************/

struct TC_Postfix05 : public TestCase {
  TC_Postfix05() : TestCase("TC_Postfix05") {;};
  void run();
};


void
TC_Postfix05::run()
{
  MemoryControl mc;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  TokenList2 tlist(&mc, alloc);
  tlist.build("a(b|c)d");
  TokenList2 tlist2(&mc, alloc);
  TokenList2::tmpTokList tmpList;
  tlist2.buildPostfix(&tlist, &tmpList);

  tlist2.beginIteration();
  ASSERT_TRUE(tlist2.verifyNext(TT_SELF_CHAR, 'a'));
  ASSERT_TRUE(tlist2.verifyNext(TT_SELF_CHAR, 'b'));
  ASSERT_TRUE(tlist2.verifyNext(TT_SELF_CHAR, 'c'));
  ASSERT_TRUE(tlist2.verifyNext(TT_PIPE));
  ASSERT_TRUE(tlist2.verifyNext(TT_CCAT));
  ASSERT_TRUE(tlist2.verifyNext(TT_SELF_CHAR, 'd'));
  ASSERT_TRUE(tlist2.verifyNext(TT_CCAT));
  ASSERT_TRUE(tlist2.verifyEnd());

  this->setStatus(true);
}

/********************/

struct TC_Postfix_MemFail_01 : public TestCase {
  TC_Postfix_MemFail_01() : TestCase("TC_Postfix_MemFail_01") {;};
  void run();
};

void
TC_Postfix_MemFail_01::run()
{
  MemoryControlWithFailure mc;
  mc.resetCounters();
  mc.disableLimit();
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  {
    TokenList2 tlist1(&mc, alloc);
    TokenList2 tlist2(&mc, alloc);
    TokenList2::tmpTokList tmpList;

    tlist1.build("a");
    tlist2.buildPostfix(&tlist1, &tmpList);
  }

  size_t numAllocs = mc.m_numAllocs;
  for (size_t lim = 0; lim < numAllocs; lim++) {
    mc.resetCounters();
    mc.setLimit(lim);

    try {
      TokenList2 tlist1(&mc, alloc);
      tlist1.build("a");

      TokenList2 tlist2(&mc, alloc);
      TokenList2::tmpTokList tmpList;

      tlist2.buildPostfix(&tlist1, &tmpList);
      ASSERT_TRUE(false);
    }
    catch (const bad_alloc &e) {
      ASSERT_TRUE(true);
    }
  }

  this->setStatus(true);
}


/****************************************************/
/****************************************************/
/* Build larger objs                                */
/****************************************************/
/****************************************************/

struct TC_BuilderBasic01 : public TestCase {
  TC_BuilderBasic01() : TestCase("TC_BuilderBasic01") {;};
  void run();
};

void
TC_BuilderBasic01::run()
{
  MemoryControlWithFailure mc;
  mc.resetCounters();
  mc.disableLimit();
  
  Builder b(&mc);

  {
    Builder *b2 = new (&mc) Builder(&mc);
    b2->~Builder();
    mc.deallocate(b2, sizeof(*b2));
  }

  this->setStatus(true);
}

/********************/

struct TC_BuilderBasic02 : public TestCase {
  TC_BuilderBasic02() : TestCase("TC_BuilderBasic02") {;};
  static void *f1(void *arg, const char *ptr, size_t len);
  void run();
};

void *
TC_BuilderBasic02::f1(void *arg, const char *ptr, size_t len)
{
  return NULL;
}

void
TC_BuilderBasic02::run()
{
  MemoryControlWithFailure mc;
  mc.resetCounters();
  mc.disableLimit();
  
  Builder b(&mc);
  b.addRegEx("a", &TC_BuilderBasic02::f1, NULL);
  BuilderLimits lim;
  NFA *nfa = b.BuildNFA(&mc, &lim);
  ASSERT_TRUE(nfa != NULL);
  nfa->~NFA();
  mc.deallocate(nfa, sizeof(*nfa));

  this->setStatus(true);
}

/********************/

struct TC_BuilderBasic03 : public TestCase {
  TC_BuilderBasic03() : TestCase("TC_BuilderBasic03") {;};
  static void *f1(void *arg, const char *ptr, size_t len);
  void run();
};

void *
TC_BuilderBasic03::f1(void *arg, const char *ptr, size_t len)
{
  return NULL;
}

void
TC_BuilderBasic03::run()
{
  MemoryControlWithFailure mc;
  mc.resetCounters();
  mc.disableLimit();

  Builder b(&mc);
  b.addRegEx("a", &TC_BuilderBasic03::f1, NULL);
  BuilderLimits lim;
  NFA *nfa = b.BuildNFA(&mc, &lim);

  ASSERT_TRUE( nfa != NULL );

  nfa->~NFA();
  mc.deallocate(nfa, sizeof(*nfa));

  this->setStatus(true);
}

/****************************************************/
/* top level                                        */
/****************************************************/
static TestSuite *
make_suite_all_tests()
{
  TestSuite *s;

  s = new TestSuite();

  s->addTestCase(new TC_Basic01());
  s->addTestCase(new TC_Basic02());

  s->addTestCase(new TC_Tokens01());
  s->addTestCase(new TC_Tokens02());
  s->addTestCase(new TC_Tokens03());
  s->addTestCase(new TC_Tokens04());
  s->addTestCase(new TC_Tokens05());
  s->addTestCase(new TC_Tokens06());
  s->addTestCase(new TC_Tokens07());
  s->addTestCase(new TC_Tokens08());

  s->addTestCase(new TC_Tokens201());
  s->addTestCase(new TC_Tokens202());
  s->addTestCase(new TC_Tokens203());
  s->addTestCase(new TC_Tokens204());
  s->addTestCase(new TC_Tokens205());
  s->addTestCase(new TC_Tokens206());
  s->addTestCase(new TC_Tokens207());
  s->addTestCase(new TC_Tokens208());
  s->addTestCase(new TC_Tokens209());
  s->addTestCase(new TC_Tokens210());

  s->addTestCase(new TC_MemFail01());
  s->addTestCase(new TC_MemFail02());
  s->addTestCase(new TC_MemFail03());
  s->addTestCase(new TC_MemFail04());
  s->addTestCase(new TC_MemFail05());

  s->addTestCase(new TC_MemFail2_02());
  s->addTestCase(new TC_MemFail2_03());
  s->addTestCase(new TC_MemFail2_04());
  s->addTestCase(new TC_MemFail2_05());

  s->addTestCase(new TC_Postfix01());
  s->addTestCase(new TC_Postfix02());
  s->addTestCase(new TC_Postfix03());
  s->addTestCase(new TC_Postfix04());
  s->addTestCase(new TC_Postfix05());

  s->addTestCase(new TC_Postfix_MemFail_01());

  s->addTestCase(new TC_BuilderBasic01());
  s->addTestCase(new TC_BuilderBasic02());
  s->addTestCase(new TC_BuilderBasic03());

  return s;
}

static TestSuite *
get_named_tests(TestSuite *all, int argc, const char **argv)
{
  TestSuite *s2 = new TestSuite;

  list<TestCase *>::iterator iter = all->tests.begin();
  while (iter != all->tests.end()) {
    for (int i = 1; i < argc; i++) {
      TestCase *tc = *iter;
      if (tc->name.compare(argv[i]) == 0) {
	TestCase *tc2 = tc->makeReference();
	s2->addTestCase(tc2);
      }
    }
    iter++;
  }

  return s2;
}

int
main(int argc, const char **argv)
{
  TestSuite *all = make_suite_all_tests();
  TestSuite *to_run = NULL;
  TestOption opts;
  
  opts.verbose = true;

  if (argc != 1) {
    to_run = get_named_tests(all, argc, argv);
    delete all;
    all = NULL;
  }
  else {
    to_run = all;
    all = NULL;
  }

  TestResult result;
  to_run->run(&result, &opts);

  result.report();
  delete to_run;

  if (result.n_run == result.n_pass
      && result.n_fail == 0
      && result.n_exceptions == 0)
    return 0;
  
  return 1;
}
