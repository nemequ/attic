/* Safer (less unsafe?) memory management in C
 * Code from <https://github.com/nemequ/attic/>
 *
 * Originally written by Evan Nemerson <evan@nemerson.com>.  This is a
 * work-in-progress; feedback is appreciated, especially bug reports
 * and constructive criticism.  I may eventually move this code into
 * Portable Snippets (<https://github.com/nemequ/portable-snippets/>),
 * so if you can't find the latest version in the repo above try
 * there.
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty.
 *
 * For details see http://creativecommons.org/publicdomain/zero/1.0/
 *
 *********************************************************************
 *
 * The basic idea is to eliminate several types of common mistakes
 * programmers make. Some of the advantages of this API include:
 *
 *  * Catches many type mismatches which C allows, mostly due to C
 *    allowing implicit casts between void* and any other pointer
 *    type.
 *  * Reduces manual, error-prone casting (like the kind required for
 *    C++ compatibility). Instead of `p = (int*) malloc(sizeof(int))`
 *    you can just do `p = ennew(int);` and it will work even in a C++
 *    compiler.
 *  * Checks for overflows when calculating the product of size and
 *    nmemb (for a good overview of this problem see
 *    <http://lteo.net/blog/2014/10/28/reallocarray-in-openbsd-integer-overflow-detection-for-free/>)
 *
 * This is accomplished largely by using types in the API instead of
 * length in bytes, and casting to the proper type withing the API
 * instead of relying on void* and implicit conversions. For example,
 * in standard C the malloc function looks like:
 *
 *   void* malloc(size_t size);
 *
 * For ennew(), on the other hand, we replace the size parameter with
 * a type and cast the result, so we end up with an API that looks
 * like:
 *
 *   T* ennew(T type);
 *
 * Obviously that's not a real prototype; we use a macro which looks like:
 *
 *   #define ennew(T) ((T*) malloc(sizeof(T)))
 *
 * Or, in C++:
 *
 *   #define ennew(T) static_cast<T*>(malloc(sizeof(T)))
 *
 * Other functions may be a bit more complex, but the basic idea
 * remains the same: do whatever we can to get the compiler to emit a
 * diagnostic when you do something wrong. For example, consider these
 * two lines:
 *
 *   int* a = malloc(sizeof(char));
 *   int* b = ennew(char);
 *
 * Compilers will happily accept the first line since malloc returns a
 * void*, which is then implicitly converted to an int* when assigning
 * to a. There is a good chance this will cause problems later since
 * you've only allocated enough room for a char (generally 1 byte) but
 * you're treating it as an int (generally 4 bytes).
 *
 * The second line, however, will cause most compilers to generate a
 * warning or error about converting a char* to an int*.
 *
 * Of course, you'll often want to allocate space for multiple
 * objects. That's easy to do with ennewa() (note the "a" suffix):
 *
 *   int* c = ennewa(int, 512);
 *
 * Which will allocate an array of 512 integers and automatically cast
 * the result to the correct type. Not only does this provide better
 * type safety than `malloc(sizeof(int) * 512)`, it is also easier to
 * use, and protects against integer overflow issues.
 *
 * There are also variants of ennew and ennewa, called ennew0 and
 * ennewa0, which return memory set to 0, as well as safer
 * alternatives to realloc (enrealloc and enresize).  There is even a
 * version of free which returns NULL, which provides a convenient way
 * to clear a pointer (set it to NULL) instead of leaving it pointing
 * at an invalid address.
 *
 *********************************************************************
 *
 * To use this, just drop the header into your project and include
 * it. No build system magic is required.
 *
 * Relationship to standard malloc/calloc/realloc/free:
 *
 *  * malloc(sizeof(T))                      -> ennew(T);
 *  * calloc(1, sizeof(T))                   -> ennew0(T);
 *  * malloc(sizeof(T) * nmemb)              -> ennewa(T, nmemb);
 *  * calloc(nmemb, sizeof(T))               -> ennewa0(T, nmemb);
 *  * realloc(ptr, sizeof(T) * nmemb)        -> enrealloc(ptr, T, nmemb);
 *  * tmp = realloc(ptr, sizeof(T) * nmemb);
 *    if (tmp == NULL)
 *      free(ptr);
 *    else
 *      ptr = tmp;                           -> ptr = enrealloc(ptr, T, nmemb);
 *  * free(ptr);
 *    ptr = NULL;                            -> ptr = enfree(ptr);
 *
 *********************************************************************
 *
 * ennew(Type T)
 * ennew0(Type T)
 *
 *   ennew() allocates space for a single T, ennew0() does the same
 *   thing except the result is 0-initialized.
 *
 * T* ennewa(Type T, size_t nmemb)
 * T* ennewa0(Type T, size_t nmemb)
 *
 *   ennewa() allocates an nmemb-long array of type T. ennewa0() does
 *   the same thing except it will initialize the array to 0.
 *
 * T* enrealloc(T* ptr, Type T, size_t nmemb):
 *
 *   This is like realloc(), only it takes a type the number of
 *   elements instead of just a size. And, of course, it returns a T*
 *   instead void*.
 *
 * T* enresize(T* ptr, Type T, size_t nmemb):
 *
 *   enresize() is like enrealloc(), except that if reallocation fails
 *   the old data is freed.
 *
 * T* enfree(T* ptr)
 *
 *   Frees ptr, and returns NULL.  This is just to make it a little
 *   easier to clear the pointer in addition to freeing it (for
 *   example, `ptr = enfree(ptr)` sets `ptr` to NULL).
 *
 *   On some compilers, namely those which implement GNU extensions
 *   such as GCC and clang as well as C++ compilers, the return value
 *   has the same type as the input, so you should get a warning if
 *   you try to do something like `x = enfree(y)` when x and y are
 *   different types. Otherwise T is void and you're on your own.
 *
 *********************************************************************
 *
 * As far as I can tell, the only (valid) thing this API really makes
 * harder is mixing types. For example, if you want to make a single
 * allocation for some metadata struct and the data itself (such as an
 * image with width and height fields).  It's a bit of a corner case,
 * though, and IMHO the additional safety for most use cases vastly
 * outweighs having to do something like `(Image*) ennewa(char, len)`
 * instead of just `malloc(len)`, especially since calculating len is
 * already likely non-trivial since you probably need to take struct
 * size, alignment, and padding into account.
 *
 * In general, all you need to do is #include this file. If you want
 * to use custom functions instead of malloc/calloc/realloc/free you
 * can define EN_MALLOC/CALLOC/REALLOC/FREE first.
 *
 * The API should work with any compiler, but some compilers (GCC,
 * clang, ICC, etc.) can provide more checks due to extensions like
 * typeof() and __builtin_types_compatible_p().
 *
 * For best results you should also include Hedley
 * (<https://nemequ.github.io/hedley>) before including this file, but
 * it is not required.
 *
 * Partial TODO:
 *
 *  * Clean up.
 *  * Figure out which other compilers and versions support
 *    __builtin_types_compatible_p (IIRC ARM does...).
 *  * Look at what other compilers (suncc, TI, IBM, etc.) offer in the
 *    way of extensions which could help.
 *  * Add annotations (GCC-style attributes and SAL) as appropriate.
 *  * Hard dependency on Hedley? Would make a bunch of stuff easier,
 *    and clean things up a bit.
 *  * C11 implementation of EN_CHECK_TYPES_AND_EXEC, using _Generic?
 *    Not sure it's possible, but I'd like to spend some time thinking
 *    about it.
 */

