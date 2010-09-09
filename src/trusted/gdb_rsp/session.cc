/*
 * Copyright 2010 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <sstream>

#include "native_client/src/trusted/gdb_rsp/packet.h"
#include "native_client/src/trusted/gdb_rsp/session.h"
#include "native_client/src/trusted/gdb_rsp/util.h"

#include "native_client/src/trusted/port/mutex.h"
#include "native_client/src/trusted/port/platform.h"
#include "native_client/src/trusted/port/transport.h"

using port::IPlatform;
using port::ITransport;
using port::IMutex;
using port::MutexLock;

// Use a timeout of 1 second
int const kSessionTimeoutMs = 1000;

namespace gdb_rsp {

Session::Session(ITransport *io_ptr)
  : mutex_(NULL),
    io_(io_ptr),
    flags_(0),
    seq_(0) {
  connected_ = (io_ptr != NULL);
  mutex_ = IMutex::Allocate();
  assert(io_);
  assert(mutex_);
}

Session::~Session() {
}

void Session::SetFlags(uint32_t flags) {
  flags_ |= flags;
}

void Session::ClearFlags(uint32_t flags) {
  flags_ &= ~flags;
}

uint32_t Session::GetFlags() {
  return flags_;
}

bool Session::DataAvailable() {
  return io_->ReadWaitWithTimeout(kSessionTimeoutMs);
}

bool Session::Connected() {
  return connected_;
}

Session::DPResult Session::GetChar(char *ch) {
  // Attempt to select this IO for reading.
  if (DataAvailable() == false) return DPR_NO_DATA;

  int32_t len = io_->Read(ch, 1);
  if (len == 0) return DPR_NO_DATA;
  if (len == -1) {
    io_->Disconnect();
    connected_ = false;
    return DPR_ERROR;
  }

  return DPR_OK;
}


Session::DPResult Session::SendPacket(Packet *pkt) {
  MutexLock lock(mutex_);

  Session::DPResult res;
  char ch;


  if (NULL == io_) return DPR_ERROR;

  // If we are ignoring ACKs..
  if (GetFlags() & IGNORE_ACK) return SendPacketOnly(pkt);

  do {
    res = SendPacketOnly(pkt);
    // Verify we sent OK
    if (res != DPR_OK) break;

    // If ACKs are off, we are done.
    if (GetFlags() & IGNORE_ACK) break;

    // Otherwise, poll for '+'
    if (GetChar(&ch) == DPR_ERROR) return DPR_ERROR;

    // Retry if we didn't get a '+'
  } while (ch != '+');

  return res;
}


Session::DPResult Session::SendPacketOnly(Packet *pkt) {
  MutexLock lock(mutex_);

  const char *ptr;
  char ch;
  std::stringstream outstr;

  char run_xsum = 0;
  int32_t seq;

  ptr = pkt->GetPayload();

  if (!pkt->GetSequence(&seq) && (GetFlags() & USE_SEQ)) {
    pkt->SetSequence(seq_++);
  }

  // Signal start of response
  outstr << '$';

  // If there is a sequence, send as two nibble 8bit value + ':'
  if (pkt->GetSequence(&seq)) {
    IntToNibble((seq & 0xFF) >> 4, &ch);
    outstr << ch;
    run_xsum += ch;

    IntToNibble(seq & 0xF, &ch);
    outstr << ch;
    run_xsum += ch;

    ch = ':';
    outstr << ch;
    run_xsum += ch;
  }

  // Send the main payload
  int offs = 0;
  while ((ch = ptr[offs++]) != 0) {
    outstr << ch;
    run_xsum += ch;
  }

  // Send XSUM as two nible 8bit value preceeded by '#'
  outstr << '#';
  IntToNibble((run_xsum >> 4) & 0xF, &ch);
  outstr << ch;
  IntToNibble(run_xsum & 0xF, &ch);
  outstr << ch;

  return SendStream(outstr.str().data());
}

// We do not take the mutex here since we already have it
// this function is protected so it can't be called directly.
Session::DPResult Session::SendStream(const char *out) {
  int32_t len = static_cast<int32_t>(strlen(out));
  int32_t sent = 0;

  while (sent < len) {
    const char *cur = &out[sent];
    int32_t tx = io_->Write(cur, len - sent);

    if (tx <= 0) {
      IPlatform::LogWarning("Send of %d bytes : '%s' failed.\n", len, out);
      io_->Disconnect();
      connected_ = false;
      return DPR_ERROR;
    }

    sent += tx;
  }

  if (GetFlags() & DEBUG_SEND) IPlatform::LogInfo("TX %s\n", out);
  return DPR_OK;
}


// Attempt to receive a packet
Session::DPResult Session::GetPacket(Packet *pkt) {
  MutexLock lock(mutex_);

  char run_xsum, fin_xsum, ch;
  std::string in;
  int has_seq, offs;

  // If nothing is waiting, return NONE
  if (io_->ReadWaitWithTimeout(kSessionTimeoutMs) == DPR_NO_DATA) {
    return DPR_NO_DATA;
  }

  // Toss characters until we see a start of command
  do {
    if (GetChar(&ch) == DPR_ERROR) return DPR_ERROR;
    in += ch;
  } while (ch != '$');

 retry:
  has_seq = 1;
  offs    = 0;

  // If nothing is waiting, return NONE
  if (io_->ReadWaitWithTimeout(kSessionTimeoutMs) == DPR_NO_DATA) {
    return DPR_NO_DATA;
  }

  // Clear the stream
  pkt->Clear();

  // Prepare XSUM calc
  run_xsum = 0;
  fin_xsum = 0;

  // Stream in the characters
  while (1) {
    if (GetChar(&ch) == DPR_ERROR) return DPR_ERROR;

    in += ch;
     // Check SEQ statemachine  xx:
    switch (offs) {
      case 0:
      case 1:
        if (!NibbleToInt(ch, 0)) has_seq = 0;
        break;

      case 2:
        if (ch != ':') has_seq = 0;
        break;
    }
    offs++;

    // If we see a '#' we must be done with the data
    if (ch == '#') break;

    // If we see a '$' we must have missed the last cmd
    if (ch == '$') {
      IPlatform::LogInfo("RX Missing $, retry.\n");
      goto retry;
    }
    // Keep a running XSUM
    run_xsum += ch;
    pkt->AddRawChar(ch);
  }


  // Get two Nibble XSUM
  if (GetChar(&ch) == DPR_ERROR) return DPR_ERROR;
  in += ch;

  int val;
  NibbleToInt(ch, & val);
  fin_xsum = val << 4;

  if (GetChar(&ch) == DPR_ERROR) return DPR_ERROR;
  in += ch;
  NibbleToInt(ch, &val);
  fin_xsum |= val;

  if (GetFlags() & DEBUG_RECV) IPlatform::LogInfo("RX %s\n", in.data());

  // Pull off teh sequence number if we have one
  if (has_seq) {
    uint8_t seq;
    char ch;

    pkt->GetWord8(&seq);
    pkt->SetSequence(seq);
    pkt->GetRawChar(&ch);
    if (ch != ':') {
      IPlatform::LogError("RX mismatched SEQ.\n");
      return DPR_ERROR;
    }
  }

  // If ACKs are off, we are done.
  if (GetFlags() & IGNORE_ACK) return DPR_OK;

  // If the XSUMs don't match, signal bad packet
  if (fin_xsum == run_xsum) {
    char out[4] = { '+', 0, 0, 0};
    int32_t seq;

    // If we have a sequence number
    if (pkt->GetSequence(&seq)) {
      // Respond with Sequence number
      IntToNibble(seq >> 4, &out[1]);
      IntToNibble(seq & 0xF, &out[2]);
    }
    return SendStream(out);
  } else {
    // Resend a bad XSUM and look for retransmit
    SendStream("-");

    IPlatform::LogInfo("RX Bad XSUM, retry\n");
    goto retry;
  }

  return DPR_OK;
}

}  // End of namespace gdb_rsp

