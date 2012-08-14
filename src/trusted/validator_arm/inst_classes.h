/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_ARM_INST_CLASSES_H_
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_ARM_INST_CLASSES_H_

#include <stdint.h>
#include "native_client/src/include/nacl_macros.h"
#include "native_client/src/include/portability.h"
#include "native_client/src/trusted/validator_arm/model.h"

/*
 * Models the "instruction classes" that the decoder produces.
 */
namespace nacl_arm_dec {

// Used to describe whether an instruction is safe, and if not, what the issue
// is.  Only instructions that MAY_BE_SAFE should be allowed in untrusted code,
// and even those may be rejected by the validator.
enum SafetyLevel {
  // The initial value of uninitialized SafetyLevels -- treat as unsafe.
  UNKNOWN = 0,

  // This instruction is left undefined by the ARMv7 ISA spec.
  UNDEFINED,
  // This instruction is not recognized by the decoder functions.
  NOT_IMPLEMENTED,
  // This instruction has unpredictable effects at runtime.
  UNPREDICTABLE,
  // This instruction is deprecated in ARMv7.
  DEPRECATED,

  // This instruction is forbidden by our SFI model.
  FORBIDDEN,
  // This instruction's operands are forbidden by our SFI model.
  FORBIDDEN_OPERANDS,

  // This instruction may be safe in untrusted code: in isolation it contains
  // nothing scary, but the validator may overrule this during global analysis.
  MAY_BE_SAFE
};

// Returns the safety level associated with i (or UNKNOWN if no such safety
// level exists).
SafetyLevel Int2SafetyLevel(uint32_t i);

// ------------------------------------------------------------------
// The following list of Interface classes are "mixed" into the class
// decoders as static constant fields. The point of these interfaces
// is to control access to data fields within the scope of the
// instruction class decoder, using higher level symbolic names.
//
// For example, register Rn may be located in different bit sequences
// in different instructions. However, they all refer to the concept
// of register Rn (some use bits 0..3 while others use bits
// 16..19). The interfaces for each possible Rn is integrated as a
// static const field named n. Hence virtuals can now use n.reg() to get
// the corresponding register within its virtual functions.
//
// Note: These fields are static const, and the corresponding methods are
// static so that the reference to the static field is only used to
// control scope look ups, and will be optimized out of compiled code.
// ------------------------------------------------------------------

// Interface class to pull out shift type from bits 5 through 6
class ShiftTypeBits5To6Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(6, 5);
  }
  // Converts the given immediate value using the shift type specified
  // by this interface. Defined in A8.4.3, page A8-11.
  static uint32_t DecodeImmShift(Instruction insn, uint32_t imm5_value) {
    return ComputeDecodeImmShift(value(insn), imm5_value);
  }
 private:
  static uint32_t ComputeDecodeImmShift(uint32_t shift_type,
                                        uint32_t imm5_value);
  NACL_DISALLOW_COPY_AND_ASSIGN(ShiftTypeBits5To6Interface);
};

// Interface class to pull out the condition in bits 28 through 31
class ConditionBits28To31Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(31, 28);
  }
  static bool defined(const Instruction& i) {
    return value(i) != 0xF;
  }
  static bool undefined(const Instruction& i)  {
    return !defined(i);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(ConditionBits28To31Interface);
};

// Interface class to pull out a Register from bits 0 through 3.
class RegBits0To3Interface {
 public:
  static uint32_t number(const Instruction& i) {
    return i.Bits(3, 0);
  }
  static Register reg(const Instruction& i) {
    return Register(number(i));
  }
  static bool IsEven(const Instruction& i) {
    return (number(i) & 0x1) == 0;
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(RegBits0To3Interface);
};

// Interface class to pull out a register from bits 8 through 11.
class RegBits8To11Interface {
 public:
  static uint32_t number(const Instruction& i) {
    return i.Bits(11, 8);
  }
  static Register reg(const Instruction& i) {
    return Register(number(i));
  }
  static bool IsEven(const Instruction& i) {
    return (number(i) & 0x1) == 0;
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(RegBits8To11Interface);
};

// Interface class to pull out a register from bits 12 through 15.
class RegBits12To15Interface {
 public:
  static uint32_t number(const Instruction& i) {
    return i.Bits(15, 12);
  }
  static Register reg(const Instruction& i) {
    return Register(number(i));
  }
  static bool IsEven(const Instruction& i) {
    return (number(i) & 0x1) == 0;
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(RegBits12To15Interface);
};

// Interface class to pull out a register from bits 16 through 19.
class RegBits16To19Interface {
 public:
  static uint32_t number(const Instruction& i) {
    return i.Bits(19, 16);
  }
  static Register reg(const Instruction& i) {
    return Register(number(i));
  }
  static bool IsEven(const Instruction& i) {
    return (number(i) & 0x1) == 0;
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(RegBits16To19Interface);
};

// Interface to pull out a register bits 0 through 3, and add 1 to it.
class RegBits0To3Plus1Interface {
 public:
  static uint32_t number(const Instruction& i) {
    return i.Bits(3, 0) + 1;
  }
  static Register reg(const Instruction& i) {
    return Register(number(i));
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(RegBits0To3Plus1Interface);
};

// Interface to pull out a register from bits 12 through 15, and add 1 to it.
class RegBits12To15Plus1Interface {
 public:
  static uint32_t number(const Instruction& i) {
    return i.Bits(15, 12) + 1;
  }
  static Register reg(const Instruction& i) {
    return Register(number(i));
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(RegBits12To15Plus1Interface);
};

// Interface class to pull out a binary immediate value from bit 22.
class Imm1Bit22Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(22, 22);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm1Bit22Interface);
};

// Interface class to pull out an immediate value in bits 0 through 3.
class Imm4Bits0To3Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(3, 0);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm4Bits0To3Interface);
};

// Interface class to pull out an immediate value in bits 0 through 7.
class Imm8Bits0To7Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(7, 0);
  }
  // Returns if the value is even.
  static bool IsEven(const Instruction& i) {
    return (value(i) & 0x1) == 0;
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm8Bits0To7Interface);
};

