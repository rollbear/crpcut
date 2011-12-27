#  Copyright 2009-2011 Bjorn Fahller <bjorn@fahller.se>
#  All rights reserved
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.

#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
#  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
#  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
#  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
#  SUCH DAMAGE.



cmake_minimum_required(VERSION 2.4)
if (COMMAND cmake_policy)
  cmake_policy(SET CMP0002 NEW)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)
project(crpcut)
include(CheckFunctionExists)
include(CheckLibraryExists)

set(CRPCUT_LIB_MAJOR_VERSION "1")
set(CRPCUT_LIB_MINOR_VERSION "4")
set(CRPCUT_LIB_PATCH_VERSION "9999")
set(CRPCUT_LIB_ALIAS_VERSION "1.5 wannabee")
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O")
set(CMAKE_CXX_FLAGS_MAINT "-ggdb -Wall -Wextra -Wcast-qual -Wconversion -Wshadow -Wunused-parameter -Wwrite-strings -pedantic -Wno-variadic-macros -D_GLIBCXX_DEBUG=1")
set(CRPCUT_LIB_VERSION "${CRPCUT_LIB_MAJOR_VERSION}.${CRPCUT_LIB_MINOR_VERSION}.${CRPCUT_LIB_PATCH_VERSION}")
add_definitions(-DCRPCUT_VERSION_STRING="${CRPCUT_LIB_VERSION}")
set(CRPCUT_LIB_SOVERSION ${CRPCUT_LIB_MAJOR_VERSION})
if (CRPCUT_LIB_ALIAS_VERSION)
  set(CRPCUT_PRINT_VERSION "${CRPCUT_LIB_VERSION} \\(a.k.a ${CRPCUT_LIB_ALIAS_VERSION}\\)")
else (CRPCUT_LIB_ALIAS_VERSION)
  set(CRPCUT_PRINT_VERSION "${CRPCUT_LIB_VERSION}")
endif (CRPCUT_LIB_ALIAS_VERSION)
set(SHAREDIR "share/crpcut-${CRPCUT_LIB_VERSION}")
file(GLOB LIB_SRCS
     src/check_name.cpp
     src/clocks.cpp
     src/collate_result.cpp
     src/comm_reporter.cpp
     src/convert_traits.cpp
     src/crpcut.cpp
     src/event.cpp
     src/fdreader.cpp
     src/fsfuncs.cpp
     src/heap.cpp
     src/namespace_info.cpp
     src/output.cpp
     src/output_buffer.cpp
     src/output_formatter.cpp
     src/pipe_pair.cpp
     src/policies.cpp
     src/poll_singleton.cpp
     src/posix_encapsulation.cpp
     src/posix_error.cpp
     src/presentation.cpp
     src/presentation_output.cpp
     src/presentation_reader.cpp
     src/printer.cpp
     src/report_reader.cpp
     src/scope.cpp
     src/tag.cpp
     src/tag_info.cpp
     src/tag_filter.cpp
     src/test_case_base.cpp
     src/test_case_factory.cpp
     src/test_case_registrator.cpp
     src/test_case_result.cpp
     src/test_suite_base.cpp
     src/test_wrapper.cpp
     src/text_formatter.cpp
     src/xml.cpp
     src/xml_formatter.cpp
)
include_directories(include)


if (NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX /usr)
endif (NOT CMAKE_INSTALL_PREFIX)

# Check for variadic macros


file(WRITE ${CMAKE_BINARY_DIR}/CMakeTmp/variadic_macro.cpp
  "#define APA(...) main(__VA_ARGS__)\nint APA(int argc, char *argv[]) {}\n")
try_compile(HAVE_VARIADIC_MACROS ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/CMakeTmp/variadic_macro.cpp)
if (NOT HAVE_VARIADIC_MACROS)
  message(FATAL_ERROR "${CMAKE_CXX_COMPILER} doesn't support variadic macros")
endif(NOT HAVE_VARIADIC_MACROS)

file(WRITE ${CMAKE_BINARY_DIR}/CMakeTmp/exit.cpp
  "#include <cstdlib>\nint main() { std::_Exit(0); }\n")
try_compile(HAVE__EXIT ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/CMakeTmp/exit.cpp)
if (NOT HAVE__EXIT)
  message(FATAL_ERROR "${CMAKE_CXX_COMPILER} doesn't support std::_Exit()")
endif(NOT HAVE__EXIT)

# Check for decltype and typeof


include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-std=c++0x HAVE_CXX0X)
file(WRITE ${CMAKE_BINARY_DIR}/CMakeTmp/typeof.cpp
  "template <typename T> T* func(T);\n"
  "typedef typeof(func(3.14)) apa;\n"
  "int main() {}\n")
