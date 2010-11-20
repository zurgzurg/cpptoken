#include <stdarg.h>

#include <list>
#include <string>
#include <sstream>
#include <exception>
#include <iostream>

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

  virtual void setup();
  virtual void run();
  virtual void teardown();

  TestCase(const char *nm) : result(NULL), name(nm) {;};

  void assertTrue(bool, const char *file, int line);
  void setStatus(bool);
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
    delete tc;
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

struct TC_Basic02 : public TestCase {
  TC_Basic02() : TestCase("TC_Basic02") {;};
  void run();
};

void
TC_Basic02::run()
{
  TokenList *toks = new TokenList("");
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

struct TC_Tokens02 : public TestCase {
  TC_Tokens02() : TestCase("TC_Tokens02") {;};
  void run();
};

void
TC_Tokens02::run()
{
  TokenList tlist("a");
  list<REToken *>::iterator iter = tlist.m_toks.begin();

  ASSERT_TRUE(tlist.equals(iter, SELF_CHAR, 'a'));
  this->setStatus(true);
}

struct TC_Tokens03 : public TestCase {
  TC_Tokens03() : TestCase("TC_Tokens03") {;};
  void run();
};

void
TC_Tokens03::run()
{
  TokenList tlist("a*");
  list<REToken *>::iterator iter = tlist.m_toks.begin();

  ASSERT_TRUE(tlist.equals(iter, SELF_CHAR, 'a'));
  iter++;
  ASSERT_TRUE(tlist.equals(iter, STAR));
  this->setStatus(true);
}

struct TC_Tokens04 : public TestCase {
  TC_Tokens04() : TestCase("TC_Tokens04") {;};
  void run();
};

void
TC_Tokens04::run()
{
  {
    TokenList tlist("a|b");
    list<REToken *>::iterator iter = tlist.m_toks.begin();
    ASSERT_TRUE(tlist.equals(iter, SELF_CHAR, 'a'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, PIPE));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, SELF_CHAR, 'b'));
    iter++;
    ASSERT_TRUE(iter == tlist.m_toks.end());
  }

  {
    TokenList tlist("ab");
    list<REToken *>::iterator iter = tlist.m_toks.begin();
    ASSERT_TRUE(tlist.equals(iter, SELF_CHAR, 'a'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, CCAT));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, SELF_CHAR, 'b'));
    iter++;
    ASSERT_TRUE(iter == tlist.m_toks.end());
  }
  
  {
    TokenList tlist("abc");
    list<REToken *>::iterator iter = tlist.m_toks.begin();
    ASSERT_TRUE(tlist.equals(iter, SELF_CHAR, 'a'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, CCAT));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, SELF_CHAR, 'b'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, CCAT, 'b'));
    iter++;
    ASSERT_TRUE(tlist.equals(iter, SELF_CHAR, 'c'));
    iter++;
    ASSERT_TRUE(iter == tlist.m_toks.end());
  }

  {
    TokenList tlist("(a)");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(LPAREN));
    ASSERT_TRUE(tlist.verifyNext(SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNext(RPAREN));
    ASSERT_TRUE(tlist.verifyEnd());
  }

  {
    TokenList tlist("[abc]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNextCharClass("abc", 3));
    ASSERT_TRUE(tlist.verifyEnd());
  }

#if 0
  {
    TokenList tlist("[a-c]");
    tlist.beginIteration();
    ASSERT_TRUE(tlist.verifyNext(LPAREN));
    ASSERT_TRUE(tlist.verifyNext(SELF_CHAR, 'a'));
    ASSERT_TRUE(tlist.verifyNext(PIPE));
    ASSERT_TRUE(tlist.verifyNext(SELF_CHAR, 'b'));
    ASSERT_TRUE(tlist.verifyNext(PIPE));
    ASSERT_TRUE(tlist.verifyNext(SELF_CHAR, 'c'));
    ASSERT_TRUE(tlist.verifyNext(RPAREN));
    ASSERT_TRUE(tlist.verifyEnd());
  }
#endif

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

  return s;
}

int
main(int argc, const char **argv)
{
  TestSuite *s = make_suite_all_tests();
  TestResult result;

  s->run(&result);
  result.report();
  delete s;

  if (result.n_run == result.n_pass
      && result.n_fail == 0
      && result.n_exceptions == 0)
    return 0;
  
  return 1;
}
