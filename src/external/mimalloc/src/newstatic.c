/* ----------------------------------------------------------------------------
Copyright (c) 2018-2024, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE   // for realpath() on Linux
#endif

#include "mimalloc.h"
#include "mimalloc/internal.h"
#include "mimalloc/atomic.h"
#include "mimalloc/prim.h"   // _mi_prim_thread_id()

#include <string.h>      // memset, strlen (for mi_strdup)
#include <stdlib.h>      // malloc, abort

#define MI_IN_ALLOC_C
#include "alloc-override.c"
#include "free.c"
#undef MI_IN_ALLOC_C

// ------------------------------------------------------
// Allocation
// ------------------------------------------------------

// Fast allocation in a page: just pop from the free list.
// Fall back to generic allocation only if the list is empty.
// Note: in release mode the (inlined) routine is about 7 instructions with a single test.
extern inline void* _mi_page_malloc_zero(mi_heap_t* heap, mi_page_t* page, size_t size, bool zero) mi_attr_noexcept
{
  if (page->block_size != 0) { // not the empty heap
    mi_assert_internal(mi_page_block_size(page) >= size);
    mi_assert_internal(_mi_is_aligned(page, MI_PAGE_ALIGN));
    mi_assert_internal(_mi_ptr_page(page)==page);
  }

  // check the free list
  mi_block_t* const block = page->free;
  if mi_unlikely(block == NULL) {
    return _mi_malloc_generic(heap, size, zero, 0);
  }
  mi_assert_internal(block != NULL && _mi_ptr_page(block) == page);

  // pop from the free list
  page->free = mi_block_next(page, block);
  page->used++;
  mi_assert_internal(page->free == NULL || _mi_ptr_page(page->free) == page);
  mi_assert_internal(page->block_size < MI_MAX_ALIGN_SIZE || _mi_is_aligned(block, MI_MAX_ALIGN_SIZE));

  #if MI_DEBUG>3
  if (page->free_is_zero && size > sizeof(*block)) {
    mi_assert_expensive(mi_mem_is_zero(block+1,size - sizeof(*block)));
  }
  #endif

  // allow use of the block internally
  // note: when tracking we need to avoid ever touching the MI_PADDING since
  // that is tracked by valgrind etc. as non-accessible (through the red-zone, see `mimalloc/track.h`)
  mi_track_mem_undefined(block, mi_page_usable_block_size(page));

  // zero the block? note: we need to zero the full block size (issue #63)
  if mi_unlikely(zero) {
    mi_assert_internal(page->block_size != 0); // do not call with zero'ing for huge blocks (see _mi_malloc_generic)
    mi_assert_internal(!mi_page_is_huge(page));
    #if MI_PADDING
    mi_assert_internal(page->block_size >= MI_PADDING_SIZE);
    #endif
    if (page->free_is_zero) {
      block->next = 0;
      mi_track_mem_defined(block, page->block_size - MI_PADDING_SIZE);
    }
    else {
      _mi_memzero_aligned(block, page->block_size - MI_PADDING_SIZE);
    }
  }

  #if (MI_DEBUG>0) && !MI_TRACK_ENABLED && !MI_TSAN
  if (!zero && !mi_page_is_huge(page)) {
    memset(block, MI_DEBUG_UNINIT, mi_page_usable_block_size(page));
  }
  #elif (MI_SECURE!=0)
  if (!zero) { block->next = 0; } // don't leak internal data
  #endif

  #if (MI_STAT>0)
  const size_t bsize = mi_page_usable_block_size(page);
  if (bsize <= MI_LARGE_MAX_OBJ_SIZE) {
    mi_heap_stat_increase(heap, malloc_normal, bsize);
    mi_heap_stat_counter_increase(heap, malloc_normal_count, 1);
    #if (MI_STAT>1)
    const size_t bin = _mi_bin(bsize);
    mi_heap_stat_increase(heap, malloc_bins[bin], 1);
    mi_heap_stat_increase(heap, malloc_requested, size - MI_PADDING_SIZE);
    #endif
  }
  #endif

  #if MI_PADDING // && !MI_TRACK_ENABLED
    mi_padding_t* const padding = (mi_padding_t*)((uint8_t*)block + mi_page_usable_block_size(page));
    ptrdiff_t delta = ((uint8_t*)padding - (uint8_t*)block - (size - MI_PADDING_SIZE));
    #if (MI_DEBUG>=2)
    mi_assert_internal(delta >= 0 && mi_page_usable_block_size(page) >= (size - MI_PADDING_SIZE + delta));
    #endif
    mi_track_mem_defined(padding,sizeof(mi_padding_t));  // note: re-enable since mi_page_usable_block_size may set noaccess
    padding->canary = mi_ptr_encode_canary(page,block,page->keys);
    padding->delta  = (uint32_t)(delta);
    #if MI_PADDING_CHECK
    if (!mi_page_is_huge(page)) {
      uint8_t* fill = (uint8_t*)padding - delta;
      const size_t maxpad = (delta > MI_MAX_ALIGN_SIZE ? MI_MAX_ALIGN_SIZE : delta); // set at most N initial padding bytes
      for (size_t i = 0; i < maxpad; i++) { fill[i] = MI_DEBUG_PADDING; }
    }
    #endif
  #endif

  return block;
}

// extra entries for improved efficiency in `alloc-aligned.c`.
extern void* _mi_page_malloc(mi_heap_t* heap, mi_page_t* page, size_t size) mi_attr_noexcept {
  return _mi_page_malloc_zero(heap,page,size,false);
}
extern void* _mi_page_malloc_zeroed(mi_heap_t* heap, mi_page_t* page, size_t size) mi_attr_noexcept {
  return _mi_page_malloc_zero(heap,page,size,true);
}

#if MI_GUARDED
mi_decl_restrict void* _mi_heap_malloc_guarded(mi_heap_t* heap, size_t size, bool zero) mi_attr_noexcept;
#endif

static inline mi_decl_restrict void* mi_heap_malloc_small_zero(mi_heap_t* heap, size_t size, bool zero) mi_attr_noexcept {
  mi_assert(heap != NULL);
  mi_assert(size <= MI_SMALL_SIZE_MAX);
  #if MI_DEBUG
  const uintptr_t tid = _mi_thread_id();
  mi_assert(heap->tld->thread_id == 0 || heap->tld->thread_id == tid); // heaps are thread local
  #endif
  #if (MI_PADDING || MI_GUARDED)
  if (size == 0) { size = sizeof(void*); }
  #endif
  #if MI_GUARDED
  if (mi_heap_malloc_use_guarded(heap,size)) {
    return _mi_heap_malloc_guarded(heap, size, zero);
  }
  #endif

  // get page in constant time, and allocate from it
  mi_page_t* page = _mi_heap_get_free_small_page(heap, size + MI_PADDING_SIZE);
  void* const p = _mi_page_malloc_zero(heap, page, size + MI_PADDING_SIZE, zero);
  mi_track_malloc(p,size,zero);

  #if MI_DEBUG>3
  if (p != NULL && zero) {
    mi_assert_expensive(mi_mem_is_zero(p, size));
  }
  #endif
  return p;
}

// allocate a small block
mi_decl_nodiscard extern inline mi_decl_restrict void* mi_heap_malloc_small(mi_heap_t* heap, size_t size) mi_attr_noexcept {
  return mi_heap_malloc_small_zero(heap, size, false);
}

mi_decl_nodiscard extern inline mi_decl_restrict void* mi_malloc_small(size_t size) mi_attr_noexcept {
  return mi_heap_malloc_small(mi_prim_get_default_heap(), size);
}

// The main allocation function
extern inline void* _mi_heap_malloc_zero_ex(mi_heap_t* heap, size_t size, bool zero, size_t huge_alignment) mi_attr_noexcept {
  // fast path for small objects
  if mi_likely(size <= MI_SMALL_SIZE_MAX) {
    mi_assert_internal(huge_alignment == 0);
    return mi_heap_malloc_small_zero(heap, size, zero);
  }
  #if MI_GUARDED
  else if (huge_alignment==0 && mi_heap_malloc_use_guarded(heap,size)) {
    return _mi_heap_malloc_guarded(heap, size, zero);
  }
  #endif
  else {
    // regular allocation
    mi_assert(heap!=NULL);
    mi_assert(heap->tld->thread_id == 0 || heap->tld->thread_id == _mi_thread_id());   // heaps are thread local
    void* const p = _mi_malloc_generic(heap, size + MI_PADDING_SIZE, zero, huge_alignment);  // note: size can overflow but it is detected in malloc_generic
    mi_track_malloc(p,size,zero);
    
    #if MI_DEBUG>3
    if (p != NULL && zero) {
      mi_assert_expensive(mi_mem_is_zero(p, size));
    }
    #endif
    return p;
  }
}

extern inline void* _mi_heap_malloc_zero(mi_heap_t* heap, size_t size, bool zero) mi_attr_noexcept {
  return _mi_heap_malloc_zero_ex(heap, size, zero, 0);
}

mi_decl_nodiscard extern inline mi_decl_restrict void* mi_heap_malloc(mi_heap_t* heap, size_t size) mi_attr_noexcept {
  return _mi_heap_malloc_zero(heap, size, false);
}

mi_decl_nodiscard extern inline mi_decl_restrict void* mi_malloc(size_t size) mi_attr_noexcept {
  return mi_heap_malloc(mi_prim_get_default_heap(), size);
}

// zero initialized small block
mi_decl_nodiscard mi_decl_restrict void* mi_zalloc_small(size_t size) mi_attr_noexcept {
  return mi_heap_malloc_small_zero(mi_prim_get_default_heap(), size, true);
}

mi_decl_nodiscard extern inline mi_decl_restrict void* mi_heap_zalloc(mi_heap_t* heap, size_t size) mi_attr_noexcept {
  return _mi_heap_malloc_zero(heap, size, true);
}

mi_decl_nodiscard mi_decl_restrict void* mi_zalloc(size_t size) mi_attr_noexcept {
  return mi_heap_zalloc(mi_prim_get_default_heap(),size);
}


mi_decl_nodiscard extern inline mi_decl_restrict void* mi_heap_calloc(mi_heap_t* heap, size_t count, size_t size) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(count,size,&total)) return NULL;
  return mi_heap_zalloc(heap,total);
}

mi_decl_nodiscard mi_decl_restrict void* mi_calloc(size_t count, size_t size) mi_attr_noexcept {
  return mi_heap_calloc(mi_prim_get_default_heap(),count,size);
}

// Uninitialized `calloc`
mi_decl_nodiscard extern mi_decl_restrict void* mi_heap_mallocn(mi_heap_t* heap, size_t count, size_t size) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(count, size, &total)) return NULL;
  return mi_heap_malloc(heap, total);
}

mi_decl_nodiscard mi_decl_restrict void* mi_mallocn(size_t count, size_t size) mi_attr_noexcept {
  return mi_heap_mallocn(mi_prim_get_default_heap(),count,size);
}

// Expand (or shrink) in place (or fail)
void* mi_expand(void* p, size_t newsize) mi_attr_noexcept {
  #if MI_PADDING
  // we do not shrink/expand with padding enabled
  MI_UNUSED(p); MI_UNUSED(newsize);
  return NULL;
  #else
  if (p == NULL) return NULL;
  const size_t size = _mi_usable_size(p,"mi_expand");
  if (newsize > size) return NULL;
  return p; // it fits
  #endif
}

void* _mi_heap_realloc_zero(mi_heap_t* heap, void* p, size_t newsize, bool zero) mi_attr_noexcept {
  // if p == NULL then behave as malloc.
  // else if size == 0 then reallocate to a zero-sized block (and don't return NULL, just as mi_malloc(0)).
  // (this means that returning NULL always indicates an error, and `p` will not have been freed in that case.)
  const size_t size = (p==NULL ? 0 : _mi_usable_size(p,"mi_realloc")); 
  if mi_unlikely(newsize <= size && newsize >= (size / 2) && newsize > 0) {  // note: newsize must be > 0 or otherwise we return NULL for realloc(NULL,0)
    mi_assert_internal(p!=NULL);
    // todo: do not track as the usable size is still the same in the free; adjust potential padding?
    // mi_track_resize(p,size,newsize)
    // if (newsize < size) { mi_track_mem_noaccess((uint8_t*)p + newsize, size - newsize); }
    return p;  // reallocation still fits and not more than 50% waste
  }
  void* newp = mi_heap_malloc(heap,newsize);
  if mi_likely(newp != NULL) {
    if (zero && newsize > size) {
      // also set last word in the previous allocation to zero to ensure any padding is zero-initialized
      const size_t start = (size >= sizeof(intptr_t) ? size - sizeof(intptr_t) : 0);
      _mi_memzero((uint8_t*)newp + start, newsize - start);
    }
    else if (newsize == 0) {
      ((uint8_t*)newp)[0] = 0; // work around for applications that expect zero-reallocation to be zero initialized (issue #725)
    }
    if mi_likely(p != NULL) {
      const size_t copysize = (newsize > size ? size : newsize);
      mi_track_mem_defined(p,copysize);  // _mi_useable_size may be too large for byte precise memory tracking..
      _mi_memcpy(newp, p, copysize);
      mi_free(p); // only free the original pointer if successful
    }
  }
  return newp;
}

mi_decl_nodiscard void* mi_heap_realloc(mi_heap_t* heap, void* p, size_t newsize) mi_attr_noexcept {
  return _mi_heap_realloc_zero(heap, p, newsize, false);
}

mi_decl_nodiscard void* mi_heap_reallocn(mi_heap_t* heap, void* p, size_t count, size_t size) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(count, size, &total)) return NULL;
  return mi_heap_realloc(heap, p, total);
}


// Reallocate but free `p` on errors
mi_decl_nodiscard void* mi_heap_reallocf(mi_heap_t* heap, void* p, size_t newsize) mi_attr_noexcept {
  void* newp = mi_heap_realloc(heap, p, newsize);
  if (newp==NULL && p!=NULL) mi_free(p);
  return newp;
}

mi_decl_nodiscard void* mi_heap_rezalloc(mi_heap_t* heap, void* p, size_t newsize) mi_attr_noexcept {
  return _mi_heap_realloc_zero(heap, p, newsize, true);
}

mi_decl_nodiscard void* mi_heap_recalloc(mi_heap_t* heap, void* p, size_t count, size_t size) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(count, size, &total)) return NULL;
  return mi_heap_rezalloc(heap, p, total);
}


mi_decl_nodiscard void* mi_realloc(void* p, size_t newsize) mi_attr_noexcept {
  return mi_heap_realloc(mi_prim_get_default_heap(),p,newsize);
}

mi_decl_nodiscard void* mi_reallocn(void* p, size_t count, size_t size) mi_attr_noexcept {
  return mi_heap_reallocn(mi_prim_get_default_heap(),p,count,size);
}

// Reallocate but free `p` on errors
mi_decl_nodiscard void* mi_reallocf(void* p, size_t newsize) mi_attr_noexcept {
  return mi_heap_reallocf(mi_prim_get_default_heap(),p,newsize);
}

mi_decl_nodiscard void* mi_rezalloc(void* p, size_t newsize) mi_attr_noexcept {
  return mi_heap_rezalloc(mi_prim_get_default_heap(), p, newsize);
}

mi_decl_nodiscard void* mi_recalloc(void* p, size_t count, size_t size) mi_attr_noexcept {
  return mi_heap_recalloc(mi_prim_get_default_heap(), p, count, size);
}



// ------------------------------------------------------
// strdup, strndup, and realpath
// ------------------------------------------------------

// `strdup` using mi_malloc
mi_decl_nodiscard mi_decl_restrict char* mi_heap_strdup(mi_heap_t* heap, const char* s) mi_attr_noexcept {
  if (s == NULL) return NULL;
  size_t len = _mi_strlen(s);
  char* t = (char*)mi_heap_malloc(heap,len+1);
  if (t == NULL) return NULL;
  _mi_memcpy(t, s, len);
  t[len] = 0;
  return t;
}

mi_decl_nodiscard mi_decl_restrict char* mi_strdup(const char* s) mi_attr_noexcept {
  return mi_heap_strdup(mi_prim_get_default_heap(), s);
}

// `strndup` using mi_malloc
mi_decl_nodiscard mi_decl_restrict char* mi_heap_strndup(mi_heap_t* heap, const char* s, size_t n) mi_attr_noexcept {
  if (s == NULL) return NULL;
  const size_t len = _mi_strnlen(s,n);  // len <= n
  char* t = (char*)mi_heap_malloc(heap, len+1);
  if (t == NULL) return NULL;
  _mi_memcpy(t, s, len);
  t[len] = 0;
  return t;
}

mi_decl_nodiscard mi_decl_restrict char* mi_strndup(const char* s, size_t n) mi_attr_noexcept {
  return mi_heap_strndup(mi_prim_get_default_heap(),s,n);
}

#ifndef __wasi__
// `realpath` using mi_malloc
#ifdef _WIN32
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

mi_decl_nodiscard mi_decl_restrict char* mi_heap_realpath(mi_heap_t* heap, const char* fname, char* resolved_name) mi_attr_noexcept {
  // todo: use GetFullPathNameW to allow longer file names
  char buf[PATH_MAX];
  DWORD res = GetFullPathNameA(fname, PATH_MAX, (resolved_name == NULL ? buf : resolved_name), NULL);
  if (res == 0) {
    errno = GetLastError(); return NULL;
  }
  else if (res > PATH_MAX) {
    errno = EINVAL; return NULL;
  }
  else if (resolved_name != NULL) {
    return resolved_name;
  }
  else {
    return mi_heap_strndup(heap, buf, PATH_MAX);
  }
}
#else
/*
#include <unistd.h>  // pathconf
static size_t mi_path_max(void) {
  static size_t path_max = 0;
  if (path_max <= 0) {
    long m = pathconf("/",_PC_PATH_MAX);
    if (m <= 0) path_max = 4096;      // guess
    else if (m < 256) path_max = 256; // at least 256
    else path_max = m;
  }
  return path_max;
}
*/
char* mi_heap_realpath(mi_heap_t* heap, const char* fname, char* resolved_name) mi_attr_noexcept {
  if (resolved_name != NULL) {
    return realpath(fname,resolved_name);
  }
  else {
    char* rname = realpath(fname, NULL);
    if (rname == NULL) return NULL;
    char* result = mi_heap_strdup(heap, rname);
    mi_cfree(rname);  // use checked free (which may be redirected to our free but that's ok)
    // note: with ASAN realpath is intercepted and mi_cfree may leak the returned pointer :-(
    return result;
  }
  /*
    const size_t n  = mi_path_max();
    char* buf = (char*)mi_malloc(n+1);
    if (buf == NULL) {
      errno = ENOMEM;
      return NULL;
    }
    char* rname  = realpath(fname,buf);
    char* result = mi_heap_strndup(heap,rname,n); // ok if `rname==NULL`
    mi_free(buf);
    return result;
  }
  */
}
#endif

