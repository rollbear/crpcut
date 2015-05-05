/*
 * Copyright 2012-2013 Bjorn Fahller <bjorn@fahller.se>
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

#include <trompeloeil.hpp>
#include <crpcut.hpp>
#include "../presentation_reader.hpp"
#include "../output/formatter.hpp"
#include "../poll.hpp"
#include "../registrator_list.hpp"
#include <deque>
#include <string>

TESTSUITE(presentation_reader)
{
  using trompeloeil::_;

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
    void add_data(const char (&cs)[N], bool with_len = true)
    {
      const size_t n = N - 1;
      const char *p = static_cast<const char*>(static_cast<const void*>(&n));
      if (with_len) data_.push_back(std::string(p, sizeof(n)));
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
    struct end_data
    {
      unsigned long critical;
      unsigned long duration;
    };
    void end_test(pid_t              id,
                  crpcut::test_phase phase,
                  bool               critical,
                  unsigned long      duration)
    {
      add_data(id);
      add_data(crpcut::comm::end_test);
      add_data(phase);
      end_data m = { critical, duration };
      add_data(sizeof(m));
      add_data(m);
    }
    template <size_t N>
    void nonempty_dir(pid_t id, crpcut::test_phase phase, const char (&name)[N])
    {
      msg<crpcut::comm::dir>(id, phase, name);
    }
    template <size_t N1, size_t N2>
    void fail(pid_t id, crpcut::test_phase phase, const char (&loc)[N1], const char (&name)[N2])
    {
      msg<crpcut::comm::fail>(id, phase, loc, name);
    }
    template <size_t N1, size_t N2>
    void exit_fail(pid_t id, crpcut::test_phase phase, const char (&loc)[N1], const char (&name)[N2])
    {
      msg<crpcut::comm::exit_fail>(id, phase, loc, name);
    }
    void exit_ok(pid_t id, crpcut::test_phase phase)
    {
      msg<crpcut::comm::exit_ok>(id, phase, "");
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
    template <size_t N1, size_t N2>
    void info(pid_t id, crpcut::test_phase phase, const char (&loc)[N1], const char (&name)[N2])
    {
      msg<crpcut::comm::info>(id, phase, loc, name);
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
    template <crpcut::comm::type type, size_t N1, size_t N2>
    void msg(pid_t id, crpcut::test_phase phase, const char (&loc)[N1], const char (&name)[N2])
    {
      add_data(id);
      add_data(type);
      add_data(phase);
      add_data(N1 - 1 + N2 - 1 + sizeof(size_t));
      add_data(loc);
      add_data(name, false);
    }
    mutable std::deque<std::string > data_;
    mutable size_t pos_;
  };

  class fmt_mock : public crpcut::output::formatter
  {
  public:
    MAKE_MOCK4(begin_case, void(std::string,bool,bool, unsigned long));
    MAKE_MOCK0(end_case, void());
    MAKE_MOCK4(terminate, void(crpcut::test_phase, std::string, std::string, std::string));
    void terminate(crpcut::test_phase phase,
                   crpcut::datatypes::fixed_string msg,
                   crpcut::datatypes::fixed_string location,
                   std::string                     dir)
    {
      terminate(phase,
                std::string(msg.str, msg.len),
                std::string(location.str, location.len),
                dir);
    }

    MAKE_MOCK3(print, void(std::string, std::string, std::string));
    void print(crpcut::datatypes::fixed_string s1,
               crpcut::datatypes::fixed_string s2,
               crpcut::datatypes::fixed_string s3)

    {
      print(std::string(s1.str, s1.len), std::string(s2.str, s2.len), std::string(s3.str, s3.len));
    }
    MAKE_MOCK2(statistics, void(unsigned, unsigned));
    MAKE_MOCK1(nonempty_dir, void(const char*));
    MAKE_MOCK2(blocked_test, void(crpcut::tag::importance,
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
    poll_mock(int n, trompeloeil::sequence *s)
      : add_fd_x(NAMED_REQUIRE_CALL(*this, do_add_fd(n, _, _)).IN_SEQUENCE(*s))
    {
    }
    MAKE_MOCK3(do_add_fd, void(int, crpcut::io*, int));
    MAKE_MOCK1(do_del_fd, void(int));
    MAKE_MOCK1(do_wait, descriptor(int));
    MAKE_CONST_MOCK0(do_num_fds, std::size_t());
  private:
    std::unique_ptr<trompeloeil::expectation> add_fd_x;
  };

  class registrator_mock : public crpcut::crpcut_test_case_registrator
  {
    typedef crpcut::crpcut_test_case_registrator R;
  public:
    registrator_mock(const char *name) : R(name, &top), top(0,0) {}
    MAKE_MOCK4(setup, void(crpcut::poll<crpcut::fdreader>&,
                           int, int, int));
    MAKE_MOCK0(run_test_case, void());
    MAKE_CONST_MOCK0(crpcut_tag, crpcut::tag &());
    MAKE_CONST_MOCK0(get_importance, crpcut::tag::importance());

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
    trompeloeil::sequence        in;
    trompeloeil::sequence        out;
    fd_mock                      fd;
    fmt_mock                     fmt;
    fmt_mock                     summary_fmt;
    poll_mock                    poll;
    registrator_mock             apa_katt;
    registrator_mock             ko;
    crpcut::registrator_list     list;
    tag_list                     tags;
    crpcut::presentation_reader  reader;
  };


  TEST(passed_critical_verbose_prints_all, fix<true>)
  {
    fd.begin_test(101, crpcut::creating, &apa_katt);
    fd.info(101, crpcut::running, "apa.cpp", "INFO << info");
    fd.stderr(101, crpcut::running, "stderr");
    fd.stdout(101, crpcut::running, "stdout");
    fd.exit_ok(101, crpcut::post_mortem);
    fd.end_test(101, crpcut::post_mortem, true, 100);

    REQUIRE_CALL(apa_katt, crpcut_tag())
      .RETURN(std::ref(tags));
    REQUIRE_CALL(fmt, begin_case("apa::katt",true, true, 100U)).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, print("info", "INFO << info", "apa.cpp")).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, print("stderr", "stderr", ""));
    REQUIRE_CALL(fmt, print("stdout", "stdout", ""));
    REQUIRE_CALL(fmt, end_case()).IN_SEQUENCE(out);
    while (!reader.read())
      ;
  }

  TEST(failed_critical_behaviour_with_nonempty_dir_prints_all, fix<true>)
  {
    fd.begin_test(101, crpcut::creating, &apa_katt);
    fd.exit_fail(101, crpcut::running, "apa.cpp", "FAIL << orm");
    fd.nonempty_dir(101, crpcut::running, "");
    fd.end_test(101, crpcut::running, true, 100);

    REQUIRE_CALL(apa_katt, crpcut_tag())
      .RETURN(std::ref(tags));
    REQUIRE_CALL(fmt, begin_case("apa::katt", false, true, 100U)).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, terminate(crpcut::running, "FAIL << orm", "apa.cpp",
                                "directory/subdir/apa::katt")).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, end_case()).IN_SEQUENCE(out);
    while (!reader.read())
      ;
  }

  TEST(nonempty_dir_with_pid_0_reports_to_formatter_immediately, fix<true>)
  {
    fd.nonempty_dir(0, crpcut::running, "apa\0");

    REQUIRE_CALL(fmt, nonempty_dir(_))
      .WITH(std::string(_1) == "apa");
    REQUIRE_CALL(summary_fmt, nonempty_dir(_))
      .WITH(std::string(_1) == "apa");
    while (!reader.read())
      ;
  }

  TEST(interleaved_tests_are_shown_in_sequence, fix<true>)
  {
    fd.begin_test(101, crpcut::creating, &apa_katt);
    fd.begin_test(20, crpcut::creating, &ko);
    fd.exit_fail(101, crpcut::running, "apa.cpp", "FAIL << orm");
    fd.fail(20, crpcut::running, "apa.cpp", "VERIFY_APA");
    fd.end_test(101, crpcut::running, true, 100);
    fd.exit_fail(20, crpcut::running, "apa.cpp", "Earlier verify failed");

    REQUIRE_CALL(apa_katt, crpcut_tag())
      .RETURN(std::ref(tags));
    REQUIRE_CALL(ko, crpcut_tag())
      .RETURN(std::ref(tags));
    REQUIRE_CALL(fmt, begin_case("apa::katt", false, true, 100U)).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, terminate(crpcut::running, "FAIL << orm", "apa.cpp", "")).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, end_case()).IN_SEQUENCE(out);

    fd.end_test(20, crpcut::running, false, 100);

    REQUIRE_CALL(fmt, begin_case("ko", false, false, 100U)).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, print("fail", "VERIFY_APA", "apa.cpp")).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, terminate(crpcut::running, "Earlier verify failed", "apa.cpp", "")).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, end_case()).IN_SEQUENCE(out);
    while (!reader.read())
      ;
  }

  TEST(stderr_stdout_info_are_not_shown_in_non_verbose_mode_pass, fix<false>)
  {
    fd.begin_test(101, crpcut::creating, &apa_katt);
    fd.info(101, crpcut::running, "apa.cpp", "INFO");
    fd.stdout(101, crpcut::running, "stdout");
    fd.stderr(101, crpcut::running, "stderr");
    fd.end_test(101, crpcut::destroying, true, 100);

    REQUIRE_CALL(apa_katt, crpcut_tag())
      .RETURN(std::ref(tags));
    while (!reader.read())
      ;
  }

  TEST(stderr_stdout_info_are_shown_in_non_verbose_fail, fix<false>)
  {
    fd.begin_test(101, crpcut::creating, &apa_katt);
    fd.info(101, crpcut::running, "apa.cpp", "INFO");
    fd.stdout(101, crpcut::running, "stdout");
    fd.stderr(101, crpcut::running, "stderr");
    fd.exit_fail(101, crpcut::running, "apa.cpp", "FAIL << orm");
    fd.end_test(101, crpcut::running, false, 100);

    REQUIRE_CALL(apa_katt, crpcut_tag())
      .RETURN(std::ref(tags));
    REQUIRE_CALL(fmt, begin_case("apa::katt", false, false, 100U)).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, print("info", "INFO", "apa.cpp")).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, print("stdout", "stdout", "")).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, print("stderr", "stderr", "")).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, terminate(crpcut::running, "FAIL << orm", "apa.cpp", "")).IN_SEQUENCE(out);
    REQUIRE_CALL(fmt, end_case()).IN_SEQUENCE(out);
    while (!reader.read())
      ;
  }

  TEST(reader_returns_true_on_fail_and_removes_from_poll_on_exception,
       fix<false>)
  {
    ASSERT_TRUE(reader.read());
    REQUIRE_CALL(poll, do_del_fd(87));
    ALLOW_CALL(ko, get_importance())
      .RETURN(crpcut::tag::critical);
    ALLOW_CALL(apa_katt, get_importance())
      .RETURN(crpcut::tag::non_critical);

    REQUIRE_CALL(fmt, blocked_test(crpcut::tag::critical, "ko"))
      .IN_SEQUENCE(in);
    REQUIRE_CALL(summary_fmt, blocked_test(crpcut::tag::critical, "ko"))
      .IN_SEQUENCE(in);
    REQUIRE_CALL(fmt, blocked_test(crpcut::tag::non_critical, "apa::katt"))
      .IN_SEQUENCE(in);
    REQUIRE_CALL(summary_fmt, blocked_test(crpcut::tag::non_critical, "apa::katt"))
      .IN_SEQUENCE(in);
    REQUIRE_CALL(fmt, statistics(0U,0U)).IN_SEQUENCE(in);
    REQUIRE_CALL(summary_fmt, statistics(0U,0U));
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


