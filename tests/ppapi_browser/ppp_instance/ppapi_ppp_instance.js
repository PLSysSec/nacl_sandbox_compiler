// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function startsWith(str, prefix) {
  return (str.indexOf(prefix) === 0);
}

function setupTests(tester, plugin) {
  //////////////////////////////////////////////////////////////////////////////
  // Test Helpers
  //////////////////////////////////////////////////////////////////////////////
  var numMessages = 0;
  function addTestListeners(numListeners, test, testFunction, runCheck) {
    var messageListener = test.wrap(function(message) {
      if (!startsWith(message.data, testFunction)) return;
      test.log(message.data);
      numMessages++;
      plugin.removeEventListener('message', messageListener, false);
      test.assertEqual(message.data, testFunction + ':PASSED');
      if (runCheck) test.assert(runCheck());
      if (numMessages < numListeners) {
        plugin.addEventListener('message', messageListener, false);
      } else {
        numMessages = 0;
        test.pass();
      }
    });
    plugin.addEventListener('message', messageListener, false);
  }

  function addTestListener(test, testFunction, runCheck) {
    return addTestListeners(1, test, testFunction, runCheck);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Tests
  //////////////////////////////////////////////////////////////////////////////

  tester.addTest('PPP_Instance::DidCreate', function() {
    assertEqual(plugin.lastError, '');
  });

  tester.addAsyncTest('PPP_Instance::DidChangeView', function(test) {
    addTestListeners(3, test, 'DidChangeView');
  });

  tester.addAsyncTest('PPP_Instance::DidChangeFocus', function(test) {
    // TODO(polina): How can I simulate focusing on Windows?
    // For now just pass explicitely.
    if (startsWith(navigator.platform, 'Win')) {
      test.log('skipping test on ' + navigator.platform);
      test.pass();
      return;
    }
    addTestListeners(2, test, 'DidChangeFocus');
    plugin.tabIndex = 0;
    plugin.focus();
    plugin.blur();
  });

  // PPP_Instance::HandleDocumentLoad is only used with full-frame plugins.
  // This is tested in tests/ppapi_browser/extension_mime_handler/

  tester.addAsyncTest('PPP_Instance::DidDestroy', function(test) {
    // Destroy the plugin triggering PPP_Instance::DidDestroy.
    // A message should be logged to stdout.
    plugin.parentNode.removeChild(plugin);
    // Unfortunately, PPB_Messaging is a per-instance interface, so
    // posting messages from PPP_Instance::DidDestory does not work.
    // TODO(polina): Is there a different way to detect this from JavaScript?
    // For now just pass explicitely.
    test.pass();
  });
}
