/*
 * GOST R 34.11-2012 core and API functions.
 *
 * $Id$
 */

#include <gost3411-2012-core.h>

/* 64-bit alignment required on 32-bit systems to produce optimized pxor
 * sequence in XLPS */
static unsigned long long __attribute__((aligned(8))) Ax[8][256];

typedef struct Ai_t
{
    uint8_t i[4];
} Ai_t;

static const Ai_t Ai[16] = {
    {{ 65, 65, 65, 65}},
    {{  3, 65, 65, 65}},
    {{  2, 65, 65, 65}},
    {{  3,  2, 65, 65}},
    {{  1, 65, 65, 65}},
    {{  3,  1, 65, 65}},
    {{  2,  1, 65, 65}},
    {{  3,  2,  1, 65}},
    {{  0, 65, 65, 65}},
    {{  3,  0, 65, 65}},
    {{  2,  0, 65, 65}},
    {{  3,  2,  0, 65}},
    {{  1,  0, 65, 65}},
    {{  3,  1,  0, 65}},
    {{  2,  1,  0, 65}},
    {{  3,  2,  1,  0}}
};

void *
memalloc(const size_t size)
{
    void *p;

    /* Ensure p is on a 64-bit boundary. */ 
    if (posix_memalign(&p, (size_t) 64, size))
        err(EX_OSERR, NULL);

    return p;
}

void
destroy(GOST3411Context *CTX)
{
    free(CTX->N);
    free(CTX->h);
    free(CTX->hash);
    free(CTX->Sigma);
    free(CTX->buffer);
    free(CTX->hexdigest);
    free(CTX);
}

GOST3411Context *
init(const uint32_t digest_size)
{
    unsigned int i, j, b;
    Ai_t idx1, idx2;
    GOST3411Context *CTX;

    CTX = memalloc(sizeof (GOST3411Context));

    CTX->N = memalloc(sizeof uint512_u);
    CTX->h = memalloc(sizeof uint512_u);
    CTX->hash = memalloc(sizeof uint512_u);
    CTX->Sigma = memalloc(sizeof uint512_u);
    CTX->buffer = memalloc(sizeof uint512_u);
    CTX->hexdigest = memalloc((size_t) 129);
    CTX->bufsize = 0;

    memcpy(CTX->N, &buffer0, sizeof buffer0);
    memcpy(CTX->h, &buffer0, sizeof buffer0);
    memcpy(CTX->hash, &buffer0, sizeof buffer0);
    memcpy(CTX->Sigma, &buffer0, sizeof buffer0);
    memcpy(CTX->buffer, &buffer0, sizeof buffer0);
    CTX->digest_size = digest_size;
    memset(CTX->hexdigest, 0, (size_t) 1);

    for (i = 0; i < 8; i++)
    {
        if (digest_size == 256)
            CTX->h->QWORD[i] = 0x0101010101010101ULL;
        else
            CTX->h->QWORD[i] = 0ULL;
    }

    for (i = 0; i < 8; i++)
    {
        for (b = 0; b < 256; b++)
        {
            j = 64 - (i << 3) - 8;
            idx1 = Ai[(b & 0x0F) >> 0];
            idx2 = Ai[(b & 0xF0) >> 4];
            Ax[i][b] = A[j + 4 + idx1.i[0]] ^ A[j + idx2.i[0]] ^
                       A[j + 4 + idx1.i[1]] ^ A[j + idx2.i[1]] ^
                       A[j + 4 + idx1.i[2]] ^ A[j + idx2.i[2]] ^
                       A[j + 4 + idx1.i[3]] ^ A[j + idx2.i[3]];
        }
    }

    return CTX;
}

static inline void
pad(GOST3411Context *CTX)
{
    unsigned char buf[64];

    if (CTX->bufsize > 63)
        return;

    memset(&buf, 0x00, sizeof buf);
    memcpy(&buf, CTX->buffer, CTX->bufsize);

    buf[CTX->bufsize] = 0x01;
    memcpy(CTX->buffer, &buf, sizeof buf);
}