mi_decl_nodiscard mi_decl_restrict char* mi_realpath(const char* fname, char* resolved_name) mi_attr_noexcept {
  return mi_heap_realpath(mi_prim_get_default_heap(),fname,resolved_name);
}
#endif

/*-------------------------------------------------------
C++ new and new_aligned
The standard requires calling into `get_new_handler` and
throwing the bad_alloc exception on failure. If we compile
with a C++ compiler we can implement this precisely. If we
use a C compiler we cannot throw a `bad_alloc` exception
but we call `exit` instead (i.e. not returning).
-------------------------------------------------------*/

#ifdef __cplusplus
#include <new>
static bool mi_try_new_handler(bool nothrow) {
  #if defined(_MSC_VER) || (__cplusplus >= 201103L)
    std::new_handler h = std::get_new_handler();
  #else
    std::new_handler h = std::set_new_handler();
    std::set_new_handler(h);
  #endif
  if (h==NULL) {
    _mi_error_message(ENOMEM, "out of memory in 'new'");
    #if defined(_CPPUNWIND) || defined(__cpp_exceptions)  // exceptions are not always enabled
    if (!nothrow) {
      throw std::bad_alloc();
    }
    #else
    MI_UNUSED(nothrow);
    #endif
    return false;
  }
  else {
    h();
    return true;
  }
}
#else
typedef void (*std_new_handler_t)(void);

#if (defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER)))  // exclude clang-cl, see issue #631
std_new_handler_t __attribute__((weak)) _ZSt15get_new_handlerv(void) {
  return NULL;
}
static std_new_handler_t mi_get_new_handler(void) {
  return _ZSt15get_new_handlerv();
}
#else
// note: on windows we could dynamically link to `?get_new_handler@std@@YAP6AXXZXZ`.
static std_new_handler_t mi_get_new_handler() {
  return NULL;
}
#endif

static bool mi_try_new_handler(bool nothrow) {
  std_new_handler_t h = mi_get_new_handler();
  if (h==NULL) {
    _mi_error_message(ENOMEM, "out of memory in 'new'");
    if (!nothrow) {
      abort();  // cannot throw in plain C, use abort
    }
    return false;
  }
  else {
    h();
    return true;
  }
}
#endif

mi_decl_export mi_decl_noinline void* mi_heap_try_new(mi_heap_t* heap, size_t size, bool nothrow ) {
  void* p = NULL;
  while(p == NULL && mi_try_new_handler(nothrow)) {
    p = mi_heap_malloc(heap,size);
  }
  return p;
}

static mi_decl_noinline void* mi_try_new(size_t size, bool nothrow) {
  return mi_heap_try_new(mi_prim_get_default_heap(), size, nothrow);
}


mi_decl_nodiscard mi_decl_restrict void* mi_heap_alloc_new(mi_heap_t* heap, size_t size) {
  void* p = mi_heap_malloc(heap,size);
  if mi_unlikely(p == NULL) return mi_heap_try_new(heap, size, false);
  return p;
}

mi_decl_nodiscard mi_decl_restrict void* mi_new(size_t size) {
  return mi_heap_alloc_new(mi_prim_get_default_heap(), size);
}


mi_decl_nodiscard mi_decl_restrict void* mi_heap_alloc_new_n(mi_heap_t* heap, size_t count, size_t size) {
  size_t total;
  if mi_unlikely(mi_count_size_overflow(count, size, &total)) {
    mi_try_new_handler(false);  // on overflow we invoke the try_new_handler once to potentially throw std::bad_alloc
    return NULL;
  }
  else {
    return mi_heap_alloc_new(heap,total);
  }
}

mi_decl_nodiscard mi_decl_restrict void* mi_new_n(size_t count, size_t size) {
  return mi_heap_alloc_new_n(mi_prim_get_default_heap(), count, size);
}


mi_decl_nodiscard mi_decl_restrict void* mi_new_nothrow(size_t size) mi_attr_noexcept {
  void* p = mi_malloc(size);
  if mi_unlikely(p == NULL) return mi_try_new(size, true);
  return p;
}

mi_decl_nodiscard mi_decl_restrict void* mi_new_aligned(size_t size, size_t alignment) {
  void* p;
  do {
    p = mi_malloc_aligned(size, alignment);
  }
  while(p == NULL && mi_try_new_handler(false));
  return p;
}

mi_decl_nodiscard mi_decl_restrict void* mi_new_aligned_nothrow(size_t size, size_t alignment) mi_attr_noexcept {
  void* p;
  do {
    p = mi_malloc_aligned(size, alignment);
  }
  while(p == NULL && mi_try_new_handler(true));
  return p;
}

mi_decl_nodiscard void* mi_new_realloc(void* p, size_t newsize) {
  void* q;
  do {
    q = mi_realloc(p, newsize);
  } while (q == NULL && mi_try_new_handler(false));
  return q;
}

mi_decl_nodiscard void* mi_new_reallocn(void* p, size_t newcount, size_t size) {
  size_t total;
  if mi_unlikely(mi_count_size_overflow(newcount, size, &total)) {
    mi_try_new_handler(false);  // on overflow we invoke the try_new_handler once to potentially throw std::bad_alloc
    return NULL;
  }
  else {
    return mi_new_realloc(p, total);
  }
}

#if MI_GUARDED
// We always allocate a guarded allocation at an offset (`mi_page_has_aligned` will be true).
// We then set the first word of the block to `0` for regular offset aligned allocations (in `alloc-aligned.c`)
// and the first word to `~0` for guarded allocations to have a correct `mi_usable_size`

static void* mi_block_ptr_set_guarded(mi_block_t* block, size_t obj_size) {
  // TODO: we can still make padding work by moving it out of the guard page area
  mi_page_t* const page = _mi_ptr_page(block);
  mi_page_set_has_aligned(page, true);
  block->next = MI_BLOCK_TAG_GUARDED;

  // set guard page at the end of the block
  const size_t block_size = mi_page_block_size(page);  // must use `block_size` to match `mi_free_local`
  const size_t os_page_size = _mi_os_page_size();
  mi_assert_internal(block_size >= obj_size + os_page_size + sizeof(mi_block_t));
  if (block_size < obj_size + os_page_size + sizeof(mi_block_t)) {
    // should never happen
    mi_free(block);
    return NULL;
  }
  uint8_t* guard_page = (uint8_t*)block + block_size - os_page_size;
  // note: the alignment of the guard page relies on blocks being os_page_size aligned which
  // is ensured in `mi_arena_page_alloc_fresh`.
  mi_assert_internal(_mi_is_aligned(block, os_page_size));
  mi_assert_internal(_mi_is_aligned(guard_page, os_page_size));
  if (!page->memid.is_pinned && _mi_is_aligned(guard_page, os_page_size)) {
    const bool ok = _mi_os_protect(guard_page, os_page_size);
    if (!ok) {
      _mi_warning_message("failed to set a guard page behind object (object %p of size %zu)\n", block, block_size);
    }
  }
  else {
    _mi_warning_message("unable to set a guard page behind an object due to pinned memory (large OS pages?) (object %p of size %zu)\n", block, block_size);
  }

  // align pointer just in front of the guard page
  size_t offset = block_size - os_page_size - obj_size;
  mi_assert_internal(offset > sizeof(mi_block_t));
  if (offset > MI_PAGE_MAX_OVERALLOC_ALIGN) {
    // give up to place it right in front of the guard page if the offset is too large for unalignment
    offset = MI_PAGE_MAX_OVERALLOC_ALIGN;
  }
  void* p = (uint8_t*)block + offset;  
  mi_track_align(block, p, offset, obj_size);
  mi_track_mem_defined(block, sizeof(mi_block_t));
  return p;
}

mi_decl_restrict void* _mi_heap_malloc_guarded(mi_heap_t* heap, size_t size, bool zero) mi_attr_noexcept
{
  #if defined(MI_PADDING_SIZE)
  mi_assert(MI_PADDING_SIZE==0);
  #endif
  // allocate multiple of page size ending in a guard page
  // ensure minimal alignment requirement?
  const size_t os_page_size = _mi_os_page_size();
  const size_t obj_size = (mi_option_is_enabled(mi_option_guarded_precise) ? size : _mi_align_up(size, MI_MAX_ALIGN_SIZE));
  const size_t bsize    = _mi_align_up(_mi_align_up(obj_size, MI_MAX_ALIGN_SIZE) + sizeof(mi_block_t), MI_MAX_ALIGN_SIZE);
  const size_t req_size = _mi_align_up(bsize + os_page_size, os_page_size);
  mi_block_t* const block = (mi_block_t*)_mi_malloc_generic(heap, req_size, zero, 0 /* huge_alignment */);
  if (block==NULL) return NULL;
  void* const p = mi_block_ptr_set_guarded(block, obj_size);

  // stats
  mi_track_malloc(p, size, zero);  
  if (p != NULL) {
    if (!mi_heap_is_initialized(heap)) { heap = mi_prim_get_default_heap(); }
    #if MI_STAT>1
    mi_heap_stat_decrease(heap, malloc_requested, req_size);
    mi_heap_stat_increase(heap, malloc_requested, size);
    #endif
    mi_heap_stat_counter_increase(heap, malloc_guarded_count, 1);
  }
  #if MI_DEBUG>3
  if (p != NULL && zero) {
    mi_assert_expensive(mi_mem_is_zero(p, size));
  }
  #endif
  return p;
}
#endif

// ------------------------------------------------------
// ensure explicit external inline definitions are emitted!
// ------------------------------------------------------

#ifdef __cplusplus
void* _mi_externs[] = {
  (void*)&_mi_page_malloc,
  (void*)&_mi_page_malloc_zero,
  (void*)&_mi_heap_malloc_zero,
  (void*)&_mi_heap_malloc_zero_ex,
  (void*)&mi_malloc,
  (void*)&mi_malloc_small,
  (void*)&mi_zalloc_small,
  (void*)&mi_heap_malloc,
  (void*)&mi_heap_zalloc,
  (void*)&mi_heap_malloc_small
  // (void*)&mi_heap_alloc_new,
  // (void*)&mi_heap_alloc_new_n
};
#endif

/* ----------------------------------------------------------------------------
Copyright (c) 2018-2021, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

#include "mimalloc.h"
#include "mimalloc/internal.h"
#include "mimalloc/prim.h"  // mi_prim_get_default_heap

#include <string.h>     // memset

// ------------------------------------------------------
// Aligned Allocation
// ------------------------------------------------------

static bool mi_malloc_is_naturally_aligned( size_t size, size_t alignment ) {
  // objects up to `MI_PAGE_MIN_BLOCK_ALIGN` are always allocated aligned to their size
  mi_assert_internal(_mi_is_power_of_two(alignment) && (alignment > 0));
  if (alignment > size) return false;
  const size_t bsize = mi_good_size(size);
  const bool ok = (bsize <= MI_PAGE_MAX_START_BLOCK_ALIGN2 && _mi_is_power_of_two(bsize));
  if (ok) { mi_assert_internal((bsize & (alignment-1)) == 0); } // since both power of 2 and alignment <= size
  return ok;
}

#if MI_GUARDED
static mi_decl_restrict void* mi_heap_malloc_guarded_aligned(mi_heap_t* heap, size_t size, size_t alignment, bool zero) mi_attr_noexcept {
  // use over allocation for guarded blocksl
  mi_assert_internal(alignment > 0 && alignment < MI_PAGE_MAX_OVERALLOC_ALIGN);
  const size_t oversize = size + alignment - 1;
  void* base = _mi_heap_malloc_guarded(heap, oversize, zero);
  void* p = _mi_align_up_ptr(base, alignment);
  mi_track_align(base, p, (uint8_t*)p - (uint8_t*)base, size);
  mi_assert_internal(mi_usable_size(p) >= size);
  mi_assert_internal(_mi_is_aligned(p, alignment));
  return p;
}

static void* mi_heap_malloc_zero_no_guarded(mi_heap_t* heap, size_t size, bool zero) {
  const size_t rate = heap->guarded_sample_rate;
  // only write if `rate!=0` so we don't write to the constant `_mi_heap_empty`
  if (rate != 0) { heap->guarded_sample_rate = 0; }
  void* p = _mi_heap_malloc_zero(heap, size, zero);
  if (rate != 0) { heap->guarded_sample_rate = rate; }
  return p;
}
#else
static void* mi_heap_malloc_zero_no_guarded(mi_heap_t* heap, size_t size, bool zero) {
  return _mi_heap_malloc_zero(heap, size, zero);
}
#endif

// Fallback aligned allocation that over-allocates -- split out for better codegen
static mi_decl_noinline void* mi_heap_malloc_zero_aligned_at_overalloc(mi_heap_t* const heap, const size_t size, const size_t alignment, const size_t offset, const bool zero) mi_attr_noexcept
{
  mi_assert_internal(size <= (MI_MAX_ALLOC_SIZE - MI_PADDING_SIZE));
  mi_assert_internal(alignment != 0 && _mi_is_power_of_two(alignment));

  void* p;
  size_t oversize;
  if mi_unlikely(alignment > MI_PAGE_MAX_OVERALLOC_ALIGN) {
    // use OS allocation for large alignments and allocate inside a singleton page (not in an arena)
    // This can support alignments >= MI_PAGE_ALIGN by ensuring the object can be aligned
    // in the first (and single) page such that the page info is `MI_PAGE_ALIGN` bytes before it (and can be found in the _mi_page_map).
    if mi_unlikely(offset != 0) {
      // todo: cannot support offset alignment for very large alignments yet
      #if MI_DEBUG > 0
      _mi_error_message(EOVERFLOW, "aligned allocation with a large alignment cannot be used with an alignment offset (size %zu, alignment %zu, offset %zu)\n", size, alignment, offset);
      #endif
      return NULL;
    }
    oversize = (size <= MI_SMALL_SIZE_MAX ? MI_SMALL_SIZE_MAX + 1 /* ensure we use generic malloc path */ : size);
    // note: no guarded as alignment > 0
    p = _mi_heap_malloc_zero_ex(heap, oversize, zero, alignment); // the page block size should be large enough to align in the single huge page block
    if (p == NULL) return NULL;
  }
  else {
    // otherwise over-allocate
    oversize = (size < MI_MAX_ALIGN_SIZE ? MI_MAX_ALIGN_SIZE : size) + alignment - 1;  // adjust for size <= 16; with size 0 and aligment 64k, we would allocate a 64k block and pointing just beyond that.
    p = mi_heap_malloc_zero_no_guarded(heap, oversize, zero);
    if (p == NULL) return NULL;
  }
  
  // .. and align within the allocation
  const uintptr_t align_mask = alignment - 1;  // for any x, `(x & align_mask) == (x % alignment)`
  const uintptr_t poffset = ((uintptr_t)p + offset) & align_mask;
  const uintptr_t adjust  = (poffset == 0 ? 0 : alignment - poffset);
  mi_assert_internal(adjust < alignment);
  void* aligned_p = (void*)((uintptr_t)p + adjust);

  // note: after the above allocation, the page may be abandoned now (as it became full, see `page.c:_mi_malloc_generic`)
  // and we no longer own it. We should be careful to only read constant fields in the page, 
  // or use safe atomic access as in `mi_page_set_has_aligned`.
  // (we can access the page though since the just allocated pointer keeps it alive)
  mi_page_t* page = _mi_ptr_page(p);
  if (aligned_p != p) {
    mi_page_set_has_aligned(page, true);
    #if MI_GUARDED
    // set tag to aligned so mi_usable_size works with guard pages
    if (adjust >= sizeof(mi_block_t)) {
      mi_block_t* const block = (mi_block_t*)p;
      block->next = MI_BLOCK_TAG_ALIGNED;
    }
    #endif
    _mi_padding_shrink(page, (mi_block_t*)p, adjust + size);
  }
  // todo: expand padding if overallocated ?

  mi_assert_internal(mi_page_usable_block_size(page) >= adjust + size);
  mi_assert_internal(((uintptr_t)aligned_p + offset) % alignment == 0);
  mi_assert_internal(mi_usable_size(aligned_p)>=size);
  mi_assert_internal(mi_usable_size(p) == mi_usable_size(aligned_p)+adjust);
  #if MI_DEBUG > 1
  mi_page_t* const apage = _mi_ptr_page(aligned_p);
  void* unalign_p = _mi_page_ptr_unalign(apage, aligned_p);
  mi_assert_internal(p == unalign_p);
  #endif

  // now zero the block if needed
  //if (alignment > MI_PAGE_MAX_OVERALLOC_ALIGN) {
  //  // for the tracker, on huge aligned allocations only from the start of the large block is defined
  //  mi_track_mem_undefined(aligned_p, size);
  //  if (zero) {
  //    _mi_memzero_aligned(aligned_p, mi_usable_size(aligned_p));
  //  }
  //}

  if (p != aligned_p) {
    mi_track_align(p,aligned_p,adjust,mi_usable_size(aligned_p));
    #if MI_GUARDED
    mi_track_mem_defined(p, sizeof(mi_block_t));
    #endif
  }
  return aligned_p;
}

