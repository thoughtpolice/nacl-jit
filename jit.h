#ifndef _NACL_JIT_H
#define _NACL_JIT_H

/** DynASM encoding state */
typedef struct {
  struct dasm_State* D;
  uint8_t* frag;
  size_t fragsz;
  void** glob;
} BuildCtx;

/** DynASM definitions */
#define DASM_MAXSECTION 1024
#define DASM_ALIGNED_WRITES 1
#define DASM_CHECKS 1

/* Psuedo-binders for type definition */
#define Dst_TYPE BuildCtx
#define Dst ctx
#define Dst_DECL Dst_TYPE* Dst
#define Dst_REF (Dst->D)

/* DynASM engine */
#include "dasm_proto.h"
#if defined(ARCH_X86_64)
#include "dasm_x86.h"
#include "jit_x86.h"
#elif defined(ARCH_X86)
#include "dasm_x86.h"
#include "jit_x86_64.h"
#elif defined(ARCH_ARM)
#error Architecture not supported
#include "dasm_arm.h"
#include "jit_arm.h"
#else
#error Architecture not supported
#endif

#endif /*  _NACL_JIT_H */
