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

#include <crpcut.hpp>

template <typename T, size_t N>
size_t size(T (&)[N])
{
  return N;
}

TESTSUITE(list_elem)
{
  class C : public crpcut::datatypes::list_elem<C> {};
  TEST(a_default_constructed_list_is_empty)
  {
    C c;
    ASSERT_TRUE(c.is_empty());
    ASSERT_TRUE(c.next() == &c);
    ASSERT_TRUE(c.prev() == &c);
  }

  TEST(link_after_maintains_order)
  {
    C root;
    INFO << "&root=" << &root;
    ASSERT_TRUE(root.is_empty());

    C el1;
    INFO << "&el1=" << &el1;
    ASSERT_TRUE(el1.is_empty());

    root.link_after(el1);

    ASSERT_FALSE(el1.is_empty());
    ASSERT_FALSE(root.is_empty());

    ASSERT_TRUE(root.next() == &el1);
    ASSERT_TRUE(root.prev() == &el1);
    ASSERT_TRUE(el1.next() == &root);
    ASSERT_TRUE(el1.prev() == &root);

    C el2;
    INFO << "&el2=" << &el2;
    ASSERT_TRUE(el2.is_empty());

    el2.link_after(el1);

    ASSERT_FALSE(el2.is_empty());
    ASSERT_FALSE(el1.is_empty());
    ASSERT_FALSE(root.is_empty());

    ASSERT_TRUE(el1.next() == &el2);
    ASSERT_TRUE(el1.prev() == &root);
    ASSERT_TRUE(el2.next() == &root);
    ASSERT_TRUE(el2.prev() == &el1);
    ASSERT_TRUE(root.next() == &el1);
    ASSERT_TRUE(root.prev() == &el2);

    C el3;
    INFO << "&el3=" << &el3;
    ASSERT_TRUE(el3.is_empty());

    el3.link_after(el2);

    ASSERT_FALSE(el3.is_empty());
    ASSERT_FALSE(el2.is_empty());
    ASSERT_FALSE(el1.is_empty());
    ASSERT_FALSE(root.is_empty());

    ASSERT_TRUE(el2.next() == &el3);
    ASSERT_TRUE(el2.prev() == &el1);
    ASSERT_TRUE(el3.next() == &root);
    ASSERT_TRUE(el3.prev() == &el2);
    ASSERT_TRUE(root.next() == &el1);
    ASSERT_TRUE(root.prev() == &el3);
    ASSERT_TRUE(el1.next() == &el2);
    ASSERT_TRUE(el1.prev() == &root);
  }

  TEST(link_before_maintains_order)
  {
    C root;
    INFO << "&root=" << &root;
    ASSERT_TRUE(root.is_empty());

    C el1;
    INFO << "&el1=" << &el1;
    ASSERT_TRUE(el1.is_empty());

    root.link_before(el1);

    ASSERT_FALSE(el1.is_empty());
    ASSERT_FALSE(root.is_empty());

    ASSERT_TRUE(root.prev() == &el1);
    ASSERT_TRUE(root.next() == &el1);
    ASSERT_TRUE(el1.prev() == &root);
    ASSERT_TRUE(el1.next() == &root);

    C el2;
    INFO << "&el2=" << &el2;
    ASSERT_TRUE(el2.is_empty());

    el2.link_before(el1);

    ASSERT_FALSE(el2.is_empty());
    ASSERT_FALSE(el1.is_empty());
    ASSERT_FALSE(root.is_empty());

    ASSERT_TRUE(el1.prev() == &el2);
    ASSERT_TRUE(el1.next() == &root);
    ASSERT_TRUE(el2.prev() == &root);
    ASSERT_TRUE(el2.next() == &el1);
    ASSERT_TRUE(root.prev() == &el1);
    ASSERT_TRUE(root.next() == &el2);

    C el3;
    INFO << "&el3=" << &el3;
    ASSERT_TRUE(el3.is_empty());

    el3.link_before(el2);

    ASSERT_FALSE(el3.is_empty());
    ASSERT_FALSE(el2.is_empty());
    ASSERT_FALSE(el1.is_empty());
    ASSERT_FALSE(root.is_empty());

    ASSERT_TRUE(el2.prev() == &el3);
    ASSERT_TRUE(el2.next() == &el1);
    ASSERT_TRUE(el3.prev() == &root);
    ASSERT_TRUE(el3.next() == &el2);
    ASSERT_TRUE(root.prev() == &el1);
    ASSERT_TRUE(root.next() == &el3);
    ASSERT_TRUE(el1.prev() == &el2);
    ASSERT_TRUE(el1.next() == &root);
  }

  TEST(destruction_maintains_order)
  {
    C root;
    {
      C el1;
      el1.link_after(root);
      {
        C el2;
        el2.link_after(el1);
        {
          C el3;
          el3.link_after(el2);
          {
            C el4;
            el4.link_after(el1);

            ASSERT_FALSE(root.is_empty());
            ASSERT_FALSE(el1.is_empty());
            ASSERT_FALSE(el2.is_empty());
            ASSERT_FALSE(el3.is_empty());
            ASSERT_FALSE(el4.is_empty());

            ASSERT_TRUE(root.next() == &el1);
            ASSERT_TRUE(el1.next() == &el4);
            ASSERT_TRUE(el4.next() == &el2);
            ASSERT_TRUE(el2.next() == &el3);
            ASSERT_TRUE(el3.next() == &root);
            ASSERT_TRUE(root.prev() == &el3);
            ASSERT_TRUE(el3.prev() == &el2);
            ASSERT_TRUE(el2.prev() == &el4);
            ASSERT_TRUE(el4.prev() == &el1);
            ASSERT_TRUE(el1.prev() == &root);
          }
          ASSERT_FALSE(root.is_empty());
          ASSERT_FALSE(el1.is_empty());
          ASSERT_FALSE(el2.is_empty());
          ASSERT_FALSE(el3.is_empty());

          ASSERT_TRUE(root.next() == &el1);
          ASSERT_TRUE(el1.next() == &el2);
          ASSERT_TRUE(el2.next() == &el3);
          ASSERT_TRUE(el3.next() == &root);
          ASSERT_TRUE(root.prev() == &el3);
          ASSERT_TRUE(el3.prev() == &el2);
          ASSERT_TRUE(el2.prev() == &el1);
          ASSERT_TRUE(el1.prev() == &root);
        }
        ASSERT_FALSE(root.is_empty());
        ASSERT_FALSE(el1.is_empty());
        ASSERT_FALSE(el2.is_empty());

        ASSERT_TRUE(root.next() == &el1);
        ASSERT_TRUE(el1.next() == &el2);
        ASSERT_TRUE(el2.next() == &root);
        ASSERT_TRUE(root.prev() == &el2);
        ASSERT_TRUE(el2.prev() == &el1);
        ASSERT_TRUE(el1.prev() == &root);
      }
      ASSERT_FALSE(root.is_empty());
      ASSERT_FALSE(el1.is_empty());

      ASSERT_TRUE(root.next() == &el1);
      ASSERT_TRUE(el1.next() == &root);
      ASSERT_TRUE(root.prev() == &el1);
      ASSERT_TRUE(el1.prev() == &root);
    }
    ASSERT_TRUE(root.is_empty());

    ASSERT_TRUE(root.next() == &root);
    ASSERT_TRUE(root.prev() == &root);
  }

  TEST(two_lists_can_be_joined_with_link_after)
  {
    C list1;
    C l1e1;
    C l1e2;
    l1e1.link_after(list1);
    l1e2.link_after(l1e1);

    C list2;
    C l2e1;
    C l2e2;

    l2e1.link_after(list2);
    l2e2.link_after(l2e1);

    list2.link_after(l1e2);

    ASSERT_TRUE(list1.next() == &l1e1);
    ASSERT_TRUE(l1e1.next() == &l1e2);
    ASSERT_TRUE(l1e2.next() == &list2);
    ASSERT_TRUE(list2.next() == &l2e1);
    ASSERT_TRUE(l2e1.next() == &l2e2);
    ASSERT_TRUE(l2e2.next() == &list1);

    ASSERT_TRUE(list1.prev() == &l2e2);
    ASSERT_TRUE(l2e2.prev() == &l2e1);
    ASSERT_TRUE(l2e1.prev() == &list2);
    ASSERT_TRUE(list2.prev() == &l1e2);
    ASSERT_TRUE(l1e2.prev() == &l1e1);
    ASSERT_TRUE(l1e1.prev() == &list1);

  }

  TEST(two_lists_can_be_joined_with_link_before)
  {
    C list1;
    C l1e1;
    C l1e2;
    l1e1.link_before(list1);
    l1e2.link_before(l1e1);

    C list2;
    C l2e1;
    C l2e2;

    l2e1.link_before(list2);
    l2e2.link_before(l2e1);

    list2.link_before(l1e2);

    ASSERT_TRUE(list1.prev() == &l1e1);
    ASSERT_TRUE(l1e1.prev() == &l1e2);
    ASSERT_TRUE(l1e2.prev() == &list2);
    ASSERT_TRUE(list2.prev() == &l2e1);
    ASSERT_TRUE(l2e1.prev() == &l2e2);
    ASSERT_TRUE(l2e2.prev() == &list1);

    ASSERT_TRUE(list1.next() == &l2e2);
    ASSERT_TRUE(l2e2.next() == &l2e1);
    ASSERT_TRUE(l2e1.next() == &list2);
    ASSERT_TRUE(list2.next() == &l1e2);
    ASSERT_TRUE(l1e2.next() == &l1e1);
    ASSERT_TRUE(l1e1.next() == &list1);
  }
}
