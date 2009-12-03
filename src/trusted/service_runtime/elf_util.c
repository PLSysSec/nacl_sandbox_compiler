/*
 * Copyright 2009 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl helper functions to deal with elf images
 */

#include "native_client/src/include/portability.h"

#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "native_client/src/include/elf_constants.h"
#include "native_client/src/include/nacl_elf.h"
#include "native_client/src/include/nacl_macros.h"

#include "native_client/src/shared/platform/nacl_log.h"

#include "native_client/src/trusted/service_runtime/elf_util.h"
#include "native_client/src/trusted/service_runtime/nacl_config.h"

/* private */
struct NaClElfImage {
  Elf32_Ehdr ehdr;
  Elf32_Phdr phdrs[NACL_MAX_PROGRAM_HEADERS];
  int loadable[NACL_MAX_PROGRAM_HEADERS];
};


enum NaClPhdrCheckAction {
  PCA_NONE,
  PCA_TEXT_CHECK,
  PCA_IGNORE  /* ignore this segment.  currently used only for PT_PHDR. */
};


struct NaClPhdrChecks {
  Elf32_Word                p_type;
  Elf32_Word                p_flags;  /* rwx */
  enum NaClPhdrCheckAction  action;
  int                       required;  /* only for text for now */
  Elf32_Word                p_vaddr;  /* if non-zero, vaddr must be this */
};

/*
 * Other than empty segments, these are the only ones that are allowed.
 */
static const struct NaClPhdrChecks nacl_phdr_check_data[] = {
  /* phdr */
  { PT_PHDR, PF_R, PCA_IGNORE, 0, 0, },
  /* text */
  { PT_LOAD, PF_R|PF_X, PCA_TEXT_CHECK, 1, NACL_TRAMPOLINE_END, },
  /* rodata */
  { PT_LOAD, PF_R, PCA_NONE, 0, 0, },
  /* data/bss */
  { PT_LOAD, PF_R|PF_W, PCA_NONE, 0, 0, },
#if NACL_ARCH(NACL_BUILD_ARCH) == NACL_arm
  /* arm exception handling unwind info (for c++)*/
  /* TODO(robertm): for some reason this does NOT end up in ro maybe because
   *             it is relocatable. Try hacking the linker script to move it.
   */
  { PT_ARM_EXIDX, PF_R, PCA_IGNORE, 0, 0, },
#endif
  /*
   * allow optional GNU stack permission marker, but require that the
   * stack is non-executable.
   */
  { PT_GNU_STACK, PF_R|PF_W, PCA_NONE, 0, 0, },
};



static void NaClDumpElfHeader(Elf32_Ehdr *elf_hdr) {
#define DUMP(m,f)    do { NaClLog(2,                            \
                                  #m " = %" f "\n",             \
                                  elf_hdr->m); } while (0)
  DUMP(e_ident+1, ".3s");
  DUMP(e_type, "#x");
  DUMP(e_machine, "#x");
  DUMP(e_version, "#x");
  DUMP(e_entry, "#x");
  DUMP(e_phoff, "#x");
  DUMP(e_shoff, "#x");
  DUMP(e_flags, "#x");
  DUMP(e_ehsize, "#x");
  DUMP(e_phentsize, "#x");
  DUMP(e_phnum, "#x");
  DUMP(e_shentsize, "#x");
  DUMP(e_shnum, "#x");
  DUMP(e_shstrndx, "#x");
#undef DUMP
 NaClLog(2, "sizeof(Elf32_Ehdr) = %x\n", (int) sizeof *elf_hdr);
}


static void NaClDumpElfProgramHeader(Elf32_Phdr *phdr) {
 #define DUMP(mem) do {                                         \
     NaClLog(2, "%s: %x\n", #mem, phdr->mem);      \
   } while (0)

   DUMP(p_type);
   DUMP(p_offset);
   DUMP(p_vaddr);
   DUMP(p_paddr);
   DUMP(p_filesz);
   DUMP(p_memsz);
   DUMP(p_flags);
   NaClLog(2, " (%s %s %s)\n",
           (phdr->p_flags & PF_R) ? "PF_R" : "",
           (phdr->p_flags & PF_W) ? "PF_W" : "",
           (phdr->p_flags & PF_X) ? "PF_X" : "");
   DUMP(p_align);
 #undef  DUMP
   NaClLog(2, "\n");
 }