// Generic primitive aligned allocation -- split out for better codegen
static mi_decl_noinline void* mi_heap_malloc_zero_aligned_at_generic(mi_heap_t* const heap, const size_t size, const size_t alignment, const size_t offset, const bool zero) mi_attr_noexcept
{
  mi_assert_internal(alignment != 0 && _mi_is_power_of_two(alignment));
  // we don't allocate more than MI_MAX_ALLOC_SIZE (see <https://sourceware.org/ml/libc-announce/2019/msg00001.html>)
  if mi_unlikely(size > (MI_MAX_ALLOC_SIZE - MI_PADDING_SIZE)) {
    #if MI_DEBUG > 0
    _mi_error_message(EOVERFLOW, "aligned allocation request is too large (size %zu, alignment %zu)\n", size, alignment);
    #endif
    return NULL;
  }

  // use regular allocation if it is guaranteed to fit the alignment constraints.
  // this is important to try as the fast path in `mi_heap_malloc_zero_aligned` only works when there exist
  // a page with the right block size, and if we always use the over-alloc fallback that would never happen.
  if (offset == 0 && mi_malloc_is_naturally_aligned(size,alignment)) {
    void* p = mi_heap_malloc_zero_no_guarded(heap, size, zero);
    mi_assert_internal(p == NULL || ((uintptr_t)p % alignment) == 0);
    const bool is_aligned_or_null = (((uintptr_t)p) & (alignment-1))==0;
    if mi_likely(is_aligned_or_null) {
      return p;
    }
    else {
      // this should never happen if the `mi_malloc_is_naturally_aligned` check is correct..
      mi_assert(false);
      mi_free(p);
    }
  }

  // fall back to over-allocation
  return mi_heap_malloc_zero_aligned_at_overalloc(heap,size,alignment,offset,zero);
}


// Primitive aligned allocation
static void* mi_heap_malloc_zero_aligned_at(mi_heap_t* const heap, const size_t size, const size_t alignment, const size_t offset, const bool zero) mi_attr_noexcept
{
  // note: we don't require `size > offset`, we just guarantee that the address at offset is aligned regardless of the allocated size.
  if mi_unlikely(alignment == 0 || !_mi_is_power_of_two(alignment)) { // require power-of-two (see <https://en.cppreference.com/w/c/memory/aligned_alloc>)
    #if MI_DEBUG > 0
    _mi_error_message(EOVERFLOW, "aligned allocation requires the alignment to be a power-of-two (size %zu, alignment %zu)\n", size, alignment);
    #endif
    return NULL;
  }

  #if MI_GUARDED
  if (offset==0 && alignment < MI_PAGE_MAX_OVERALLOC_ALIGN && mi_heap_malloc_use_guarded(heap,size)) {
    return mi_heap_malloc_guarded_aligned(heap, size, alignment, zero);
  }
  #endif

  // try first if there happens to be a small block available with just the right alignment
  // since most small power-of-2 blocks (under MI_PAGE_MAX_BLOCK_START_ALIGN2) are already
  // naturally aligned this can be often the case.
  if mi_likely(size <= MI_SMALL_SIZE_MAX && alignment <= size) {
    const uintptr_t align_mask = alignment-1;       // for any x, `(x & align_mask) == (x % alignment)`
    const size_t padsize = size + MI_PADDING_SIZE;
    mi_page_t* page = _mi_heap_get_free_small_page(heap, padsize);
    if mi_likely(page->free != NULL) {
      const bool is_aligned = (((uintptr_t)page->free + offset) & align_mask)==0;
      if mi_likely(is_aligned)
      {
        void* p = (zero ? _mi_page_malloc_zeroed(heap,page,padsize) : _mi_page_malloc(heap,page,padsize)); // call specific page malloc for better codegen
        mi_assert_internal(p != NULL);
        mi_assert_internal(((uintptr_t)p + offset) % alignment == 0);
        mi_track_malloc(p,size,zero);
        return p;
      }
    }
  }

  // fallback to generic aligned allocation
  return mi_heap_malloc_zero_aligned_at_generic(heap, size, alignment, offset, zero);
}


// ------------------------------------------------------
// Optimized mi_heap_malloc_aligned / mi_malloc_aligned
// ------------------------------------------------------

mi_decl_nodiscard mi_decl_restrict void* mi_heap_malloc_aligned_at(mi_heap_t* heap, size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_malloc_zero_aligned_at(heap, size, alignment, offset, false);
}

mi_decl_nodiscard mi_decl_restrict void* mi_heap_malloc_aligned(mi_heap_t* heap, size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_malloc_aligned_at(heap, size, alignment, 0);
}

// ------------------------------------------------------
// Aligned Allocation
// ------------------------------------------------------

mi_decl_nodiscard mi_decl_restrict void* mi_heap_zalloc_aligned_at(mi_heap_t* heap, size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_malloc_zero_aligned_at(heap, size, alignment, offset, true);
}

mi_decl_nodiscard mi_decl_restrict void* mi_heap_zalloc_aligned(mi_heap_t* heap, size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_zalloc_aligned_at(heap, size, alignment, 0);
}

mi_decl_nodiscard mi_decl_restrict void* mi_heap_calloc_aligned_at(mi_heap_t* heap, size_t count, size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(count, size, &total)) return NULL;
  return mi_heap_zalloc_aligned_at(heap, total, alignment, offset);
}

mi_decl_nodiscard mi_decl_restrict void* mi_heap_calloc_aligned(mi_heap_t* heap, size_t count, size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_calloc_aligned_at(heap,count,size,alignment,0);
}

mi_decl_nodiscard mi_decl_restrict void* mi_malloc_aligned_at(size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_malloc_aligned_at(mi_prim_get_default_heap(), size, alignment, offset);
}

mi_decl_nodiscard mi_decl_restrict void* mi_malloc_aligned(size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_malloc_aligned(mi_prim_get_default_heap(), size, alignment);
}

mi_decl_nodiscard mi_decl_restrict void* mi_zalloc_aligned_at(size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_zalloc_aligned_at(mi_prim_get_default_heap(), size, alignment, offset);
}

mi_decl_nodiscard mi_decl_restrict void* mi_zalloc_aligned(size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_zalloc_aligned(mi_prim_get_default_heap(), size, alignment);
}

mi_decl_nodiscard mi_decl_restrict void* mi_calloc_aligned_at(size_t count, size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_calloc_aligned_at(mi_prim_get_default_heap(), count, size, alignment, offset);
}

mi_decl_nodiscard mi_decl_restrict void* mi_calloc_aligned(size_t count, size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_calloc_aligned(mi_prim_get_default_heap(), count, size, alignment);
}


// ------------------------------------------------------
// Aligned re-allocation
// ------------------------------------------------------

static void* mi_heap_realloc_zero_aligned_at(mi_heap_t* heap, void* p, size_t newsize, size_t alignment, size_t offset, bool zero) mi_attr_noexcept {
  mi_assert(alignment > 0);
  if (alignment <= sizeof(uintptr_t)) return _mi_heap_realloc_zero(heap,p,newsize,zero);
  if (p == NULL) return mi_heap_malloc_zero_aligned_at(heap,newsize,alignment,offset,zero);
  size_t size = mi_usable_size(p);
  if (newsize <= size && newsize >= (size - (size / 2))
      && (((uintptr_t)p + offset) % alignment) == 0) {
    return p;  // reallocation still fits, is aligned and not more than 50% waste
  }
  else {
    // note: we don't zero allocate upfront so we only zero initialize the expanded part
    void* newp = mi_heap_malloc_aligned_at(heap,newsize,alignment,offset);
    if (newp != NULL) {
      if (zero && newsize > size) {
        // also set last word in the previous allocation to zero to ensure any padding is zero-initialized
        size_t start = (size >= sizeof(intptr_t) ? size - sizeof(intptr_t) : 0);
        _mi_memzero((uint8_t*)newp + start, newsize - start);
      }
      _mi_memcpy_aligned(newp, p, (newsize > size ? size : newsize));
      mi_free(p); // only free if successful
    }
    return newp;
  }
}

static void* mi_heap_realloc_zero_aligned(mi_heap_t* heap, void* p, size_t newsize, size_t alignment, bool zero) mi_attr_noexcept {
  mi_assert(alignment > 0);
  if (alignment <= sizeof(uintptr_t)) return _mi_heap_realloc_zero(heap,p,newsize,zero);
  size_t offset = ((uintptr_t)p % alignment); // use offset of previous allocation (p can be NULL)
  return mi_heap_realloc_zero_aligned_at(heap,p,newsize,alignment,offset,zero);
}

mi_decl_nodiscard void* mi_heap_realloc_aligned_at(mi_heap_t* heap, void* p, size_t newsize, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_realloc_zero_aligned_at(heap,p,newsize,alignment,offset,false);
}

mi_decl_nodiscard void* mi_heap_realloc_aligned(mi_heap_t* heap, void* p, size_t newsize, size_t alignment) mi_attr_noexcept {
  return mi_heap_realloc_zero_aligned(heap,p,newsize,alignment,false);
}

mi_decl_nodiscard void* mi_heap_rezalloc_aligned_at(mi_heap_t* heap, void* p, size_t newsize, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_realloc_zero_aligned_at(heap, p, newsize, alignment, offset, true);
}

mi_decl_nodiscard void* mi_heap_rezalloc_aligned(mi_heap_t* heap, void* p, size_t newsize, size_t alignment) mi_attr_noexcept {
  return mi_heap_realloc_zero_aligned(heap, p, newsize, alignment, true);
}

mi_decl_nodiscard void* mi_heap_recalloc_aligned_at(mi_heap_t* heap, void* p, size_t newcount, size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(newcount, size, &total)) return NULL;
  return mi_heap_rezalloc_aligned_at(heap, p, total, alignment, offset);
}

mi_decl_nodiscard void* mi_heap_recalloc_aligned(mi_heap_t* heap, void* p, size_t newcount, size_t size, size_t alignment) mi_attr_noexcept {
  size_t total;
  if (mi_count_size_overflow(newcount, size, &total)) return NULL;
  return mi_heap_rezalloc_aligned(heap, p, total, alignment);
}

mi_decl_nodiscard void* mi_realloc_aligned_at(void* p, size_t newsize, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_realloc_aligned_at(mi_prim_get_default_heap(), p, newsize, alignment, offset);
}

mi_decl_nodiscard void* mi_realloc_aligned(void* p, size_t newsize, size_t alignment) mi_attr_noexcept {
  return mi_heap_realloc_aligned(mi_prim_get_default_heap(), p, newsize, alignment);
}

mi_decl_nodiscard void* mi_rezalloc_aligned_at(void* p, size_t newsize, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_rezalloc_aligned_at(mi_prim_get_default_heap(), p, newsize, alignment, offset);
}

mi_decl_nodiscard void* mi_rezalloc_aligned(void* p, size_t newsize, size_t alignment) mi_attr_noexcept {
  return mi_heap_rezalloc_aligned(mi_prim_get_default_heap(), p, newsize, alignment);
}

mi_decl_nodiscard void* mi_recalloc_aligned_at(void* p, size_t newcount, size_t size, size_t alignment, size_t offset) mi_attr_noexcept {
  return mi_heap_recalloc_aligned_at(mi_prim_get_default_heap(), p, newcount, size, alignment, offset);
}

mi_decl_nodiscard void* mi_recalloc_aligned(void* p, size_t newcount, size_t size, size_t alignment) mi_attr_noexcept {
  return mi_heap_recalloc_aligned(mi_prim_get_default_heap(), p, newcount, size, alignment);
}

/* ----------------------------------------------------------------------------
Copyright (c) 2018-2021, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

// ------------------------------------------------------------------------
// mi prefixed publi definitions of various Posix, Unix, and C++ functions
// for convenience and used when overriding these functions.
// ------------------------------------------------------------------------
#include "mimalloc.h"
#include "mimalloc/internal.h"

// ------------------------------------------------------
// Posix & Unix functions definitions
// ------------------------------------------------------

#include <errno.h>
#include <string.h>  // memset
#include <stdlib.h>  // getenv

#ifdef _MSC_VER
#pragma warning(disable:4996)  // getenv _wgetenv
#endif

#ifndef EINVAL
#define EINVAL 22
#endif
#ifndef ENOMEM
#define ENOMEM 12
#endif


mi_decl_nodiscard size_t mi_malloc_size(const void* p) mi_attr_noexcept {
  // if (!mi_is_in_heap_region(p)) return 0;
  return mi_usable_size(p);
}

mi_decl_nodiscard size_t mi_malloc_usable_size(const void *p) mi_attr_noexcept {
  // if (!mi_is_in_heap_region(p)) return 0;
  return mi_usable_size(p);
}

mi_decl_nodiscard size_t mi_malloc_good_size(size_t size) mi_attr_noexcept {
  return mi_good_size(size);
}

void mi_cfree(void* p) mi_attr_noexcept {
  if (mi_is_in_heap_region(p)) {
    mi_free(p);
  }
}

int mi_posix_memalign(void** p, size_t alignment, size_t size) mi_attr_noexcept {
  // Note: The spec dictates we should not modify `*p` on an error. (issue#27)
  // <http://man7.org/linux/man-pages/man3/posix_memalign.3.html>
  if (p == NULL) return EINVAL;
  if ((alignment % sizeof(void*)) != 0) return EINVAL;                 // natural alignment
  // it is also required that alignment is a power of 2 and > 0; this is checked in `mi_malloc_aligned`
  if (alignment==0 || !_mi_is_power_of_two(alignment)) return EINVAL;  // not a power of 2
  void* q = mi_malloc_aligned(size, alignment);
  if (q==NULL && size != 0) return ENOMEM;
  mi_assert_internal(((uintptr_t)q % alignment) == 0);
  *p = q;
  return 0;
}

mi_decl_nodiscard mi_decl_restrict void* mi_memalign(size_t alignment, size_t size) mi_attr_noexcept {
  void* p = mi_malloc_aligned(size, alignment);
  mi_assert_internal(((uintptr_t)p % alignment) == 0);
  return p;
}

mi_decl_nodiscard mi_decl_restrict void* mi_valloc(size_t size) mi_attr_noexcept {
  return mi_memalign( _mi_os_page_size(), size );
}

mi_decl_nodiscard mi_decl_restrict void* mi_pvalloc(size_t size) mi_attr_noexcept {
  size_t psize = _mi_os_page_size();
  if (size >= SIZE_MAX - psize) return NULL; // overflow
  size_t asize = _mi_align_up(size, psize);
  return mi_malloc_aligned(asize, psize);
}

mi_decl_nodiscard mi_decl_restrict void* mi_aligned_alloc(size_t alignment, size_t size) mi_attr_noexcept {
  // C11 requires the size to be an integral multiple of the alignment, see <https://en.cppreference.com/w/c/memory/aligned_alloc>.
  // unfortunately, it turns out quite some programs pass a size that is not an integral multiple so skip this check..
  /* if mi_unlikely((size & (alignment - 1)) != 0) { // C11 requires alignment>0 && integral multiple, see <https://en.cppreference.com/w/c/memory/aligned_alloc>
      #if MI_DEBUG > 0
      _mi_error_message(EOVERFLOW, "(mi_)aligned_alloc requires the size to be an integral multiple of the alignment (size %zu, alignment %zu)\n", size, alignment);
      #endif
      return NULL;
    }
  */
  // C11 also requires alignment to be a power-of-two (and > 0) which is checked in mi_malloc_aligned
  void* p = mi_malloc_aligned(size, alignment);
  mi_assert_internal(((uintptr_t)p % alignment) == 0);
  return p;
}

mi_decl_nodiscard void* mi_reallocarray( void* p, size_t count, size_t size ) mi_attr_noexcept {  // BSD
  void* newp = mi_reallocn(p,count,size);
  if (newp==NULL) { errno = ENOMEM; }
  return newp;
}

mi_decl_nodiscard int mi_reallocarr( void* p, size_t count, size_t size ) mi_attr_noexcept { // NetBSD
  mi_assert(p != NULL);
  if (p == NULL) {
    errno = EINVAL;
    return EINVAL;
  }
  void** op = (void**)p;
  void* newp = mi_reallocarray(*op, count, size);
  if mi_unlikely(newp == NULL) { return errno; }
  *op = newp;
  return 0;
}

void* mi__expand(void* p, size_t newsize) mi_attr_noexcept {  // Microsoft
  void* res = mi_expand(p, newsize);
  if (res == NULL) { errno = ENOMEM; }
  return res;
}

mi_decl_nodiscard mi_decl_restrict unsigned short* mi_wcsdup(const unsigned short* s) mi_attr_noexcept {
  if (s==NULL) return NULL;
  size_t len;
  for(len = 0; s[len] != 0; len++) { }
  size_t size = (len+1)*sizeof(unsigned short);
  unsigned short* p = (unsigned short*)mi_malloc(size);
  if (p != NULL) {
    _mi_memcpy(p,s,size);
  }
  return p;
}

mi_decl_nodiscard mi_decl_restrict unsigned char* mi_mbsdup(const unsigned char* s)  mi_attr_noexcept {
  return (unsigned char*)mi_strdup((const char*)s);
}

int mi_dupenv_s(char** buf, size_t* size, const char* name) mi_attr_noexcept {
  if (buf==NULL || name==NULL) return EINVAL;
  if (size != NULL) *size = 0;
  char* p = getenv(name);        // mscver warning 4996
  if (p==NULL) {
    *buf = NULL;
  }
  else {
    *buf = mi_strdup(p);
    if (*buf==NULL) return ENOMEM;
    if (size != NULL) *size = _mi_strlen(p);
  }
  return 0;
}

int mi_wdupenv_s(unsigned short** buf, size_t* size, const unsigned short* name) mi_attr_noexcept {
  if (buf==NULL || name==NULL) return EINVAL;
  if (size != NULL) *size = 0;
#if !defined(_WIN32) || (defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP))
  // not supported
  *buf = NULL;
  return EINVAL;
#else
  unsigned short* p = (unsigned short*)_wgetenv((const wchar_t*)name);  // msvc warning 4996
  if (p==NULL) {
    *buf = NULL;
  }
  else {
    *buf = mi_wcsdup(p);
    if (*buf==NULL) return ENOMEM;
    if (size != NULL) *size = wcslen((const wchar_t*)p);
  }
  return 0;
#endif
}

mi_decl_nodiscard void* mi_aligned_offset_recalloc(void* p, size_t newcount, size_t size, size_t alignment, size_t offset) mi_attr_noexcept { // Microsoft
  return mi_recalloc_aligned_at(p, newcount, size, alignment, offset);
}

