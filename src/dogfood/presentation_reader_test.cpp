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
#include "../registrator_list.hpp"
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
    void begin_test(pid_t                                       id,
                    crpcut::test_phase                          phase,
                    const crpcut::crpcut_test_case_registrator *test)
    {
      add_data(id);
      add_data(crpcut::comm::begin_test);
      add_data(phase);
      add_data(sizeof(test));
      add_data(test);
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
    MOCK_METHOD0(end_case, void());
    MOCK_METHOD3(terminate, void(crpcut::test_phase, std::string, std::string));
    void terminate(crpcut::test_phase phase,
                   crpcut::datatypes::fixed_string s1,
                   std::string                     s2)
    {
      terminate(phase, std::string(s1.str, s1.len), s2);
    }

    MOCK_METHOD2(print, void(std::string, std::string));
    void print(crpcut::datatypes::fixed_string s1,
               crpcut::datatypes::fixed_string s2)

    {
      print(std::string(s1.str, s1.len), std::string(s2.str, s2.len));
    }
    MOCK_METHOD2(statistics, void(unsigned, unsigned));
    MOCK_METHOD1(nonempty_dir, void(const char*));
    MOCK_METHOD2(blocked_test, void(crpcut::tag::importance,
                                    std::string));
    void blocked_test(crpcut::tag::importance i,
                      crpcut::datatypes::fixed_string s)
    {
      blocked_test(i, std::string(s.str, s.len));
    }
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

  class registrator_mock : public crpcut::crpcut_test_case_registrator
  {
    typedef crpcut::crpcut_test_case_registrator R;
  public:
    registrator_mock(const char *name) : R(name, &top), top(0,0) {}
    MOCK_METHOD5(setup, void(crpcut::poll<crpcut::fdreader>&,
                             pid_t,
                             int, int, int));
    MOCK_METHOD0(run_test_case, void());
    MOCK_CONST_METHOD0(crpcut_tag, crpcut::tag &());
    MOCK_CONST_METHOD0(get_importance, crpcut::tag::importance());

    crpcut::namespace_info top;
  };
  class tag_list : public crpcut::tag_list_root
  {
  public:
    virtual crpcut::datatypes::fixed_string get_name() const
    {
      static const crpcut::datatypes::fixed_string s = { "", 0 };
      return s;
    }
  };

  template <bool verbose>
  struct fix
  {
    fix()
      : fd(87),
        fmt(),
        summary_fmt(),
        poll(87, &in),
        apa_katt("apa::katt"),
        ko("ko"),
        reader(poll, fd, fmt, summary_fmt, verbose, "directory/subdir", list)
    {
      ko.link_before(list);
      apa_katt.link_before(list);
    }
    Sequence                     in;
    Sequence                     out;
    fd_mock                      fd;
    StrictMock<fmt_mock>         fmt;
    StrictMock<fmt_mock>         summary_fmt;
    StrictMock<poll_mock>        poll;
    StrictMock<registrator_mock> apa_katt;
    StrictMock<registrator_mock> ko;
    crpcut::registrator_list     list;
    tag_list                     tags;
    crpcut::presentation_reader  reader;
  };


  TEST(passed_critical_verbose_prints_all, fix<true>)
  {
    fd.begin_test(101, crpcut::creating, &apa_katt);
    fd.info(101, crpcut::running, "INFO << info");
    fd.stderr(101, crpcut::running, "stderr");
    fd.stdout(101, crpcut::running, "stdout");
    fd.exit_ok(101, crpcut::post_mortem, "");
    fd.end_test(101, crpcut::post_mortem, true);

    EXPECT_CALL(apa_katt, crpcut_tag())
      .WillOnce(ReturnRef(tags));
    EXPECT_CALL(fmt, begin_case("apa::katt",true, true)).InSequence(out);
    EXPECT_CALL(fmt, print("info", "INFO << info")).InSequence(out);
    EXPECT_CALL(fmt, print("stderr", "stderr"));
    EXPECT_CALL(fmt, print("stdout", "stdout"));
    EXPECT_CALL(fmt, end_case()).InSequence(out);
    while (!reader.read())
      ;
  }

  TEST(failed_critical_behaviour_with_nonempty_dir_prints_all, fix<true>)
  {
    fd.begin_test(101, crpcut::creating, &apa_katt);
    fd.exit_fail(101, crpcut::running, "FAIL << orm");
    fd.nonempty_dir(101, crpcut::running, "");
    fd.end_test(101, crpcut::running, true);

    EXPECT_CALL(apa_katt, crpcut_tag())
      .WillOnce(ReturnRef(tags));
    EXPECT_CALL(fmt, begin_case("apa::katt", false, true)).InSequence(out);
    EXPECT_CALL(fmt, terminate(crpcut::running, "FAIL << orm",
                               "directory/subdir/apa::katt")).InSequence(out);
    EXPECT_CALL(fmt, end_case()).InSequence(out);
    while (!reader.read())
      ;
  }

  TEST(nonempty_dir_with_pid_0_reports_to_formatter_immediately, fix<true>)
  {
    fd.nonempty_dir(0, crpcut::running, "apa\0");

    EXPECT_CALL(fmt, nonempty_dir(testing::StrEq("apa")));
    EXPECT_CALL(summary_fmt, nonempty_dir(testing::StrEq("apa")));
    while (!reader.read())
      ;
  }

  TEST(interleaved_tests_are_shown_in_sequence, fix<true>)
  {
    fd.begin_test(101, crpcut::creating, &apa_katt);
    fd.begin_test(20, crpcut::creating, &ko);
    fd.exit_fail(101, crpcut::running, "FAIL << orm");
    fd.fail(20, crpcut::running, "VERIFY_APA");
    fd.end_test(101, crpcut::running, true);

    EXPECT_CALL(apa_katt, crpcut_tag())
      .WillOnce(ReturnRef(tags));
    EXPECT_CALL(ko, crpcut_tag())
      .WillOnce(ReturnRef(tags));
    EXPECT_CALL(fmt, begin_case("apa::katt", false, true)).InSequence(out);
    EXPECT_CALL(fmt, terminate(crpcut::running, "FAIL << orm", "")).InSequence(out);
    EXPECT_CALL(fmt, end_case()).InSequence(out);

    fd.end_test(20, crpcut::running, false);

    EXPECT_CALL(fmt, begin_case("ko", false, false)).InSequence(out);
    EXPECT_CALL(fmt, print("fail", "VERIFY_APA")).InSequence(out);
    EXPECT_CALL(fmt, terminate(crpcut::running, "", "")).InSequence(out);
    EXPECT_CALL(fmt, end_case()).InSequence(out);
    while (!reader.read())
      ;
  }

  TEST(stderr_stdout_info_are_not_shown_in_non_verbose_mode_pass, fix<false>)
  {
    fd.begin_test(101, crpcut::creating, &apa_katt);
    fd.info(101, crpcut::running, "INFO");
    fd.stdout(101, crpcut::running, "stdout");
    fd.stderr(101, crpcut::running, "stderr");
    fd.end_test(101, crpcut::destroying, true);

    EXPECT_CALL(apa_katt, crpcut_tag())
      .WillOnce(ReturnRef(tags));
    while (!reader.read())
      ;
  }

  TEST(stderr_stdout_info_are_shown_in_non_verbose_fail, fix<false>)
  {
    fd.begin_test(101, crpcut::creating, &apa_katt);
    fd.info(101, crpcut::running, "INFO");
    fd.stdout(101, crpcut::running, "stdout");
    fd.stderr(101, crpcut::running, "stderr");
    fd.exit_fail(101, crpcut::running, "FAIL << orm");
    fd.end_test(101, crpcut::running, false);

    EXPECT_CALL(apa_katt, crpcut_tag())
      .WillOnce(ReturnRef(tags));
    EXPECT_CALL(fmt, begin_case("apa::katt", false, false)).InSequence(out);
    EXPECT_CALL(fmt, print("info", "INFO")).InSequence(out);
    EXPECT_CALL(fmt, print("stdout", "stdout")).InSequence(out);
    EXPECT_CALL(fmt, print("stderr", "stderr")).InSequence(out);
    EXPECT_CALL(fmt, terminate(crpcut::running, "FAIL << orm", "")).InSequence(out);
    EXPECT_CALL(fmt, end_case()).InSequence(out);
    while (!reader.read())
      ;
  }

  TEST(reader_returns_true_on_fail_and_removes_from_poll_on_exception,
       fix<false>)
  {
    ASSERT_TRUE(reader.read());
    EXPECT_CALL(poll, do_del_fd(87));
    EXPECT_CALL(ko, get_importance())
      .WillRepeatedly(Return(crpcut::tag::critical));
    EXPECT_CALL(apa_katt, get_importance())
      .WillRepeatedly(Return(crpcut::tag::non_critical));

    EXPECT_CALL(fmt, blocked_test(crpcut::tag::critical, "ko"))
      .InSequence(in);
    EXPECT_CALL(summary_fmt, blocked_test(crpcut::tag::critical, "ko"))
      .InSequence(in);
    EXPECT_CALL(fmt, blocked_test(crpcut::tag::non_critical, "apa::katt"))
      .InSequence(in);
    EXPECT_CALL(summary_fmt, blocked_test(crpcut::tag::non_critical, "apa::katt"))
      .InSequence(in);
    EXPECT_CALL(fmt, statistics(0,0)).InSequence(in);
    EXPECT_CALL(summary_fmt, statistics(0,0));
    reader.exception();
  }


  TEST(faulty_command_aborts, EXPECT_SIGNAL_DEATH(SIGABRT), NO_CORE_FILE,
       fix<false>)
  {
    fd.add_data(pid_t(101));
    fd.add_data(crpcut::comm::type(101));
    fd.add_data(crpcut::running);
    fd.add_data(std::size_t(0));
    reader.read();
  }

  TEST(reader_asserts_on_write, fix<false>,
       EXPECT_SIGNAL_DEATH(SIGABRT), NO_CORE_FILE)
  {
    reader.write();
  }
}


