/*
 * Copyright 2009-2012 Bjorn Fahller <bjorn@fahller.se>
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef CRPCUT_HPP
#define CRPCUT_HPP

#ifdef GMOCK_INCLUDE_GMOCK_GMOCK_H_
#undef ADD_FAILURE
#undef ASSERT_ANY_THROW
#undef ASSERT_DEATH
#undef ASSERT_DEBUG_DEATH
#undef ASSERT_DOUBLE_EQ
#undef ASSERT_EQ
#undef ASSERT_EXIT
#undef ASSERT_FALSE
#undef ASSERT_FLOAT_EQ
#undef ASSERT_GE
#undef ASSERT_GT
#undef ASSERT_HRESULT_FAILED
#undef ASSERT_HRESULT_SUCCEEDED
#undef ASSERT_LE
#undef ASSERT_LT
#undef ASSERT_NE
#undef ASSERT_NEAR
#undef ASSERT_NO_FATAL_FAILURE
#undef ASSERT_NO_THROW
#undef ASSERT_PRED
#undef ASSERT_PRED_FORMAT
#undef ASSERT_STRCASEEQ
#undef ASSERT_STRCASENE
#undef ASSERT_STREQ
#undef ASSERT_STRNE
#undef ASSERT_THROW
#undef ASSERT_TRUE
#undef EXPECT_ANY_THROW
#undef EXPECT_DEATH
#undef EXPECT_DEBUG_DEATH
#undef EXPECT_DOUBLE_EQ
#undef EXPECT_EQ
#undef EXPECT_EXIT
#undef EXPECT_FALSE
#undef EXPECT_FATAL_FAILURE
#undef EXPECT_FATAL_FAILURE_ON_ALL_THREADS
#undef EXPECT_FLOAT_EQ
#undef EXPECT_GE
#undef EXPECT_GT
#undef EXPECT_HRESULT_FAILED
#undef EXPECT_HRESULT_SUCCEEDED
#undef EXPECT_LE
#undef EXPECT_LT
#undef EXPECT_NE
#undef EXPECT_NEAR
#undef EXPECT_NONFATAL_FAILURE
#undef EXPECT_NONFATAL_FAILURE_ON_ALL_THREADS
#undef EXPECT_NO_FATAL_FAILURE
#undef EXPECT_NO_THROW
#undef EXPECT_PRED
#undef EXPECT_PRED_FORMAT
#undef EXPECT_STRCASEEQ
#undef EXPECT_STRCASENE
#undef EXPECT_STREQ
#undef EXPECT_STRNE
#undef EXPECT_THROW
#undef EXPECT_TRUE
#undef FAIL
#undef FRIEND_TEST
#undef GTEST_ASSERT_
#undef GTEST_CASE_NAMESPACE_
#undef GTEST_IMPL_CMP_HELPER_
#undef GTEST_INCLUDE_GTEST_GTEST_DEATH_TEST_H_
#undef GTEST_INCLUDE_GTEST_GTEST_H_
#undef GTEST_INCLUDE_GTEST_GTEST_MESSAGE_H_
#undef GTEST_INCLUDE_GTEST_GTEST_PARAM_TEST_H_
#undef GTEST_INCLUDE_GTEST_GTEST_PRED_IMPL_H_
#undef GTEST_INCLUDE_GTEST_GTEST_PROD_H_
#undef GTEST_INCLUDE_GTEST_GTEST_SPI_H_
#undef GTEST_INCLUDE_GTEST_GTEST_TEST_PART_H_
#undef GTEST_INCLUDE_GTEST_GTEST_TYPED_TEST_H_
#undef GTEST_PRED
#undef GTEST_PRED_FORMAT
#undef GTEST_REGISTERED_TEST_NAMES_
#undef GTEST_TYPED_TEST_CASE_P_STATE_
#undef GTEST_TYPE_PARAMS_
#undef INSTANTIATE_TEST_CASE_P
#undef INSTANTIATE_TYPED_TEST_CASE_P
#undef REGISTER_TYPED_TEST_CASE_P
#undef RUN_ALL_TESTS
#undef SCOPED_TRACE
#undef SUCCEED
#undef TEST
#undef TEST_F
#undef TEST_P
#undef TYPED_TEST
#undef TYPED_TEST_CASE
#undef TYPED_TEST_CASE_P
#undef TYPED_TEST_P

#define CRPCUT_DEFINE_REPORTER                                          \
  class crpcut_reporter : public ::testing::EmptyTestEventListener      \
  {                                                                     \
  public:                                                               \
    virtual void OnTestPartResult(const testing::TestPartResult& result) \
    {                                                                   \
      if (result.failed())                                              \
        {                                                               \
          crpcut::heap::set_limit(crpcut::heap::system);                \
          std::ostringstream os;                                        \
          if (result.file_name())                                       \
            {                                                           \
              os << result.file_name()                                  \
                 << ":"                                                 \
                 << result.line_number()                                \
                 << "\n";                                               \
            }                                                           \
          os << result.summary() << result.message();                   \
          crpcut::comm::report(crpcut::comm::exit_fail, os);            \
        }                                                               \
    }                                                                   \
  };                                                                    \
  ::testing::TestEventListeners& listeners =                            \
                ::testing::UnitTest::GetInstance()->listeners();        \
  delete listeners.Release(listeners.default_result_printer());         \
  listeners.Append(new crpcut_reporter())

#else

#define CRPCUT_DEFINE_REPORTER do {} while (0)

// In a way this isn't nice, but the resulting compiler error gives
// the user a very obvious hint about what's wrong and what to do instead

#define ERRMSG "You must include <gmock/gmock.h> before <crpcut_hpp>"
#define EXPECT_CALL ERRMSG
#define ON_CALL ERRMSG
#define MOCK_METHOD0 ERRMSG
#define MOCK_METHOD0_T ERRMSG
#define MOCK_METHOD1 ERRMSG
#define MOCK_METHOD1_T ERRMSG
#define MOCK_METHOD2 ERRMSG
#define MOCK_METHOD2_T ERRMSG
#define MOCK_METHOD3 ERRMSG
#define MOCK_METHOD3_T ERRMSG
#define MOCK_METHOD4 ERRMSG
#define MOCK_METHOD4_T ERRMSG
#define MOCK_METHOD5 ERRMSG
#define MOCK_METHOD5_T ERRMSG
#define MOCK_METHOD6 ERRMSG
#define MOCK_METHOD6_T ERRMSG
#define MOCK_METHOD7 ERRMSG
#define MOCK_METHOD7_T ERRMSG
#define MOCK_METHOD8 ERRMSG
#define MOCK_METHOD8_T ERRMSG
#define MOCK_METHOD9 ERRMSG
#define MOCK_METHOD9_T ERRMSG
#define MOCK_METHOD10 ERRMSG
#define MOCK_METHOD10_T ERRMSG

#define MOCK_CONST_METHOD0 ERRMSG
#define MOCK_CONST_METHOD0_T ERRMSG
#define MOCK_CONST_METHOD1 ERRMSG
#define MOCK_CONST_METHOD1_T ERRMSG
#define MOCK_CONST_METHOD2 ERRMSG
#define MOCK_CONST_METHOD2_T ERRMSG
#define MOCK_CONST_METHOD3 ERRMSG
#define MOCK_CONST_METHOD3_T ERRMSG
#define MOCK_CONST_METHOD4 ERRMSG
#define MOCK_CONST_METHOD4_T ERRMSG
#define MOCK_CONST_METHOD5 ERRMSG
#define MOCK_CONST_METHOD5_T ERRMSG
#define MOCK_CONST_METHOD6 ERRMSG
#define MOCK_CONST_METHOD6_T ERRMSG
#define MOCK_CONST_METHOD7 ERRMSG
#define MOCK_CONST_METHOD7_T ERRMSG
#define MOCK_CONST_METHOD8 ERRMSG
#define MOCK_CONST_METHOD8_T ERRMSG
#define MOCK_CONST_METHOD9 ERRMSG
#define MOCK_CONST_METHOD9_T ERRMSG
#define MOCK_CONST_METHOD10 ERRMSG
#define MOCK_CONST_METHOD10_T ERRMSG

#define MOCK_METHOD0_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD0_T_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD1_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD1_T_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD2_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD2_T_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD3_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD3_T_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD4_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD4_T_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD5_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD5_T_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD6_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD6_T_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD7_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD7_T_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD8_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD8_T_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD9_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD9_T_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD10_WITH_CALLTYPE ERRMSG
#define MOCK_METHOD10_T_WITH_CALLTYPE ERRMSG

#endif

#ifdef __INTEL_COMPILER
#  ifndef __GXX_EXPERIMENTAL_CPP0X__
#    define CRPCUT_DECLTYPE typeof
#  else
#    define CRPCUT_EXPERIMENTAL_CXX0X
#    define CRPCUT_DECLTYPE decltype
#  endif
#  define BOOST_TR1
#  define CRPCUT_NORETURN
#else
#  ifdef __GNUG__
#    ifdef __GXX_EXPERIMENTAL_CXX0X__
#      define CRPCUT_DECLTYPE decltype
#      define CRPCUT_EXPERIMENTAL_CXX0X
#      if (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)
#        define CRPCUT_SUPPORTS_VTEMPLATES
#      endif
#    else
#      define CRPCUT_DECLTYPE typeof
#    endif
#    define CRPCUT_NORETURN __attribute__((noreturn))
#    ifndef __EXCEPTIONS
#      define CRPCUT_NO_EXCEPTION_SUPPORT
#    endif
#  else
#    define CRPCUT_NORETURN
#  endif
#endif
#include <stdexcept>
#include <sstream>
#include <string>
#include <iostream>
#include <ostream>
#include <iomanip>
#include <cerrno>
#include <cassert>
#ifdef CRPCUT_EXPERIMENTAL_CXX0X
#  include <type_traits>
#  include <array>
#else
#  ifdef BOOST_TR1
#    include <boost/tr1/type_traits.hpp>
#    include <boost/tr1/array.hpp>
#  else
#    include <tr1/type_traits>
#    include <tr1/array>
#  endif
#endif
#include <cstring>
#include <cstdlib>
#include <limits>
#include <memory>
extern "C"
{
#include <limits.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <regex.h>
#include <stdint.h>
}

namespace std {
#if (not defined(CRPCUT_EXPERIMENTAL_CXX0X) || defined (BOOST_TR1))
  using std::tr1::array;
  using std::tr1::remove_cv;
  using std::tr1::remove_reference;
#endif
}



#ifdef CRPCUT_NO_EXCEPTION_SUPPORT
#ifndef try
#define try if (true)
#endif
#define CATCH_BLOCK(specifier, code)
#else
#define CATCH_BLOCK(specifier, code) catch (specifier) code
#endif

#define ANY_CODE -1
#define CRPCUT_VERBATIM(x) x

namespace crpcut {

  template <typename T>
  struct eval_t
  {
    typedef const T &type;
    static type func(const T& t) { return t; }
  };

  template <typename T>
  typename eval_t<T>::type eval(const T& t);

  namespace expr {
    template <typename T>
    const T& gen();
  }

  class test_case_factory;

  typedef enum { verbatim, uppercase, lowercase } case_convert_type;
  template <case_convert_type>
  class collate_t;

  namespace wrapped { // stdc and posix functions
    CRPCUT_NORETURN void abort();
    void                 free(const void* p);
    void                *malloc(size_t n);
    ssize_t              read(int fd, void* p, size_t s);
    int                  regcomp(regex_t*, const char*, int);
    size_t               regerror(int, const regex_t*, char*, size_t);
    int                  regexec(const regex_t*, const char*,
                                 size_t, regmatch_t[], int);
    void                 regfree(regex_t*);
    int                  strcmp(const char *l, const char *r);
    char *               strerror(int n);
    size_t               strlen(const char *r);
    ssize_t              write(int fd, const void* p, size_t s);
    int                  close(int fd);
  }

  namespace lib {
    // works like std::strcpy, except the return value is the pointer to
    // the nul terminator in the destination, making concatenations easy
    // and cheap
    template <typename T, typename U>
    T strcpy(T d, U s)
    {
      while ((*d = *s))
        {
          ++d;
          ++s;
        }
      return d;
    }
  }


  namespace libs
  {
    const char * const * libc();
    const char * const * librt();
  }

  namespace libwrapper {

    class dlloader // thin wrapper around dlopen(), dlclose(), dlsym()
    {
    protected:
      static void *load(const char *const *lib);
      static void *symbol(void *, const char *name);
      static void unload(void*);
      static void assert_lib_is_loaded(void*);
      dlloader() {}
      ~dlloader() {}
    private:
    };

    template <const char * const * (&lib)()>
    class loader : dlloader
    {
      static void *& libptr()
      {
        static void *p = 0;
        return p;
      }
      loader()  {
        libptr() = dlloader::load(lib());
        assert_lib_is_loaded(libptr());
      }
      ~loader() { unload(libptr()); libptr() = 0;}
    public:
      static bool is_loaded() { return libptr(); }
      template <typename T>
      T sym(const char *name)
      {
        assert_lib_is_loaded(libptr());
        union {       // I don't like silencing the warning this way,
          T func;     // but it should be safe. *IF* the function pointer
          void *addr; // can't be represented by void*, dlsym() can't
        } dlr;        // exist either.
        dlr.addr = symbol(libptr(), name);
        return dlr.func;
      }
      static bool has_symbol(const char *name)
      {
        return is_loaded() && symbol(libptr(), name);
      }
      static loader& obj()
      {
        static loader o;
        return o;
      }
    };

  }


  namespace stream_checker
  {
    template <typename V, typename U>
    char operator<<(V&, const U&);

    template <typename T>
    struct is_output_streamable
    {
    private:
      static std::ostream &os;
      static T& t;

      static char check(char);
      static std::pair<char, char> check(std::ostream&);
    public:
      static const bool value = sizeof(check(os << t)) != sizeof(char);
    };


    template <typename T>
    struct is_output_streamable<const T>
    {
      static const bool value = is_output_streamable<T>::value;
    };

    template <typename T>
    struct is_output_streamable<volatile T>
    {
      static const bool value = is_output_streamable<T>::value;
    };

    template <typename T>
    struct is_output_streamable<T&>
    {
      static const bool value = is_output_streamable<T>::value;
    };

    template <size_t N>
    struct is_output_streamable<char[N]>
    {
      static const bool value = true;
    };

    template <size_t N>
    struct is_output_streamable<const char[N]>
    {
      static const bool value = true;
    };

    template <typename T>
    struct is_output_streamable<T*>
    {
      static const bool value = true;
    };

    template <typename T>
    struct is_output_streamable<T (*)()>
    {
      static const bool value = false;
    };

    template <typename T, typename P1>
    struct is_output_streamable<T (*)(P1)>
    {
      static const bool value = false;
    };

    template <typename T, typename P1, typename P2>
    struct is_output_streamable<T (*)(P1, P2)>
    {
      static const bool value = false;
    };

    template <typename T, typename P1, typename P2, typename P3>
    struct is_output_streamable<T (*)(P1, P2, P3)>
    {
      static const bool value = false;
    };

    template <typename T, typename P1, typename P2, typename P3, typename P4>
    struct is_output_streamable<T (*)(P1, P2, P3, P4)>
    {
      static const bool value = false;
    };

    template <typename T, typename P1, typename P2, typename P3, typename P4,
              typename P5>
    struct is_output_streamable<T (*)(P1, P2, P3, P4, P5)>
    {
      static const bool value = false;
    };

    template <typename T, typename P1, typename P2, typename P3, typename P4,
              typename P5, typename P6>
    struct is_output_streamable<T (*)(P1, P2, P3, P4, P5, P6)>
    {
      static const bool value = false;
    };

    template <typename T, typename P1, typename P2, typename P3, typename P4,
              typename P5, typename P6, typename P7>
    struct is_output_streamable<T (*)(P1, P2, P3, P4, P5, P6, P7)>
    {
      static const bool value = false;
    };

    template <typename T, typename P1, typename P2, typename P3, typename P4,
              typename P5, typename P6, typename P7, typename P8>
    struct is_output_streamable<T (*)(P1, P2, P3, P4, P5, P6, P7, P8)>
    {
      static const bool value = false;
    };

    template <typename T, typename P1, typename P2, typename P3, typename P4,
              typename P5, typename P6, typename P7, typename P8, typename P9>
    struct is_output_streamable<T (*)(P1, P2, P3, P4, P5, P6, P7, P8, P9)>
    {
      static const bool value = false;
    };

    template <typename T, typename C>
    struct is_output_streamable<T (C::*)()>
    {
      static const bool value = false;
    };

    template <typename T, typename C, typename P1>
    struct is_output_streamable<T (C::*)(P1)>
    {
      static const bool value = false;
    };

    template <typename T, typename C,
              typename P1, typename P2>
    struct is_output_streamable<T (C::*)(P1, P2)>
    {
      static const bool value = false;
    };

    template <typename T, typename C,
              typename P1, typename P2, typename P3>
    struct is_output_streamable<T (C::*)(P1, P2, P3)>
    {
      static const bool value = false;
    };

    template <typename T, typename C,
              typename P1, typename P2, typename P3, typename P4>
    struct is_output_streamable<T (C::*)(P1, P2, P3, P4)>
    {
      static const bool value = false;
    };

    template <typename T, typename C,
              typename P1, typename P2, typename P3, typename P4,
              typename P5>
    struct is_output_streamable<T (C::*)(P1, P2, P3, P4, P5)>
    {
      static const bool value = false;
    };

    template <typename T, typename C,
              typename P1, typename P2, typename P3, typename P4,
              typename P5, typename P6>
    struct is_output_streamable<T (C::*)(P1, P2, P3, P4, P5, P6)>
    {
      static const bool value = false;
    };

    template <typename T, typename C,
              typename P1, typename P2, typename P3, typename P4,
              typename P5, typename P6, typename P7>
    struct is_output_streamable<T (C::*)(P1, P2, P3, P4, P5, P6, P7)>
    {
      static const bool value = false;
    };

    template <typename T, typename C,
              typename P1, typename P2, typename P3, typename P4,
              typename P5, typename P6, typename P7, typename P8>
    struct is_output_streamable<T (C::*)(P1, P2, P3, P4, P5, P6, P7, P8)>
    {
      static const bool value = false;
    };

    template <typename T, typename C,
              typename P1, typename P2, typename P3, typename P4,
              typename P5, typename P6, typename P7, typename P8, typename P9>
    struct is_output_streamable<T (C::*)(P1, P2, P3, P4, P5, P6, P7, P8, P9)>
    {
      static const bool value = false;
    };
  } // namespace stream_checker


  class crpcut_none {};


  namespace datatypes {


    template <typename T, std::size_t N>
    class array_v : private std::array<T, N>
    {
      typedef std::array<T, N> array;
    public:
      typedef typename array::value_type             value_type;
      typedef typename array::reference              reference;
      typedef typename array::const_reference        const_reference;
      typedef typename array::iterator               iterator;
      typedef typename array::const_iterator         const_iterator;
      typedef typename array::size_type              size_type;
      typedef typename array::difference_type        difference_type;
      typedef typename array::reverse_iterator       reverse_iterator;
      typedef typename array::const_reverse_iterator const_reverse_iterator;

      array_v();
      using array::begin;
      iterator end();
      const_iterator end() const;
      reverse_iterator rbegin();
      const_reverse_iterator rbegin() const;
      using array::rend;
      size_type size() const;
      using array::max_size;
      bool empty() const;

      using array::operator[];
      reference at(size_type n);
      const_reference at(size_type n) const;
      using array::front;
      reference back();
      const_reference back() const;
      using array::data;
      void push_back(const value_type &x);
      void pop_back();
    private:
      size_t num_elements;
    };

    class crpcut_none {};

    template <typename T1 = crpcut_none, typename T2 = crpcut_none>
    class tlist : public T1,
                  public T2
    {
    public:
      typedef T1 head;
      typedef T2 tail;
    };

    template <typename T>
    class tlist<crpcut_none, T>
    {
      typedef crpcut_none head;
      typedef crpcut_none tail;
    };

#ifdef CRPCUT_SUPPORTS_VTEMPLATES


    template <typename... Ts>
    struct tlist_maker;


    template <typename T>
    struct tlist_maker<T>
    {
      typedef tlist<T, tlist<> >    type;
    };

    template <typename T, typename ... Tail>
    struct tlist_maker<T, Tail...>
    {
      typedef tlist<T, typename tlist_maker<Tail...>::type> type;
    };
#else

    template <typename T1 = crpcut_none, typename T2 = crpcut_none,
              typename T3 = crpcut_none, typename T4 = crpcut_none,
              typename T5 = crpcut_none, typename T6 = crpcut_none,
              typename T7 = crpcut_none, typename T8 = crpcut_none,
              typename T9 = crpcut_none, typename T10 = crpcut_none,
              typename T11 = crpcut_none, typename T12 = crpcut_none,
              typename T13 = crpcut_none, typename T14 = crpcut_none,
              typename T15 = crpcut_none, typename T16 = crpcut_none,
              typename T17 = crpcut_none, typename T18 = crpcut_none>
    struct tlist_maker
    {
      typedef tlist<
        T1,
        tlist<
          T2,
          tlist<
            T3,
            tlist<
              T4,
              tlist<
                T5,
                tlist<
                  T6,
                  tlist<
                    T7,
                    tlist<
                      T8,
                      tlist<
                        T9,
                        tlist<
                          T10,
                          tlist<
                            T11,
                            tlist<
                              T12,
                              tlist<
                                T13,
                                tlist<
                                  T14,
                                  tlist<
                                    T15,
                                    tlist<
                                      T16,
                                      tlist<
                                        T17,
                                        tlist<T18>
                                        >
                                      >
                                    >
                                  >
                                >
                              >
                            >
                          >
                        >
                      >
                    >
                  >
                >
              >
            >
          >
        >
      type;
    };
#endif

    struct fixed_string
    {
      const char  *str;
      std::size_t  len;
    private:
      struct secret_bool;
    public:
      static fixed_string make(const char * p, size_t len)
      {
        fixed_string s = { p, len };
        return s;
      }
      template <size_t N>
      static fixed_string make(const char (&f)[N])
      {
        fixed_string s = { f, N - 1 };
        return s;
      }
      operator const secret_bool* () const
      {
        return len ? reinterpret_cast<const secret_bool*>(this) : 0;
      }
      bool operator==(const fixed_string &s) const
      {
        if (len != s.len) return false;
        if (str == s.str) return true;
        for (std::size_t i = 0; i < len; ++i)
          {
            if (str[i] != s.str[i]) return false;
          }
        return true;
      }
      bool operator!=(const fixed_string &s) const
      {
        return !(*this == s);
      }
    };

    template <template <typename> class envelope, typename T>
    class wrap
    {
    public:
      typedef datatypes::tlist<envelope<typename T::head>,
                               typename wrap<envelope,
                                             typename T::tail>::type> type;
    };

    template <template <typename> class envelope, typename T>
    class wrap<envelope, datatypes::tlist<crpcut_none, T> >
    {
    public:
      typedef datatypes::tlist<> type;
    };



    template <typename T>
    struct string_traits;

    template <>
    struct string_traits<std::string>
    {
      static const char *get_c_str(const std::string &s) { return s.c_str(); }
    };

    template <>
    struct string_traits<const char*>
    {
      static const char *get_c_str(const char *s) { return s; }
    };

    template <>
    struct string_traits<char*>
    {
      static const char *get_c_str(const char *s) { return s; }
    };

    template <typename T>
    class list_elem
    {
    public:
      list_elem();
      virtual ~list_elem();
      void link_after(list_elem& r);
      void link_before(list_elem &r);
      T *next() { return next_; }
      T *prev() { return prev_; }
      const T *next() const { return next_; }
      const T *prev() const { return prev_; }
      bool is_empty() const;
    private:
      void unlink();
      list_elem(const list_elem&);
      list_elem& operator=(const list_elem&);
      T *next_;
      T *prev_;
    };

    template <typename T>
    inline list_elem<T>::list_elem()
      : next_(static_cast<T*>(this)),
        prev_(static_cast<T*>(this))
    {
    }

    template <typename T>
    inline list_elem<T>::~list_elem()
    {
      unlink();
    }

    template <typename T>
    inline void list_elem<T>::link_after(list_elem& r)
    {
      next_ = r.next_;
      prev_ = static_cast<T*>(&r);
      next_->prev_ = static_cast<T*>(this);
      r.next_ = static_cast<T*>(this);
    }

    template <typename T>
    inline void list_elem<T>::link_before(list_elem &r)
    {
      prev_ = r.prev_;
      next_ = static_cast<T*>(&r);
      prev_->next_ = static_cast<T*>(this);
      r.prev_ = static_cast<T*>(this);
    }

    template <typename T>
    inline bool list_elem<T>::is_empty() const
    {
      return next_ == static_cast<const T*>(this);
    }

    template <typename T>
    inline void list_elem<T>::unlink()
    {
      T *n = next_;
      T *p = prev_;
      n->prev_ = p;
      p->next_ = n;
      prev_ = static_cast<T*>(this);
      next_ = static_cast<T*>(this);
    }

  } // namespace datatypes

#ifdef CRPCUT_SUPPORTS_VTEMPLATES
  template <typename D, typename ...T>
  struct match_traits
  {
    typedef D type;
  };
#else
  template <typename D,
            typename T1,                typename T2 = crpcut_none,
            typename T3 = crpcut_none,  typename T4 = crpcut_none,
            typename T5 = crpcut_none,  typename T6 = crpcut_none,
            typename T7 = crpcut_none,  typename T8 = crpcut_none,
            typename T9 = crpcut_none>
  struct match_traits
  {
    typedef D type;
  };
#endif

#define CRPCUT_TEST_PHASES(translator)   \
  translator(creating),                  \
  translator(running),                   \
  translator(destroying),                \
  translator(post_mortem),               \
  translator(child)


  typedef enum { CRPCUT_TEST_PHASES(CRPCUT_VERBATIM) } test_phase;

  class tag_list_root;
  class tag : public datatypes::list_elem<tag>
  {
    friend class tag_list_root;
    tag();
  protected:
    tag(int len, tag_list_root *list);
    ~tag();
  public:
    typedef enum { ignored, critical, non_critical } importance;
    virtual void fail();
    virtual void pass();
    virtual size_t num_failed() const;
    virtual size_t num_passed() const;
    virtual datatypes::fixed_string get_name() const = 0;
    virtual void set_importance(importance i);
    virtual importance get_importance() const;
  private:
    size_t failed_;
    size_t passed_;
    importance importance_;
  };

  class tag_list_root : public tag
  {
  public:
    tag_list_root() : tag(), longest_tag_name_(0) {}
    virtual int longest_tag_name() const { return longest_tag_name_; }
    void store_name_length(int n) { longest_tag_name_ = n; }
    template <typename T>
    class iterator_t
    {
    public:
      iterator_t(T *p) : p_(p) {};
      iterator_t& operator++() { p_ = p_->next(); return *this; }
      iterator_t operator++(int) { iterator_t rv(*this); ++(*this); return rv;}
      T *operator->() { return p_; }
      T& operator*() { return *p_; }
      bool operator==(const iterator_t &i) const { return p_ == i.p_; }
      bool operator!=(const iterator_t &i) const { return !(operator==(i)); }
    private:
      T *p_;
    };
    typedef iterator_t<tag> iterator;
    typedef iterator_t<const tag> const_iterator;
    const_iterator begin() const { return const_iterator(next()); }
    const_iterator end() const { return const_iterator(this); }
    iterator begin() { return iterator(next()); }
    iterator end() { return iterator(this); }
  private:
    int longest_tag_name_;
  };

  template <typename T>
  class crpcut_tag_info;

  template <>
  class crpcut_tag_info<crpcut_none> : public tag_list_root
  {
    crpcut_tag_info();
    virtual datatypes::fixed_string get_name() const;
  public:
    static tag_list_root& obj();
    static iterator begin();
    static iterator end();
    static int longest_name_len();
  };

  typedef crpcut_tag_info<crpcut_none> tag_list;
  template <typename T>
  class crpcut_tag_info : public tag
  {
  public:
    static crpcut_tag_info& obj()
    {
      static crpcut_tag_info t;
      return t;
    }
  private:
    crpcut_tag_info()
      : tag(get_name_len(),
            &crpcut_tag_info<crpcut_none>::obj())
    {
    }
    int get_name_len() const;
    virtual datatypes::fixed_string get_name() const;
  };

  namespace policies {

    class crpcut_exception_translator
      : public datatypes::list_elem<crpcut_exception_translator>
    {
    public:
      static std::string try_all();
    protected:
      crpcut_exception_translator();
      ~crpcut_exception_translator();
    private:
      crpcut_exception_translator(int);
      static crpcut_exception_translator& root_object();
      crpcut_exception_translator(const crpcut_exception_translator&);
      crpcut_exception_translator& operator=(const crpcut_exception_translator&);
      virtual std::string crpcut_translate() const;
    };


    namespace deaths {
      class crpcut_none;
    }

    namespace dependencies {
      class crpcut_none {};
    }

    namespace timeout {
      typedef enum { realtime, cputime } type;

      template <type t, unsigned long ms>
      class enforcer;

      template <unsigned long ms>
      struct constructor_enforcer
      {
        static const unsigned long crpcut_constructor_timeout_ms = ms;
      };

      template <unsigned long ms>
      struct destructor_enforcer
      {
        static const unsigned long crpcut_destructor_timeout_ms = ms;
      };

    }
    class default_policy
    {
    protected:
      typedef crpcut_none crpcut_test_tag;
      typedef void crpcut_run_wrapper;

      typedef deaths::crpcut_none crpcut_expected_death_cause;

      typedef dependencies::crpcut_none crpcut_dependency;

      typedef timeout::enforcer<timeout::realtime,2000> crpcut_realtime_enforcer;
      typedef timeout::enforcer<timeout::cputime, 0> crpcut_cputime_enforcer;

      typedef timeout::constructor_enforcer<1000>
      crpcut_constructor_timeout_enforcer;
      typedef timeout::destructor_enforcer<1000>
      crpcut_destructor_timeout_enforcer;
    };

    namespace deaths {

      class crpcut_none
      {
      public:
        ~crpcut_none() {}
        virtual bool crpcut_is_expected_exit(int) const;
        virtual bool crpcut_is_expected_signal(int) const;
        virtual void crpcut_expected_death(std::ostream &os) const;
        virtual void crpcut_on_ok_action(const char *wd_name) const;
        virtual unsigned long crpcut_calc_deadline(unsigned long ts) const;
      };

      template <int N, typename action>
      class signal : public virtual crpcut_none,
                     private action
      {
      public:
        virtual void crpcut_on_ok_action(const char *wd_name) const
        {
          action::crpcut_on_ok_action(wd_name);
        }
        virtual bool crpcut_is_expected_signal(int code) const;
        virtual void crpcut_expected_death(std::ostream &os) const;
      };

      template <int N, typename action>
      class exit : public virtual crpcut_none,
                   private action
      {
      public:
        virtual void crpcut_on_ok_action(const char *wd_name) const
        {
          action::crpcut_on_ok_action(wd_name);
        }
        virtual bool crpcut_is_expected_exit(int code) const;
        virtual void crpcut_expected_death(std::ostream &os) const;
      };

      template <unsigned long N>
      class timeout : public virtual crpcut_none
      {
      public:
        virtual bool          crpcut_is_expected_signal(int code) const;
        virtual void          crpcut_expected_death(std::ostream &os) const;
        virtual unsigned long crpcut_calc_deadline(unsigned long ts) const;
      };

      class wrapper;
      class timeout_wrapper;

      class no_action
      {
      public:
        virtual void crpcut_on_ok_action(const char *) const;
      };

      class wipe_working_dir
      {
      public:
        virtual void crpcut_on_ok_action(const char *wd_name) const;
      private:
      };
    } // namespace deaths

    template <int N, typename action = deaths::no_action>
    class signal_death : protected virtual default_policy
    {
    public:
      typedef deaths::wrapper crpcut_run_wrapper;
      typedef deaths::signal<N, action> crpcut_expected_death_cause;
    };

    template <int N, typename action = deaths::no_action>
    class exit_death : protected virtual default_policy
    {
    public:
      typedef deaths::wrapper  crpcut_run_wrapper;
      typedef deaths::exit<N, action>  crpcut_expected_death_cause;
    };

    template <unsigned long N>
    class realtime_timeout_death : protected virtual default_policy
    {
    public:
      typedef deaths::timeout_wrapper crpcut_run_wrapper;
      typedef deaths::timeout<N>      crpcut_expected_death_cause;
      typedef timeout::enforcer<timeout::realtime, N> crpcut_realtime_enforcer;
    };

    class any_exception_wrapper;

    template <typename exc>
    class exception_wrapper;

    template <typename T>
    class exception_specifier;

    template <typename T>
    class exception_specifier<void (T)> : protected virtual default_policy
    {
    public:
      typedef exception_wrapper<T> crpcut_run_wrapper;
    };

    template <>
    class exception_specifier<void (...)> : protected virtual default_policy
    {
    public:
      typedef any_exception_wrapper crpcut_run_wrapper;
    };

    class no_core_file : protected virtual default_policy
    {
    protected:
      no_core_file();
    };

    namespace dependencies {

      class basic_enforcer;
      class crpcut_base
      {
      protected:
      public:
        void crpcut_inc();
        ~crpcut_base();
        crpcut_base();
        void crpcut_add(basic_enforcer * other);
        bool crpcut_can_run() const;
        bool crpcut_failed() const;
        bool crpcut_succeeded() const;
        void crpcut_uninhibit_dependants();
        void crpcut_register_success(bool value = true);
      private:
        virtual void crpcut_add_action(basic_enforcer *other);
        virtual void crpcut_dec_action() {}
        enum { crpcut_success, crpcut_fail, crpcut_not_run } crpcut_state;
        int crpcut_num;
        basic_enforcer *crpcut_dependants;
      };

      class basic_enforcer : public virtual crpcut_base
      {
        friend class crpcut_base;
        basic_enforcer *next;
      protected:
        basic_enforcer();
      };


      template <typename T>
      class enforcer : private basic_enforcer
      {
      public:
        enforcer();
      };

      template <typename T, typename U = crpcut::crpcut_none>
      struct nested
      {
        typedef typename T::crpcut_dependency type;
      };

      template <>
      struct nested<crpcut::crpcut_none, crpcut::crpcut_none>
      {
        typedef crpcut::crpcut_none type;
      };

    } // namespace dependencies

    template <typename T>
    class dependency_policy : protected virtual default_policy
    {
    public:
      typedef typename datatypes::wrap<dependencies::enforcer,
                                       T>::type  crpcut_dependency;
    };

    namespace timeout {

      template <type t, unsigned long timeout_ms>
      class enforcer;

      class cputime_enforcer
      {
      public:
        cputime_enforcer(unsigned long timeout_ms);
        ~cputime_enforcer();
      private:

        unsigned long duration_ms;
        unsigned long start_timestamp_ms;
      };

      class monotonic_enforcer
      {
      protected:
        monotonic_enforcer(unsigned long timeout_ms);
        ~monotonic_enforcer();
      private:

        unsigned long duration_ms;
        unsigned long start_timestamp_ms;
      };

      template <unsigned long timeout_ms>
      class enforcer<cputime, timeout_ms>
      {
      public:
        static const unsigned long crpcut_cputime_timeout_ms = timeout_ms;
      };


      template <unsigned long timeout_ms>
      class enforcer<realtime, timeout_ms> : public monotonic_enforcer
      {
      public:
        enforcer();
      };

    } // namespace timeout


    template <timeout::type t, unsigned long timeout_ms>
    class timeout_policy;

    template <unsigned long timeout_ms>
    class timeout_policy<timeout::realtime, timeout_ms>
      : protected virtual default_policy
    {
    public:
      typedef timeout::enforcer<timeout::realtime, timeout_ms>
      crpcut_realtime_enforcer;
    };

    template <unsigned long timeout_ms>
    class timeout_policy<timeout::cputime, timeout_ms>
      : protected virtual default_policy
    {
    public:
      typedef timeout::enforcer<timeout::cputime, timeout_ms>
      crpcut_cputime_enforcer;
    };


    template <unsigned long ms>
    struct constructor_timeout_policy : public virtual default_policy
    {
      typedef timeout::constructor_enforcer<ms>
      crpcut_constructor_timeout_enforcer;
    };

    template <unsigned long ms>
    struct destructor_timeout_policy : public virtual default_policy
    {
      typedef timeout::destructor_enforcer<ms>
      crpcut_destructor_timeout_enforcer;
    };

    template <typename T>
    struct tag_policy : public virtual policies::default_policy
    {
      typedef T crpcut_test_tag;
    };
  } // namespace policies

  class crpcut_test_case_registrator;

  class test_case_base : protected virtual policies::default_policy
  {
    virtual void crpcut_run_test() = 0;
  protected:
    test_case_base();
    ~test_case_base();
  public:
    virtual crpcut_test_case_registrator& crpcut_get_reg() const = 0;
    virtual void test() = 0;
    void run();
    void crpcut_test_finished();
  private:

    bool finished;
  };

  class test_case_factory;

  namespace stream {
    template <typename charT, class traits = std::char_traits<charT> >
    class oabuf : public std::basic_streambuf<charT, traits>
    {
      typedef std::basic_streambuf<charT, traits> parent;
    public:
      oabuf(charT *begin_, charT *end_)
      {
        parent::setp(begin_, end_);
      }
      const charT *begin() const { return parent::pbase(); }
      const charT *end() const { return parent::pptr(); }
    };

    template <typename charT, typename traits = std::char_traits<charT> >
    class basic_oastream : private oabuf<charT, traits>,
                           public  std::basic_ostream<charT, traits>
    {
    public:
      basic_oastream(charT *begin_, charT *end_)
        : oabuf<charT, traits>(begin_, end_),
          std::basic_ostream<charT, traits>(this)
      {
      }
      basic_oastream(charT *begin_, size_t size_)
        : oabuf<charT, traits>(begin_, begin_ + size_),
          std::basic_ostream<charT, traits>(this)
      {
      }
      template <size_t N>
      basic_oastream(charT (&buff)[N])
        : oabuf<charT, traits>(buff, buff + N),
          std::basic_ostream<charT, traits>(this)
      {
      }
      using oabuf<charT, traits>::begin;
      using oabuf<charT, traits>::end;
      std::size_t size() const { return size_t(end() - begin()); }
      operator datatypes::fixed_string() const
      {
        datatypes::fixed_string rv = { begin(), size() };
        return rv;
      }
    };

    template <typename charT, class traits = std::char_traits<charT> >
    class iabuf : public std::basic_streambuf<charT, traits>
    {
    public:
      iabuf(const charT *begin, const charT *end)
      {
        std::basic_streambuf<charT, traits>::setbuf(const_cast<charT *>(begin),
                                                    end - begin);
        std::basic_streambuf<charT, traits>::setg(const_cast<charT *>(begin),
                                                  const_cast<charT *>(begin),
                                                  const_cast<charT *>(end));
      }
    };

    template <typename charT, typename traits = std::char_traits<charT> >
    class basic_iastream : private iabuf<charT, traits>, public std::basic_istream<charT, traits>
    {
    public:
      basic_iastream(const charT *begin, const charT *end)
        : iabuf<charT, traits>(begin, end),
          std::basic_istream<charT, traits>(this)
      {
      }
      basic_iastream(const charT *begin)
        : iabuf<charT, traits>(begin, begin + wrapped::strlen(begin)),
          std::basic_istream<charT, traits>(this)
      {
      }
    };

    template <size_t N,
              typename charT = char,
              typename traits = std::char_traits<charT> >
    class toastream : public basic_oastream<charT, traits>
    {
    public:
      toastream() : basic_oastream<charT, traits>(buffer, N) {}
    private:
      charT buffer[N];
    };

    typedef basic_oastream<char> oastream;
    typedef basic_iastream<char> iastream;

  } // stream

  namespace comm {

#define CRPCUT_COMM_MSGS(translator)             \
    translator(stdout),           /*  1 */       \
      translator(stderr),         /*  2 */       \
      translator(info),           /*  3 */       \
      translator(exit_ok),        /*  4 */       \
      translator(exit_fail),      /*  5 */       \
      translator(fail),           /*  6 */       \
      translator(dir),            /*  7 */       \
      translator(set_timeout),    /*  8 */       \
      translator(cancel_timeout), /*  9 */       \
      translator(begin_test),     /* 10 */       \
      translator(end_test)        /* 11 */

    typedef enum {
      CRPCUT_COMM_MSGS(CRPCUT_VERBATIM),
      kill_me = 0x100
    } type;


    // protocol is type -> size_t(length) -> char[length]. length may be 0.
    // reader acknowledges with length.

    class reporter
    {
      int write_fd;
      int read_fd;
    public:
      reporter();
      void set_fds(int read, int write);
      void operator()(type t, const std::ostringstream &os) const;
      template <size_t N>
      void operator()(type t, const stream::toastream<N> &os) const;
      void operator()(type t, const stream::oastream &os) const;
      void operator()(type t, const char *msg) const;
      void operator()(type t, const char *msg, size_t len) const;
      template <size_t N>
      void operator()(type t, const char (&msg)[N]) const
      {
        operator()(t, msg, N - 1);
      }
      template <typename T>
      void operator()(type t, const T& data) const;
    private:
      template <typename T>
      void write(const T& t) const;

      template <typename T>
      void read(T& t) const;
    };

    extern reporter report;

    template <type t>
    class direct_reporter
    {
    public:
      direct_reporter();
      template <typename V>
      direct_reporter& operator<<(V& v);
      template <typename V>
      direct_reporter& operator<<(const V& v);
      template <typename V>
      direct_reporter& operator<<(V (&p)(V)){ os << p; return *this; }
      template <typename V>
      direct_reporter& operator<<(V& (&p)(V&)){ os << p; return *this; }
      direct_reporter& operator<<(std::ostream& (&p)(std::ostream&))
      {
        os << p; return *this;
      }
      direct_reporter& operator<<(std::ios& (&p)(std::ios&))
      {
        os << p; return *this;
      }
      direct_reporter& operator<<(std::ios_base& (&p)(std::ios_base&))
      {
        os << p; return *this;
      }
      ~direct_reporter();
    private:
      direct_reporter(const direct_reporter &);
      direct_reporter& operator=(const direct_reporter&);
      size_t heap_limit;
      std::ostringstream os;
    };

  } // namespace comm

  namespace heap {
    const size_t system = ~size_t();
    size_t allocated_bytes();
    size_t allocated_objects();
    size_t set_limit(size_t n); // must be higher than allocated_bytes()

    class control {
    public:
      static bool is_enabled() { return enabled; }
    private:
      static void enable();
      friend class ::crpcut::test_case_factory;
      static bool enabled;
    };

    struct mem_list_element
    {
      mem_list_element        *next;
      mem_list_element        *prev;
      mem_list_element        *stack;
      size_t                   stack_size;
      size_t                   mem;
      int                      type;
    };

    class local_root : public mem_list_element
    {
    public:
      typedef const local_root *bool_test;
      local_root(comm::type type, const char *file, size_t line);
      local_root(const local_root&);
      ~local_root();
      operator bool_test() const { return 0; }
      void nonsense_func() const {}
      void insert_object(mem_list_element *p);
      void remove_object(mem_list_element *p);
      static local_root* current();
    private:
      local_root();
      void assert_empty() const;
      local_root& operator=(const local_root&);

      char      const * const file_;
      size_t            const line_;
      mutable local_root     *old_root_;
      comm::type        const check_type_;

      static local_root      *current_root;
    };

  }

  template <comm::type>
  struct crpcut_check_name
  {
    static const char *string();
  };



  struct namespace_info
  {
  public:
    namespace_info(const char *n, namespace_info *p);
    const char* match_name(const char *n) const;
    // returns 0 on mismatch, otherwise a pointer into n where namespace name
    // ended.

    std::size_t full_name_len() const;
    friend std::ostream &operator<<(std::ostream &, const namespace_info &);
  private:
    const char *name;
    namespace_info *parent;
  };

  template <typename T>
  class poll;

  class fdreader
  {
  public:
    bool read(bool do_reply);
    crpcut_test_case_registrator *get_registrator() const;
    virtual void close();
    void unregister();
    ~fdreader() { }
  protected:
    fdreader(crpcut_test_case_registrator *r, int fd = 0);
    void set_fd(int fd, poll<fdreader> *poller);
    crpcut_test_case_registrator *const reg_;
  private:
    virtual bool do_read(int fd, bool do_reply) = 0;
    int fd_;
    poll<fdreader> *poller_;
  };

  template <comm::type t>
  class reader : public fdreader
  {
  public:
    reader(crpcut_test_case_registrator *r, int fd = 0);
    using fdreader::set_fd;
  private:
    virtual bool do_read(int fd, bool do_reply);
  };

  class report_reader : public fdreader
  {
  public:
    report_reader(crpcut_test_case_registrator *r);
    virtual void close();
    void set_fds(int in_fd, int out_fd, poll<fdreader> *read_poller);
  private:
    virtual bool do_read(int fd, bool do_reply);
    int response_fd;
  };

  class test_suite_base;
  class crpcut_test_case_registrator
    : public virtual policies::deaths::crpcut_none,
      public virtual policies::dependencies::crpcut_base
  {
    friend class test_suite_base;
  public:
    crpcut_test_case_registrator(const char *name, const namespace_info &ns,
                                 unsigned long cputime_limit_ms);
    friend std::ostream &operator<<(std::ostream &os,
                                    const crpcut_test_case_registrator &t)
    {
      return t.crpcut_print_name(os);
    }
    std::size_t crpcut_full_name_len() const;
    bool crpcut_match_name(const char *name) const;
    void crpcut_setup(poll<fdreader> &poller,
                      pid_t pid,
                      int in_fd, int out_fd,
                      int stdout_fd,
                      int stderr_fd);
    void crpcut_manage_death();
    crpcut_test_case_registrator *crpcut_unlink();
    void crpcut_link_after(crpcut_test_case_registrator*);
    void crpcut_kill();
    unsigned long crpcut_ms_until_deadline() const;
    void crpcut_clear_deadline();
    bool crpcut_deadline_is_set() const;
    static bool crpcut_timeout_compare(const crpcut_test_case_registrator *lh,
                                       const crpcut_test_case_registrator *rh);
    void crpcut_unregister_fds();
    crpcut_test_case_registrator *crpcut_get_next() const;
    void crpcut_set_wd(unsigned n);
    void crpcut_goto_wd() const;
    pid_t crpcut_get_pid() const;
    test_phase crpcut_get_phase() const;
    bool crpcut_has_active_readers() const;
    void crpcut_deactivate_reader();
    void crpcut_activate_reader();
    void crpcut_set_timeout(unsigned long);
    void crpcut_run_test_case();
    virtual tag& crpcut_tag() const = 0;
  protected:
    virtual void crpcut_do_run_test_case() = 0;
    crpcut_test_case_registrator();
    void crpcut_manage_test_case_execution(test_case_base*);
    void crpcut_prepare_destruction(unsigned long ms);
    void crpcut_prepare_construction(unsigned long ms);
  private:
    bool crpcut_cputime_timeout(unsigned long ms) const;
    std::ostream &crpcut_print_name(std::ostream &) const ;

    const char                   *crpcut_name_;
    const namespace_info         *crpcut_ns_info;
    crpcut_test_case_registrator *crpcut_next;
    crpcut_test_case_registrator *crpcut_prev;
    crpcut_test_case_registrator *crpcut_suite_list;
    unsigned                      crpcut_active_readers;
    bool                          crpcut_killed;
    bool                          crpcut_death_note;
    bool                          crpcut_deadline_set;
    pid_t                         crpcut_pid_;
    unsigned long                 crpcut_absolute_deadline_ms;
    struct timeval                crpcut_cpu_time_at_start;
    unsigned                      crpcut_dirnum;
    report_reader                 crpcut_rep_reader;
    reader<comm::stdout>          crpcut_stdout_reader;
    reader<comm::stderr>          crpcut_stderr_reader;
    test_phase                    crpcut_phase;
    const unsigned long           crpcut_cputime_limit_ms;
    friend class report_reader;
  };

  class test_case_factory
  {
  public:
    static void set_charset(const char *set_name);
    static const char *get_charset();
    static const char *get_output_charset();
    static const char *get_illegal_rep();
    static const unsigned max_parallel = 8;

    static int run_test(int argc, char *argv[],
                        std::ostream &os = std::cerr);
    static int run_test(int argc, const char *argv[],
                        std::ostream &os = std::cerr);
    static void introduce_name(pid_t pid, const char *name, size_t len);
    static void present(pid_t pid, comm::type t, test_phase phase,
                        size_t len, const char *buff);
    static bool tests_as_child_procs();
    static bool timeouts_enabled();
    static bool is_backtrace_enabled();
    static void set_deadline(crpcut_test_case_registrator *i);
    static void clear_deadline(crpcut_test_case_registrator *i);
    static void return_dir(unsigned num);
    static const char *get_working_dir();
    static void test_succeeded(crpcut_test_case_registrator*);
    static const char *get_start_dir();
    static const char *get_parameter(const char *name);
    static bool is_naughty_child();
    static unsigned long calc_cputime(const struct timeval&);
    template <typename T>
    static void get_parameter(const char *name, T& t)
    {
      const char *v = get_parameter(name);
      if (v)
        {
          stream::iastream is(v);
          if (is >> t)
            {
              return;
            }
        }
      size_t len = 80 + wrapped::strlen(name) + (v ? wrapped::strlen(v) : 0);
      char *msg_str = static_cast<char*>(alloca(len));
      stream::oastream msg(msg_str, msg_str + len);
      msg << "Parameter " << name << " with ";
      if (v)
        {
          msg << "value \"" << v << "\"";
        }
      else
        {
          msg << "no value";
        }
      msg << " cannot be interpreted as desired type";
      comm::report(comm::exit_fail, msg);
    }
    template <typename T>
    static T get_parameter(const char *name)
    {
      T rv;
      get_parameter<T>(name, rv);
      return rv;
    }
  private:
    static test_case_factory& obj();
    test_case_factory();
    void kill_presenter_process();
    void manage_children(unsigned max_pending_children, poll<fdreader> &poller);
    void start_test(crpcut_test_case_registrator *i, poll<fdreader> &poller);

    int do_run(int argc, const char *argv[], std::ostream &os);
    void do_present(pid_t pid, comm::type t, test_phase phase,
                    size_t len, const char *buff);
    void do_introduce_name(pid_t pid, const char *name, size_t len);
    void do_set_deadline(crpcut_test_case_registrator *i);
    void do_clear_deadline(crpcut_test_case_registrator *i);
    void do_return_dir(unsigned num);
    const char *do_get_working_dir() const;
    const char *do_get_start_dir() const;
    const char *do_get_parameter(const char *name) const;
    void do_set_charset(const char *set_name);
    const char *do_get_charset() const;
    const char *do_get_output_charset() const;
    const char *do_get_illegal_rep() const;
    unsigned long do_calc_cputime(const struct timeval&);
    friend class crpcut_test_case_registrator;

    class registrator_list : public crpcut_test_case_registrator
    {
      virtual bool match_name(const char *) const { return false; }
      virtual std::ostream& print_name(std::ostream &os) const { return os; }
      virtual void crpcut_do_run_test_case() {}
      virtual tag& crpcut_tag() const { return crpcut_tag_info<crpcut::crpcut_none>::obj(); }
    };

    typedef datatypes::array_v<crpcut_test_case_registrator*,
                               max_parallel> timeout_queue;


    struct timeval   accumulated_cputime;
    pid_t            current_pid;
    registrator_list reg;
    unsigned         pending_children;
    bool             verbose_mode;
    bool             enable_timeouts;
    bool             nodeps;
    unsigned         num_parallel;
    bool             single_process;
    bool             backtrace_enabled;
    unsigned         num_registered_tests;
    unsigned         num_selected_tests;
    unsigned         num_tests_run;
    unsigned         num_successful_tests;
    int              presenter_pipe;
    timeout_queue    deadlines;
    unsigned         working_dirs[max_parallel];
    unsigned         first_free_working_dir;
    char             dirbase[PATH_MAX];
    char             homedir[PATH_MAX];
    const char *     charset;
    const char *     output_charset;
    const char *     illegal_rep;
    const char **    argv;
  };

  template <comm::type t>
  bool reader<t>::do_read(int fd, bool)
  {
    static char buff[1024];
    for (;;)
      {
        ssize_t rv = wrapped::read(fd, buff, sizeof(buff));
        if (rv == 0) return false;
        if (rv == -1)
          {
            int n = errno;
            assert(n == EINTR);
            (void)n; // silence warning
          }

        test_case_factory::present(reg_->crpcut_get_pid(), t,
                                   reg_->crpcut_get_phase(),
                                   size_t(rv), buff);
        return true;
      }
  }

  template <typename C>
  struct test_wrapper
  {
    static void run(test_case_base *t);
  };

  template <typename exc>
  struct test_wrapper<policies::exception_wrapper<exc> >
  {
    static void run(test_case_base* t)
    {
      try {
        t->test();
      }
      catch (exc&) {
        return;
      }
      catch (...)
        {
          std::string s = "Unexpectedly caught "
            + policies::crpcut_exception_translator::try_all();
          size_t length = s.length();
          char *buff = static_cast<char*>(alloca(length));
          s.copy(buff, length);
          std::string().swap(s);
          comm::report(comm::exit_fail, buff, length);
        }
      comm::report(comm::exit_fail,
                   "Unexpectedly did not throw");
    }
  };
  template <typename T,
            size_t size_limit = sizeof(T),
            bool b = stream_checker::is_output_streamable<T>::value>
  struct conditional_streamer
  {
    static void stream(std::ostream &os, const T& t)
    {
      os << t;
    }
  };

  template <typename T, size_t size_limit>
  struct conditional_streamer<T, size_limit, false>
  {
    static void stream(std::ostream &os, const T& t)
    {
      if (sizeof(T) > size_limit)
        {
          os << '?';
          return;
        }
      static const char lf[] = "\n    ";
      const size_t bytes = sizeof(T);
      os << bytes << "-byte object <";
      if (bytes > 8) os << lf;
      const char *p = static_cast<const char *>(static_cast<const void*>(&t));
      char old_fill = os.fill();
      std::ios_base::fmtflags old_flags = os.flags();
      os   << std::setfill('0') ;
      size_t n = 0;
      for (; n < sizeof(T); ++n)
        {
          os << std::hex << std::setw(2)
             << (static_cast<unsigned>(p[n]) & 0xff);
          if ((n & 15) == 15)
            {
              os << lf;
            }
          else if ((n & 3) == 3 && n != bytes - 1)
            {
              os << "  ";
            }
          else if ((n & 1) == 1 && n != bytes - 1)
            {
              os << ' ';
            }
        }
      if (bytes > 8 && (n & 15) != 0)
        {
          os << lf;
        }
      os.flags(old_flags);
      os.fill(old_fill);
      os  << '>';
    }
  };

  template <typename T>
  void conditionally_stream(std::ostream &os, const T& t)
  {
    conditional_streamer<T, sizeof(T)>::stream(os, t);
  }

  template <size_t N, typename T>
  void conditionally_stream(std::ostream &os, const T& t)
  {
    conditional_streamer<T, N>::stream(os, t);
  }

  class null_cmp
  {
    class secret;
  public:
    static char func(secret*);
    static char (&func(...))[2];
    template <typename T>
    static char (&func(T*))[2];
  };

  template <typename T>
  class is_struct // or class or union
  {
    template <typename U>
    static char check_member(double U::*);
    template <typename U>
    static char (&check_member(...))[2];
  public:
    static const bool value = (sizeof(check_member<T>(0)) == 1);
  };

  template <bool b, typename T1, typename T2>
  struct if_else
  {
    typedef T1 type;
  };

  template <typename T1, typename T2>
  struct if_else<false, T1, T2>
  {
    typedef T2 type;
  };

  template <typename T>
  struct param_traits
  {
    typedef typename if_else<is_struct<T>::value, const  T&, T>::type type;
  };

  template <typename T>
  struct param_traits<const T>
  {
    typedef typename param_traits<T>::type type;
  };

  template <typename T>
  struct param_traits<volatile T>
  {
    typedef typename param_traits<T>::type type;
  };

  template <typename T>
  struct param_traits<const volatile T>
  {
    typedef typename param_traits<T>::type type;
  };

  template <typename T, size_t N>
  struct param_traits<T[N]>
  {
    typedef T *type;
  };

  template <typename T, size_t N>
  struct param_traits<const T[N]>
  {
    typedef const T *type;
  };

  template <typename T, size_t N>
  struct param_traits<volatile T[N]>
  {
    typedef volatile T *type;
  };

  template <typename T, size_t N>
  struct param_traits<const volatile T[N]>
  {
    typedef volatile const T *type;
  };

  template <typename T>
  struct param_traits<T&>
  {
    typedef typename param_traits<T>::type type;
  };

  template <typename T>
  bool stream_param(std::ostream &os,
                    const char *prefix,
                    const char *name, const T& t)
  {
    std::ostringstream tmp;
    conditionally_stream(tmp, t);
    std::string str = tmp.str();
    if (str != name)
      {
        os << prefix << name << " = " << str;
        return true;
      }
    return false;
  }

  class tester_base
  {
  protected:
    tester_base(const char *loc, const char *ops)
      : location(loc), op(ops)
    {
    }
    template <comm::type action, typename T1, typename T2>
    void verify(bool b, T1 v1, const char *n1, T2 v2, const char *n2) const
    {
      if (!b)
        {
          heap::set_limit(heap::system);
          using std::ostringstream;
          ostringstream os;
          os << location
             << "\n" << crpcut_check_name<action>::string()
             << "_" << op << "(" << n1 << ", " << n2 << ")";

          static const char *prefix[] = { "\n  where ", "\n        " };
          bool prev = stream_param(os, prefix[0], n1, v1);
          stream_param(os, prefix[prev], n2, v2);
          std::string s(os.str());
          os.str(std::string());
          size_t len = s.length();
          char *p = static_cast<char *>(alloca(len));
          s.copy(p, len);
          std::string().swap(s);
          os.~ostringstream();
          new (&os) ostringstream();
          comm::report(action, p, len);
        }
      }
  private:
    const char *location;
    const char *op;
  };

  template <comm::type action, typename T1, typename T2>
  class tester_t : tester_base
  {
    typedef typename param_traits<T1>::type type1;
    typedef typename param_traits<T2>::type type2;
  public:
    tester_t(const char *loc, const char *ops) : tester_base(loc, ops) {}
    void EQ(type1 v1, const char *n1, type2 v2, const char *n2) const
    {
      verify<action, type1, type2>(v1 == v2, v1, n1, v2, n2);
    }
    void NE(type1 v1, const char *n1, type2 v2, const char *n2) const
    {
      verify<action, type1, type2>(v1 != v2, v1, n1, v2, n2);
    }
    void GT(type1 v1, const char *n1, type2 v2, const char *n2) const
    {
      verify<action, type1, type2>(v1 > v2, v1, n1, v2, n2);
    }
    void GE(type1 v1, const char *n1, type2 v2, const char *n2) const
    {
      verify<action, type1, type2>(v1 >= v2, v1, n1, v2, n2);
    }
    void LT(type1 v1, const char *n1, type2 v2, const char *n2) const
    {
      verify<action, type1, type2>(v1 < v2, v1, n1, v2, n2);
    }
    void LE(type1 v1, const char *n1, type2 v2, const char *n2) const
    {
      verify<action, type1, type2>(v1 <= v2, v1, n1, v2, n2);
    }
  };

  template <comm::type action, typename T1>
  class tester_t<action, T1, void> : tester_base
  {
    typedef typename param_traits<T1>::type type1;
  public:
    tester_t(const char *loc, const char *ops) : tester_base(loc, ops) {}
    template <typename T2>
    void EQ(type1 v1, const char *n1, T2 v2, const char *n2) const
    {
      verify<action, type1, T2>(v1 == 0, v1, n1, v2, n2);
    }
    template <typename T2>
    void NE(type1 v1, const char *n1, T2 v2, const char *n2) const
    {
      verify<action, type1, T2>(v1 != 0, v1, n1, v2, n2);
    }
    template <typename T2>
    void GT(type1 v1, const char *n1, T2 v2, const char *n2) const
    {
      verify<action, type1, T2>(v1 > 0, v1, n1, v2, n2);
    }
    template <typename T2>
    void GE(type1 v1, const char *n1, T2 v2, const char *n2) const
    {
      verify<action, type1, T2>(v1 >= 0, v1, n1, v2, n2);
    }
    template <typename T2>
    void LT(type1 v1, const char *n1, T2 v2, const char *n2) const
    {
      verify<action, type1, T2>(v1 < 0, v1, n1, v2, n2);
    }
    template <typename T2>
    void LE(type1 v1, const char *n1, T2 v2, const char *n2) const
    {
      verify<action, type1, T2>(v1 <= 0, v1, n1, v2, n2);
    }
  };

  template <comm::type action, typename T2>
  class tester_t<action, void, T2> : tester_base
  {
    typedef typename param_traits<T2>::type type2;
  public:
    tester_t(const char *loc, const char *ops) : tester_base(loc, ops) {}
    template <typename T1>
    void EQ(T1 v1, const char *n1, type2 v2, const char *n2) const
    {
      verify<action, T1, type2>(0 == v2, v1, n1, v2, n2);
    }
    template <typename T1>
    void NE(T1 v1, const char *n1, type2 v2, const char *n2) const
    {
      verify<action, T1, type2>(0 != v2, v1, n1, v2, n2);
    }
    template <typename T1>
    void GT(T1 v1, const char *n1, type2 v2, const char *n2) const
    {
      verify<action, T1, type2>(0 > v2, v1, n1, v2, n2);
    }
    template <typename T1>
    void GE(T1 v1, const char *n1, type2 v2, const char *n2) const
    {
      verify<action, T1, type2>(0 >= v2, v1, n1, v2, n2);
    }
    template <typename T1>
    void LT(T1 v1, const char *n1, type2 v2, const char *n2) const
    {
      verify<action, T1, type2>(0 < v2, v1, n1, v2, n2);
    }
    template <typename T1>
    void LE(T1 v1, const char *n1, type2 v2, const char *n2) const
    {
      verify<action, T1, type2>(0 <= v2, v1, n1, v2, n2);
    }
  };

  template <comm::type action>
  class tester_t<action, void, void> /* pretty bizarre */ : tester_base
  {
  public:
    void EQ(int, const char*,int, const char*) const { }
    void NE(int, const char *n1, int, const char *n2) const
    {
      verify<action, int,int>(false, 0, n1, 0, n2);
    }
    void GT(int, const char *n1, int, const char *n2) const
    {
      verify<action, int,int>(false, 0, n1, 0, n2);
    }
    void GE(int, const char*, int, const char *) const { }
    void LT(int, const char *n1, int, const char *n2) const
    {
      verify<action, int,int>(false, 0, n1, 0, n2);
    }
    void LE(int, const char*, int, const char*) const { }
  };

  template <comm::type action, bool null1, typename T1, bool null2, typename T2>
  tester_t<action,
           typename if_else<null1, void, T1>::type,
           typename if_else<null2, void, T2>::type>
  tester(const char *loc, const char *op)
  {
    tester_t<action,
             typename if_else<null1, void, T1>::type,
             typename if_else<null2, void, T2>::type> v(loc, op);
    return v;
  }

  template <comm::type action>
  class bool_tester
  {
    const char *loc_;
  public:
    bool_tester(const char *loc) : loc_(loc) {}
    template <typename T>
    void check_true(const T& v, const char *vn) const
    {
      if (eval(v)) {} else { report("_TRUE", v, vn); }
    }
    template <typename T>
    void assert_false(const T& v, const char *vn) const
    {
      if (eval(v)) { report("_FALSE", v, vn); }
    }
  private:
    template <typename T>
    void report(const char *name, const T& v, const char *vn) const
    {
      heap::set_limit(heap::system);
      using std::ostringstream;
      using std::stringbuf;
      ostringstream os;

      os << loc_ << "\n" << crpcut_check_name<action>::string()
         << name << "(" << vn << ")\n"
        "  is evaluated as:\n    ";
      conditionally_stream<8>(os, v);
      std::string s(os.str());
      os.~ostringstream();
      new (&os) ostringstream();
      size_t len = s.length();
      char *p = static_cast<char*>(alloca(len));
      s.copy(p, len);
      std::string().swap(s);
      comm::report(action, p, len);
    }
  };

  template <case_convert_type converter>
  struct convert_traits
  {
    static const char *do_convert(char *lo, const char *, const std::locale &)
    {
      return lo;
    }
  };

  template <>
  const char *
  convert_traits<uppercase>
  ::do_convert(char *lo, const char *, const std::locale &);

  template <>
  const char *
  convert_traits<lowercase>
  ::do_convert(char *lo, const char *, const std::locale &);


  class collate_result
  {
    class comparator;
    const char *r;
    const std::string intl;
    std::locale locale;
    enum { left, right } side;
    collate_result(const char *refstr, std::string comp, const std::locale& l);
  public:
    collate_result(const collate_result& o);
    operator const comparator*() const;
    const comparator* operator()() const;
    collate_result& set_lh();
    friend
    std::ostream &operator<<(std::ostream& os, const collate_result &obj);

    template <case_convert_type>
    friend class crpcut::collate_t;
  };

