#!/usr/bin/ruby

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

require "rexml/document"

class Test
  def initialize(result)
    @result = result
    @log = {}
    @files = []
  end
  def expected_result?(re)
    @result =~ re
  end
  def log(type, re)
    if @log.has_key? type then
      @log[type].push re
    else
      @log[type] = [ re ]
    end
    self
  end
  def file(name)
    @files.push name
    self
  end
  def result_of(test)
    result = test.attributes['result']
    return "#{result} when #{@result} expected" if @result != result
    log = @log.clone
    dirname = nil
    test.elements.each('log/*') do |entry|
      text = entry.text || ""
      name = entry.name
      t = log[name]
      return "#{name} unexpected" if !t
      t = t.clone
      if name == 'violation' then
        dirname = entry.attributes['nonempty_dir']
        if dirname then
          begin
            isdir = dirname && File.stat(dirname).directory?
          rescue
          end
          return "#{dirname} is not a directory" if !isdir
        end
      end
      re = t.delete_at(0)
      return "Too many #{name}'s" if !re
      return "#{text} doesn't match #{name}" if !re.match(text)
    end
    return "#{dirname} has unexpected files" if dirname && @files.empty?
    return "#{@files} is missing" if !dirname && !@files.empty?
    @files.each() do | name |
      path=dirname + '/' + name
      is_found=nil
      begin
        is_found = File.stat(path)
      rescue
      end
      return "#{name} is missing" if !is_found
      if is_found.directory? then
        Dir::rmdir(path)
      else
        File::unlink(path)
      end
    end
    begin
      dirname && Dir::rmdir(dirname)
    rescue
      return "working dir has unexpected files"
    end
    result
  end
end

