#!/usr/bin/python
#
# Copyright (c) 2012 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

"""
Responsible for generating the testing decoders based on
parsed table representations.
"""

# This file generates testing code for our class decoder. The decoder
# tables are specifically written to minimize the number of decoder
# classes needed to parse valid ARM instructions. For testing, this is
# a problem. We can't (easily) tell if the intended instruction rules
# of ARM are being met, since there is not a one-to-one mapping from
# class decoders to rules.
#
# For example, consider the following two rows (from armv7.table):
#
# | 0011x      -        = Binary4RegisterShiftedOp => Defs12To15RdRnRsRmNotPc
#                         Rsb_Rule_144_A1_P288
#                         cccc0000011snnnnddddssss0tt1mmmm
#                         RegsNotPc
# | 0100x      -        = Binary4RegisterShiftedOp => Defs12To15RdRnRsRmNotPc
#                         Add_Rule_7_A1_P26
#                         cccc0000100snnnnddddssss0tt1mmmm
#                         RegsNotPc
#
# Both rows state to return a Binary4RegisterShiftedOp class decoder.
# The sequence of four symbols correspond to (in order presented):
#
#    baseline - The name of the class decoder that should be used for testing.
#    actual - The name of the class decoder to use in sel_ldr
#    rule - A unique name identifying the rule from the manual that
#       defines what the selected class decoder is to decode.
#    pattern - The sequence of bits defines by the rule (above)
#    constraints - Any additional constraints assumed by the rule.
#
# All but the baseline is optional. The remaining fields provide
# additional documentation and information for testing (which is used
# by this file). If the actual is not specified (prefixed by '=>')
# then it is assumed to have the same value as the baseline.
#
# If these two rows had a mergable bit pattern (which they do not),
# these rows would still not mergable since the actions are
# different. However, for sel_ldr, they both state to use a
# Binary4RegisterShiftedOp. The remaining identifiers are added data
# for testing only.
#
# We fix this by defining a notion of "action_filter" where one can
# choose to keep only those fields that are applicable. For sel_ldr,
# it's only 'actual'. For testing, it will include other fields,
# depending on the context.
#
# Note: The current ARM instruction table has both new and old
# actions. Old actions only define the 'InstClass' entry. If the
# remaining fields are omitted, the corresponding testing for those
# entries are omitted.
#
# Note: See dgen_decoder_output.py for more details on how we build a
# decoder for sel_ldr.
#
# For testing, we would like to know the specific instruction rule
# that was being tested. Further, we would like to know what
# instruction rule was chosen for each decoder class selection made by
# the parse tables.  To do this, we do two levels of wrapping.
#
# This file generates a set of wrapper classes, each a subclass of
# NamedClassDecoder. One is generated for each InstClass needed by
# sel_ldr (i.e. only the 'actual' field). These named classes correspond
# to what sel_ldr will select.
#
# The named version of each named InstClass is:
#
#  class NamedInstClass : public NamedClassDecoder {
#   public:
#    inline NamedInstClass()
#        : NamedClassDecoder(decoder_, "InstClass")
#    {}
#  virtual ~NamedInstClass() {}
#
# private:
#  Binary3RegisterShiftedTest decoder_;
#  NACL_DISALLOW_COPY_AND_ASSIGN(NamedInstClass);
#};
#
# This makes sure that each decoder class can be identified using a
# separate class decoder. For rows without rules, the corresponding
# named class 'NamedInstClass' will be used. If a row also has
# a rule, the 'NamedInstClass' is converted to 'NamedRuleInstClass' where
# 'Rule' is the name of the rule.
#
# The base class for NamedClassDecoder is specified in
# "named_class_decoder.h".  This file defines a class that takes a
# ClassDecoder (reference) C and a print name NAME, and builds a
# corresponding ClassDecoder that acts like C, but will print out
# NAME. The behaviour of C is maintained by dispatching each virtual
# on the NamedClassDecoder to the corresponding virtual on C.
#
# We then define the class decoder Decoder, by defining a derived
# instance of DecoderState as follows:
#
# class NamedDecoder : DecoderState {
#  public:
#   explicit NamedDecoder();
#   virtual ~NamedDecoder();
#  const NamedClassDecoder& decode_named(const Instruction) const;
#  virtual const ClassDecoder& decode(const Instruction) const;
#  ...
# };
#
# The method decode is the expected API for the NamedDecoder, which is
# an instance of DecoderState (defined in decode.h). The method
# decode_named is the same, but returns NamedClassDecoder's so that
# good error messages can be generated by the test harnesses for
# ClassDecoder's (see decoder_tester.h for more details on
# ClassDecoder test harnesses).
#
# To the NamedDecoder, we add a constant field NamedClassDecoder for
# each possible class decoder method decode_named could return, or
# that we could use in automatically generated tests. These fields
# allow us to only create the corresponding decoder classes once
# (during constructor initialization).
#
# Finally, we add a method corresponding to each defined decoder
# table.  The forms of these decoders is:
#
#  inline const NamedClassDecoder& decode_TABLE(
#     const nacl_arm_dec::Instruction insn) const;
#
# Each of these methods are defined as inline methods so that they can
# be optimized away in the corresponding top level methods (i.e.
# decode_named and decode).
#
# For testing, there are three files generated:
#
#     decoder_named_classes.h
#     decoder_named_decoder.h
#     decoder_named.cc
#     decoder_tests.cc
#
# File decoder_named_classes.h defines the class declarations for the
# generated Rule classes, and named class decoder classes. File
# decoder_named_decoder.h defines the decoder class NamedDecoder
# (discussed above). decoder_named.cc contains the corresponding
# implementations of the constructors and methods of these classes.
#
# decoder_tests.cc generates an automatic test harness executable,
# that will test each instruction Rule. Each test generates all
# possible matches the the corresponding Pattern of the table rule,
# and calls the corresponding tester associated with the class decoder
# of that row. By default, the tester is presumed to be named.
#
#    InstClassTester
#
# If the row defines a Constraints identifier, then the tester
#
#    InstClassTesterConstraints
#
# is used instead.