try_compile(HAVE_TYPEOF ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/CMakeTmp/typeof.cpp)
if (NOT HAVE_TYPEOF)
  if (NOT HAVE_CXX0X)
    mesage(FATAL_ERROR
      "${CMAKE_CXX_COMPILER} supports neither decltype nor typeof")
  endif (NOT HAVE_CXX0X)
endif (NOT HAVE_TYPEOF)

# check for TR1

file(WRITE ${CMAKE_BINARY_DIR}/CMakeTmp/tr1.cpp
  "#include <tr1/type_traits>\n"
  "const bool v = std::tr1::is_reference<int>::type::value;\n"
  "int main() { return v; }\n")
try_compile(HAVE_TR1 ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/CMakeTmp/tr1.cpp)
if(NOT HAVE_TR1)
  file(WRITE ${CMAKE_BINARY_DIR}/CMakeTmp/boost_tr1.cpp
    "#include <boost/tr1/type_traits.hpp>\n"
    "const bool v = std::tr1::is_reference<int>::type::value;\n"
    "int main() { return v; }\n")
  try_compile(HAVE_BOOST_TR1 ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/CMakeTmp/boost_tr1.cpp)
  if (NOT HAVE_BOOST_TR1)
    message(FATAL_ERROR "${CMAKE_CXX_COMPILER} doesn't have std/tr1 nor boost/tr1")
  endif (NOT HAVE_BOOST_TR1)
endif(NOT HAVE_TR1)

# check for valgrind

file(WRITE ${CMAKE_BINARY_DIR}/CMakeTmp/valgrind.cpp
  "#include <valgrind/valgrind.h>\n"
  "int main(int argc, char *argv[])\n"
  "{\n"
  "  VALGRIND_MALLOCLIKE_BLOCK(argv, 4, 0, 0);\n"
  "  return 0;\n"
  "}\n")
try_compile(HAVE_VALGRIND ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/CMakeTmp/valgrind.cpp)

# check for backtrace

check_function_exists("backtrace" USE_BACKTRACE)
if (USE_BACKTRACE)
  add_definitions(-DUSE_BACKTRACE)
  message("*** using backtrace() for information on heap objects")
endif (USE_BACKTRACE)

# check for clock_gettime()

set(EXTRA_LIBS "${EXTRA_LIBS} -l${CMAKE_DL_LIBS}")
check_function_exists("clock_gettime()" HAVE_CLOCK_GETTIME)
if(HAVE_CLOCK_GETTIME)
  add_definitions(-DHAVE_CLOCK_GETTIME=libc)
else(HAVE_CLOCK_GETTIME)
  check_library_exists("rt" "clock_gettime" "" RT_HAS_CLOCK_GETTIME)
  if(RT_HAS_CLOCK_GETTIME)
    set(EXTRA_LIBS "-lrt ${EXTRA_LIBS}")
    set(HAVE_CLOCK_GETTIME 1)
    add_definitions(-DHAVE_CLOCK_GETTIME=librt)
  endif(RT_HAS_CLOCK_GETTIME)
endif(HAVE_CLOCK_GETTIME)
if(HAVE_CLOCK_GETTIME)
  set(MAY_HAVE_MONO 1)
  set(MAY_HAVE_CPU 1)
  set(HAVE_CLOCK 1)
endif(HAVE_CLOCK_GETTIME)

# check for mach_absolute_time()

check_function_exists("mach_absolute_time" HAVE_MACH_ABSOLUTE_TIME)
if(HAVE_MACH_ABSOLUTE_TIME)
  set(MAY_HAVE_MONO 1)
  set(HAVE_CLOCK 1)
  add_definitions(-DHAVE_MACH_ABSOLUTE_TIME)
endif(HAVE_MACH_ABSOLUTE_TIME)


# check for getitimer()

check_function_exists("getitimer" HAVE_ITIMER)
if(HAVE_ITIMER)
  add_definitions(-DHAVE_ITIMER)
  set(MAY_HAVE_CPU 1)
  set(HAVE_CLOCK 1)
endif(HAVE_ITIMER)

# check for gettimeofday()

check_function_exists("gettimeofday" HAVE_GETTIMEOFDAY)
if(HAVE_GETTIMEOFDAY)
  add_definitions(-DHAVE_GETTIMEOFDAY)
  set(HAVE_CLOCK 1)
endif(HAVE_GETTIMEOFDAY)


if (NOT HAVE_CLOCK)
  message(FATAL_ERROR "Velly velly solly, but you have no clocks!")
endif (NOT HAVE_CLOCK)
if (NOT MAY_HAVE_CPU)
  message(FATAL_ERROR "Velly velly solly, but I can't measure CPU-time")
endif (NOT MAY_HAVE_CPU)
if (NOT MAY_HAVE_MONO)
  message("Monotonic time source unavailable - using realtime clocks!")
