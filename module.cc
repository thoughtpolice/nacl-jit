// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/// @file
/// This example demonstrates loading and running a very simple NaCl
/// module.  To load the NaCl module, the browser first looks for the
/// CreateModule() factory method (at the end of this file).  It calls
/// CreateModule() once to load the module code from your .nexe.  After the
/// .nexe code is loaded, CreateModule() is not called again.
///
/// Once the .nexe code is loaded, the browser then calls the
/// LoadProgressModule::CreateInstance()
/// method on the object returned by CreateModule().  It calls CreateInstance()
/// each time it encounters an <embed> tag that references your NaCl module.

#include <errno.h>

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "nacl/nacl_dyncode.h"

#include "jit.h"

namespace jit {
/// The Instance class.  One of these exists for each instance of your NaCl
/// module on the web page.  The browser will ask the Module object to create
/// a new Instance for each occurrence of the <embed> tag that has these
/// attributes:
/// <pre>
///     type="application/x-nacl"
///     nacl="hello_world.nmf"
/// </pre>

#define SAY(x) PostMessage(pp::Var(x))
class JitInstance : public pp::Instance {
 public:
  explicit JitInstance(PP_Instance instance)
      : pp::Instance(instance) {}
  virtual ~JitInstance() {}

  virtual void HandleMessage(const pp::Var& msg) {
    if (!msg.is_string())
      return;
    std::string message = msg.AsString();
    if (message != "GO") 
      return;
    
    SAY("Building code...");
    Dst_DECL = (Dst_TYPE*)malloc(sizeof(Dst_TYPE));
    if(0 != jit_build_code(Dst)) {
      SAY("Could not build code!");
      free(Dst);
      return;
    }
    
    SAY("Validating JIT code...");
    SAY(((int32_t)Dst->frag));
    SAY(((int32_t)Dst->fragsz));
    if (0 != jit_validate_code(Dst)) {
      SAY("could not validate code!");
      jit_destroy_code(Dst);
      free(Dst);
      return;
    }

    SAY("Executing JIT code...");
    jit_fp func = jit_get_fp(Dst);
    //int rc = func();
    int rc = 0;

    SAY("Done.");
    SAY(rc);
    jit_destroy_code(Dst);
    free(Dst);
    return;
  }
};
#undef SAY

/// The Module class.  The browser calls the CreateInstance() method to create
/// an instance of your NaCl module on the web page.  The browser creates a new
/// instance for each <embed> tag with
/// <code>type="application/x-nacl"</code>.
class JitModule : public pp::Module {
 public:
  JitModule() : pp::Module() {}
  virtual ~JitModule() {}

  /// Create and return a HelloWorldInstance object.
  /// @param[in] instance a handle to a plug-in instance.
  /// @return a newly created HelloWorldInstance.
  /// @note The browser is responsible for calling @a delete when done.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new JitInstance(instance);
  }
};
}  // namespace jit


namespace pp {
/// Factory function called by the browser when the module is first loaded.
/// The browser keeps a singleton of this module.  It calls the
/// CreateInstance() method on the object you return to make instances.  There
/// is one instance per <embed> tag on the page.  This is the main binding
/// point for your NaCl module with the browser.
/// @return new JitModule.
/// @note The browser is responsible for deleting returned @a Module.
Module* CreateModule() {
  return new jit::JitModule();
}
}  // namespace pp
