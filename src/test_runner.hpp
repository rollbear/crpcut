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

#ifndef TEST_RUNNER_HPP_
#define TEST_RUNNER_HPP_

#include <crpcut.hpp>
#include "registrator_list.hpp"
#include "test_environment.hpp"

namespace crpcut {

  class working_dir_allocator;
  class deadline_monitor;
  class test_case_registrator;
  class test_environment;

  class test_runner
  {
  protected:
    test_runner();
  public:
    static test_runner& obj();
    virtual ~test_runner();
    virtual void introduce_test(pid_t                               pid,
                                const crpcut_test_case_registrator *reg);
    virtual void present(pid_t pid, comm::type t, test_phase phase,
                         size_t len, const char *buff);
  public:
    static int run_test(int argc, char *argv[],
                        std::ostream &os = std::cerr);
    static int run_test(int argc, const char *argv[],
                        std::ostream &os = std::cerr);
    virtual test_environment *environment() const;
  private:
    virtual void set_deadline(crpcut_test_case_registrator *i);
    virtual void clear_deadline(crpcut_test_case_registrator *i);
    virtual void return_dir(unsigned num);
    virtual unsigned long calc_cputime(const struct timeval&);
    void schedule_tests(std::size_t num_parallel, poll<fdreader> &poller);
    int  spawn_test_runner();
    void manage_children(std::size_t max_pending_children, poll<fdreader> &poller);
    void start_test(crpcut_test_case_registrator *i, poll<fdreader> &poller);
    int do_run(cli::interpreter *cli, std::ostream &os, tag_list_root &tags);

    friend class crpcut_test_case_registrator;

    test_environment        *env_;
    cli::interpreter        *cli_;
    struct timeval           accumulated_cputime_;
    registrator_list         reg_;
    unsigned                 num_pending_children_;
    comm::wfile_descriptor   presenter_pipe_;
    deadline_monitor        *deadlines_;
    working_dir_allocator   *working_dirs_;
    char                     dirbase_[PATH_MAX];
  };


}
#endif // TEST_CASE_FACTORY_HPP_
