/* copied from pyconfig.h */
#define SIZEOF_SHORT 2
#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#define ALIGNOF_LONG 4
#define SIZEOF_LONG_LONG 8
#define SIZEOF_DOUBLE 8
#define SIZEOF_FLOAT 4

#if defined(__APPLE__)

# undef SIZEOF_LONG
# undef SIZEOF_PTHREAD_T
# undef SIZEOF_SIZE_T
# undef SIZEOF_TIME_T
# undef SIZEOF_VOID_P
# undef SIZEOF__BOOL
# undef SIZEOF_UINTPTR_T
# undef SIZEOF_PTHREAD_T
# undef WORDS_BIGENDIAN
# undef DOUBLE_IS_ARM_MIXED_ENDIAN_IEEE754
# undef DOUBLE_IS_BIG_ENDIAN_IEEE754
# undef DOUBLE_IS_LITTLE_ENDIAN_IEEE754
# undef HAVE_GCC_ASM_FOR_X87

#    undef VA_LIST_IS_ARRAY
#    if defined(__LP64__) && defined(__x86_64__)
#        define VA_LIST_IS_ARRAY 1
#    endif

#    undef HAVE_LARGEFILE_SUPPORT
#    ifndef __LP64__
#         define HAVE_LARGEFILE_SUPPORT 1
#    endif

#    undef SIZEOF_LONG
#    ifdef __LP64__
#        define SIZEOF__BOOL            1
#        define SIZEOF__BOOL            1
#        define SIZEOF_LONG             8
#        define SIZEOF_PTHREAD_T        8
#        define SIZEOF_SIZE_T           8
#        define SIZEOF_TIME_T           8
#        define SIZEOF_VOID_P           8
#        define SIZEOF_UINTPTR_T        8
#        define SIZEOF_PTHREAD_T        8
#    else
#        ifdef __ppc__
#           define SIZEOF__BOOL         4
#        else
#           define SIZEOF__BOOL         1
#        endif
#        define SIZEOF_LONG             4
#        define SIZEOF_PTHREAD_T        4
#        define SIZEOF_SIZE_T           4
#        define SIZEOF_TIME_T           4
#        define SIZEOF_VOID_P           4
#        define SIZEOF_UINTPTR_T        4
#        define SIZEOF_PTHREAD_T        4
#    endif

#    if defined(__LP64__)
/* MacOSX 10.4 (the first release to support 64-bit code
 * at all) only supports 64-bit in the UNIX layer.
 * Therefore suppress the toolbox-glue in 64-bit mode.
 */

 /* In 64-bit mode setpgrp always has no arguments, in 32-bit
  * mode that depends on the compilation environment
  */
#       undef SETPGRP_HAVE_ARG

#    endif

#ifdef __BIG_ENDIAN__
#define WORDS_BIGENDIAN 1
#define DOUBLE_IS_BIG_ENDIAN_IEEE754
#else
#define DOUBLE_IS_LITTLE_ENDIAN_IEEE754
#endif /* __BIG_ENDIAN */

#ifdef __i386__
# define HAVE_GCC_ASM_FOR_X87
#endif

  /*
   * The definition in pyconfig.h is only valid on the OS release
   * where configure ran on and not necessarily for all systems where
   * the executable can be used on.
   *
   * Specifically: OSX 10.4 has limited supported for '%zd', while
   * 10.5 has full support for '%zd'. A binary built on 10.5 won't
   * work properly on 10.4 unless we suppress the definition
   * of PY_FORMAT_SIZE_T
   */
#undef  PY_FORMAT_SIZE_T


#endif


#define NPY_SIZEOF_SHORT SIZEOF_SHORT
#define NPY_SIZEOF_INT SIZEOF_INT
#define NPY_SIZEOF_LONG SIZEOF_LONG
#define NPY_SIZEOF_FLOAT 4
#define NPY_SIZEOF_COMPLEX_FLOAT 8
#define NPY_SIZEOF_DOUBLE 8
#define NPY_SIZEOF_COMPLEX_DOUBLE 16
#define NPY_SIZEOF_LONGDOUBLE 8
#define NPY_SIZEOF_COMPLEX_LONGDOUBLE 16
#define NPY_SIZEOF_PY_INTPTR_T 8
#define NPY_SIZEOF_OFF_T 4
#define NPY_SIZEOF_PY_LONG_LONG 8
#define NPY_SIZEOF_LONGLONG 8
#define NPY_NO_SIGNAL 1
#define NPY_NO_SMP 0
#define NPY_HAVE_DECL_ISNAN
#define NPY_HAVE_DECL_ISINF
#define NPY_HAVE_DECL_SIGNBIT
#define NPY_HAVE_DECL_ISFINITE
#define NPY_USE_C99_COMPLEX 1
#define NPY_USE_C99_FORMATS 1
#define NPY_VISIBILITY_HIDDEN 
#define NPY_ABI_VERSION 0x01000009
#define NPY_API_VERSION 0x00000010

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif
