#include <errno.h>
#include <assert.h>
#include "nacl/nacl_dyncode.h"

#include "support.h"
#include "jit.h"

int
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
  if ((ret = dasm_link(Dst, &codesz))) return ret;
  assert(codesz > 0 && "Did you fill out the assembly code?");
  code = (uint8_t*)malloc(codesz);
  if ((ret = dasm_encode(Dst, (void*)code))) return ret;

  Dst->glob   = glob;
  Dst->frag   = code;
  Dst->fragsz = codesz;

  return ret;
}

int
jit_validate_code(Dst_DECL)
{
  int ret = 0;
  if (Dst->fragsz > BUF_SIZE) return -1;

  /* Native client shenanigans */
  uint8_t buf[BUF_SIZE];
  uint8_t* load_area = allocate_code_space(1);
  /* Resulting fragments must be aligned and padded */
  copy_and_pad_fragment(buf, sizeof(buf), (char*)Dst->frag, Dst->fragsz);
  
  if((ret = nacl_load_code(load_area, buf, sizeof(buf)))) return ret;

  free(Dst->frag);
  Dst->frag = load_area;
  Dst->fragsz = sizeof(buf);

  return ret;
}

jit_fp
jit_get_fp(Dst_DECL)
{
  return (jit_fp)Dst->frag;
}

void
jit_destroy_code(Dst_DECL)
{
  dasm_free(Dst);
  free(Dst->glob);
  // TODO: nacl_dyncode_destroy
}
