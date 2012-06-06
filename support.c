/** Snipped from native client test suite, files:
 * native_client/tests/dynamic_code_loading/dynamic_load_test.c
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "nacl/nacl_dyncode.h"

#include "support.h"

int nacl_load_code(void *dest, void *src, int size) {
  int rc = nacl_dyncode_create(dest, src, size);
  /* Undo the syscall wrapper's errno handling, because it's more
     convenient to test a single return value. */
  return rc == 0 ? 0 : -errno;
}

uint8_t* next_addr = NULL;

uint8_t* allocate_code_space(int pages) {
  uint8_t* addr;
  if (next_addr == NULL) {
    next_addr = (uint8_t*) DYNAMIC_CODE_SEGMENT_START;
  }
  addr = next_addr;
  next_addr += PAGE_SIZE * pages;
  //assert(next_addr < (char *) DYNAMIC_CODE_SEGMENT_END);
  return addr;
}

void fill_int32(uint8_t *data, size_t size, int32_t value) {
  int i;
  assert(size % 4 == 0);
  /* All the archs we target supported unaligned word read/write, but
     check that the pointer is aligned anyway. */
  assert(((uintptr_t) data) % 4 == 0);
  for (i = 0; i < size / 4; i++)
    ((uint32_t *) data)[i] = value;
}

void fill_nops(uint8_t *data, size_t size) {
#if defined(__i386__) || defined(__x86_64__)
  memset(data, 0x90, size); /* NOPs */
#elif defined(__arm__)
  fill_int32(data, size, 0xe1a00000); /* NOP (MOV r0, r0) */
#else
# error "Unknown arch"
#endif
}

void fill_hlts(uint8_t *data, size_t size) {
#if defined(__i386__) || defined(__x86_64__)
  memset(data, 0xf4, size); /* HLTs */
#elif defined(__arm__)
  fill_int32(data, size, 0xe1266676); /* BKPT 0x6666 */
#else
# error "Unknown arch"
#endif
}

void copy_and_pad_fragment(void *dest,
                           size_t dest_size,
                           const char *frag,
                           size_t frag_size) {
  assert(dest_size % NACL_BUNDLE_SIZE == 0);
  assert(frag_size <= dest_size);
  fill_nops(dest, dest_size);
  memcpy(dest, frag, frag_size);
}