mi_decl_nodiscard void* mi_aligned_recalloc(void* p, size_t newcount, size_t size, size_t alignment) mi_attr_noexcept { // Microsoft
  return mi_recalloc_aligned(p, newcount, size, alignment);
}
/* ----------------------------------------------------------------------------
Copyright (c) 2019-2024, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
"Arenas" are fixed area's of OS memory from which we can allocate
large blocks (>= MI_ARENA_MIN_BLOCK_SIZE, 4MiB).
In contrast to the rest of mimalloc, the arenas are shared between
threads and need to be accessed using atomic operations.

Arenas are also used to for huge OS page (1GiB) reservations or for reserving
OS memory upfront which can be improve performance or is sometimes needed
on embedded devices. We can also employ this with WASI or `sbrk` systems
to reserve large arenas upfront and be able to reuse the memory more effectively.

The arena allocation needs to be thread safe and we use an atomic bitmap to allocate.
-----------------------------------------------------------------------------*/

#include "mimalloc.h"
#include "mimalloc/internal.h"
#include "bitmap.h"


/* -----------------------------------------------------------
  Arena allocation
----------------------------------------------------------- */

#define MI_ARENA_BIN_COUNT      (MI_BIN_COUNT)
#define MI_ARENA_MIN_SIZE       (MI_BCHUNK_BITS * MI_ARENA_SLICE_SIZE)           // 32 MiB (or 8 MiB on 32-bit)
#define MI_ARENA_MAX_SIZE       (MI_BITMAP_MAX_BIT_COUNT * MI_ARENA_SLICE_SIZE)

// A memory arena descriptor
typedef struct mi_arena_s {
  mi_memid_t          memid;                // memid of the memory area
  mi_subproc_t*       subproc;              // subprocess this arena belongs to (`this 'in' this->subproc->arenas`)

  size_t              slice_count;          // total size of the area in arena slices (of `MI_ARENA_SLICE_SIZE`)
  size_t              info_slices;          // initial slices reserved for the arena bitmaps
  int                 numa_node;            // associated NUMA node
  bool                is_exclusive;         // only allow allocations if specifically for this arena
  _Atomic(mi_msecs_t) purge_expire;         // expiration time when slices can be purged from `slices_purge`.

  mi_bbitmap_t*       slices_free;          // is the slice free? (a binned bitmap with size classes)
  mi_bitmap_t*        slices_committed;     // is the slice committed? (i.e. accessible)
  mi_bitmap_t*        slices_dirty;         // is the slice potentially non-zero?
  mi_bitmap_t*        slices_purge;         // slices that can be purged
  mi_bitmap_t*        pages;                // all registered pages (abandoned and owned)
  mi_bitmap_t*        pages_abandoned[MI_BIN_COUNT];  // abandoned pages per size bin (a set bit means the start of the page)
                                            // the full queue contains abandoned full pages
  // followed by the bitmaps (whose sizes depend on the arena size)
  // note: when adding bitmaps revise `mi_arena_info_slices_needed`
} mi_arena_t;


/* -----------------------------------------------------------
  Arena id's
----------------------------------------------------------- */

mi_arena_id_t _mi_arena_id_none(void) {
  return NULL;
}

mi_arena_t* _mi_arena_from_id(mi_arena_id_t id) {
  return (mi_arena_t*)id;
}


static bool mi_arena_id_is_suitable(mi_arena_t* arena, mi_arena_t* req_arena) {
  return ((arena == req_arena) ||                        // they match,
          (req_arena == NULL && !arena->is_exclusive));  // or the arena is not exclusive, and we didn't request a specific one
}

bool _mi_arena_memid_is_suitable(mi_memid_t memid, mi_arena_t* request_arena) {
  if (memid.memkind == MI_MEM_ARENA) {
    return mi_arena_id_is_suitable(memid.mem.arena.arena, request_arena);
  }
  else {
    return mi_arena_id_is_suitable(NULL, request_arena);
  }
}

size_t mi_arenas_get_count(mi_subproc_t* subproc) {
  return mi_atomic_load_relaxed(&subproc->arena_count);
}

mi_arena_t* mi_arena_from_index(mi_subproc_t* subproc, size_t idx) {
  mi_assert_internal(idx < mi_arenas_get_count(subproc));
  return mi_atomic_load_ptr_relaxed(mi_arena_t, &subproc->arenas[idx]);
}

static size_t mi_arena_info_slices(mi_arena_t* arena) {
  return arena->info_slices;
}

#if MI_DEBUG > 1
static bool mi_arena_has_page(mi_arena_t* arena, mi_page_t* page) {
  return (page->memid.memkind == MI_MEM_ARENA &&
          page->memid.mem.arena.arena == arena &&
          mi_bitmap_is_setN(arena->pages, page->memid.mem.arena.slice_index, 1));
}
#endif

/* -----------------------------------------------------------
  Util
----------------------------------------------------------- */


// Size of an arena
static size_t mi_arena_size(mi_arena_t* arena) {
  return mi_size_of_slices(arena->slice_count);
}

// Start of the arena memory area
static uint8_t* mi_arena_start(mi_arena_t* arena) {
  return ((uint8_t*)arena);
}

// Start of a slice
uint8_t* mi_arena_slice_start(mi_arena_t* arena, size_t slice_index) {
  return (mi_arena_start(arena) + mi_size_of_slices(slice_index));
}

// Arena area
void* mi_arena_area(mi_arena_id_t arena_id, size_t* size) {
  if (size != NULL) *size = 0;
  mi_arena_t* arena = _mi_arena_from_id(arena_id);
  if (arena == NULL) return NULL;
  if (size != NULL) { *size = mi_size_of_slices(arena->slice_count); }
  return mi_arena_start(arena);
}


// Create an arena memid
static mi_memid_t mi_memid_create_arena(mi_arena_t* arena, size_t slice_index, size_t slice_count) {
  mi_assert_internal(slice_index < UINT32_MAX);
  mi_assert_internal(slice_count < UINT32_MAX);
  mi_assert_internal(slice_count > 0);
  mi_assert_internal(slice_index < arena->slice_count);
  mi_memid_t memid = _mi_memid_create(MI_MEM_ARENA);
  memid.mem.arena.arena = arena;
  memid.mem.arena.slice_index = (uint32_t)slice_index;
  memid.mem.arena.slice_count = (uint32_t)slice_count;
  return memid;
}

// get the arena and slice span
static mi_arena_t* mi_arena_from_memid(mi_memid_t memid, size_t* slice_index, size_t* slice_count) {
  mi_assert_internal(memid.memkind == MI_MEM_ARENA);
  mi_arena_t* arena = memid.mem.arena.arena;
  if (slice_index) *slice_index = memid.mem.arena.slice_index;
  if (slice_count) *slice_count = memid.mem.arena.slice_count;
  return arena;
}

static mi_arena_t* mi_page_arena(mi_page_t* page, size_t* slice_index, size_t* slice_count) {
  // todo: maybe store the arena* directly in the page?
  return mi_arena_from_memid(page->memid, slice_index, slice_count);
}

static size_t mi_page_full_size(mi_page_t* page) {  
  if (page->memid.memkind == MI_MEM_ARENA) {
    return page->memid.mem.arena.slice_count * MI_ARENA_SLICE_SIZE;
  }
  else if (mi_memid_is_os(page->memid) || page->memid.memkind == MI_MEM_EXTERNAL) {
    mi_assert_internal((uint8_t*)page->memid.mem.os.base <= (uint8_t*)page);
    const ptrdiff_t presize = (uint8_t*)page - (uint8_t*)page->memid.mem.os.base;
    mi_assert_internal((ptrdiff_t)page->memid.mem.os.size >= presize);
    return (presize > (ptrdiff_t)page->memid.mem.os.size ? 0 : page->memid.mem.os.size - presize);
  }
  else {
    return 0;
  }
}

/* -----------------------------------------------------------
  Arena Allocation
----------------------------------------------------------- */

static mi_decl_noinline void* mi_arena_try_alloc_at(
  mi_arena_t* arena, size_t slice_count, bool commit, size_t tseq, mi_memid_t* memid)
{
  size_t slice_index;
  if (!mi_bbitmap_try_find_and_clearN(arena->slices_free, slice_count, tseq, &slice_index)) return NULL;

  // claimed it!
  void* p = mi_arena_slice_start(arena, slice_index);
  *memid = mi_memid_create_arena(arena, slice_index, slice_count);
  memid->is_pinned = arena->memid.is_pinned;

  // set the dirty bits and track which slices become accessible
  size_t touched_slices = slice_count;
  if (arena->memid.initially_zero) {
    size_t already_dirty = 0;
    memid->initially_zero = mi_bitmap_setN(arena->slices_dirty, slice_index, slice_count, &already_dirty);
    mi_assert_internal(already_dirty <= touched_slices);
    touched_slices -= already_dirty;
  }

  // set commit state
  if (commit) {        
    // commit requested, but the range may not be committed as a whole: ensure it is committed now
    const size_t already_committed = mi_bitmap_popcountN(arena->slices_committed, slice_index, slice_count);
    if (already_committed < slice_count) {  
      // not all committed, try to commit now
      bool commit_zero = false;
      if (!_mi_os_commit_ex(p, mi_size_of_slices(slice_count), &commit_zero, mi_size_of_slices(slice_count - already_committed))) {
        // if the commit fails, release ownership, and return NULL;
        // note: this does not roll back dirty bits but that is ok.
        mi_bbitmap_setN(arena->slices_free, slice_index, slice_count);
        return NULL;
      }
      if (commit_zero) { 
        memid->initially_zero = true; 
      }
            
      // set the commit bits 
      mi_bitmap_setN(arena->slices_committed, slice_index, slice_count, NULL);      
      
      // committed
      #if MI_DEBUG > 1
      if (memid->initially_zero) {
        if (!mi_mem_is_zero(p, mi_size_of_slices(slice_count))) {
          _mi_error_message(EFAULT, "interal error: arena allocation was not zero-initialized!\n");
          memid->initially_zero = false;
        }
      }
      #endif    
    }
    else {
      // already fully commited.
      // if the OS has overcommit, and this is the first time we access these pages, then
      // count the commit now (as at arena reserve we didn't count those commits as these are on-demand)
      if (_mi_os_has_overcommit() && touched_slices > 0) {
        mi_subproc_stat_increase( arena->subproc, committed, mi_size_of_slices(touched_slices));
      }
    }
    
    mi_assert_internal(mi_bitmap_is_setN(arena->slices_committed, slice_index, slice_count));
    memid->initially_committed = true;
    
    // tool support
    if (memid->initially_zero) {
      mi_track_mem_defined(p, slice_count * MI_ARENA_SLICE_SIZE);
    }
    else {
      mi_track_mem_undefined(p, slice_count * MI_ARENA_SLICE_SIZE);
    }
  }
  else {
    // no need to commit, but check if already fully committed
    // commit requested, but the range may not be committed as a whole: ensure it is committed now
    memid->initially_committed = mi_bitmap_is_setN(arena->slices_committed, slice_index, slice_count);
    if (!memid->initially_committed) {
      // partly committed.. adjust stats
      size_t already_committed_count = 0;
      mi_bitmap_setN(arena->slices_committed, slice_index, slice_count, &already_committed_count);
      mi_bitmap_clearN(arena->slices_committed, slice_index, slice_count);
      mi_os_stat_decrease(committed, mi_size_of_slices(already_committed_count));
    }
  }

  mi_assert_internal(mi_bbitmap_is_clearN(arena->slices_free, slice_index, slice_count));
  if (commit) { mi_assert_internal(mi_bitmap_is_setN(arena->slices_committed, slice_index, slice_count)); }
  mi_assert_internal(mi_bitmap_is_setN(arena->slices_dirty, slice_index, slice_count));

  return p;
}


static int mi_reserve_os_memory_ex2(mi_subproc_t* subproc, size_t size, bool commit, bool allow_large, bool exclusive, mi_arena_id_t* arena_id);

// try to reserve a fresh arena space
static bool mi_arena_reserve(mi_subproc_t* subproc, size_t req_size, bool allow_large, mi_arena_id_t* arena_id)
{
  const size_t arena_count = mi_arenas_get_count(subproc);
  if (arena_count > (MI_MAX_ARENAS - 4)) return false;

  // calc reserve
  size_t arena_reserve = mi_option_get_size(mi_option_arena_reserve);
  if (arena_reserve == 0) return false;

  if (!_mi_os_has_virtual_reserve()) {
    arena_reserve = arena_reserve/4;  // be conservative if virtual reserve is not supported (for WASM for example)
  }
  arena_reserve = _mi_align_up(arena_reserve, MI_ARENA_SLICE_SIZE);

  if (arena_count >= 1 && arena_count <= 128) {
    // scale up the arena sizes exponentially every 4 entries
    const size_t multiplier = (size_t)1 << _mi_clamp(arena_count/4, 0, 16);
    size_t reserve = 0;
    if (!mi_mul_overflow(multiplier, arena_reserve, &reserve)) {
      arena_reserve = reserve;
    }
  }

  // check arena bounds
  const size_t min_reserve = MI_ARENA_MIN_SIZE;
  const size_t max_reserve = MI_ARENA_MAX_SIZE;   // 16 GiB
  if (arena_reserve < min_reserve) {
    arena_reserve = min_reserve;
  }
  else if (arena_reserve > max_reserve) {
    arena_reserve = max_reserve;
  }

  if (arena_reserve < req_size) return false;  // should be able to at least handle the current allocation size

  // commit eagerly?
  bool arena_commit = false;
  const bool overcommit = _mi_os_has_overcommit();
  if (mi_option_get(mi_option_arena_eager_commit) == 2) { arena_commit = overcommit; }
  else if (mi_option_get(mi_option_arena_eager_commit) == 1) { arena_commit = true; }

  // on an OS with overcommit (Linux) we don't count the commit yet as it is on-demand. Once a slice
  // is actually allocated for the first time it will be counted.
  const bool adjust = (overcommit && arena_commit);
  if (adjust) {
    mi_subproc_stat_adjust_decrease( subproc, committed, arena_reserve);
  }
  // and try to reserve the arena
  int err = mi_reserve_os_memory_ex2(subproc, arena_reserve, arena_commit, allow_large, false /* exclusive? */, arena_id);
  if (err != 0) {
    if (adjust) { mi_subproc_stat_adjust_increase( subproc, committed, arena_reserve); } // roll back
    // failed, try a smaller size?
    const size_t small_arena_reserve = (MI_SIZE_BITS == 32 ? 128*MI_MiB : 1*MI_GiB);
    if (adjust) { mi_subproc_stat_adjust_decrease( subproc, committed, arena_reserve); }
    if (arena_reserve > small_arena_reserve) {
      // try again
      err = mi_reserve_os_memory_ex(small_arena_reserve, arena_commit, allow_large, false /* exclusive? */, arena_id);
      if (err != 0 && adjust) { mi_subproc_stat_adjust_increase( subproc, committed, arena_reserve); } // roll back
    }
  }
  return (err==0);
}




/* -----------------------------------------------------------
  Arena iteration
----------------------------------------------------------- */

static inline bool mi_arena_is_suitable(mi_arena_t* arena, mi_arena_t* req_arena, int numa_node, bool allow_pinned) {
  if (!allow_pinned && arena->memid.is_pinned) return false;
  if (!mi_arena_id_is_suitable(arena, req_arena)) return false;
  if (req_arena == NULL) { // if not specific, check numa affinity
    const bool numa_suitable = (numa_node < 0 || arena->numa_node < 0 || arena->numa_node == numa_node);
    if (!numa_suitable) return false;
  }
  return true;
}

#define mi_forall_arenas(subproc, req_arena, tseq, name_arena) { \
  const size_t _arena_count = mi_arenas_get_count(subproc); \
  const size_t _arena_cycle = (_arena_count == 0 ? 0 : _arena_count - 1); /* first search the arenas below the last one */ \
  /* always start searching in the arena's below the max */ \
  size_t _start = (_arena_cycle <= 1 ? 0 : (tseq % _arena_cycle)); \
  for (size_t _i = 0; _i < _arena_count; _i++) { \
    mi_arena_t* name_arena; \
    if (req_arena != NULL) { \
      name_arena = req_arena; /* if there is a specific req_arena, only search that one */\
      if (_i > 0) break;       /* only once */ \
    } \
    else { \
      size_t _idx; \
      if (_i < _arena_cycle) { \
        _idx = _i + _start; \
        if (_idx >= _arena_cycle) { _idx -= _arena_cycle; } /* adjust so we rotate through the cycle */ \
      } \
      else { \
        _idx = _i; /* remaining arena's */ \
      } \
      name_arena = mi_arena_from_index(subproc,_idx); \
    } \
    if (name_arena != NULL) \
    {

#define mi_forall_arenas_end()  \
    } \
  } \
  }

#define mi_forall_suitable_arenas(subproc, req_arena, tseq, allow_large, name_arena) \
  mi_forall_arenas(subproc, req_arena,tseq,name_arena) { \
    if (mi_arena_is_suitable(name_arena, req_arena, -1 /* todo: numa node */, allow_large)) { \

#define mi_forall_suitable_arenas_end() \
  }} \
  mi_forall_arenas_end()

/* -----------------------------------------------------------
  Arena allocation
----------------------------------------------------------- */

// allocate slices from the arenas
static mi_decl_noinline void* mi_arenas_try_find_free(
  mi_subproc_t* subproc, size_t slice_count, size_t alignment,
  bool commit, bool allow_large, mi_arena_t* req_arena, size_t tseq, mi_memid_t* memid)
{
  mi_assert_internal(slice_count <= mi_slice_count_of_size(MI_ARENA_MAX_OBJ_SIZE));
  mi_assert(alignment <= MI_ARENA_SLICE_ALIGN);
  if (alignment > MI_ARENA_SLICE_ALIGN) return NULL;

  // search arena's
  mi_forall_suitable_arenas(subproc, req_arena, tseq, allow_large, arena)
  {
    void* p = mi_arena_try_alloc_at(arena, slice_count, commit, tseq, memid);
    if (p != NULL) return p;
  }
  mi_forall_suitable_arenas_end();
  return NULL;
}