// Interface class to pull out an immediate value in bits 0 through 11.
class Imm12Bits0To11Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(11, 0);
  }
  static uint32_t get_modified_immediate(Instruction i);

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm12Bits0To11Interface);
};

// Interface class to pull out an immediate 24 address in bits 0 through 23
class Imm24AddressBits0To23Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(23, 0);
  }
  static int32_t relative_address(const Instruction& i) {
    // Sign extend and shift left 2:
    int32_t offset = (int32_t)(value(i) << 8) >> 6;
    return offset + 8;  // because r15 reads as 8 bytes ahead
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm24AddressBits0To23Interface);
};

// Interface class to pull out value in bits 8 and 9.
class Imm2Bits8To9Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(9, 8);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm2Bits8To9Interface);
};

// Interface class to pull out a Register List in bits 0 through 15
class RegisterListBits0To15Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(15, 0);
  }
  static RegisterList registers(const Instruction& i) {
    return RegisterList(value(i));
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(RegisterListBits0To15Interface);
};

// Interface class to pull out value in bit 5
class Imm1Bit5Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(5, 5);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm1Bit5Interface);
};

// Interface class to pull out value in bit 7
class Imm1Bit7Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(7, 7);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm1Bit7Interface);
};

// Interface class to pull out an immediate value in bits 7 through 11.
class Imm5Bits7To11Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(11, 7);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm5Bits7To11Interface);
};

// Interface class to pull out an immediate value in bits 8 through 11.
class Imm4Bits8To11Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(11, 8);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm4Bits8To11Interface);
};

// Interface class to pull out an immediate value in bits 8 through 19
class Imm12Bits8To19Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(19, 8);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm12Bits8To19Interface);
};

// Interface class to pull out an immediate value in bits 16 through 20.
class Imm5Bits16To20Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(20, 16);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm5Bits16To20Interface);
};

// Interface class to pull out an immediate value in bits 16 through 19.
class Imm4Bits16To19Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(19, 16);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm4Bits16To19Interface);
};

// Interface class to pull out an immediate value in bits 18 through 19.
class Imm2Bits18To19Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(19, 18);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm2Bits18To19Interface);
};

// Interface class to pull out an immediate value in bits 21 through 23.
class Imm3Bits21To23Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(23, 21);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm3Bits21To23Interface);
};

// Interface class to pull out an immediate value in bit 22.
class Imm1Bit21Interface {
 public:
  static uint32_t value(const Instruction& i) {
    return i.Bits(22, 22);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Imm1Bit21Interface);
};

