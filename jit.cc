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

static int
jit_build_code(Dst_DECL)
{
  int ret = 0;
  size_t codesz = 0;
  uint8_t* code = NULL;

  /* TODO: remove when there really are globals */
  size_t nglobs = JIT_GLOBAL__MAX < 1 ? 1 : JIT_GLOBAL__MAX;
  void** glob   = (void**)malloc(nglobs * sizeof(void*));

  dasm_init(Dst, DASM_MAXSECTION);
  dasm_setupglobal(Dst, glob, nglobs);
  dasm_setup(Dst, jit_build_actionlist);

  (void)build_code(Dst);

  /* Finalize */
  (void)dasm_checkstep(Dst, -1); /* sanity check */
  if((ret = dasm_link(Dst, &codesz))) return ret;
  code = (uint8_t*) malloc(codesz);
  if((ret = dasm_encode(Dst, (void*)code))) return ret;
  Dst->glob   = glob;
  Dst->frag   = code;
  Dst->fragsz = codesz;

  return ret;
}

static void
jit_destroy_code(Dst_DECL)
{
  dasm_free(Dst);
  free(Dst->glob);
  free(Dst->frag);
}

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
    void* src = (void*)Dst->frag;
    void* dst = (void*)malloc(Dst->fragsz);
    if (0 != nacl_dyncode_create(dst, src, Dst->fragsz)) {
      SAY("oh noes!");
      SAY(strerror(errno));

      jit_destroy_code(Dst);
      free(Dst);
      return;
    }

    SAY("Executing JIT code...");
    
    SAY("Done.");
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