// Allocate slices from the arena's -- potentially allocating a fresh arena
static mi_decl_noinline void* mi_arenas_try_alloc(
  mi_subproc_t* subproc,
  size_t slice_count, size_t alignment,
  bool commit, bool allow_large,
  mi_arena_t* req_arena, size_t tseq, mi_memid_t* memid)
{
  mi_assert(slice_count <= MI_ARENA_MAX_OBJ_SLICES);
  mi_assert(alignment <= MI_ARENA_SLICE_ALIGN);
  void* p;

  // try to find free slices in the arena's
  p = mi_arenas_try_find_free(subproc, slice_count, alignment, commit, allow_large, req_arena, tseq, memid);
  if (p != NULL) return p;

  // did we need a specific arena?
  if (req_arena != NULL) return NULL;

  // don't create arena's while preloading (todo: or should we?)
  if (_mi_preloading()) return NULL;

  // otherwise, try to reserve a new arena -- but one thread at a time.. (todo: allow 2 or 4 to reduce contention?)
  const size_t arena_count = mi_arenas_get_count(subproc);
  mi_lock(&subproc->arena_reserve_lock) {
    if (arena_count == mi_arenas_get_count(subproc)) {
      // we are the first to enter the lock, reserve a fresh arena
      mi_arena_id_t arena_id = 0;
      mi_arena_reserve(subproc, mi_size_of_slices(slice_count), allow_large, &arena_id);
    }
    else {
      // another thread already reserved a new arena
    }
  }
  // try once more to allocate in the new arena
  mi_assert_internal(req_arena == NULL);
  p = mi_arenas_try_find_free(subproc, slice_count, alignment, commit, allow_large, req_arena, tseq, memid);
  if (p != NULL) return p;

  return NULL;
}

// Allocate from the OS (if allowed)
static void* mi_arena_os_alloc_aligned(
  size_t size, size_t alignment, size_t align_offset,
  bool commit, bool allow_large,
  mi_arena_id_t req_arena_id, mi_memid_t* memid)
{
  // if we cannot use OS allocation, return NULL
  if (mi_option_is_enabled(mi_option_disallow_os_alloc) || req_arena_id != _mi_arena_id_none()) {
    errno = ENOMEM;
    return NULL;
  }

  if (align_offset > 0) {
    return _mi_os_alloc_aligned_at_offset(size, alignment, align_offset, commit, allow_large, memid);
  }
  else {
    return _mi_os_alloc_aligned(size, alignment, commit, allow_large, memid);
  }
}


// Allocate large sized memory
void* _mi_arenas_alloc_aligned( mi_subproc_t* subproc,
  size_t size, size_t alignment, size_t align_offset,
  bool commit, bool allow_large,
  mi_arena_t* req_arena, size_t tseq, mi_memid_t* memid)
{
  mi_assert_internal(memid != NULL);
  mi_assert_internal(size > 0);

  // *memid = _mi_memid_none();
  // const int numa_node = _mi_os_numa_node(&tld->os); // current numa node

  // try to allocate in an arena if the alignment is small enough and the object is not too small (as for heap meta data)
  if (!mi_option_is_enabled(mi_option_disallow_arena_alloc) &&           // is arena allocation allowed?
      size >= MI_ARENA_MIN_OBJ_SIZE && size <= MI_ARENA_MAX_OBJ_SIZE &&  // and not too small/large
      alignment <= MI_ARENA_SLICE_ALIGN && align_offset == 0)            // and good alignment
  {
    const size_t slice_count = mi_slice_count_of_size(size);
    void* p = mi_arenas_try_alloc(subproc,slice_count, alignment, commit, allow_large, req_arena, tseq, memid);
    if (p != NULL) return p;
  }

  // fall back to the OS
  void* p = mi_arena_os_alloc_aligned(size, alignment, align_offset, commit, allow_large, req_arena, memid);
  return p;
}

void* _mi_arenas_alloc(mi_subproc_t* subproc, size_t size, bool commit, bool allow_large, mi_arena_t* req_arena, size_t tseq, mi_memid_t* memid)
{
  return _mi_arenas_alloc_aligned(subproc, size, MI_ARENA_SLICE_SIZE, 0, commit, allow_large, req_arena, tseq, memid);
}



/* -----------------------------------------------------------
  Arena page allocation
----------------------------------------------------------- */

static bool mi_arena_try_claim_abandoned(size_t slice_index, mi_arena_t* arena, mi_heaptag_t heap_tag, bool* keep_abandoned) {
  // found an abandoned page of the right size
  mi_page_t* const page  = (mi_page_t*)mi_arena_slice_start(arena, slice_index);
  // can we claim ownership?
  if (!mi_page_try_claim_ownership(page)) {
    // there was a concurrent free ..
    // we need to keep it in the abandoned map as the free will call `mi_arena_page_unabandon`,
    // and wait for readers (us!) to finish. This is why it is very important to set the abandoned
    // bit again (or otherwise the unabandon will never stop waiting).
    *keep_abandoned = true;
    return false;
  }
  if (heap_tag != page->heap_tag) {
    // wrong heap_tag.. we need to unown again
    // note: this normally never happens unless heaptags are actually used.
    // (an unown might free the page, and depending on that we can keep it in the abandoned map or not)
    // note: a minor wrinkle: the page will still be mapped but the abandoned map entry is (temporarily) clear at this point.
    //       so we cannot check in `mi_arenas_free` for this invariant to hold.
    const bool freed = _mi_page_unown(page);
    *keep_abandoned = !freed;
    return false;
  }
  // yes, we can reclaim it, keep the abandoned map entry clear
  *keep_abandoned = false;
  return true;
}

static mi_page_t* mi_arenas_page_try_find_abandoned(mi_subproc_t* subproc, size_t slice_count, size_t block_size, mi_arena_t* req_arena, mi_heaptag_t heaptag, size_t tseq)
{
  MI_UNUSED(slice_count);
  const size_t bin = _mi_bin(block_size);
  mi_assert_internal(bin < MI_BIN_COUNT);

  // any abandoned in our size class?
  mi_assert_internal(subproc != NULL);
  if (mi_atomic_load_relaxed(&subproc->abandoned_count[bin]) == 0) {
    return NULL;
  }

  // search arena's
  const bool allow_large = true;
  mi_forall_suitable_arenas(subproc, req_arena, tseq, allow_large, arena)
  {
    size_t slice_index;
    mi_bitmap_t* const bitmap = arena->pages_abandoned[bin];

    if (mi_bitmap_try_find_and_claim(bitmap, tseq, &slice_index, &mi_arena_try_claim_abandoned, arena, heaptag)) {
      // found an abandoned page of the right size
      // and claimed ownership.
      mi_page_t* page = (mi_page_t*)mi_arena_slice_start(arena, slice_index);
      mi_assert_internal(mi_page_is_owned(page));
      mi_assert_internal(mi_page_is_abandoned(page));
      mi_assert_internal(mi_arena_has_page(arena,page));
      mi_atomic_decrement_relaxed(&subproc->abandoned_count[bin]);
      mi_tld_t* tld = _mi_thread_tld();
      mi_tld_stat_decrease( tld, pages_abandoned, 1);
      mi_tld_stat_counter_increase( tld, pages_reclaim_on_alloc, 1);

      _mi_page_free_collect(page, false);  // update `used` count
      mi_assert_internal(mi_bbitmap_is_clearN(arena->slices_free, slice_index, slice_count));
      mi_assert_internal(page->slice_committed > 0 || mi_bitmap_is_setN(arena->slices_committed, slice_index, slice_count));
      mi_assert_internal(mi_bitmap_is_setN(arena->slices_dirty, slice_index, slice_count));
      mi_assert_internal(_mi_is_aligned(page, MI_PAGE_ALIGN));
      mi_assert_internal(_mi_ptr_page(page)==page);
      mi_assert_internal(_mi_ptr_page(mi_page_start(page))==page);
      mi_assert_internal(mi_page_block_size(page) == block_size);
      mi_assert_internal(!mi_page_is_full(page));
      return page;
    }
  }
  mi_forall_suitable_arenas_end();
  return NULL;
}

// Allocate a fresh page
static mi_page_t* mi_arenas_page_alloc_fresh(size_t slice_count, size_t block_size, size_t block_alignment,
                                            mi_arena_t* req_arena, bool commit, mi_tld_t* tld)
{
  const bool allow_large = (MI_SECURE < 2); // 2 = guard page at end of each arena page
  const bool os_align = (block_alignment > MI_PAGE_MAX_OVERALLOC_ALIGN);
  const size_t page_alignment = MI_ARENA_SLICE_ALIGN;

  // try to allocate from free space in arena's
  mi_memid_t memid = _mi_memid_none();
  mi_page_t* page = NULL;
  const size_t alloc_size = mi_size_of_slices(slice_count);
  if (!mi_option_is_enabled(mi_option_disallow_arena_alloc) && // allowed to allocate from arena's?
      !os_align &&                            // not large alignment
      slice_count <= MI_ARENA_MAX_OBJ_SLICES) // and not too large
  {
    page = (mi_page_t*)mi_arenas_try_alloc(tld->subproc, slice_count, page_alignment, commit, allow_large, req_arena, tld->thread_seq, &memid);
    if (page != NULL) {
      mi_assert_internal(mi_bitmap_is_clearN(memid.mem.arena.arena->pages, memid.mem.arena.slice_index, memid.mem.arena.slice_count));
      mi_bitmap_set(memid.mem.arena.arena->pages, memid.mem.arena.slice_index);
    }
  }

  // otherwise fall back to the OS
  if (page == NULL) {
    if (os_align) {
      // note: slice_count already includes the page
      mi_assert_internal(slice_count >= mi_slice_count_of_size(block_size) + mi_slice_count_of_size(page_alignment));
      page = (mi_page_t*)mi_arena_os_alloc_aligned(alloc_size, block_alignment, page_alignment /* align offset */, commit, allow_large, req_arena, &memid);
    }
    else {
      page = (mi_page_t*)mi_arena_os_alloc_aligned(alloc_size, page_alignment, 0 /* align offset */, commit, allow_large, req_arena, &memid);
    }
  }

  if (page == NULL) return NULL;
  mi_assert_internal(_mi_is_aligned(page, MI_PAGE_ALIGN));
  mi_assert_internal(!os_align || _mi_is_aligned((uint8_t*)page + page_alignment, block_alignment));

  // guard page at the end of mimalloc page?
  #if MI_SECURE < 2
  const size_t page_noguard_size = alloc_size;
  #else
  mi_assert(alloc_size > _mi_os_secure_guard_page_size());
  const size_t page_noguard_size = alloc_size - _mi_os_secure_guard_page_size();
  if (memid.initially_committed) {
    _mi_os_secure_guard_page_set_at((uint8_t*)page + page_noguard_size, memid.is_pinned);
  }
  #endif

  // claimed free slices: initialize the page partly
  if (!memid.initially_zero && memid.initially_committed) {
    mi_track_mem_undefined(page, slice_count * MI_ARENA_SLICE_SIZE);
    _mi_memzero_aligned(page, sizeof(*page));
  }
  else if (memid.initially_committed) {
    mi_track_mem_defined(page, slice_count * MI_ARENA_SLICE_SIZE);
  }
  #if MI_DEBUG > 1
  if (memid.initially_zero && memid.initially_committed) {
    if (!mi_mem_is_zero(page, page_noguard_size)) {
      _mi_error_message(EFAULT, "internal error: page memory was not zero initialized.\n");
      memid.initially_zero = false;
      _mi_memzero_aligned(page, sizeof(*page));
    }
  }
  #endif
  mi_assert(MI_PAGE_INFO_SIZE >= mi_page_info_size());

  size_t block_start;
  #if MI_GUARDED
  // in a guarded build, we align pages with blocks a multiple of an OS page size, to the OS page size
  // this ensures that all blocks in such pages are OS page size aligned (which is needed for the guard pages)
  const size_t os_page_size = _mi_os_page_size();
  mi_assert_internal(MI_PAGE_ALIGN >= os_page_size);
  if (!os_align && block_size % os_page_size == 0 && block_size > os_page_size /* at least 2 or more */ ) {
    block_start = _mi_align_up(mi_page_info_size(), os_page_size);
  }
  else
  #endif
  if (os_align) {
    block_start = MI_PAGE_ALIGN;
  }
  else if (_mi_is_power_of_two(block_size) && block_size <= MI_PAGE_MAX_START_BLOCK_ALIGN2) {
    // naturally align all power-of-2 blocks
    block_start = _mi_align_up(mi_page_info_size(), block_size);
  }
  else {
    // otherwise start after the info
    block_start = mi_page_info_size();
  }
  const size_t reserved    = (os_align ? 1 : (page_noguard_size - block_start) / block_size);
  mi_assert_internal(reserved > 0 && reserved <= UINT16_MAX);

  // commit first block?
  size_t commit_size = 0;
  if (!memid.initially_committed) {
    commit_size = _mi_align_up(block_start + block_size, MI_PAGE_MIN_COMMIT_SIZE);
    if (commit_size > page_noguard_size) { commit_size = page_noguard_size; }
    bool is_zero;
    if mi_unlikely(!_mi_os_commit(page, commit_size, &is_zero)) {
      _mi_arenas_free( page, alloc_size, memid );
      return NULL;
    }
    if (!memid.initially_zero && !is_zero) {
      _mi_memzero_aligned(page, commit_size);
    }
  }

  // initialize
  page->reserved = (uint16_t)reserved;
  page->page_start = (uint8_t*)page + block_start;
  page->block_size = block_size;
  page->slice_committed = commit_size;
  page->memid = memid;
  page->free_is_zero = memid.initially_zero;
  if (block_size > 0 && _mi_is_power_of_two(block_size)) {
    page->block_size_shift = (uint8_t)mi_ctz(block_size);
  }
  else {
    page->block_size_shift = 0;
  }
  // and own it
  mi_page_try_claim_ownership(page);

  // register in the page map
  if mi_unlikely(!_mi_page_map_register(page)) {
    _mi_arenas_free( page, alloc_size, memid );
    return NULL;
  }

  // stats
  mi_tld_stat_increase(tld, pages, 1);
  mi_tld_stat_increase(tld, page_bins[_mi_page_bin(page)], 1);

  mi_assert_internal(_mi_ptr_page(page)==page);
  mi_assert_internal(_mi_ptr_page(mi_page_start(page))==page);
  mi_assert_internal(mi_page_block_size(page) == block_size);
  mi_assert_internal(mi_page_is_abandoned(page));
  mi_assert_internal(mi_page_is_owned(page));
  return page;
}

// Allocate a regular small/medium/large page.
static mi_page_t* mi_arenas_page_regular_alloc(mi_heap_t* heap, size_t slice_count, size_t block_size) {
  mi_arena_t* req_arena = heap->exclusive_arena;
  mi_tld_t* const tld = heap->tld;

  // 1. look for an abandoned page
  mi_page_t* page = mi_arenas_page_try_find_abandoned(tld->subproc, slice_count, block_size, req_arena, heap->tag, tld->thread_seq);
  if (page != NULL) {
    return page;  // return as abandoned
  }

  // 2. find a free block, potentially allocating a new arena
  const long commit_on_demand = mi_option_get(mi_option_page_commit_on_demand);
  const bool commit = (slice_count <= mi_slice_count_of_size(MI_PAGE_MIN_COMMIT_SIZE) ||  // always commit small pages
                       (commit_on_demand == 2 && _mi_os_has_overcommit()) || (commit_on_demand == 0));
  page = mi_arenas_page_alloc_fresh(slice_count, block_size, 1, req_arena, commit, tld);
  if (page != NULL) {
    mi_assert_internal(page->memid.memkind != MI_MEM_ARENA || page->memid.mem.arena.slice_count == slice_count);
    if (!_mi_page_init(heap, page)) {
      _mi_arenas_free( page, mi_page_full_size(page), page->memid );
      return NULL;
    }
    return page;
  }

  return NULL;
}

// Allocate a page containing one block (very large, or with large alignment)
static mi_page_t* mi_arenas_page_singleton_alloc(mi_heap_t* heap, size_t block_size, size_t block_alignment) {
  mi_arena_t* req_arena = heap->exclusive_arena;
  mi_tld_t* const tld = heap->tld;
  const bool os_align = (block_alignment > MI_PAGE_MAX_OVERALLOC_ALIGN);
  const size_t info_size = (os_align ? MI_PAGE_ALIGN : mi_page_info_size());
  #if MI_SECURE < 2
  const size_t slice_count = mi_slice_count_of_size(info_size + block_size);
  #else
  const size_t slice_count = mi_slice_count_of_size(_mi_align_up(info_size + block_size, _mi_os_secure_guard_page_size()) + _mi_os_secure_guard_page_size());
  #endif

  mi_page_t* page = mi_arenas_page_alloc_fresh(slice_count, block_size, block_alignment, req_arena, true /* commit singletons always */, tld);
  if (page == NULL) return NULL;

  mi_assert(page->reserved == 1);
  if (!_mi_page_init(heap, page)) {
    _mi_arenas_free(page, mi_page_full_size(page), page->memid);
    return NULL;
  }

  return page;
}


mi_page_t* _mi_arenas_page_alloc(mi_heap_t* heap, size_t block_size, size_t block_alignment) {
  mi_page_t* page;
  if mi_unlikely(block_alignment > MI_PAGE_MAX_OVERALLOC_ALIGN) {
    mi_assert_internal(_mi_is_power_of_two(block_alignment));
    page = mi_arenas_page_singleton_alloc(heap, block_size, block_alignment);
  }
  else if (block_size <= MI_SMALL_MAX_OBJ_SIZE) {
    page = mi_arenas_page_regular_alloc(heap, mi_slice_count_of_size(MI_SMALL_PAGE_SIZE), block_size);
  }
  else if (block_size <= MI_MEDIUM_MAX_OBJ_SIZE) {
    page = mi_arenas_page_regular_alloc(heap, mi_slice_count_of_size(MI_MEDIUM_PAGE_SIZE), block_size);
  }
  #if MI_ENABLE_LARGE_PAGES
  else if (block_size <= MI_LARGE_MAX_OBJ_SIZE) {
    page = mi_arenas_page_regular_alloc(heap, mi_slice_count_of_size(MI_LARGE_PAGE_SIZE), block_size);
  }
  #endif
  else {
    page = mi_arenas_page_singleton_alloc(heap, block_size, block_alignment);
  }
  // mi_assert_internal(page == NULL || _mi_page_segment(page)->subproc == tld->subproc);
  mi_assert_internal(_mi_is_aligned(page, MI_PAGE_ALIGN));
  mi_assert_internal(_mi_ptr_page(page)==page);
  mi_assert_internal(_mi_ptr_page(mi_page_start(page))==page);
  mi_assert_internal(block_alignment <= MI_PAGE_MAX_OVERALLOC_ALIGN || _mi_is_aligned(mi_page_start(page), block_alignment));

  return page;
}