#if !defined(ENMEM_H)
#define ENMEM_H

#if !defined(EN_MALLOC)
#  define EN_MALLOC malloc
#endif
#if !defined(EN_CALLOC)
#  define EN_CALLOC calloc
#endif
#if !defined(EN_REALLOC)
#  define EN_REALLOC realloc
#endif
#if !defined(EN_FREE)
#  define EN_FREE free
#endif
#if !defined(EN_INCOMPATIBLE_TYPES)
#  define EN_INCOMPATIBLE_TYPES "incompatible types"
#endif

#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#if defined(HEDLEY_UNLIKELY)
#  define EN_UNLIKELY(expr) HEDLEY_UNLIKELY(expr)
#elif defined(__GNUC__) && (__GNUC__ >= 3)
#  define EN_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#else
#  define EN_UNLIKELY(expr) (!!(expr))
#endif

#define EN_NO_OVERFLOW (((size_t) 1) << (sizeof(size_t) * 4))

#if defined(__has_builtin)
#  if __has_builtin(__builtin_mul_overflow) && !defined(__ibmxl__)
#    define EN_MUL_OVERFLOW
#  endif
#elif defined(__GNUC__) && (__GNUC__ > 4) && !defined(__INTEL_COMPILER)
#  define EN_MUL_OVERFLOW
#endif

#if defined(__cplusplus)
#elif \
  (defined(__GNUC__) && ((__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)))
#  define EN_TYPES_COMPATIBLE_P
#endif

#if defined(__has_feature)
#  if __has_feature(c_static_assert)
#    define EN_STATIC_ASSERT
#  endif
#elif \
  (defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))) || \
  (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 1500))
#  define EN_STATIC_ASSERT
#endif

#if defined(__cplusplus)
template<typename T> static T* enchktype_(T* ptr, T* stmt) { (void) ptr; return stmt; }
#elif defined(EN_TYPES_COMPATIBLE_P) && defined(EN_STATIC_ASSERT) && 0
#  define EN_CHECK_TYPES_AND_EXEC(t1, t2, stmt) (__extension__ ({ \
       _Static_assert(__builtin_types_compatible_p(t1, t2), EN_INCOMPATIBLE_TYPES); \
       (stmt); \
     }))