import dgen_core
import dgen_opt
import dgen_output

"""The current command line arguments to use"""
_cl_args = {}

# The following defines naming conventions used for identifiers.
# Note: DECODER will be replaced by 'actual' and 'baseline', defining
# how both types of symbols are generated.

CLASS = '%(DECODER)s_%(rule)s'
NAMED_CLASS = 'Named%(DECODER)s_%(rule)s'
INSTANCE = '%(DECODER_class)s_instance_'
BASE_TESTER='%(decoder_base)sTester%(constraints)s'
DECODER_TESTER='%(baseline)sTester_%(rule)s_%(constraints)s'

def _install_action(decoder, action, values):
  # Installs common names needed from the given decoder action. This
  # code is somewhat inefficient in that most cases, most of the
  # added strings are not needed. On the other hand, by having a
  # single routine that generates all action specific names at one
  # spot, it is much easier to change definitions.
  values['baseline'] = action.baseline
  values['actual'] = action.actual
  values['decoder_base'] = decoder.base_class(values['baseline'])
  values['rule'] = action.rule
  values['constraints'] = action.constraints if action.constraints else ''
  values['pattern'] = action.pattern
  values['baseline_class'] =  _decoder_replace(CLASS, 'baseline') % values
  values['actual_class'] = _decoder_replace(CLASS, 'actual') % values
  _install_baseline_and_actuals('named_DECODER_class', NAMED_CLASS, values)
  _install_baseline_and_actuals('DECODER_instance', INSTANCE, values)
  values['base_tester'] = BASE_TESTER % values
  values['decoder_tester'] = DECODER_TESTER % values

def _decoder_replace(string, basis):
  return string.replace('DECODER', basis)

def _install_key_pattern(key, pattern, basis, values):
  # Replace DECODER in key and pattern with basis, then
  # install into values.
  values[_decoder_replace(key, basis)] = (
      _decoder_replace(pattern, basis) % values)

def _install_baseline_and_actuals(key, pattern, values):
  # Replace DECODER with 'baseline' and 'actual', apply it
  # to the key and pattern, and then install into values.
  for basis in ['baseline', 'actual']:
    _install_key_pattern(key, pattern, basis, values)

