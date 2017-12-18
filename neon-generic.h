/* C11 Generic NEON Aliases
 * Evan Nemerson <evan@nemerson.com>
 * Public domain.
 *
 * This uses C11 generic selectors to find the correct NEON intrinsic
 * based on the argument(s). For example, instead of writing
 * `vaddq_s16(a, b)`, you can just write `vadd(a, b)`.
 *
 * I'm not sure when, or if, I'll finish this, so if someone else
 * wants to pick it up please feel free. There is a list of functions
 * at <https://developer.arm.com/technologies/neon/intrinsics>.
 */

#include <arm_neon.h>

#if !defined(NEON_GENERIC_H)
#define NEON_GENERIC_H

#if defined(__aarch64__) || defined(_M_ARM64)
#  define NEON_GENERIC_AARCH64
#endif

#define NEON_GENERIC_FUNC_X(pfx, name, sfx) pfx##name##sfx
#define NEON_GENERIC_FUNC(name, sfx) NEON_GENERIC_FUNC_X(v, name, sfx)

#if defined(NEON_GENERIC_AARCH64)
#  define vadd(a, b)                            \
  _Generic((a),                                 \
    int8x8_t:    NEON_GENERIC_FUNC(add,   _s8),	\
    int8x16_t:   NEON_GENERIC_FUNC(add,  q_s8), \
    int16x4_t:   NEON_GENERIC_FUNC(add,  _s16),	\
    int16x8_t:   NEON_GENERIC_FUNC(add, q_s16), \
    int32x2_t:   NEON_GENERIC_FUNC(add,  _s32), \
    int32x4_t:   NEON_GENERIC_FUNC(add, q_s32), \
    int64x1_t:   NEON_GENERIC_FUNC(add,  _s64), \
    int64x2_t:   NEON_GENERIC_FUNC(add, q_s64), \
    uint8x8_t:   NEON_GENERIC_FUNC(add,   _u8), \
    uint8x16_t:  NEON_GENERIC_FUNC(add,  q_u8), \
    uint16x4_t:  NEON_GENERIC_FUNC(add,  _u16), \
    uint16x8_t:  NEON_GENERIC_FUNC(add, q_u16), \
    uint32x2_t:  NEON_GENERIC_FUNC(add,  _u32), \
    uint32x4_t:  NEON_GENERIC_FUNC(add, q_u32), \
    uint64x1_t:  NEON_GENERIC_FUNC(add,  _u64), \
    uint64x2_t:  NEON_GENERIC_FUNC(add, q_u64), \
    int64_t:     NEON_GENERIC_FUNC(add, d_s64), \
    uint64_t:    NEON_GENERIC_FUNC(add, d_u64), \
    float32x2_t: NEON_GENERIC_FUNC(add,  _f32), \
    float32x4_t: NEON_GENERIC_FUNC(add, q_f32), \
    float64x1_t: NEON_GENERIC_FUNC(add,  _f64), \
    float64x2_t: NEON_GENERIC_FUNC(add, q_f64) \
  )
#else
#  define vadd(a, b)                            \
  _Generic((a),                                 \
    int8x8_t:    NEON_GENERIC_FUNC(add,   _s8), \
    int8x16_t:   NEON_GENERIC_FUNC(add,  q_s8), \
    int16x4_t:   NEON_GENERIC_FUNC(add,  _s16), \
    int16x8_t:   NEON_GENERIC_FUNC(add, q_s16), \
    int32x2_t:   NEON_GENERIC_FUNC(add,  _s32), \
    int32x4_t:   NEON_GENERIC_FUNC(add, q_s32), \
    int64x1_t:   NEON_GENERIC_FUNC(add,  _s64), \
    int64x2_t:   NEON_GENERIC_FUNC(add, q_s64), \
    uint8x8_t:   NEON_GENERIC_FUNC(add,   _u8), \
    uint8x16_t:  NEON_GENERIC_FUNC(add,  q_u8), \
    uint16x4_t:  NEON_GENERIC_FUNC(add,  _u16), \
    uint16x8_t:  NEON_GENERIC_FUNC(add, q_u16), \
    uint32x2_t:  NEON_GENERIC_FUNC(add,  _u32), \
    uint32x4_t:  NEON_GENERIC_FUNC(add, q_u32), \
    uint64x1_t:  NEON_GENERIC_FUNC(add,  _u64), \
    uint64x2_t:  NEON_GENERIC_FUNC(add, q_u64), \
    float32x2_t: NEON_GENERIC_FUNC(add,  _f32), \
    float32x4_t: NEON_GENERIC_FUNC(add, q_f32)  \
  )
#endif

#endif /* !defined(NEON_GENERIC_H) */
