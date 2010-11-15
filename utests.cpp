#include <list>
#include <string>
#include <sstream>
#include <exception>
#include <iostream>

using namespace std;

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

  TestResult() : n_run(0), n_pass(0), n_exceptions(0), n_asserts(0) {;};

  void report();
};

void
TestResult::report()
{
  cout << "num_run=" << this->n_run
       << " num_pass=" << this->n_pass
       << " num_exceptions=" << this->n_exceptions
       << " num_asserts=" << this->n_asserts
       << "\n";
  return;
}

/****************************************************/
struct TestCase {
  TestResult *result;
  string name;

  virtual void setup();
  virtual void run();
  virtual void teardown();

  TestCase(const char *nm) : result(NULL), name(nm) {;};

  void assertTrue(bool);
  void setPass();
  void setFail();
};

void
TestCase::assertTrue(bool c)
{
  if (this->result)
    this->result->n_asserts++;
  if (c == true)
    return;
  throw TestFailure(__FILE__,__LINE__);
}

void
TestCase::setPass()
{
  if (this->result)
    this->result->n_pass++;
}

void
TestCase::setFail()
{
  if (this->result)
    this->result->n_fail++;
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

  void run(TestResult *);
  void addTestCase(TestCase *);
};

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
      tc->run();
      tc->teardown();

      result->n_run++;
    }
    catch (TestFailure e) {
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
  this->assertTrue(true);
  this->setPass();
}


/*************/

TestSuite *
make_basic_suite()
{
  TestSuite *s;

  s = new TestSuite();
  s->addTestCase(new TC_Basic01());

  return s;
}

/****************************************************/
int
main(int argc, const char **argv)
{
  TestSuite *s = make_basic_suite();
  TestResult result;

  s->run(&result);
  result.report();

  return 0;
}