#ifdef CRPCUT_SUPPORTS_VTEMPLATES
  template <int N, typename ...T>
  class param_holder;

  template <int N>
  class param_holder<N>
  {
  public:
    param_holder() {}
    template <typename P, typename ...V>
    bool apply(P& func, const V &...v) const {
      return func(v...);
    }
    void print_to(std::ostream &) const { }
  };

  template <int N, typename T, typename ...Tail>
  class param_holder<N, T, Tail...> : public param_holder<N+1, Tail...>
  {
  public:
    param_holder(const T& v, const Tail&...tail)
      : param_holder<N+1,Tail...>(tail...),
        val(v)
    {
    }
    template <typename P, typename ...V>
    bool apply(P& func, const V&...v) const {
      return param_holder<N+1,Tail...>::apply(func, v..., val);
    }
    void print_to(std::ostream &os) const {
      os << "  param" << N << " = " << val << '\n';
      param_holder<N+1, Tail...>::print_to(os);
    }
  private:
    const T& val;
  };

  template <typename ...T>
  param_holder<1, T...> params(const T&... v)
  {
    return param_holder<1, T...>(v...);
  }

#else

  template <int N, typename T>
  class holder
  {
  protected:
    holder(const T& v) : val(v) {}
    const T& getval() const { return val; }
    void print_to(std::ostream &os) const
    {
      os << "  param" << N << " = " << val << "\n";
    }
  private:
    const T &val;
  };

  template <int N>
  class holder<N, crpcut_none> : private crpcut_none
  {
  protected:
    holder(const crpcut_none&) {}
    void print_to(std::ostream&) const {};
    const crpcut_none& getval() const { return *this; }
  };

  template <typename T1,               typename T2 = crpcut_none,
            typename T3 = crpcut_none, typename T4 = crpcut_none,
            typename T5 = crpcut_none, typename T6 = crpcut_none,
            typename T7 = crpcut_none, typename T8 = crpcut_none,
            typename T9 = crpcut_none>
  class param_holder
    : holder<1, T1>, holder<2, T2>, holder<3, T3>,
      holder<4, T4>, holder<5, T5>, holder<6, T6>,
      holder<7, T7>, holder<8, T8>, holder<9, T9>
  {
  public:
    param_holder(const T1 &v1, const T2 &v2 = T2(), const T3 &v3 = T3(),
                 const T4 &v4 = T4(), const T5 &v5 = T5(),
                 const T6 &v6 = T6(), const T7 &v7 = T7(),
                 const T8 &v8 = T8(), const T9 &v9 = T9())
      : holder<1, T1>(v1),
        holder<2, T2>(v2),
        holder<3, T3>(v3),
        holder<4, T4>(v4),
        holder<5, T5>(v5),
        holder<6, T6>(v6),
        holder<7, T7>(v7),
        holder<8, T8>(v8),
        holder<9, T9>(v9)
    {}
    template <typename P>
    bool apply(P &pred) const;
    void print_to(std::ostream &os) const
    {
      holder<1, T1>::print_to(os);
      holder<2, T2>::print_to(os);
      holder<3, T3>::print_to(os);
      holder<4, T4>::print_to(os);
      holder<5, T5>::print_to(os);
      holder<6, T6>::print_to(os);
      holder<7, T7>::print_to(os);
      holder<8, T8>::print_to(os);
      holder<9, T9>::print_to(os);
    }
  };

  template <typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6,
            typename T7, typename T8, typename T9>
  struct call_traits
  {
    template <typename P>
    static bool call(P &p,
                     const T1 &t1, const T2 &t2, const T3 &t3,
                     const T4 &t4, const T5 &t5, const T6 &t6,
                     const T7 &t7, const T8 &t8, const T9 &t9)
    {
      return p(t1, t2, t3, t4, t5, t6, t7, t8, t9);
    }
  };

  template <typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6,
            typename T7, typename T8>
  struct call_traits<T1, T2, T3, T4, T5, T6, T7, T8, crpcut_none>
  {
    template <typename P>
    static bool call(P &p,
                     const T1 &t1, const T2 &t2, const T3 &t3,
                     const T4 &t4, const T5 &t5, const T6 &t6,
                     const T7 &t7, const T8 &t8, const crpcut_none&)
    {
      return p(t1, t2, t3, t4, t5, t6, t7, t8);
    }
  };

  template <typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6,
            typename T7>
  struct call_traits<T1, T2, T3, T4, T5, T6, T7, crpcut_none, crpcut_none>
  {
    template <typename P>
    static bool call(P &p,
                     const T1 &t1, const T2 &t2, const T3 &t3,
                     const T4 &t4, const T5 &t5, const T6 &t6,
                     const T7 &t7, const crpcut_none&, const crpcut_none&)
    {
      return p(t1, t2, t3, t4, t5, t6, t7);
    }
  };

  template <typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6>
  struct call_traits<T1, T2, T3, T4, T5, T6,
                     crpcut_none, crpcut_none, crpcut_none>
  {
    template <typename P>
    static bool call(P &p,
                     const T1 &t1, const T2 &t2, const T3 &t3,
                     const T4 &t4, const T5 &t5, const T6 &t6,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&)
    {
      return p(t1, t2, t3, t4, t5, t6);
    }
  };

  template <typename T1, typename T2, typename T3,
            typename T4, typename T5>
  struct call_traits<T1, T2, T3, T4, T5,
                     crpcut_none, crpcut_none, crpcut_none, crpcut_none>
  {
    template <typename P>
    static bool call(P &p, const T1& t1, const T2 &t2, const T3 &t3,
                     const T4 &t4, const T5 &t5, const crpcut_none&,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&)
    {
      return p(t1, t2, t3, t4, t5);
    }
  };

  template <typename T1, typename T2, typename T3,
            typename T4>
  struct call_traits<T1, T2, T3, T4,
                     crpcut_none, crpcut_none, crpcut_none,
                     crpcut_none, crpcut_none>
  {
    template <typename P>
    static bool call(P &p, const T1& t1, const T2 &t2, const T3 &t3,
                     const T4 &t4, const crpcut_none&, const crpcut_none&,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&)
    {
      return p(t1, t2, t3, t4);
    }
  };

  template <typename T1, typename T2, typename T3>
  struct call_traits<T1, T2, T3, crpcut_none,
                     crpcut_none, crpcut_none, crpcut_none, crpcut_none,
                     crpcut_none>
  {
    template <typename P>
    static bool call(P &p, const T1& t1, const T2 &t2, const T3 &t3,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&, const crpcut_none&)
    {
      return p(t1, t2, t3);
    }
  };

  template <typename T1, typename T2>
  struct call_traits<T1, T2, crpcut_none, crpcut_none, crpcut_none, crpcut_none, crpcut_none, crpcut_none, crpcut_none>
  {
    template <typename P>
    static bool call(P &p, const T1& t1, const T2 &t2, const crpcut_none&,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&, const crpcut_none&)
    {
      return p(t1, t2);
    }
  };

  template <typename T1>
  struct call_traits<T1, crpcut_none, crpcut_none,
                     crpcut_none, crpcut_none, crpcut_none, crpcut_none,
                     crpcut_none, crpcut_none>
  {
    template <typename P>
    static bool call(P &p, const T1& t1, const crpcut_none&,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&)
    {
      return p(t1);
    }
  };

  template <>
  struct call_traits<crpcut_none, crpcut_none, crpcut_none, crpcut_none,
                     crpcut_none, crpcut_none, crpcut_none, crpcut_none,
                     crpcut_none>
  {
    template <typename P>
    static bool call(P &p,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&, const crpcut_none&,
                     const crpcut_none&)
    {
      return p();
    }
  };


  template <typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6,
            typename T7, typename T8, typename T9>
  template <typename P>
  bool
  param_holder<T1, T2, T3, T4, T5, T6, T7, T8, T9>::apply(P &pred) const
  {
    typedef call_traits<T1, T2, T3, T4, T5, T6, T7, T8, T9> traits;
    return traits::call(pred,
                        holder<1, T1>::getval(),
                        holder<2, T2>::getval(),
                        holder<3, T3>::getval(),
                        holder<4, T4>::getval(),
                        holder<5, T5>::getval(),
                        holder<6, T6>::getval(),
                        holder<7, T7>::getval(),
                        holder<8, T8>::getval(),
                        holder<9, T9>::getval());
  }

  template <typename T1>
  param_holder<T1>
  params(const T1& t1)
  {
    typedef param_holder<T1> R;
    return R(t1);
  }

  template <typename T1, typename T2>
  param_holder<T1, T2>
  params(const T1& t1, const T2 &t2)
  {
    typedef param_holder<T1, T2> R;
    return R(t1, t2);
  }

  template <typename T1, typename T2, typename T3>
  param_holder<T1, T2, T3>
  params(const T1& t1, const T2 &t2, const T3 &t3)
  {
    typedef param_holder<T1, T2, T3> R;
    return R(t1, t2, t3);
  }

  template <typename T1, typename T2, typename T3, typename T4>
  param_holder<T1, T2, T3, T4>
  params(const T1& t1, const T2 &t2, const T3 &t3, const T4 &t4)
  {
    typedef param_holder<T1, T2, T3, T4> R;
    return R(t1, t2, t3, t4);
  }

  template <typename T1, typename T2, typename T3, typename T4, typename T5>
  param_holder<T1, T2, T3, T4, T5>
  params(const T1& t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5)
  {
    typedef param_holder<T1, T2, T3, T4, T5> R;
    return R(t1, t2, t3, t4, t5);
  }

  template <typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6>
  param_holder<T1, T2, T3, T4, T5, T6>
  params(const T1& t1, const T2 &t2, const T3 &t3,
         const T4 &t4, const T5 &t5, const T6 &t6)
  {
    typedef param_holder<T1, T2, T3, T4, T5, T6> R;
    return R(t1, t2, t3, t4, t5, t6);
  }

  template <typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6,
            typename T7>
  param_holder<T1, T2, T3, T4, T5, T6, T7>
  params(const T1& t1, const T2 &t2, const T3 &t3,
         const T4 &t4, const T5 &t5, const T6 &t6,
         const T7 &t7)
  {
    typedef param_holder<T1, T2, T3, T4, T5, T6, T7> R;
    return R(t1, t2, t3, t4, t5, t6, t7);
  }

  template <typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6,
            typename T7, typename T8>
  param_holder<T1, T2, T3, T4, T5, T6, T7, T8>
  params(const T1& t1, const T2 &t2, const T3 &t3,
         const T4 &t4, const T5 &t5, const T6 &t6,
         const T7 &t7, const T8 &t8)
  {
    typedef param_holder<T1, T2, T3, T4, T5, T6, T7, T8> R;
    return R(t1, t2, t3, t4, t5, t6, t7, t8);
  }

  template <typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6,
            typename T7, typename T8, typename T9>
  param_holder<T1, T2, T3, T4, T5, T6, T7, T8, T9>
  params(const T1& t1, const T2 &t2, const T3 &t3,
         const T4 &t4, const T5 &t5, const T6 &t6,
         const T7 &t7, const T8 &t8, const T9 &t9)
  {
    typedef param_holder<T1, T2, T3, T4, T5, T6, T7, T8, T9> R;
    return R(t1, t2, t3, t4, t5, t6, t7, t8, t9);
  }

  inline
  param_holder<crpcut_none>
  params()
  {
    return param_holder<crpcut_none>(crpcut_none());
  }