def _generate_baseline_and_actual(code, symbol, decoder,
                                  values, out, actions=['rule']):
  """ Generates code to define the given symbol. Does so for both
      baseline and actual decoders, filtering using actions.

      code - The code to generate.
      symbol - The symbol being defined.
      decoder - The decoder (tables) to use.
      values - The name map to use to generate code.
      actions - The fields to keep when generating code.
  """
  generated_symbols = set()

  # Generate one for each type of basline decoder.
  baseline_actions = actions[:]
  baseline_actions.insert(0, 'baseline');
  baseline_code = _decoder_replace(code, 'baseline')
  baseline_symbol = _decoder_replace(symbol, 'baseline');
  for d in decoder.action_filter(baseline_actions).decoders():
    _install_action(decoder, d, values);
    sym_name = (baseline_symbol % values)
    if sym_name not in generated_symbols:
      out.write(baseline_code % values)
      generated_symbols.add(sym_name)

  # Generate one for each actual type that is different than the
  # baseline.
  actual_actions = actions[:]
  actual_actions.insert(0, 'actual-not-baseline')
  actual_code = _decoder_replace(code, 'actual')
  actual_symbol = _decoder_replace(symbol, 'actual')
  for d in decoder.action_filter(actual_actions).decoders():
    # Note: 'actual-not-baseline' sets actual to None if same as baseline.
    if d.actual:
      _install_action(decoder, d, values);
      sym_name = (actual_symbol % values)
      if sym_name not in generated_symbols:
        out.write(actual_code % values)
        generated_symbols.add(sym_name)

# Defines the header for decoder_named_classes.h
NAMED_CLASSES_H_HEADER="""%(FILE_HEADER)s
%(NOT_TCB_MESSAGE)s

#ifndef %(IFDEF_NAME)s
#define %(IFDEF_NAME)s

#include "native_client/src/trusted/validator_arm/actual_classes.h"
#include "native_client/src/trusted/validator_arm/baseline_classes.h"
#include "native_client/src/trusted/validator_arm/named_class_decoder.h"
"""

RULE_CLASSES_HEADER="""
/*
 * Define rule decoder classes.
 */
namespace nacl_arm_dec {

"""

RULE_CLASS="""class %(DECODER_class)s
    : public %(DECODER)s {
 public:
  virtual ~%(DECODER_class)s() {}
};

"""

RULE_CLASS_SYM="%(DECODER_class)s"

NAMED_DECODERS_HEADER="""}  // nacl_arm_dec

namespace nacl_arm_test {

/*
 * Define named class decoders for each class decoder.
 * The main purpose of these classes is to introduce
 * instances that are named specifically to the class decoder
 * and/or rule that was used to parse them. This makes testing
 * much easier in that error messages use these named classes
 * to clarify what row in the corresponding table was used
 * to select this decoder. Without these names, debugging the
 * output of the test code would be nearly impossible
 */

"""

NAMED_CLASS_DECLARE="""class %(named_DECODER_class)s
    : public NamedClassDecoder {
 public:
  inline %(named_DECODER_class)s()
    : NamedClassDecoder(decoder_, "%(DECODER)s %(rule)s")
  {}
  virtual ~%(named_DECODER_class)s() {}

 private:
  nacl_arm_dec::%(DECODER_class)s decoder_;
  NACL_DISALLOW_COPY_AND_ASSIGN(%(named_DECODER_class)s);
};

"""

NAMED_CLASS_DECLARE_SYM="%(named_DECODER_class)s"

NAMED_CLASSES_H_FOOTER="""
// Defines the default parse action if the table doesn't define
// an action.
class NotImplementedNamed : public NamedClassDecoder {
 public:
  inline NotImplementedNamed()
    : NamedClassDecoder(decoder_, "not implemented")
  {}
  virtual ~NotImplementedNamed() {}

 private:
  nacl_arm_dec::NotImplemented decoder_;
  NACL_DISALLOW_COPY_AND_ASSIGN(NotImplementedNamed);
};

} // namespace nacl_arm_test
#endif  // %(IFDEF_NAME)s
"""