// Interface class to pull out bit 5 on branch instructions,
// to indicate whether the link register is also updated.
class UpdatesLinkRegisterBit5Interface {
 public:
  static bool IsUpdated(const Instruction i) {
    return i.Bit(5);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(UpdatesLinkRegisterBit5Interface);
};

// Interface class to pull out bit 6 as a flag.
class Bit6FlagInterface {
 public:
  static bool IsDefined(const Instruction& i) {
    return i.Bit(6);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Bit6FlagInterface);
};

// Interface class to pull out S (update) bit 20, which
// defines if the condition bits in APSR are updated by the instruction.
class UpdatesConditionsBit20Interface {
 public:
  // Returns true if bit is set that states that the condition bits
  // APSR is updated.
  static bool is_updated(const Instruction i) {
    return i.Bit(20);
  }
  // Returns the conditions register if it is used.
  static Register conds_if_updated(const Instruction i) {
    return is_updated(i) ? kConditions : kRegisterNone;
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(UpdatesConditionsBit20Interface);
};

// Interface class to pull out to arm register bit 20, which
// when true, updates the corresponging core register.
class UpdatesArmRegisterBit20Interface {
 public:
  static bool IsDefined(const Instruction& i) {
    return i.Bit(20);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(UpdatesArmRegisterBit20Interface);
};

// Interface class to pull out W (writes) bit 21, which
// defines if the istruction writes a value into the base address
// register (Rn).
class WritesBit21Interface {
 public:
  static bool IsDefined(const Instruction& i) {
    return i.Bit(21);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(WritesBit21Interface);
};

// Interface class to pull out bit 21.
class FlagBit21Interface {
 public:
  static bool IsDefined(const Instruction& i) {
    return i.Bits(21, 21);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(FlagBit21Interface);
};

// Interface class to pull out R (read SPSR) bit 22. which (for at
// least MRS) defines if one should use the SPSR register instead of
// the APSR/CPSR.
class ReadSpsrBit22Interface {
 public:
  static bool IsDefined(const Instruction& i) {
    return i.Bit(22);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(ReadSpsrBit22Interface);
};

// Interface to pull out U (direction) bit 23, which defines if
// we should add (rather than subtract) the offset to the base address.
class AddOffsetBit23Interface {
 public:
  static bool IsAdd(const Instruction& i) {
    return i.Bit(23);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(AddOffsetBit23Interface);
};

// Interace to pull out P (pre/post) increment bit 24 flag, used
// for indexing.
class PrePostIndexingBit24Interface {
 public:
  static bool IsDefined(const Instruction& i) {
    return i.Bit(24);
  }
  static bool IsPreIndexing(const Instruction& i) {
    return i.Bit(24);
  }
  static bool IsPostIndexing(const Instruction& i) {
    return !i.Bit(24);
  }

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(PrePostIndexingBit24Interface);
};

// A class decoder is designed to decode a set of instructions that
// have the same semantics, in terms of what the validator needs. This
// includes the bit ranges in the instruction that correspond to
// assigned registers.  as well as whether the instruction is safe to
// use within the validator.
//
// The important property of these class decoders is that the
// corresponding DecoderState (defined in decoder.h) will inspect the
// instruction bits and then dispatch the appropriate class decoder.
//
// The virtuals defined in this class are intended to be used solely
// for the purpose of the validator. For example, for virtual "defs",
// the class decoder will look at the bits defining the assigned
// register of the instruction (typically in bits 12 through 15) and
// add that register to the set of registers returned by the "defs"
// virtual.
//
// There is an underlying assumption that class decoders are constant
// and only provide implementation details for the instructions they
// should be applied to. In general, class decoders should not be
// copied or assigned. Hence, only a no-argument constructor should be
// provided.
class ClassDecoder {
 public:
  // Checks how safe this instruction is, in isolation.
  // This will detect any violation in the ARMv7 spec -- undefined encodings,
  // use of registers that are unpredictable -- and the most basic constraints
  // in our SFI model.  Because ClassDecoders are referentially-transparent and
  // cannot touch global state, this will not check things that may vary with
  // ABI version.
  //
  // The most positive result this can return is called MAY_BE_SAFE because it
  // is necessary, but not sufficient: the validator has the final say.
  virtual SafetyLevel safety(Instruction i) const = 0;

  // Gets the set of registers affected when an instruction executes.  This set
  // is complete, and includes
  //  - explicit destination register(s),
  //  - changes to condition flags,
  //  - indexed-addressing writeback,
  //  - changes to r15 by branches,
  //  - implicit register results, like branch-with-link.
  //
  // Note: If you are unsure if an instruction changes condition flags, be
  // conservative and add it to the set of registers returned by this
  // function. Failure to do so may cause a potential break in pattern
  // atomicity, which checks that two instructions run under the same condition.
  //
  // The default implementation returns a ridiculous bitmask that suggests that
  // all possible side effects will occur -- override if this is not
  // appropriate. :-)
  virtual RegisterList defs(Instruction i) const;

  // Gets the set of registers that this instruction defines through immediate
  // indexed addressing writeback -- a subset of the defs() set.
  //
  // This distinction is useful for operations like SP-relative loads, because
  // the maximum displacement that immediate addressing can produce is small.
  //
  // Note that this does not include defs produced by *register* indexed
  // addressing writeback, since they have no useful properties in our model.
  //
  // Stubbed to indicate that no such addressing occurs.
  virtual RegisterList immediate_addressing_defs(Instruction i) const;

  // For instructions that can read or write memory, gets the register used as
  // the base for generating the effective address.
  //
  // It is stubbed to return nonsense.
  virtual Register base_address_register(Instruction i) const;

