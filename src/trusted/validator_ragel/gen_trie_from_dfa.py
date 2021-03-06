# Copyright (c) 2013 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A script to dump a trie from a validator DFA."""

import multiprocessing
import optparse
import sys
import traceback

import dfa_parser
import dfa_traversal
import trie
import validator

NOP = 0x90


def PadToBundleSize(byte_list):
  """Return bytes appended with NOPs to form a full bundle."""
  assert len(byte_list) <= validator.BUNDLE_SIZE
  return byte_list + [NOP] * (validator.BUNDLE_SIZE - len(byte_list))


# All the conditions for an x86_64 instruction under which it may be valid.
# Any register might be restricted. None is included because it indicates
# that no register is restricted coming into the instruction.
ALL_CONDITIONS = set(validator.ALL_REGISTERS + [None])


def CheckAndFormatRRInfo(input_rr_set, output_rr):
  """Formats restricted register information in order to store them in a trie.

  Args:
    input_rr_set: The set of allowed input restricted registers. This is
    either an individual register that is required to be restriced (or
    encodes that RSP/RBP cannot be restricted). Not empty.

    output_rr: An output restricted register produced (or None)

  Returns:
    input_rr_str: A string version encoding the input restricted register.
    output_rr_str: A string version encoding the output restricted register.

  Raises:
    ValueError: If input_rr_set or output_rr is invalid.
  """
  assert input_rr_set
  input_rr_str = None

  # If multiple different restricted registers cause the instruction to be
  # validated, then essentially any nonspecial register should be allowed.
  # Notably, this means for example that either '%r13' or '%r12' being
  # restricted incoming would cause the instruction to be valid but '%r8'
  # being restricted on the input would cause an invalid instruction.
  if len(input_rr_set) > 1:
    if (input_rr_set != (ALL_CONDITIONS - set((validator.NC_REG_RSP,
                                               validator.NC_REG_RBP)))):
      raise ValueError('Invalid input rr set', input_rr_set)
    input_rr_str = 'any_nonspecial'
  else:
    input_rr_str = validator.REGISTER_NAMES[input_rr_set.pop()]

  if output_rr == validator.NC_REG_R15:
    raise ValueError('Instruction produces R15 as a restricted register.')

  if output_rr is None:
    output_rr_str = 'None'
  else:
    output_rr_str = validator.REGISTER_NAMES[output_rr]

  return input_rr_str, output_rr_str


def ValidateInstructionAndGetRR(bitness, instruction, validator_inst):
  """Verify the byte list against the DFA and get restricted register info.

  Even though walking the DFA might result in instruction being accepted,
  the instruction might actually get rejected by a DFA action. So, we run
  the byte sequence through the validator to actually ensure that the bytes
  are accepted before passing them onto a trie. Additionally, extract
  restricted register information for the byte sequence.

  Args:
    bitness: [32 or 64]
    instruction: byte list obtained from walking accept states of the DFA.
    validator_inst: instance of the x86 validator loaded with DFA+actions.
  Returns:
    valid: True/False about whether the byte sequence is actually accepted.
    input_rr: Incoming restricted registers that are allowed to accept the
              sequence. Always empty for the x86-32 case.
    output_rr: Outgoing condition that is generated by the DFA.
               Always empty for the x86-32 case.
  Raises:
    ValueError: If instruction generates multiple output restricted registers.
  """
  bundle = bytes(PadToBundleSize(instruction))
  if bitness == 32:
    result = validator_inst.ValidateChunk(bundle, bitness)
    return (result, '', '')
  else:
    # Walk through all of the conditions of different registers being input
    # restricted, as well as no input rrs being restricted and see if any of
    # them cause the byte sequence to be validated. We need to do this
    # because we are processing an instruction at a time and not a bundle
    # at a time.
    valid_input_rrs = set()
    valid_output_rrs = set()
    for initial_rr in ALL_CONDITIONS:
      valid, final_rr = validator_inst.ValidateAndGetFinalRestrictedRegister(
          bundle, len(instruction), initial_rr)
      if valid:
        valid_input_rrs.add(initial_rr)
        valid_output_rrs.add(final_rr)

    # If no input RRs are allowed then instruction is not valid.
    if not valid_input_rrs:
      return (False, '', '')

    # All valid input RRs should produce one unique output RR (could be None).
    # i.e the produced output rr should not vary based on input RR.
    if len(valid_output_rrs) != 1:
      raise ValueError('Multiple output RRs produced', instruction,
                       valid_output_rrs)
    known_final_rr = valid_output_rrs.pop()

    input_rr_str, output_rr_str = CheckAndFormatRRInfo(
        valid_input_rrs, known_final_rr)
    return (True, input_rr_str, output_rr_str)