NaClErrorCode NaClElfImageValidateAbi(struct NaClElfImage *image) {
  const Elf32_Ehdr *hdr = &image->ehdr;

  if (ELFOSABI_NACL != hdr->e_ident[EI_OSABI]) {
    NaClLog(LOG_ERROR, "Expected OSABI %d, got %d\n",
            ELFOSABI_NACL,
            hdr->e_ident[EI_OSABI]);
    return LOAD_BAD_ABI;
  }

  if (EF_NACL_ABIVERSION != hdr->e_ident[EI_ABIVERSION]) {
    NaClLog(LOG_ERROR, "Expected ABIVERSION %d, got %d\n",
            EF_NACL_ABIVERSION,
            hdr->e_ident[EI_ABIVERSION]);
      return LOAD_BAD_ABI;
  }

  return LOAD_OK;
}


NaClErrorCode NaClElfImageValidateElfHeader(struct NaClElfImage *image) {
  const Elf32_Ehdr *hdr = &image->ehdr;

  if (memcmp(hdr->e_ident, ELFMAG, SELFMAG)) {
    NaClLog(LOG_ERROR, "bad elf magic\n");
    return LOAD_BAD_ELF_MAGIC;
  }

  if (ELFCLASS32 != hdr->e_ident[EI_CLASS]) {
    NaClLog(LOG_ERROR, "bad elf class\n");
    return LOAD_NOT_32_BIT;
  }

  if (ET_EXEC != hdr->e_type) {
    NaClLog(LOG_ERROR, "non executable\n");
    return LOAD_NOT_EXEC;
  }

  if (EM_EXPECTED_BY_NACL != hdr->e_machine) {
    NaClLog(LOG_ERROR, "bad machine\n");
    return LOAD_BAD_MACHINE;
  }

  if (EV_CURRENT != hdr->e_version) {
    NaClLog(LOG_ERROR, "bad elf version\n");
    return LOAD_BAD_ELF_VERS;
  }

  return LOAD_OK;
}

/* TODO(robertm): decouple validation from computation of
                   static_text_end and max_vaddr */
