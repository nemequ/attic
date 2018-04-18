#if !defined(IS_CONSTANT_H)
#define IS_CONSTANT_H

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  if defined(__INTPTR_TYPE__)
#    define IS_CONSTANT(x) _Generic((1 ? (void*) ((__INTPTR_TYPE__) ((x) * 0)) : (int*) 0), int*: 1, void*: 0)
#  else
#    define IS_CONSTANT(x) _Generic((1 ? (void*) ((x) * 0) : (int*) 0), int*: 1, void*: 0)
#  endif
#elif \
  (defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))) || \
  (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 1600)) || \
  (defined(__TINYC__) && (__TINYC__ >= 919)) || \
  (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 5040000))
#  define IS_CONSTANT(x) __builtin_constant_p(x)
#elif \
  defined(__TI_COMPILER_VERSION__) || \
  defined(__xlC__)
#  define IS_CONSTANT(x) ( \
       sizeof(void) !=	\
       sizeof(*( \
         1 ? \
           ((void*) ((x) * 0L) ) : \
           ((struct { char v[sizeof(void) * 2]; } *) 1) \
         ) \
       ) \
     )
#endif

#endif /* defined(IS_CONSTANT_H) */
