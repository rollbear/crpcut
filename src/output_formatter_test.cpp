/*
 * Copyright 2012-2012 Bjorn Fahller <bjorn@fahller.se>
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

#include <crpcut.hpp>
#include "output_formatter.hpp"
#include "output_buffer.hpp"
TESTSUITE(output_formatter)
{
  class empty_formatter : public crpcut::output::formatter
  {
  public:
    virtual ~empty_formatter();
  private:
    void begin_case(const char *, std::size_t, bool, bool) {}
    void end_case() {}
    void terminate(crpcut::test_phase,
                   const char*,
                   std::size_t,
                   const char *,
                   std::size_t) {}
    void print(const char *, std::size_t, const char *, std::size_t) {}
    void statistics(unsigned, unsigned, unsigned, unsigned) {}
    void nonempty_dir(const char*) {}
    void blocked_test(const crpcut::crpcut_test_case_registrator *) {}
    void tag_summary(const char *, std::size_t, std::size_t, bool) {}
  public:
    typedef enum { no_escape, with_escape } escape_type;
    empty_formatter(crpcut::output::buffer &buff,
                    const char *to,
                    const char *subst,
                    escape_type esc = no_escape)
      : crpcut::output::formatter(buff, to, subst),
        escape_(esc == with_escape)
    {
    }
    using formatter::phase_str;
    using formatter::write;
    virtual crpcut::fixed_string escape(char c) const
    {
      static const crpcut::fixed_string none = { 0, 0 };
      if (!escape_ || c < 0 || c >= ' ')
        {
          return none;
        }
      static char buff[] = "      ";
      static crpcut::fixed_string esc = { buff, 4 };
      crpcut::stream::oastream out(buff);
      out << "\\x" << std::hex << std::setw(2) << std::setfill('0') << int(c);
      return esc;
    }
  private:
    bool escape_;
  };

  empty_formatter::~empty_formatter()
  {
  }

  class buffer : public crpcut::output::buffer
  {
  public:
    template <size_t N>
    buffer(size_t block_size, const char (&f)[N])
      : block_size_(block_size),
        offset_(0U),
        begin_(f),
        size_(N - 1)
    {
    }
    virtual ~buffer() { ASSERT_TRUE(offset_ == size_); }
    std::pair<const char*, std::size_t> get_buffer() const
    {
      abort();
    }
    void advance() { abort(); }
    ssize_t write(const char *buff, std::size_t len)
    {
      size_t remaining = size_ - offset_;
      size_t max_chunk = remaining > block_size_ ? block_size_ : remaining;
      size_t chunk_size = len >  max_chunk ? max_chunk : len;
      ASSERT_TRUE(offset_ + chunk_size <= size_);
      for (size_t n = 0; n < chunk_size; ++n)
        {
          ASSERT_TRUE(begin_[offset_ + n] == buff[n]);
        }
      offset_ += chunk_size;
      return ssize_t(chunk_size);
    }
    bool is_empty() const { abort(); }
  private:
    size_t block_size_;
    size_t offset_;
    char const *const begin_;
    size_t const      size_;
  };

  TEST(illegal_to_charset_throws, EXPECT_EXCEPTION(std::runtime_error))
  {
    buffer b(0, "");
    try {
      empty_formatter f(b, "tjolahopp!", "apa");
    }
    catch (std::runtime_error &e)
    {
      static const char m[] = "Can't convert from \"UTF-8\" to \"tjolahopp!\"";
      ASSERT_TRUE(std::string(e.what()) == m);
      throw;
    }
  }

  std::string str(crpcut::fixed_string s)
  {
    return std::string(s.str, s.len);
  }

  TEST(all_phase_strings_are_correct)
  {
    typedef empty_formatter f;
    VERIFY_TRUE(str(f::phase_str(crpcut::creating)) == "\"creating\"");
    VERIFY_TRUE(str(f::phase_str(crpcut::running)) == "\"running\"");
    VERIFY_TRUE(str(f::phase_str(crpcut::destroying)) == "\"destroying\"");
    VERIFY_TRUE(str(f::phase_str(crpcut::post_mortem)) == "\"post_mortem\"");
    VERIFY_TRUE(str(f::phase_str(crpcut::child)) == "\"child\"");
  }

  static const char msg[] =
    "abcdefghijklmnopqrstuvwxyz\n"
    "abcdefghijklmnopqrstuvwxyz\n"
    "abcdefghijklmnopqrstuvwxyz\n"
    "abcdefghijklmnopqrstuvwxyz\n"
    "abcdefghijklmnopqrstuvwxyz\n"
    "abcdefghijklmnopqrstuvwxyz\n"
    "abcdefghijklmnopqrstuvwxyz\n"
    "abcdefghijklmnopqrstuvwxyz\n"
    "abcdefghijklmnopqrstuvwxyz\n"
    "abcdefghijklmnopqrstuvwxyz\n";

  TEST(char_array_verbatim_write)
  {
    buffer b(7, msg);
    empty_formatter f(b, "UTF-8", "--illegal--");
    std::size_t rv = f.write(msg);
    ASSERT_TRUE(rv == sizeof(msg) - 1);
  }

  TEST(char_ptr_verbatim_write)
  {
    buffer b(11, msg);
    empty_formatter f(b, "UTF-8", "--illegal--");
    const char* m = msg;
    std::size_t rv = f.write(m);
    ASSERT_TRUE(rv == sizeof(msg) - 1);
  }

  TEST(char_ptr_with_len_write)
  {
    buffer b(11, msg);
    empty_formatter f(b, "UTF-8", "--illegal--");
    const char* m = msg;
    std::size_t rv = f.write(m, sizeof(msg) - 1);
    ASSERT_TRUE(rv == sizeof(msg) - 1);
  }

  template <typename T, size_t N>
    void assert_integer_write(T t, const char (&array)[N])
  {
    buffer b(N/2, array);
    empty_formatter f(b, "UTF-8", "--illegal--");
    std::size_t rv = f.write(t);
    ASSERT_TRUE(rv == N - 1);
  }

#define ASSERT_INTEGER_WRITE(t, n) assert_integer_write((t)(n), #n)

  TEST(integer_writes)
  {
    ASSERT_INTEGER_WRITE(short, -32760);
    ASSERT_INTEGER_WRITE(unsigned short, 65530);
    ASSERT_INTEGER_WRITE(int, -2000000000);
    ASSERT_INTEGER_WRITE(unsigned, 4000000000);
  }

  TEST(string_with_illegal_chars_gets_substitutions)
  {
    static const char in[] =  "abc\x85" "def\xf0" "gh";
    static const char out[] =  "abc--illegal--def--illegal--gh";
    buffer b(3, out);
    empty_formatter f(b, "UTF-8", "--illegal--");
    std::size_t rv = f.write(in, crpcut::output::formatter::translated);
    ASSERT_TRUE(rv == sizeof(in) - 1);
  }

  TEST(escapes_are_honoured_and_and_illegal_chars_substituted)
  {
    static const char in[] =  "abc\x05" "def\xf0" "gh";
    static const char out[] =  "abc\\x05def--illegal--gh";
    buffer b(3, out);
    empty_formatter f(b, "UTF-8", "--illegal--", empty_formatter::with_escape);
    std::size_t rv = f.write(in, crpcut::output::formatter::translated);
    ASSERT_TRUE(rv == sizeof(in) - 1);
  }

  TEST(verbatim_write_does_not_translate_anything)
  {
    static const char in[] =  "abc\x05" "def\xf0" "gh";
    buffer b(3, in);
    empty_formatter f(b, "UTF-8", "--illegal--", empty_formatter::with_escape);
    std::size_t rv = f.write(in, crpcut::output::formatter::verbatim);
    ASSERT_TRUE(rv == sizeof(in) - 1);
  }
}
