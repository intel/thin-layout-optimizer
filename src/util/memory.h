#ifndef SRC_D_UTIL_D_MEMORY_H_
#define SRC_D_UTIL_D_MEMORY_H_

#include "src/util/bits.h"
#include "src/util/compiler.h"
#include "src/util/vec.h"

#include <assert.h>
#include <malloc.h>
#include <stdint.h>
#include <sys/mman.h>

#include "src/util/debug.h"

////////////////////////////////////////////////////////////////////////////////
// Wrappers/helpers for memory allocation.


namespace tlo {

static constexpr size_t k_page_size = 4096;

template<typename Tp_t>
    requires(std::is_pointer<Tp_t>::value)
static constexpr Tp_t align_ptr(Tp_t p, uintptr_t alignment) {
    uintptr_t intptr = reinterpret_cast<uintptr_t>(p);
    intptr += (alignment - 1);
    intptr &= (-alignment);
    return reinterpret_cast<Tp_t>(intptr);
}


static char *
str_malloc(size_t len) {
    char * s = reinterpret_cast<char *>(malloc(len + 1));
    assert(s != nullptr);
    s[len] = '\0';
    return s;
}

static void
str_free(char * s, size_t len) {
    free(s);
    (void)len;
}

static void
str_free(char const * s, size_t len) {
    str_free(const_cast<char *>(s), len + 1);
}


static void *
buf_alloc(size_t len) {   
    void * p = malloc(len);
    assert(p != nullptr);
    return p;
}

static void *
buf_zalloc(size_t len) {
    void * p = calloc(len, 1);
    assert(p != nullptr);
    return p;
}

static void *
buf_realloc(void * oldp, size_t oldlen, size_t newlen) {
    void * p = realloc(oldp, newlen);
    assert(p != nullptr || newlen == 0);
    return p;
    (void)oldlen;
}

static void
buf_free(void * p, size_t len) {
    free(p);
    (void)len;
}


static void
buf_free(void const * p, size_t len) {
    buf_free(const_cast<void *>(p), len);
}

template<typename T_t>
static T_t *
arr_alloc(size_t nelem) {
    return reinterpret_cast<T_t *>(buf_alloc(sizeof(T_t) * nelem));
}

template<typename T_t>
static T_t *
arr_zalloc(size_t nelem) {
    return reinterpret_cast<T_t *>(buf_zalloc(sizeof(T_t) * nelem));
}

template<typename T_t>
static T_t *
arr_realloc(T_t * oldp, size_t old_nelem, size_t new_nelem) {
    return reinterpret_cast<T_t *>(
        buf_realloc(oldp, sizeof(T_t) * old_nelem, sizeof(T_t) * new_nelem));
}

template<typename T_t>
static void
arr_free(T_t * p, size_t nelem) {
    buf_free(reinterpret_cast<void *>(p), sizeof(T_t) * nelem);
}

template<typename T_t>
static void
arr_free(const T_t * p, size_t nelem) {
    arr_free<T_t>(const_cast<T_t *>(p), nelem);
}


namespace sys {
static void *
getmem(size_t nbytes) {
    // If sanitizer is enabled just use malloc, otherwise we basically defeat
    // the sanitizer.
#ifdef TLO_SANITIZED
    void * p = buf_zalloc(nbytes);
#else
    void * p = mmap(nullptr, nbytes, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(p != MAP_FAILED && p != nullptr);
#endif
    return p;
}

static void *
get_safemem(size_t nbytes) {
#ifdef TLO_SANITIZED
    return getmem(nbytes);
#else
    // Basically getmem but ensures that surrounding pages have no permissions
    // (so overread/overwrite will properly fault).
    nbytes      = roundup(nbytes, 4096);
    uint8_t * p = reinterpret_cast<uint8_t *>(mmap(
        nullptr, nbytes + 8192, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    assert(p != MAP_FAILED && p != nullptr);
    assert(mprotect(p + 4096, nbytes, PROT_READ | PROT_WRITE) == 0);
    return p + 4096;
#endif
}

static void
freemem(void * p, size_t nbytes) {
#ifdef TLO_SANITIZED
    buf_free(p, nbytes);
#else
    munmap(p, nbytes);
#endif
}

static void
freemem(void const * p, size_t nbytes) {
    freemem(const_cast<void *>(p), nbytes);
}

static void
free_safemem(void * p, size_t nbytes) {
#ifdef TLO_SANITIZED
    freemem(p, nbytes);
#else
    munmap(reinterpret_cast<uint8_t *>(p) - 4096, nbytes + 8192);
#endif
}

static void
free_safemem(void const * p, size_t nbytes) {
    free_safemem(const_cast<void *>(p), nbytes);
}


}  // namespace sys

static constexpr size_t k_bump_alloc_page_size = static_cast<size_t>(1) << 21;

// Bump allocator. Just a little efficiency thing. Allocated `Tk_buf_sz` bytes
// at a time.
template<size_t Tk_buf_sz = k_bump_alloc_page_size>
struct bump_alloc_t {
    struct free_pair_t {
        void * p_;
        size_t sz_;

        constexpr free_pair_t() = default;
        constexpr free_pair_t(void * p, size_t sz) noexcept : p_(p), sz_(sz) {}
    };
    static_assert(has_okay_type_traits<free_pair_t>::value);

    uint8_t *          cur_;
    size_t             remaining_;
    vec_t<free_pair_t> to_free_;

    bump_alloc_t() noexcept
        : cur_(reinterpret_cast<uint8_t *>(sys::getmem(Tk_buf_sz))),
          remaining_(Tk_buf_sz) {}
    ~bump_alloc_t() {
        sys::freemem(cur_ - (Tk_buf_sz - remaining_), Tk_buf_sz);
        for (const free_pair_t & to_free : to_free_) {
            sys::freemem(to_free.p_, to_free.sz_);
        }
    }

    // Actual API. Get `nbytes` aligned to `alignment`.
    void *
    get(size_t nbytes, size_t alignment) noexcept {
        // TODO: Maybe just templatize so alignment can be worked out by type.
        assert(alignment != 0 && alignment <= k_page_size &&
               is_pow2(alignment));
        void *    p;
        uint8_t * aligned_cur = align_ptr(cur_, alignment);
        nbytes += static_cast<size_t>(aligned_cur - cur_);
        if (TLO_UNLIKELY(nbytes > remaining_)) {
            if (TLO_UNLIKELY(nbytes > Tk_buf_sz)) {
                p = sys::getmem(nbytes);
                to_free_.emplace_back(p, nbytes);
                return p;
            }
            to_free_.emplace_back(
                reinterpret_cast<void *>(cur_ - (Tk_buf_sz - remaining_)),
                Tk_buf_sz);
            p          = sys::getmem(Tk_buf_sz);
            cur_       = reinterpret_cast<uint8_t *>(p) + nbytes;
            remaining_ = Tk_buf_sz - nbytes;
            return p;
        }


        p = reinterpret_cast<void *>(aligned_cur);
        cur_ += nbytes;
        remaining_ -= nbytes;

        return p;
    }

    void
    try_unget(size_t sz) noexcept {
        size_t nalloced = Tk_buf_sz - remaining_;
        if (TLO_LIKELY(nalloced >= sz)) {
            remaining_ += sz;
            cur_ -= sz;
        }
    }


    // get zerod memory. sys memory always zerod so its pretty simple.
    void *
    getz(size_t nbytes, size_t alignment) noexcept {
        return get(nbytes, alignment);
    }


    template<typename T_t>
    T_t *
    get_arr(size_t nelem) noexcept {
        return reinterpret_cast<T_t *>(get(sizeof(T_t) * nelem, alignof(T_t)));
    }

    template<typename T_t>
    T_t *
    getz_arr(size_t nelem) noexcept {
        return get_arr<T_t>(nelem);
    }

    template<typename T_t>
    T_t *
    getT() noexcept {
        return get_arr<T_t>(1);
    }

    template<typename T_t>
    T_t *
    getzT() noexcept {
        return getz_arr<T_t>(1);
    }


    template<typename T_t>
    void
    try_ungetT(size_t nelem) noexcept {
        return try_unget(nelem * sizeof(T_t));
    }
};


}  // namespace tlo

#endif
