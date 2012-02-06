/*
 * Copyright 2012 Bjorn Fahller <bjorn@fahller.se>
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


#include <gmock/gmock.h>
#include <crpcut.hpp>
#include "../presentation_reader.hpp"
#include "../output/formatter.hpp"
#include "../poll.hpp"
#include <deque>
#include <string>

using namespace testing;
TESTSUITE(presentation_reader)
{
  class fd_mock : public crpcut::comm::rfile_descriptor
  {
  public:
    fd_mock(int n) : crpcut::comm::rfile_descriptor(n), pos_(0U) {}
    template <typename T>
    void add_data(const T& t)
    {
      const void* addr = &t;
      const char *cs = static_cast<const char*>(addr);
      data_.push_back(std::string(cs, sizeof(t)));
    }
    template <size_t N>
    void add_data(const char (&cs)[N])
    {
      const size_t n = N - 1;
      const char *p = static_cast<const char*>(static_cast<const void*>(&n));
      data_.push_back(std::string(p, sizeof(n)));
      if (n) data_.push_back(std::string(cs, n));
    }
    virtual ssize_t read(void *p, size_t n) const
    {
      if (data_.empty()) return 0;
      size_t s = std::min(n, data_.front().size() - pos_);
      char *d = static_cast<char *>(p);
      data_.front().copy(d, s, pos_);
      pos_+= s;
      if (pos_ == data_.front().size())
        {
          pos_ = 0;
          data_.pop_front();
        }
      return ssize_t(s);
    }
    template <size_t N>
    void begin_test(pid_t id, crpcut::test_phase phase, const char (&name)[N])
    {
      msg<crpcut::comm::begin_test>(id, phase, name);
    }
    void end_test(pid_t id, crpcut::test_phase phase, bool critical)
    {
      add_data(id);
      add_data(crpcut::comm::end_test);
      add_data(phase);
      add_data(sizeof(critical));
      add_data(critical);
    }
    template <size_t N>
    void nonempty_dir(pid_t id, crpcut::test_phase phase, const char (&name)[N])
    {
      msg<crpcut::comm::dir>(id, phase, name);
    }
    template <size_t N>
    void fail(pid_t id, crpcut::test_phase phase, const char (&name)[N])
    {
      msg<crpcut::comm::fail>(id, phase, name);
    }
    template <size_t N>
    void exit_fail(pid_t id, crpcut::test_phase phase, const char (&name)[N])
    {
      msg<crpcut::comm::exit_fail>(id, phase, name);
    }
    template <size_t N>
    void exit_ok(pid_t id, crpcut::test_phase phase, const char (&name)[N])
    {
      msg<crpcut::comm::exit_ok>(id, phase, name);
    }
    template <size_t N>
    void stdout(pid_t id, crpcut::test_phase phase, const char (&name)[N])
    {
      msg<crpcut::comm::stdout>(id, phase, name);
    }
    template <size_t N>
    void stderr(pid_t id, crpcut::test_phase phase, const char (&name)[N])
    {
      msg<crpcut::comm::stderr>(id, phase, name);
    }
    template <size_t N>
    void info(pid_t id, crpcut::test_phase phase, const char (&name)[N])
    {
      msg<crpcut::comm::info>(id, phase, name);
    }
  private:
    template <crpcut::comm::type type, size_t N>
    void msg(pid_t id, crpcut::test_phase phase, const char (&name)[N])
    {
      add_data(id);
      add_data(type);
      add_data(phase);
      add_data(name);
    }
    mutable std::deque<std::string > data_;
    mutable size_t pos_;
  };

  class fmt_mock : public crpcut::output::formatter
  {
  public:
    MOCK_METHOD3(begin_case, void(std::string,bool,bool));
    void begin_case(crpcut::datatypes::fixed_string s, bool b1, bool b2)
    {
      begin_case(std::string(s.str, s.len), b1, b2);
    }
    MOCK_METHOD0(end_case, void());
    MOCK_METHOD3(terminate, void(crpcut::test_phase, std::string, std::string));
    void terminate(crpcut::test_phase phase,
                   crpcut::datatypes::fixed_string s1,
                   crpcut::datatypes::fixed_string s2)
    {
      terminate(phase, std::string(s1.str, s1.len), std::string(s2.str, s2.len));
    }

    MOCK_METHOD2(print, void(std::string, std::string));
    void print(crpcut::datatypes::fixed_string s1,
               crpcut::datatypes::fixed_string s2)

    {
      print(std::string(s1.str, s1.len), std::string(s2.str, s2.len));
    }
    MOCK_METHOD4(statistics, void(unsigned, unsigned, unsigned, unsigned));
    MOCK_METHOD1(nonempty_dir, void(const char*));
    MOCK_METHOD1(blocked_test, void(crpcut::datatypes::fixed_string));

  };

  class poll_mock : public crpcut::poll<crpcut::io>
  {
  public:
    poll_mock(int n, Sequence *s)
    {
      EXPECT_CALL(*this, do_add_fd(n, _, _)).InSequence(*s);
    }
    MOCK_METHOD3(do_add_fd, void(int, crpcut::io*, int));
    MOCK_METHOD1(do_del_fd, void(int));
    MOCK_METHOD1(do_wait, descriptor(int));
    MOCK_CONST_METHOD0(do_num_fds, std::size_t());
  };

  template <bool verbose>
  struct fix
  {
    fix()
    : fd(87),
      fmt(),
      poll(87, &in),
      reader(poll, fd, fmt, verbose, "directory/subdir")
    {
    }
    Sequence in;
    Sequence out;
    fd_mock                     fd;
    StrictMock<fmt_mock>        fmt;
    StrictMock<poll_mock>       poll;
    crpcut::presentation_reader reader;
  };

  TEST(passed_critical_behaviour_verbose_with_info, fix<true>)
  {
    fd.begin_test(101, crpcut::creating, "apa::katt");
    ASSERT_FALSE(reader.read());
    fd.info(101, crpcut::running, "en liten misse");
    ASSERT_FALSE(reader.read());
    fd.exit_ok(101, crpcut::post_mortem, "");
    ASSERT_FALSE(reader.read());
    fd.end_test(101, crpcut::post_mortem, true);

    EXPECT_CALL(fmt, begin_case("apa::katt",true, true));
    EXPECT_CALL(fmt, print("info","en liten misse"));
    EXPECT_CALL(fmt, end_case());
    ASSERT_FALSE(reader.read());
  }

  TEST(failed_critical_behaviour_with_nonempty_dir, fix<true>)
  {
    fd.begin_test(101, crpcut::creating, "apa::katt");
    ASSERT_FALSE(reader.read());
    fd.exit_fail(101, crpcut::running, "FAIL << orm");
    ASSERT_FALSE(reader.read());
    fd.nonempty_dir(101, crpcut::running, "");
    ASSERT_FALSE(reader.read());
    fd.end_test(101, crpcut::running, true);

    EXPECT_CALL(fmt, begin_case("apa::katt", false, true));
    EXPECT_CALL(fmt, terminate(crpcut::running, "FAIL << orm",
                               "directory/subdir/apa::katt"));
    EXPECT_CALL(fmt, end_case());
    ASSERT_FALSE(reader.read());
  }

  TEST(interleaved_tests_are_shown_in_sequence, fix<true>)
  {
    fd.begin_test(101, crpcut::creating, "apa::katt");
    ASSERT_FALSE(reader.read());
    fd.begin_test(20, crpcut::creating, "ko");
    ASSERT_FALSE(reader.read());
    fd.exit_fail(101, crpcut::running, "FAIL << orm");
    ASSERT_FALSE(reader.read());
    fd.fail(20, crpcut::running, "VERIFY_APA");
    ASSERT_FALSE(reader.read());
    fd.end_test(101, crpcut::running, true);

    EXPECT_CALL(fmt, begin_case("apa::katt", false, true));
    EXPECT_CALL(fmt, terminate(crpcut::running, "FAIL << orm", ""));
    EXPECT_CALL(fmt, end_case());

    ASSERT_FALSE(reader.read());
    fd.end_test(20, crpcut::running, false);

    EXPECT_CALL(fmt, begin_case("ko", false, false));
    EXPECT_CALL(fmt, print("fail", "VERIFY_APA"));
    EXPECT_CALL(fmt, terminate(crpcut::running, "", ""));
    EXPECT_CALL(fmt, end_case());
    ASSERT_FALSE(reader.read());
  }

  TEST(stderr_stdout_info_are_not_shown_in_non_verbose_mode_pass, fix<false>)
  {
    fd.begin_test(101, crpcut::creating, "apa::katt");
    ASSERT_FALSE(reader.read());
    fd.info(101, crpcut::running, "INFO");
    ASSERT_FALSE(reader.read());
    fd.stdout(101, crpcut::running, "stdout");
    ASSERT_FALSE(reader.read());
    fd.stderr(101, crpcut::running, "stderr");
    ASSERT_FALSE(reader.read());
    fd.end_test(101, crpcut::destroying, true);
    ASSERT_FALSE(reader.read());
  }

  TEST(stderr_stdout_info_are_shown_in_non_verbose_fail, fix<false>)
  {
    fd.begin_test(101, crpcut::creating, "apa::katt");
    ASSERT_FALSE(reader.read());
    fd.info(101, crpcut::running, "INFO");
    ASSERT_FALSE(reader.read());
    fd.stdout(101, crpcut::running, "stdout");
    ASSERT_FALSE(reader.read());
    fd.stderr(101, crpcut::running, "stderr");
    ASSERT_FALSE(reader.read());
    fd.exit_fail(101, crpcut::running, "FAIL << orm");
    ASSERT_FALSE(reader.read());
    fd.end_test(101, crpcut::running, false);

    EXPECT_CALL(fmt, begin_case("apa::katt", false, false));
    EXPECT_CALL(fmt, print("info", "INFO"));
    EXPECT_CALL(fmt, print("stdout", "stdout"));
    EXPECT_CALL(fmt, print("stderr", "stderr"));
    EXPECT_CALL(fmt, terminate(crpcut::running, "FAIL << orm", ""));
    EXPECT_CALL(fmt, end_case());
    ASSERT_FALSE(reader.read());
  }

  TEST(reader_returns_true_on_fail_and_removes_from_poll_on_exception,
       fix<false>)
  {
    ASSERT_TRUE(reader.read());
    EXPECT_CALL(poll, do_del_fd(87));
    reader.exception();
  }
}