class WorkerState(object):
  """Maintains an uncompressed subtrie while receiving accepting instructions.

  The main thread will compress this subtrie into the main trie when this
  worker is done. This is because it's inefficient (and difficult to maintain
  threadsafety) to compress nodes into a common node cache in the workers
  themselves.
  """

  def __init__(self, validator):
    self.total_instructions = 0
    self.num_valid = 0
    self.validator = validator
    self.sub_trie = trie.Node()
    self.node_cache = trie.NodeCache()

  def ReceiveInstruction(self, byte_list):
    """Update trie if sequence passes validator and sandboxing checks."""
    self.total_instructions += 1

    # This is because even though the state is accepting with regards to the
    # DFA, extra DFA actions or other validator rules might make the sequence
    # invalid.
    is_valid, input_rr, output_rr = ValidateInstructionAndGetRR(
        options.bitness, byte_list, self.validator)
    if is_valid:
      self.num_valid += 1
      trie.AddToUncompressedTrie(
          self.sub_trie, map(str, byte_list),
          trie.AcceptInfo(input_rr=input_rr, output_rr=output_rr))


def Worker(dfa_prefix_and_state_index):
  """Traverse a portion of the DFA, and compute the associated subtrie."""
  dfa_prefix, dfa_state_index = dfa_prefix_and_state_index
  worker_state = WorkerState(worker_validator)

  try:
    dfa_traversal.TraverseTree(
        dfa.states[dfa_state_index],
        final_callback=worker_state.ReceiveInstruction,
        prefix=dfa_prefix,
        anyfield=0)

  except Exception:
    traceback.print_exc()  # because multiprocessing imap swallows traceback
    raise

  worker_state.sub_trie = worker_state.node_cache.Merge(
      worker_state.node_cache.empty_node, worker_state.sub_trie)

  return (
      worker_state.total_instructions,
      worker_state.num_valid,
      worker_state.sub_trie)


def ParseOptions():
  """Parse command line options."""
  parser = optparse.OptionParser(usage='%prog [options] xmlfile')

  parser.add_option('--bitness',
                    choices=['32', '64'],
                    help='The subarchitecture: 32 or 64')
  parser.add_option('--validator_dll',
                    help='Path to librdfa_validator_dll')
  parser.add_option('--decoder_dll',
                    help='Path to librdfa_decoder_dll')
  parser.add_option('--trie_path',
                    help='Path to write the output trie to')

  options, args = parser.parse_args()
  options.bitness = int(options.bitness)

  if not options.trie_path:
    parser.error('specify an output path to a trie')

  if len(args) != 1:
    parser.error('specify one xml file')

  xml_file, = args

  return options, xml_file


def main():
  # We keep these global to share state graph between workers spawned by
  # multiprocess. Passing it every time is slow.
  global options, xml_file
  global dfa
  global worker_validator
  options, xml_file = ParseOptions()
  dfa = dfa_parser.ParseXml(xml_file)
  worker_validator = validator.Validator(
      validator_dll=options.validator_dll,
      decoder_dll=options.decoder_dll)

  assert dfa.initial_state.is_accepting
  assert not dfa.initial_state.any_byte

  sys.stderr.write('%d states\n' % len(dfa.states))
  num_suffixes = dfa_traversal.GetNumSuffixes(dfa.initial_state)
  num_instructions = sum(
      num_suffixes[t.to_state]
      for t in dfa.initial_state.forward_transitions.values())
  sys.stderr.write('%d instructions\n' % num_instructions)
  tasks = dfa_traversal.CreateTraversalTasks(dfa.states,
                                             dfa.initial_state)
  sys.stderr.write('%d tasks\n' % len(tasks))

  pool = multiprocessing.Pool()
  results = pool.imap_unordered(Worker, tasks)

  total = 0
  num_valid = 0

  node_cache = trie.NodeCache()
  full_trie = node_cache.empty_node

  # The individual workers create subtries that we merge in and compress here.
  for count, valid_count, sub_trie, in results:
    total += count
    num_valid += valid_count
    full_trie = node_cache.Merge(full_trie, sub_trie)
    sys.stderr.write('%.2f%% completed\n' % (total * 100.0 / num_instructions))
  sys.stderr.write('%d instructions were processed\n' % total)
  sys.stderr.write('%d valid instructions\n' % num_valid)

  trie.WriteToFile(options.trie_path, full_trie)

if __name__ == '__main__':
  main()
