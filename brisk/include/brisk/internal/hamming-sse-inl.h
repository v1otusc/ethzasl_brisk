/*
 Copyright (C) 2011 The Autonomous Systems Lab, ETH Zurich,
 Stefan Leutenegger, Simon Lynen and Margarita Chli.

 Copyright(C) 2013 The Autonomous Systems Lab, ETH Zurich,
 Stefan Leutenegger and Simon Lynen.

 BRISK - Binary Robust Invariant Scalable Keypoints
 Reference implementation of
 [1] Stefan Leutenegger,Margarita Chli and Roland Siegwart, BRISK:
 Binary Robust Invariant Scalable Keypoints, in Proceedings of
 the IEEE International Conference on Computer Vision(ICCV2011).

 This file is part of BRISK.

 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the <organization> nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef INTERNAL_HAMMING_SSE_INL_H_
#define INTERNAL_HAMMING_SSE_INL_H_

#include <emmintrin.h>
#include <tmmintrin.h>

namespace brisk {

#ifdef __GNUC__
static const char __attribute__((aligned(16))) MASK_4bit[16] = {0xf, 0xf, 0xf,
  0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf};
static const uint8_t __attribute__((aligned(16))) POPCOUNT_4bit[16] = {0, 1, 1,
  2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
static const __m128i shiftval = _mm_set_epi32(0, 0, 0, 4);
#endif
#ifdef _MSC_VER
__declspec(align(16)) static const char MASK_4bit[16] = {0xf, 0xf, 0xf, 0xf,
  0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf};
__declspec(align(16)) static const uint8_t POPCOUNT_4bit[16] = {0, 1, 1, 2,
  1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
static const __m128i shiftval = _mm_set_epi32(0, 0, 0, 4);
#endif
// - SSSE3 - better alorithm, minimized psadbw usage -
// adapted from http://wm.ite.pl/articles/sse-popcount.html
__inline__ uint32_t HammingSse::SSSE3PopcntofXORed(const __m128i* signature1,
const __m128i*signature2,
const int numberOf128BitWords) {
  uint32_t result = 0;

  register __m128i xmm0;
  register __m128i xmm1;
  register __m128i xmm2;
  register __m128i xmm3;
  register __m128i xmm4;
  register __m128i xmm5;
  register __m128i xmm6;
  register __m128i xmm7;

  // __asm__ volatile("movdqa(%0), %%xmm7" : : "a"(POPCOUNT_4bit) : "xmm7");
  xmm7 = _mm_load_si128(reinterpret_cast<const __m128i*>(POPCOUNT_4bit));
  // __asm__ volatile("movdqa(%0), %%xmm6" : : "a"(MASK_4bit) : "xmm6");
  xmm6 = _mm_load_si128(reinterpret_cast<const __m128i*>(MASK_4bit));
  // xmm5 -- global accumulator.
  // __asm__ volatile("pxor  %%xmm5, %%xmm5" : : : "xmm5");
  xmm5 = _mm_setzero_si128();

  const __m128i* end = signature1 + numberOf128BitWords;

  // __asm__ volatile("movdqa %xmm5, %xmm4"); // xmm4 -- local accumulator.
  xmm4 = xmm5;// _mm_load_si128(&xmm5);

  // for(n=0; n < numberOf128BitWords; n++) {
  do {
    // __asm__ volatile("movdqa(%0), %%xmm0" : : "a"(signature1++) : "xmm0");
    // __asm__ volatile(
    //  "movdqa (%0), %%xmm0 \n"
    // "pxor  (%0), %%xmm0  \n"
    xmm0 = _mm_xor_si128(*signature1++, *signature2++);
    //  "movdqu  %%xmm0, %%xmm1 \n"
    xmm1 = xmm0;// _mm_loadu_si128(&xmm0);
    //  "psrlw     $4, %%xmm1 \n"
    xmm1 = _mm_srl_epi16(xmm1, shiftval);
    //  "pand   %%xmm6, %%xmm0 \n" // xmm0 := lower nibbles.
    xmm0 = _mm_and_si128(xmm0, xmm6);
    //  "pand   %%xmm6, %%xmm1 \n" // xmm1 := higher nibbles.
    xmm1 = _mm_and_si128(xmm1, xmm6);
    //  "movdqu  %%xmm7, %%xmm2 \n"
    xmm2 = xmm7;// _mm_loadu_si128(&xmm7);
    //  "movdqu  %%xmm7, %%xmm3 \n" // Get popcount.
    xmm3 = xmm7;// _mm_loadu_si128(&xmm7);
    //  "pshufb  %%xmm0, %%xmm2 \n" // For all nibbles.
    xmm2 = _mm_shuffle_epi8(xmm2, xmm0);
    //  "pshufb  %%xmm1, %%xmm3 \n" // Using PSHUFB.
    xmm3 = _mm_shuffle_epi8(xmm3, xmm1);
    //  "paddb   %%xmm2, %%xmm4 \n" // Update local.
    xmm4 = _mm_add_epi8(xmm4, xmm2);
    //  "paddb   %%xmm3, %%xmm4 \n" // Accumulator.
    xmm4 = _mm_add_epi8(xmm4, xmm3);
    //  :
    //  : "a"(buffer++)
    //  : "xmm0","xmm1","xmm2","xmm3","xmm4"
    // );
  }while (signature1 < end);
  // Update global accumulator(two 32-bits counters).
  // __asm__ volatile(
  //  /*"pxor %xmm0, %xmm0 \n"*/
  //  "psadbw %%xmm5, %%xmm4 \n"
  xmm4 = _mm_sad_epu8(xmm4, xmm5);
  //  "paddd %%xmm4, %%xmm5 \n"
  xmm5 = _mm_add_epi32(xmm5, xmm4);
  //  :
  //  :
  //  : "xmm4","xmm5"
  // );
  // finally add together 32-bits counters stored in global accumulator.
  // __asm__ volatile(
  //  "movhlps  %%xmm5, %%xmm0 \n"
  // TODO(slynen): fix with appropriate intrinsic
  xmm0 = _mm_cvtps_epi32(_mm_movehl_ps(_mm_cvtepi32_ps(xmm0),
      _mm_cvtepi32_ps(xmm5)));
  //  "paddd   %%xmm5, %%xmm0 \n"
  xmm0 = _mm_add_epi32(xmm0, xmm5);
  //  "movd   %%xmm0, %%eax \n"
  result = _mm_cvtsi128_si32(xmm0);
  //  : "=a"(result) : : "xmm5","xmm0"
  // );
  return result;
}
}
// namespace brisk
#endif  // INTERNAL_HAMMING_SSE_INL_H_
