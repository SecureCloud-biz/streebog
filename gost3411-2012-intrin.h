/*
 * $Id$
 */

#include <emmintrin.h>
#include <mmintrin.h>

#define LO(v) ((unsigned char) (v))
#define HI(v) ((unsigned char) (((unsigned int) (v)) >> 8))

#ifdef __i386__
#define EXTRACT EXTRACT32
#else
#define EXTRACT EXTRACT64
#endif

#define LOAD(P, xmm0, xmm1, xmm2, xmm3) { \
    const __m128i *__m128p = (const __m128i *) &P[0]; \
    xmm0 = _mm_load_si128(&__m128p[0]); \
    xmm1 = _mm_load_si128(&__m128p[1]); \
    xmm2 = _mm_load_si128(&__m128p[2]); \
    xmm3 = _mm_load_si128(&__m128p[3]); \
}

#define XLOAD(x, y, xmm0, xmm1, xmm2, xmm3) { \
    const __m128i *px = (const __m128i *) &x[0]; \
    const __m128i *py = (const __m128i *) &y[0]; \
    xmm0 = _mm_xor_si128(px[0], py[0]); \
    xmm1 = _mm_xor_si128(px[1], py[1]); \
    xmm2 = _mm_xor_si128(px[2], py[2]); \
    xmm3 = _mm_xor_si128(px[3], py[3]); \
}

#define UNLOAD(P, xmm0, xmm1, xmm2, xmm3) { \
    __m128i *__m128p = (__m128i *) &P[0]; \
    _mm_store_si128(&__m128p[0], xmm0); \
    _mm_store_si128(&__m128p[1], xmm1); \
    _mm_store_si128(&__m128p[2], xmm2); \
    _mm_store_si128(&__m128p[3], xmm3); \
}

#define X128(x, y, z) { \
    __m128i xmm0, xmm1, xmm2, xmm3; \
    XLOAD(x, y, xmm0, xmm1, xmm2, xmm3); \
    UNLOAD(  z, xmm0, xmm1, xmm2, xmm3); \
}

#define X128R(xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7) { \
    xmm0 = _mm_xor_si128(xmm0, xmm4); \
    xmm1 = _mm_xor_si128(xmm1, xmm5); \
    xmm2 = _mm_xor_si128(xmm2, xmm6); \
    xmm3 = _mm_xor_si128(xmm3, xmm7); \
}

#define X128M(P, xmm0, xmm1, xmm2, xmm3) { \
    const __m128i *__m128p = (const __m128i *) &P[0]; \
    xmm0 = _mm_xor_si128(xmm0, _mm_load_si128(&__m128p[0])); \
    xmm1 = _mm_xor_si128(xmm1, _mm_load_si128(&__m128p[1])); \
    xmm2 = _mm_xor_si128(xmm2, _mm_load_si128(&__m128p[2])); \
    xmm3 = _mm_xor_si128(xmm3, _mm_load_si128(&__m128p[3])); \
}

#ifndef __ICC
#define _mm_cvtsi64_m64(v) (__m64) v
#define _mm_cvtm64_si64(v) (long long) v
#endif

#define _mm_xor_64(mm0, mm1) _mm_xor_si64(mm0, _mm_cvtsi64_m64(mm1))

#define EXTRACT32(row, xmm0, xmm1, xmm2, xmm3, xmm4) {\
    register unsigned short ax; \
    __m64 mm0, mm1; \
     \
    ax = (unsigned short) _mm_extract_epi16(xmm0, row + 0); \
    mm0  = _mm_cvtsi64_m64(Ax[0][Pi[LO(ax)]]); \
    mm1  = _mm_cvtsi64_m64(Ax[0][Pi[HI(ax)]]); \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm0, row + 4); \
    mm0 = _mm_xor_64(mm0, Ax[1][Pi[LO(ax)]]); \
    mm1 = _mm_xor_64(mm1, Ax[1][Pi[HI(ax)]]); \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm1, row + 0); \
    mm0 = _mm_xor_64(mm0, Ax[2][Pi[LO(ax)]]); \
    mm1 = _mm_xor_64(mm1, Ax[2][Pi[HI(ax)]]); \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm1, row + 4); \
    mm0 = _mm_xor_64(mm0, Ax[3][Pi[LO(ax)]]); \
    mm1 = _mm_xor_64(mm1, Ax[3][Pi[HI(ax)]]); \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm2, row + 0); \
    mm0 = _mm_xor_64(mm0, Ax[4][Pi[LO(ax)]]); \
    mm1 = _mm_xor_64(mm1, Ax[4][Pi[HI(ax)]]); \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm2, row + 4); \
    mm0 = _mm_xor_64(mm0, Ax[5][Pi[LO(ax)]]); \
    mm1 = _mm_xor_64(mm1, Ax[5][Pi[HI(ax)]]); \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm3, row + 0); \
    mm0 = _mm_xor_64(mm0, Ax[6][Pi[LO(ax)]]); \
    mm1 = _mm_xor_64(mm1, Ax[6][Pi[HI(ax)]]); \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm3, row + 4); \
    mm0 = _mm_xor_64(mm0, Ax[7][Pi[LO(ax)]]); \
    mm1 = _mm_xor_64(mm1, Ax[7][Pi[HI(ax)]]); \
    \
    xmm4 = _mm_set_epi64(mm1, mm0); \
}

