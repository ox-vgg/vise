/**
 * James Philbin <philbinj@gmail.com>
 * Engineering Department
 * University of Oxford
 * Copyright (C) 2006. All rights reserved.
 * 
 * Use and modify all you like, but do NOT redistribute. No warranty is 
 * expressed or implied. No liability or responsibility is assumed.
 */
#ifndef __JP_DIST2_HPP
#define __JP_DIST2_HPP

#include <inttypes.h>

// *** (double, double) ***
inline
double
jp_dist_l2_dd_slow(const double* a, const double* b, unsigned D)
{
  double ret = 0.0;
  for (unsigned d=0; d<D; ++d) {
    ret += (a[d]-b[d])*(a[d]-b[d]);
  }
  return ret;
}

inline
double
jp_dist_l2(const double* a, const double* b, unsigned D)
{
  return jp_dist_l2_dd_slow(a, b, D);
}

// *** (float,float) ***
inline
float
jp_dist_l2_ff_slow(const float* a, const float* b, unsigned D)
{
  float ret = 0.0f;
  for (unsigned d=0; d<D; ++d) {
    ret += (a[d] - b[d])*(a[d] - b[d]);
  }
  return ret;
}

#ifdef __SSE__
inline
float
jp_dist_l2_ff_sse(const float* a, const float* b, unsigned D)
{
#if 0
  float sum_sqr __attribute__ ((aligned(16)));
  unsigned d = 0;
  __asm__ (
    "movl %[D], %%eax\n\t"
    "andl $-16, %%eax\n\t"

    "pxor %%xmm6, %%xmm6\n\t"
    "pxor %%xmm7, %%xmm7\n\t"

    "loop_begin_%=:\n\t"

    "movups 0(%[a]), %%xmm0\n\t"
    "movups 0(%[b]), %%xmm1\n\t"

    "movups 16(%[a]), %%xmm2\n\t"
    "movups 16(%[b]), %%xmm3\n\t"

    "subps %%xmm1, %%xmm0\n\t"
    "subps %%xmm3, %%xmm2\n\t"

    "mulps %%xmm0, %%xmm0\n\t"
    "mulps %%xmm2, %%xmm2\n\t"

    "addps %%xmm0, %%xmm6\n\t"
    "addps %%xmm2, %%xmm7\n\t"

    "movups 32(%[a]), %%xmm0\n\t"
    "movups 32(%[b]), %%xmm1\n\t"

    "movups 48(%[a]), %%xmm2\n\t"
    "movups 48(%[b]), %%xmm3\n\t"

    "subps %%xmm1, %%xmm0\n\t"
    "subps %%xmm3, %%xmm2\n\t"

    "mulps %%xmm0, %%xmm0\n\t"
    "mulps %%xmm2, %%xmm2\n\t"

    "addps %%xmm0, %%xmm6\n\t"
    "addps %%xmm2, %%xmm7\n\t"

    "add $64, %[a]\n\t"
    "add $64, %[b]\n\t"
    "add $16, %[d]\n\t"
    "sub $16, %%eax\n\t"
    "test %%eax, %%eax\n\t"
    "jnz loop_begin_%=\n\t"

    "addps %%xmm6, %%xmm7\n\t"
    "pshufd $14, %%xmm7, %%xmm6\n\t"
    "addps %%xmm6, %%xmm7\n\t"
    "pshufd $1, %%xmm7, %%xmm6\n\t"
    "addps %%xmm6, %%xmm7\n\t"
    "movd %%xmm7, %[sum_sqr]\n\t"
   :
    [sum_sqr] "=m" (sum_sqr),
    [a] "=r" (a),
    [b] "=r" (b),
    [d] "=r" (d)
   :
    "1" (a),
    "2" (b),
    "3" (d),
    [D] "m" (D)
   :
    "%eax", "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm6", "%xmm7"
  );
  for ( ; d < D; ++d) {
    sum_sqr += (a[d] - b[d])*(a[d] - b[d]);
  }
  return sum_sqr;
#else
    float dsq;
    __asm__ (
      "pxor %%xmm4, %%xmm4\n\t"
      "pxor %%xmm5, %%xmm5\n\t"
      "mov %[D], %%eax\n\t"

      // --- Unroll 8 ---
      "test $-8, %%eax\n\t"
      "jz .L_11_%=\n\t"
      ".L_10_%=:\n\t"
      "movlps 0(%[a]), %%xmm0\n\t"
      "movhps 8(%[a]), %%xmm0\n\t"
      "movlps 0(%[b]), %%xmm1\n\t"
      "movhps 8(%[b]), %%xmm1\n\t"
      "movlps 16(%[a]), %%xmm2\n\t"
      "movhps 24(%[a]), %%xmm2\n\t"
      "movlps 16(%[b]), %%xmm3\n\t"
      "movhps 24(%[b]), %%xmm3\n\t"
      
      "subps %%xmm1, %%xmm0\n\t"
      "subps %%xmm3, %%xmm2\n\t"

      "mulps %%xmm0, %%xmm0\n\t"
      "add $32, %[a]\n\t"
      "mulps %%xmm2, %%xmm2\n\t"

      "addps %%xmm0, %%xmm4\n\t"
      "add $32, %[b]\n\t"
      "addps %%xmm2, %%xmm5\n\t"

      "sub $8, %%eax\n\t"

      "test $-8, %%eax\n\t"
      "jnz .L_10_%=\n\t"
      ".L_11_%=:\n\t"
      // --- End ---

      "test $7, %%eax\n\t"
      "jz .L_14_%=\n\t"

      // --- Finish up 4 ---
      "test $4, %%eax\n\t"
      "jz .L_12_%=\n\t"
      
      "movlps 0(%[a]), %%xmm0\n\t"
      "movhps 8(%[a]), %%xmm0\n\t"
      "movlps 0(%[b]), %%xmm1\n\t"
      "movhps 8(%[b]), %%xmm1\n\t"

      "subps %%xmm1, %%xmm0\n\t"
      "mulps %%xmm0, %%xmm0\n\t"
      "addps %%xmm0, %%xmm4\n\t"

      "add $16, %[a]\n\t"
      "add $16, %[b]\n\t"
      "sub $4, %%eax\n\t"
      ".L_12_%=:\n\t"
      // --- End ---

      // --- Finish up 2 ---
      "test $2, %%eax\n\t"
      "jz .L_13_%=\n\t"
      "movss 0(%[b]), %%xmm0\n\t"
      "movss 4(%[b]), %%xmm1\n\t"

      "subss 0(%[a]), %%xmm0\n\t"
      "subss 4(%[a]), %%xmm1\n\t"

      "mulss %%xmm0, %%xmm0\n\t"
      "mulss %%xmm1, %%xmm1\n\t"

      "addss %%xmm0, %%xmm4\n\t"
      "addss %%xmm1, %%xmm5\n\t"

      "add $8, %[a]\n\t"
      "add $8, %[b]\n\t"
      "sub $2, %%eax\n\t"
      ".L_13_%=:\n\t"
      // --- End ---

      // --- Finish up 1 ---
      "test $1, %%eax\n\t"
      "jz .L_14_%=\n\t"
      "movss 0(%[b]), %%xmm0\n\t"
      "subss 0(%[a]), %%xmm0\n\t"
      "mulss %%xmm0, %%xmm0\n\t"
      "addss %%xmm0, %%xmm4\n\t"
      ".L_14_%=:\n\t"
      // --- End ---

      // --- Horizontal add ---
      "addps %%xmm5, %%xmm4\n\t"

      "movhlps %%xmm4, %%xmm5\n\t"
      "addps %%xmm5, %%xmm4\n\t"

      "movaps %%xmm4, %%xmm5\n\t"
      "shufps $1, %%xmm4, %%xmm5\n\t"
      "addss %%xmm5, %%xmm4\n\t"
      // --- End ---

      "movss %%xmm4, %[dsq]\n\t"
     :
      [dsq] "=m" (dsq),
      [a] "=r" (a),
      [b] "=r" (b)
     :
      "1" (a),
      "2" (b),
      [D] "m" (D)
     :
      "%eax", "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5"
     );
    return dsq;
#endif
}
#endif

