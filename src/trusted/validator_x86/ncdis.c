/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * ncdis.c - disassemble using NaCl decoder.
 * Mostly for testing.
 */


#ifndef NACL_TRUSTED_BUT_NOT_TCB
#error("This file is not meant for use in the TCB")
#endif

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "native_client/src/shared/gio/gio.h"
#include "native_client/src/shared/utils/types.h"
#include "native_client/src/shared/utils/flags.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/trusted/validator/ncfileutil.h"
#include "native_client/src/trusted/validator/x86/ncval_seg_sfi/ncdecode_verbose.h"
#include "native_client/src/trusted/validator/x86/ncval_seg_sfi/ncvalidate_internaltypes.h"
#include "native_client/src/trusted/validator_x86/nc_inst_state.h"
#include "native_client/src/trusted/validator_x86/ncopcode_desc.h"
#include "native_client/src/trusted/validator_x86/nc_read_segment.h"
#include "native_client/src/trusted/validator_x86/ncdis_segments.h"

/* The name of the executable that is being run. */
static const char* exec_name = "???";

static void Fatal(const char *fmt, ...) {
  FILE* fp = stdout;
  va_list ap;
  fprintf(fp, "Fatal: ");
  va_start(ap, fmt);
  vfprintf(fp, fmt, ap);
  va_end(ap);
  exit(-1);
}

void Info(const char *fmt, ...) {
  FILE* fp = stdout;
  va_list ap;
  fprintf(fp, "Info: ");
  va_start(ap, fmt);
  vfprintf(fp, fmt, ap);
  va_end(ap);
}

static void usage() {
  fprintf(stderr,
          "usage: ncdis [options] [file]\n"
          "\n"
          "Options are:\n"
          "--commands=<file>\n"
          "\tAdditional command line arguments are specified in the given\n"
          "\tfile ('#' acts as a comment character). Use '-' as its value to\n"
          "\tredirect command line arguments from standard input.\n"
          "--help\n"
          "\tPrint out this usage message\n"
          "--hex_text=<file>\n"
          "\tDefine code section as sequence of (textual) hexidecimal bytes\n"
          "\tdefined in the given file. Lines beginning with '#' will be\n"
          "\treated as comments. If the first non-comment line begins with\n"
          "\t'@' the following hexidecimal number will be used as the\n"
          "\tbeginning (RIP/EIP) instruction address of the code segment.\n"
          "\tUse '-' as its value to redirect standard input as the\n"
          "\ttext file to process.\n"
          "-i=XXXX\n"
          "\tXXXX specifies the sequence of hexidecimal digits that define\n"
          "\tan instruction to be decoded.\n"
          "--internal\n"
          "\tFor the iterator model (only), prints out each the decoded\n"
          "\tinstruction, followed by the internals for the matched\n"
          "\tinstruction.\n"
          "--pc=XXX\n"
          "\tSet program counter (i.e. RIP or EIP) to XXX.\n"
          "--self_document\n"
          "\tProcess input hext_text file in such a way, that it also\n"
          "\trepresents the output that will be generated by ncdis.\n"
          "\tThat is, copy comment lines (i.e. lines beginning with\n"
          "\t'#') to stdout. In addition, it assumes that each line\n"
          "\tconsists of an '-i' command line argument (and possibly\n"
          "\ta '--pc' command line argument, followed by a '#',\n"
          "\tfollowed by the corresponding disassembled text. On such\n"
          "\tlines, the input is copied up to (and including) the '#'.,\n"
          "\tand then the disassembled instruction is printed.\n"
          "--use_iter\n"
          "\tThis executable defines two different implementations of the\n"
          "\tinstruction disasssembler. The iterator model\n"
          "\tdisassembles code for the x86-64 validator, while the\n"
          "\tnon-interator model disassembles code the the\n"
          "\tx86-32 validator. Future releases will hopefully remove\n"
          "\tthe need for this flag\n"
          );
  exit(1);
}

static int AnalyzeSections(ncfile *ncf) {
  int badsections = 0;
  int ii;
  const Elf_Shdr* shdr = ncf->sheaders;

  for (ii = 0; ii < ncf->shnum; ii++) {
    Info("section %d sh_addr %x offset %x flags %x\n",
         ii, (uint32_t)shdr[ii].sh_addr,
         (uint32_t)shdr[ii].sh_offset, (uint32_t)shdr[ii].sh_flags);
    if ((shdr[ii].sh_flags & SHF_EXECINSTR) != SHF_EXECINSTR)
      continue;
    Info("parsing section %d\n", ii);
    NaClDisassembleSegment(ncf->data + (shdr[ii].sh_addr - ncf->vbase),
                           shdr[ii].sh_addr, shdr[ii].sh_size);
  }
  return -badsections;
}

