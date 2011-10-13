#ifndef _CPPTOKEN_H_
#define _CPPTOKEN_H_

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

/**
 * cpptoken is a library that generated scanners at runtime.
 *
 * @note There are many functions that have public scope, but
 * are not intended for use outside of the cpptoken package. In
 * general any function that it not documented as being part of
 * the API should not be used. The functions are declared
 * with public scope to assist in testing.
 *
 */

/**
 * @mainpage notitle
 * @namespace cpptoken
 *
 * The cpptoken package provides regular expression matching services
 * that are well suited for building lexers. The lexer objects can be
 * built at run time.
 *
 */

/**
 * @page regularexpressions Regular Expression Syntax 
 *
 * This page describes the regular expressions syntax supported
 * by cpptoken.
 *
 * @section resyntop Regular Expression Syntax
 *
 * For ASCII regular expressions.
 *
 * The cpptoken package only supported DFA related regular expressions.
 * Specifically there are many constructs that only have meaning when
 * using an NFA based matched.
 *
 * First the operators supported: concatenation, alternation, kleene star,
 * parenthesis, char classes.
 *
 *    \<re\> \<re\> -- two regular 
 * 
 * @section resynbasic
 *
 * A basic regular expression is a single character. Special meta characters
 * need to be escaped in some way. The full set of special characters is
 * <tt>{ } [ ] * |<\tt>
 *
 * @section resynconcat
 * 
 * The most basic
 * 
 * @section resyncharclass Character Classes
 *
 * A character class is text between <tt>[</tt> and <tt>]</tt>. All
 * characters within the square brackets can be matched. A dash
 * character <tt>-</tt> can be used to specify a range of
 * characters. To include the dash character list it as the first
 * or last character. A circumflex <tt>^<\tt> is used to negate
 * the character class. If the circumflex character is at any position
 * other than right after the opening square bracket it is treated as
 * a normal character.
 * 
 * @section resynquant Quantifiers
 * 
 * cpptoken supports quantifiers, which can be used to specify the
 * number of times that a regular expression can be repeated. A
 * quantifier looks like <tt>{X,Y}</tt>, or <tt>{X}</tt> or
 * <tt>{,Y}</tt> or <tt>{X,}</tt>. To help with readability whitespace
 * can be added after the opening curly brace, surrounding the comma,
 * and before the closing brace. A quantier specifies that the
 * immediately preceding regular expression is repeated from X to Y
 * times. If both X and Y are present then Y must be greater than or
 * equal to X. If a single number is in the curly braces then the
 * previous regular expression must be repeated exactly X times. If
 * there is a comma and only one number the missing number is either
 * zero or infinity.
 *
 * @note Caution should be used with quantifiers. Since cpptoken builds
 * DFA's the quantifiers will be fully "unrolled". Meaning that the
 * simply shorthand can expand to very large regular expressions and
 * result in a correspondingly large DFA. For example "a{1,30000}" will
 * create a DFA with more than thirty thousand states, yet the regular
 * expression itself is only ten characters long.
 * 
 * To facilitate control over run away regular expressions there is
 * a BuilderLimits class that can be used to limit the number of states
 * in the DFA as well as the total amount of time for building the DFA.
 * 
 * @section resynalt Alternation
 * 
 * The <tt>|</tt> character is used as a kind of "or" choice in a regular
 * expression. For RE1|RE2 the meaning is regular expression 1 or regular
 * expression 2.
 * 
 * @note Watch out for precedence.
 * 
 * 
 */


namespace cpptoken {

class FABase;
class NFA;
class REToken;

struct TokenList2;
struct PatternAction;

/*********************************************************/

/**
 * MemoryControl controls memory allocation.
 *
 * An instance of this class is required to create a Builder object.
 * The user provided allocate and deallocate methods will be used for
 * all memory management.
 * 
 */
class MemoryControl {
 public:
  virtual void *allocate(size_t);
  virtual void deallocate(void *, size_t);
};

/*********************************************************/

/**
*
* The Alloc template is intended for internal use
* of cpptoken. It is here because I could not find
* a way to make the template private. Please do not
* use it.
*
*/
template <class T>
class Alloc  {
  MemoryControl *mc;