NaClErrorCode NaClElfImageValidateProgramHeaders(
  struct NaClElfImage *image,
  uint32_t            addr_bits,
  uint32_t            *static_text_end,
  uintptr_t           *max_vaddr) {
    /*
     * Scan phdrs and do sanity checks in-line.  Verify that the load
     * address is NACL_TRAMPOLINE_END, that we have a single text
     * segment.  Data and TLS segments are not required, though it is
     * hard to avoid with standard tools, but in any case there should
     * be at most one each.  Ensure that no segment's vaddr is outside
     * of the address space.  Ensure that PT_GNU_STACK is present, and
     * that x is off.
     */
  const Elf32_Ehdr *hdr = &image->ehdr;
  int         seen_seg[NACL_ARRAY_SIZE(nacl_phdr_check_data)];

  int         segnum;
  const Elf32_Phdr  *php;
  size_t      j;

  *max_vaddr = NACL_TRAMPOLINE_END;

  /*
   * nacl_phdr_check_data is small, so O(|check_data| * nap->elf_hdr.e_phum)
   * is okay.
   */
  memset(seen_seg, 0, sizeof seen_seg);
  for (segnum = 0; segnum < hdr->e_phnum; ++segnum) {
    php = &image->phdrs[segnum];
    NaClLog(3, "Looking at segment %d, type 0x%x, p_flags 0x%x\n",
            segnum, php->p_type, php->p_flags);
    for (j = 0; j < NACL_ARRAY_SIZE(nacl_phdr_check_data); ++j) {
      if (php->p_type == nacl_phdr_check_data[j].p_type
          && php->p_flags == nacl_phdr_check_data[j].p_flags) {
        NaClLog(2, "Matched nacl_phdr_check_data[%"PRIdS"]\n", j);
        if (seen_seg[j] > 0) {
          NaClLog(2, "Segment %d is a type that has been seen\n", segnum);
          return LOAD_DUP_SEGMENT;
        }
        ++seen_seg[j];

        if (PCA_IGNORE == nacl_phdr_check_data[j].action) {
          NaClLog(3, "Ignoring\n");
          goto next_seg;
        }

        if (0 != php->p_memsz) {
          /*
           * We will load this segment later.  Do the sanity checks.
           */
          if (0 != nacl_phdr_check_data[j].p_vaddr
              && (nacl_phdr_check_data[j].p_vaddr != php->p_vaddr)) {
            NaClLog(2,
                    ("Segment %d: bad virtual address: 0x%08x,"
                     " expected 0x%08x\n"),
                    segnum,
                    php->p_vaddr,
                    nacl_phdr_check_data[j].p_vaddr);
            return LOAD_SEGMENT_BAD_LOC;
          }
          if (php->p_vaddr < NACL_TRAMPOLINE_END) {
            NaClLog(2, "Segment %d: virtual address (0x%08x) too low\n",
                    segnum,
                    php->p_vaddr);
            return LOAD_SEGMENT_OUTSIDE_ADDRSPACE;
          }
          /*
           * integer overflow?  Elf32_Addr and Elf32_Word are uint32_t,
           * so the addition/comparison is well defined.
           */
          if (php->p_vaddr + php->p_memsz < php->p_vaddr) {
            NaClLog(2,
                    "Segment %d: p_memsz caused integer overflow\n",
                    segnum);
            return LOAD_SEGMENT_OUTSIDE_ADDRSPACE;
          }
          if (php->p_vaddr + php->p_memsz >= (1U << addr_bits)) {
            NaClLog(2,
                    "Segment %d: too large, ends at 0x%08x\n",
                    segnum,
                    php->p_vaddr + php->p_memsz);
            return LOAD_SEGMENT_OUTSIDE_ADDRSPACE;
          }
          if (php->p_filesz > php->p_memsz) {
            NaClLog(2,
                    ("Segment %d: file size 0x%08x larger"
                     " than memory size 0x%08x\n"),
                    segnum,
                    php->p_filesz,
                    php->p_memsz);
            return LOAD_SEGMENT_BAD_PARAM;
          }

          image->loadable[segnum] = 1;
          /* record our decision that we will load this segment */

          /*
           * NACL_TRAMPOLINE_END <= p_vaddr
           *                     <= p_vaddr + p_memsz
           *                     < (1U << nap->addr_bits)
           */
          if (*max_vaddr < php->p_vaddr + php->p_memsz) {
            *max_vaddr = php->p_vaddr + php->p_memsz;
          }
        }

        switch (nacl_phdr_check_data[j].action) {
          case PCA_NONE:
            break;
          case PCA_TEXT_CHECK:
            if (0 == php->p_memsz) {
              return LOAD_BAD_ELF_TEXT;
            }
            *static_text_end = NACL_TRAMPOLINE_END + php->p_filesz;
            break;
          case PCA_IGNORE:
            break;
        }
        goto next_seg;
      }
    }
    /* segment not in nacl_phdr_check_data */
    if (0 == php->p_memsz) {
      NaClLog(3, "Segment %d zero size: ignored\n", segnum);
      continue;
    }
    NaClLog(2,
            "Segment %d is of unexpected type 0x%x, flag 0x%x\n",
            segnum,
            php->p_type,
            php->p_flags);
    return LOAD_BAD_SEGMENT;
 next_seg:
    {}
  }
  for (j = 0; j < NACL_ARRAY_SIZE(nacl_phdr_check_data); ++j) {
    if (nacl_phdr_check_data[j].required && !seen_seg[j]) {
      return LOAD_REQUIRED_SEG_MISSING;
    }
  }

  /*
   * Memory allocation will use NaClRoundPage(nap->break_addr), but
   * the system notion of break is always an exact address.  Even
   * though we must allocate and make accessible multiples of pages,
   * the linux-style brk system call (which returns current break on
   * failure) permits an arbitrarily aligned address as argument.
   */

  return LOAD_OK;
}


struct NaClElfImage *NaClElfImageNew(struct Gio  *gp) {
  struct NaClElfImage *result;
  struct NaClElfImage image;
  int                 cur_ph;

  memset(image.loadable, 0, sizeof image.loadable);
  if ((*gp->vtbl->Read)(gp,
                        &image.ehdr,
                        sizeof image.ehdr)
      != sizeof image.ehdr) {
    /* Consider making this fatal */
    NaClLog(2, "could not load elf headers\n");
    return 0;
  }