#define EXTRACT64(row, xmm0, xmm1, xmm2, xmm3, xmm4) {\
    __m128i tmm4; \
    register unsigned short ax; \
    register unsigned long long r0, r1; \
     \
    ax = (unsigned short) _mm_extract_epi16(xmm0, row + 0); \
    r0  = Ax[0][Pi[LO(ax)]]; \
    r1  = Ax[0][Pi[HI(ax)]]; \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm0, row + 4); \
    r0 ^= Ax[1][Pi[LO(ax)]]; \
    r1 ^= Ax[1][Pi[HI(ax)]]; \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm1, row + 0); \
    r0 ^= Ax[2][Pi[LO(ax)]]; \
    r1 ^= Ax[2][Pi[HI(ax)]]; \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm1, row + 4); \
    r0 ^= Ax[3][Pi[LO(ax)]]; \
    r1 ^= Ax[3][Pi[HI(ax)]]; \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm2, row + 0); \
    r0 ^= Ax[4][Pi[LO(ax)]]; \
    r1 ^= Ax[4][Pi[HI(ax)]]; \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm2, row + 4); \
    r0 ^= Ax[5][Pi[LO(ax)]]; \
    r1 ^= Ax[5][Pi[HI(ax)]]; \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm3, row + 0); \
    r0 ^= Ax[6][Pi[LO(ax)]]; \
    r1 ^= Ax[6][Pi[HI(ax)]]; \
    \
    ax = (unsigned short) _mm_extract_epi16(xmm3, row + 4); \
    r0 ^= Ax[7][Pi[LO(ax)]]; \
    r1 ^= Ax[7][Pi[HI(ax)]]; \
    \
    xmm4 = _mm_cvtsi64_si128((long long) r0); \
    tmm4 = _mm_cvtsi64_si128((long long) r1); \
    xmm4 = _mm_unpacklo_epi64(xmm4, tmm4); \
}

#define XLPS128M(P, xmm0, xmm1, xmm2, xmm3) { \
    __m128i tmm0, tmm1, tmm2, tmm3; \
    X128M(P, xmm0, xmm1, xmm2, xmm3); \
    \
    EXTRACT(0, xmm0, xmm1, xmm2, xmm3, tmm0); \
    EXTRACT(1, xmm0, xmm1, xmm2, xmm3, tmm1); \
    EXTRACT(2, xmm0, xmm1, xmm2, xmm3, tmm2); \
    EXTRACT(3, xmm0, xmm1, xmm2, xmm3, tmm3); \
    \
    xmm0 = tmm0; \
    xmm1 = tmm1; \
    xmm2 = tmm2; \
    xmm3 = tmm3; \
}

#define XLPS128R(xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7) { \
    __m128i tmm0, tmm1, tmm2, tmm3; \
    X128R(xmm4, xmm5, xmm6, xmm7, xmm0, xmm1, xmm2, xmm3); \
    \
    EXTRACT(0, xmm4, xmm5, xmm6, xmm7, tmm0); \
    EXTRACT(1, xmm4, xmm5, xmm6, xmm7, tmm1); \
    EXTRACT(2, xmm4, xmm5, xmm6, xmm7, tmm2); \
    EXTRACT(3, xmm4, xmm5, xmm6, xmm7, tmm3); \
    \
    xmm4 = tmm0; \
    xmm5 = tmm1; \
    xmm6 = tmm2; \
    xmm7 = tmm3; \
}

#define ROUND128(i, xmm0, xmm2, xmm4, xmm6, xmm1, xmm3, xmm5, xmm7) {\
    XLPS128M((&C[i]), xmm0, xmm2, xmm4, xmm6); \
    XLPS128R(xmm0, xmm2, xmm4, xmm6, xmm1, xmm3, xmm5, xmm7); \
}