  // Checks whether the instruction computes its read or write address as
  // base address + immediate.
  //
  // It is stubbed to return false.
  virtual bool offset_is_immediate(Instruction i) const;

  // For indirect branch instructions, returns the register being moved into
  // r15.  Otherwise, reports kRegisterNone.
  //
  // Note that this exclusively describes instructions that write r15 from a
  // register, unmodified.  This means BX, BLX, and MOV without shift.  Not
  // even BIC, which we allow to write to r15, is modeled this way.
  //
  virtual Register branch_target_register(Instruction i) const;

  // Checks whether the instruction is a direct relative branch -- meaning it
  // adds a constant offset to r15.
  virtual bool is_relative_branch(Instruction i) const;

  // For relative branches, gets the offset added to the instruction's
  // virtual address to find the target.  The results are bogus unless
  // is_relative_branch() returns true.
  //
  // Note that this is different than the offset added to r15 at runtime, since
  // r15 reads as 8 bytes ahead.  This function does the math so you don't have
  // to.
  virtual int32_t branch_target_offset(Instruction i) const;

  // Checks whether this instruction is the special bit sequence that marks
  // the start of a literal pool.
  virtual bool is_literal_pool_head(Instruction i) const;

  // Checks that an instruction clears a certain pattern of bits in all its
  // (non-flag) result registers.  The mask should include 1s in the positions
  // that should be cleared.
  virtual bool clears_bits(Instruction i, uint32_t mask) const;

  // Checks that an instruction will set Z if certain bits in r (chosen by 1s in
  // the mask) are clear.
  //
  // Note that the inverse does not hold: the actual instruction i may require
  // *more* bits to be clear to set Z.  This is fine.
  virtual bool sets_Z_if_bits_clear(Instruction i,
                                    Register r,
                                    uint32_t mask) const;

 protected:
  ClassDecoder() {}
  virtual ~ClassDecoder() {}

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(ClassDecoder);
};

//----------------------------------------------------------------
// The following class decoders define common cases, defining a
// concept that simply associates a non MAY_BE_SAFE with the
// instructions it processes. As such, they provide default
// implementation that returns the corresponding safety value, and
// assumes nothing else interesting happens.
//----------------------------------------------------------------

class UnsafeClassDecoder : public ClassDecoder {
 public:
  explicit UnsafeClassDecoder(SafetyLevel safety)
      : safety_(safety) {}

  // Return the safety associated with this class.
  virtual SafetyLevel safety(Instruction i) const;

  // Switch off the def warnings -- it's already forbidden!
  virtual RegisterList defs(Instruction i) const;

 private:
  SafetyLevel safety_;
  NACL_DISALLOW_COPY_AND_ASSIGN(UnsafeClassDecoder);
};

class Forbidden : public UnsafeClassDecoder {
 public:
  Forbidden() : UnsafeClassDecoder(FORBIDDEN) {}

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Forbidden);
};

// Represents the undefined space in the instruction encoding.
class Undefined : public UnsafeClassDecoder {
 public:
  Undefined() : UnsafeClassDecoder(UNDEFINED) {}

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Undefined);
};

// Represents that the instruction is not implemented.
class NotImplemented : public UnsafeClassDecoder {
 public:
  NotImplemented() : UnsafeClassDecoder(NOT_IMPLEMENTED) {}

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(NotImplemented);
};

// Represents instructions that have been deprecated in ARMv7.
class Deprecated : public UnsafeClassDecoder {
 public:
  Deprecated() : UnsafeClassDecoder(DEPRECATED) {}

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Deprecated);
};

// Represents an unpredictable encoding.  Note that many instructions may
// *become* unpredictable based on their operands -- this is used only for
// the case where a large space of the instruction set is unpredictable.
class Unpredictable : public UnsafeClassDecoder {
 public:
  Unpredictable() : UnsafeClassDecoder(UNPREDICTABLE) {}

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(Unpredictable);
};

// Defines the base class for all coprocessor instructions. We only
// only allow coprocessors 101x, which defined VFP operations.
// +----------------------------------------+--------+----------------+
// |3130292827262524232221201918171615141312|1110 9 8| 7 6 5 4 3 2 1 0|
// +----------------------------------------+--------+----------------+
// |                                        | coproc |                |
// +----------------------------------------+--------+----------------+
// Also doesn't permit updates to PC.

class CoprocessorOp : public ClassDecoder {
 public:
  // Accessor to non-vector register fields.
  static const Imm4Bits8To11Interface coproc;

  CoprocessorOp() {}

  virtual SafetyLevel safety(Instruction i) const;
  // Default assumes defs={}
  virtual RegisterList defs(Instruction i) const;

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(CoprocessorOp);
};

}  // namespace

#endif  // NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_ARM_INST_CLASSES_H_