static void AnalyzeCodeSegments(ncfile *ncf, const char *fname) {
  if (AnalyzeSections(ncf) < 0) {
    fprintf(stderr, "%s: text validate failed\n", fname);
  }
}

/* Capture a sequence of bytes defining an instruction (up to a
 * MAX_BYTES_PER_X86_INSTRUCTION). This sequence is used to run
 * a (debug) test of the disassembler.
 */
static uint8_t FLAGS_decode_instruction[NACL_MAX_BYTES_PER_X86_INSTRUCTION];

/* Define the number of bytes supplied for a debug instruction. */
static int FLAGS_decode_instruction_size = 0;

/* Flag defining the value of the pc to use when decoding an instruction
 * through decode_instruction.
 */
static uint32_t FLAGS_decode_pc = 0;

/* Flag defining an input file to use as command line arguments
 * (one per input line). When specified, run the disassembler
 * on each command line. The empty string "" denotes that no command
 * line file was specified. A dash ("-") denotes that standard input
 * should be used to get command line arguments.
 */
static char* FLAGS_commands = "";

/* Flag defining the name of a hex text to be used as the code segment. Assumes
 * that the pc associated with the code segment is defined by
 * FLAGS_decode_pc.
 */
static char* FLAGS_hex_text = "";

/* Flag, when used in combination with the commands flag, will turn
 * on input copy rules, making the genrated output contain comments
 * and the command line arguments as part of the corresponding
 * generated output. For more details on this, see ProcessInputFile
 * below.
 */
static Bool FLAGS_self_document = FALSE;

/*
 * Store default values of flags on the first call. On subsequent
 * calls, resets the flags to the default value.
 *
 * *WARNING* In order for this to work, this function must be
 * called before GrokFlags
 *
 * NOTE: we only allow the specification of -use_iter at the top-level
 * command line..
 */
static void ResetFlags() {
  int i;
  static uint32_t DEFAULT_decode_pc;
  static char* DEFAULT_commands;
  static Bool DEFAULT_self_document;
  static Bool is_first_call = TRUE;
  if (is_first_call) {
    DEFAULT_decode_pc = FLAGS_decode_pc;
    DEFAULT_commands = FLAGS_commands;
    DEFAULT_self_document = FLAGS_self_document;
    is_first_call = FALSE;
  }

  FLAGS_decode_pc = DEFAULT_decode_pc;
  FLAGS_commands = DEFAULT_commands;
  FLAGS_self_document = DEFAULT_self_document;
  /* Always clear the decode instruction. */
  FLAGS_decode_instruction_size = 0;
  for (i = 0; i < NACL_MAX_BYTES_PER_X86_INSTRUCTION; ++i) {
    FLAGS_decode_instruction[i] = 0;
  }
}

/* Returns true if all characters in the string are zero. */
static Bool IsZero(const char* arg) {
  while (*arg) {
    if ('0' != *arg) {
      return FALSE;
    }
    ++arg;
  }
  return TRUE;
}

uint8_t HexToByte(const char* hex_value) {
  unsigned long value = strtoul(hex_value, NULL, 16);
  /* Verify that arg is all zeros when zero is returned. Otherwise,
   * assume that the zero value was due to an error.
   */
  if (0L == value && !IsZero(hex_value)) {
    Fatal("-i option specifies illegal hex value '%s'\n", hex_value);
  }
  return (uint8_t) value;
}

/* Recognizes flags in argv, processes them, and then removes them.
 * Returns the updated value for argc.
 */
