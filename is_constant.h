/* Macros to determite whether an expression is a constant.
 *
 * Authors:
 *   Martin Uecker <Martin.Uecker@med.uni-goettingen.de>
 *   Evan Nemerson <evan@nemerson.com>
 *
 * Please file bug reports at
 * <https://github.com/nemequ/attic/issues>.
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty.
 *
 * For details, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 ********************************************************************
 *
 * - IS_CONSTEXPR(expr): Return 1 if `expr` is a constant expression,
 *     0 if it is not. If there is no known way to extract this
 *     information from the compiler then this macro will be
 *     undefined.
 *
 * - REQUIRE_CONSTEXPR(expr): If `expr` is known *not* to be a
 *     constant expression, return -1. Otherwise, return `expr`. This
 *     is primarily useful if you want to generate an error when you
 *     accidentally try to create a variable-length array (basically,
 *     the same thing as -Werror=vla). This macro is always available,
 *     but the result may always be `expr` for some compilers.
 *
 *     Usage: char foo[REQUIRE_CONSTEXPR(expr)];
 *
 * - IS_CONSTANT(expr): If the compiler can prove that `expr` is a
 *     compile-time constant, return 1. Otherwise, return 0. It can be
 *     used to choose between code paths which can be constant folded
 *     and those which are fast at run time. This macro is always
 *     available, but the result may always be 0 for some compilers.
 *
 *     Usage:
 *
 *       int x = IS_CONSTANT(expr) ?
 *         CONST_FOLDABLE_MACRO(expr) :
 *         fast_runtime_func(expr);
 *
 * - DIAGNOSTIC_ERROR_VLA: Try to ask the compiler to emit an error if
 *     a VLA is encountered. The advantage is that this doesn't
 *     require modifications to existing code. The disadvantages are
 *     that it's not as widely supported (only GCC and clang, plus
 *     compilers based on them, can emit the diagnsostic right now),
 *     and that using conformant array parameters will trigger the
 *     diagnostic. This is always available, but may do nothing on
 *     some compilers.
 */

#if !defined(IS_CONSTANT_H)
#define IS_CONSTANT_H

#if \
  (defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))) || \
  (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 1600)) || \
  (defined(__TINYC__) && (__TINYC__ >= 919)) || \
  (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 5040000))
#  define IS_CONSTANT(x) __builtin_constant_p(x)
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(__cplusplus) && !defined(__SUNPRO_C)
#  if defined(__INTPTR_TYPE__)
#    define IS_CONSTEXPR(expr) _Generic((1 ? (void*) ((__INTPTR_TYPE__) ((expr) * 0)) : (int*) 0), int*: 1, void*: 0)
#  else
#    include <stdint.h>
#    define IS_CONSTEXPR(expr) _Generic((1 ? (void*) ((intptr_t) ((expr) * 0)) : (int*) 0), int*: 1, void*: 0)
#  endif
#elif /* Compilers known to support sizeof(void) */ \
  defined(__GNUC__) || \
  defined(__INTEL_COMPILER) || \
  defined(__TINYC__) || \
  defined(__TI_COMPILER_VERSION__) || \
  defined(__xlC__)
#  define IS_CONSTEXPR(expr) ( \
       sizeof(void) !=	\
       sizeof(*( \
         1 ? \
           ((void*) ((expr) * 0L) ) : \
           ((struct { char v[sizeof(void) * 2]; } *) 1) \
         ) \
       ) \
     )
#else
#  if !defined(IS_CONSTANT)
#    define IS_CONSTANT(expr) (0)
#  endif
#  define REQUIRE_CONSTEXPR(expr) (expr)
#endif

#if !defined(IS_CONSTANT)
#  define IS_CONSTANT(expr) IS_CONSTEXPR(expr)
#endif

#if !defined(REQUIRE_CONSTEXPR)
#  define REQUIRE_CONSTEXPR(expr) (IS_CONSTEXPR(expr) ? (expr) : (-1))
#endif

#if defined(__has_warning)
#  if __has_warning("-Wvla")
#    define DIAGNOSTIC_ERROR_VLA _Pragma("clang diagnostic error \"-Wvla\"")
#  else
#    define DIAGNOSTIC_ERROR_VLA
#  endif
#elif defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3)))
#  define DIAGNOSTIC_ERROR_VLA _Pragma("GCC diagnostic error \"-Wvla\"")
#elif defined(__IAR_SYSTEMS_ICC__) && (__IAR_SYSTEMS_ICC__ >= 8)
#  define DIAGNOSTIC_ERROR_VLA _Pragma("diag_error=Pe411")
#else
#  define DIAGNOSTIC_ERROR_VLA
#endif

#endif /* defined(IS_CONSTANT_H) */