A_H='asserts_and_depends\.cpp:\d+\s+'
P_H='parametrized\.cpp:\d+\s+'
PR_H='predicates\.cpp:\d+\s+'
RE_H='regex\.cpp:\d+\s+'
FP_H='fp\.cpp:\d+\s+'
A_T='Actual time to completion was'
S_E='std::exception\s+what\(\)'
R_E='std::range_error'
TESTS = {
  'asserts::should_fail_assert_exception_with_wrong_exception' =>
  Test.new('FAILED').
  log('violation',
      /caught std::exception\s+what\(\)=/me),

  'asserts::should_fail_assert_no_throw_with_std_exception_string_apa' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_NO_THROW\(throw #{R_E}\("apa"\)\)\s+caught #{S_E}=apa/me),

  'asserts::should_fail_assert_no_throw_with_unknown_exception' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_NO_THROW\(\s*throw\s+1\s*\)\s+caught\s+\.\.\./me),

  'asserts::should_fail_assert_throw_any_with_no_exception' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_THROW\(i=1, \.\.\.\)\s*Did not throw/me),

  'asserts::should_fail_assert_throw_with_no_exception' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_THROW\(i=1, std::exception\)\s+Did not throw/me),

  'asserts::should_fail_on_assert_eq_with_fixture' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_EQ\(num, 3\)\s+where\s+num\s*=\s*4/me),

  'asserts::should_fail_on_assert_false_with_fixture' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_FALSE\(num\)\n\s+is evaluated as:\n\s+3/me),

  'asserts::should_fail_on_assert_ge_with_fixture' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_GE\(num, 3\)\s+where\s+num\s*=\s*2/me),

  'asserts::should_fail_on_assert_gt_with_fixture' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_GT\(num, 3\)\s+where\s+num\s*=\s+3/me),

  'asserts::should_fail_on_assert_gt_with_unstreamable_param_i' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_GT\(i, num\)\s+where\s+i\s*=\s*\d+-byte object <[03 ]+>\s+num\s*=\s*3/me),

  'asserts::should_fail_on_assert_le_with_fixture' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_LE\(num, 3\)\s+where\s+num\s*=\s*4/me),

  'asserts::should_fail_on_assert_lt_with_fixture' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_LT\(num, 3\)\s+where\s+num\s*=\s*3/me),

  'asserts::should_fail_on_assert_ne_with_fixture' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_NE\(num, 3\)\s+where\s+num\s*=\s*3/me),

  'asserts::should_fail_on_assert_true_with_fixture' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_TRUE\(num\)\n\s+is evaluated as:\s+0/me),

  'asserts::should_succeed_assert_no_throw' =>
  Test.new('PASSED').
  log('stdout', /i=.*/),

  'asserts::should_succeed_assert_throw_with_correct_exception' =>
  Test.new('PASSED'),

  'asserts::should_succeed_on_assert_eq_with_fixture' =>
  Test.new('PASSED'),

  'asserts::should_succeed_on_assert_false_with_fixture' =>
  Test.new('PASSED'),

  'asserts::should_succeed_on_assert_ge_with_fixture' =>
  Test.new('PASSED'),

  'asserts::should_succeed_on_assert_gt_with_fixture' =>
  Test.new('PASSED'),

  'asserts::should_succeed_on_assert_le_with_fixture' =>
  Test.new('PASSED'),

  'asserts::should_succeed_on_assert_lt_with_fixture' =>
  Test.new('PASSED'),

  'asserts::should_succeed_on_assert_ne_with_fixture' =>
  Test.new('PASSED'),

  'asserts::should_succeed_on_assert_true_with_fixture' =>
  Test.new('PASSED'),

  'asserts::should_succeed_throw_any_with_int_exception' =>
  Test.new('PASSED'),

  'asserts::should_succeed_pointer_eq_0' =>
  Test.new('PASSED'),

  'asserts::should_succeed_0_eq_pointer' =>
  Test.new('PASSED'),

  'asserts::should_succeed_void_ptr_eq_ptr' =>
  Test.new('PASSED'),

  'asserts::should_succeed_ptr_eq_void_ptr' =>
  Test.new('PASSED'),

  'asserts::should_fail_pointer_eq_0' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_EQ\(pi, 0\)\n\s+where pi = (0x)?[[:xdigit:]]+$/me),

  'asserts::should_fail_0_eq_pointer' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_EQ\(0, pi\)\n\s+where pi = (0x)?[[:xdigit:]]+$/me),

  'asserts::should_fail_void_ptr_eq_ptr' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_EQ\(pv, pi\)\n\s+where pv = (0|\(nil\))\n\s+pi = (0x)?[[:xdigit:]]+$/me),

  'asserts::should_fail_ptr_eq_void_ptr' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_EQ\(pi, pv\)\n\s+where pi = (0x)?[[:xdigit:]]+\n\s+pv = (0|\(nil\))\s*$/me),

  'asserts::should_fail_eq_volatile' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_EQ\(n, m\)\n\s+where n = 3\n\s+m = 4\s*/me),

  'asserts::should_fail_false_volatile' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_FALSE\(n\)\n\s+is evaluated as:\n\s+3\s*/me),

  'asserts::should_fail_false_const_volatile' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_FALSE\(n\)\n\s+is evaluated as:\n\s+3\s*/me),

  'asserts::should_fail_true_volatile' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_TRUE\(n\)\n\s+is evaluated as:\n\s*0\s*/me),

  'asserts::should_fail_true_const_volatile' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_TRUE\(n\)\n\s+is evaluated as:\n\s*0\s*/me),

  'asserts::should_succeed_class_const_int_member' =>
  Test.new('PASSED'),

  'asserts::should_succeed_0_eq_pointer_to_member' =>
  Test.new('PASSED'),

  'asserts::should_fail_on_assert_true_with_small_unstreamable_param' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_TRUE\(i\)\n\s+is evaluated as:\n\s+.*<[0 ]*>\s*/me),

  'asserts::should_fail_on_assert_true_with_large_unstreamable_param' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_TRUE\(i\)\n\s+is evaluated as:\n\s+\?\s*/me),

  'asserts::expr::should_fail_on_assert_true_with_small_unstreamable_param' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_TRUE\(i - 4 < unstreamable<int>\(0\)\)\n\s+is evaluated as:\n\s+.*<[04 ]*> - 4 < .*<[0 ]*>\s*/me),

  'asserts::expr::should_fail_on_assert_true_with_large_unstreamable_param' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_TRUE\(i - 4 < unstreamable<long double>\(0\)\)\n\s+is evaluated as:\n\s+ \? - 4 < \?\s*/me),

  'asserts::expr::should_fail_on_assert_true_with_lt' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_TRUE\(n < num\)\n\s+is evaluated as:\n\s+4 < 3\s*/me),

  'asserts::expr::should_fail_on_assert_true_with_add_lt' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_TRUE\(n \+ num < 5\)\n\s+is evaluated as:\n\s+4 \+ 3 < 5\s*/me),

  'asserts::expr::should_fail_on_assert_true_with_sub_lt' =>
  Test.new('FAILED').
  log('violation',
      /#{A_H}ASSERT_TRUE\(n - num < 0\)\n\s+is evaluated as:\n\s+4 - 3 < 0\s*/me),

  'verify::should_succeed_verify_throw_with_correct_exception' =>
  Test.new('PASSED').
  log('info',
      /after/),

  'verify::should_fail_verify_exception_with_wrong_exception' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_THROW.*caught std::exception\s+what\(\)=/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_verify_throw_with_no_exception' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_THROW.*Did not throw/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_verify_throw_with_unexpected_c_string' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_THROW\([^\)]*\)\n\s+caught\s+\"apa\"/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_verify_throw_with_translated_invalid_argument' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_THROW\(.*\"apa\"\).*\)\s+caught invalid_argument\s+what\(\)=apa/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_succeed_verify_no_throw' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_fail_verify_throw_any_with_no_exception' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_THROW.*Did not throw/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_succeed_throw_any_with_int_exception' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_fail_verify_no_throw_with_unknown_exception' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_NO_THROW.*caught\s+\.\.\./me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_verify_no_throw_with_unexpected_c_string' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_NO_THROW.*caught \"apa\"/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_verify_no_throw_with_translated_exception' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_NO_THROW\(.*\"apa\"\).*\).*caught invalid_argument\n\s+what\(\)=apa/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_verify_no_throw_with_std_exception_string_apa' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_NO_THROW.*caught\s+#{S_E}=apa/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_succeed_on_verify_eq_with_fixture' =>
  Test.new('PASSED').
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_on_verify_eq_with_fixture' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_EQ\(num, 3\)\s+where\s+num\s*=\s*4/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_succeed_on_verify_ne_with_fixture' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_fail_on_verify_ne_with_fixture' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_NE\(num, 3\)\s+where\s+num\s*=\s*3/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_succeed_on_verify_gt_with_fixture' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_fail_on_verify_gt_with_fixture' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_GT\(num, 3\)\s+where\s+num\s*=\s+3/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_succeed_on_verify_ge_with_fixture' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_fail_on_verify_ge_with_fixture' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_GE\(num, 3\)\s+where\s+num\s*=\s*2/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_succeed_on_verify_lt_with_fixture' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_fail_on_verify_lt_with_fixture' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_LT\(num, 3\)\s+where\s+num\s*=\s*3/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_succeed_on_verify_le_with_fixture' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_fail_on_verify_le_with_fixture' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_LE\(num, 3\)\s+where\s+num\s*=\s*4/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_succeed_on_verify_true_with_fixture' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_fail_on_verify_true_with_fixture' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_TRUE\(num\)\n\s+is evaluated as:\s+0/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_succeed_on_verify_false_with_fixture' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_fail_on_verify_false_with_fixture' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_FALSE\(num\)\n\s+is evaluated as:\n\s+3/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_on_verify_gt_with_unstreamable_param_i' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_GT\(i, num\)\s+where\s+i\s*=\s*\d+-byte object <[03 ]+>\s+num\s*=\s*3/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_on_verify_true_with_small_unstreamable_param' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_TRUE\(i\)\n\s+is evaluated as:\n\s+.*<[0 ]*>\s*/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_on_verify_true_with_large_unstreamable_param' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_TRUE\(i\)\n\s+is evaluated as:\n\s+\?\s*/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_succeed_pointer_eq_0' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_succeed_0_eq_pointer' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_succeed_void_ptr_eq_ptr' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_succeed_ptr_eq_void_ptr' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_fail_pointer_eq_0' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_EQ\(pi, 0\)\n\s+where pi = (0x)?[[:xdigit:]]+$/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_0_eq_pointer' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_EQ\(0, pi\)\n\s+where pi = (0x)?[[:xdigit:]]+$/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_void_ptr_eq_ptr' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_EQ\(pv, pi\)\n\s+where pv = (0|\(nil\))\n\s+pi = (0x)?[[:xdigit:]]+$/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_ptr_eq_void_ptr' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_EQ\(pi, pv\)\n\s+where pi = (0x)?[[:xdigit:]]+\n\s+pv = (0|\(nil\))\s*$/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_eq_volatile' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_EQ\(n, m\)\n\s+where n = 3\n\s+m = 4\s*/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_false_volatile' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_FALSE\(n\)\n\s+is evaluated as:\n\s+3\s*/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_false_const_volatile' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_FALSE\(n\)\n\s+is evaluated as:\n\s+3\s*/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_true_volatile' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_TRUE\(n\)\n\s+is evaluated as:\n\s*0\s*/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_fail_true_const_volatile' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_TRUE\(n\)\n\s+is evaluated as:\n\s*0\s*/me).
  log('info', /after/).
  log('violation', //),

  'verify::should_succeed_class_const_int_member' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::should_succeed_0_eq_pointer_to_member' =>
  Test.new('PASSED').
  log('info', /after/),

  'verify::expr::should_fail_on_verify_true_with_small_unstreamable_param' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_TRUE\(i - 4 < unstreamable<int>\(0\)\)\n\s+is evaluated as:\n\s+.*<[04 ]*> - 4 < .*<[0 ]*>\s*/me).
  log('info', /after/).
  log('violation', //),

  'verify::expr::should_fail_on_verify_true_with_large_unstreamable_param' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_TRUE\(i - 4 < unstreamable<long double>\(0\)\)\n\s+is evaluated as:\n\s+ \? - 4 < \?\s*/me).
  log('info', /after/).
  log('violation', //),

  'verify::expr::should_fail_on_verify_true_with_lt' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_TRUE\(n < num\)\n\s+is evaluated as:\n\s+4 < 3\s*/me).
  log('info', /after/).
  log('violation', //),

  'verify::expr::should_fail_on_verify_true_with_add_lt' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_TRUE\(n \+ num < 5\)\n\s+is evaluated as:\n\s+4 \+ 3 < 5\s*/me).
  log('info', /after/).
  log('violation', //),

  'verify::expr::should_fail_on_verify_true_with_sub_lt' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_TRUE\(n - num < 0\)\n\s+is evaluated as:\n\s+4 - 3 < 0\s*/me).
  log('info', /after/).
  log('violation', //),


  'death::by_exception::should_fail_any_exception' =>
  Test.new('FAILED').
  log('violation',
      /Unexpectedly did not throw/),

  'death::by_exception::should_fail_due_to_std_exception_with_string_apa' =>
  Test.new('FAILED').
  log('violation',
      /Unexpectedly caught #{S_E}=apa/me),

  'death::by_exception::should_fail_due_to_unknown_exception' =>
  Test.new('FAILED').
  log('violation',
      /Unexpectedly caught \.\.\./),

  'death::by_exception::should_fail_due_to_c_string_exception' =>
  Test.new('FAILED').
  log('violation',
      /Unexpectedly caught \"apa\"/),

  'death::by_exception::should_fail_with_no_exception' =>
  Test.new('FAILED').
  log('violation',
      /Unexpectedly did not throw/),

  'death::by_exception::should_fail_with_wrong_exception' =>
  Test.new('FAILED').
  log('violation',
      /Unexpectedly caught std::exception\n.*/),

  'death::by_exception::should_fail_with_c_string_exception' =>
  Test.new('FAILED').
  log('violation',
      /Unexpectedly caught \"apa\"/),

  'death::by_exception::should_fail_with_translated_exception' =>
  Test.new('FAILED').
  log('violation',
      /Unexpectedly caught invalid_argument\n\s+what\(\)=apa/),


  'death::by_exception::should_succed_with_any_exception' =>
  Test.new('PASSED'),

  'death::by_exception::should_succeed_with_range_error_thrown' =>
  Test.new('PASSED'),

  'death::by_exit::should_fail_with_exit_code_3' =>
  Test.new('FAILED').
  log('violation',
      /Exited with code 3\s+Expected normal exit/me),
  'death::by_exit::should_fail_with_no_exit' =>
  Test.new('FAILED').
  log('violation',
      /Unexpectedly survived\s+Expected exit with code 3/me),

  'death::by_exit::should_fail_with_wrong_exit_code' =>
  Test.new('FAILED').
  log('violation',
      /Exited with code 4\s+Expected exit with code 3/me),

  'death::by_exit::should_succeed_with_exit_code_3' =>
  Test.new('PASSED'),

  'death::by_exit::should_succeed_with_wiped_working_dir' =>
  Test.new('PASSED'),

  'death::by_exit::should_fail_wipe_with_left_behind_files_due_to_wrong_exit_code' =>
  Test.new('FAILED').
  file("katt/apa").
  file("katt").
  log('violation', /.*/me),

  'death::by_exit::should_fail_wipe_with_left_behind_files_due_to_signal_death' =>
  Test.new('FAILED').
  file("katt/apa").
  file("katt").
  log('violation', /.*/me),

  'death::by_signal::should_fail_with_left_behind_core_dump_due_to_death_on_signal_11' =>
  Test.new('FAILED').
  log('violation',
      /Died with core dump/).
  file('core'),

  'death::by_signal::should_fail_with_normal_exit' =>
  Test.new('FAILED').
  log('violation',
      /Unexpectedly survived\s+Expected signal 11/me),

  'death::by_signal::should_fail_with_wrong_signal' =>
  Test.new('FAILED').
  log('violation',
      /Died on signal 6\s+Expected signal 11/me),

  'death::by_signal::should_fail_without_core_dump_with_death_on_signal_11' =>
  Test.new('FAILED').
  log('violation',
      /Died on signal 11\s+Expected normal exit/me),

  'death::by_signal::should_succeed_with_death_on_signal_11' =>
  Test.new('PASSED'),

  'death::by_signal::should_succeed_with_wiped_working_dir' =>
  Test.new('PASSED'),

  'death::by_signal::should_fail_wipe_with_left_behind_files_due_to_wrong_signal' =>
  Test.new('FAILED').
  file("katt/apa").
  file("katt").
  log('violation', /.*/me),

  'death::by_signal::should_fail_wipe_with_left_behind_files_due_to_exit' =>
  Test.new('FAILED').
  file("katt/apa").
  file("katt").
  log('violation', /.*/me),

  'default_success' =>
  Test.new('PASSED'),

 'depends::should_not_run_due_to_one_failed_dependency_success_otherwise' =>
  Test.new('PASSED'),

  'depends::should_succeed_after_success_dependencies' =>
  Test.new('PASSED'),

  'ext_parameters::should_succeed_expected_value' =>
  Test.new('PASSED').
  log('info',
      /katt/),

  'ext_parameters::should_succeed_no_value' =>
  Test.new('PASSED'),

  'ext_parameters::should_succeed_no_value_with_too_long_name' =>
  Test.new('PASSED'),

  'ext_parameters::should_succeed_value_interpret' =>
  Test.new('PASSED'),

  'ext_parameters::should_fail_value_interpret' =>
  Test.new('FAILED').
  log('violation',
      /Parameter apa with value "katt" cannot be interpreted/),

  'ext_parameters::should_fail_no_value_interpret' =>
  Test.new('FAILED').
  log('violation',
      /Parameter orm with no value cannot be interpreted/),

  'fp::abs::should_succeed_add_epsilon_float' =>
  Test.new('PASSED'),

  'fp::abs::should_succeed_sub_epsilon_float' =>
  Test.new('PASSED'),

  'fp::abs::should_fail_add_2epsilon_float' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*abs_diff.*param1 = \d+\.\d+.*Max allowed difference is.*Actual difference is.*/me),

  'fp::abs::should_fail_sub_2epsilon_float' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*abs_diff.*param1 = \d+\.\d+.*Max allowed difference is.*Actual difference is.*/me),

  'fp::abs::should_succeed_add_epsilon_double' =>
  Test.new('PASSED'),

  'fp::abs::should_succeed_sub_epsilon_double' =>
  Test.new('PASSED'),

  'fp::abs::should_fail_add_2epsilon_double' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*abs_diff.*param1 = \d+\.\d+.*Max allowed difference is.*Actual difference is.*/me),

  'fp::abs::should_fail_sub_2epsilon_double' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*abs_diff.*param1 = \d+\.\d+.*Max allowed difference is.*Actual difference is.*/me),


  'fp::abs::should_succeed_add_epsilon_long_double' =>
  Test.new('PASSED'),

  'fp::abs::should_succeed_sub_epsilon_long_double' =>
  Test.new('PASSED'),

  'fp::abs::should_fail_add_2epsilon_long_double' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*abs_diff.*param1 = \d+\.\d+.*Max allowed difference is.*Actual difference is/me),

  'fp::abs::should_fail_sub_2epsilon_long_double' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*abs_diff.*(\s+param[12] = \d+\.\d+){2}.*Max allowed difference is.*Actual difference is/me),

  'fp::relative::should_fail_relative_epsilon_float' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*relative_diff.*(\s+param[12] = \d+\.\d+){2}.*Max allowed relative difference is.*Actual relative difference is/me),

  'fp::relative::should_succeed_relative_epsilon_float' =>
  Test.new('PASSED'),

  'fp::relative::should_fail_relative_epsilon_double' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*relative_diff.*(\s+param[12] = \d+\.\d+){2}.*Max allowed relative difference is.*Actual relative difference is/me),

  'fp::relative::should_succeed_relative_epsilon_double' =>
  Test.new('PASSED'),

  'fp::relative::should_fail_relative_epsilon_long_double' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*relative_diff.*(\s+param[12] = \d+\.\d+){2}.*Max allowed relative difference is.*Actual relative difference is/me),

  'fp::relative::should_succeed_relative_epsilon_long_double' =>
  Test.new('PASSED'),

  'fp::ulps::using_float::should_succeed_equal_zeroes_0_ulps' =>
  Test.new('PASSED'),

  'fp::ulps::using_float::should_succeed_eps_diff_1_ulp' =>
  Test.new('PASSED'),

  'fp::ulps::using_float::should_fail_eps_diff_0_ulp' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(0U\), f1, f2\)\n\s+param1 = 1\n\s+param2 = 1/me),

  'fp::ulps::using_float::should_succeed_high_denorm_1_ulp' =>
  Test.new('PASSED'),

  'fp::ulps::using_float::should_fail_high_denorm_0_ulp' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(0U\), f1, f2\)\n\s+param1 = [0-9\.e+-]+\n\s+param2 = [0-9\.e+-]+/me),

  'fp::ulps::using_float::should_succeed_low_denorm_1_ulp' =>
  Test.new('PASSED'),

  'fp::ulps::using_float::should_fail_low_denorm_0_ulp' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(0U\), f1, f2\)\n\s+param1 = 0\n\s+param2 = [0-9\.e+-]+/me),

  'fp::ulps::using_float::should_succeed_pos_neg_denorm_min_2_ulps' =>
  Test.new('PASSED'),

  'fp::ulps::using_float::should_fail_pos_neg_denorm_min_1_ulp' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(1U\), f1, f2\)\n\s+param1 = [0-9\.e+-]+\n\s+param2 = -[0-9\.e+-]+/me),

  'fp::ulps::using_float::should_fail_nan' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(~unsigned\(\)\), f1, f2\)\n\s+param1 = -?[Nn][Aa][Nn]\n\s+param2 = 0/me),

  'fp::ulps::using_float::should_fail_max_inf_1_ulp' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(1U\), f1, f2\)\n\s+param1 = [Ii][Nn][Ff]\n\s+param2 = [0-9\.e+-]+/me),

  'fp::ulps::using_float::should_succeed_max_inf_1_ulp' =>
  Test.new('PASSED'),


  'fp::ulps::using_double::should_succeed_equal_zeroes_0_ulps' =>
  Test.new('PASSED'),

  'fp::ulps::using_double::should_succeed_eps_diff_1_ulp' =>
  Test.new('PASSED'),

  'fp::ulps::using_double::should_fail_eps_diff_0_ulp' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(0U\), f1, f2\)\n\s+param1 = 1\n\s+param2 = 1/me),

  'fp::ulps::using_double::should_succeed_high_denorm_1_ulp' =>
  Test.new('PASSED'),

  'fp::ulps::using_double::should_fail_high_denorm_0_ulp' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(0U\), f1, f2\)\n\s+param1 = [0-9\.e+-]+\n\s+param2 = [0-9\.e+-]+/me),

  'fp::ulps::using_double::should_succeed_low_denorm_1_ulp' =>
  Test.new('PASSED'),

  'fp::ulps::using_double::should_fail_low_denorm_0_ulp' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(0U\), f1, f2\)\n\s+param1 = 0\n\s+param2 = [0-9\.e+-]+/me),

  'fp::ulps::using_double::should_succeed_pos_neg_denorm_min_2_ulps' =>
  Test.new('PASSED'),

  'fp::ulps::using_double::should_fail_pos_neg_denorm_min_1_ulp' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(1U\), f1, f2\)\n\s+param1 = [0-9\.e+-]+\n\s+param2 = -[0-9\.e+-]+/me),

  'fp::ulps::using_double::should_fail_nan' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(~unsigned\(\)\), f1, f2\)\n\s+param1 = -?[Nn][Aa][Nn]\n\s+param2 = 0/me),

  'fp::ulps::using_double::should_fail_max_inf_1_ulp' =>
  Test.new('FAILED').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(1U\), f1, f2\)\n\s+param1 = [Ii][Nn][Ff]\n\s+param2 = [0-9\.e+-]+/me),

  'fp::ulps::using_double::should_succeed_max_inf_1_ulp' =>
  Test.new('PASSED'),


  'parametrized::should_fail_assert_lt_char_array_string' =>
  Test.new('FAILED').
  log('violation',
      /#{P_H}ASSERT_LT\(p1, p2\)\s+where p1 = orm\s+p2 = katt/me),

  'parametrized::should_fail_assert_lt_int_char' =>
  Test.new('FAILED').
  log('violation',
      /#{P_H}ASSERT_LT\(p1, p2\)\s+where p1 = 800\s+p2 = A/me),

  'parametrized::should_fail_assert_lt_int_double' =>
  Test.new('FAILED').
  log('violation',
      /#{P_H}ASSERT_LT\(p1, p2\)\s+where p1 = 4\s+p2 = 3.14[12]\d*/me),

  'parametrized::should_succeed_assert_lt_char_array_string' =>
  Test.new('PASSED'),

  'parametrized::should_succeed_assert_lt_int_char' =>
  Test.new('PASSED'),

  'parametrized::should_succeed_assert_lt_int_double' =>
  Test.new('PASSED'),

  'predicates::should_succeed_simple_func' =>
  Test.new('PASSED'),

  'predicates::should_fail_simple_func' =>
  Test.new('FAILED').
  log('violation',
       /#{PR_H}ASSERT_PRED\(is_positive, v\)\s+param1 = -1/me),

  'predicates::should_succeed_simple_func_with_param_side_effect' =>
  Test.new('PASSED'),

  'predicates::should_fail_simple_func_with_param_side_effect' =>
  Test.new('FAILED').
  log('violation',
       /#{PR_H}ASSERT_PRED\(is_positive, --v\)\s+param1 = -1/me),
#
  'predicates::should_succeed_verify_simple_func' =>
  Test.new('PASSED').
  log('info', /after/),

  'predicates::should_fail_verify_simple_func' =>
  Test.new('FAILED').
  log('fail',
       /#{PR_H}VERIFY_PRED\(is_positive, v\)\s+param1 = -1/me).
  log('info', /after/).
  log('violation', //),

  'predicates::should_succeed_verify_simple_func_with_param_side_effect' =>
  Test.new('PASSED').
  log('info', /after/),

  'predicates::should_fail_verify_simple_func_with_param_side_effect' =>
  Test.new('FAILED').
  log('fail',
       /#{PR_H}VERIFY_PRED\(is_positive, --v\)\s+param1 = -1/me).
  log('info', /after/).
  log('violation', //),
#
  'predicates::should_succeed_func_wrap_class' =>
  Test.new('PASSED'),

  'predicates::should_fail_func_wrap_class' =>
  Test.new('FAILED').
  log('violation',
      /#{PR_H}ASSERT_PRED\(bifuncwrap.*less.*strcmp.*katt.*apa\"\)\s+param1 = katt\s+param2 = apa/me),

#
  'predicates::should_succeed_verify_func_wrap_class' =>
  Test.new('PASSED').
  log('info', /after/),

  'predicates::should_fail_verify_func_wrap_class' =>
  Test.new('FAILED').
  log('fail',
      /#{PR_H}VERIFY_PRED\(bifuncwrap.*less.*strcmp.*katt.*apa\"\)\s+param1 = katt\s+param2 = apa/me).
  log('info', /after/).
  log('violation', //),
#
  'predicates::should_succeed_streamable_pred' =>
  Test.new('PASSED'),

  'predicates::should_fail_streamable_pred' =>
  Test.new('FAILED').
  log('violation',
      /#{PR_H}ASSERT_PRED\(string_equal\(.*"katt"\)\s+param1 = katt\s+string_equal.*\) :\ncompare.*equal to "apa"/me),

  'predicates::should_succeed_ptr_deref_eq' =>
  Test.new('PASSED'),

  'predicates::should_fail_ptr_deref_eq' =>
  Test.new('FAILED').
  log('violation',
      /#{PR_H}ASSERT_PRED.*pointing to:\s+4.*pointing to:\s+3/me),

#
  'predicates::should_succeed_verify_streamable_pred' =>
  Test.new('PASSED').
  log('info', /after/),

  'predicates::should_fail_verify_streamable_pred' =>
  Test.new('FAILED').
  log('fail',
      /#{PR_H}VERIFY_PRED\(string_equal\(.*"katt"\)\s+param1 = katt\s+string_equal.*\) :\ncompare.*equal to "apa"/me).
  log('info', /after/).
  log('violation', //),

  'predicates::should_succeed_verify_ptr_deref_eq' =>
  Test.new('PASSED').
  log('info', /after/),

  'predicates::should_fail_verify_ptr_deref_eq' =>
  Test.new('FAILED').
  log('fail',
      /#{PR_H}VERIFY_PRED.*pointing to:\s+4.*pointing to:\s+3/me).
  log('info', /after/).
  log('violation', //),
#
  'collate::should_succeed_collation_string' =>
  Test.new('PASSED'),

  'collate::should_succeed_collation_char_array' =>
  Test.new('PASSED'),

  'collate::should_fail_collation_string' =>
  Test.new('FAILED').
  log('violation',
      /left hand value = \"app\"\n\s+.*right hand value = \"apa\"/me),

  'collate::should_fail_collation_char_array' =>
  Test.new('FAILED').
  log('violation',
      /left hand value = \"APP\"\n\s+.*right hand value = \"APA\"/me),

  'collate::should_succeed_equal_upcase' =>
  Test.new('PASSED'),

  'collate::should_fail_with_nonexisting_locale' =>
  Test.new('FAILED').
  log('violation',
      /ASSERT_.*caught\s+std::exception/me),

  'regex::should_succeed_simple_re' =>
  Test.new('PASSED'),

  'regex::should_fail_illegal_re' =>
  Test.new('FAILED').
  log('violation',
      /#{RE_H}ASSERT_PRED.*regex.*\)\s+param1 = apa.*\"\[a\"\) :\n.*\n/me),

  'regex::should_fail_no_match' =>
  Test.new('FAILED').
  log('violation',
      /#{RE_H}ASSERT_PRED.*regex.*\)\n\s+param1 = katt.*\) :\ndid not match/me),

  'regex::should_fail_case_mismatch' =>
  Test.new('FAILED').
  log('violation',
      /#{RE_H}ASSERT_PRED.*regex.*\)\n\s+param1 = APA.*\) :\ndid not match/me),

  'regex::should_succeed_case_mismatch' =>
  Test.new('PASSED'),

  'regex::should_fail_ere_paren_on_non_e_re' =>
  Test.new('FAILED').
  log('violation',
      /#{RE_H}ASSERT_PRED.*\).*\)\s+param1 = apakattkattkatttupp.*did not match/me),

  'regex::should_succeed_ere_paren_on_e_re' =>
  Test.new('PASSED'),

  'regex::should_succeed_non_ere_paren_on_non_e_re' =>
  Test.new('PASSED'),

  'regex::should_fail_non_ere_paren_on_e_re' =>
  Test.new('FAILED').
  log('violation',
      /#{RE_H}ASSERT_PRED.*regex::e\), \".*\"\)\s+param1 = apakattkattkatttupp.*did not match/me),

  'regex::should_succeed_paren_litteral_e_re' =>
  Test.new('PASSED'),

  'regex::should_succeed_paren_litteral_non_e_re' =>
  Test.new('PASSED'),

  'regex::should_fail_ere_on_non_e_re' =>
  Test.new('FAILED').
  log('violation',
      /#{RE_H}ASSERT_PRED.*\"apa\+\"\), \"apaaa\"\)\n\s+param1 = apaaa.*did not match\n/me),

  'regex::should_succeed_ere_on_e_re' =>
  Test.new('PASSED'),

  'should_fail_after_delay' =>
  Test.new('FAILED').
  log('violation',
      /Exited with code 1\s+Expected normal exit/me),

  'should_fail_due_to_left_behind_files' =>
  Test.new('FAILED').
  log('violation', /$/e).
  file("apa"),

  'should_succeed_reading_file_in_start_dir' =>
  Test.new('PASSED').
  log('info', /in.rdstate\(\)=\d-byte object <[ 0-9A-Fa-f]+>/),

 'should_not_run_due_to_failed_left_behind_files_success_otherwise' =>
  Test.new('PASSED'),

  'output::should_fail_with_terminate' =>
  Test.new('FAILED').
  log('violation',
      /output.cpp:\d+\n\s*apa=(0[Xx])?1[fF]/),

  'output::should_succeed_with_info' =>
  Test.new('PASSED').
  log('info',
      /apa=3/),

  'output::should_fail_with_info' =>
  Test.new('FAILED').
  log('info',
      /apa=3/).
  log('violation',
      /Exited with code 1\s+Expected normal exit/me),

  'output::should_succeed_with_info_endl' =>
  Test.new('PASSED').
  log('info',/apa\nkatt/me),

  'output::should_fail_with_death_and_left_behind_core_dump' =>
  Test.new('FAILED').
  log('stderr',
      /output\.cpp:\d+.*[Aa]ssert/me).
  log('violation',
      /Died with core dump/).
  file('core'),

  'output::should_fail_with_death_due_to_assert_on_stderr' =>
  Test.new('FAILED').
  log('stderr',
      /output\.cpp:\d+.*[Aa]ssert/me).
  log('violation',
      /Died on signal \d+\s+Expected normal exit/me),

  'output::should_succeed_with_stderr' =>
  Test.new('PASSED').
  log('stderr', /hello/),

  'output::should_succeed_with_stdout' =>
  Test.new('PASSED').
  log('stdout', /hello/),

  'output::should_succeed_with_big_unstreamable_obj' =>
  Test.new('PASSED').
  log('info',
      /byte object <(\n[ a-fA-F0-9]+){2}\s*\n\s+>/me),

  'suite_deps::simple_all_ok::should_succeed' =>
  Test.new('PASSED'),

  'suite_deps::simple_all_ok::should_also_succeed' =>
  Test.new('PASSED'),

  'suite_deps::simple_all_fail::should_succeed' =>
  Test.new('PASSED'),

  'suite_deps::simple_all_fail::should_fail' =>
  Test.new('FAILED').
  log('violation',
      /ASSERT/),

  'suite_deps::should_succeed' =>
  Test.new('PASSED'),

  'suite_deps::should_not_run_success' =>
  Test.new('PASSED'),

  'suite_deps::blocked_suite::cross_dep_violation_should_succeed' =>
  Test.new('PASSED'),

  'suite_deps::blocked_suite::should_not_run_success' =>
  Test.new('PASSED'),

  'suite_deps::blocked_case::should_not_run_success' =>
  Test.new('PASSED'),

  'suite_deps::blocked_case::nested_blocked::should_not_run_success' =>
  Test.new('PASSED'),

  'suite_deps::should_run_remote_suite::should_succeed' =>
  Test.new('PASSED'),

  'suite_deps::blocked_remote_suite::should_not_run_success' =>
  Test.new('PASSED'),

  'suite_deps::should_run_suite::should_succeed' =>
  Test.new('PASSED'),

  'suite_deps::should_run_case::should_succeed' =>
  Test.new('PASSED'),

  'suite_deps::should_run_case::nested_run::should_succeed' =>
  Test.new('PASSED'),

  'suite_deps::should_run_suite::should_also_succeed' =>
  Test.new('PASSED'),


  'timeouts::should_fail_slow_cputime_deadline' =>
  Test.new('FAILED').
  log('info', /.*/me).
  log('violation',
      /CPU-time timeout 100ms exceeded.\s+#{A_T} (([2-9]\d\d)|(1\d\d\d))ms/me),

  'timeouts::should_fail_slow_cputime_deadline_by_death' =>
  Test.new('FAILED').
  log('violation',
      /Died on signal \d+\s+Expected normal exit/me),

  'timeouts::should_fail_slow_realtime_deadline' =>
  Test.new('FAILED').
  log('violation',
      /Realtime timeout 100ms exceeded\.\s+#{A_T} [2-9]\d\dms/me),

  'timeouts::should_fail_slow_realtime_deadline_by_death' =>
  Test.new('FAILED').
  log('violation',
      /Timed out - killed/),

  'timeouts::should_succeed_slow_cputime_deadline' =>
  Test.new('PASSED'),

  'timeouts::should_succeed_slow_realtime_deadline' =>
  Test.new('PASSED'),

  'timeouts::should_fail_slow_save_from_stuck_constructor' =>
  Test.new('FAILED').
  log('violation',
      /Timed out - killed/),
  
  'timeouts::should_fail_quick_save_from_stuck_constructor' =>
  Test.new('FAILED').
  log('violation',
      /Timed out - killed/),

  'timeouts::should_fail_slow_save_from_stuck_destructor' =>
  Test.new('FAILED').
  log('violation',
      /Timed out - killed/),

  'timeouts::should_fail_quick_save_from_stuck_destructor' =>
  Test.new('FAILED').
  log('violation',
      /Timed out - killed/),

  'timeouts::expected::should_succeed_sleep' =>
  Test.new('PASSED'),

  'timeouts::expected::should_fail_early_return' =>
  Test.new('FAILED').
  log('violation',
      /Unexpectedly survived\nExpected 100ms realtime timeout/),

  'timeouts::expected::should_fail_cputime' =>
  Test.new('FAILED').
  log('violation',
      /Test consumed \d{2,3}ms CPU-time\nLimit was 3ms/),

  'timeouts::expected::should_succeed_cputime' =>
  Test.new('PASSED'),

  'timeouts::scoped::should_succeed_realtime_short_sleep' =>
  Test.new('PASSED'),

  'timeouts::scoped::should_fail_realtime_short_sleep' =>
  Test.new('FAILED').
  log('violation',
      /ASSERT_SCOPE_MAX_REALTIME_MS.*Actual time used was 2[0-5]ms/me),

  'timeouts::scoped::should_succeed_oversleep' =>
  Test.new('PASSED'),

  'timeouts::scoped::should_fail_cputime_long' =>
  Test.new('FAILED').
  log('violation',
      /ASSERT_SCOPE_MAX_CPUTIME_MS.*Actual time used was [1-5]\d\d\dms/me),


  'timeouts::scoped::should_succeed_verify_realtime_short_sleep' =>
  Test.new('PASSED').
  log('info', /after/),

  'timeouts::scoped::should_fail_verify_realtime_short_sleep' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_SCOPE_MAX_REALTIME_MS.*Actual time used was 2[0-5]ms/me).
  log('info', /after/).
  log('violation', //),

  'timeouts::scoped::should_succeed_verify_oversleep' =>
  Test.new('PASSED').
  log('info', /after/),

  'timeouts::scoped::should_fail_verify_cputime_long' =>
  Test.new('FAILED').
  log('fail',
      /VERIFY_SCOPE_MAX_CPUTIME_MS.*Actual time used was [1-5]\d\d\dms/me).
  log('info', /after/).
  log('violation', //),

  'very_slow_success' =>
  Test.new('PASSED'),

  'wrapped::should_succeed_in_range' =>
  Test.new('PASSED').
  log('info',
      /d=0.523\d+/),

  'wrapped::should_fail_assert_lt' =>
  Test.new('FAILED').
  log('violation',
      /wrapped\.cpp:\d+\s+ASSERT_LT\(d, 1\.\d*\)\s+where d = 1\.(1|0999).*/me),

  'heap::should_succeed_allocation_leak' =>
  Test.new('PASSED').
  log('info',
      /p1=.*/),

  'heap::should_fail_scope_leak_free' =>
  Test.new('FAILED').
  log('info',
      /p=.*/).
  log('violation',
      /1 object\n.*100 bytes at (0x)?[0-9A-Fa-f]+ allocated with malloc.*/me),

  'heap::should_succeed_scope_leak_free' =>
  Test.new('PASSED'),

  'heap::should_succeed_malloc_free_balance' =>
  Test.new('PASSED'),

  'heap::should_fail_verify_scope_leak_free' =>
  Test.new('FAILED').
  log('info', //).
  log('fail',
      /1 object\n.*100 bytes at (0x)?[0-9A-Fa-f]+ allocated with malloc/me).
  log('info', //).
  log('violation', //),

  'heap::should_succeed_verify_scope_leak_free' =>
  Test.new('PASSED').
  log('info', /after/),

  'heap::should_succeed_verify_malloc_free_balance' =>
  Test.new('PASSED').
  log('info', /after/),

  'heap::should_succeed_empty_balance_fix' =>
  Test.new('PASSED'),

  'heap::should_succeed_malloc_balance_fix' =>
  Test.new('PASSED'),

  'heap::should_succeed_worlds_worst_strcpy' =>
  Test.new('PASSED'),

  'heap::should_succeed_malloc_blast_limit' =>
  Test.new('PASSED'),

  'heap::should_succeed_new_blast_limit' =>
  Test.new('PASSED'),

  'heap::should_succeed_new_array_blast_limit' =>
  Test.new('PASSED'),

  'heap::should_succeed_new_nothrow_blast_limit' =>
  Test.new('PASSED'),

  'heap::should_succeed_new_array_nothrow_blast_limit' =>
  Test.new('PASSED'),

  'heap::should_succeed_blast_limit_with_string' =>
  Test.new('PASSED'),

  'heap::should_fail_limit_too_low' =>
  Test.new('FAILED').
  log('violation',
      /heap::set_limit.*is below current/),

  'heap::should_fail_cross_malloc_delete' =>
  Test.new('FAILED').
  log('violation',
      /DEALLOC FAIL\ndelete.*using malloc/me),

  'heap::should_fail_cross_malloc_delete_array' =>
  Test.new('FAILED').
  log('violation',
      /DEALLOC FAIL\ndelete\[\].*using malloc/me),

  'heap::should_fail_cross_new_free' =>
  Test.new('FAILED').
  log('violation',
      /DEALLOC FAIL\nfree.*using new/me),

  'heap::should_fail_cross_new_delete_array' =>
  Test.new('FAILED').
  log('violation',
      /DEALLOC FAIL\ndelete\[\].*using new/me),

  'heap::should_fail_cross_new_array_free' =>
  Test.new('FAILED').
  log('violation',
      /DEALLOC FAIL\nfree.*using new\[\]/me),

  'heap::should_fail_cross_new_array_delete' =>
  Test.new('FAILED').
  log('violation',
      /DEALLOC FAIL\ndelete.*using new\[\]/me),

  'heap::should_succeed_new_handler' =>
  Test.new('PASSED'),

  'heap::should_succeed_new_handler_no_ballast' =>
  Test.new('PASSED'),

  'heap::should_succeed_nothrow_new_handler' =>
  Test.new('PASSED'),

  'heap::should_succeed_nothrow_new_handler_no_ballast' =>
  Test.new('PASSED'),

  'bad_forks::fork_and_let_child_hang_should_fail' =>
  Test.new('FAILED').
  log('violation',
      /Timed out - killed/),

  'bad_forks::fork_and_let_child_run_test_code_should_fail' =>
  Test.new('FAILED').
  log('violation',
      /I am child/)
}

GMOCK_TESTS = {
  'google_mock::basic_success' =>
  Test.new('PASSED'),

  'google_mock::should_fail_by_calling_with_wrong_value' =>
  Test.new('FAILED').
  log('violation',
      /mock function call.*call: func\(4\).*equal to 3\s+Actual: 4/me),

  'google_mock::should_fail_by_calling_too_often' =>
  Test.new('FAILED').
  log('violation',
      /more times than expected.*func\(3\)/me),

  'google_mock::should_fail_by_not_calling' =>
  Test.new('FAILED').
  log('violation',
      /call count doesn't match.*Actual: never called/me),

  'google_mock::sequence_success_1' =>
  Test.new('PASSED'),

  'google_mock::sequence_success_2' =>
  Test.new('PASSED'),

  'google_mock::sequence_should_fail_incomplete' =>
  Test.new('FAILED').
  log('violation',
      /call count doesn't match.*Actual: never called/me),

  'google_mock::sequence_should_fail_one_too_many' =>
  Test.new('FAILED').
  log('violation',
      /called more times than expected.*Actual: called twice/me),

  'google_mock::sequence_should_fail_one_wrong_value' =>
  Test.new('FAILED').
  log('violation',
      /Unexpected mock function call.*call: func\(4\).*none matched:/me),

  'google_mock::success_with_unstreamable_type' =>
  Test.new('PASSED'),

  'google_mock::should_fail_with_unstreamable_type_wrong_value' =>
  Test.new('FAILED').
  log('violation',
      /Unexpected mock.*Expected.*equal.*3.*Actual.*4.*>/me)
}
tests=TESTS;
tests.merge! GMOCK_TESTS if ARGV[0] == 'gmock'
fails = 0
oks = 0
tests.each do |key, val|
  if val.expected_result?(/PASSED/) then
    oks+=1
  else
    fails+= 1
  end
end

BLOCKED_TESTS =
  [
   'depends::should_not_run_due_to_one_failed_dependency_success_otherwise',
   'should_not_run_due_to_failed_left_behind_files_success_otherwise',
   'suite_deps::should_not_run',
   'suite_deps::blocked_suite::should_not_run_success',
   'suite_deps::blocked_remote_suite::should_not_run_success',
   'suite_deps::blocked_case::should_not_run_success',
   'suite_deps::blocked_case::nested_blocked::should_not_run_success'
]

def check_file(file_name, tests, names, results, run_count, fail_count, success_count, blocked)
  collection = {}
  report = false
  file = open("|#{file_name}")
  s = file.read
  file.close
  doc = REXML::Document.new s
  rc = $?.exitstatus
  if rc != fail_count then
    puts "\lwrong retcode #{rc}, expecting #{fail_count}"
    report = true
  end
  doc.elements.each('crpcut/test') do |e|
    name = e.attributes['name'];
    t = tests[name]
    if !t then
      r = "unknown test case"
    elsif blocked.include?(name) || name !~ names || !t.expected_result?(results) then
      r = "shouldn't have run"
    else
      r = t.result_of(e)
    end
    collection[r] = [] if !collection[r]
    collection[r].push name
  end
  stats = doc.elements['crpcut/statistics']
  regs = stats.elements['registered_test_cases'].text.to_i
  runs = stats.elements['run_test_cases'].text.to_i
  fails = stats.elements['failed_test_cases'].text.to_i
  dirname = nil
  begin
    dir = doc.elements['crpcut/remaining_files'].attributes['nonempty_dir']
    Dir::rmdir(dir)
  rescue SystemCallError
    report = true
    puts "\nworking dir not empty"
  rescue
  end
  t = collection.delete('FAILED')
  count = (t && t.size) || 0
  if count != fail_count then
    puts "\n#{count} failed cases, expected #{fail_count}"
    report = true
  end
  if fails != fail_count then
    puts "\n#{fails} failed in statistics, expected #{fail_count}"
    report = true
  end
  if runs != run_count then
    puts "\n#{runs} run tests in statistics, expected #{run_count}"
    report = true
  end
  if regs != tests.size then
    puts "\n#{regs} registered tests in statistics, expected #{tests.size}"
report = true
  end
  t = collection.delete('PASSED');
  count = (t && t.size) || 0
  if count != success_count then
    puts "\n#{count} successful found, expected #{success_count}"
    report = true
  end
  if collection.size > 0 then
    puts "\n"
    report = true
    collection.each do |r, l|
      puts r
      l.each { |name| puts "  #{name}" }
    end
  end
  puts "PASSED" if !report
  !report
end

RUNS={
  "            default_success" =>
  [ /default_success/,    /PASSED/,      1,  0,  0, BLOCKED_TESTS ],

  "            asserts" =>
  [ /^asserts::/,         /FAILED/, 47, 30,  0, BLOCKED_TESTS ],

  " -v         asserts" =>
  [ /^asserts::/,         /.*/,     47, 30, 17, BLOCKED_TESTS ],

  " -c 8       asserts" =>
  [ /^asserts::/,         /FAILED/, 47, 30,  0, BLOCKED_TESTS ],

  " -c 8 -v    asserts" =>
  [ /^asserts::/,         /.*/,     47, 30, 17, BLOCKED_TESTS ],

  " -n         asserts" =>
  [ /^asserts::/,         /FAILED/, 47, 30,  0, [] ],

  " -n -v      asserts" =>
  [ /^asserts::/,         /.*/,     47, 30, 17, [] ],

  " -n -c 8    asserts" =>
  [ /^asserts::/,         /FAILED/, 47, 30,  0, [] ],

  " -n -c 8 -v asserts" =>
  [ /^asserts::/,         /.*/,     47, 30, 17, [] ],

  "            asserts death" =>
  [ /^(asserts|death)::/, /FAILED/, 72, 49,  0, BLOCKED_TESTS ],

  " -v         asserts death" =>
  [ /^(asserts|death)::/, /.*/,     72, 49, 23, BLOCKED_TESTS ],

  " -c 8       asserts death" =>
  [ /^(asserts|death)::/, /FAILED/, 72, 49,  0, BLOCKED_TESTS ],

  " -c 8 -v    asserts death" =>
  [ /^(asserts|death)::/, /.*/,     72, 49, 23, BLOCKED_TESTS ],

  " -n         asserts death" =>
  [ /^(asserts|death)::/, /FAILED/, 72, 49,  0, [] ],

  " -n -v      asserts death" =>
  [ /^(asserts|death)::/, /.*/,     72, 49, 23, [] ],

  " -n -c 8    asserts death" =>
  [ /^(asserts|death)::/, /FAILED/, 72, 49,  0, [] ],

  " -n -c 8 -v asserts death" =>
  [ /^(asserts|death)::/, /.*/,     72, 49, 23, [] ],

  ""         =>
  [ /.*/,                 /FAILED/, tests.size - BLOCKED_TESTS.size, fails,  0, BLOCKED_TESTS ],

  " -v"      =>
  [ /.*/,                 /.*/,     tests.size - BLOCKED_TESTS.size, fails, oks - BLOCKED_TESTS.size, BLOCKED_TESTS ],

  " -c 8"    =>
  [ /.*/,                 /FAILED/, tests.size - BLOCKED_TESTS.size, fails,  0, BLOCKED_TESTS ],

  " -c 8 -v" =>
  [ /.*/,                 /.*/,     tests.size - BLOCKED_TESTS.size, fails, oks - BLOCKED_TESTS.size, BLOCKED_TESTS ],

  " -n"         =>
  [ /.*/,                 /FAILED/, tests.size, fails,  0,                       [] ],

  " -n -v"      =>
  [ /.*/,                 /.*/,     tests.size, fails, oks, [] ],

  " -n -c 8"    =>
  [ /.*/,                 /FAILED/, tests.size, fails,  0,                       [] ],

  " -n -c 8 -v" =>
  [ /.*/,                 /.*/,     tests.size, fails, oks, [] ],

  " -x -c 8 -o /tmp/crpcutst$$ -q;v=$?;cat /tmp/crpcutst$$;rm /tmp/crpcutst$$;exit $v" =>
  [ /.*/,                 /FAILED/, tests.size - BLOCKED_TESTS.size, fails,  0, BLOCKED_TESTS ]
}

File.open("apafil", 'w') { |f| f.write("apa\n") }
ulimit = open("|bash -c \"ulimit -c\"").read.chomp
if ulimit != "unlimited" && ulimit.to_i == 0  then
  puts "You must allow core dumps for the selt test to succeed."
  puts "Do that by issuing the command:"
  puts "> ulimit -c 100000"
  exit 1
end
puts "Self test takes a little over a minute to complete"
RUNS.each do | params, expects |
  print "%-70s: " % "./test/testprog -x -p apa=katt#{params}"
  STDOUT.flush
  exit 1 if !check_file("./test/testprog -x -p apa=katt#{params}", tests, *expects)
end
dirname = "/tmp/crpcut_selftest_dir_#{$$}"
Dir.mkdir(dirname)
testname="should_fail_due_to_left_behind_files"
prog="./test/testprog -o /dev/null -q -d #{dirname} #{testname}"
print "%-70s: " % prog
file = open("|#{prog}")
s = file.read
file.close
is_error=false
if !s.empty? then
  puts
  puts "Unexpected stdout for -q"
  is_error = true
end
file_found = nil
begin
  file_found = File.stat("#{dirname}/#{testname}/apa").file?
rescue
end
if !file_found then
  puts if !is_error
  puts "File was not created"
  is_error = true
end
begin
  File.unlink "#{dirname}/#{testname}/apa"
  Dir.rmdir "#{dirname}/#{testname}"
  Dir.rmdir dirname
rescue
  puts if !is_error
  puts "Couldn't remove created files"
  is_error = true
end
puts "PASSED" if !is_error
File.unlink "./apafil"

begin
  File.unlink "./core"
rescue
  # do nothing, we don't care. Just as long as it's not there
end
testname="asserts::should_fail_void_ptr_eq_ptr"
prog="./test/testprog -s #{testname}"
print "%-70s: " % prog
file = open("|#{prog}")
s = file.read
file.close
is_error=false
file_found = nil
begin
  file_found = File.stat("./core").file?
rescue
end
if !file_found then
  puts if !is_error
  puts "File was not created"
  is_error = true
end
begin
  File.unlink "./core"
rescue
  puts if !is_error
  puts "Couldn't remove created files"
  is_error = true
end
puts "PASSED" if !is_error

