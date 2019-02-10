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

#include "../buffer_vector.hpp"
#include <crpcut.hpp>

TESTSUITE(buffer_vector)
{
  class counted
  {
    counted();
  public:
    counted(int* p) : p_(p) { ++*p; }
    ~counted() { --*p_; }
    counted(const counted& p) : p_(p.p_) { ++*p_; }
    counted& operator=(const counted& p)
    {
      --*p_;
      p_ = p.p_;
      ++*p_;
      return *this;
    }
  private:
    int *p_;
  };

  TEST(construction_followed_by_destruction_does_nothing)
  {
    char s[3*sizeof(counted)];
    for (std::size_t i = 0; i < sizeof(s); ++i)
      {
        s[i] = char(i);
      }
    {
      crpcut::buffer_vector<counted> v(s+sizeof(counted), 1);
    }
    for (std::size_t i = 0; i < sizeof(s); ++i)
      {
        ASSERT_TRUE(std::size_t(s[i]) == i);
      }
  }

  TEST(push_back_to_allowed_size_has_no_side_effects,
       DEPENDS_ON(construction_followed_by_destruction_does_nothing))
   {
     static char buffer[sizeof(counted)*8];
     for (size_t i = 0U; i < sizeof(buffer); ++i)
       {
         buffer[i] = char(i);
       }
     static int counters[6];
     {
       crpcut::buffer_vector<counted> b(buffer + sizeof(counted), 6);
       for (size_t i = 0U; i < 6U; ++i)
         {
           b.push_back(counted(&counters[i]));
         }
       for (size_t i = 0U; i < 6U; ++i)
         {
           ASSERT_TRUE(counters[i] == 1);
         }
     }
     for (size_t i = 0U; i < 6U; ++i)
       {
         ASSERT_TRUE(counters[i] == 0);
       }
     for (size_t i = 0U; i < sizeof(counted); ++i)
       {
         ASSERT_TRUE(size_t(buffer[i]) == i);
         const size_t offset = 7*sizeof(counted);
         ASSERT_TRUE(size_t(buffer[i + offset]) == i + offset);
       }
   }

  TEST(pop_back_gives_pushed_values_in_reversed_order)
  {
    void *buffer = alloca(crpcut::buffer_vector<unsigned>::space_for(8));
    crpcut::buffer_vector<unsigned> b(buffer, 8);
    ASSERT_TRUE(b.size() == 0U);
    ASSERT_TRUE(b.push_back(1U) == 1U);
    ASSERT_TRUE(b.size() == 1U);
    ASSERT_TRUE(b.push_back(2U) == 2U);
    ASSERT_TRUE(b.size() == 2U);
    ASSERT_TRUE(b.push_back(3U) == 3U);
    ASSERT_TRUE(b.size() == 3U);
    ASSERT_TRUE(b.push_back(4U) == 4U);
    ASSERT_TRUE(b.size() == 4U);
    ASSERT_TRUE(b.push_back(5U) == 5U);
    ASSERT_TRUE(b.size() == 5U);
    ASSERT_TRUE(b.push_back(6U) == 6U);
    ASSERT_TRUE(b.size() == 6U);
    ASSERT_TRUE(b.push_back(7U) == 7U);
    ASSERT_TRUE(b.size() == 7U);
    ASSERT_TRUE(b.push_back(8U) == 8U);
    ASSERT_TRUE(b.size() == 8U);
    ASSERT_TRUE(b.back() == 8U);
    b.pop_back();
    ASSERT_TRUE(b.size() == 7U);
    ASSERT_TRUE(b.back() == 7U);
    b.pop_back();
    ASSERT_TRUE(b.size() == 6U);
    ASSERT_TRUE(b.back() == 6U);
    b.pop_back();
    ASSERT_TRUE(b.size() == 5U);
    ASSERT_TRUE(b.back() == 5U);
    b.pop_back();
    ASSERT_TRUE(b.size() == 4U);
    ASSERT_TRUE(b.back() == 4U);
    b.pop_back();
    ASSERT_TRUE(b.size() == 3U);
    ASSERT_TRUE(b.back() == 3U);
    b.pop_back();
    ASSERT_TRUE(b.size() == 2U);
    ASSERT_TRUE(b.back() == 2U);
    b.pop_back();
    ASSERT_TRUE(b.size() == 1U);
    ASSERT_TRUE(b.back() == 1U);
    b.pop_back();
    ASSERT_TRUE(b.size() == 0U);
  }
#ifndef NDEBUG
  TEST(constructing_with_null_space_aborts,
       DEPENDS_ON(construction_followed_by_destruction_does_nothing),
       EXPECT_SIGNAL_DEATH(SIGABRT),
       NO_CORE_FILE)
  {
    crpcut::buffer_vector<counted> b(nullptr, 1U);
  }

  TEST(construction_with_null_capacity_aborts,
       DEPENDS_ON(construction_followed_by_destruction_does_nothing),
       EXPECT_SIGNAL_DEATH(SIGABRT),
       NO_CORE_FILE)
  {
    char buffer[10];
    crpcut::buffer_vector<counted> b(buffer, 0U);
  }

  TEST(push_back_one_too_many_aborts,
       DEPENDS_ON(push_back_to_allowed_size_has_no_side_effects),
       EXPECT_SIGNAL_DEATH(SIGABRT),
       NO_CORE_FILE)
  {
    static char buffer[sizeof(size_t)*8];
    crpcut::buffer_vector<size_t> b(buffer, 8);
    for (size_t i = 0; i < 8; ++i)
      {
        b.push_back(i);
      }
    INFO << "This one should be OK";
    b.push_back(111U);
    FAIL << "shouldn't have reached here";
  }

  TEST(pop_back_on_empty_aborts,
       DEPENDS_ON(pop_back_gives_pushed_values_in_reversed_order),
       EXPECT_SIGNAL_DEATH(SIGABRT),
       NO_CORE_FILE)
  {
    static char buffer[sizeof(int)];
    crpcut::buffer_vector<int> b(buffer, 1);
    b.push_back(1);
    b.pop_back();
    INFO << "OK so far";
    b.pop_back();
    FAIL << "Shouldn't have gotten here";
  }

  TEST(back_on_empty_aborts,
       DEPENDS_ON(pop_back_gives_pushed_values_in_reversed_order),
       EXPECT_SIGNAL_DEATH(SIGABRT),
       NO_CORE_FILE)
  {
    static char buffer[sizeof(int)];
    crpcut::buffer_vector<int> b(buffer, 1);
    b.push_back(1);
    b.pop_back();
    INFO << "OK so far";
    b.back();
    FAIL << "Shouldn't have gotten here";
  }
#endif
}