int GrokFlags(int argc, const char *argv[]) {
  int i;
  int new_argc;
  char* hex_instruction;
  Bool help = FALSE;
  if (argc == 0) return 0;
  exec_name = argv[0];
  new_argc = 1;
  for (i = 1; i < argc; ++i) {
    const char* arg = argv[i];
    if (GrokUint32HexFlag("--pc", arg, &FLAGS_decode_pc) ||
        GrokCstringFlag("--commands", arg, &FLAGS_commands) ||
        GrokCstringFlag("--hex_text", arg, &FLAGS_hex_text) ||
        GrokBoolFlag("--self_document", arg, &FLAGS_self_document) ||
        GrokBoolFlag("--use_iter", arg, &NACL_FLAGS_use_iter) ||
        GrokBoolFlag("--internal", arg, &NACL_FLAGS_internal) ||
        GrokBoolFlag("--help", arg, &help)) {
      if (help) usage();
      continue;
    }
    if (GrokCstringFlag("-i", arg, &hex_instruction)) {
      int i = 0;
      char buffer[3];
      char* buf = &(hex_instruction[0]);
      buffer[2] = '\0';
      while (*buf) {
        buffer[i++] = *(buf++);
        if (i == 2) {
          uint8_t byte = HexToByte(buffer);
          FLAGS_decode_instruction[FLAGS_decode_instruction_size++] = byte;
          if (FLAGS_decode_instruction_size >
              NACL_MAX_BYTES_PER_X86_INSTRUCTION) {
            Fatal("-i=%s specifies too long of a hex value\n", hex_instruction);
          }
          i = 0;
        }
      }
      if (i != 0) {
        Fatal("-i=%s doesn't specify a sequence of bytes\n", hex_instruction);
      }
    } else {
      argv[new_argc++] = argv[i];
    }
  }
  return new_argc;
}

/* Process the command line arguments. */
static const char* GrokArgv(int argc, const char* argv[]) {
  if (NACL_FLAGS_internal && ! NACL_FLAGS_use_iter) {
    Fatal("Can't specify -internal without -use_iter");
  }
  if (argc != 2) {
    Fatal("no filename specified\n");
  }
  return argv[argc-1];
}

static void ProcessCommandLine(int argc, const char* argv[]);

/* Defines the maximum number of characters allowed on an input line
 * of the input text defined by the commands command line option.
 */
#define MAX_INPUT_LINE 4096

/* Defines the characters used as (token) separators to recognize command
 * line arguments when processing lines of text in the text file specified
 * by the commands command line option.
 */
#define CL_SEPARATORS " \t\n"

/* Copies the text from the input line (which should be command line options),
 * up to any trailing comments (i.e. the pound sign).
 *   input_line - The line of text to process.
 *   tokens - The extracted text from the input_line.
 *   max_length - The maximum length of input_line and tokens.
 *
 * Note: If input_line doesn't end with a null terminator, one is automatically
 * inserted.
 */
static void CopyCommandLineTokens(char* input_line,
                                  char* token_text,
                                  size_t max_length) {
  size_t i;
  for (i = 0; i < max_length; ++i) {
    char ch;
    if (max_length == i + 1) {
      /* Be sure we end the string with a null terminator. */
      input_line[i] = '\0';
    }
    ch = input_line[i];
    token_text[i] = ch;
    if (ch == '\0') return;
    if (ch == '#') {
      token_text[i] = '\0';
      return;
    }
  }
}

/* Tokenize the given text to find command line arguments, and
 * add them to the given list of command line arguments.
 *
 * *WARNING* This function will (destructively) modify the
 * contents of token_text, by converting command line option
 * separator characters into newlines.
 */
static void ExtractTokensAndAddToArgv(
    char* token_text,
    int* argc,
    const char* argv[]) {
  /* Note: Assume that each command line argument corresponds to
   * non-blank text, which is a HACK, but should be sufficient for
   * what we need.
   */
  char* token = strtok(token_text, CL_SEPARATORS);
  while (token != NULL) {
    argv[(*argc)++] = token;
    token = strtok(NULL, CL_SEPARATORS);
  }
}

/* Print out the contents of text, up to the first occurence of the
 * pound sign.
 */
static void PrintUpToPound(const char text[]) {
  int i;
  struct Gio* g = NaClLogGetGio();
  for (i = 0; i < MAX_INPUT_LINE; ++i) {
    char ch = text[i];
    switch (ch) {
      case '#':
        gprintf(g, "%c", ch);
        return;
      case '\0':
        return;
      default:
        gprintf(g, "%c", ch);
        break;
    }
  }
}

/* Reads the given text file and processes the command line options specified
 * inside of it. Each line specifies a separate sequence of command line
 * arguments to process.
 *
 * Note:
 * (a) The '#' is used as a comment delimiter.
 * (b) whitespace lines are ignored.
 * (c) If flag --self_document is specified, comment lines and whitespace
 *     lines will automatically be copied to stdout. In addition, command
 *     line arguments will be copied to stdout before processing them.
 *     Further, if the command line arguments are followed by a comment,
 *     only text up to (and including) the '#' will be copied. This allows
 *     the input file to contain the (hopefully single lined) output that
 *     would be generated by the given command line arguments. Therefore,
 *     if set up correctly, the output of the disassembler (in this case)
 *     should be the same as the input file (making it easy to use the
 *     input file as the the corresponding GOLD file to test against).
 */