void _mi_arenas_page_free(mi_page_t* page, mi_tld_t* stats_tld /* can be NULL */) {
  mi_assert_internal(_mi_is_aligned(page, MI_PAGE_ALIGN));
  mi_assert_internal(_mi_ptr_page(page)==page);
  mi_assert_internal(mi_page_is_owned(page));
  mi_assert_internal(mi_page_all_free(page));
  mi_assert_internal(mi_page_is_abandoned(page));
  mi_assert_internal(page->next==NULL && page->prev==NULL);

  // statistics
  if (stats_tld != NULL) { 
    mi_tld_stat_decrease(stats_tld, page_bins[_mi_page_bin(page)], 1);
    mi_tld_stat_decrease(stats_tld, pages, 1);
  }
  else {
    mi_os_stat_decrease(page_bins[_mi_page_bin(page)], 1);
    mi_os_stat_decrease(pages, 1);
  }
  const size_t block_size = mi_page_block_size(page);
  if (mi_page_is_huge(page)) {
    mi_os_stat_decrease(malloc_huge, block_size);
  }

  // assertions
  #if MI_DEBUG>1
  if (page->memid.memkind==MI_MEM_ARENA && !mi_page_is_full(page)) {
    size_t bin = _mi_bin(block_size);
    size_t slice_index;
    size_t slice_count;
    mi_arena_t* arena = mi_page_arena(page, &slice_index, &slice_count);

    mi_assert_internal(mi_bbitmap_is_clearN(arena->slices_free, slice_index, slice_count));
    mi_assert_internal(page->slice_committed > 0 || mi_bitmap_is_setN(arena->slices_committed, slice_index, slice_count));
    mi_assert_internal(mi_bitmap_is_clearN(arena->pages_abandoned[bin], slice_index, 1));
    mi_assert_internal(mi_bitmap_is_setN(page->memid.mem.arena.arena->pages, page->memid.mem.arena.slice_index, 1));
    // note: we cannot check for `!mi_page_is_abandoned_and_mapped` since that may
    // be (temporarily) not true if the free happens while trying to reclaim
    // see `mi_arana_try_claim_abandoned`
  }
  #endif

  // recommit guard page at the end?
  // we must do this since we may later allocate large spans over this page and cannot have a guard page in between
  #if MI_SECURE >= 2
  if (!page->memid.is_pinned) {
    _mi_os_secure_guard_page_reset_before((uint8_t*)page + mi_page_full_size(page));
  }
  #endif

  // unregister page
  _mi_page_map_unregister(page);
  if (page->memid.memkind == MI_MEM_ARENA) {
    mi_arena_t* arena = page->memid.mem.arena.arena;
    mi_bitmap_clear(arena->pages, page->memid.mem.arena.slice_index);
    if (page->slice_committed > 0) {
      // if committed on-demand, set the commit bits to account commit properly
      mi_assert_internal(mi_page_full_size(page) >= page->slice_committed);
      const size_t total_slices = page->slice_committed / MI_ARENA_SLICE_SIZE;  // conservative
      //mi_assert_internal(mi_bitmap_is_clearN(arena->slices_committed, page->memid.mem.arena.slice_index, total_slices));
      mi_assert_internal(page->memid.mem.arena.slice_count >= total_slices);
      if (total_slices > 0) {
        mi_bitmap_setN(arena->slices_committed, page->memid.mem.arena.slice_index, total_slices, NULL);
      }
      // any left over?
      const size_t extra = page->slice_committed % MI_ARENA_SLICE_SIZE;
      if (extra > 0) {
        // pretend it was decommitted already
        mi_os_stat_decrease(committed, extra);
      }
    }
    else {
      mi_assert_internal(mi_bitmap_is_setN(arena->slices_committed, page->memid.mem.arena.slice_index, page->memid.mem.arena.slice_count));
    }
  }
  _mi_arenas_free(page, mi_page_full_size(page), page->memid);
}

/* -----------------------------------------------------------
  Arena abandon
----------------------------------------------------------- */

void _mi_arenas_page_abandon(mi_page_t* page, mi_tld_t* tld) {
  mi_assert_internal(_mi_is_aligned(page, MI_PAGE_ALIGN));
  mi_assert_internal(_mi_ptr_page(page)==page);
  mi_assert_internal(mi_page_is_owned(page));
  mi_assert_internal(mi_page_is_abandoned(page));
  mi_assert_internal(!mi_page_all_free(page));
  mi_assert_internal(page->next==NULL && page->prev == NULL);

  if (page->memid.memkind==MI_MEM_ARENA && !mi_page_is_full(page)) {
    // make available for allocations
    size_t bin = _mi_bin(mi_page_block_size(page));
    size_t slice_index;
    size_t slice_count;
    mi_arena_t* arena = mi_page_arena(page, &slice_index, &slice_count);
    mi_assert_internal(!mi_page_is_singleton(page));
    mi_assert_internal(mi_bbitmap_is_clearN(arena->slices_free, slice_index, slice_count));
    mi_assert_internal(page->slice_committed > 0 || mi_bitmap_is_setN(arena->slices_committed, slice_index, slice_count));
    mi_assert_internal(mi_bitmap_is_setN(arena->slices_dirty, slice_index, slice_count));

    mi_page_set_abandoned_mapped(page);
    const bool wasclear = mi_bitmap_set(arena->pages_abandoned[bin], slice_index);
    MI_UNUSED(wasclear); mi_assert_internal(wasclear);
    mi_atomic_increment_relaxed(&arena->subproc->abandoned_count[bin]);
    mi_tld_stat_increase(tld, pages_abandoned, 1);
  }
  else {
    // page is full (or a singleton), or the page is OS/externally allocated
    // leave as is; it will be reclaimed when an object is free'd in the page
    mi_subproc_t* subproc = _mi_subproc();
    // but for non-arena pages, add to the subproc list so these can be visited
    if (page->memid.memkind != MI_MEM_ARENA && mi_option_is_enabled(mi_option_visit_abandoned)) {
      mi_lock(&subproc->os_abandoned_pages_lock) {
        // push in front
        page->prev = NULL;
        page->next = subproc->os_abandoned_pages;
        if (page->next != NULL) { page->next->prev = page; }
        subproc->os_abandoned_pages = page;
      }
    }
    mi_tld_stat_increase(tld, pages_abandoned, 1);
  }
  _mi_page_unown(page);
}

bool _mi_arenas_page_try_reabandon_to_mapped(mi_page_t* page) {
  mi_assert_internal(_mi_is_aligned(page, MI_PAGE_ALIGN));
  mi_assert_internal(_mi_ptr_page(page)==page);
  mi_assert_internal(mi_page_is_owned(page));
  mi_assert_internal(mi_page_is_abandoned(page));
  mi_assert_internal(!mi_page_is_abandoned_mapped(page));
  mi_assert_internal(!mi_page_is_full(page));
  mi_assert_internal(!mi_page_all_free(page));
  mi_assert_internal(!mi_page_is_singleton(page));
  if (mi_page_is_full(page) || mi_page_is_abandoned_mapped(page) || page->memid.memkind != MI_MEM_ARENA) {
    return false;
  }
  else {
    mi_tld_t* tld = _mi_thread_tld();
    mi_tld_stat_counter_increase( tld, pages_reabandon_full, 1);
    mi_tld_stat_adjust_decrease( tld, pages_abandoned, 1 );  // adjust as we are not abandoning fresh
    _mi_arenas_page_abandon(page,tld);
    return true;
  }
}

// called from `mi_free` if trying to unabandon an abandoned page
void _mi_arenas_page_unabandon(mi_page_t* page) {
  mi_assert_internal(_mi_is_aligned(page, MI_PAGE_ALIGN));
  mi_assert_internal(_mi_ptr_page(page)==page);
  mi_assert_internal(mi_page_is_owned(page));
  mi_assert_internal(mi_page_is_abandoned(page));

  if (mi_page_is_abandoned_mapped(page)) {
    mi_assert_internal(page->memid.memkind==MI_MEM_ARENA);
    // remove from the abandoned map
    size_t bin = _mi_bin(mi_page_block_size(page));
    size_t slice_index;
    size_t slice_count;
    mi_arena_t* arena = mi_page_arena(page, &slice_index, &slice_count);

    mi_assert_internal(mi_bbitmap_is_clearN(arena->slices_free, slice_index, slice_count));
    mi_assert_internal(page->slice_committed > 0 || mi_bitmap_is_setN(arena->slices_committed, slice_index, slice_count));

    // this busy waits until a concurrent reader (from alloc_abandoned) is done
    mi_bitmap_clear_once_set(arena->pages_abandoned[bin], slice_index);
    mi_page_clear_abandoned_mapped(page);
    mi_atomic_decrement_relaxed(&arena->subproc->abandoned_count[bin]);
    mi_tld_stat_decrease(_mi_thread_tld(), pages_abandoned, 1);
  }
  else {
    // page is full (or a singleton), page is OS allocated
    mi_tld_stat_decrease(_mi_thread_tld(), pages_abandoned, 1);
    // if not an arena page, remove from the subproc os pages list
    if (page->memid.memkind != MI_MEM_ARENA && mi_option_is_enabled(mi_option_visit_abandoned)) {
      mi_subproc_t* subproc = _mi_subproc();
      mi_lock(&subproc->os_abandoned_pages_lock) {
        if (page->prev != NULL) { page->prev->next = page->next; }
        if (page->next != NULL) { page->next->prev = page->prev; }
        if (subproc->os_abandoned_pages == page) { subproc->os_abandoned_pages = page->next; }
        page->next = NULL;
        page->prev = NULL;
      }
    }
  }
}


/* -----------------------------------------------------------
  Arena free
----------------------------------------------------------- */
static void mi_arena_schedule_purge(mi_arena_t* arena, size_t slice_index, size_t slices);
static void mi_arenas_try_purge(bool force, bool visit_all, mi_tld_t* tld);

void _mi_arenas_free(void* p, size_t size, mi_memid_t memid) {
  if (p==NULL) return;
  if (size==0) return;

  // need to set all memory to undefined as some parts may still be marked as no_access (like padding etc.)
  mi_track_mem_undefined(p, size);

  if (mi_memkind_is_os(memid.memkind)) {
    // was a direct OS allocation, pass through
    _mi_os_free(p, size, memid);
  }
  else if (memid.memkind == MI_MEM_ARENA) {
    // allocated in an arena
    size_t slice_count;
    size_t slice_index;
    mi_arena_t* arena = mi_arena_from_memid(memid, &slice_index, &slice_count);
    mi_assert_internal((size%MI_ARENA_SLICE_SIZE)==0);
    mi_assert_internal((slice_count*MI_ARENA_SLICE_SIZE)==size);
    mi_assert_internal(mi_arena_slice_start(arena,slice_index) <= (uint8_t*)p);
    mi_assert_internal(mi_arena_slice_start(arena,slice_index) + mi_size_of_slices(slice_count) > (uint8_t*)p);
    // checks
    if (arena == NULL) {
      _mi_error_message(EINVAL, "trying to free from an invalid arena: %p, size %zu, memid: 0x%zx\n", p, size, memid);
      return;
    }
    mi_assert_internal(slice_index < arena->slice_count);
    mi_assert_internal(slice_index >= mi_arena_info_slices(arena));
    if (slice_index < mi_arena_info_slices(arena) || slice_index > arena->slice_count) {
      _mi_error_message(EINVAL, "trying to free from an invalid arena block: %p, size %zu, memid: 0x%zx\n", p, size, memid);
      return;
    }

    // potentially decommit
    if (!arena->memid.is_pinned /* && !arena->memid.initially_committed */) { // todo: allow decommit even if initially committed?
      // (delay) purge the page
      mi_arena_schedule_purge(arena, slice_index, slice_count);
    }

    // and make it available to others again
    bool all_inuse = mi_bbitmap_setN(arena->slices_free, slice_index, slice_count);
    if (!all_inuse) {
      _mi_error_message(EAGAIN, "trying to free an already freed arena block: %p, size %zu\n", mi_arena_slice_start(arena,slice_index), mi_size_of_slices(slice_count));
      return;
    };
  }
  else if (memid.memkind == MI_MEM_META) {
    _mi_meta_free(p, size, memid);
  }
  else {
    // arena was none, external, or static; nothing to do
    mi_assert_internal(mi_memid_needs_no_free(memid));
  }

  // try to purge expired decommits
  // mi_arenas_try_purge(false, false, NULL);
}

// Purge the arenas; if `force_purge` is true, amenable parts are purged even if not yet expired
void _mi_arenas_collect(bool force_purge, bool visit_all, mi_tld_t* tld) {
  mi_arenas_try_purge(force_purge, visit_all, tld);
}


// Is a pointer contained in the given arena area?
bool mi_arena_contains(mi_arena_id_t arena_id, const void* p) {
  mi_arena_t* arena = _mi_arena_from_id(arena_id);
  return (mi_arena_start(arena) <= (const uint8_t*)p &&
          mi_arena_start(arena) + mi_size_of_slices(arena->slice_count) >(const uint8_t*)p);
}

// Is a pointer inside any of our arenas?
bool _mi_arenas_contain(const void* p) {
  mi_subproc_t* subproc = _mi_subproc();
  const size_t max_arena = mi_arenas_get_count(subproc);
  for (size_t i = 0; i < max_arena; i++) {
    mi_arena_t* arena = mi_atomic_load_ptr_acquire(mi_arena_t, &subproc->arenas[i]);
    if (arena != NULL && mi_arena_contains(arena,p)) {
      return true;
    }
  }
  return false;
}



/* -----------------------------------------------------------
  Remove an arena.
----------------------------------------------------------- */

// destroy owned arenas; this is unsafe and should only be done using `mi_option_destroy_on_exit`
// for dynamic libraries that are unloaded and need to release all their allocated memory.
static void mi_arenas_unsafe_destroy(mi_subproc_t* subproc) {
  const size_t max_arena = mi_arenas_get_count(subproc);
  size_t new_max_arena = 0;
  for (size_t i = 0; i < max_arena; i++) {
    mi_arena_t* arena = mi_atomic_load_ptr_acquire(mi_arena_t, &subproc->arenas[i]);
    if (arena != NULL) {
      // mi_lock_done(&arena->abandoned_visit_lock);
      mi_atomic_store_ptr_release(mi_arena_t, &subproc->arenas[i], NULL);
      if (mi_memkind_is_os(arena->memid.memkind)) {
        _mi_os_free(mi_arena_start(arena), mi_arena_size(arena), arena->memid);
      }
    }
  }

  // try to lower the max arena.
  size_t expected = max_arena;
  mi_atomic_cas_strong_acq_rel(&subproc->arena_count, &expected, new_max_arena);
}


// destroy owned arenas; this is unsafe and should only be done using `mi_option_destroy_on_exit`
// for dynamic libraries that are unloaded and need to release all their allocated memory.
void _mi_arenas_unsafe_destroy_all(mi_tld_t* tld) {
  mi_arenas_unsafe_destroy(_mi_subproc());
  _mi_arenas_collect(true /* force purge */, true /* visit all*/, tld);  // purge non-owned arenas
}


/* -----------------------------------------------------------
  Add an arena.
----------------------------------------------------------- */

static bool mi_arenas_add(mi_subproc_t* subproc, mi_arena_t* arena, mi_arena_id_t* arena_id) {
  mi_assert_internal(arena != NULL);
  mi_assert_internal(arena->slice_count > 0);
  if (arena_id != NULL) { *arena_id = NULL; }

  // first try to find a NULL entry
  const size_t count = mi_arenas_get_count(subproc);
  size_t i;
  for (i = 0; i < count; i++) {
    if (mi_arena_from_index(subproc,i) == NULL) {
      mi_arena_t* expected = NULL;
      if (mi_atomic_cas_ptr_strong_release(mi_arena_t, &subproc->arenas[i], &expected, arena)) {
        // success
        if (arena_id != NULL) { *arena_id = arena; }
        return true;
      }
    }
  }

  // otherwise increase the max
  i = mi_atomic_increment_acq_rel(&subproc->arena_count);
  if (i >= MI_MAX_ARENAS) {
    mi_atomic_decrement_acq_rel(&subproc->arena_count);
    arena->subproc = NULL;
    return false;
  }

  mi_subproc_stat_counter_increase(arena->subproc, arena_count, 1);
  mi_atomic_store_ptr_release(mi_arena_t,&subproc->arenas[i], arena);
  if (arena_id != NULL) { *arena_id = arena; }
  return true;
}

static size_t mi_arena_info_slices_needed(size_t slice_count, size_t* bitmap_base) {
  if (slice_count == 0) slice_count = MI_BCHUNK_BITS;
  mi_assert_internal((slice_count % MI_BCHUNK_BITS) == 0);
  const size_t base_size = _mi_align_up(sizeof(mi_arena_t), MI_BCHUNK_SIZE);
  const size_t bitmaps_count = 4 + MI_BIN_COUNT; // commit, dirty, purge, pages, and abandonded
  const size_t bitmaps_size = bitmaps_count * mi_bitmap_size(slice_count, NULL) + mi_bbitmap_size(slice_count, NULL); // + free
  const size_t size = base_size + bitmaps_size;

  const size_t os_page_size = _mi_os_page_size();
  const size_t info_size = _mi_align_up(size, os_page_size) + _mi_os_secure_guard_page_size();
  const size_t info_slices = mi_slice_count_of_size(info_size);

  if (bitmap_base != NULL) *bitmap_base = base_size;
  return info_slices;
}

static mi_bitmap_t* mi_arena_bitmap_init(size_t slice_count, uint8_t** base) {
  mi_bitmap_t* bitmap = (mi_bitmap_t*)(*base);
  *base = (*base) + mi_bitmap_init(bitmap, slice_count, true /* already zero */);
  return bitmap;
}

static mi_bbitmap_t* mi_arena_bbitmap_init(size_t slice_count, uint8_t** base) {
  mi_bbitmap_t* bbitmap = (mi_bbitmap_t*)(*base);
  *base = (*base) + mi_bbitmap_init(bbitmap, slice_count, true /* already zero */);
  return bbitmap;
}


