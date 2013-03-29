/*
 * Copyright 2009-2013 Bjorn Fahller <bjorn@fahller.se>
 * All rights reserved

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
#include "heap.hpp"

#include "wrapped/posix_encapsulation.hpp"
#ifdef HAVE_VALGRIND
#include <valgrind/valgrind.h>
#include <valgrind/memcheck.h>
namespace {
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wconversion"
#endif
  inline void valgrind_make_mem_noaccess(void *a, size_t  b) { VALGRIND_MAKE_MEM_NOACCESS(a, b);}
  inline void valgrind_make_mem_undefined(void *a, size_t b) { VALGRIND_MAKE_MEM_UNDEFINED(a, b);}
  inline void valgrind_make_mem_defined(void *a, size_t b) { VALGRIND_MAKE_MEM_DEFINED(a, b);}
#if defined(__GNUC__) && __GNUC_PREREQ(4,6)
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
  inline void valgrind_create_mempool(void *a,size_t b,bool c) { VALGRIND_CREATE_MEMPOOL(a,b,c);}
  inline void valgrind_mempool_free(void *a, void  *b) { VALGRIND_MEMPOOL_FREE(a, b);}
  inline void valgrind_mempool_alloc(void *a, void *b, size_t c) { VALGRIND_MEMPOOL_ALLOC(a, b, c);}
#if defined(__GNUC__) && __GNUC_PREREQ(4,6)
#pragma GCC diagnostic warning "-Wunused-but-set-variable"
#endif
#ifdef __GNUC__
#pragma GCC diagnostic warning "-Wconversion"
#endif
}
#else
namespace {
  template <typename A, typename B, typename C>
  inline void valgrind_create_mempool(A,B,C) {}
  template <typename A, typename B>
  inline void valgrind_make_mem_noaccess(A, B) {}
  template <typename A, typename B>
  inline void valgrind_make_mem_undefined(A, B) {}
  template <typename A, typename B, typename C, typename D>
  inline void valgrind_malloclike_block(A, B, C, D) {}
  template <typename A, typename B>
  inline void valgrind_make_mem_defined(A, B) {}
  template <typename A, typename B>
  inline void valgrind_freelike_block(A, B) {}
  template <typename A, typename B>
  inline void valgrind_mempool_free(A, B) {}
  template <typename A, typename B, typename C>
  inline void valgrind_mempool_alloc(A, B, C) {}
}
#endif

namespace {
  typedef enum { raw, by_malloc, by_new_elem, by_new_array } alloc_type;
  template <typename T, size_t N>
  T subscript(const T (&array)[N], size_t n)
  {
    return n >= N ? 0 : array[n];
  }
  const char* alloc_name(size_t t)
  {
    static const char *names[] = {
        "raw", "malloc", "new", "new[]"
    };
    const char *name = subscript(names, t);
    return name ? name : "<illegal>";
  }
  const char *free_name(size_t t)
  {
    static const char *names[] = {
        "raw", "free", "delete", "delete[]"
    };
    const char *name = subscript(names, t);
    return name ? name : "<illegal>";
  }
  inline void *zeromem(void *p, size_t n)
  {
    if (p)
      {
        char *d = static_cast<char *>(p);
        while (n--) *d++ = 0;
      }
    return p;
  }

  inline void *copymem(void *d, const void *s, size_t n)
  {
    char *dc = static_cast<char *>(d);
    const char *sc = static_cast<const char *>(s);
    while (n--) *dc++ = *sc++;
    return d;
  }

  bool backtrace_enabled = false;

}
namespace crpcut
{

  namespace heap
  {

    void enable_backtrace()
    {
      backtrace_enabled = true;
    }

    class new_handler_caller
    {
      typedef void (*bool_type)();
    public:
      new_handler_caller() throw () : handler(std::set_new_handler(0)) {}
      ~new_handler_caller() throw () { std::set_new_handler(handler); }
      void operator()() const { handler(); }
      operator bool_type () const throw () { return handler; }
    private:
      std::new_handler handler;
    };

    mem_list_element global_root = { &global_root, &global_root, 0, 0, 0, 0 };

    mem_list_element *local_root::current_root = &global_root;

    static mem_list_element *raw_alloc_mem(size_t s) throw ();

    namespace {
      const size_t num_elems = 50000;
      mem_list_element vector[num_elems];
      size_t current_offset = 0;

      size_t limit = heap::system;
      size_t bytes;
      size_t objects;

#ifdef USE_BACKTRACE
      class function
      {
      public:
        function(const char *name)
          : address(::dlsym(RTLD_DEFAULT, name))
        {
        }
        template <typename R, typename A, typename B>
        R call(A a, B b) const
        {
          union {
            R (*f)(A, B);
            void *p;
          } cheat;
          cheat.p = address;
          return cheat.f(a, b);
        }
        operator void*() const { return address; }
      private:
        void *address;
      };

      function backtrace("backtrace");
      function backtrace_symbols("backtrace_symbols");
#endif // USE_BACKTRACE

      void show_stack(std::ostringstream &msg,
                      const char         *header,
                      mem_list_element   *stack,
                      size_t              size)
      {
#ifdef USE_BACKTRACE
        if (!backtrace_enabled) return;
        msg << header;
        void *stack_addr = 0;
        if (stack) stack_addr = stack + 1;
        if (size == 0)
          {
            void *buffer[50];
            size = size_t(backtrace.call<int>(buffer, 50));
            stack_addr = buffer;
          }
        if (stack_addr && size)
          {
            void **bt = static_cast<void**>(stack_addr);
            valgrind_make_mem_defined(bt, size * sizeof(void*));
            char **alloc_stack = backtrace_symbols.call<char**>(bt, size);
            bool started = false;
            for (size_t i = 1; i < size; ++i)
              {
                const char *frame = alloc_stack[i];
                const char *in_libcrpcut = wrapped::strstr(frame,
                                                           "libcrpcut");
                if (!started && !in_libcrpcut)
                  {
                    started = true;
                    msg << '\n' << alloc_stack[i-1];
                  }
                if (started && in_libcrpcut) break;
                if (started) {
                  msg << '\n' << alloc_stack[i];
                }
              }
            free(alloc_stack);
          }
#else // shut up compiler warnings
        (void)msg;
        (void)header;
        (void)stack;
        (void)size;
#endif

    }

      void save_backtrace(mem_list_element *p)
      {
#ifdef USE_BACKTRACE
        if (control::is_enabled())
          {
            if (backtrace_enabled && backtrace)
              {
                static void* buffer[50];
                size_t const elems  = size_t(backtrace.call<int>(buffer, 50));
                size_t const ebytes = elems*sizeof(void*);
                if (mem_list_element *sr = raw_alloc_mem(ebytes))
                  {
                    sr->mem = ebytes;
                    sr->prev = sr->next = 0;
                    sr->type = raw;
                    void *addr = sr+1;
                    void **stack = static_cast<void**>(addr);
                    std::copy(buffer, buffer + elems, stack);
                    p->stack = sr;
                    p->stack_size = elems;
                  }
              }
          }
#else // shut up compiler warning
        (void)p;
#endif
      }

    }

    local_root::local_root(const local_root &orig)
      : mem_list_element(orig),
        location_(orig.location_),
        old_root_(orig.old_root_),
        check_type_(orig.check_type_)
    {
      orig.old_root_ = 0;
    }

    local_root::~local_root()
    {
      valgrind_make_mem_defined(this, sizeof(mem_list_element));
      current_root = old_root_;
      assert_empty();
      unlink();
    }
    mem_list_element* local_root::current()
    {
      return current_root;
    }


    void local_root::assert_empty() const
    {
      valgrind_make_mem_defined(const_cast<local_root*>(this), sizeof(*this));
      if (next == this) return;

      unsigned count = 0;
      for (mem_list_element *p = next; p != this;p = p->next)
        {
          valgrind_make_mem_defined(p, sizeof(mem_list_element));
          ++count;
        }
      std::ostringstream msg;
      msg << count << (count == 1 ? " object\n\n" : " objects\n\n");
      for (mem_list_element *p = next; p != this;)
        {
          mem_list_element *n = p->next;
          msg << p->mem << " byte";
          if (p->mem != 1) msg << 's';
          msg << " at " << p+1 << " allocated with " << alloc_name(p->type);
          show_stack(msg, "\nAllocated at:", p->stack, p->stack_size);
          valgrind_make_mem_undefined(p, sizeof(mem_list_element));
          if (n != this) msg << '\n';
          p = n;
        }
      if (check_type_ == comm::exit_fail)
        {
          comm::direct_reporter<comm::exit_fail>(location_)
            << "ASSERT_SCOPE_LEAK_FREE\n"
            << msg.str();
        }
      else
        {
          comm::direct_reporter<comm::fail>(location_)
            << "VERIFY_SCOPE_LEAK_FREE\n"
            << msg.str();
        }
      valgrind_make_mem_undefined(const_cast<local_root*>(this), sizeof(*this));
    }

    void mem_list_element::insert_last(mem_list_element *p)
    {
      valgrind_make_mem_defined(this, sizeof(mem_list_element));
      p->prev = prev;
      valgrind_make_mem_defined(p->prev, sizeof(mem_list_element));
      p->prev->next = p;
      prev = p;
      valgrind_make_mem_noaccess(p->prev, sizeof(mem_list_element));
      valgrind_make_mem_noaccess(this, sizeof(mem_list_element));
      p->next = this;
    }

    void mem_list_element::unlink()
    {
      valgrind_make_mem_defined(this, sizeof(mem_list_element));
      mem_list_element *next_elem = next;
      mem_list_element *prev_elem = prev;
      valgrind_make_mem_defined(next_elem, sizeof(mem_list_element));
      valgrind_make_mem_defined(prev_elem, sizeof(mem_list_element));
      if (next_elem)
        {
          next_elem->prev = prev_elem;
          prev_elem->next = next_elem;
        }
      valgrind_make_mem_noaccess(next_elem, sizeof(mem_list_element));
      valgrind_make_mem_noaccess(prev_elem, sizeof(mem_list_element));
      valgrind_make_mem_noaccess(this, sizeof(mem_list_element));
    }
    bool control::enabled;

    void control::enable()
    {
#ifdef USE_BACKTRACE
      void *p;
      if (backtrace) backtrace.call<int>(&p, 1);
#endif
      enabled = true;
    }
    static void alloc_type_check(mem_list_element *p, alloc_type type) throw ()
    {
      size_t current_type = p->type;
      if (current_type != size_t(type))
        {
          void *addr = p + 1;
          if (control::is_enabled())
            {
              std::ostringstream msg;
              show_stack(msg, "\nAlloc stack:", p->stack, p->stack_size);
              show_stack(msg, "\nNow at:", 0, 0);
              comm::direct_reporter<crpcut::comm::exit_fail>(crpcut_test_monitor::current_test()->get_location())
                << "DEALLOC FAIL\n"
                << free_name(type) << " " << addr << " was allocated using "
                << alloc_name(current_type)
                << msg.str();
            }
          else
            {
              static const char msg[]="alloc/dealloc type mismatch\n";
              wrapped::write(2, msg, sizeof(msg) - 1);
              wrapped::abort();
            }
        }
    }

    class recurse_counter
    {
    public:
      recurse_counter(int &count) : counter(count) { ++counter; }
      ~recurse_counter() { --counter; }
      operator const void*() const { return counter ? this : 0; }
    private:
      recurse_counter(const recurse_counter&);
      recurse_counter &operator=(const recurse_counter&);
      int &counter;
    };

    static mem_list_element *raw_alloc_mem(size_t s) throw ()
    {
      const size_t blocks = (s + sizeof(mem_list_element) - 1)/sizeof(mem_list_element) + 1;
      static bool has_malloc_sym = false;
      static int recursive = -1;
      recurse_counter recurse_checker(recursive);
      typedef libwrapper::loader<libs::rtld_next> loader;

      if (!use_local_heap)
        {
          has_malloc_sym |= !recursive && loader::has_symbol("malloc");
        }
      if (use_local_heap || !has_malloc_sym)
        {
          if (current_offset == 0)
            {
              valgrind_create_mempool(vector, sizeof(mem_list_element), 0);
              valgrind_make_mem_noaccess(vector, sizeof(vector));
            }
          mem_list_element *p = &vector[current_offset];
          current_offset += blocks + 1;
          return p;
        }
      const size_t size = s + 2*sizeof(mem_list_element);
      void *addr = crpcut::wrapped::malloc(size);
      mem_list_element *p = static_cast<mem_list_element*>(addr);
      return p;
    }


    class recursive_check : recurse_counter
    {
    public:
      recursive_check() : recurse_counter(count) {};
      using recurse_counter::operator const void*;
    private:
      static int count;
    };

    int recursive_check::count = -1;


    static void *alloc_mem(size_t s, alloc_type type) throw ()
    {
      recursive_check is_recursive;
      if (!is_recursive)
        {
          const size_t current_limit = limit;
          if (bytes + s > current_limit)
            {
              return 0;
            }
        }

      mem_list_element *p = raw_alloc_mem(s);
      if (p  != 0)
        {
          valgrind_make_mem_undefined(p, sizeof(mem_list_element));
          p->mem = s;
          p->stack = 0;
          p->type = type;
          if (is_recursive)
            {
              p->next = p->prev = 0;
            }
          else
            {
              mem_list_element *root = local_root::current();
              root->insert_last(p);
              bytes += s;
              ++objects;
              save_backtrace(p);
            }
          valgrind_make_mem_noaccess(p, sizeof(mem_list_element));
          ++p;
          valgrind_make_mem_undefined(p, s);
          valgrind_make_mem_noaccess(p + s, sizeof(mem_list_element));
          valgrind_mempool_alloc(vector, p, s);
        }
      return p;
    }

    static void free_mem_raw(mem_list_element *p) throw()
    {
      if (p >= vector && p < &vector[num_elems])
        {
          return;
        }
      crpcut::wrapped::free(p);
    }

    static void free_mem(void *addr, alloc_type expected) throw ()
    {
      if (!addr) return;

      using namespace crpcut::heap;

      mem_list_element *p = static_cast<mem_list_element*>(addr);
      --p;
      valgrind_make_mem_defined(p, sizeof(mem_list_element));
      alloc_type_check(p, expected);
      if (p->stack) free_mem_raw(p->stack);
      recursive_check is_recursive;
      if (!is_recursive)
        {
          bytes-= p->mem;
          --objects;
        }
      p->unlink();
      valgrind_mempool_free(vector, addr);
      free_mem_raw(p);
    }

    std::bad_alloc bad_alloc_exc;

    void *alloc_new_mem(size_t s, alloc_type type) throw (std::bad_alloc)
    {
      void *p = 0;
      for (;;)
        {
          new_handler_caller handler;
          p = crpcut::heap::alloc_mem(s, type);
          if (p) break;
          if (!handler) throw bad_alloc_exc;
          handler();
        }
      return p;
    }

    size_t set_limit(size_t n)
    {
      if (n < bytes)
        {
          if (control::is_enabled())
            {
              size_t now_bytes = bytes;
              comm::direct_reporter<crpcut::comm::exit_fail>(crpcut_test_monitor::current_test()->get_location())
                << "heap::set_limit(" << n
                << ") is below current use of " << now_bytes
                << " bytes";
            }
          else
            {
              static const char msg[]
                = "heap::set_limit() below current use\n";
              wrapped::write(2, msg, sizeof(msg) - 1);
              wrapped::abort();
            }
        }
      size_t rv = limit;
      limit = n;
      return rv;
    }
    size_t allocated_bytes()
    {
      return bytes;
    }
    size_t allocated_objects()
    {
      return objects;
    }
  }
}

extern "C"
{

  void *malloc(size_t s) throw ()
  {
    return crpcut::heap::alloc_mem(s, by_malloc);
  }

  void free(void *addr) throw ()
  {
    crpcut::heap::free_mem(addr, by_malloc);
  }

  void *calloc(size_t n, size_t s) throw ()
  {
    return zeromem(malloc(n*s), n*s);
  }

  void *realloc(void *addr, size_t s) throw ()
  {
    if (addr == 0) return malloc(s);
    if (s == 0)
      {
        free(addr);
        return 0;
      }
    crpcut::heap::mem_list_element *p = static_cast<crpcut::heap::mem_list_element*>(addr);
    valgrind_make_mem_defined(p - 1, sizeof(crpcut::heap::mem_list_element));
    crpcut::heap::alloc_type_check(p-1, by_malloc);
    const size_t block_size = p[-1].mem;
    valgrind_make_mem_noaccess(p - 1, sizeof(crpcut::heap::mem_list_element));
    if (s <= block_size) return addr;

    void *new_addr = malloc(s);
    if (new_addr)
      {
        copymem(new_addr, p, block_size);
        free(addr);
      }
    return new_addr;
  }

}

#if __cplusplus == 201103L
#define THROW(...)
#define NOTHROW noexcept
#else
#define THROW(...) throw (__VA_ARGS__)
#define NOTHROW throw ()
#endif
void *operator new(size_t s) THROW (std::bad_alloc)
{
  return crpcut::heap::alloc_new_mem(s, by_new_elem);
}

void *operator new(size_t s, const std::nothrow_t&) NOTHROW
{
  try {
    return crpcut::heap::alloc_new_mem(s, by_new_elem);
  }
  catch (std::bad_alloc&)  {
  }
  return 0;
}

void *operator new[](size_t s) THROW (std::bad_alloc)
{
  return crpcut::heap::alloc_new_mem(s, by_new_array);
}

void *operator new[](size_t s, const std::nothrow_t&) NOTHROW
{
  try {
    return crpcut::heap::alloc_new_mem(s, by_new_array);
  }
  catch (std::bad_alloc&)  {
  }
  return 0;
}

void operator delete[](void *p) NOTHROW
{
  crpcut::heap::free_mem(p, by_new_array);
}

void operator delete[](void *p, const std::nothrow_t&) NOTHROW
{
  crpcut::heap::free_mem(p, by_new_array);
}

void operator delete(void *p) NOTHROW
{
  crpcut::heap::free_mem(p, by_new_elem);
}

void operator delete(void *p, const std::nothrow_t&) NOTHROW
{
  crpcut::heap::free_mem(p, by_new_elem);
}