 public:
  typedef T                value_type;
  typedef T*               pointer;
  typedef const T*         const_pointer;
  typedef T&               reference;
  typedef const T&         const_reference;
  typedef std::size_t      size_type;
  typedef std::ptrdiff_t   difference_type;

  template <class U>
  struct rebind {
    typedef Alloc<U> other;
  };

  pointer address (reference value) const {
    return &value;
  }

  const_pointer address (const_reference value) const {
    return &value;
  }

  Alloc() throw() {
    this->mc = NULL;
  }

  Alloc(const Alloc&other) throw() {
    this->mc = other.mc;
  }

  template <class U>
  Alloc(const Alloc<U>&other) throw() {
    this->mc = other.getMC();
  }

  ~Alloc() throw() {
  }

  size_type max_size () const throw() {
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
  }

  pointer allocate (size_type num, const void* = 0) {
    pointer ret = (pointer)this->mc->allocate(num * sizeof(T));
    return ret;
  }

  void construct (pointer p, const T& value) {
    new((void*)p)T(value);
  }

  void destroy (pointer p) {
    p->~T();
  }

  void deallocate (pointer p, size_type num) {
    this->mc->deallocate(p, num * sizeof(T));
  }

  ////////

  MemoryControl *getMC() const throw() {return this->mc;}
  void setMC(MemoryControl *obj) throw() { this->mc = obj; }
};

template <class T1, class T2>
bool operator== (const Alloc<T1>&, const Alloc<T2>&) throw() {
  return true;
}

template <class T1, class T2>
bool operator!= (const Alloc<T1>&, const Alloc<T2>&) throw() {
  return false;
}

/*******************************************************/

/**
 * Used to represent a syntax error in a regular expression.
 */
class SyntaxError : public std::exception {
  private:
    size_t m_errIdx;
    const char *m_reason;

  public:
    SyntaxError(size_t idx, const char *msg)
      : std::exception(),
        m_errIdx(idx),
        m_reason(msg) {;};

    /**
     * Not yet supported.
     *
     * Currently this will always return a fixed value, most likely 1 or 0.
     */
    size_t getErrorIndex() const throw();
};

/*******************************************************/

/// Function to be called when a token is matched
typedef void *(*action_func)(void *userArg, const char *str, size_t len);

typedef unsigned char uchar;

/// Private type
typedef list<uchar, Alloc<uchar> > UCharList2;

/**
 * Represents run time limits for DFA construction.
 *
 * Currently the limits are unused, but are in place for the future.
 */
class BuilderLimits {
  size_t m_maxTimeInSeconds;
  size_t m_maxStates;

 public:
  BuilderLimits() : m_maxTimeInSeconds(0), m_maxStates(0) {;};
  BuilderLimits(size_t tm, size_t st)
    : m_maxTimeInSeconds(0),
      m_maxStates(0) {;};
};

/**
 * Primary interface to cpptoken.
 *
 * An instance of the Builder class is used to construct the
 * DFA that will be used to match text.
 *
 * Note the copy constructor and assignment operator are private
 * and not implemented, so there is no supported way to perform
 * those operations.
 *
 */
class Builder {
 private:
  MemoryControl *m_mc;
  list<PatternAction *, Alloc<PatternAction *> > *m_pats;

  UCharList2 *m_tmpCharList;
  UCharList2 *m_tmpInvCharList;

 public:
  /**
   * Only way of creating a Builder object.
   *
   * The MemoryControl object must not be NULL, it will be used
   * for all memory allocation.
   */
  Builder(MemoryControl *);
  
  ~Builder();

  /// This is a private function.
  static void *operator new(size_t sz);
  /// This is a private function.
  static void *operator new(size_t sz, MemoryControl *mc);
  /// This is a private function.
  static void operator delete(void *ptr, size_t sz, MemoryControl *mc);

  /* public api */

  /**
   * Add a regular expression to the builder.
   */
  void addRegEx(const char *regex, action_func, void *userArg);

  /**
   *
   */
  void addRegEx(const char *regex, void *tok);
  
  /* not for external use */
  NFA *BuildNFA(MemoryControl *, BuilderLimits *);

  /* tokenize regex */
  TokenList2 *tokenizeRegEx(const char *regex, size_t start, size_t len);

 private:
  Builder(const Builder &);
  Builder &operator=(const Builder &);
};


}

#endif
