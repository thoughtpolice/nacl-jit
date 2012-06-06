/** Snipped from native client test suite, files:
 * native_client/tests/dynamic_code_loading/dynamic_segment.h
 * native_client/tests/dynamic_code_loading/dynamic_load_test.h
 * native_client/tests/dynamic_code_loading/dynamic_load_test.c
 */
#ifndef _SUPPORT_H_
#define _SUPPORT_H_

#if defined(__x86_64__)
/* On x86-64, template functions do not fit in 32-byte buffers */
#define BUF_SIZE 64
#elif defined(__i386__) || defined(__arm__)
#define BUF_SIZE 32
#else
#error "Unknown Platform"
#endif


#if defined(__i386__) || defined(__x86_64__)
#define NACL_BUNDLE_SIZE  32
#elif defined(__arm__)
#define NACL_BUNDLE_SIZE  16
#else
#error "Unknown Platform"
#endif

#define PAGE_SIZE 0x10000

extern char _etext[];
/* TODO(sehr): add a sysconf to get the page size, rather than this magic. */
#define DYNAMIC_CODE_PAGE_SIZE     (PAGE_SIZE)
#define DYNAMIC_CODE_ALIGN(addr)   \
  ((((uintptr_t) (addr)) + DYNAMIC_CODE_PAGE_SIZE - 1) & \
   ~(DYNAMIC_CODE_PAGE_SIZE - 1))

#define DYNAMIC_CODE_SEGMENT_START (DYNAMIC_CODE_ALIGN(_etext))

/* 
 * TODO(bsy): get this value from the toolchain.  Get the toolchain 
 * team to provide this value.
 */
#define NUM_BUNDLES_FOR_HLT 3

#ifdef __cplusplus
extern "C" {
#endif 
int nacl_load_code(void*, void*, int);
uint8_t* allocate_code_space(int);
void fill_int32(uint8_t*, size_t, int32_t);
void fill_nops(uint8_t*, size_t);
void copy_and_pad_fragment(void*, size_t, const char*, size_t);
#ifdef __cplusplus
}
#endif

#endif /* _SUPPORT_H_ */
