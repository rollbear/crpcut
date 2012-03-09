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

#ifndef TEST_CASE_FACTORY_HPP_
#define TEST_CASE_FACTORY_HPP_

#include <crpcut.hpp>

namespace crpcut {

  class working_dir_allocator;
  class deadline_monitor;

  class test_case_factory
  {
  public:
    virtual ~test_case_factory();
    static void set_charset(const char *set_name);
    static const char *get_charset();
    static const char *get_output_charset();
    static const char *get_illegal_rep();

    static int run_test(int argc, char *argv[],
                        std::ostream &os = std::cerr);
    static int run_test(int argc, const char *argv[],
                        std::ostream &os = std::cerr);
    virtual void introduce_name(pid_t pid, const char *name, size_t len);
    virtual void present(pid_t pid, comm::type t, test_phase phase,
                         size_t len, const char *buff);
    virtual bool tests_as_child_procs();
    virtual bool timeouts_enabled();
    virtual void set_deadline(crpcut_test_case_registrator *i);
    virtual void clear_deadline(crpcut_test_case_registrator *i);
    virtual void return_dir(unsigned num);
    virtual void test_succeeded(crpcut_test_case_registrator*);
    virtual unsigned long calc_cputime(const struct timeval&);

    static bool is_backtrace_enabled();
    static const char *get_start_dir();
    static const char *get_parameter(const char *name);

    class registrator_list : public crpcut_test_case_registrator
    {
    public:
      virtual void run_test_case();
      virtual tag& crpcut_tag() const;
      virtual tag::importance get_importance() const;
      virtual void setup(poll<fdreader>    &, pid_t, int, int, int, int);
    };
    static test_case_factory& obj();
  protected:
    test_case_factory();
  private:
    void list_tests(const char *const *names,
                    tag_list_root     &tags,
                    std::ostream      &os);
    bool schedule_tests(std::size_t num_parallel, poll<fdreader> &poller);
    void show_summary(unsigned       num_selected_tests,
                      tag_list_root &tags) const;
    void kill_presenter_process();
    void manage_children(std::size_t max_pending_children, poll<fdreader> &poller);
    void start_test(crpcut_test_case_registrator *i, poll<fdreader> &poller);

    int do_run(cli::interpreter *cli, std::ostream &os, tag_list_root &tags);
    const char *do_get_start_dir() const;
    void do_set_charset(const char *set_name);
    const char *do_get_charset() const;
    friend class crpcut_test_case_registrator;


    cli::interpreter        *cli_;
    struct timeval           accumulated_cputime_;
    pid_t                    current_pid_;
    registrator_list         reg_;
    unsigned                 num_pending_children_;
    unsigned                 num_tests_run_;
    unsigned                 num_successful_tests_;
    int                      presenter_pipe_;
    deadline_monitor        *deadlines_;
    working_dir_allocator   *working_dirs_;
    char                     dirbase_[PATH_MAX];
    char                     homedir_[PATH_MAX];
    const char              *charset_;
  };


}
#endif // TEST_CASE_FACTORY_HPP_