#elif defined(EN_TYPES_COMPATIBLE_P)
/* If you're getting an error from this about an array having a
 * negative size, it's because your types don't match. This is just a
 * trick to get the compiler to emit an error at compile-time; newer
 * compilers have more descriptive error messages, but this is the
 * best we could do for your compiler. */
#  define EN_CHECK_TYPES_AND_EXEC(t1, t2, stmt) (__extension__ ({ \
       ((void)sizeof(char[1 - 2*!(__builtin_types_compatible_p(t1, t2))])); \
       (stmt); \
     }))
#else
#  define EN_CHECK_TYPES_AND_EXEC(t1, t2, stmt) (stmt)
#endif

#if defined(__cplusplus)
  template<typename T>
  static T* enfree(T* ptr) {
    free(static_cast<void*>(ptr));
    return NULL;
  }
#elif defined(__GNUC__)
#  define enfree(ptr) ((__typeof__(*ptr)*) (EN_FREE(ptr), NULL))
#else
#  define enfree(ptr) (EN_FREE(ptr), (void*) NULL)
#endif

#if defined(__cplusplus)
#  define ennew(T) static_cast<T*>(EN_MALLOC(sizeof(T)))
#  define ennew0(T) static_cast<T*>(EN_CALLOC(1, sizeof(T)))
#else
#  define ennew(T) ((T*) EN_MALLOC(sizeof(T)))
#  define ennew0(T) ((T*) EN_CALLOC(1, sizeof(T)))
#endif

static void* ennewa_(size_t size, size_t nmemb) {
  size_t alloc_size;

  if (EN_UNLIKELY(!nmemb))
    return NULL;

#if defined(EN_MUL_OVERFLOW)
  if (EN_UNLIKELY(__builtin_mul_overflow(size, nmemb, &alloc_size))) {
    errno = ENOMEM;
    return NULL;
  }
#else
  if (EN_UNLIKELY(EN_UNLIKELY(nmemb >= EN_NO_OVERFLOW) || EN_UNLIKELY(size >= EN_NO_OVERFLOW)) && EN_UNLIKELY((SIZE_MAX / nmemb) < size)) {
    errno = ENOMEM;
    return NULL;
  } else {
    alloc_size = size * nmemb;
  }
#endif

  return EN_MALLOC(alloc_size);
}
#if defined(__cplusplus)
#  define ennewa(T, nmemb) static_cast<T*>(ennewa_(sizeof(T), nmemb))
#  define ennewa0(T, nmemb) static_cast<T*>(EN_CALLOC(nmemb, sizeof(T)))
#else
#  define ennewa(T, nmemb) ((T*) ennewa_(sizeof(T), nmemb))
#  define ennewa0(T, nmemb) ((T*) EN_CALLOC(nmemb, sizeof(T)))
#endif

static void* enrealloc_(void* ptr, size_t size, size_t nmemb) {
  size_t alloc_size;

  if (EN_UNLIKELY(!nmemb))
    return enfree(ptr);

#if defined(EN_MUL_OVERFLOW)
  if (EN_UNLIKELY(__builtin_mul_overflow(size, nmemb, &alloc_size))) {
    errno = ENOMEM;
    return NULL;
  }
#else
  if ((EN_UNLIKELY(nmemb >= EN_NO_OVERFLOW) || EN_UNLIKELY(size >= EN_NO_OVERFLOW)) && EN_UNLIKELY((SIZE_MAX / nmemb) < size)) {
    errno = ENOMEM;
    return NULL;
  }
  else {
    alloc_size = size * nmemb;
  }
#endif

  return EN_REALLOC(ptr, alloc_size);
}
#if defined(__cplusplus)
#  define enrealloc(ptr, T, nmemb) enchktype_(ptr, static_cast<T*>(enrealloc_(ptr, sizeof(T), nmemb)))
#else
#  define enrealloc(ptr, T, nmemb) EN_CHECK_TYPES_AND_EXEC(__typeof__(ptr[0]), T, ((T*) enrealloc_(ptr, sizeof(T), nmemb)))
#endif

static void* enresize_(void* ptr, size_t size, size_t nmemb) {
  void* tmp_ = enrealloc_(ptr, size, nmemb);
  if (EN_UNLIKELY(tmp_ == NULL))
    EN_FREE(ptr);
  return tmp_;
}
#if defined(__cplusplus)
#  define enresize(ptr, T, nmemb) enchktype_(ptr, static_cast<T*>(enresize_(ptr, sizeof(T), nmemb)))
#else
#  define enresize(ptr, T, nmemb) EN_CHECK_TYPES_AND_EXEC(__typeof__(ptr[0]), T, ((T*) enresize_(ptr, sizeof(T), nmemb)))
#endif

#endif /* !defined(ENMEM_H) */