static bool mi_manage_os_memory_ex2(mi_subproc_t* subproc, void* start, size_t size, int numa_node, bool exclusive, mi_memid_t memid, mi_arena_id_t* arena_id) mi_attr_noexcept
{
  mi_assert(_mi_is_aligned(start,MI_ARENA_SLICE_SIZE));
  mi_assert(start!=NULL);
  if (arena_id != NULL) { *arena_id = _mi_arena_id_none(); }
  if (start==NULL) return false;
  if (!_mi_is_aligned(start,MI_ARENA_SLICE_SIZE)) {
    // we can align the start since the memid tracks the real base of the memory.
    void* const aligned_start = _mi_align_up_ptr(start, MI_ARENA_SLICE_SIZE);
    const size_t diff = (uint8_t*)aligned_start - (uint8_t*)start;
    if (diff >= size || (size - diff) < MI_ARENA_SLICE_SIZE) {
      _mi_warning_message("after alignment, the size of the arena becomes too small (memory at %p with size %zu)\n", start, size);
      return false;
    }
    start = aligned_start;
    size = size - diff;
  }

  const size_t slice_count = _mi_align_down(size / MI_ARENA_SLICE_SIZE, MI_BCHUNK_BITS);
  if (slice_count > MI_BITMAP_MAX_BIT_COUNT) {  // 16 GiB for now
    // todo: allow larger areas (either by splitting it up in arena's or having larger arena's)
    _mi_warning_message("cannot use OS memory since it is too large (size %zu MiB, maximum is %zu MiB)", size/MI_MiB, mi_size_of_slices(MI_BITMAP_MAX_BIT_COUNT)/MI_MiB);
    return false;
  }
  size_t bitmap_base;
  const size_t info_slices = mi_arena_info_slices_needed(slice_count, &bitmap_base);
  if (slice_count < info_slices+1) {
    _mi_warning_message("cannot use OS memory since it is not large enough (size %zu KiB, minimum required is %zu KiB)", size/MI_KiB, mi_size_of_slices(info_slices+1)/MI_KiB);
    return false;
  }

  mi_arena_t* arena = (mi_arena_t*)start;

  // commit & zero if needed
  if (!memid.initially_committed) {
    // leave a guard OS page decommitted at the end
    if (!_mi_os_commit(arena, mi_size_of_slices(info_slices) - _mi_os_secure_guard_page_size(), NULL)) {
      _mi_warning_message("unable to commit meta data for provided OS memory");
      return false;
    }
  }
  else {
    // if MI_SECURE, set a guard page at the end
    _mi_os_secure_guard_page_set_before((uint8_t*)arena + mi_size_of_slices(info_slices), memid.is_pinned);
  }
  if (!memid.initially_zero) {
    _mi_memzero(arena, mi_size_of_slices(info_slices) - _mi_os_secure_guard_page_size());
  }

  // init
  arena->subproc      = subproc;
  arena->memid        = memid;
  arena->is_exclusive = exclusive;
  arena->slice_count  = slice_count;
  arena->info_slices  = info_slices;
  arena->numa_node    = numa_node; // TODO: or get the current numa node if -1? (now it allows anyone to allocate on -1)
  arena->purge_expire = 0;
  // mi_lock_init(&arena->abandoned_visit_lock);

  // init bitmaps
  uint8_t* base = mi_arena_start(arena) + bitmap_base;
  arena->slices_free = mi_arena_bbitmap_init(slice_count,&base);
  arena->slices_committed = mi_arena_bitmap_init(slice_count,&base);
  arena->slices_dirty = mi_arena_bitmap_init(slice_count,&base);
  arena->slices_purge = mi_arena_bitmap_init(slice_count, &base);
  arena->pages = mi_arena_bitmap_init(slice_count, &base);
  for( size_t i = 0; i < MI_ARENA_BIN_COUNT; i++) {
    arena->pages_abandoned[i] = mi_arena_bitmap_init(slice_count,&base);
  }
  mi_assert_internal(mi_size_of_slices(info_slices) >= (size_t)(base - mi_arena_start(arena)));

  // reserve our meta info (and reserve slices outside the memory area)
  mi_bbitmap_unsafe_setN(arena->slices_free, info_slices /* start */, arena->slice_count - info_slices);
  if (memid.initially_committed) {
    mi_bitmap_unsafe_setN(arena->slices_committed, 0, arena->slice_count);
  }
  else {
    mi_bitmap_setN(arena->slices_committed, 0, info_slices, NULL);
  }
  if (!memid.initially_zero) {
    mi_bitmap_unsafe_setN(arena->slices_dirty, 0, arena->slice_count);
  }
  else {
    mi_bitmap_setN(arena->slices_dirty, 0, info_slices, NULL);
  }

  return mi_arenas_add(subproc, arena, arena_id);
}


bool mi_manage_os_memory_ex(void* start, size_t size, bool is_committed, bool is_pinned, bool is_zero, int numa_node, bool exclusive, mi_arena_id_t* arena_id) mi_attr_noexcept {
  mi_memid_t memid = _mi_memid_create(MI_MEM_EXTERNAL);
  memid.mem.os.base = start;
  memid.mem.os.size = size;
  memid.initially_committed = is_committed;
  memid.initially_zero = is_zero;
  memid.is_pinned = is_pinned;
  return mi_manage_os_memory_ex2(_mi_subproc(), start, size, numa_node, exclusive, memid, arena_id);
}

// Reserve a range of regular OS memory
static int mi_reserve_os_memory_ex2(mi_subproc_t* subproc, size_t size, bool commit, bool allow_large, bool exclusive, mi_arena_id_t* arena_id) {
  if (arena_id != NULL) *arena_id = _mi_arena_id_none();
  size = _mi_align_up(size, MI_ARENA_SLICE_SIZE); // at least one slice
  mi_memid_t memid;
  void* start = _mi_os_alloc_aligned(size, MI_ARENA_SLICE_ALIGN, commit, allow_large, &memid);
  if (start == NULL) return ENOMEM;
  if (!mi_manage_os_memory_ex2(subproc, start, size, -1 /* numa node */, exclusive, memid, arena_id)) {
    _mi_os_free_ex(start, size, commit, memid);
    _mi_verbose_message("failed to reserve %zu KiB memory\n", _mi_divide_up(size, 1024));
    return ENOMEM;
  }
  _mi_verbose_message("reserved %zu KiB memory%s\n", _mi_divide_up(size, 1024), memid.is_pinned ? " (in large os pages)" : "");
  // mi_debug_show_arenas(true, true, false);

  return 0;
}

// Reserve a range of regular OS memory
int mi_reserve_os_memory_ex(size_t size, bool commit, bool allow_large, bool exclusive, mi_arena_id_t* arena_id) mi_attr_noexcept {
  return mi_reserve_os_memory_ex2(_mi_subproc(), size, commit, allow_large, exclusive, arena_id);
}

// Manage a range of regular OS memory
bool mi_manage_os_memory(void* start, size_t size, bool is_committed, bool is_large, bool is_zero, int numa_node) mi_attr_noexcept {
  return mi_manage_os_memory_ex(start, size, is_committed, is_large, is_zero, numa_node, false /* exclusive? */, NULL);
}

// Reserve a range of regular OS memory
int mi_reserve_os_memory(size_t size, bool commit, bool allow_large) mi_attr_noexcept {
  return mi_reserve_os_memory_ex(size, commit, allow_large, false, NULL);
}


/* -----------------------------------------------------------
  Debugging
----------------------------------------------------------- */
static size_t mi_debug_show_bfield(mi_bfield_t field, char* buf, size_t* k) {
  size_t bit_set_count = 0;
  for (int bit = 0; bit < MI_BFIELD_BITS; bit++) {
    bool is_set = ((((mi_bfield_t)1 << bit) & field) != 0);
    if (is_set) bit_set_count++;
    buf[*k++] = (is_set ? 'x' : '.');
  }
  return bit_set_count;
}

typedef enum mi_ansi_color_e {
  MI_BLACK = 30,
  MI_MAROON,
  MI_DARKGREEN,
  MI_ORANGE,
  MI_NAVY,
  MI_PURPLE,
  MI_TEAL,
  MI_GRAY,
  MI_DARKGRAY = 90,
  MI_RED,
  MI_GREEN,
  MI_YELLOW,
  MI_BLUE,
  MI_MAGENTA,
  MI_CYAN,
  MI_WHITE
} mi_ansi_color_t;

static void mi_debug_color(char* buf, size_t* k, mi_ansi_color_t color) {
  *k += _mi_snprintf(buf + *k, 32, "\x1B[%dm", (int)color);
}

static int mi_page_commit_usage(mi_page_t* page) {
  // if (mi_page_size(page) <= MI_PAGE_MIN_COMMIT_SIZE) return 100;
  const size_t committed_size = mi_page_committed(page);
  const size_t used_size = page->used * mi_page_block_size(page);
  return (int)(used_size * 100 / committed_size);
}

static size_t mi_debug_show_page_bfield(mi_bfield_t field, char* buf, size_t* k, mi_arena_t* arena, size_t slice_index, long* pbit_of_page, mi_ansi_color_t* pcolor_of_page ) {
  size_t bit_set_count = 0;
  long bit_of_page = *pbit_of_page;
  mi_ansi_color_t color = *pcolor_of_page;
  mi_ansi_color_t prev_color = MI_GRAY;
  for (int bit = 0; bit < MI_BFIELD_BITS; bit++, bit_of_page--) {
    bool is_set = ((((mi_bfield_t)1 << bit) & field) != 0);
    void* start = mi_arena_slice_start(arena, slice_index + bit);
    char c = ' ';
    if (is_set) {
      mi_assert_internal(bit_of_page <= 0);
      bit_set_count++;
      c = 'p';
      color = MI_GRAY;
      mi_page_t* page = (mi_page_t*)start;
      if (mi_page_is_singleton(page)) { c = 's'; }
      else if (mi_page_is_full(page)) { c = 'f'; }
      if (!mi_page_is_abandoned(page)) { c = _mi_toupper(c); }
      int commit_usage = mi_page_commit_usage(page);
      if (commit_usage < 25) { color = MI_MAROON; }
      else if (commit_usage < 50) { color = MI_ORANGE; }
      else if (commit_usage < 75) { color = MI_TEAL; }
      else color = MI_DARKGREEN;
      bit_of_page = (long)page->memid.mem.arena.slice_count;
    }
    else {
      c = '?';
      if (bit_of_page > 0) { c = '-'; }
      else if (_mi_meta_is_meta_page(start)) { c = 'm'; color = MI_GRAY; }
      else if (slice_index + bit < arena->info_slices) { c = 'i'; color = MI_GRAY; }
      // else if (mi_bitmap_is_setN(arena->pages_purge, slice_index + bit, NULL)) { c = '*'; }
      else if (mi_bbitmap_is_setN(arena->slices_free, slice_index+bit,1)) {
        if (mi_bitmap_is_set(arena->slices_purge, slice_index + bit)) { c = '~'; color = MI_ORANGE; }
        else if (mi_bitmap_is_setN(arena->slices_committed, slice_index + bit, 1)) { c = '_'; color = MI_GRAY; }
        else { c = '.'; color = MI_GRAY; }
      }
      if (bit==MI_BFIELD_BITS-1 && bit_of_page > 1) { c = '>'; }
    }
    if (color != prev_color) {
      mi_debug_color(buf, k, color);
      prev_color = color;
    }
    buf[*k] = c; *k += 1;
  }
  mi_debug_color(buf, k, MI_GRAY);
  *pbit_of_page = bit_of_page;
  *pcolor_of_page = color;
  return bit_set_count;
}

static size_t mi_debug_show_chunks(const char* header1, const char* header2, const char* header3, size_t slice_count, size_t chunk_count, mi_bchunk_t* chunks, _Atomic(uint8_t)* chunk_bins, bool invert, mi_arena_t* arena, bool narrow) {
  _mi_output_message("\x1B[37m%s%s%s (use/commit: \x1B[31m0 - 25%%\x1B[33m - 50%%\x1B[36m - 75%%\x1B[32m - 100%%\x1B[0m)\n", header1, header2, header3);
  const size_t fields_per_line = (narrow ? 2 : 4);
  size_t bit_count = 0;
  size_t bit_set_count = 0;
  for (size_t i = 0; i < chunk_count && bit_count < slice_count; i++) {
    char buf[5*MI_BCHUNK_BITS + 64]; _mi_memzero(buf, sizeof(buf));
    size_t k = 0;
    mi_bchunk_t* chunk = &chunks[i];

    if (i<10)        { buf[k++] = ('0' + (char)i); buf[k++] = ' '; buf[k++] = ' '; }
    else if (i<100)  { buf[k++] = ('0' + (char)(i/10)); buf[k++] = ('0' + (char)(i%10)); buf[k++] = ' '; }
    else if (i<1000) { buf[k++] = ('0' + (char)(i/100)); buf[k++] = ('0' + (char)((i%100)/10)); buf[k++] = ('0' + (char)(i%10)); }

    char chunk_kind = ' ';
    if (chunk_bins != NULL) {
      switch (mi_atomic_load_relaxed(&chunk_bins[i])) {
        case MI_CBIN_SMALL:  chunk_kind = 'S'; break;
        case MI_CBIN_MEDIUM: chunk_kind = 'M'; break;
        case MI_CBIN_LARGE:  chunk_kind = 'L'; break;
        case MI_CBIN_OTHER:  chunk_kind = 'X'; break;
        // case MI_BBIN_NONE: chunk_kind = 'N'; break;
      }
    }
    buf[k++] = chunk_kind;
    buf[k++] = ' ';

    long bit_of_page = 0;
    mi_ansi_color_t color_of_page = MI_GRAY;
    for (size_t j = 0; j < MI_BCHUNK_FIELDS; j++) {
      if (j > 0 && (j % fields_per_line) == 0) {
        // buf[k++] = '\n'; _mi_memset(buf+k,' ',7); k += 7;
        _mi_output_message("  %s\n\x1B[37m", buf);
        _mi_memzero(buf, sizeof(buf));
        _mi_memset(buf, ' ', 5); k = 5;
      }
      if (bit_count < slice_count) {
        mi_bfield_t bfield = chunk->bfields[j];
        if (invert) bfield = ~bfield;
        size_t xcount = (arena!=NULL ? mi_debug_show_page_bfield(bfield, buf, &k, arena, bit_count, &bit_of_page, &color_of_page)
                                     : mi_debug_show_bfield(bfield, buf, &k));
        if (invert) xcount = MI_BFIELD_BITS - xcount;
        bit_set_count += xcount;
        buf[k++] = ' ';
      }
      else {
        _mi_memset(buf + k, 'o', MI_BFIELD_BITS);
        k += MI_BFIELD_BITS;
      }
      bit_count += MI_BFIELD_BITS;
    }
    _mi_output_message("  %s\n\x1B[37m", buf);
  }
  _mi_output_message("\x1B[0m  total ('x'): %zu\n", bit_set_count);
  return bit_set_count;
}

static size_t mi_debug_show_bitmap_binned(const char* header1, const char* header2, const char* header3, size_t slice_count, mi_bitmap_t* bitmap, _Atomic(uint8_t)* chunk_bins, bool invert, mi_arena_t* arena, bool narrow) {
  return mi_debug_show_chunks(header1, header2, header3, slice_count, mi_bitmap_chunk_count(bitmap), &bitmap->chunks[0], chunk_bins, invert, arena, narrow);
}

static void mi_debug_show_arenas_ex(bool show_pages, bool narrow) mi_attr_noexcept {
  mi_subproc_t* subproc = _mi_subproc();
  size_t max_arenas = mi_arenas_get_count(subproc);
  //size_t free_total = 0;
  //size_t slice_total = 0;
  //size_t abandoned_total = 0;
  size_t page_total = 0;
  for (size_t i = 0; i < max_arenas; i++) {
    mi_arena_t* arena = mi_atomic_load_ptr_acquire(mi_arena_t, &subproc->arenas[i]);
    if (arena == NULL) break;
    mi_assert(arena->subproc == subproc);
    // slice_total += arena->slice_count;
    _mi_output_message("arena %zu at %p: %zu slices (%zu MiB)%s, subproc: %p\n", i, arena, arena->slice_count, mi_size_of_slices(arena->slice_count)/MI_MiB, (arena->memid.is_pinned ? ", pinned" : ""), arena->subproc);
    //if (show_inuse) {
    //  free_total += mi_debug_show_bbitmap("in-use slices", arena->slice_count, arena->slices_free, true, NULL);
    //}
    //if (show_committed) {
    //  mi_debug_show_bitmap("committed slices", arena->slice_count, arena->slices_committed, false, NULL);
    //}
    // todo: abandoned slices
    //if (show_purge) {
    //  purge_total += mi_debug_show_bitmap("purgeable slices", arena->slice_count, arena->slices_purge, false, NULL);
    //}
    if (show_pages) {
      const char* header1 = "pages (p:page, f:full, s:singleton, P,F,S:not abandoned, i:arena-info, m:meta-data, ~:free-purgable, _:free-committed, .:free-reserved)";
      const char* header2 = (narrow ? "\n      " : " ");
      const char* header3 = "(chunk bin: S:small, M : medium, L : large, X : other)";
      page_total += mi_debug_show_bitmap_binned(header1, header2, header3, arena->slice_count, arena->pages, arena->slices_free->chunk_bins, false, arena, narrow);
    }
  }
  // if (show_inuse)     _mi_output_message("total inuse slices    : %zu\n", slice_total - free_total);
  // if (show_abandoned) _mi_verbose_message("total abandoned slices: %zu\n", abandoned_total);
  if (show_pages)     _mi_output_message("total pages in arenas: %zu\n", page_total);
}

void mi_debug_show_arenas(void) mi_attr_noexcept {
  mi_debug_show_arenas_ex(true /* show pages */, false /* narrow? */);
}


