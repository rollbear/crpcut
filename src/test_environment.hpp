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



#ifndef TEST_ENVIRONMENT_HPP_
#define TEST_ENVIRONMENT_HPP_

extern "C"
{
#  include <dirent.h>
}
namespace crpcut {
  namespace cli {
    class interpreter;
  }
  class test_environment
  {
  protected:
    test_environment() : cli_(0), charset_(0) {} // for test use
  public:
    test_environment(cli::interpreter *cli);
    virtual ~test_environment();

    virtual void set_charset(const char *set_name);
    virtual const char *get_charset() const;
    virtual const char *get_output_charset() const;
    virtual const char *get_illegal_rep() const;
    virtual bool tests_as_child_procs() const;
    virtual bool timeouts_enabled() const;
    virtual unsigned timeout_multiplier() const;
    virtual const char *get_start_dir() const;
    virtual const char *get_parameter(const char *name) const;
    static void set_default_charset(const char *charset);
  private:

    cli::interpreter       *cli_;
    const char             *charset_;
    char                    homedir_[PATH_MAX];
    static const char      *default_charset;
  };
}


#endif // TEST_ENVIRONMENT_HPP_
