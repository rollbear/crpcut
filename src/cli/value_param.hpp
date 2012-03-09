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



#ifndef VALUE_PARAM_HPP_
#define VALUE_PARAM_HPP_

#include "param.hpp"
#include <sstream>
#include <cassert>
#include <crpcut.hpp>
namespace crpcut {
  namespace cli {
    template <typename T>
    class value_param : public param
    {
      typedef bool (value_param::*bool_type)(const char *, bool);
    public:
      template <size_t N>
      value_param(char short_form, const char (&long_form)[N],
                  const char *value_description,
                  const char *param_description,
                  param_list &root);
      operator bool_type() const;
      const T& get_value() const;
    protected:
      virtual bool match_value(const char *, bool);
    private:
      void read_value(const char *p);
      T    value_;
      bool active_;
    };

    template <typename T> template <size_t N>
    value_param<T>::value_param(char short_form, const char (&long_form)[N],
                                const char *value_description,
                                const char *param_description,
                                param_list &root)
      : param(short_form, long_form,
              value_description, param::mandatory,
              param_description,
              root),
        value_(),
        active_(false)
    {
    }

    template <typename T>
    value_param<T>::operator typename value_param<T>::bool_type() const
    {
      return active_ ? &value_param::match_value : 0;
    }

    template <typename T>
    bool
    value_param<T>::match_value(const char *p, bool)
    {
      if (!p || *p == 0)
        {
          std::ostringstream os;
          syntax(os) << " expects a value";
          throw exception(os.str());
        }
      if (active_)
        {
          std::ostringstream os;
          syntax(os) << " can only be used once";
          throw exception(os.str());
        }
      read_value(p);
      return true;
    }

    template <typename T>
    void
    value_param<T>::read_value(const char *p)
    {
      stream::iastream is(p);
      char unwanted_tail;
      if (!(is >> value_) || (is >> unwanted_tail))
        {
          std::ostringstream os;
          syntax(os) << " - can't interpret \"" << p << "\"";
          throw exception(os.str());
        }
      active_ = true;
    }

    template <>
    inline
    void
    value_param<const char*>::read_value(const char *p)
    {
      value_ = p;
      active_ = true;
    }

    template <typename T>
    const T&
    value_param<T>::get_value() const
    {
      assert(active_);
      return value_;
    }
  }
}






#endif // VALUE_PARAM_HPP_
