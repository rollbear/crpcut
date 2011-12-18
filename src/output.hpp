/*
 * Copyright 2009-2011 Bjorn Fahller <bjorn@fahller.se>
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

#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <vector>

extern "C" {
  #include <iconv.h>
}

namespace crpcut
{
  namespace output
  {
    struct fixed_string
    {
      const char *str;
      size_t      len;
    private:
      struct secret_bool;
    public:
      operator const secret_bool* () const
      {
        return len ? reinterpret_cast<const secret_bool*>(this) : 0;
      }
    };

    class buffer
    {
    public:
      static std::pair<const char*, size_t> get_buffer();
      static void advance();
      static ssize_t write(const char *buff, size_t len);
      static bool is_empty();
    private:
      buffer();
      ~buffer();

      static buffer& obj();
      std::pair<const char*, size_t> do_get_buffer() const;
      void do_advance();
      ssize_t do_write(const char *buff, size_t len);
      bool do_is_empty() const;
      struct block
      {
        block() :next(0), len(0) {}
        static const size_t size = 128;

        block *next;
        char    mem[size];
        size_t  len;
      };


      block  *head;
      block **current;
    };

    inline std::pair<const char*, size_t> buffer::get_buffer()
    {
      return obj().do_get_buffer();
    }

    inline void buffer::advance()
    {
      obj().do_advance();
    }

    inline ssize_t buffer::write(const char *buff, size_t len)
    {
      return obj().do_write(buff, len);
    }

    inline bool buffer::is_empty()
    {
      return obj().do_is_empty();
    }

    inline buffer::buffer()
      : head(0),
        current(&head)
    {
    }

    inline buffer& buffer::obj()
    {
      static buffer object;
      return object;
    }

    inline bool buffer::do_is_empty() const
    {
      return !head;
    }

    template <bool b>
    struct enable_if;
    template <>
    struct enable_if<true>
    {
      typedef void type;
    };
    class formatter
    {

    public:
      typedef implementation::crpcut_test_case_registrator test_case_reg;
      typedef enum { translated, verbatim } type;
      virtual void begin_case(const char *name,
                              size_t      name_len,
                              bool        result,
                              bool        critical) = 0;
      virtual void end_case()  = 0;
      virtual void terminate(test_phase phase,
                             const char *msg,
                             size_t      msg_len,
                             const char *dirname = 0,
                             size_t      dn_len = 0) = 0;
      virtual void print(const char *tag, size_t tlen, const char *data, size_t dlen) = 0;
      virtual void statistics(unsigned num_registered,
                              unsigned num_selected,
                              unsigned num_run,
                              unsigned num_failed) = 0;
      virtual void nonempty_dir(const  char*)  = 0;
      virtual void blocked_test(const test_case_reg *)  = 0;
      virtual void tag_summary(const char *tag_name,
                               size_t      num_passed,
                               size_t      num_failed,
                               bool        critical) = 0;
      virtual ~formatter();
    protected:
      formatter(const char *to_charset, const char *subst);
      size_t write(const char *s, type t = verbatim) const
      {
        return write(s, wrapped::strlen(s), t);
      }
      template <size_t N>
      size_t write(const char (&str)[N], type t = verbatim) const
      {
        return write(&str[0], N - 1, t);
      }
      size_t write(const stream::oastream &o, type t = verbatim) const
      {
        return write(o.begin(), o.size(), t);
      }
      size_t write(const char *str, size_t len, type t = verbatim) const;
      template <typename T>
      size_t write(T val,
                   typename enable_if<std::numeric_limits<T>::is_integer>::type * = 0)
      {
        stream::toastream<std::numeric_limits<T>::digits10> o;
        o << val;
        return write(o.begin(), o.size());
      }
    private:
      virtual fixed_string escape(char c) const;
      size_t do_write(const char *p, size_t len) const;
      size_t do_write_converted(const char *buff, size_t len) const;

      iconv_t iconv_handle;
      const char *illegal_substitute;
      size_t illegal_substitute_len;
    };


    class xml_formatter : public formatter
    {
    public:
      xml_formatter(const char *id, int argc, const char *argv[]);
      virtual ~xml_formatter();
      virtual void begin_case(const char *name,
                              size_t      name_len,
                              bool        result,
                              bool        critical);
      virtual void end_case();
      virtual void terminate(test_phase phase,
                             const char *msg,
                             size_t      msg_len,
                             const char *dirname = 0,
                             size_t      dn_len = 0);
      virtual void print(const char *tag, size_t tlen, const char *data, size_t dlen);
      virtual void statistics(unsigned num_registered,
                              unsigned num_selected,
                              unsigned num_run,
                              unsigned num_failed);
      virtual void nonempty_dir(const char *s);
      virtual void blocked_test(const test_case_reg*);
      virtual void tag_summary(const char *tag_name,
                               size_t      num_passed,
                               size_t      num_failed,
                               bool        critical);
    private:
      virtual fixed_string escape(char c) const;
      void make_closed();

      size_t                    non_critical_fail_sum;
      bool                      last_closed_;
      bool                      blocked_tests_;
      bool                      tag_summary_;
    };

    struct tag_result
    {
      tag_result(std::string n, size_t p, size_t f, bool c)
        : name(n), passed(p), failed(f), critical(c) {}
      std::string name;
      size_t      passed;
      size_t      failed;
      bool        critical;
    };

    class text_formatter : public formatter
    {
      std::vector<tag_result> tag_results;
    public:
      text_formatter(const char *, int, const char**);
      virtual void begin_case(const char *name,
                              size_t      name_len,
                              bool        result,
                              bool        critical);
      virtual void end_case();
      virtual void terminate(test_phase phase,
                             const char *msg,
                             size_t      msg_len,
                             const char *dirname = 0,
                             size_t      dn_len = 0);
      virtual void print(const char *tag, size_t tlen, const char *data, size_t dlen);
      virtual void statistics(unsigned num_registered,
                              unsigned num_selected,
                              unsigned num_run,
                              unsigned num_failed);
      virtual void nonempty_dir(const char *s);
      virtual void blocked_test(const test_case_reg *i);
      virtual void tag_summary(const char *tag_name,
                               size_t      num_passed,
                               size_t      num_failed,
                               bool        critical);
    private:
      bool did_output_;
      bool blocked_tests_;
      formatter::type conversion_type_;
    };

  }
}
#endif // OUTPUT_HPP
