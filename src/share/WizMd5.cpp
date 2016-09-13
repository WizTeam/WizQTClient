#include "WizMd5.h"

#include <stdexcept>
#include <QFile>

#include "WizMisc.h"
#include "../utils/WizLogger.h"
#include "utils/WizMisc.h"


namespace wizmd5
{
    typedef unsigned char md5byte;
    typedef unsigned int UWORD32;

    struct MD5Context {
        UWORD32 buf[4];
        UWORD32 bytes[2];
        UWORD32 in[16];
    };

    static void MD5Init(struct MD5Context *context);
    static void MD5Update(struct MD5Context *context, md5byte const *buf, unsigned len);
    static void MD5Final(struct MD5Context *context, unsigned char digest[16]);
    static void MD5Transform(UWORD32 buf[4], UWORD32 const in[16]);

    static void byteSwap(UWORD32 *buf, unsigned words)
    {
        const quint32 byteOrderTest = 0x1;
        if (((char *)&byteOrderTest)[0] == 0) {
            md5byte *p = (md5byte *)buf;

            do {
                *buf++ = (UWORD32)((unsigned)p[3] << 8 | p[2]) << 16 |
                         ((unsigned)p[1] << 8 | p[0]);
                p += 4;
            } while (--words);
        }
    }

    /*
     * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
     * initialization constants.
     */
    static void
            MD5Init(struct MD5Context *ctx)
    {
        ctx->buf[0] = 0x67452301;
        ctx->buf[1] = 0xefcdab89;
        ctx->buf[2] = 0x98badcfe;
        ctx->buf[3] = 0x10325476;

        ctx->bytes[0] = 0;
        ctx->bytes[1] = 0;
    }

    /*
     * Update context to reflect the concatenation of another buffer full
     * of bytes.
     */
    static void
            MD5Update(struct MD5Context *ctx, md5byte const *buf, unsigned len)
    {
        UWORD32 t;

        /* Update byte count */

        t = ctx->bytes[0];
        if ((ctx->bytes[0] = t + len) < t)
            ctx->bytes[1]++;	/* Carry from low to high */

        t = 64 - (t & 0x3f);	/* Space available in ctx->in (at least 1) */
        if (t > len) {
            memcpy((md5byte *)ctx->in + 64 - t, buf, len);
            return;
        }
        /* First chunk is an odd size */
        memcpy((md5byte *)ctx->in + 64 - t, buf, t);
        byteSwap(ctx->in, 16);
        MD5Transform(ctx->buf, ctx->in);
        buf += t;
        len -= t;

        /* Process data in 64-byte chunks */
        while (len >= 64) {
            memcpy(ctx->in, buf, 64);
            byteSwap(ctx->in, 16);
            MD5Transform(ctx->buf, ctx->in);
            buf += 64;
            len -= 64;
        }

        /* Handle any remaining bytes of data. */
        memcpy(ctx->in, buf, len);
    }

    /*
     * Final wrapup - pad to 64-byte boundary with the bit pattern
     * 1 0* (64-bit count of bits processed, MSB-first)
     */
    static void
            MD5Final(struct MD5Context *ctx, md5byte digest[16])
    {
        int count = ctx->bytes[0] & 0x3f;	/* Number of bytes in ctx->in */
        md5byte *p = (md5byte *)ctx->in + count;

        /* Set the first char of padding to 0x80.  There is always room. */
        *p++ = 0x80;

        /* Bytes of padding needed to make 56 bytes (-8..55) */
        count = 56 - 1 - count;

        if (count < 0) {	/* Padding forces an extra block */
            memset(p, 0, count + 8);
            byteSwap(ctx->in, 16);
            MD5Transform(ctx->buf, ctx->in);
            p = (md5byte *)ctx->in;
            count = 56;
        }
        memset(p, 0, count);
        byteSwap(ctx->in, 14);

        /* Append length in bits and transform */
        ctx->in[14] = ctx->bytes[0] << 3;
        ctx->in[15] = ctx->bytes[1] << 3 | ctx->bytes[0] >> 29;
        MD5Transform(ctx->buf, ctx->in);

        byteSwap(ctx->buf, 4);
        memcpy(digest, ctx->buf, 16);
        memset(ctx, 0, sizeof(MD5Context));	/* In case it's sensitive */
    }

#ifndef ASM_MD5