/* -----------------------------------------------------------
  Reserve a huge page arena.
----------------------------------------------------------- */
// reserve at a specific numa node
int mi_reserve_huge_os_pages_at_ex(size_t pages, int numa_node, size_t timeout_msecs, bool exclusive, mi_arena_id_t* arena_id) mi_attr_noexcept {
  if (arena_id != NULL) *arena_id = NULL;
  if (pages==0) return 0;
  if (numa_node < -1) numa_node = -1;
  if (numa_node >= 0) numa_node = numa_node % _mi_os_numa_node_count();
  size_t hsize = 0;
  size_t pages_reserved = 0;
  mi_memid_t memid;
  void* p = _mi_os_alloc_huge_os_pages(pages, numa_node, timeout_msecs, &pages_reserved, &hsize, &memid);
  if (p==NULL || pages_reserved==0) {
    _mi_warning_message("failed to reserve %zu GiB huge pages\n", pages);
    return ENOMEM;
  }
  _mi_verbose_message("numa node %i: reserved %zu GiB huge pages (of the %zu GiB requested)\n", numa_node, pages_reserved, pages);

  if (!mi_manage_os_memory_ex2(_mi_subproc(), p, hsize, numa_node, exclusive, memid, arena_id)) {
    _mi_os_free(p, hsize, memid);
    return ENOMEM;
  }
  return 0;
}

int mi_reserve_huge_os_pages_at(size_t pages, int numa_node, size_t timeout_msecs) mi_attr_noexcept {
  return mi_reserve_huge_os_pages_at_ex(pages, numa_node, timeout_msecs, false, NULL);
}

// reserve huge pages evenly among the given number of numa nodes (or use the available ones as detected)
int mi_reserve_huge_os_pages_interleave(size_t pages, size_t numa_nodes, size_t timeout_msecs) mi_attr_noexcept {
  if (pages == 0) return 0;

  // pages per numa node
  size_t numa_count = (numa_nodes > 0 ? numa_nodes : _mi_os_numa_node_count());
  if (numa_count <= 0) numa_count = 1;
  const size_t pages_per = pages / numa_count;
  const size_t pages_mod = pages % numa_count;
  const size_t timeout_per = (timeout_msecs==0 ? 0 : (timeout_msecs / numa_count) + 50);

  // reserve evenly among numa nodes
  for (size_t numa_node = 0; numa_node < numa_count && pages > 0; numa_node++) {
    size_t node_pages = pages_per;  // can be 0
    if (numa_node < pages_mod) node_pages++;
    int err = mi_reserve_huge_os_pages_at(node_pages, (int)numa_node, timeout_per);
    if (err) return err;
    if (pages < node_pages) {
      pages = 0;
    }
    else {
      pages -= node_pages;
    }
  }

  return 0;
}

int mi_reserve_huge_os_pages(size_t pages, double max_secs, size_t* pages_reserved) mi_attr_noexcept {
  MI_UNUSED(max_secs);
  _mi_warning_message("mi_reserve_huge_os_pages is deprecated: use mi_reserve_huge_os_pages_interleave/at instead\n");
  if (pages_reserved != NULL) *pages_reserved = 0;
  int err = mi_reserve_huge_os_pages_interleave(pages, 0, (size_t)(max_secs * 1000.0));
  if (err==0 && pages_reserved!=NULL) *pages_reserved = pages;
  return err;
}





/* -----------------------------------------------------------
  Arena purge
----------------------------------------------------------- */

static long mi_arena_purge_delay(void) {
  // <0 = no purging allowed, 0=immediate purging, >0=milli-second delay
  return (mi_option_get(mi_option_purge_delay) * mi_option_get(mi_option_arena_purge_mult));
}

// reset or decommit in an arena and update the commit bitmap
// assumes we own the area (i.e. slices_free is claimed by us)
// returns if the memory is no longer committed (versus reset which keeps the commit)
static bool mi_arena_purge(mi_arena_t* arena, size_t slice_index, size_t slice_count) {
  mi_assert_internal(!arena->memid.is_pinned);
  mi_assert_internal(mi_bbitmap_is_clearN(arena->slices_free, slice_index, slice_count));

  const size_t size = mi_size_of_slices(slice_count);
  void* const p = mi_arena_slice_start(arena, slice_index);
  //const bool all_committed = mi_bitmap_is_setN(arena->slices_committed, slice_index, slice_count);
  size_t already_committed;
  mi_bitmap_setN(arena->slices_committed, slice_index, slice_count, &already_committed); // pretend all committed.. (as we lack a clearN call that counts the already set bits..)
  const bool all_committed = (already_committed == slice_count);
  const bool needs_recommit = _mi_os_purge_ex(p, size, all_committed /* allow reset? */, mi_size_of_slices(already_committed));

  if (needs_recommit) {
    // no longer committed
    mi_bitmap_clearN(arena->slices_committed, slice_index, slice_count);
    // we just counted in the purge to decommit all, but the some part was not committed so adjust that here
    // mi_os_stat_decrease(committed, mi_size_of_slices(slice_count - already_committed));
  }
  else if (!all_committed) {
    // we cannot assume any of these are committed any longer (even with reset since we did setN and may have marked uncommitted slices as committed)
    mi_bitmap_clearN(arena->slices_committed, slice_index, slice_count);
    // we adjust the commit count as parts will be re-committed
    // mi_os_stat_decrease(committed, mi_size_of_slices(already_committed));
  }

  return needs_recommit;
}


// Schedule a purge. This is usually delayed to avoid repeated decommit/commit calls.
// Note: assumes we (still) own the area as we may purge immediately
static void mi_arena_schedule_purge(mi_arena_t* arena, size_t slice_index, size_t slice_count) {
  const long delay = mi_arena_purge_delay();
  if (arena->memid.is_pinned || delay < 0 || _mi_preloading()) return;  // is purging allowed at all?

  mi_assert_internal(mi_bbitmap_is_clearN(arena->slices_free, slice_index, slice_count));
  if (delay == 0) {
    // purge directly
    mi_arena_purge(arena, slice_index, slice_count);
  }
  else {
    // schedule purge
    const mi_msecs_t expire = _mi_clock_now() + delay;
    mi_msecs_t expire0 = 0;
    if (mi_atomic_casi64_strong_acq_rel(&arena->purge_expire, &expire0, expire)) {
      // expiration was not yet set
      // maybe set the global arenas expire as well (if it wasn't set already)
      mi_assert_internal(expire0==0);
      mi_atomic_casi64_strong_acq_rel(&arena->subproc->purge_expire, &expire0, expire);
    }
    else {
      // already an expiration was set
    }
    mi_bitmap_setN(arena->slices_purge, slice_index, slice_count, NULL);
  }
}

typedef struct mi_purge_visit_info_s {
  mi_msecs_t now;
  mi_msecs_t delay;
  bool all_purged;
  bool any_purged;
} mi_purge_visit_info_t;

static bool mi_arena_try_purge_range(mi_arena_t* arena, size_t slice_index, size_t slice_count) {
  if (mi_bbitmap_try_clearN(arena->slices_free, slice_index, slice_count)) {
    // purge
    bool decommitted = mi_arena_purge(arena, slice_index, slice_count); MI_UNUSED(decommitted);
    mi_assert_internal(!decommitted || mi_bitmap_is_clearN(arena->slices_committed, slice_index, slice_count));
    // and reset the free range
    mi_bbitmap_setN(arena->slices_free, slice_index, slice_count);
    return true;
  }
  else {
    // was allocated again already
    return false;
  }
}

static bool mi_arena_try_purge_visitor(size_t slice_index, size_t slice_count, mi_arena_t* arena, void* arg) {
  mi_purge_visit_info_t* vinfo = (mi_purge_visit_info_t*)arg;
  // try to purge: first claim the free blocks
  if (mi_arena_try_purge_range(arena, slice_index, slice_count)) {
    vinfo->any_purged = true;
    vinfo->all_purged = true;
  }
  else if (slice_count > 1)
  {
    // failed to claim the full range, try per slice instead
    for (size_t i = 0; i < slice_count; i++) {
      const bool purged = mi_arena_try_purge_range(arena, slice_index + i, 1);
      vinfo->any_purged = vinfo->any_purged || purged;
      vinfo->all_purged = vinfo->all_purged && purged;
    }
  }
  // don't clear the purge bits as that is done atomically be the _bitmap_forall_set_ranges
  // mi_bitmap_clearN(arena->slices_purge, slice_index, slice_count);
  return true; // continue
}

// returns true if anything was purged
static bool mi_arena_try_purge(mi_arena_t* arena, mi_msecs_t now, bool force)
{
  // check pre-conditions
  if (arena->memid.is_pinned) return false;

  // expired yet?
  mi_msecs_t expire = mi_atomic_loadi64_relaxed(&arena->purge_expire);
  if (!force && (expire == 0 || expire > now)) return false;

  // reset expire
  mi_atomic_store_release(&arena->purge_expire, (mi_msecs_t)0);
  mi_subproc_stat_counter_increase(arena->subproc, arena_purges, 1);

  // go through all purge info's  (with max MI_BFIELD_BITS ranges at a time)
  // this also clears those ranges atomically (so any newly freed blocks will get purged next
  // time around)
  mi_purge_visit_info_t vinfo = { now, mi_arena_purge_delay(), true /*all?*/, false /*any?*/};
  _mi_bitmap_forall_setc_ranges(arena->slices_purge, &mi_arena_try_purge_visitor, arena, &vinfo);

  return vinfo.any_purged;
}


static void mi_arenas_try_purge(bool force, bool visit_all, mi_tld_t* tld)
{
  // try purge can be called often so try to only run when needed
  const long delay = mi_arena_purge_delay();
  if (_mi_preloading() || delay <= 0) return;  // nothing will be scheduled

  // check if any arena needs purging?
  mi_subproc_t* subproc = tld->subproc;
  const mi_msecs_t now = _mi_clock_now();
  const mi_msecs_t arenas_expire = mi_atomic_load_acquire(&subproc->purge_expire);
  if (!visit_all && !force && (arenas_expire == 0 || arenas_expire > now)) return;

  const size_t max_arena = mi_arenas_get_count(subproc);
  if (max_arena == 0) return;

  // allow only one thread to purge at a time (todo: allow concurrent purging?)
  static mi_atomic_guard_t purge_guard;
  mi_atomic_guard(&purge_guard)
  {
    // increase global expire: at most one purge per delay cycle
    if (arenas_expire > now) { mi_atomic_store_release(&subproc->purge_expire, now + (delay/10)); }
    const size_t arena_start = tld->thread_seq % max_arena;
    size_t max_purge_count = (visit_all ? max_arena : (max_arena/4)+1);
    bool all_visited = true;
    bool any_purged = false;
    for (size_t _i = 0; _i < max_arena; _i++) {
      size_t i = _i + arena_start;
      if (i >= max_arena) { i -= max_arena; }
      mi_arena_t* arena = mi_arena_from_index(subproc,i);
      if (arena != NULL) {
        if (mi_arena_try_purge(arena, now, force)) {
          any_purged = true;
          if (max_purge_count <= 1) {
            all_visited = false;
            break;
          }
          max_purge_count--;
        }
      }
    }
    if (all_visited && !any_purged) {
      mi_atomic_store_release(&subproc->purge_expire, 0);
    }
  }
}

/* -----------------------------------------------------------
  Visit abandoned pages
----------------------------------------------------------- */

typedef struct mi_abandoned_page_visit_info_s {
  int heap_tag;
  mi_block_visit_fun* visitor;
  void* arg;
  bool visit_blocks;
} mi_abandoned_page_visit_info_t;

static bool abandoned_page_visit(mi_page_t* page, mi_abandoned_page_visit_info_t* vinfo) {
  if (page->heap_tag != vinfo->heap_tag) { return true; } // continue
  mi_heap_area_t area;
  _mi_heap_area_init(&area, page);
  if (!vinfo->visitor(NULL, &area, NULL, area.block_size, vinfo->arg)) {
    return false;
  }
  if (vinfo->visit_blocks) {
    return _mi_heap_area_visit_blocks(&area, page, vinfo->visitor, vinfo->arg);
  }
  else {
    return true;
  }
}

static bool abandoned_page_visit_at(size_t slice_index, size_t slice_count, mi_arena_t* arena, void* arg) {
  MI_UNUSED(slice_count);
  mi_abandoned_page_visit_info_t* vinfo = (mi_abandoned_page_visit_info_t*)arg;
  mi_page_t* page = (mi_page_t*)mi_arena_slice_start(arena, slice_index);
  mi_assert_internal(mi_page_is_abandoned_mapped(page));
  return abandoned_page_visit(page, vinfo);
}

// Visit all abandoned pages in this subproc.
bool mi_abandoned_visit_blocks(mi_subproc_id_t subproc_id, int heap_tag, bool visit_blocks, mi_block_visit_fun* visitor, void* arg) {
  mi_abandoned_page_visit_info_t visit_info = { heap_tag, visitor, arg, visit_blocks };
  MI_UNUSED(subproc_id); MI_UNUSED(heap_tag); MI_UNUSED(visit_blocks); MI_UNUSED(visitor); MI_UNUSED(arg);

  // visit abandoned pages in the arenas
  // we don't have to claim because we assume we are the only thread running (in this subproc).
  // (but we could atomically claim as well by first doing abandoned_reclaim and afterwards reabandoning).
  bool ok = true;
  mi_subproc_t* subproc = _mi_subproc_from_id(subproc_id);
  mi_forall_arenas(subproc, NULL, 0, arena) {
    mi_assert_internal(arena->subproc == subproc);
    for (size_t bin = 0; ok && bin < MI_BIN_COUNT; bin++) {
      // todo: if we had a single abandoned page map as well, this can be faster.
      if (mi_atomic_load_relaxed(&subproc->abandoned_count[bin]) > 0) {
        ok = _mi_bitmap_forall_set(arena->pages_abandoned[bin], &abandoned_page_visit_at, arena, &visit_info);
      }
    }
  }
  mi_forall_arenas_end();
  if (!ok) return false;

  // visit abandoned pages in OS allocated memory
  // (technically we don't need the lock as we assume we are the only thread running in this subproc)
  mi_lock(&subproc->os_abandoned_pages_lock) {
    for (mi_page_t* page = subproc->os_abandoned_pages; ok && page != NULL; page = page->next) {
      ok = abandoned_page_visit(page, &visit_info);
    }
  }

  return ok;
}


/* -----------------------------------------------------------
  Unloading and reloading an arena.
----------------------------------------------------------- */
static bool mi_arena_page_register(size_t slice_index, size_t slice_count, mi_arena_t* arena, void* arg) {
  MI_UNUSED(arg); MI_UNUSED(slice_count);
  mi_assert_internal(slice_count == 1);
  mi_page_t* page = (mi_page_t*)mi_arena_slice_start(arena, slice_index);
  mi_assert_internal(mi_bitmap_is_setN(page->memid.mem.arena.arena->pages, page->memid.mem.arena.slice_index, 1));
  if (!_mi_page_map_register(page)) return false; // break
  mi_assert_internal(_mi_ptr_page(page)==page);
  return true;
}

mi_decl_nodiscard static bool mi_arena_pages_reregister(mi_arena_t* arena) {
  return _mi_bitmap_forall_set(arena->pages, &mi_arena_page_register, arena, NULL);
}

mi_decl_export bool mi_arena_unload(mi_arena_id_t arena_id, void** base, size_t* accessed_size, size_t* full_size) {
  mi_arena_t* arena = _mi_arena_from_id(arena_id);
  if (arena==NULL) {
    return false;
  }
  else if (!arena->is_exclusive) {
    _mi_warning_message("cannot unload a non-exclusive arena (id %zu at %p)\n", arena_id, arena);
    return false;
  }
  else if (arena->memid.memkind != MI_MEM_EXTERNAL) {
    _mi_warning_message("can only unload managed arena's for external memory (id %zu at %p)\n", arena_id, arena);
    return false;
  }

  // find accessed size
  size_t asize;
  // scan the commit map for the highest entry
  // scan the commit map for the highest entry
  size_t idx;
  //if (mi_bitmap_bsr(arena->slices_committed, &idx)) {
  //  asize = (idx + 1)* MI_ARENA_SLICE_SIZE;
  //}
  if (mi_bitmap_bsr(arena->pages, &idx)) {
    mi_page_t* page = (mi_page_t*)mi_arena_slice_start(arena, idx);
    const size_t page_slice_count = page->memid.mem.arena.slice_count;
    asize = mi_size_of_slices(idx + page_slice_count);
  }
  else {
    asize = mi_arena_info_slices(arena) * MI_ARENA_SLICE_SIZE;
  }
  if (base != NULL) { *base = (void*)arena; }
  if (full_size != NULL) { *full_size = arena->memid.mem.os.size;  }
  if (accessed_size != NULL) { *accessed_size = asize; }

  // unregister the pages
  _mi_page_map_unregister_range(arena, asize);

  // set the entry to NULL
  mi_subproc_t* subproc = arena->subproc;
  const size_t count = mi_arenas_get_count(subproc);
  for(size_t i = 0; i < count; i++) {
    if (mi_arena_from_index(subproc, i) == arena) {
      mi_atomic_store_ptr_release(mi_arena_t, &subproc->arenas[i], NULL);
      if (i + 1 == count) { // try adjust the count?
        size_t expected = count;
        mi_atomic_cas_strong_acq_rel(&subproc->arena_count, &expected, count-1);
      }
      break;
    }
  }
  return true;
}

mi_decl_export bool mi_arena_reload(void* start, size_t size, mi_arena_id_t* arena_id) {
  // assume the memory area is already containing the arena
  if (arena_id != NULL) { *arena_id = _mi_arena_id_none(); }
  if (start == NULL || size == 0) return false;
  mi_arena_t* arena = (mi_arena_t*)start;
  mi_memid_t memid = arena->memid;
  if (memid.memkind != MI_MEM_EXTERNAL) {
    _mi_warning_message("can only reload arena's from external memory (%p)\n", arena);
    return false;
  }
  if (memid.mem.os.base != start) {
    _mi_warning_message("the reloaded arena base address differs from the external memory (arena: %p, external: %p)\n", arena, start);
    return false;
  }
  if (memid.mem.os.size != size) {
    _mi_warning_message("the reloaded arena size differs from the external memory (arena size: %zu, external size: %zu)\n", arena->memid.mem.os.size, size);
    return false;
  }
  if (!arena->is_exclusive) {
    _mi_warning_message("the reloaded arena is not exclusive\n");
    return false;
  }

  arena->is_exclusive = true;
  arena->subproc = _mi_subproc();
  if (!mi_arenas_add(arena->subproc, arena, arena_id)) {
    return false;
  }
  if (!mi_arena_pages_reregister(arena)) {
    // todo: clear arena entry in the subproc?
    return false;
  }
  return true;
}