  NaClDumpElfHeader(&image.ehdr);

  /* read program headers */
  if (image.ehdr.e_phnum > NACL_MAX_PROGRAM_HEADERS) {
    /* Consider making this fatal */
    NaClLog(2, "too many prog headers\n");
    return 0;
  }

  if (image.ehdr.e_phentsize < sizeof image.phdrs[0]) {
    NaClLog(2, "bad prog headers size\n");
    return 0;
  }

  if ((*gp->vtbl->Seek)(gp,
                        image.ehdr.e_phoff,
                        SEEK_SET) == (size_t) -1) {
    NaClLog(2, "cannot seek tp prog headers\n");
    return 0;
  }

  if ((*gp->vtbl->Read)(gp,
                        &image.phdrs[0],
                        image.ehdr.e_phnum * sizeof image.phdrs[0])
      != (image.ehdr.e_phnum * sizeof image.phdrs[0])) {
    NaClLog(2, "cannot load tp prog headers\n");
    return 0;
  }

  for (cur_ph = 0; cur_ph <  image.ehdr.e_phnum; ++cur_ph) {
    NaClDumpElfProgramHeader(&image.phdrs[cur_ph]);
  }

  /* we delay allocating till the end to avoid cleanup code */
  result = malloc(sizeof image);
  if (result == 0) {
    NaClLog(LOG_FATAL, "no enough memory for image meta data\n");
    return 0;
  }
  memcpy(result, &image, sizeof image);
  return result;
}


NaClErrorCode NaClElfImageLoad(struct NaClElfImage *image,
                               struct Gio          *gp,
                               uint32_t            addr_bits,
                               uintptr_t           mem_start) {
  int               segnum;
  uintptr_t         paddr;
  uintptr_t         end_vaddr;

  for (segnum = 0; segnum < image->ehdr.e_phnum; ++segnum) {
    const Elf32_Phdr *php = &image->phdrs[segnum];

    /* did we decide that we will load this segment earlier? */
    if (!image->loadable[segnum]) {
      continue;
    }

    NaClLog(2, "loading segment %d", segnum);
    end_vaddr = php->p_vaddr + php->p_filesz;
    /* integer overflow? */
    if (end_vaddr < php->p_vaddr) {
      NaClLog(LOG_FATAL, "parameter error should have been detected already\n");
    }
    /*
     * is the end virtual address within the NaCl application's
     * address space?  if it is, it implies that the start virtual
     * address is also.
     */
    if (end_vaddr >= (1U << addr_bits)) {
      NaClLog(LOG_FATAL, "parameter error should have been detected already\n");
    }

    paddr = mem_start + php->p_vaddr;

    if ((*gp->vtbl->Seek)(gp, php->p_offset, SEEK_SET) == (size_t) -1) {
      NaClLog(LOG_ERROR, "seek failure segment %d", segnum);
      return LOAD_SEGMENT_BAD_PARAM;
    }
    if ((Elf32_Word) (*gp->vtbl->Read)(gp, (void *) paddr, php->p_filesz)
        != php->p_filesz) {
      NaClLog(LOG_ERROR, "load failure segment %d", segnum);
      return LOAD_SEGMENT_BAD_PARAM;
    }
    /* region from p_filesz to p_memsz should already be zero filled */
  }

  return LOAD_OK;
}


void NaClElfImageDelete(struct NaClElfImage *image) {
  free(image);
}


uint32_t NaClElfImageGetEntryPoint(struct NaClElfImage *image) {
  return image->ehdr.e_entry;
}


/* TODO(robertm): this code should enforce that either 16 or 32 bit alignment is
                  is set - there are currently some problems with ARM, though
*/
int NaClElfImageGetBundleSize(struct NaClElfImage *image) {
  unsigned long eflags = image->ehdr.e_flags & EF_NACL_ALIGN_MASK;
  if (eflags) {
    if (eflags == EF_NACL_ALIGN_16) {
     return 16;
    } else if (eflags == EF_NACL_ALIGN_32) {
      return 32;
    } else {
      NaClLog(LOG_ERROR, "strange alignment");
      return 0;
    }
  } else {
    return 32;
  }
}