    /* The four core functions - F1 is optimized somewhat */

    /* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

    /* This is the central step in the MD5 algorithm. */
#define MD5STEP(f,w,x,y,z,in,s) \
    (w += f(x,y,z) + in, w = (w<<s | w>>(32-s)) + x)

            /*
     * The core of the MD5 algorithm, this alters an existing MD5 hash to
     * reflect the addition of 16 longwords of new data.  MD5Update blocks
     * the data and converts bytes into longwords for this routine.
     */
            static void
            MD5Transform(UWORD32 buf[4], UWORD32 const in[16])
    {
        UWORD32 a, b, c, d;

        a = buf[0];
        b = buf[1];
        c = buf[2];
        d = buf[3];

        MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
        MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
        MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
        MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
        MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
        MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
        MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
        MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
        MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
        MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
        MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
        MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
        MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
        MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
        MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
        MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

        MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
        MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
        MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
        MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
        MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
        MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
        MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
        MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
        MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
        MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
        MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
        MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
        MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
        MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
        MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
        MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

        MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
        MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
        MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
        MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
        MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
        MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
        MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
        MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
        MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
        MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
        MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
        MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
        MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
        MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
        MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
        MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

        MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
        MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
        MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
        MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
        MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
        MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
        MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
        MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
        MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
        MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
        MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
        MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
        MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
        MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
        MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
        MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

        buf[0] += a;
        buf[1] += b;
        buf[2] += c;
        buf[3] += d;
    }

#endif

#if 0   //can not run under macosx
    typedef unsigned char *POINTER;
    typedef unsigned short int UINT2;
    typedef unsigned long int UINT4;

    /* MD5 context. */
    typedef struct {
        UINT4 state[4];                                   /* state (ABCD) */
        UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
        unsigned char buffer[64];                         /* input buffer */
    } MD5_CTX;


#define MD_CTX MD5_CTX
#define MDInit MD5Init
#define MDUpdate MD5Update
#define MDFinal MD5Final

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