endif (NOT MAY_HAVE_MONO)


# check google-mock
if (WITH_GOOGLE_MOCK)
  if (NOT GOOGLE_MOCK_DIR)
    find_library(GOOGLE_MOCK_LIB "gmock")
    if (NOT GOOGLE_MOCK_LIB)
      message(FATAL_ERROR
        "Can't find google mock, try to add -DGOOGLE_MOCK_DIR=<path> to the command line")
    endif (NOT GOOGLE_MOCK_LIB)
    set(GMOCK_LIBS "-lgmock -lgtest")
  else (NOT GOOGLE_MOCK_DIR)
    include_directories(AFTER ${GOOGLE_MOCK_DIR}/include)
    set(GMOCK_LIBS "-Wl,-rpath,${CMAKE_BINARY_DIR}/lib:${GOOGLE_MOCK_DIR}/lib -L${GOOGLE_MOCK_DIR}/lib -lgmock -lgtest")
  endif (NOT GOOGLE_MOCK_DIR)
  file(GLOB GMOCK_LIB_SRCS src/gmock.cpp)
  set(SELFTEST_PARAM ${SELFTEST_PARAM} gmock)

  file(WRITE ${CMAKE_BINARY_DIR}/CMakeTmp/gmock.cpp
    "#include <gmock/gmock.h>\nclass C : public ::testing::EmptyTestEventListener {};\nint main() {\n  C obj;\n  ::testing::UnitTest::GetInstance()->listeners().Append(&obj);\n}\n")
  try_compile(HAVE_GMOCK ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/CMakeTmp/gmock.cpp
    CMAKE_FLAGS -DINCLUDE_DIRECTORIES=${GOOGLE_MOCK_DIR}/include
    -DLINK_DIRECTORIES=${GOOGLE_MOCK_DIR}/lib
    -DLINK_LIBRARIES=gtest)
  if (NOT HAVE_GMOCK)
    set(GMOCK_PTHREAD_DIR "gtest -lpthread")
    try_compile(HAVE_GMOCK ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/CMakeTmp/gmock.cpp
    CMAKE_FLAGS -DINCLUDE_DIRECTORIES=${GOOGLE_MOCK_DIR}/include
    -DLINK_DIRECTORIES=${GOOGLE_MOCK_DIR}/lib
    -DLINK_LIBRARIES=${GMOCK_PTHREAD_DIR})
    if (NOT HAVE_GMOCK)
      message(FATAL_ERROR "Unknown version of Google Mock")
    endif (NOT HAVE_GMOCK)
    set(GMOCK_LIBS "${GMOCK_LIBS} -lpthread")
  endif (NOT HAVE_GMOCK)

endif (WITH_GOOGLE_MOCK)

include_directories(include)

add_library(crpcut SHARED ${LIB_SRCS})
set_target_properties(crpcut PROPERTIES
 VERSION ${CRPCUT_LIB_VERSION}
 SOVERSION ${CRPCUT_LIB_SOVERSION})

if(GMOCK_LIBS)
  file(GLOB GMOCK_TEST_SRCS   test-src/gmock.cpp)
endif(GMOCK_LIBS)