#endif
  template <typename P,
            bool streamable = stream_checker::is_output_streamable<P>::value>
  struct predicate_streamer
  {
    predicate_streamer(const char *name, const P& pred) : n(name), p(pred) {}
    std::ostream &stream_to(std::ostream & os) const
    {
      return os << n << " :\n" << p << '\n';
    }
  private:
    const char *n;
    const P& p;
  };

  template <typename P>
  struct predicate_streamer<P, false>
  {
    predicate_streamer(const char *,const P&) {}
    std::ostream &stream_to(std::ostream &os) const { return os; }
  private:
  };

  template <typename P, bool unstreamable>
  std::ostream &operator<<(std::ostream &os,
                           const predicate_streamer<P, unstreamable>& s)
  {
    return s.stream_to(os);
  }

  template <typename P>
  predicate_streamer<P> stream_predicate(const char *n, const P& p)
  {
    return predicate_streamer<P>(n, p);
  }

  template <typename Pred, typename Params>
  bool
  match_pred(std::string &msg, const char *sp, Pred p, const Params &params)
  {
    bool b = params.apply(p);
    if (!b)
      {
        heap::set_limit(heap::system);
        std::ostringstream out;
        params.print_to(out);
        out << stream_predicate(sp, p);
        msg = out.str();
      }
    return b;
  }

  template <case_convert_type converter>
  class collate_t
  {
  public:
    collate_t(const std::string &reference,
              const std::locale &loc = std::locale())
      : ref(reference),
        locale(loc)
    {
      char *p = &*ref.begin();
      convert_traits<converter>::do_convert(p,
                                            p + ref.length(),
                                            loc);
    }
    collate_t(const char *reference, const std::locale& loc = std::locale())
      : ref(reference),
        locale(loc)
    {
      char *p = &*ref.begin();
      convert_traits<converter>::do_convert(p,
                                            p + ref.length(),
                                            loc);
    }

    collate_result operator<(const std::string &s) const
    {
      collate_result rv(compare(s) < 0
                        ? 0
                        : s.c_str(),
                        ref,
                        locale);
      return rv;
    }
    collate_result operator<(const char *r) const
    {
      collate_result rv(compare(r) < 0
                        ? 0
                        : r,
                        ref,
                        locale);
      return rv;
    }

    collate_result operator<=(const std::string &s) const
    {
      collate_result rv(compare(s) <= 0
                        ? 0
                        : s.c_str(),
                        ref,
                        locale);
      return rv;
    }
    collate_result operator<=(const char *r) const
    {
      collate_result rv(compare(r) <= 0
                        ? 0
                        : r,
                        ref,
                        locale);
      return rv;
    }

    collate_result operator>(const std::string &s) const
    {
      collate_result rv(compare(s) > 0
                        ? 0
                        : s.c_str(),
                        ref,
                        locale);
      return rv;
    }
    collate_result operator>(const char *r) const
    {
      collate_result rv(compare(r) > 0
                        ? 0
                        : r,
                        ref,
                        locale);
      return rv;
    }

    collate_result operator>=(const std::string &s) const
    {
      collate_result rv(compare(s) >= 0
                        ? 0
                        : s.c_str(),
                        ref,
                        locale);
      return rv;
    }
    collate_result operator>=(const char *r) const
    {
      collate_result rv(compare(r) >= 0
                        ? 0
                        : r,
                        ref,
                        locale);
      return rv;
    }

    collate_result operator==(const std::string &s) const
    {
      collate_result rv(compare(s) == 0
                        ? 0
                        : s.c_str(),
                        ref,
                        locale);
      return rv;
    }
    collate_result operator==(const char *r) const
    {
      collate_result rv(compare(r) == 0
                        ? 0
                        : r,
                        ref,
                        locale);
      return rv;
    }

    collate_result operator!=(const std::string &s) const
    {
      collate_result rv(compare(s) != 0
                        ? 0
                        : s.c_str(),
                        ref,
                        locale);
      return rv;
    }
    collate_result operator!=(const char *r) const
    {
      collate_result rv(compare(r) != 0
                        ? 0
                        : r,
                        ref,
                        locale);
      return rv;
    }
  private:
    int compare(std::string s) const
    {
      char *begin = &*s.begin();
      char *end = begin + s.length();
      convert_traits<converter>::do_convert(begin,
                                            end,
                                            locale);
      typedef std::collate<char> coll;
      const coll &fac = std::use_facet<coll>(locale);
      return fac.compare(ref.c_str(), ref.c_str() + ref.length(),
                         begin, end);
    }
    int compare(const char *p) const
    {
      return compare(std::string(p));
    }
    int compare(const char *p, size_t len) const
    {
      return compare(std::string(p, len));
    }
    std::string locale_name() const { return locale.name(); }
    const char *reference_string() const { return ref.c_str(); }
  private:
    std::string ref;
    std::locale locale;
  };


  //// template and inline func implementations

  namespace datatypes {

    template <typename T, std::size_t N>
    array_v<T, N>::array_v()
      : num_elements(0)
    {
    }

    template <typename T, std::size_t N>
    typename array_v<T, N>::iterator
    array_v<T, N>::end()
    {
      return iterator(&operator[](size() - 1) + 1);
    }

    template <typename T, std::size_t N>
    typename array_v<T, N>::const_iterator
    array_v<T, N>::end() const
    {
      return iterator(&operator[](size() - 1) + 1);
    }

    template <typename T, std::size_t N>
    typename array_v<T, N>::reverse_iterator
    array_v<T, N>::rbegin()
    {
      return reverse_iterator(end());
    }

    template <typename T, std::size_t N>
    typename array_v<T, N>::const_reverse_iterator
    array_v<T, N>::rbegin() const
    {
      return reverse_iterator(end());
    }

    template <typename T, std::size_t N>
    typename array_v<T, N>::size_type
    array_v<T, N>::size() const
    {
      return num_elements;
    }

    template <typename T, std::size_t N>
    bool
    array_v<T, N>::empty() const
    {
      return size() == 0;
    }

    template <typename T, std::size_t N>
    typename array_v<T, N>::reference
    array_v<T, N>::at(size_type n)
    {
      if (n >= num_elements) throw std::out_of_range("array_v::at");
      return operator[](n);
    }

    template <typename T, std::size_t N>
    typename array_v<T, N>::const_reference
    array_v<T, N>::at(size_type n) const
    {
      if (n >= num_elements) throw std::out_of_range("array_v::at");
      return operator[](n);
    }

    template <typename T, std::size_t N>
    typename array_v<T, N>::reference
    array_v<T, N>::back()
    {
      return *(end() - !empty());
    }

    template <typename T, std::size_t N>
    typename array_v<T, N>::const_reference
    array_v<T, N>::back() const
    {
      return *(end() - !empty());
    }

    template <typename T, std::size_t N>
    void
    array_v<T, N>::push_back(const value_type &x)
    {
      assert(num_elements <= N);
      operator[](num_elements++) = x;
    }

    template <typename T, std::size_t N>
    void
    array_v<T, N>::pop_back()
    {
      assert(num_elements);
      --num_elements;
    }

    template <std::size_t N>
    struct fp_rep;

#define CRPCUT_MAKE_FP_REP(x)                   \
    template <>                                 \
    struct fp_rep<sizeof(x)>                    \
    {                                           \
      typedef x type;                           \
    }

    CRPCUT_MAKE_FP_REP(uint32_t);
    CRPCUT_MAKE_FP_REP(uint64_t);

#undef CRPCUT_MAKE_FP_REP

    template <typename T>
    union fp
    {
      T data;
      typename fp_rep<sizeof(T)>::type rep;
    };

  } // namespace datatypes


  namespace policies {

    namespace deaths{


      template <int N, typename action>
      bool
      signal<N, action>::crpcut_is_expected_signal(int code) const
      {
        return N == ANY_CODE || code == N;
      }

      template <int N, typename action>
      void
      signal<N, action>::crpcut_expected_death(std::ostream &os) const
      {
        if (N == ANY_CODE)
          {
            os << "any signal";
          }
        else
          {
            os << "signal " << N;
          }
      }

      template <int N, typename action>
      bool
      exit<N, action>::crpcut_is_expected_exit(int code) const
      {
        return N == ANY_CODE || code == N;
      }

      template <int N, typename action>
      void
      exit<N, action>::crpcut_expected_death(std::ostream &os) const
      {
        if (N == ANY_CODE)
          {
            os << "exit with any code";
          }
        else
          {
            os << "exit with code " << N;
          }
      }


      template <unsigned long N>
      bool
      timeout<N>::crpcut_is_expected_signal(int code) const
      {
        return
          !test_case_factory::timeouts_enabled()
          || code == SIGKILL
          || code == SIGXCPU;
      }

      template <unsigned long N>
      void
      timeout<N>::crpcut_expected_death(std::ostream &os) const
      {
        os << N << "ms realtime timeout";
      }

      template <unsigned long N>
      unsigned long
      timeout<N>::crpcut_calc_deadline(unsigned long ts) const
      {
        return ts;
      }

    } // namespace deaths

    namespace dependencies {


      template <typename T>
      enforcer<T>::enforcer()
      {
        T::crpcut_reg().crpcut_add(this);
      }
    } // namespace dependencies

    namespace timeout {
      template <unsigned long timeout_ms>
      enforcer<realtime, timeout_ms>::enforcer()
        : monotonic_enforcer(timeout_ms)
      {
      }
    } // namespace timeout

  } // namespace policies


  namespace comm {


    template <size_t N>
    void
    reporter::operator()(type t, const stream::toastream<N> &os) const
    {
      const stream::oastream &os_(os);
      operator()(t, os_);
    }

    template <typename T>
    void
    reporter::write(const T& t) const
    {
      const size_t len = sizeof(T);
      size_t bytes_written = 0;
      const char *p = static_cast<const char*>(static_cast<const void*>(&t));
      while (bytes_written < len)
        {
          ssize_t rv = wrapped::write(write_fd,
                                      p + bytes_written,
                                      len - bytes_written);
          if (rv == -1 && errno == EINTR) continue;
          if (rv <= 0) throw "write failed";
          bytes_written += size_t(rv);
        }
    }

    template <typename T>
    void
    reporter::operator()(comm::type t, const T& data) const
    {
      assert(test_case_factory::tests_as_child_procs());
      write(t);
      size_t len = sizeof(data);
      write(len);
      write(data);
      read(len);
      assert(len == sizeof(data));
    }

    template <typename T>
    void
    reporter::read(T& t) const
    {
      const size_t len = sizeof(T);
      size_t bytes_read = 0;
      char *p = static_cast<char*>(static_cast<void*>(&t));
      while (bytes_read < len)
        {
          ssize_t rv = wrapped::read(read_fd,
                                     p + bytes_read,
                                     len - bytes_read);
          if (rv == -1 && errno == EINTR) continue;
          if (rv <= 0) {
            throw "read failed";
          }
          bytes_read += size_t(rv);
        }
    }

    template <comm::type type>
    direct_reporter<type>::direct_reporter()
      : heap_limit(heap::set_limit(heap::system))
    {
    }

    template <comm::type t> template <typename V>
    direct_reporter<t>& direct_reporter<t>::operator<<(const V& v)
    {
      conditionally_stream(os, v);
      return *this;
    }

    template <comm::type t> template <typename V>
    direct_reporter<t>& direct_reporter<t>::operator<<(V& v)
    {
      conditionally_stream(os, v);
      return *this;
    }

    template <comm::type type>
    direct_reporter<type>::~direct_reporter()
    {
      using std::ostringstream;
      std::string s;
      os.str().swap(s);
      os.~ostringstream();
      new (&os) ostringstream(); // Just how ugly is this?
      size_t len = s.length();
      char *p = static_cast<char*>(alloca(len));
      s.copy(p, len);
      std::string().swap(s);
      heap::set_limit(heap_limit);
      report(type, p, len);
    }
  } // namespace comm

  template <comm::type t>
  inline
  reader<t>::reader(crpcut_test_case_registrator *r, int fd)
    : fdreader(r, fd)
  {
  }

  template <typename T, case_convert_type type>
  collate_result operator==(T r, const collate_t<type> &c)
  {
    return (c == r).set_lh();
  }

  template <typename T, case_convert_type type>
  collate_result operator!=(T r, const collate_t<type> &c)
  {
    return (c != r).set_lh();
  }

  template <typename T, case_convert_type type>
  collate_result operator<(T r, const collate_t<type> &c)
  {
    return (c > r).set_lh();
  }

  template <typename T, case_convert_type type>
  collate_result operator<=(T r, const collate_t<type> &c)
  {
    return (c >= r).set_lh();
  }

  template <typename T, case_convert_type type>
  collate_result operator>(T r, const collate_t<type> &c)
  {
    return (c < r).set_lh();
  }

  template <typename T, case_convert_type type>
  collate_result operator>=(T r, const collate_t<type> &c)
  {
    return (c <= r).set_lh();
  }