    const unsigned char PADDING[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    /* F, G, H and I are basic MD5 functions.
*/
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

    /* ROTATE_LEFT rotates x left n bits.
*/
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

    /* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
*/
#define FF(a, b, c, d, x, s, ac) { \
    (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
           (a) = ROTATE_LEFT ((a), (s)); \
                 (a) += (b); \
                    }
#define GG(a, b, c, d, x, s, ac) { \
(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
       (a) = ROTATE_LEFT ((a), (s)); \
             (a) += (b); \
                }
#define HH(a, b, c, d, x, s, ac) { \
(a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
       (a) = ROTATE_LEFT ((a), (s)); \
             (a) += (b); \
                }
#define II(a, b, c, d, x, s, ac) { \
(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
       (a) = ROTATE_LEFT ((a), (s)); \
             (a) += (b); \
                }

/* Note: Replace "for loop" with standard memset if possible.
*/
void MD5_memset (POINTER output, int value, unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++)
        ((char *)output)[i] = (char)value;
}


/* Decodes input (unsigned char) into output (UINT4). Assumes len is
a multiple of 4.
*/
void Decode (UINT4 *output, const unsigned char *input, unsigned int len)
{
    unsigned int i, j;

    for (i = 0, j = 0; j < len; i++, j += 4)
        output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
                    (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

/* MD5 basic transformation. Transforms state based on block.
*/
void MD5Transform (UINT4 state[4], const unsigned char block[64])
{
    UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    Decode (x, block, 64);

    /* Round 1 */
    FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
    FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
    FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
    FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
    FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
    FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
    FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
    FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
    FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
    FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
    FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    /* Round 2 */
    GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
    GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
    GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
    GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
    GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
    GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
    GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
    GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
    GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
    GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
    GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
    GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
    HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
    HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
    HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
    HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
    HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
    HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
    HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
    HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
    HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
    HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

    /* Round 4 */
    II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
    II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
    II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
    II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
    II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
    II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
    II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
    II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
    II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
    II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    /* Zeroize sensitive information.
    */
    MD5_memset ((POINTER)x, 0, sizeof (x));
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
a multiple of 4.
*/
void Encode (unsigned char *output, UINT4 *input, unsigned int len)
{
    unsigned int i, j;

    for (i = 0, j = 0; j < len; i++, j += 4) {
        output[j] = (unsigned char)(input[i] & 0xff);
        output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
        output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
        output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
    }
}


/* Note: Replace "for loop" with standard memcpy if possible.
*/

void MD5_memcpy (POINTER output, POINTER input, unsigned int len)
{
    unsigned int i;

    for (i = 0; i < len; i++)
        output[i] = input[i];
}


/* MD5 initialization. Begins an MD5 operation, writing a new context.
*/
void MD5Init (MD5_CTX *context)
{
    context->count[0] = context->count[1] = 0;
    /* Load magic initialization constants.
    */
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
operation, processing another message block, and updating the
context.
*/
void MD5Update (MD5_CTX *context, const unsigned char *input, unsigned int inputLen)
{
    unsigned int i, index, partLen;

    /* Compute number of bytes mod 64 */
    index = (unsigned int)((context->count[0] >> 3) & 0x3F);

    /* Update number of bits */
    if ((context->count[0] += ((UINT4)inputLen << 3))
        < ((UINT4)inputLen << 3))
        context->count[1]++;
    context->count[1] += ((UINT4)inputLen >> 29);

    partLen = 64 - index;

    /* Transform as many times as possible.
    */
    if (inputLen >= partLen) {
        MD5_memcpy((POINTER)&context->buffer[index], (POINTER)input, partLen);
        MD5Transform (context->state, context->buffer);

        for (i = partLen; i + 63 < inputLen; i += 64)
            MD5Transform (context->state, &input[i]);

        index = 0;
    }
    else
        i = 0;

    /* Buffer remaining input */
    MD5_memcpy
            ((POINTER)&context->buffer[index], (POINTER)&input[i],
             inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
the message digest and zeroizing the context.
*/
void MD5Final (unsigned char digest[16], MD5_CTX *context)
{
    unsigned char bits[8];
    unsigned int index, padLen;

    /* Save number of bits */
    Encode (bits, context->count, 8);

    /* Pad out to 56 mod 64.
    */
    index = (unsigned int)((context->count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    MD5Update (context, PADDING, padLen);

    /* Append length (before padding) */
    MD5Update (context, bits, 8);

    /* Store state in digest */
    Encode (digest, context->state, 16);

    /* Zeroize sensitive information.
    */
    MD5_memset ((POINTER)context, 0, sizeof (*context));
}

#endif

}


void WizMd5(const unsigned char* pBuffer, DWORD dwLen, BYTE* pResult)
{
    wizmd5::MD5Context context;
    wizmd5::MD5Init (&context);
    wizmd5::MD5Update (&context, pBuffer, dwLen);
    wizmd5::MD5Final (&context, pResult);
}

CString WizMd5String(const unsigned char* pBuffer, DWORD dwLen)
{
    if (0 == dwLen)
        return CString();
    //
    DWORD dwBuffer[4] = {0, 0, 0, 0};
    //
    WizMd5(pBuffer, dwLen, (BYTE *)&dwBuffer);
    //
    CString str;
    str.format("%8x%8x%8x%8x", dwBuffer[0], dwBuffer[1], dwBuffer[2], dwBuffer[3]);
    //
    return str;
}

CString WizMd5StringNoSpace(const unsigned char* pBuffer, DWORD dwLen)
{
    if (0 == dwLen)
        return CString();
    //
    DWORD dwBuffer[4] = {0, 0, 0, 0};
    //
    WizMd5(pBuffer, dwLen, (BYTE *)&dwBuffer);
    //
    CString str;
    str.format("%08x%08x%08x%08x", dwBuffer[0], dwBuffer[1], dwBuffer[2], dwBuffer[3]);
    //
    return str;
}

CString WizMd5StringNoSpaceJava(const unsigned char* pBuffer, DWORD dwLen)
{
    if (0 == dwLen)
        return CString();
    //
    BYTE szBuffer[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    //
    WizMd5(pBuffer, dwLen, szBuffer);
    //
    CString str;
    str.format("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
               szBuffer[0], szBuffer[1], szBuffer[2], szBuffer[3],
               szBuffer[4], szBuffer[5], szBuffer[6], szBuffer[7],
               szBuffer[8], szBuffer[9], szBuffer[10], szBuffer[11],
               szBuffer[12], szBuffer[13], szBuffer[14], szBuffer[15]);
    //
    return str;
}

BOOL WizMd5(const QByteArray& arr, BYTE* pResult)
{
    ::WizMd5((const unsigned char*)(arr.constData()), arr.length(), pResult);
    return TRUE;
}

CString WizMd5StringNoSpaceJava(const QByteArray& arr)
{
    if (arr.isEmpty())
        return CString();
    //
    BYTE szBuffer[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    //
    WizMd5(arr, szBuffer);
    //
    CString str;
    str.format("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
               szBuffer[0], szBuffer[1], szBuffer[2], szBuffer[3],
               szBuffer[4], szBuffer[5], szBuffer[6], szBuffer[7],
               szBuffer[8], szBuffer[9], szBuffer[10], szBuffer[11],
               szBuffer[12], szBuffer[13], szBuffer[14], szBuffer[15]);
    //
    return str;
}

CString WizMd5FileStringNoSpaceJava(const CString& strFileName)
{
    QFile file(strFileName);

    if (!file.open(QIODevice::ReadOnly)) {
        return CString();
    }

    CString ret = WizMd5StringNoSpaceJava(file.readAll());

    file.close();

    return ret;
}

BOOL WizMd5File(const CString& strFileName, BYTE* pResult)
{
    qint64 len = Utils::WizMisc::getFileSize(strFileName);
    if (len <= 0)
        return FALSE;
    //
    QFile file(strFileName);
    //
    if (!file.open(QIODevice::ReadOnly))
        return false;
    //
    wizmd5::MD5Context context;
    //
    wizmd5::MD5Init (&context);
    //
    const UINT BUFFER_SIZE = 1024;
    //
    BOOL bRet = FALSE;
    //
    BYTE* pBuffer = NULL;
    try
    {
        pBuffer = new BYTE[BUFFER_SIZE];
        //
        int nCount = len / BUFFER_SIZE;
        //
        for (int i = 0; i < nCount; i++)
        {
            if (BUFFER_SIZE != file.read((char*)pBuffer, BUFFER_SIZE))
                throw std::runtime_error("Failed to read data from stream!");
            wizmd5::MD5Update (&context, pBuffer, BUFFER_SIZE);
        }
        //
        int nLast = len % BUFFER_SIZE;
        if (nLast)
        {
            int read = file.read((char*)pBuffer, nLast);
            if (nLast != read)
                throw std::runtime_error("Failed to read data from stream!");
            wizmd5::MD5Update (&context, pBuffer, nLast);
        }
        //
        bRet = TRUE;
    }
    catch (const std::exception& err)
    {
        TOLOG(CString(err.what()));
    }
    //
    delete [] pBuffer;
    //
    wizmd5::MD5Final (&context, pResult);
    //
    file.close();;
    //
    return bRet;
}

CString WizMd5FileString(const CString& strFileName)
{
    DWORD arrayMd5[4] = {0, 0, 0, 0};
    if (!WizMd5File(strFileName, (unsigned char *)arrayMd5))
        return CString();
    //
    CString str;
    str.format("%08x%08x%08x%08x", arrayMd5[0], arrayMd5[1], arrayMd5[2], arrayMd5[3]);
    return str;
}


CString WizPasswordToMd5StringNoSpace(const CString& strPassword)
{
    if (strPassword.isEmpty())
        return CString();
    //
    return WizMd5StringNoSpace((const unsigned char*)strPassword.utf16(), sizeof(unsigned short) * strPassword.getLength());
}

CString WizMd5StringNoSpace(const CString& str)
{
    return WizPasswordToMd5StringNoSpace(str);
}