def generate_named_classes_h(decoder, decoder_name, filename, out, cl_args):
  """Defines named classes needed for decoder testing.

  Args:
    tables: list of Table objects to process.
    decoder_name: The name of the decoder state to build.
    filename: The (localized) name for the .h file.
    out: a COutput object to write to.
    cl_args: A dictionary of additional command line arguments.
  """
  global _cl_args
  if not decoder.primary: raise Exception('No tables provided.')
  _cl_args = cl_args

  values = {
      'FILE_HEADER': dgen_output.HEADER_BOILERPLATE,
      'NOT_TCB_MESSAGE' : dgen_output.NOT_TCB_BOILERPLATE,
      'IFDEF_NAME' : dgen_output.ifdef_name(filename),
      'decoder_name': decoder_name,
      }
  out.write(NAMED_CLASSES_H_HEADER % values)
  out.write(RULE_CLASSES_HEADER)
  _generate_baseline_and_actual(RULE_CLASS, RULE_CLASS_SYM,
                                decoder, values, out)
  out.write(NAMED_DECODERS_HEADER)
  _generate_baseline_and_actual(NAMED_CLASS_DECLARE, NAMED_CLASS_DECLARE_SYM,
                                decoder, values, out)
  out.write(NAMED_CLASSES_H_FOOTER % values)

NAMED_DECODER_H_HEADER="""%(FILE_HEADER)s
%(NOT_TCB_MESSAGE)s

#ifndef %(IFDEF_NAME)s
#define %(IFDEF_NAME)s

#include "native_client/src/trusted/validator_arm/decode.h"
#include "%(FILENAME_BASE)s_classes.h"
#include "native_client/src/trusted/validator_arm/named_class_decoder.h"

namespace nacl_arm_test {

// Defines a (named) decoder class selector for instructions
class Named%(decoder_name)s : nacl_arm_dec::DecoderState {
 public:
  explicit Named%(decoder_name)s();
  virtual ~Named%(decoder_name)s();

  // Parses the given instruction, returning the named class
  // decoder to use.
  const NamedClassDecoder& decode_named(
     const nacl_arm_dec::Instruction) const;

  // Parses the given instruction, returning the class decoder
  // to use.
  virtual const nacl_arm_dec::ClassDecoder& decode(
     const nacl_arm_dec::Instruction) const;

  // The following fields define the set of class decoders
  // that can be returned by the API function "decode_named". They
  // are created once as instance fields, and then returned
  // by the table methods above. This speeds up the code since
  // the class decoders need to only be bulit once (and reused
  // for each call to "decode_named")."""

DECODER_STATE_FIELD="""
  const %(named_DECODER_class)s %(DECODER_instance)s;"""

DECODER_STATE_FIELD_NAME="%(named_DECODER_class)s"

DECODER_STATE_DECODER_COMMENTS="""
 private:

  // The following list of methods correspond to each decoder table,
  // and implements the pattern matching of the corresponding bit
  // patterns. After matching the corresponding bit patterns, they
  // either call other methods in this list (corresponding to another
  // decoder table), or they return the instance field that implements
  // the class decoder that should be used to decode the particular
  // instruction."""

DECODER_STATE_DECODER="""
  inline const NamedClassDecoder& decode_%(table)s(
      const nacl_arm_dec::Instruction insn) const;"""

NAMED_DECODER_H_FOOTER="""
  // Defines default action if parse tables don't define what action
  // to take.
  const NotImplementedNamed not_implemented_;
};

} // namespace nacl_arm_test
#endif  // %(IFDEF_NAME)s
"""

def generate_named_decoder_h(decoder, decoder_name, filename, out, cl_args):
    """Generates the named decoder for testing.

    Args:
        tables: list of Table objects to process.
        decoder_name: The name of the decoder state to build.
        filename: The (localized) name for the .h file.
        out: a COutput object to write to.
        cl_args: A dictionary of additional command line arguments.
    """
    global _cl_args
    if not decoder.primary: raise Exception('No tables provided.')
    assert filename.endswith('_decoder.h')
    _cl_args = cl_args

    values = {
        'FILE_HEADER': dgen_output.HEADER_BOILERPLATE,
        'NOT_TCB_MESSAGE' : dgen_output.NOT_TCB_BOILERPLATE,
        'IFDEF_NAME' : dgen_output.ifdef_name(filename),
        'FILENAME_BASE': filename[:-len('_decoder.h')],
        'decoder_name': decoder_name,
        }
    out.write(NAMED_DECODER_H_HEADER % values)
    _generate_baseline_and_actual(DECODER_STATE_FIELD, DECODER_STATE_FIELD_NAME,
                                  decoder, values, out)
    out.write(DECODER_STATE_DECODER_COMMENTS)
    for table in decoder.tables():
      values['table'] = table.name
      out.write(DECODER_STATE_DECODER % values)
    out.write(NAMED_DECODER_H_FOOTER % values)

