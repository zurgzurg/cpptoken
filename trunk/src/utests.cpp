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
public:
  TestFailure(const char *file, int line);
  ~TestFailure() throw() {;};

  const char *what() const throw();
};

TestFailure::TestFailure(const char *file, int line)
{
  stringstream tmp;
  tmp << "TestFailure excecption: ";
  tmp << file;
  tmp << ":";
  tmp << line;
  tmp << "\n";
  this->msg = tmp.str();
}

const char *
TestFailure::what() const throw()
{
  const char *r;

  r = this->msg.c_str();

  return r;
}

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
  throw TestFailure(fname, line);
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

  void run(TestResult *);
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
TestSuite::run(TestResult *result)
{
  list<TestCase *>::iterator iter;
  
  iter = this->tests.begin();
  while (iter != this->tests.end()) {
    TestCase *tc = *iter;

    tc->result = result;
    try {
      tc->setup();

      result->n_run++;
      tc->statusSet = false;
      tc->run();

      if (tc->statusSet == false)
	result->n_fail++;

    }
    catch (TestFailure e) {
      result->tests_e.push_back( tc->name + e.what() );
      result->n_exceptions++;
    }

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

  TokenList *toks = new TokenList(alloc, "");
  delete toks;
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

  TokenList tlist(alloc, "a");
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

  TokenList tlist(alloc, "a*");
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
    TokenList tlist(alloc, "a|b");
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
    TokenList tlist(alloc, "ab");
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
    TokenList tlist(alloc, "abc");
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
    TokenList tlist(alloc, "(a)");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_LPAREN));
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNext(TT_RPAREN));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "[abc]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNextCharClass("abc", 3));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "[a-c]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNextCharClass("abc", 3));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "[-c]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNextCharClass("-c", 2));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "[a-cx-]");
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
    TokenList tlist(alloc, "[^abc]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyCharClassLength(256 - 3));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "[^abc]");
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
    TokenList tlist(alloc, "a{1,2}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 1, true, 2));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "a{10,20}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 10, true, 20));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "a{ 10,20}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 10, true, 20));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "a{ 10 , 20}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 10, true, 20));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "a{ 2 , 3 }");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 2, true, 3));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "a{2,}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 2, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "a{ 23 ,}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 23, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "a{ 23, }");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 23, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "a{2}");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 2, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "a{ 2 }");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 2, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "a{ 22 }");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(TT_SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNextQuantifier(true, 22, false, 0));
    tlist.incrementIterator();
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist(alloc, "a{,3}");
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
    TokenList tlist(alloc, "{2 2}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(e.getErrorIndex() == 3);
  }

  try {
    TokenList tlist(alloc, "{2  2}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(e.getErrorIndex() == 4);
  }

  try {
    TokenList tlist(alloc, "{2-}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(e.getErrorIndex() == 2);
  }

  try {
    TokenList tlist(alloc, "{9999999999999999999999999999999999999999999}");
    ASSERT_TRUE(false);
  }
  catch (const SyntaxError &e) {
    ASSERT_TRUE(true);
  }

  try {
    TokenList tlist(alloc, "{9, 99999999999999999999999999999999999999999999}");
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
    TokenList tlist(alloc, "a[b");
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
    TokenList tlist(alloc, "a[b");
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
  size_t  n_allocs;
  size_t  n_deallocs;

  bool    use_limit;
  size_t  limit;

  virtual void *allocate(size_t);
  virtual void deallocate(void *, size_t);
};

void *
MemoryControlWithFailure::allocate(size_t sz)
{
  this->n_allocs++;
  if (this->use_limit && this->n_allocs > this->limit)
    throw bad_alloc();
  void *ptr = ::operator new(sz);
  return ptr;
}

void
MemoryControlWithFailure::deallocate(void *ptr, size_t sz)
{
  this->n_deallocs++;
  ::operator delete(ptr);
}

struct TC_MemFail02 : public TestCase {
  TC_MemFail02() : TestCase("TC_MemFail02") {;};
  void run();
};

void
TC_MemFail02::run()
{
  MemoryControlWithFailure mc;
  mc.n_allocs = 0;
  mc.n_deallocs = 0;
  mc.use_limit = false;
  Alloc<REToken *> alloc;
  alloc.setMC(&mc);

  {
    TokenList tlist(alloc, "a");
  }

  cout << "num allocs=" << mc.n_allocs << "\n";
  cout << "num deallocs=" << mc.n_deallocs << "\n";

  size_t n_allocs = mc.n_allocs;
  for (size_t lim = 0; lim < n_allocs; lim++) {

    mc.n_allocs = 0;
    mc.n_deallocs = 0;
    mc.use_limit = true;
    mc.limit = lim;
    cout << "alloc with lim=" << lim << "\n";

    try {
      TokenList tlist(alloc, "a");
      ASSERT_TRUE(false);
    }
    catch (const bad_alloc &e) {
      cout << "got one out of mem exception\n";
      ASSERT_TRUE(true);
    }
  }

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

  s->addTestCase(new TC_MemFail01());
  s->addTestCase(new TC_MemFail02());

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
  to_run->run(&result);

  result.report();
  delete to_run;

  if (result.n_run == result.n_pass
      && result.n_fail == 0
      && result.n_exceptions == 0)
    return 0;
  
  return 1;
}