static inline void
add512(const union uint512_u *x, const union uint512_u *y, union uint512_u *r)
{
    unsigned int CF;
    unsigned int i;

    CF = 0;
    for (i = 0; i < 8; i++)
    {
        r->QWORD[i] = x->QWORD[i] + y->QWORD[i] + CF;
        /* Actually it is sufficient to compare *r with ANY argument. However,
         * one argument of add512() call MAY point to *r at the same time.
         * Consider add512(x, y, x) or ad512(x, y, y). We avoid confusion by
         * comparing *r with both arguments. Instead of *y, assume that *x
         * probably points to *r, so *y goes first.
         */
        if ( (r->QWORD[i] < y->QWORD[i]) || 
             (r->QWORD[i] < x->QWORD[i]) )
            CF = 1;
        else
            CF = 0;
    }
}

static void
g(union uint512_u *h, const union uint512_u *N, const union uint512_u *m)
{
    __m128i xmm0, xmm2, xmm4, xmm6; /* XMMR0-quadruple */
    __m128i xmm1, xmm3, xmm5, xmm7; /* XMMR1-quadruple */
    unsigned int i;

    LOAD(N, xmm0, xmm2, xmm4, xmm6);
    XLPS128M(h, xmm0, xmm2, xmm4, xmm6);

    LOAD(m, xmm1, xmm3, xmm5, xmm7);
    XLPS128R(xmm0, xmm2, xmm4, xmm6, xmm1, xmm3, xmm5, xmm7);

    for (i = 0; i < 11; i++)
        ROUND128(i, xmm0, xmm2, xmm4, xmm6, xmm1, xmm3, xmm5, xmm7);

    XLPS128M((&C[11]), xmm0, xmm2, xmm4, xmm6);
    X128R(xmm0, xmm2, xmm4, xmm6, xmm1, xmm3, xmm5, xmm7);

    X128M(h, xmm0, xmm2, xmm4, xmm6);
    X128M(m, xmm0, xmm2, xmm4, xmm6);

    UNLOAD(h, xmm0, xmm2, xmm4, xmm6);

    /* Restore the Floating-point status on the CPU */
    _mm_empty();
}

static inline void
round2(GOST3411Context *CTX)
{
    g(CTX->h, CTX->N, CTX->buffer);

    add512(CTX->N, &buffer512, CTX->N);
    add512(CTX->Sigma, CTX->buffer, CTX->Sigma);
}

static inline void
round3(GOST3411Context *CTX)
{
    union uint512_u buf;

    memset(&buf, 0x00, sizeof buf);
    memcpy(&buf, CTX->buffer, CTX->bufsize);
    memcpy(CTX->buffer, &buf, sizeof uint512_u);

    memset(&buf, 0x00, sizeof buf);
    buf.QWORD[0] = CTX->bufsize << 3;

    pad(CTX);

    g(CTX->h, CTX->N, CTX->buffer);

    add512(CTX->N, &buf, CTX->N);
    add512(CTX->Sigma, CTX->buffer, CTX->Sigma);

    g(CTX->h, &buffer0, CTX->N);

    g(CTX->h, &buffer0, CTX->Sigma);
    memcpy(CTX->hash, CTX->h, sizeof uint512_u);
}

void
update(GOST3411Context *CTX, const char *data, size_t len)
{
    size_t chunksize;

    while (len)
    {
        chunksize = 64 - CTX->bufsize;
        if (chunksize > len)
            chunksize = len;

        memcpy(CTX->buffer, data, chunksize);

        CTX->bufsize += chunksize;
        data += chunksize;
        len -= chunksize;

        if (CTX->bufsize == 64)
        {
            round2(CTX);
            CTX->bufsize = 0;
        }
    }
}

void
final(GOST3411Context *CTX)
{
    int i, j;
    char *buf;

    round3(CTX);
    CTX->bufsize = 0;

    buf = memalloc((size_t) 17);

    if (CTX->digest_size == 256)
        j = 4;
    else
        j = 0;

    i = 7;
    while (i >= j)
    {
        snprintf(buf, (size_t) 17, "%.16llx", CTX->hash->QWORD[i]);
        strncat(CTX->hexdigest, buf, (size_t) 16);
        i--;
    }

    free(buf);
}