inline
float
jp_dist_l2(const float* a, const float* b, unsigned D)
{
#ifdef __SSE__
  return jp_dist_l2_ff_sse(a, b, D);
#else
  return jp_dist_l2_ff_slow(a, b, D);
#endif
}

// *** (unsigned char, unsigned char) ***
inline
uint32_t
jp_dist_l2_ucuc_slow(const unsigned char* a, const unsigned char* b, unsigned D)
{
  uint32_t ret = 0;
  for (unsigned d=0; d<D; ++d) {
    ret += (a[d] - b[d])*(a[d] - b[d]);
  }
  return ret;
}

#ifdef __SSE2__
inline
uint32_t
jp_dist_l2_ucuc_sse2(const unsigned char* a, const unsigned char* b, unsigned D)
{
  uint32_t sum_sqr __attribute__((aligned(16)));

  static const unsigned char mask[16] __attribute__((aligned(16))) = 
                                        {0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 
                                         0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00};
  unsigned d = 0;
  __asm__ (
    "movl %[D], %%eax\n\t"          // eax = 0
    "andl $-16, %%eax\n\t"

    "pxor %%xmm6, %%xmm6\n\t"       // Accumulator1
    "pxor %%xmm7, %%xmm7\n\t"       // Accumulator2
    
    "loop_begin_%=:\n\t"

    "movdqu 0(%[a]), %%xmm0\n\t"     // 0 = 2 = a[0:16]
    "movdqa %%xmm0, %%xmm2\n\t"      
    "movdqu 0(%[b]), %%xmm1\n\t"     // 1 = b[0:16]
    
    "movdqu 16(%[a]), %%xmm3\n\t"    // 3 = 5 = a[16:32]
    "movdqa %%xmm3, %%xmm5\n\t"     
    "movdqu 16(%[b]), %%xmm4\n\t"    // 4 = b[16:32]

    "psubusb %%xmm1, %%xmm0\n\t"    // 0 = max(a-b,[0])[0:16]
    "psubusb %%xmm2, %%xmm1\n\t"    // 1 = max(b-a,[0])[0:16]
    "paddusb %%xmm1, %%xmm0\n\t"    // 0 = |a-b|[0:16]

    "psubusb %%xmm4, %%xmm3\n\t"    // For 16:32
    "psubusb %%xmm5, %%xmm4\n\t"
    "paddusb %%xmm4, %%xmm3\n\t"

    "movdqa %%xmm0, %%xmm1\n\t"   // 0 = 1 = |a-b|[0:16]
    "movdqa %%xmm3, %%xmm4\n\t"   // 3 = 4 = |a-b|[16:32]
    
    "pand 0(%[mask]), %%xmm0\n\t" // 0 = lower nums
    "pmaddwd %%xmm0, %%xmm0\n\t"  // 0 = sum(|a-b|^2)
    "paddd %%xmm0, %%xmm6\n\t"

    "pand 0(%[mask]), %%xmm3\n\t"
    "pmaddwd %%xmm3, %%xmm3\n\t"
    "paddd %%xmm3, %%xmm7\n\t"

    "psrldq $1, %%xmm1\n\t"        
    "pand 0(%[mask]), %%xmm1\n\t" // 1 = higher nums
    "pmaddwd %%xmm1, %%xmm1\n\t"  // 1 = sum(|a-b|^2)
    "paddd %%xmm1, %%xmm6\n\t"

    "psrldq $1, %%xmm4\n\t"
    "pand 0(%[mask]), %%xmm4\n\t"
    "pmaddwd %%xmm4, %%xmm4\n\t"
    "paddd %%xmm4, %%xmm7\n\t"

    "add $32, %[a]\n\t"
    "add $32, %[b]\n\t"
    "add $32, %[d]\n\t"
    "subl $32, %%eax\n\t"
    "test %%eax, %%eax\n\t"
    "jnz loop_begin_%=\n\t"

    "paddd %%xmm6, %%xmm7\n\t"
    "pshufd $14, %%xmm7, %%xmm6\n\t"
    "paddd %%xmm6, %%xmm7\n\t"
    "pshufd $1, %%xmm7, %%xmm6\n\t"
    "paddd %%xmm6, %%xmm7\n\t"
    "movd %%xmm7, %[sum_sqr]\n\t"
   : // Outputs
    [sum_sqr] "=m" (sum_sqr),
    [a] "=r" (a),
    [b] "=r" (b),
    [d] "=r" (d)
   : // Inputs
    "1" (a),
    "2" (b),
    "3" (d),
    [mask] "r" (mask),
    [D] "m" (D)
   : // Clobbers
    "%eax", "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7"
  );
  for ( ; d < D; ++d) {
    sum_sqr += (a[d] - b[d])*(a[d] - b[d]);
  }
  return sum_sqr;
}
#endif

inline
uint32_t
jp_dist_l2(const unsigned char* a, const unsigned char* b, unsigned D)
{
#ifdef __SSE2__
  return jp_dist_l2_ucuc_sse2(a, b, D);
#else
  return jp_dist_l2_ucuc_slow(a, b, D);
#endif
}

// *** (float, unsigned char) ***
#if 0
inline
float
jp_dist_l2_ucf_sse2(const unsigned char* a, const float* b, unsigned D)
{
  float sum_sqr __attribute__ ((aligned(16)));
  static const unsigned char mask[16] __attribute__((aligned(16))) = 
                                        {0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
                                         0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00};
  unsigned d = 0;

//  __asm__ (
//    "movl %[D], %%eax\n\t"
//    "andl $-16, %%eax\n\t"
//
//    "pxor %%xmm7, %%xmm7\n\t"
//
//    "loop_begin_%=:\n\t"
//
//    "movdqu 0(%[a]), %%xmm0\n\t"
//
//
//    )
    
}
#endif

#endif