# Defines the source for DECODER_named.cc
NAMED_CC_HEADER="""%(FILE_HEADER)s
%(NOT_TCB_MESSAGE)s
#include "%(FILENAME_BASE)s_decoder.h"

using nacl_arm_dec::ClassDecoder;
using nacl_arm_dec::Instruction;

namespace nacl_arm_test {

Named%(decoder_name)s::Named%(decoder_name)s()
{}

Named%(decoder_name)s::~Named%(decoder_name)s() {}
"""

PARSE_TABLE_METHOD_HEADER="""
/*
 * Implementation of table %(table_name)s.
 * Specified by: %(citation)s
 */
const NamedClassDecoder& Named%(decoder_name)s::decode_%(table_name)s(
     const nacl_arm_dec::Instruction insn) const {
"""

METHOD_DISPATCH_BEGIN="""
  if (%s"""

METHOD_DISPATCH_CONTINUE=""" &&
      %s"""

METHOD_DISPATCH_END=")"""

PARSE_TABLE_METHOD_ROW="""
    return %(action)s;
"""

PARSE_TABLE_METHOD_FOOTER="""
  // Catch any attempt to fall through...
  return not_implemented_;
}

"""

NAMED_CC_FOOTER="""
const NamedClassDecoder& Named%(decoder_name)s::
decode_named(const nacl_arm_dec::Instruction insn) const {
  return decode_%(entry_table_name)s(insn);
}

const nacl_arm_dec::ClassDecoder& Named%(decoder_name)s::
decode(const nacl_arm_dec::Instruction insn) const {
  return decode_named(insn).named_decoder();
}

}  // namespace nacl_arm_test
"""

def generate_named_cc(decoder, decoder_name, filename, out, cl_args):
    """Implementation of the test decoder in .cc file

    Args:
        tables: list of Table objects to process.
        decoder_name: The name of the decoder state to build.
        filename: The (localized) name for the .h file.
        out: a COutput object to write to.
        cl_args: A dictionary of additional command line arguments.
    """
    global _cl_args
    if not decoder.primary: raise Exception('No tables provided.')
    assert filename.endswith('.cc')
    _cl_args = cl_args

    values = {
        'FILE_HEADER': dgen_output.HEADER_BOILERPLATE,
        'NOT_TCB_MESSAGE' : dgen_output.NOT_TCB_BOILERPLATE,
        'FILENAME_BASE' : filename[:-len('.cc')],
        'decoder_name': decoder_name,
        'entry_table_name': decoder.primary.name,
        }
    out.write(NAMED_CC_HEADER % values)
    _generate_decoder_method_bodies(decoder, values, out)
    out.write(NAMED_CC_FOOTER % values)

def _generate_decoder_method_bodies(decoder, values, out):
  for table in decoder.tables():
    # Add the default row as the last in the optimized row, so that
    # it is applied if all other rows do not.
    opt_rows = sorted(
        dgen_opt.optimize_rows(
            table.action_filter(['baseline', 'rule']).rows(False)))
    if table.default_row:
      opt_rows.append(table.default_row)

    opt_rows = table.add_column_to_rows(opt_rows)
    print ("Table %s: %d rows minimized to %d"
           % (table.name, len(table.rows()), len(opt_rows)))

    values['table_name'] = table.name
    values['citation'] = table.citation,
    out.write(PARSE_TABLE_METHOD_HEADER % values)

    # Add message to stop compilation warnings if this table
    # doesn't require subtables to select a class decoder.
    if not table.methods():
      out.write("  UNREFERENCED_PARAMETER(insn);")

    for row in opt_rows:
      if row.action.__class__.__name__ == 'DecoderAction':
        _install_action(decoder, row.action, values)
        action = '%(baseline_instance)s' % values
      elif row.action.__class__.__name__ == 'DecoderMethod':
        action = 'decode_%s(insn)' % row.action.name
      else:
        raise Exception('Bad table action: %s' % row.action)
      # Each row consists of a set of bit patterns defining if the row
      # is applicable. Convert this into a sequence of anded C test
      # expressions. For example, convert the following pair of bit
      # patterns:
      #
      #   xxxx1010xxxxxxxxxxxxxxxxxxxxxxxx
      #   xxxxxxxxxxxxxxxxxxxxxxxxxxxx0101
      #
      # Each instruction is masked to get the the bits, and then
      # tested against the corresponding expected bits. Hence, the
      # above example is converted to:
      #
      #    ((insn & 0x0F000000) != 0x0C000000) &&
      #    ((insn & 0x0000000F) != 0x00000005)
      out.write(METHOD_DISPATCH_BEGIN %
                row.patterns[0].to_c_expr('insn.Bits()'))
      for p in row.patterns[1:]:
        out.write(METHOD_DISPATCH_CONTINUE % p.to_c_expr('insn.Bits()'))
      out.write(METHOD_DISPATCH_END)
      values['action'] = action
      out.write(PARSE_TABLE_METHOD_ROW % values)
    out.write(PARSE_TABLE_METHOD_FOOTER % values)

# Define the source for DECODER_tests.cc
TEST_CC_HEADER="""%(FILE_HEADER)s
%(NOT_TCB_MESSAGE)s

#include "gtest/gtest.h"
#include "native_client/src/trusted/validator_arm/actual_vs_baseline.h"
#include "native_client/src/trusted/validator_arm/actual_classes.h"
#include "native_client/src/trusted/validator_arm/baseline_classes.h"
#include "native_client/src/trusted/validator_arm/inst_classes_testers.h"

namespace nacl_arm_test {

// Generates a derived class decoder tester for each decoder action
// in the parse tables. This derived class introduces a default
// constructor that automatically initializes the expected decoder
// to the corresponding instance in the generated DecoderState.
"""

TESTER_CLASS="""
class %(decoder_tester)s
    : public %(base_tester)s {
 public:
  %(decoder_tester)s()
    : %(base_tester)s(
      state_.%(baseline_instance)s)
  {}
};
"""

TEST_HARNESS="""
// Defines a gtest testing harness for tests.
class %(decoder_name)sTests : public ::testing::Test {
 protected:
  %(decoder_name)sTests() {}
};

// The following test each pattern specified in parse decoder tables.
"""

TEST_FUNCTION_ACTUAL_VS_BASELINE="""
TEST_F(%(decoder_name)sTests,
       %(decoder_tester)s_%(pattern)s_Test) {
  %(decoder_tester)s baseline_tester;
  %(named_actual_class)s actual;
  ActualVsBaselineTester a_vs_b_tester(actual, baseline_tester);
  a_vs_b_tester.Test("%(pattern)s");
}
"""

TEST_FUNCTION_BASELINE="""
TEST_F(%(decoder_name)sTests,
       %(decoder_tester)s_%(pattern)s_Test) {
  %(decoder_tester)s tester;
  tester.Test("%(pattern)s");
}
"""

TEST_CC_FOOTER="""
}  // namespace nacl_arm_test

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
"""

def generate_tests_cc(decoder, decoder_name, out, cl_args, tables):
  global _cl_args
  if not decoder.primary: raise Exception('No tables provided.')
  _cl_args = cl_args

  values = {
      'FILE_HEADER': dgen_output.HEADER_BOILERPLATE,
      'NOT_TCB_MESSAGE' : dgen_output.NOT_TCB_BOILERPLATE,
      'decoder_name': decoder_name,
      }
  out.write(TEST_CC_HEADER % values)
  _generate_rule_testers(decoder, values, out)
  out.write(TEST_HARNESS % values)
  _generate_test_patterns(_decoder_restricted_to_tables(decoder, tables),
                          values, out)
  out.write(TEST_CC_FOOTER % values)

def _generate_rule_testers(decoder, values, out):
  for d in decoder.action_filter(['baseline', 'rule', 'constraints']).rules():
    _install_action(decoder, d, values)
    out.write(TESTER_CLASS % values)

def _decoder_restricted_to_tables(decoder, tables):
  if not tables:
    return decoder
  new_decoder = dgen_core.Decoder()
  for tbl in [tbl for tbl in decoder.tables() if tbl.name in tables]:
    new_decoder.add(tbl)
  return new_decoder

def _generate_test_patterns(decoder, values, out):
  for d in decoder.decoders():
    if d.pattern:
      _install_action(decoder, d, values)
      if d.actual == d.baseline:
        out.write(TEST_FUNCTION_BASELINE % values)
      else:
        out.write(TEST_FUNCTION_ACTUAL_VS_BASELINE % values)
