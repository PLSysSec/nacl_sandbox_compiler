/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


// Utility class for launching sel_ldr.

#ifndef NATIVE_CLIENT_SRC_TRUSTED_NONNACL_UTIL_SEL_LDR_LAUNCHER_H_
#define NATIVE_CLIENT_SRC_TRUSTED_NONNACL_UTIL_SEL_LDR_LAUNCHER_H_

#include <vector>

#include "native_client/src/include/nacl_string.h"
#include "native_client/src/include/portability.h"
#include "native_client/src/shared/imc/nacl_imc.h"

struct NaClSrpcChannel;
struct NaClDesc;

namespace nacl {

// TODO(robertm): Move this to new header if it becomes more popular.
template <class T> nacl::string ToString(const T& t) {
  nacl::stringstream ss;
  ss << t;
  return ss.str();
}

/*
 * This class defines an interface that locates sel_ldr.
 */
class SelLdrLocator {
 public:
  // Returns a directory with sel_ldr.
  virtual void GetDirectory(char* buffer, size_t len) = 0;

  SelLdrLocator() {}

  virtual ~SelLdrLocator() { }

  DISALLOW_COPY_AND_ASSIGN(SelLdrLocator);
};

/*
 * Default implementation of SelLdrLocator which tries to
 * locate a browser plugin. Default constructor of SelLdrLauncher use it.
 */
class PluginSelLdrLocator : public SelLdrLocator {
 public:
  // We have different implementations for all platforms.
  virtual void GetDirectory(char* buffer, size_t len);

  PluginSelLdrLocator() {
  }

  DISALLOW_COPY_AND_ASSIGN(PluginSelLdrLocator);
};

/*
 * This class encapsulates the process of launching an instance of sel_ldr
 * to communicate over an IMC channel.
 */
struct SelLdrLauncher {
 public:
  SelLdrLauncher();

  explicit SelLdrLauncher(SelLdrLocator* sel_ldr_locator);

  // Creates a socket pair.  Maps the first socket into the NaCl
  // subprocess's FD table as dest_fd.  Returns the second socket.
  // Returns kInvalidHandle on failure.
  Handle ExportImcFD(int dest_fd);

  // TODO(robertm): shouldn't some of these args go in the constructor
  // application_argv can take a "$CHAN" as an argument which will be replaced
  // with `imc_fd` number if the target application is a NaCl module
  // and with `channel_` number if the target application is a native OS
  // binary.

  void Init(const nacl::string& application_name,
            int imc_fd,
            const std::vector<nacl::string>& sel_ldr_argv,
            const std::vector<nacl::string>& application_argv);

  // If subprocess creation fails, both child_ and channel_ are set to
  // kInvalidHandle. We have different implementations for unix and win.
  // NOTE: you must call Init() and InitChannelBuf() before Launch()
  bool Launch();

  bool Start(const nacl::string& application_name,
             int imc_fd,
             const std::vector<nacl::string>& sel_ldr_argv,
             const std::vector<nacl::string>& app_argv) {
    Init(application_name, imc_fd, sel_ldr_argv, app_argv);
    return Launch();
  }

  // TODO(gregoryd): add some background story here
#ifdef NACL_STANDALONE
  // Should never be called when not running in Chrome.
  bool Start(const char* url, int imc_fd) {
    UNREFERENCED_PARAMETER(url);
    UNREFERENCED_PARAMETER(imc_fd);
    return false;
  }
#else
  // Launch sel_ldr process in Chrome by sending a message
  // to the browser process.
  bool Start(const char* url, int imc_fd);
#endif
  // OpenSrpcChannels essentially is a triple Ctor for the three
  // NaClSrpcChannel objects; if it returns true (success), all were
  // constructed; if it returns false, none were constructed (and thus
  // none needs to be Dtor'd).
  bool OpenSrpcChannels(NaClSrpcChannel* command,
                        NaClSrpcChannel* untrusted_command,
                        NaClSrpcChannel* untrusted);

  ~SelLdrLauncher();

  Handle child() const { return child_; }

  // User is responsible for invoking channel() and then taking
  // ownership of the handle prior to the Dtor firing.
  Handle channel() const { return channel_; }

  nacl::string GetApplicationName() const { return application_name_; }

  // Returns the socket address used to connect to the sel_ldr.
  struct NaClDesc* GetSelLdrSocketAddress() const { return sock_addr_; }

  // Kill the child process.  The channel() remains valid, but nobody
  // is talking on the other end.  Returns true if successful.
  bool KillChild();

 private:
  void GetPluginDirectory(char* buffer, size_t len);

  nacl::string GetSelLdrPathName();

  nacl::string ExpandVar(nacl::string arg);

  void BuildArgv(std::vector<nacl::string>* argv);

  void InitChannelBuf(Handle handle);

  void CloseHandlesAfterLaunch();

  Handle child_;
  Handle channel_;
  // The following strings and vectors are used by BuildArgv to
  // create a command line, they are initialized by InitBasic().
  nacl::string channel_number_;
  nacl::string sel_ldr_;
  nacl::string application_name_;
  std::vector<nacl::string> sel_ldr_argv_;
  std::vector<nacl::string> application_argv_;
  // unlike the others above this is set from within Launch();
  nacl::string channel_buf_;
  std::vector<Handle> close_after_launch_;

  // The socket address returned from sel_ldr for connects.
  struct NaClDesc* sock_addr_;

  SelLdrLocator* sel_ldr_locator_;
};

}  // namespace nacl

#endif  // NATIVE_CLIENT_SRC_TRUSTED_NONNACL_UTIL_SEL_LDR_LAUNCHER_H_