file(GLOB TEST_SRCS
     test-src/unitt.cpp
     test-src/asserts_and_depends.cpp
     test-src/verify.cpp
     test-src/output.cpp
     test-src/timeouts.cpp
     test-src/death_by_exit.cpp
     test-src/death_by_signal.cpp
     test-src/death_by_exception.cpp
     test-src/parametrized.cpp
     test-src/wrapped.cpp
     test-src/ext_parameters.cpp
     test-src/predicates.cpp
     test-src/regex.cpp
     test-src/fp.cpp
     test-src/collate.cpp
     test-src/suite_deps1.cpp
     test-src/suite_deps2.cpp
     test-src/heap.cpp
     test-src/bad_forks.cpp
     ${GMOCK_TEST_SRCS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FEATURE_SELECTION}")
add_executable(testprog EXCLUDE_FROM_ALL ${TEST_SRCS})
if (GMOCK_LIBS)
  target_link_libraries(testprog crpcut "${GMOCK_LIBS} ${EXTRA_LIBS}")
else (GMOCK_LIBS)
  target_link_libraries(testprog crpcut ${EXTRA_LIBS})
endif (GMOCK_LIBS)

find_program(AWK "awk")
find_program(BASH "bash")
find_program(XMLLINT "xmllint")

if (WITH_HTML_DOCS)

  add_custom_target(can_dump_core
    ulimit -c | egrep -q "\\(Unlimited\\)\\|\\[1-9\\]" && touch can_dump_core)
  find_program(XSLTPROC "xsltproc")
  find_program(SED "sed")
  file(GLOB HTML_CHUNK_XSL
    /usr/share/xml/docbook/stylesheet/docbook-xsl-ns/html/chunk.xsl)
  if (NOT HTML_CHUNK_XSL)
    file(GLOB HTML_CHUNK_XSL
      /usr/share/sgml/docbook/xsl-ns-stylesheets/html/chunk.xsl)
  endif(NOT HTML_CHUNK_XSL)
  find_program(GDB "gdb")
  if (NOT XSLTPROC)
    message("xsltproc is not found - Can't build documentation")
  else (NOT XSLTPROC)
    if (NOT HTML_CHUNK_XSL)
      message("docbook-xsl-ns not found, can't build docs")
    else (NOT HTML_CHUNK_XSL)
      if (NOT GDB)
        message("gdb not found, can't build docs")
      else(NOT GDB)
        if (NOT SED)
          message("sed not found, can't build docs")
        else (NOT SED)
          if (NOT HAVE_GMOCK)
            message("google-mock not found, can't build docs")
          endif(NOT HAVE_GMOCK)
        endif(NOT SED)
      endif (NOT GDB)
    endif(NOT HTML_CHUNK_XSL)
  endif(NOT XSLTPROC)

  if(XSLTPROC AND HTML_CHUNK_XSL AND GDB AND SED AND HAVE_GMOCK)
    file(GLOB DOC_SRC_LIB "doc-src")
    file(GLOB DOC_SOURCES "${DOC_SRC_LIB}/*.xml")
    file(GLOB TOP_DOC_SOURCE "${DOC_SRC_LIB}/docs.xml")
    add_custom_target(
      stripped-src
      test -d stripped-src || mkdir stripped-src
      )
    add_custom_target(
      doc-bin
      test -d doc-bin || mkdir doc-bin
      )
    add_custom_target(
      sample-output
      test -d sample-output || mkdir sample-output
      )
    add_custom_target(
      html
      test -d html || mkdir html
      )

    add_custom_target(
      refman
      test -d html/refman || mkdir html/refman
      DEPENDS html
      )

    set(STRIP_SOURCES sed "0,/^$/D\\;s/\\(.*\\)/     \\1/g")

    function(sample_code src_name)
      set(COMPILE_PROGRAM 1)
      set (OPTION 1)
      foreach(f ${ARGN})
        if (DEFINED OPTION)
          unset(OPTION)
          if ("${f}" STREQUAL "NOCOMPILE")
            unset(COMPILE_PROGRAM)
          elseif ("${f}" STREQUAL "COMPILE_OPT")
            set(X_COMPILE_OPT 1)
          elseif("${f}" STREQUAL "LIBS")
            set(X_LIBS 1)
          else()
            message(FATAL_ERROR
              "Got \"${f}\": Expected COMPILE_OPT")
          endif()
        else()
          if (DEFINED X_COMPILE_OPT)
            SET(COMPILE_OPT ${f})
            unset(X_COMPILE_OPT)
          elseif(DEFINED X_LIBS)
            set(LINK_LIBS ${f})
            unset(X_LIBS)
          else()
            message(FATAL_ERROR "What now?")
          endif()
          set(OPTION 1)
        endif()
      endforeach()
      GET_FILENAME_COMPONENT(basename ${src_name} NAME_WE)
      message("basename=\"${basename}\"")
      if (${basename} STREQUAL ${src_name})
        set(filename "${src_name}.cpp")
      else()
        set(filename "${src_name}")
      endif()
      file(GLOB SRC "${DOC_SRC_LIB}/samples/${filename}")
      add_custom_target(
        "${filename}"
        ${STRIP_SOURCES} < ${SRC} > ${filename}
        DEPENDS stripped-src ${SRC}
        WORKING_DIRECTORY stripped-src
        VERBATIM)
      if (DEFINED COMPILE_PROGRAM)
        add_executable(${basename} EXCLUDE_FROM_ALL ${SRC})
        if (DEFINED COMPILE_OPT)
          set_target_properties(
            "${basename}"
            PROPERTIES
            COMPILE_FLAGS ${COMPILE_OPT})
        endif()
        target_link_libraries(${basename} crpcut ${LINK_LIBS} ${EXTRA_LIBS})
      endif()
    endfunction(sample_code)

    function(sample_output program)
      foreach(f ${ARGN})
        if ("${f}" STREQUAL "OUTNAME")
          set(X_OUTNAME 1)
          unset(X_PARAMS)
        elseif("${f}" STREQUAL "PARAMS")
          set(X_PARAMS 1)
        elseif("${f}" STREQUAL "FILTER")
          unset(X_PARAMS)
          set(X_FILTER 1)
        elseif (DEFINED X_OUTNAME)
          set(OUTNAME ${f})
        elseif(DEFINED X_PARAMS)
          if (DEFINED PARAMS)
            set(PARAMS "${PARAMS};${f}")
          else()
            set(PARAMS ${f})
          endif()
        elseif(DEFINED X_FILTER)
          set(FILTER ${f})
        else()
          message(FATAL_ERROR
            "Got \"${f}\": Expected one of OUTNAME|PARAMS|FILTER")
        endif()
      endforeach()
      if (NOT DEFINED OUTNAME)
        set (OUTNAME "${program}.out")
      endif()
      if (NOT DEFINED FILTER)
        set (FILTER "s/\\(.*\\)/     \\1/g")
      endif()
      add_custom_target(
        "${OUTNAME}"
        ${program} ${PARAMS} | c++filt -t | sed ${FILTER} > "${OUTNAME}"
        DEPENDS ${program}
        WORKING_DIRECTORY sample-output
        VERBATIM)
    endfunction(sample_output)

    function(sample_result name)
      sample_code(${name})
      sample_output(${name} ${ARGN})
    endfunction(sample_result)

    sample_result(all_tests)
    sample_result(exit_any_code)
    sample_code(assert_eq_fails COMPILE_OPT "-ggdb")
    sample_output(assert_eq_fails)

    file(GLOB DEBUG_CMD "${DOC_SRC_LIB}/samples/debug_cmd")
    add_custom_target(
      debug_assert_eq_fails.out
      ${GDB} ../test/assert_eq_fails  < ${DEBUG_CMD} | sed "1,1s/^.*\\(GNU.*\\)/bash > gdb .\\/test\\/assert_eq_fails\\n\\1/g;s/\\((gdb).*\\)(gdb) \\(.*\\)$/\\1\\n\\2/g;s/^\\(..\\)\\((gdb).*\\)/\\1\\n\\2/g;s/\\[answered.*//g"  > debug_assert_eq_fails.out
      DEPENDS sample-output assert_eq_fails
      WORKING_DIRECTORY sample-output
      VERBATIM)

    sample_result(verify_eq_fails)
    sample_result(assert_ge_fails)

    sample_result(verify_ge_fails)
    sample_result(assert_le_fails)
    sample_result(verify_le_fails)
    sample_result(assert_lt_fails)
    sample_result(verify_lt_fails)
    sample_result(assert_gt_fails)
    sample_result(verify_gt_fails)
    sample_result(assert_ne_fails)
    sample_result(verify_ne_fails)
    sample_result(assert_false_fails)
    sample_result(verify_false_fails)
    sample_result(assert_true_fails)
    sample_result(describe_exception)
    sample_result(verify_true_fails)
    sample_result(assert_no_throw_fails)
    sample_result(verify_no_throw_fails)
    sample_result(assert_throw_fails)
    sample_result(verify_throw_fails)
    sample_result(assert_pred_fails)
    sample_result(verify_pred_fails)
    sample_result(assert_scope_max_cpu)
    sample_result(verify_scope_max_cpu)
    sample_result(assert_scope_max_real)
    sample_result(verify_scope_max_real)
    sample_result(assert_scope_min_real)
    sample_result(verify_scope_min_real)
    sample_result(assert_scope_leak_free)
    sample_result(verify_scope_leak_free)
    sample_result(usleep)
    sample_result(wrap_func)
    sample_result(wrap_v_func)
    sample_result(wrapped)
    sample_result(collate)
    sample_result(collate_case)
    sample_result(deadline_cpu)
    sample_result(deadline_realtime)
    sample_result(depends_on)
    sample_result(expect_exception)
    sample_result(expect_exit)
    sample_result(expect_signal_death)
    sample_result(expect_realtime_timeout)
    sample_result(fail)
    sample_result(fixture_construction_timeout)
    sample_result(fixture_destruction_timeout)
    sample_result(info)
    sample_result(no_core)
    sample_result(infinite_loop)
    sample_result(file_fail)
    sample_result(ambiguous_fix)
    sample_result(simple_fix)
    sample_result(file_fix)
    sample_result(abs_diff)
    sample_result(relative_diff)
    sample_result(ulps_diff)
    sample_result(regex)
    sample_result(bare-minimum)
    sample_output(bare-minimum PARAMS "--list"
                               OUTNAME bare-minimum-list.out)
    sample_output(bare-minimum PARAMS "--verbose"
                               OUTNAME bare-minimum-verbose.out)
    sample_result(iso8859_1_code PARAMS "--verbose" "--output-charset=UTF8")
    sample_result(utf8_code PARAMS "--verbose" "--output-charset=UTF8")
    sample_result(simple_testsuite)
    sample_result(test_tag PARAMS "--tags=-known_bug/uncertain")
    sample_code(report_example)
    sample_output(report_example PARAMS "--verbose" "--tags=/another_tag"
                                 OUTNAME "report_example_human.out")
    sample_output(report_example PARAMS "--xml=yes" "-verbose" "--tags=/another_tag"
                                 OUTNAME "report_example_xml.out")
    sample_code(ilist_element.hpp NOCOMPILE)

    sample_result(itest)
    sample_code(symtable.hpp NOCOMPILE)
    sample_result(symtable-test PARAMS "-v")
    sample_result(symtable-test2)
    sample_result(suite-example PARAMS "-v" "--nodeps" "normal_access")
    sample_result(string-template-fix PARAMS "-v")
    sample_result(regex-simple PARAMS "-v")
    sample_code(sorted-names.hpp NOCOMPILE)
    sample_result(collate_example PARAMS "-v")

    sample_code(stream-cast.hpp NOCOMPILE)
    sample_result(heap-check)

    sample_code(process-example.hpp NOCOMPILE)
    sample_result(process-test)

    sample_result(simple-pred)
    sample_result(template-pred)
    sample_code(gmock-example LIBS ${GMOCK_LIBS})
    sample_output(gmock-example)

    sample_result(simple-depend)
    sample_code(suite_depend.cpp NOCOMPILE)
    sample_code(suite_define.cpp NOCOMPILE)
    #
    # suite_depend
    #
    file(GLOB SUITE_DEPEND "${DOC_SRC_LIB}/samples/suite_depend.cpp")
    file(GLOB SUITE_DEFINE "${DOC_SRC_LIB}/samples/suite_define.cpp")
    add_executable(suite_depend EXCLUDE_FROM_ALL
      ${SUITE_DEPEND} ${SUITE_DEFINE})
    target_link_libraries(suite_depend crpcut ${EXTRA_LIBS})
    add_custom_target(
      suite_depend.out
      suite_depend | sed "s/\\(.*\\)/     \\1/g" > suite_depend.out
      DEPENDS suite_depend sample-output
      WORKING_DIRECTORY sample-output
      VERBATIM)

    add_custom_target(
      doc-src
      test -L doc-src || ln -s ${DOC_SRC_LIB} doc-src
      )

    add_custom_target(
      crpcut.xsl
      cat ${DOC_SRC_LIB}/crpcut-template.xsl | sed "s?CHUNK?${HTML_CHUNK_XSL}?g\;s?PVERSION?${CRPCUT_PRINT_VERSION}?g\;s?VERSION?${CRPCUT_LIB_VERSION}?g" > crpcut.xsl
      )

    #
    # html docs
    #
    add_custom_target(
      html_docs ALL
      ${XSLTPROC}
      --xinclude
      ${CMAKE_BINARY_DIR}/crpcut.xsl - < ${TOP_DOC_SOURCE}
      DEPENDS html stripped-src
      doc-src
      stripped-src/all_tests.cpp sample-output/all_tests.out
      stripped-src/exit_any_code.cpp sample-output/exit_any_code.out
      stripped-src/assert_ne_fails.cpp sample-output/assert_ne_fails.out
      stripped-src/verify_ne_fails.cpp sample-output/verify_ne_fails.out
      stripped-src/assert_le_fails.cpp sample-output/assert_le_fails.out
      stripped-src/verify_le_fails.cpp sample-output/verify_le_fails.out
      stripped-src/assert_lt_fails.cpp sample-output/assert_lt_fails.out
      stripped-src/verify_lt_fails.cpp sample-output/verify_lt_fails.out
      stripped-src/assert_eq_fails.cpp sample-output/assert_eq_fails.out
      stripped-src/verify_eq_fails.cpp sample-output/verify_eq_fails.out
      stripped-src/assert_ge_fails.cpp sample-output/assert_ge_fails.out
      stripped-src/verify_ge_fails.cpp sample-output/verify_ge_fails.out
      sample-output/debug_assert_eq_fails.out
      stripped-src/assert_gt_fails.cpp sample-output/assert_gt_fails.out
      stripped-src/verify_gt_fails.cpp sample-output/verify_gt_fails.out
      stripped-src/assert_false_fails.cpp sample-output/assert_false_fails.out
      stripped-src/verify_false_fails.cpp sample-output/verify_false_fails.out
      stripped-src/assert_true_fails.cpp sample-output/assert_true_fails.out
      stripped-src/describe_exception.cpp sample-output/describe_exception.out
      stripped-src/verify_true_fails.cpp sample-output/verify_true_fails.out
      stripped-src/assert_no_throw_fails.cpp sample-output/assert_no_throw_fails.out
      stripped-src/verify_no_throw_fails.cpp sample-output/verify_no_throw_fails.out
      stripped-src/assert_throw_fails.cpp sample-output/assert_throw_fails.out
      stripped-src/verify_throw_fails.cpp sample-output/verify_throw_fails.out
      stripped-src/assert_pred_fails.cpp sample-output/assert_pred_fails.out
      stripped-src/verify_pred_fails.cpp sample-output/verify_pred_fails.out
      stripped-src/assert_scope_max_cpu.cpp sample-output/assert_scope_max_cpu.out
      stripped-src/verify_scope_max_cpu.cpp sample-output/verify_scope_max_cpu.out
      stripped-src/assert_scope_max_real.cpp sample-output/assert_scope_max_real.out
      stripped-src/verify_scope_max_real.cpp sample-output/verify_scope_max_real.out
      stripped-src/assert_scope_min_real.cpp sample-output/assert_scope_min_real.out
      stripped-src/verify_scope_min_real.cpp sample-output/verify_scope_min_real.out
      stripped-src/assert_scope_leak_free.cpp sample-output/assert_scope_leak_free.out
      stripped-src/verify_scope_leak_free.cpp sample-output/verify_scope_leak_free.out
      stripped-src/usleep.cpp sample-output/usleep.out
      stripped-src/wrap_func.cpp sample-output/wrap_func.out
      stripped-src/wrap_v_func.cpp sample-output/wrap_v_func.out
      stripped-src/wrapped.cpp sample-output/wrapped.out
      stripped-src/collate.cpp sample-output/collate.out
      stripped-src/collate_case.cpp sample-output/collate_case.out
      stripped-src/deadline_cpu.cpp sample-output/deadline_cpu.out
      stripped-src/deadline_realtime.cpp sample-output/deadline_realtime.out
      stripped-src/depends_on.cpp sample-output/depends_on.out
      stripped-src/expect_exception.cpp sample-output/expect_exception.out
      stripped-src/expect_exit.cpp sample-output/expect_exit.out
      stripped-src/expect_signal_death.cpp sample-output/expect_signal_death.out
      stripped-src/expect_realtime_timeout.cpp sample-output/expect_realtime_timeout.out
      stripped-src/fixture_construction_timeout.cpp sample-output/fixture_construction_timeout.out
      stripped-src/fixture_destruction_timeout.cpp sample-output/fixture_destruction_timeout.out
      stripped-src/fail.cpp sample-output/fail.out
      stripped-src/info.cpp sample-output/info.out
      stripped-src/no_core.cpp sample-output/no_core.out
      stripped-src/abs_diff.cpp sample-output/abs_diff.out
      stripped-src/relative_diff.cpp sample-output/relative_diff.out
      stripped-src/ulps_diff.cpp sample-output/ulps_diff.out
      stripped-src/regex.cpp sample-output/regex.out
      stripped-src/bare-minimum.cpp sample-output/bare-minimum.out
      sample-output/bare-minimum-list.out
      sample-output/bare-minimum-verbose.out
      stripped-src/utf8_code.cpp sample-output/utf8_code.out
      stripped-src/iso8859_1_code.cpp sample-output/iso8859_1_code.out
      stripped-src/simple_testsuite.cpp sample-output/simple_testsuite.out
      stripped-src/test_tag.cpp sample-output/test_tag.out
      stripped-src/report_example.cpp sample-output/report_example_human.out
      sample-output/report_example_xml.out
      stripped-src/ilist_element.hpp
      stripped-src/itest.cpp sample-output/itest.out
      stripped-src/simple-depend.cpp sample-output/simple-depend.out
      stripped-src/simple_fix.cpp sample-output/simple_fix.out
      stripped-src/file_fix.cpp sample-output/file_fix.out
      stripped-src/ambiguous_fix.cpp sample-output/ambiguous_fix.out
      stripped-src/file_fail.cpp sample-output/file_fail.out
      stripped-src/infinite_loop.cpp sample-output/infinite_loop.out
      stripped-src/suite_depend.cpp sample-output/suite_depend.out
      stripped-src/suite_define.cpp
      stripped-src/symtable.hpp
      stripped-src/symtable-test.cpp sample-output/symtable-test.out
      stripped-src/symtable-test2.cpp sample-output/symtable-test2.out
      stripped-src/suite-example.cpp sample-output/suite-example.out
      stripped-src/string-template-fix.cpp sample-output/string-template-fix.out
      stripped-src/regex-simple.cpp sample-output/regex-simple.out
      stripped-src/collate_example.cpp sample-output/collate_example.out
      stripped-src/sorted-names.hpp
      stripped-src/heap-check.cpp sample-output/heap-check.out
      stripped-src/stream-cast.hpp
      stripped-src/process-example.hpp stripped-src/process-test.cpp
      sample-output/process-test.out
      stripped-src/simple-pred.cpp sample-output/simple-pred.out
      stripped-src/template-pred.cpp sample-output/template-pred.out
      stripped-src/gmock-example.cpp sample-output/gmock-example.out
      can_dump_core
      ${DOC_SOURCES} ${DOC_SRC_LIB}/crpcut-doc.css crpcut.xsl
      WORKING_DIRECTORY html)

   add_custom_target(
     homepage-${CRPCUT_LIB_VERSION}.html
     sed "s:PVERSION:${CRPCUT_PRINT_VERSION}:g\;s:VERSION:${CRPCUT_LIB_VERSION}:g" < ${DOC_SRC_LIB}/homepage.html > homepage-${CRPCUT_LIB_VERSION}.html
     DEPENDS ${DOC_SRC_LIB}/homepage.html
     )
   add_custom_target(
     homepage-${CRPCUT_LIB_VERSION}.css
     test -L homepage-${CRPCUT_LIB_VERSION}.css || ln -s ${DOC_SRC_LIB}/homepage.css ./homepage-${CRPCUT_LIB_VERSION}.css
     )
   add_custom_target(
     changelog-${CRPCUT_LIB_VERSION}.html
     ${AWK} "-f" ${DOC_SRC_LIB}/mkchangelog.awk "-v" version=${CRPCUT_LIB_VERSION} < ${CMAKE_SOURCE_DIR}/ChangeLog.txt > changelog-${CRPCUT_LIB_VERSION}.html
     DEPENDS ${CMAKE_SOURCE_DIR}/ChangeLog.txt ${DOC_SRC_LIB}/mkchangelog.awk
     )
   add_custom_target(
     sourceforge-release
     ${DOC_SRC_LIB}/make-release-archives "${CMAKE_SOURCE_DIR}" "${CRPCUT_LIB_VERSION}" "${CMAKE_INSTALL_PREFIX}"
     DEPENDS
     changelog-${CRPCUT_LIB_VERSION}.html
     homepage-${CRPCUT_LIB_VERSION}.html
     homepage-${CRPCUT_LIB_VERSION}.css

     )
  endif(XSLTPROC AND HTML_CHUNK_XSL AND GDB AND SED AND HAVE_GMOCK)
endif(WITH_HTML_DOCS)

find_program(RUBY "ruby")
if(RUBY)
  add_custom_target(selftest ${RUBY} ${CMAKE_SOURCE_DIR}/selftest.rb ${SELFTEST_PARAM} DEPENDS testprog VERBATIM)
else(RUBY)
  message("ruby is not found - selftest target not available")
endif(RUBY)

if (AWK AND BASH AND XMLLINT)
   add_custom_target(sanitycheck ${BASH} ${CMAKE_SOURCE_DIR}/sanitycheck.sh ${AWK} ${CMAKE_SOURCE_DIR} ${SELFTEST_PARAM} DEPENDS testprog VERBATIM)
endif (AWK AND BASH AND XMLLINT)

if (NOT DEFINED LIBRARY_OUTPUT_PATH)
  if (NOT DEFINED LIB_SUFFIX)
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
      set(LIB_SUFFIX 64)
    endif ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
  endif (NOT DEFINED LIB_SUFFIX)
  set(LIBRARY_OUTPUT_PATH "lib${LIB_SUFFIX}")
endif (NOT DEFINED LIBRARY_OUTPUT_PATH)

set(EXECUTABLE_OUTPUT_PATH test)

if(HAVE_CXX0X AND NOT HAVE_TYPEOF)
  message("*** IMPORTANT *** You must compile your tests using -std=c++0x")
  add_definitions(-std=c++0x)
endif(HAVE_CXX0X AND NOT HAVE_TYPEOF)
if (HAVE_VALGRIND)
  message("*** compiling with support for valgrind")
  add_definitions(-DHAVE_VALGRIND)
  set(CMAKE_CXX_FLAGS_MAINT "${CMAKE_CXX_FLAGS_MAINT} -Wno-long-long")
endif (HAVE_VALGRIND)

if (WITH_HTML_DOCS)
  install(
    DIRECTORY ${CMAKE_BINARY_DIR}/html
    DESTINATION "${SHAREDIR}/doc"
    )
  install(
    DIRECTORY ${DOC_SRC_LIB}/icons/
    DESTINATION "${SHAREDIR}/doc/html/images"
    FILES_MATCHING PATTERN "*.png"
    )
  install(
    FILES ${DOC_SRC_LIB}/crpcut-doc.css
    DESTINATION "${SHAREDIR}/doc/html"
    )
endif (WITH_HTML_DOCS)

install(
  FILES include/crpcut.hpp
  DESTINATION include
)

install(TARGETS crpcut DESTINATION ${LIBRARY_OUTPUT_PATH})
install(
  FILES ${TEST_SRCS}
  DESTINATION "${SHAREDIR}/example_source"
)
install(
  FILES crpcut.xsd TODO
  DESTINATION "${SHAREDIR}"
  )

