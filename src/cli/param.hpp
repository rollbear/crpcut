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



#ifndef CLI_PARAM_HPP_
#define CLI_PARAM_HPP_

#include <exception>
#include <string>
#include <crpcut.hpp>

namespace crpcut {
  namespace cli {
    class param_list;
    class param : public datatypes::list_elem<param>
    {
    public:
      typedef enum { mandatory, optional } value_requirement;
      class exception;
      template <size_t N>
      param(char short_form, const char (&long_form)[N],
            const char *param_description,
            param_list &root);
      template <size_t N>
      param(char short_form, const char (&long_form)[N],
            const char        *value_description,
            value_requirement  req,
            const char        *param_description,
            param_list &root);
      virtual ~param();
      const char *const *match(const char *const *);
      friend std::ostream &operator<<(std::ostream &os, const param &p)
      {
        return p.print_to(os);
      }
      std::ostream &syntax(std::ostream &) const;
    protected:
      virtual bool match_value(const char *, bool is_short);
      static const char *match_or_end(const char *p, char c);
    private:
      std::ostream &print_to(std::ostream &) const;
      const char *const       long_form_;
      const size_t            long_form_len_;
      const char              short_form_;
      const char *const       value_description_;
      const char *const       param_description_;
      const value_requirement req_;
    };

    class param::exception : public std::exception
    {
    public:
      exception(std::string s) : s_(s) {}
      ~exception() throw () {}
      const char *what() const throw () { return s_.c_str(); }
    private:
      std::string s_;
    };

    class param_list : public datatypes::list_elem<param>
    {
    public:
      param_list();
      const char *const *match_all(const char *const *p);
    };

    template <size_t N>
    param::param(char short_form, const char (&long_form)[N],
                 const char *param_description,
                 param_list &root)
      : long_form_(long_form),
        long_form_len_(N - 1),
        short_form_(short_form),
        value_description_(0),
        param_description_(param_description),
        req_(optional)
    {
      link_before(root);
    }

    template <size_t N>
    param::param(char short_form, const char (&long_form)[N],
                 const char        *value_description,
                 value_requirement  req,
                 const char         *param_description,
                 param_list &root)
      : long_form_(long_form),
        long_form_len_(N - 1),
        short_form_(short_form),
        value_description_(value_description),
        param_description_(param_description),
        req_(req)
    {
      link_before(root);
    }
  }
}





#endif // CLI_PARAM_HPP_
