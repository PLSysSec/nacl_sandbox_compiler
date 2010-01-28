/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Generic I/O interface.
 */
#include "native_client/src/include/portability.h"

#include "native_client/src/trusted/gio/gio.h"

struct GioVtbl const    kGioFileVtbl = {
  GioFileRead,
  GioFileWrite,
  GioFileSeek,
  GioFileFlush,
  GioFileClose,
  GioFileDtor,
};


int GioFileCtor(struct GioFile  *self,
                char const      *fname,
                char const      *mode) {
  self->base.vtbl = (struct GioVtbl *) NULL;
  self->iop = fopen(fname, mode);
  if (NULL == self->iop) {
    return 0;
  }
  self->base.vtbl = &kGioFileVtbl;
  return 1;
}


int GioFileRefCtor(struct GioFile   *self,
                   FILE             *iop) {
  self->iop = iop;

  self->base.vtbl = &kGioFileVtbl;
  return 1;
}


size_t GioFileRead(struct Gio  *vself,
                   void        *buf,
                   size_t      count) {
  struct GioFile  *self = (struct GioFile *) vself;
  return fread(buf, 1, count, self->iop);
}


size_t GioFileWrite(struct Gio *vself,
                    void       *buf,
                    size_t     count) {
  struct GioFile  *self = (struct GioFile *) vself;
  return fwrite(buf, 1, count, self->iop);
}


size_t GioFileSeek(struct Gio  *vself,
                  off_t       offset,
                  int         whence) {
  struct GioFile  *self = (struct GioFile *) vself;
  int             ret;
  ret = fseek(self->iop, offset, whence);
  if (-1 == ret) return -1;
  return (size_t) ftell(self->iop);
}


int GioFileFlush(struct Gio *vself) {
  struct GioFile  *self = (struct GioFile *) vself;

  return fflush(self->iop);
}


int GioFileClose(struct Gio *vself){
  struct GioFile  *self = (struct GioFile *) vself;
  int             rv;
  rv = (EOF == fclose(self->iop)) ? -1 : 0;
  if (0 == rv) {
    self->iop = (FILE *) 0;
  }
  return rv;
}


void  GioFileDtor(struct Gio  *vself) {
  struct GioFile  *self = (struct GioFile *) vself;
  if (0 != self->iop) {
    (void) fclose(self->iop);
  }
}


int fggetc(struct Gio   *gp) {
  char    ch;

  return (*gp->vtbl->Read)(gp, &ch, 1) == 1 ? ch : EOF;
}
