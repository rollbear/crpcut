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

#include <trompeloeil.hpp>
#include <crpcut.hpp>
#include "../../output/writer.hpp"
#include "../../output/buffer.hpp"
#include "../../output/text_modifier.hpp"
#include <sstream>

TESTSUITE(output)
{
  using trompeloeil::_;

  TESTSUITE(text_modifier)
  {
    class test_buffer : public crpcut::output::buffer
    {
    public:
      using buff = std::pair<const char*, std::size_t>;
      MAKE_CONST_MOCK0(get_buffer, buff());
      MAKE_MOCK0(advance, void());
      MAKE_MOCK2(write, ssize_t(const char*, std::size_t));
      MAKE_CONST_MOCK0(is_empty, bool());
    };

    using crpcut::output::text_modifier;

    TEST(nullstring_does_nothing)
    {
      text_modifier obj(0);
      std::ostringstream os;
      obj.write_to(os, text_modifier::NORMAL);
      ASSERT_TRUE(os.str() == "");
      obj.write_to(os, text_modifier::PASSED);
      ASSERT_TRUE(os.str() == "");
      obj.write_to(os, text_modifier::FAILED);
      ASSERT_TRUE(os.str() == "");
      obj.write_to(os, text_modifier::NCFAILED);
      ASSERT_TRUE(os.str() == "");
      obj.write_to(os, text_modifier::NCPASSED);
      ASSERT_TRUE(os.str() == "");
      obj.write_to(os, text_modifier::BLOCKED);
      ASSERT_TRUE(os.str() == "");
      obj.write_to(os, text_modifier::PASSED_SUM);
      ASSERT_TRUE(os.str() == "");
      obj.write_to(os, text_modifier::FAILED_SUM);
      ASSERT_TRUE(os.str() == "");
      obj.write_to(os, text_modifier::NCPASSED_SUM);
      ASSERT_TRUE(os.str() == "");
      obj.write_to(os, text_modifier::NCFAILED_SUM);
      ASSERT_TRUE(os.str() == "");
      obj.write_to(os, text_modifier::BLOCKED_SUM);
      ASSERT_TRUE(os.str() == "");

      test_buffer buff;

      FORBID_CALL(buff, write(_,_));

      crpcut::output::writer w(buff, "UTF-8", "--illegal--");
      obj.write_to(w, text_modifier::NORMAL);
      obj.write_to(w, text_modifier::PASSED);
      obj.write_to(w, text_modifier::FAILED);
      obj.write_to(w, text_modifier::NCFAILED);
      obj.write_to(w, text_modifier::NCPASSED);
      obj.write_to(w, text_modifier::BLOCKED);
      obj.write_to(w, text_modifier::PASSED_SUM);
      obj.write_to(w, text_modifier::FAILED_SUM);
      obj.write_to(w, text_modifier::NCPASSED_SUM);
      obj.write_to(w, text_modifier::NCFAILED_SUM);
      obj.write_to(w, text_modifier::BLOCKED_SUM);
    }

    class fix
    {
    protected:
      fix() : writer(buff, "UTF-8", "--illegal--") {}
      test_buffer buff;
      crpcut::output::writer writer;
      trompeloeil::sequence seq;
    };

    TEST(set_values_are_honoured, fix)
    {
      static const char config[] =
        " 0"
        " PASSED=1"
        " FAILED=2"
        " NCFAILED=3"
        " NCPASSED=4"
        " BLOCKED=5"
        " PASSED_SUM=6"
        " FAILED_SUM=7"
        " NCPASSED_SUM=8"
        " NCFAILED_SUM=9"
        " BLOCKED_SUM=10"
        " ";
      text_modifier modifier(config);
      REQUIRE_CALL(buff, write(_,1U)).WITH(*_1=='0').IN_SEQUENCE(seq).RETURN(ssize_t(_2));
      REQUIRE_CALL(buff, write(_,1U)).WITH(*_1=='1').IN_SEQUENCE(seq).RETURN(ssize_t(_2));
      REQUIRE_CALL(buff, write(_,1U)).WITH(*_1=='2').IN_SEQUENCE(seq).RETURN(ssize_t(_2));
      REQUIRE_CALL(buff, write(_,1U)).WITH(*_1=='3').IN_SEQUENCE(seq).RETURN(ssize_t(_2));
      REQUIRE_CALL(buff, write(_,1U)).WITH(*_1=='4').IN_SEQUENCE(seq).RETURN(ssize_t(_2));
      REQUIRE_CALL(buff, write(_,1U)).WITH(*_1=='5').IN_SEQUENCE(seq).RETURN(ssize_t(_2));
      REQUIRE_CALL(buff, write(_,1U)).WITH(*_1=='6').IN_SEQUENCE(seq).RETURN(ssize_t(_2));
      REQUIRE_CALL(buff, write(_,1U)).WITH(*_1=='7').IN_SEQUENCE(seq).RETURN(ssize_t(_2));
      REQUIRE_CALL(buff, write(_,1U)).WITH(*_1=='8').IN_SEQUENCE(seq).RETURN(ssize_t(_2));
      REQUIRE_CALL(buff, write(_,1U)).WITH(*_1=='9').IN_SEQUENCE(seq).RETURN(ssize_t(_2));
      REQUIRE_CALL(buff, write(_,2U))
        .WITH(std::string(_1,_2) == "10")
        .IN_SEQUENCE(seq)
        .RETURN(ssize_t(_2));
      modifier.write_to(writer, text_modifier::NORMAL);
      modifier.write_to(writer, text_modifier::PASSED);
      modifier.write_to(writer, text_modifier::FAILED);
      modifier.write_to(writer, text_modifier::NCFAILED);
      modifier.write_to(writer, text_modifier::NCPASSED);
      modifier.write_to(writer, text_modifier::BLOCKED);
      modifier.write_to(writer, text_modifier::PASSED_SUM);
      modifier.write_to(writer, text_modifier::FAILED_SUM);
      modifier.write_to(writer, text_modifier::NCPASSED_SUM);
      modifier.write_to(writer, text_modifier::NCFAILED_SUM);
      modifier.write_to(writer, text_modifier::BLOCKED_SUM);
    }

    TEST(failed_propagates, fix)
    {
      static const char config[] =
        " 0"
        " FAILED=F"
        " ";

      text_modifier modifier(config);
      {
        REQUIRE_CALL(buff, write(_,_))
          .WITH(std::string(_1,_2) == "F")
          .TIMES(4)
          .RETURN(ssize_t(_2));
        modifier.write_to(writer, text_modifier::FAILED);
        modifier.write_to(writer, text_modifier::NCFAILED);
        modifier.write_to(writer, text_modifier::FAILED_SUM);
        modifier.write_to(writer, text_modifier::NCFAILED_SUM);
      }
      {
        REQUIRE_CALL(buff, write(_,_))
          .WITH(std::string(_1,_2) == "0")
          .RETURN(ssize_t(_2));
        modifier.write_to(writer, text_modifier::NORMAL);

        modifier.write_to(writer, text_modifier::PASSED);
        modifier.write_to(writer, text_modifier::NCPASSED);
        modifier.write_to(writer, text_modifier::BLOCKED);
        modifier.write_to(writer, text_modifier::PASSED_SUM);
        modifier.write_to(writer, text_modifier::NCPASSED_SUM);
        modifier.write_to(writer, text_modifier::BLOCKED_SUM);
      }
    }

    TEST(ncfailed_propagates, fix)
    {
      static const char config[] =
        " 0"
        " NCFAILED=F"
        " ";

      text_modifier modifier(config);
      {
        REQUIRE_CALL(buff, write(_,_))
          .WITH(std::string(_1,_2) == "F")
          .TIMES(2)
          .RETURN(ssize_t(_2));
        modifier.write_to(writer, text_modifier::NCFAILED);
        modifier.write_to(writer, text_modifier::NCFAILED_SUM);
      }
      {
        REQUIRE_CALL(buff, write(_,_))
          .WITH(std::string(_1,_2) == "0")
          .RETURN(ssize_t(_2));
        modifier.write_to(writer, text_modifier::NORMAL);
      }

      modifier.write_to(writer, text_modifier::FAILED_SUM);
      modifier.write_to(writer, text_modifier::FAILED);
      modifier.write_to(writer, text_modifier::PASSED);
      modifier.write_to(writer, text_modifier::NCPASSED);
      modifier.write_to(writer, text_modifier::BLOCKED);
      modifier.write_to(writer, text_modifier::PASSED_SUM);
      modifier.write_to(writer, text_modifier::NCPASSED_SUM);
      modifier.write_to(writer, text_modifier::BLOCKED_SUM);
    }

    TEST(ncpassed_propagates, fix)
    {
      static const char config[] =
        " 0"
        " NCPASSED=P"
        " ";

      text_modifier modifier(config);
      {
        REQUIRE_CALL(buff, write(_,_))
          .WITH(std::string(_1,_2) == "P")
          .TIMES(2)
          .RETURN(ssize_t(_2));
        modifier.write_to(writer, text_modifier::NCPASSED);
        modifier.write_to(writer, text_modifier::NCPASSED_SUM);
      }
      {
        REQUIRE_CALL(buff, write(_,_))
          .WITH(std::string(_1,_2) == "0")
          .RETURN(ssize_t(_2));
        modifier.write_to(writer, text_modifier::NORMAL);
      }
      FORBID_CALL(buff, write(_,_));
      modifier.write_to(writer, text_modifier::NCFAILED);
      modifier.write_to(writer, text_modifier::NCFAILED_SUM);
      modifier.write_to(writer, text_modifier::FAILED_SUM);
      modifier.write_to(writer, text_modifier::FAILED);
      modifier.write_to(writer, text_modifier::PASSED);
      modifier.write_to(writer, text_modifier::BLOCKED);
      modifier.write_to(writer, text_modifier::PASSED_SUM);
      modifier.write_to(writer, text_modifier::BLOCKED_SUM);
    }

    TEST(blocked_propagates, fix)
    {
      static const char config[] =
        " 0"
        " BLOCKED=B"
        " ";

      text_modifier modifier(config);
      {
        REQUIRE_CALL(buff, write(_,_))
          .WITH(std::string(_1,_2) == "B")
          .TIMES(2)
          .RETURN(ssize_t(_2));
        modifier.write_to(writer, text_modifier::BLOCKED);
        modifier.write_to(writer, text_modifier::BLOCKED_SUM);
      }
      {
        REQUIRE_CALL(buff, write(_,_))
          .WITH(std::string(_1,_2) == "0")
          .RETURN(ssize_t(_2));
        modifier.write_to(writer, text_modifier::NORMAL);
      }
      FORBID_CALL(buff, write(_,_));
      modifier.write_to(writer, text_modifier::NCPASSED);
      modifier.write_to(writer, text_modifier::NCPASSED_SUM);
      modifier.write_to(writer, text_modifier::NCFAILED);
      modifier.write_to(writer, text_modifier::NCFAILED_SUM);
      modifier.write_to(writer, text_modifier::FAILED_SUM);
      modifier.write_to(writer, text_modifier::FAILED);
      modifier.write_to(writer, text_modifier::PASSED);
      modifier.write_to(writer, text_modifier::PASSED_SUM);
    }

    TEST(terminals_do_not_propagate, fix)
    {
        static const char config[] =
      " 0"
      " PASSED_SUM=PS"
      " FAILED_SUM=FS"
      " NCFAILED_SUM=NFS"
      " NCPASSED_SUM=NPS"
      " BLOCKED_SUM=BS"
      " ";

        text_modifier modifier(config);
        {
          REQUIRE_CALL(buff, write(_, _))
            .WITH(std::string(_1,_2) == "PS")
            .RETURN(ssize_t(_2));
          modifier.write_to(writer, text_modifier::PASSED_SUM);
        }
        {
          REQUIRE_CALL(buff, write(_, _))
            .WITH(std::string(_1,_2) == "FS")
            .RETURN(2);
          modifier.write_to(writer, text_modifier::FAILED_SUM);
        }
        {
          REQUIRE_CALL(buff, write(_, _))
            .WITH(std::string(_1, _2) == "BS")
            .RETURN(2);
          modifier.write_to(writer, text_modifier::BLOCKED_SUM);
        }
        {
          REQUIRE_CALL(buff, write(_, _))
            .WITH(std::string(_1,_2) == "NFS")
            .RETURN(ssize_t(_2));
          modifier.write_to(writer, text_modifier::NCFAILED_SUM);
        }
        {
          REQUIRE_CALL(buff, write(_, _))
            .WITH(std::string(_1, _2) == "NPS")
            .RETURN(ssize_t(_2));
          modifier.write_to(writer, text_modifier::NCPASSED_SUM);
        }
        {
          REQUIRE_CALL(buff, write(_, _))
            .WITH(std::string(_1, _2) == "0")
            .RETURN(ssize_t(_2));
          modifier.write_to(writer, text_modifier::NORMAL);
        }
        FORBID_CALL(buff, write(_,_));
        modifier.write_to(writer, text_modifier::PASSED);
        modifier.write_to(writer, text_modifier::FAILED);
        modifier.write_to(writer, text_modifier::NCFAILED);
        modifier.write_to(writer, text_modifier::NCPASSED);
        modifier.write_to(writer, text_modifier::BLOCKED);
    }

    TEST(variable_subset_length_throws)
    {
      ASSERT_THROW(text_modifier(" 0 BLOCKED_SU=apa "),
                   text_modifier::illegal_decoration_format&,
                   "BLOCKED_SU is not a decorator");
    }

    TEST(variable_superset_length_throws)
    {
      ASSERT_THROW(text_modifier(" 0 BLOCKED_SUMM=apa "),
                   text_modifier::illegal_decoration_format&,
                   "BLOCKED_SUMM is not a decorator");
    }

    TEST(lacking_assign_throws)
    {
      ASSERT_THROW(text_modifier(" 0 BLOCKED_SUM "),
                   text_modifier::illegal_decoration_format&,
                   "Missing = after name");
    }

    TEST(lacking_terminator_throws)
    {
      ASSERT_THROW(text_modifier(" 0 BLOCKED_SUM=apa"),
                   text_modifier::illegal_decoration_format&,
                   "Missing separator after value for BLOCKED_SUM");
    }
  }
}