static void ProcessInputFile(FILE* file) {
  char input_line[MAX_INPUT_LINE];
  const Bool self_document = FLAGS_self_document;
  while (fgets(input_line, MAX_INPUT_LINE, file) != NULL) {
    char token_text[MAX_INPUT_LINE];
    const char* line_argv[MAX_INPUT_LINE];
    int line_argc = 0;

    /* Copy the input line (up to the first #) into token_text */
    CopyCommandLineTokens(input_line, token_text, MAX_INPUT_LINE);

    /* Tokenize the commands to build argv.
     * Note: Since each token is separated by a blank,
     * and the input is no more than MAX_INPUT_LINE,
     * we know (without checking) that line_argc
     * will not exceed MAX_INPUT_LINE.
     */
    line_argv[line_argc++] = exec_name;
    ExtractTokensAndAddToArgv(token_text, &line_argc, line_argv);

    /* Process the parsed input line. */
    if (1 == line_argc) {
      /* No command line arguments. */
      if (self_document) {
        printf("%s", input_line);
      }
    } else {
      /* Process the tokenized command line. */
      if (self_document) {
        PrintUpToPound(input_line);
      }
      ProcessCommandLine(line_argc, line_argv);
    }
  }
  ResetFlags();
}

/* Run the disassembler using the given command line arguments. */
static void ProcessCommandLine(int argc, const char* argv[]) {
  int new_argc;

  ResetFlags();
  new_argc = GrokFlags(argc, argv);
  if (FLAGS_decode_instruction_size > 0) {
    /* Command line options specify an instruction to decode, run
     * the disassembler on the instruction to print out the decoded
     * results.
     */
    if (new_argc > 1) {
      Fatal("unrecognized option '%s'\n", argv[1]);
    } else if (0 != strcmp(FLAGS_commands, "")) {
      Fatal("can't specify -commands and -b options simultaneously\n");
    }
    NaClDisassembleSegment(FLAGS_decode_instruction, FLAGS_decode_pc,
                           FLAGS_decode_instruction_size);
  } else if (0 != strcmp(FLAGS_hex_text, "")) {
    uint8_t bytes[MAX_INPUT_LINE];
    size_t num_bytes;
    NaClPcAddress pc;
    if (0 == strcmp(FLAGS_hex_text, "-")) {
      num_bytes = NaClReadHexTextWithPc(stdin, &pc, bytes, MAX_INPUT_LINE);
      NaClDisassembleSegment(bytes, pc, (NaClMemorySize) num_bytes);
    } else {
      FILE* input = fopen(FLAGS_hex_text, "r");
      if (NULL == input) {
        Fatal("Can't open hex text file: %s\n", FLAGS_hex_text);
      }
      num_bytes = NaClReadHexTextWithPc(input, &pc, bytes, MAX_INPUT_LINE);
      fclose(input);
      NaClDisassembleSegment(bytes, pc, (NaClMemorySize) num_bytes);
    }
  } else if (0 != strcmp(FLAGS_commands, "")) {
    /* Use the given input file to find command line arguments,
     * and process.
     */
    if (0 == strcmp(FLAGS_commands, "-")) {
      ProcessInputFile(stdin);
    } else {
      FILE* input = fopen(FLAGS_commands, "r");
      if (NULL == input) {
        Fatal("Can't open commands file: %s\n", FLAGS_commands);
      }
      ProcessInputFile(input);
      fclose(input);
    }
  } else {
    /* Command line should specify an executable to disassemble.
     * Read the file and disassemble it.
     */
    ncfile *ncf;
    const char* filename = GrokArgv(new_argc, argv);

    Info("processing %s", filename);
    ncf = nc_loadfile_depending(filename, NULL);
    if (ncf == NULL) {
      Fatal("nc_loadfile(%s): %s\n", filename, strerror(errno));
    }

    AnalyzeCodeSegments(ncf, filename);

    nc_freefile(ncf);
  }
}

int main(int argc, const char *argv[]) {
  struct GioFile gout_file;
  struct Gio* gout = (struct Gio*) &gout_file;
  if (!GioFileRefCtor(&gout_file, stdout)) {
    fprintf(stderr, "Unable to create gio file for stdout!\n");
    return 1;
  }
  NaClLogModuleInitExtended(LOG_INFO, gout);
  ProcessCommandLine(argc, argv);
  NaClLogModuleFini();
  GioFileDtor(gout);
  return 0;
}