#ifdef CRPCUT_SUPPORTS_VTEMPLATES
  template <typename D, typename ...T>
  typename match_traits<D, T...>::type
  match(T... t)
  {
    return typename match_traits<D, T...>::type(t...);
  }
#else
  template <typename D, typename T>
  typename match_traits<D, T>::type
  match(T t)
  {
    typedef match_traits<D, T> traits;
    typename traits::type rv(t);
    return rv;
  }

  template <typename D, typename T1, typename T2>
  typename match_traits<D, T1, T2>::type
  match(T1 t1, T2 t2)
  {
    typedef match_traits<D, T1, T2> traits;
    typename traits::type rv(t1, t2);
    return rv;
  }

  template <typename D,
            typename T1, typename T2, typename T3>
  typename match_traits<D, T1, T2, T3>::type
  match(T1 t1, T2 t2, T3 t3)
  {
    typedef match_traits<D, T1, T2, T3> traits;
    typename traits::type rv(t1, t2, t3);
    return rv;
  }

  template <typename D,
            typename T1, typename T2, typename T3,
            typename T4>
  typename match_traits<D, T1, T2, T3, T4>::type
  match(T1 t1, T2 t2, T3 t3, T4 t4)
  {
    typedef match_traits<D, T1, T2, T3, T4> traits;
    typename traits::type rv(t1, t2, t3, t4);
    return rv;
  }

  template <typename D,
            typename T1, typename T2, typename T3,
            typename T4, typename T5>
  typename match_traits<D, T1, T2, T3, T4, T5>::type
  match(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
  {
    typedef match_traits<D, T1, T2, T3, T4, T5> traits;
    typename traits::type rv(t1, t2, t3, t4, t5);
    return rv;
  }

  template <typename D,
            typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6>
  typename match_traits<D, T1, T2, T3, T4, T5, T6>::type
  match(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
  {
    typedef match_traits<D, T1, T2, T3, T4, T5, T6> traits;
    typename traits::type rv(t1, t2, t3, t4, t5, t6);
    return rv;
  }

  template <typename D,
            typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6,
            typename T7>
  typename match_traits<D, T1, T2, T3, T4, T5, T6, T7>::type
  match(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
  {
    typedef match_traits<D, T1, T2, T3, T4, T5, T6, T7> traits;
    typename traits::type rv(t1, t2, t3, t4, t5, t6, t7);
    return rv;
  }

  template <typename D,
            typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6,
            typename T7, typename T8>
  typename match_traits<D, T1, T2, T3, T4, T5, T6, T7, T8>::type
  match(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
  {
    typedef match_traits<D, T1, T2, T3, T4, T5, T6, T7, T8> traits;
    typename traits::type rv(t1, t2, t3, t4, t5, t6, t7, t8);
    return rv;
  }

  template <typename D,
            typename T1, typename T2, typename T3,
            typename T4, typename T5, typename T6,
            typename T7, typename T8, typename T9>
  typename match_traits<D, T1, T2, T3, T4, T5, T6, T7, T8, T9>::type
  match(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9)
  {
    typedef match_traits<D, T1, T2, T3, T4, T5, T6, T7, T8, T9> traits;
    typename traits::type rv(t1, t2, t3, t4, t5, t6, t7, t8, t9);
    return rv;
  }
#endif
  class abs_diff
  {
  public:
    template <typename T>
    class type
    {
    public:
      type(T v) : t(v) {}
      template <typename U>
      bool operator()(U lh, U rh, T* = static_cast<U*>(0))
      {
        diff = lh-rh;
        if (diff < U()) diff = -diff;
        return diff <= t;
      }
      friend std::ostream& operator<<(std::ostream &os, const type<T>& obj)
      {
        os << "\n    Max allowed difference is "
           << std::setprecision(std::numeric_limits<T>::digits10)
           << obj.t
           << "\n    Actual difference is "
           << std::setprecision(std::numeric_limits<T>::digits10)
           << obj.diff;
        return os;
      }
    private:
      T t;
      T diff;
    };
  };

  template <typename T>
  struct match_traits<abs_diff, T>
  {
    typedef typename abs_diff::template type<T> type;
  };

  class relative_diff
  {
  public:
    template <typename T>
    class type
    {
    public:
      type(T v) : t(v) {}
      template <typename U>
      bool operator()(U lh, U rh, T* = static_cast<U*>(0))
      {
        U ldiff = lh - rh;
        if (ldiff < U()) ldiff = -ldiff;
        U lsum = lh + rh;
        if (lsum < U()) lsum = -lsum;
        diff = 2 * ldiff / lsum;
        return diff <= t;
      }
      friend std::ostream& operator<<(std::ostream &os, const type<T>& obj)
      {
        os << "\n    Max allowed relative difference is "
           << std::setprecision(std::numeric_limits<T>::digits10)
           << obj.t
           << "\n    Actual relative difference is "
           << obj.diff;
        return os;
      }
    private:
      T t;
      T diff;
    };
  };

  template <typename T>
  class long_double_not_supported {};

  template <>
  class long_double_not_supported<long double>;

  template <typename T, bool b = std::numeric_limits<T>::is_iec559>
  struct must_be_ieee754_fp_type : long_double_not_supported<T>
  {
    must_be_ieee754_fp_type(int) {}
  };

  template <typename T>
  struct must_be_ieee754_fp_type<T, false>;



  typedef enum { exclude_inf, include_inf } inf_in_ulps_diff;

  class ulps_diff
  {
  public:
    ulps_diff(unsigned max, inf_in_ulps_diff inf_val= exclude_inf)
      : max_diff(max),
        inf(inf_val)
    {
    }
    template <typename T>
    bool operator()(T lh,
                    T rh,
                    const must_be_ieee754_fp_type<T> & = 0)
    {
      typedef typename datatypes::fp_rep<sizeof(T)>::type rep;
      if (lh != lh) return false; // NaN
      if (rh != rh) return false; // NaN
      if (!inf && std::numeric_limits<T>::max() / lh == 0.0) return false;
      if (!inf && std::numeric_limits<T>::max() / rh == 0.0) return false;
      datatypes::fp<T> fl;
      datatypes::fp<T> fr;
      fl.data = lh;
      fr.data = rh;
      fl.rep = signbit2bias(fl.rep);
      fr.rep = signbit2bias(fr.rep);
      diff = (fl.rep > fr.rep ? fl.rep - fr.rep : fr.rep - fl.rep);
      return diff <= max_diff;
    }
    friend std::ostream& operator<<(std::ostream &os, const ulps_diff& d)
    {
      return os << "    Max allowed diff = " << d.max_diff
                << " ULPS\n    Actual diff = " << d.diff << " ULPS";
    }
  private:
    template <typename T>
    static T signbit2bias(T t)
    {
      static const T one = T() + 1;
      static const T neg_bit = one << (std::numeric_limits<T>::digits - 1);
      if (t & neg_bit)
        {
            return ~t + 1;
        }
      return t | neg_bit;
    }

    uint64_t         max_diff;
    inf_in_ulps_diff inf;
    uint64_t         diff;
  };
  template <typename T>
  struct match_traits<relative_diff, T>
  {
    typedef typename relative_diff::template type<T> type;
  };

  class regex
  {
    class type
    {
    public:
      template <typename T>
      type(T t, int flags)
        : errmsg(0)
      {
        int i = wrapped::regcomp(&r,
                                 datatypes::string_traits<T>::get_c_str(t),
                                 flags | REG_NOSUB);
        if (i != 0)
          {
            size_t n = wrapped::regerror(i, &r, 0, 0);
            errmsg = new char[n];
            wrapped::regerror(i, &r, errmsg, n);
          }
      }
      template <typename U>
      bool operator()(U t)
      {
        if (errmsg) return false;
        int i = wrapped::regexec(&r,
                                 datatypes::string_traits<U>::get_c_str(t),
                                 0,
                                 0,
                                 0);
        if (i != 0 && i != REG_NOMATCH)
          {
            size_t n = wrapped::regerror(i, &r, 0, 0);
            errmsg = new char[n];
            wrapped::regerror(i, &r, errmsg, n);
          }
        return !i;
      }
      friend std::ostream& operator<<(std::ostream &os, const type &obj)
      {
        if (obj.errmsg)
          return os << obj.errmsg;
        return os << "did not match";
      }
      ~type()
      {
        wrapped::regfree(&r);
        delete[] errmsg;
      }
    private:
      regex_t r;
      char *errmsg;
    };
  public:
    typedef enum {
      e = REG_EXTENDED,
      i = REG_ICASE,
      m = REG_NEWLINE
    } regflag;
    template <typename T>
    regex(T t,
          regflag f1 = regflag(),
          regflag f2 = regflag(),
          regflag f3 = regflag())
      : p(new type(t, f1 | f2 | f3))
    {
    }
    regex(const regex& r)
      : p(r.p)
    {
    }
    template <typename T>
    bool operator()(T t)
    {
      return (*p)(t);
    }
    friend std::ostream& operator<<(std::ostream &os, const regex &r)
    {
      return os << *r.p;
    }
  private:
    mutable std::auto_ptr<type> p; // Yeach! Ugly
  };


  inline
  collate_t<verbatim>
  collate(const std::string &s, const std::locale& l = std::locale())
  {
    return collate_t<verbatim>(s, l);
  }

  template <case_convert_type type>
  collate_t<type>
  collate(const std::string &s, const std::locale &l = std::locale())
  {
    return collate_t<type>(s, l);
  }

  class test_suite_base : public policies::dependencies::basic_enforcer
  {
  protected:
    test_suite_base();
  public:
    void add_case(crpcut_test_case_registrator* r);
    void report_success();
  private:
    unsigned num_containing_cases;
    crpcut_test_case_registrator *list;
  };

  template <typename T>
  class test_suite : public test_suite_base
  {
  public:
    test_suite() {}
    static test_suite& crpcut_reg()
    {
      static test_suite object;
      return object;
    }
    virtual void crpcut_add_action(policies::dependencies::basic_enforcer* e)
    {
      e->crpcut_inc(); // how to handle the case where this is empty?
    }
  private:
    virtual void crpcut_dec_action()
    {
      crpcut_register_success();
    }
  };

  template <typename T>
  inline void get_parameter(const char *name, T& t)
  {
    return test_case_factory::get_parameter(name, t);
  }

  template <typename T>
  inline T get_parameter(const char *name)
  {
    return test_case_factory::get_parameter<T>(name);
  }



#define CRPCUT_BINOP(name, opexpr)                                      \
  namespace expr {                                                      \
    template <typename T, typename U>                                   \
    class name                                                          \
    {                                                                   \
    public:                                                             \
      name(const T& t, const U& u) : t_(t), u_(u) {}                    \
      friend                                                            \
        std::ostream &operator<<(std::ostream &os, const name &a)       \
      {                                                                 \
        conditionally_stream<8>(os, a.t_);                              \
        os << " " << #opexpr << " ";                                    \
        conditionally_stream<8>(os, a.u_);                              \
        return os;                                                      \
      }                                                                 \
      friend struct eval_t<name>;                                       \
    private:                                                            \
      const T& t_;                                                      \
      const U& u_;                                                      \
    };                                                                  \
  }

#define CRPCUT_OPFUNC(name, opexpr)                                     \
  namespace expr {                                                      \
    template <typename T, typename U>                                   \
    name<T, U> operator opexpr(const T& t, const U& u)                  \
    {                                                                   \
      return name<T, U>(t, u);                                          \
    }                                                                   \
  }                                                                     \
  template <typename T, typename U>                                     \
  struct eval_t<expr::name<T, U> >                                      \
  {                                                                     \
    typedef typename eval_t<T>::type ttype;                             \
    typedef typename eval_t<U>::type utype;                             \
    typedef typename std::remove_reference<typename std::remove_cv<ttype>::type>::type trtype; \
    typedef typename std::remove_reference<typename std::remove_cv<utype>::type>::type urtype; \
    typedef CRPCUT_DECLTYPE(::crpcut::expr::gen<trtype>() opexpr ::crpcut::expr::gen<urtype>()) type; \
    static type func(const expr::name<T, U>& n)                         \
    {                                                                   \
      return eval(n.t_) opexpr eval(n.u_);                              \
    }                                                                   \
  };                                                                    \

#define CRPCUT_OPS(x)                           \
  x(eq_op, ==)                                  \
  x(ne_op, !=)                                  \
  x(ls_op, <<)                                  \
  x(rs_op, >>)                                  \
  x(lt_op, <)                                   \
  x(le_op, <=)                                  \
  x(gt_op, >)                                   \
  x(ge_op, >=)                                  \
  x(add_op, +)                                  \
  x(sub_op, -)                                  \
  x(mul_op, *)                                  \
  x(div_op, /)                                  \
  x(mod_op, %)                                  \
  x(and_op, &)                                  \
  x(or_op, |)                                   \
  x(xor_op, ^)

  CRPCUT_OPS(CRPCUT_BINOP)
  CRPCUT_OPS(CRPCUT_OPFUNC)
#undef CRPCUT_OFPUNC
#undef CRPCUT_BINOP
#undef CRPCUT_OPS

  namespace expr {
    template <typename T, typename U>
    struct use_two_assertions_instead  operator&&(const T&, const U&);

    template <typename T, typename U>
    struct refactor_this_expression  operator||(const T&, const U&);

    template <typename T>
    class atom
    {
    public:
      atom(const T& t) : t_(t) {}
      friend struct eval_t<atom>;
      friend
      std::ostream &operator<<(std::ostream &os, const atom& a)
      {
        conditionally_stream<8>(os, a.t_);
        return os;
      }
    private:
      const T& t_;
    };
  }

  template <typename T>
  struct eval_t<expr::atom<T> >
  {
    typedef const T& type;
    static type func(const expr::atom<T> &n) { return n.t_; }
  };

  namespace expr
  {
    class hook
    {
    public:
      template <case_convert_type type>
      const crpcut::collate_t<type>& operator->*(const crpcut::collate_t<type>& r) const
      {
        return r;
      }
      template <typename T>
      atom<T> operator->*(const T& t) const
      {
        return atom<T>(t);
      }
    };
  }

  template <typename T>
  typename eval_t<T>::type eval(const T& t) { return eval_t<T>::func(t); }

  namespace scope {
    class time_base
    {
    public:
      struct min
      {
        static const char *name();
        static bool busted(unsigned long now, unsigned long deadline);
      };
      struct max
      {
        static const char *name();
        static bool busted(unsigned long now, unsigned long deadline);
      };
      struct realtime
      {
        static const char *name();
        static unsigned long now();
      };
      struct cputime
      {
        static const char *name();
        static unsigned long now();
      };
      ~time_base() {};
      operator bool() const { return false; }
      void silence_warning() const {}
    protected:
      time_base(unsigned long deadline, const char *filename, size_t line);
      unsigned long const deadline_;
      char const *  const filename_;
      size_t        const line_;
    };
    template <comm::type action, typename cond, typename clock>
    class time : public time_base
    {
    public:
      time(unsigned long ms, const char *file, size_t line)
        : time_base(clock::now() + ms, file, line),
          limit_(ms)
      {
      }
      ~time()
      {
        if (limit_ + 1) // wrap around to 0 signals disabled temporary
          {
            unsigned long t = clock::now();
            if (test_case_factory::timeouts_enabled()
                && cond::busted(t, deadline_))
              {
                comm::direct_reporter<action>()
                  << filename_ << ":" << line_ << "\n"
                  << crpcut_check_name<action>::string()
                  << "_SCOPE_" << cond::name()
                  << "_" << clock::name() << "_MS(" << limit_ << ")"
                  "\nActual time used was " << t - (deadline_-limit_) << "ms";
              }
          }
      }
      time(const time& r)
        : time_base(r),
          limit_(r.limit_)
      {
        typedef unsigned long ul;
        r.limit_ = ~(ul());
      }
    private:
      time& operator=(const time&);

      unsigned long mutable limit_;
    };
  }
  void set_charset(const char *set_name);
  const char *get_start_dir();
  const char *get_parameter(const char *name);

  int  run(int argc, char *argv[], std::ostream &os = std::cerr);
  int  run(int argc, const char *argv[], std::ostream &os = std::cerr);

} // namespace crpcut

extern crpcut::namespace_info crpcut_current_namespace;

// Note, the order of inheritance below is important. test_case_base
// destructor signals ending of test case, so it must be listed as the
// first base class so that its instance is destroyed last

#define CRPCUT_TEST_CASE_DEF(test_case_name, ...)                       \
  class test_case_name                                                  \
    : crpcut::test_case_base, __VA_ARGS__                               \
  {                                                                     \
    friend struct crpcut::test_wrapper<crpcut_run_wrapper>;              \
    friend class crpcut::policies::dependencies::enforcer<test_case_name>; \
    friend class crpcut::crpcut_test_case_registrator;  \
    test_case_name() {}                                                 \
    ~test_case_name() {}                                                \
    virtual void crpcut_run_test()                                      \
    {                                                                   \
      crpcut_realtime_enforcer rt;                                      \
      using crpcut::policies::timeout::cputime_enforcer;                \
      cputime_enforcer ct(crpcut_cputime_enforcer::crpcut_cputime_timeout_ms); \
      (void)rt; /* silence warning */                                   \
      (void)ct; /* silence warning */                                   \
      crpcut::test_wrapper<crpcut_run_wrapper>::run(this);              \
      if (crpcut::test_case_factory::tests_as_child_procs())            \
        {                                                               \
          crpcut_test_finished(); /* tell destructor to report success */ \
        }                                                               \
    }                                                                   \
    void test();                                                        \
    class crpcut_registrator                                            \
      : public crpcut::crpcut_test_case_registrator,    \
        private virtual crpcut::policies::dependencies::crpcut_base,    \
        public virtual test_case_name::crpcut_expected_death_cause,     \
        private virtual test_case_name::crpcut_dependency,              \
        public virtual crpcut_testsuite_dep,                            \
        public test_case_name::crpcut_constructor_timeout_enforcer,     \
        public test_case_name::crpcut_destructor_timeout_enforcer       \
    {                                                                   \
       typedef crpcut::crpcut_test_case_registrator     \
         crpcut_registrator_base;                                       \
       static const unsigned long crpcut_cputime_timeout_ms             \
         =test_case_name::crpcut_cputime_enforcer::crpcut_cputime_timeout_ms; \
    public:                                                             \
       crpcut_registrator()                                             \
         : crpcut_registrator_base(#test_case_name,                     \
                                   crpcut_current_namespace,            \
                                   crpcut_cputime_timeout_ms)           \
         {                                                              \
           crpcut::test_suite<crpcut_testsuite_id>::crpcut_reg().add_case(this); \
           crpcut::crpcut_tag_info<crpcut_test_tag>::obj();             \
         }                                                              \
       virtual void crpcut_do_run_test_case()                           \
       {                                                                \
         CRPCUT_DEFINE_REPORTER;                                        \
         crpcut_prepare_construction(crpcut_constructor_timeout_ms);    \
         test_case_name obj;                                            \
         crpcut_manage_test_case_execution(&obj);                       \
         crpcut_prepare_destruction(crpcut_destructor_timeout_ms);      \
       }                                                                \
       virtual crpcut::tag& crpcut_tag() const                          \
       {                                                                \
         return crpcut::crpcut_tag_info<crpcut_test_tag>::obj();        \
       }                                                                \
    };                                                                  \
    static crpcut_registrator &crpcut_reg()                             \
    {                                                                   \
      static crpcut_registrator obj;                                    \
      return obj;                                                       \
    }                                                                   \
    virtual crpcut::crpcut_test_case_registrator&       \
      crpcut_get_reg() const                                            \
    {                                                                   \
      return crpcut_reg();                                              \
    }                                                                   \
    class crpcut_trigger                                                \
    {                                                                   \
    public:                                                             \
      crpcut_trigger() { crpcut_reg(); }                                \
    };                                                                  \
    static crpcut_trigger crpcut_trigger_obj;                           \
  };                                                                    \


#define TEST_DEF(test_case_name, ...)                                   \
  CRPCUT_TEST_CASE_DEF(test_case_name, __VA_ARGS__)                     \
  test_case_name::crpcut_trigger test_case_name::crpcut_trigger_obj;    \
  void test_case_name::test()

#define DISABLED_TEST_DEF(test_case_name, ...)                          \
  CRPCUT_TEST_CASE_DEF(test_case_name, __VA_ARGS__)                     \
  void test_case_name::test()

#define CRPCUT_CONCAT(a, b) a ## b

#define CRPCUT_CONCAT_(a, b) CRPCUT_CONCAT(a,b)

#define CRPCUT_LOCAL_NAME(prefix) \
  CRPCUT_CONCAT_(crpcut_local_  ## prefix ## _, __LINE__)

#define CRPCUT_STRINGIZE(a) #a
#define CRPCUT_STRINGIZE_(a) CRPCUT_STRINGIZE(a)

#ifndef CRPCUT_EXPERIMENTAL_CXX0X
#define CRPCUT_REFTYPE(expr) \
  const CRPCUT_DECLTYPE(expr) &
#else
namespace crpcut {
  namespace datatypes {
    template <typename T>
    const volatile typename std::remove_cv<typename std::remove_reference<T>::type>::type &gettype();

  }
}
#define CRPCUT_REFTYPE(expr) \
  CRPCUT_DECLTYPE(crpcut::datatypes::gettype<CRPCUT_DECLTYPE(expr)>())
#endif

#define NO_CORE_FILE \
  protected virtual crpcut::policies::no_core_file

#define WIPE_WORKING_DIR \
  crpcut::policies::deaths::wipe_working_dir

#define EXPECT_EXIT(...) \
  protected virtual crpcut::policies::exit_death<__VA_ARGS__>

#define EXPECT_REALTIME_TIMEOUT_MS(time) \
  protected virtual crpcut::policies::realtime_timeout_death<time>

#define EXPECT_SIGNAL_DEATH(...) \
  protected virtual crpcut::policies::signal_death<__VA_ARGS__>


#ifndef CRPCUT_NO_EXCEPTION_SUPPORT
#define EXPECT_EXCEPTION(type) \
  protected virtual crpcut::policies::exception_specifier<void (type)>
#define CRPCUT_DECLARE_EXCEPTION_TRANSLATOR(class_name, t)              \
  class class_name : public crpcut::policies::crpcut_exception_translator \
  {                                                                     \
    std::string crpcut_translate() const                                \
    {                                                                   \
      return crpcut_catcher(crpcut_do_translate);                       \
    }                                                                   \
    static std::string crpcut_do_translate(t);                          \
    template <typename U>                                               \
      static std::string crpcut_catcher(std::string (U))                \
    {                                                                   \
      try {                                                             \
        throw;                                                          \
      }                                                                 \
      catch(U u)                                                        \
        {                                                               \
          return crpcut_do_translate(u);                                \
        }                                                               \
    }                                                                   \
  }

#define CRPCUT_DESCRIBE_EXCEPTION(t)                                    \
  CRPCUT_DECLARE_EXCEPTION_TRANSLATOR(CRPCUT_LOCAL_NAME(specific_exception_translator), t); \
  namespace {                                                           \
    CRPCUT_LOCAL_NAME(specific_exception_translator) CRPCUT_LOCAL_NAME(exception_translator_obj); \
  }                                                                     \
  std::string CRPCUT_LOCAL_NAME(specific_exception_translator)::crpcut_do_translate(t)

#define CRPCUT_DEFINE_EXCEPTION_TRANSLATOR_CLASS(class_name, t) \
  CRPCUT_DECLARE_EXCEPTION_TRANSLATOR(class_name, t);           \
  std::string class_name::crpcut_do_translate(t)

#endif

#define DEPENDS_ON(...) \
  crpcut::policies::dependency_policy<crpcut::datatypes::tlist_maker<__VA_ARGS__>::type >



#define DEADLINE_CPU_MS(time) \
  crpcut::policies::timeout_policy<crpcut::policies::timeout::cputime, time>

#define FIXTURE_CONSTRUCTION_DEADLINE_REALTIME_MS(time) \
  public crpcut::policies::constructor_timeout_policy<time>
#define FIXTURE_DESTRUCTION_DEADLINE_REALTIME_MS(time) \
  public crpcut::policies::destructor_timeout_policy<time>

#define DEADLINE_REALTIME_MS(time) \
  crpcut::policies::timeout_policy<crpcut::policies::timeout::realtime, time>

#define CRPCUT_WRAP_FUNC(lib, name, rv, param_list, param)              \
  extern "C" typedef rv (*f_ ## name ## _t) param_list;                 \
  rv name param_list                                                    \
  {                                                                     \
    static f_ ## name ## _t f_ ## name                                  \
      = ::crpcut::libwrapper::loader< ::crpcut::libs::lib>::obj().sym<f_ ## name ## _t>(#name); \
    return f_ ## name param;                                            \
  }


#define CRPCUT_WRAP_V_FUNC(lib, name, rv, param_list, param)            \
  extern "C" typedef rv (*f_ ## name ## _t) param_list;                 \
  rv name param_list                                                    \
  {                                                                     \
    static f_ ## name ## _t f_ ## name                                  \
      = ::crpcut::libwrapper::loader< ::crpcut::libs::lib>::obj().sym<f_ ## name ## _t>(#name); \
    f_ ## name param;                                                   \
  }


#define CRPCUT_IS_ZERO_LIT(x) (sizeof(crpcut::null_cmp::func(x)) == 1)


#define CRPCUT_BINARY_CHECK(action, name, lh, rh)                       \
  do {                                                                  \
    try {                                                               \
      crpcut::tester                                    \
        <crpcut::comm::action,                                          \
         CRPCUT_IS_ZERO_LIT(lh), CRPCUT_DECLTYPE(lh),                   \
         CRPCUT_IS_ZERO_LIT(rh), CRPCUT_DECLTYPE(rh)>                   \
        (__FILE__ ":" CRPCUT_STRINGIZE_(__LINE__), #name)               \
        .name(lh, #lh, rh, #rh);                                        \
    }                                                                   \
    CATCH_BLOCK(..., {                                                  \
        std::string CRPCUT_LOCAL_NAME(s)                                \
          = crpcut::policies::crpcut_exception_translator::try_all();   \
        CRPCUT_CHECK_REPORT_HEAD(action) <<                             \
          "_" #name "(" #lh ", " #rh ")\n"                              \
          "  caught "                                                 \
                                         << CRPCUT_LOCAL_NAME(s);       \
    })                                                                \
  } while(0)



#define CRPCUT_CHECK_TRUE(action, a)                                    \
  do {                                                                  \
    try {                                                               \
      crpcut::bool_tester<crpcut::comm::action>         \
        (__FILE__ ":" CRPCUT_STRINGIZE_(__LINE__))                      \
        .check_true((crpcut::expr::hook()->*a), #a);                    \
    }                                                                   \
    CATCH_BLOCK(..., {                                                  \
        std::string CRPCUT_LOCAL_NAME(s)                                \
          = crpcut::policies::crpcut_exception_translator::try_all();   \
        CRPCUT_CHECK_REPORT_HEAD(action) << "_TRUE"                     \
          "(" #a ")\n"                                                  \
          "  caught "                                                 \
                                         << CRPCUT_LOCAL_NAME(s);       \
      })                                                                \
  } while(0)

#define ASSERT_TRUE(a) CRPCUT_CHECK_TRUE(exit_fail, a)
#define VERIFY_TRUE(a) CRPCUT_CHECK_TRUE(fail, a)

#define CRPCUT_CHECK_FALSE(action, a)                                   \
  do {                                                                  \
    try {                                                               \
      crpcut::bool_tester<crpcut::comm::action>         \
        (__FILE__ ":" CRPCUT_STRINGIZE_(__LINE__))                      \
        .assert_false((crpcut::expr::hook()->*a), #a);                  \
    }                                                                   \
    CATCH_BLOCK(..., {                                                  \
        std::string CRPCUT_LOCAL_NAME(s)                                \
          = crpcut::policies::crpcut_exception_translator::try_all();   \
        CRPCUT_CHECK_REPORT_HEAD(action) << "_FALSE"                    \
          "(" #a ")\n"                                                  \
          "  caught "                                                 \
                                         << CRPCUT_LOCAL_NAME(s);       \
      })                                                                \
  } while(0)

#define ASSERT_FALSE(a) CRPCUT_CHECK_FALSE(exit_fail, a)
#define VERIFY_FALSE(a) CRPCUT_CHECK_FALSE(fail, a)

#define ASSERT_EQ(lh, rh)  CRPCUT_BINARY_CHECK(exit_fail, EQ, lh, rh)

#define ASSERT_NE(lh, rh)  CRPCUT_BINARY_CHECK(exit_fail, NE, lh, rh)

#define ASSERT_GE(lh, rh)  CRPCUT_BINARY_CHECK(exit_fail, GE, lh, rh)

#define ASSERT_GT(lh, rh)  CRPCUT_BINARY_CHECK(exit_fail, GT, lh, rh)

#define ASSERT_LT(lh, rh)  CRPCUT_BINARY_CHECK(exit_fail, LT, lh, rh)

#define ASSERT_LE(lh, rh)  CRPCUT_BINARY_CHECK(exit_fail, LE, lh, rh)

#define VERIFY_EQ(lh, rh)  CRPCUT_BINARY_CHECK(fail, EQ, lh, rh)

#define VERIFY_NE(lh, rh)  CRPCUT_BINARY_CHECK(fail, NE, lh, rh)

#define VERIFY_GE(lh, rh)  CRPCUT_BINARY_CHECK(fail, GE, lh, rh)

#define VERIFY_GT(lh, rh)  CRPCUT_BINARY_CHECK(fail, GT, lh, rh)

#define VERIFY_LT(lh, rh)  CRPCUT_BINARY_CHECK(fail, LT, lh, rh)

#define VERIFY_LE(lh, rh)  CRPCUT_BINARY_CHECK(fail, LE, lh, rh)

#ifndef CRPCUT_NO_EXCEPTION_SUPPORT
#define CRPCUT_CHECK_THROW(action, expr, exc)                           \
  do {                                                                  \
    try {                                                               \
      try {                                                             \
        expr;                                                           \
        CRPCUT_CHECK_REPORT_HEAD(action) <<                             \
          "_THROW(" #expr ", " #exc ")\n"                         \
          "  Did not throw";                                            \
      }                                                                 \
      catch (exc) {                                                     \
        break;                                                          \
      }                                                                 \
    }                                                                   \
    CATCH_BLOCK(..., {                                                  \
        std::string CRPCUT_LOCAL_NAME(s)                                \
          = crpcut::policies::crpcut_exception_translator::try_all();   \
        CRPCUT_CHECK_REPORT_HEAD(action) << "_THROW"                    \
          "(" #expr ", " #exc  ")\n"                                    \
          "  caught "                                                 \
                                         << CRPCUT_LOCAL_NAME(s);       \
      })                                                                \
  } while (0)

#define ASSERT_THROW(expr, exc)                                         \
  CRPCUT_CHECK_THROW(exit_fail, expr, exc)

#define VERIFY_THROW(expr, exc)                                         \
  CRPCUT_CHECK_THROW(fail, expr, exc)
#endif

#ifndef CRPCUT_NO_EXCEPTION_SUPPORT
#define CRPCUT_CHECK_NO_THROW(action, expr)                             \
  do {                                                                  \
    try {                                                               \
      expr;                                                             \
    }                                                                   \
    CATCH_BLOCK(..., {                                                  \
        std::string CRPCUT_LOCAL_NAME(s)                                \
          = crpcut::policies::crpcut_exception_translator::try_all();   \
        CRPCUT_CHECK_REPORT_HEAD(action) << "_NO_THROW"                 \
          "(" #expr ")\n"                                               \
          "  caught "                                                   \
                                         << CRPCUT_LOCAL_NAME(s);       \
      })                                                                \
  } while (0)

#define ASSERT_NO_THROW(expr)                                           \
  CRPCUT_CHECK_NO_THROW(exit_fail, expr)

#define VERIFY_NO_THROW(expr)                                           \
  CRPCUT_CHECK_NO_THROW(fail, expr)
#endif


#define CRPCUT_CHECK_REPORT_HEAD(action)                                \
  crpcut::comm::direct_reporter<crpcut::comm::action>()                 \
  << __FILE__ ":" CRPCUT_STRINGIZE_(__LINE__) "\n"                      \
    << crpcut::crpcut_check_name<crpcut::comm::action>::string()        \

#define CRPCUT_CHECK_PRED(action, pred, ...)                            \
  do {                                                                  \
    static const char CRPCUT_LOCAL_NAME(sep)[][3] = { ", ", "" };       \
    try {                                                               \
      std::string CRPCUT_LOCAL_NAME(m);                                 \
      if (!crpcut::match_pred(CRPCUT_LOCAL_NAME(m),     \
                                              #pred,                    \
                                              pred,                     \
                                              crpcut::params(__VA_ARGS__))) \
        {                                                               \
          size_t CRPCUT_LOCAL_NAME(len) = CRPCUT_LOCAL_NAME(m).length(); \
          char *CRPCUT_LOCAL_NAME(p)                                    \
            = static_cast<char*>(alloca(CRPCUT_LOCAL_NAME(len)+1));     \
          CRPCUT_LOCAL_NAME(m).copy(CRPCUT_LOCAL_NAME(p),               \
                                    CRPCUT_LOCAL_NAME(len));            \
          CRPCUT_LOCAL_NAME(p)[CRPCUT_LOCAL_NAME(len)]=0;               \
          std::string().swap(CRPCUT_LOCAL_NAME(m));                     \
          CRPCUT_CHECK_REPORT_HEAD(action)                              \
            << "_PRED(" #pred                                           \
            << CRPCUT_LOCAL_NAME(sep)[!*#__VA_ARGS__]                   \
            << #__VA_ARGS__ ")\n"                                       \
            << CRPCUT_LOCAL_NAME(p);                                    \
        }                                                               \
    }                                                                   \
    CATCH_BLOCK(..., {                                                  \
        std::string CRPCUT_LOCAL_NAME(s)                                \
          = crpcut::policies::crpcut_exception_translator::try_all();   \
        CRPCUT_CHECK_REPORT_HEAD(action) << "PRED("                     \
          "(" #pred                                                     \
        << CRPCUT_LOCAL_NAME(sep)[!*#__VA_ARGS__]                       \
        << #__VA_ARGS__ ")\n"                                           \
          "  caught "                                                 \
                                         << CRPCUT_LOCAL_NAME(s);       \
      })                                                                \
  } while (0)

#define ASSERT_PRED(pred, ...)                                          \
  CRPCUT_CHECK_PRED(exit_fail, pred, __VA_ARGS__)

#define VERIFY_PRED(pred, ...)                                          \
  CRPCUT_CHECK_PRED(fail, pred, __VA_ARGS__)


#define CRPCUT_CHECK_SCOPE_TYPE_TIME_MS(action, type, clock, ms)        \
  if (const crpcut::scope::time_base& CRPCUT_LOCAL_NAME(time_scope)     \
      = crpcut::scope::time<crpcut::comm::action,                       \
                            crpcut::scope::time_base::type,             \
                            crpcut::scope::time_base::clock>(ms,        \
                                                             __FILE__,  \
                                                             __LINE__)) \
    { CRPCUT_LOCAL_NAME(time_scope).silence_warning(); } else           \

#define ASSERT_SCOPE_MAX_REALTIME_MS(ms)        \
  CRPCUT_CHECK_SCOPE_TYPE_TIME_MS(exit_fail, max, realtime, ms)

#define ASSERT_SCOPE_MIN_REALTIME_MS(ms)        \
  CRPCUT_CHECK_SCOPE_TYPE_TIME_MS(exit_fail, min, realtime, ms)

#define ASSERT_SCOPE_MAX_CPUTIME_MS(ms)         \
  CRPCUT_CHECK_SCOPE_TYPE_TIME_MS(exit_fail, max, cputime, ms)

#define VERIFY_SCOPE_MAX_REALTIME_MS(ms)        \
  CRPCUT_CHECK_SCOPE_TYPE_TIME_MS(fail, max, realtime, ms)

#define VERIFY_SCOPE_MIN_REALTIME_MS(ms)        \
  CRPCUT_CHECK_SCOPE_TYPE_TIME_MS(fail, min, realtime, ms)

#define VERIFY_SCOPE_MAX_CPUTIME_MS(ms)         \
  CRPCUT_CHECK_SCOPE_TYPE_TIME_MS(fail, max, cputime, ms)

#define CRPCUT_CHECK_SCOPE_HEAP_LEAK_FREE(type)                         \
  if (const crpcut::heap::local_root & CRPCUT_LOCAL_NAME(leak_free_scope) \
      = crpcut::heap::local_root(crpcut::comm::type,__FILE__, __LINE__)) \
    {                                                                   \
      CRPCUT_LOCAL_NAME(leak_free_scope).nonsense_func();  \
    }                                                                   \
  else

#define ASSERT_SCOPE_HEAP_LEAK_FREE \
  CRPCUT_CHECK_SCOPE_HEAP_LEAK_FREE(exit_fail)

#define VERIFY_SCOPE_HEAP_LEAK_FREE \
  CRPCUT_CHECK_SCOPE_HEAP_LEAK_FREE(fail)

class crpcut_testsuite_id;
class crpcut_testsuite_dep
  :
  public virtual crpcut::policies::dependencies::crpcut_base
{
};

#define TEST(...) TEST_DEF(__VA_ARGS__, crpcut::crpcut_none)
#define DISABLED_TEST(...) DISABLED_TEST_DEF(__VA_ARGS__, crpcut::crpcut_none)

#define TESTSUITE_DEF(name, ...)                                        \
  namespace name {                                                      \
    typedef crpcut_testsuite_dep crpcut_parent_testsuite_dep;           \
    namespace {                                                         \
      class crpcut_testsuite_dep                                        \
        : public virtual crpcut_parent_testsuite_dep,                   \
          public virtual crpcut::policies::dependencies::nested<__VA_ARGS__>::type \
      {                                                                 \
      };                                                                \
      static crpcut::namespace_info *parent_namespace                   \
      = &crpcut_current_namespace;                                      \
    }                                                                   \
    class crpcut_testsuite_id;                                          \
    static crpcut::namespace_info                                       \
    crpcut_current_namespace(#name, parent_namespace);                  \
  }                                                                     \
  namespace name

#define ALL_TESTS(suite_name)                                           \
  crpcut::test_suite<suite_name :: crpcut_testsuite_id >

#define TESTSUITE(...) TESTSUITE_DEF(__VA_ARGS__, crpcut::crpcut_none)

#define INFO crpcut::comm::direct_reporter<crpcut::comm::info>()
#define FAIL crpcut::comm::direct_reporter<crpcut::comm::exit_fail>()   \
  << __FILE__ ":" CRPCUT_STRINGIZE_(__LINE__)  "\n"



#define WITH_TEST_TAG(tag_name)                         \
  crpcut::policies::tag_policy<crpcut::crpcut_tags::tag_name>

#define DEFINE_TEST_TAG(tag_name)                               \
  namespace crpcut {                                            \
    namespace crpcut_tags {                                     \
      struct tag_name;                                          \
    }                                                           \
    template <>                                                 \
    inline                                                      \
    int                                                         \
    crpcut_tag_info<crpcut::crpcut_tags::tag_name>              \
    ::get_name_len() const                                      \
    {                                                           \
      return sizeof(#tag_name) - 1;                             \
    }                                                           \
    template <>                                                 \
    inline                                                      \
    crpcut::datatypes::fixed_string                             \
    crpcut_tag_info<crpcut::crpcut_tags::tag_name>              \
    ::get_name() const                                          \
    {                                                           \
      using crpcut::datatypes::fixed_string;                    \
      fixed_string s = { #tag_name, sizeof(#tag_name) - 1};     \
      return s;                                                 \
    }                                                           \
  }                                                             \
  using crpcut::crpcut_tags::tag_name

#ifdef GMOCK_INCLUDE_GMOCK_GMOCK_H_



#endif
#endif // CRPCUT_HPP
