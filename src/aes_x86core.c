#include <assert.h>
#include <stdlib.h>
#include <openssl/aes.h>
#ifndef LIBC_H
#include "./libc.h"
#endif
#define AES_COMPACT_IN_OUTER_ROUNDS
#ifdef  AES_COMPACT_IN_OUTER_ROUNDS
# undef  AES_COMPACT_IN_INNER_ROUNDS
#endif
#undef GETU32
#define GETU32(p) (*((u32*)(p)))
typedef unsigned long int u64;
#undef ROTATE
#   define ROTATE(a,n)  ({ register unsigned int ret;   \
                asm (           \
                "roll %1,%0"        \
                : "=r"(ret)     \
                : "I"(n), "0"(a)    \
                : "cc");        \
               ret;             \
            })
#define Te0 (u32)((u64*)((u8*)Te+0))
#define Te1 (u32)((u64*)((u8*)Te+3))
#define Te2 (u32)((u64*)((u8*)Te+2))
#define Te3 (u32)((u64*)((u8*)Te+1))

#define Td0 (u32)((u64*)((u8*)Td+0))
#define Td1 (u32)((u64*)((u8*)Td+3))
#define Td2 (u32)((u64*)((u8*)Td+2))
#define Td3 (u32)((u64*)((u8*)Td+1))

/**
 * Expand the cipher key into the encryption key schedule.
 */
int AES_set_encrypt_key(const unsigned char *userKey, const int bits,
                        AES_KEY *key)
{//设置轮密钥

    u32 *rk;
    int i = 0;
    u32 temp;

    if (!userKey || !key)
        return -1;
    if (bits != 128 && bits != 192 && bits != 256)
        return -2;//固定密钥位数

    rk = key->rd_key;

    if (bits==128)
        key->rounds = 10;
//行移位
    rk[0] = GETU32(userKey     );
    rk[1] = GETU32(userKey +  4);
    rk[2] = GETU32(userKey +  8);
    rk[3] = GETU32(userKey + 12);
        while (1) {
            temp  = rk[3];//s盒变换用Te4表进行表示
            rk[4] = rk[0] ^
                ((u32)Te4[(temp >>  8) & 0xff]      ) ^
                ((u32)Te4[(temp >> 16) & 0xff] <<  8) ^
                ((u32)Te4[(temp >> 24)       ] << 16) ^
                ((u32)Te4[(temp      ) & 0xff] << 24) ^
                rcon[i];
            rk[5] = rk[1] ^ rk[4];
            rk[6] = rk[2] ^ rk[5];
            rk[7] = rk[3] ^ rk[6];
            if (++i == 10) {
                return 0;
            }
            rk += 4;
        }
    rk[4] = GETU32(userKey + 16);
    rk[5] = GETU32(userKey + 20);
    rk[6] = GETU32(userKey + 24);
    rk[7] = GETU32(userKey + 28);
    return 0;
}

/**
 * Expand the cipher key into the decryption key schedule.
 */
int AES_set_decrypt_key(const unsigned char *userKey, const int bits,
                        AES_KEY *key)
{

    u32 *rk;
    int i, j, status;
    u32 temp;

    /* first, start with an encryption schedule */
    status = AES_set_encrypt_key(userKey, bits, key);
    if (status < 0)
        return status;

    rk = key->rd_key;

    /* invert the order of the round keys: */
    for (i = 0, j = 4*(key->rounds); i < j; i += 4, j -= 4) {
        temp = rk[i    ]; rk[i    ] = rk[j    ]; rk[j    ] = temp;
        temp = rk[i + 1]; rk[i + 1] = rk[j + 1]; rk[j + 1] = temp;
        temp = rk[i + 2]; rk[i + 2] = rk[j + 2]; rk[j + 2] = temp;
        temp = rk[i + 3]; rk[i + 3] = rk[j + 3]; rk[j + 3] = temp;
    }
    /* apply the inverse MixColumn transform to all round keys but the first and the last: */
    for (i = 1; i < (key->rounds); i++) {
        rk += 4;
        for (j = 0; j < 4; j++) {
            u32 tp1, tp2, tp4, tp8, tp9, tpb, tpd, tpe, m;

            tp1 = rk[j];
            m = tp1 & 0x80808080;
            tp2 = ((tp1 & 0x7f7f7f7f) << 1) ^
                ((m - (m >> 7)) & 0x1b1b1b1b);
            m = tp2 & 0x80808080;
            tp4 = ((tp2 & 0x7f7f7f7f) << 1) ^
                ((m - (m >> 7)) & 0x1b1b1b1b);
            m = tp4 & 0x80808080;
            tp8 = ((tp4 & 0x7f7f7f7f) << 1) ^
                ((m - (m >> 7)) & 0x1b1b1b1b);
            tp9 = tp8 ^ tp1;
            tpb = tp9 ^ tp2;
            tpd = tp9 ^ tp4;
            tpe = tp8 ^ tp4 ^ tp2;
            rk[j] = tpe ^ ROTATE(tpd,16) ^
                ROTATE(tp9,8) ^ ROTATE(tpb,24);
        }
    }
    return 0;
}

/*
 * Encrypt a single block
 * in and out can overlap
 */
void AES_encrypt(const unsigned char *in, unsigned char *out,
                 const AES_KEY *key)
{

    const u32 *rk;
    u32 s0, s1, s2, s3, t[4];
    int r;

    assert(in && out && key);
    rk = key->rd_key;

    /*
     * map byte array block to cipher state
     * and add initial round key:
     */
    //初始轮密钥加
    s0 = GETU32(in     ) ^ rk[0];//数据顺序为U32{in[3],in[2],in[1],in[0]}
    s1 = GETU32(in +  4) ^ rk[1];
    s2 = GETU32(in +  8) ^ rk[2];
    s3 = GETU32(in + 12) ^ rk[3];
    prefetch256(Te4);
//TE4为加密S盒，其中循环右移为选取位数，循环左移为控制为行位移，此处为第一轮加密
    t[0] = (u32)Te4[(s0      ) & 0xff]       ^
           (u32)Te4[(s1 >>  8) & 0xff] <<  8 ^
           (u32)Te4[(s2 >> 16) & 0xff] << 16 ^
           (u32)Te4[(s3 >> 24)       ] << 24;
    t[1] = (u32)Te4[(s1      ) & 0xff]       ^
           (u32)Te4[(s2 >>  8) & 0xff] <<  8 ^
           (u32)Te4[(s3 >> 16) & 0xff] << 16 ^
           (u32)Te4[(s0 >> 24)       ] << 24;
    t[2] = (u32)Te4[(s2      ) & 0xff]       ^
           (u32)Te4[(s3 >>  8) & 0xff] <<  8 ^
           (u32)Te4[(s0 >> 16) & 0xff] << 16 ^
           (u32)Te4[(s1 >> 24)       ] << 24;
    t[3] = (u32)Te4[(s3      ) & 0xff]       ^
           (u32)Te4[(s0 >>  8) & 0xff] <<  8 ^
           (u32)Te4[(s1 >> 16) & 0xff] << 16 ^
           (u32)Te4[(s2 >> 24)       ] << 24;

    /* now do the linear transform using words */
    //第一轮经过S盒变换与行位移后，进行列混合
    {   int i;
        u32 r0, r1, r2;

        for (i = 0; i < 4; i++) {
            r0 = t[i];
            r1 = r0 & 0x80808080;
            r2 = ((r0 & 0x7f7f7f7f) << 1) ^
                ((r1 - (r1 >> 7)) & 0x1b1b1b1b);
            t[i] = r2 ^ ROTATE(r2,24) ^ ROTATE(r0,24) ^
                ROTATE(r0,16) ^ ROTATE(r0,8);
            t[i] ^= rk[4+i];
        }
    }
    /*
     * Nr - 2 full rounds:
     */
    for (rk+=8,r=key->rounds-2; r>0; rk+=4,r--) {
//8轮轮加密，四次查表加四次异或
        t[0] =  Te0[(s0      ) & 0xff] ^
            Te1[(s1 >>  8) & 0xff] ^
            Te2[(s2 >> 16) & 0xff] ^
            Te3[(s3 >> 24)       ] ^
            rk[0];
        t[1] =  Te0[(s1      ) & 0xff] ^
            Te1[(s2 >>  8) & 0xff] ^
            Te2[(s3 >> 16) & 0xff] ^
            Te3[(s0 >> 24)       ] ^
            rk[1];
        t[2] =  Te0[(s2      ) & 0xff] ^
            Te1[(s3 >>  8) & 0xff] ^
            Te2[(s0 >> 16) & 0xff] ^
            Te3[(s1 >> 24)       ] ^
            rk[2];
        t[3] =  Te0[(s3      ) & 0xff] ^
            Te1[(s0 >>  8) & 0xff] ^
            Te2[(s1 >> 16) & 0xff] ^
            Te3[(s2 >> 24)       ] ^
            rk[3];
        s0 = t[0]; s1 = t[1]; s2 = t[2]; s3 = t[3];
    }
    /*
     * apply last round and
     * map cipher state to byte array block:
     */
 prefetch256(Te4);
//最后一次加密，无列混合
    *(u32*)(out+0) =
           (u32)Te4[(s0      ) & 0xff]       ^
           (u32)Te4[(s1 >>  8) & 0xff] <<  8 ^
           (u32)Te4[(s2 >> 16) & 0xff] << 16 ^
           (u32)Te4[(s3 >> 24)       ] << 24 ^
        rk[0];
    *(u32*)(out+4) =
           (u32)Te4[(s1      ) & 0xff]       ^
           (u32)Te4[(s2 >>  8) & 0xff] <<  8 ^
           (u32)Te4[(s3 >> 16) & 0xff] << 16 ^
           (u32)Te4[(s0 >> 24)       ] << 24 ^
        rk[1];
    *(u32*)(out+8) =
           (u32)Te4[(s2      ) & 0xff]       ^
           (u32)Te4[(s3 >>  8) & 0xff] <<  8 ^
           (u32)Te4[(s0 >> 16) & 0xff] << 16 ^
           (u32)Te4[(s1 >> 24)       ] << 24 ^
        rk[2];
    *(u32*)(out+12) =
           (u32)Te4[(s3      ) & 0xff]       ^
           (u32)Te4[(s0 >>  8) & 0xff] <<  8 ^
           (u32)Te4[(s1 >> 16) & 0xff] << 16 ^
           (u32)Te4[(s2 >> 24)       ] << 24 ^
        rk[3];
}

/*
 * Decrypt a single block
 * in and out can overlap
 */
void AES_decrypt(const unsigned char *in, unsigned char *out,
                 const AES_KEY *key)
{

    const u32 *rk;
    u32 s0, s1, s2, s3, t[4];
    int r;

    assert(in && out && key);
    rk = key->rd_key;

    /*
     * map byte array block to cipher state
     * and add initial round key:
     */
    s0 = GETU32(in     ) ^ rk[0];
    s1 = GETU32(in +  4) ^ rk[1];
    s2 = GETU32(in +  8) ^ rk[2];
    s3 = GETU32(in + 12) ^ rk[3];

prefetch256(Td4);

    t[0] = (u32)Td4[(s0      ) & 0xff]       ^
           (u32)Td4[(s3 >>  8) & 0xff] <<  8 ^
           (u32)Td4[(s2 >> 16) & 0xff] << 16 ^
           (u32)Td4[(s1 >> 24)       ] << 24;
    t[1] = (u32)Td4[(s1      ) & 0xff]       ^
           (u32)Td4[(s0 >>  8) & 0xff] <<  8 ^
           (u32)Td4[(s3 >> 16) & 0xff] << 16 ^
           (u32)Td4[(s2 >> 24)       ] << 24;
    t[2] = (u32)Td4[(s2      ) & 0xff]       ^
           (u32)Td4[(s1 >>  8) & 0xff] <<  8 ^
           (u32)Td4[(s0 >> 16) & 0xff] << 16 ^
           (u32)Td4[(s3 >> 24)       ] << 24;
    t[3] = (u32)Td4[(s3      ) & 0xff]       ^
           (u32)Td4[(s2 >>  8) & 0xff] <<  8 ^
           (u32)Td4[(s1 >> 16) & 0xff] << 16 ^
           (u32)Td4[(s0 >> 24)       ] << 24;

    /* now do the linear transform using words */
    {
        int i;
        u32 tp1, tp2, tp4, tp8, tp9, tpb, tpd, tpe, m;

        for (i = 0; i < 4; i++) {
            tp1 = t[i];
            m = tp1 & 0x80808080;
            tp2 = ((tp1 & 0x7f7f7f7f) << 1) ^
                ((m - (m >> 7)) & 0x1b1b1b1b);
            m = tp2 & 0x80808080;
            tp4 = ((tp2 & 0x7f7f7f7f) << 1) ^
                ((m - (m >> 7)) & 0x1b1b1b1b);
            m = tp4 & 0x80808080;
            tp8 = ((tp4 & 0x7f7f7f7f) << 1) ^
                ((m - (m >> 7)) & 0x1b1b1b1b);
            tp9 = tp8 ^ tp1;
            tpb = tp9 ^ tp2;
            tpd = tp9 ^ tp4;
            tpe = tp8 ^ tp4 ^ tp2;
            t[i] = tpe ^ ROTATE(tpd,16) ^
                ROTATE(tp9,8) ^ ROTATE(tpb,24);
            t[i] ^= rk[4+i];
        }
    }

    s0 = t[0]; s1 = t[1]; s2 = t[2]; s3 = t[3];

    /*
     * Nr - 2 full rounds:
     */
    for (rk+=8,r=key->rounds-2; r>0; rk+=4,r--) {
    t[0] =  Td0[(s0      ) & 0xff] ^
        Td1[(s3 >>  8) & 0xff] ^
        Td2[(s2 >> 16) & 0xff] ^
        Td3[(s1 >> 24)       ] ^
        rk[0];
    t[1] =  Td0[(s1      ) & 0xff] ^
        Td1[(s0 >>  8) & 0xff] ^
        Td2[(s3 >> 16) & 0xff] ^
        Td3[(s2 >> 24)       ] ^
        rk[1];
    t[2] =  Td0[(s2      ) & 0xff] ^
        Td1[(s1 >>  8) & 0xff] ^
        Td2[(s0 >> 16) & 0xff] ^
        Td3[(s3 >> 24)       ] ^
        rk[2];
    t[3] =  Td0[(s3      ) & 0xff] ^
        Td1[(s2 >>  8) & 0xff] ^
        Td2[(s1 >> 16) & 0xff] ^
        Td3[(s0 >> 24)       ] ^
        rk[3];
    s0 = t[0]; s1 = t[1]; s2 = t[2]; s3 = t[3];
    }
    /*
     * apply last round and
     * map cipher state to byte array block:
     */
    prefetch256(Td4);

    *(u32*)(out+0) =
        ((u32)Td4[(s0      ) & 0xff])    ^
        ((u32)Td4[(s3 >>  8) & 0xff] <<  8) ^
        ((u32)Td4[(s2 >> 16) & 0xff] << 16) ^
        ((u32)Td4[(s1 >> 24)       ] << 24) ^
        rk[0];
    *(u32*)(out+4) =
        ((u32)Td4[(s1      ) & 0xff])     ^
        ((u32)Td4[(s0 >>  8) & 0xff] <<  8) ^
        ((u32)Td4[(s3 >> 16) & 0xff] << 16) ^
        ((u32)Td4[(s2 >> 24)       ] << 24) ^
        rk[1];
    *(u32*)(out+8) =
        ((u32)Td4[(s2      ) & 0xff])     ^
        ((u32)Td4[(s1 >>  8) & 0xff] <<  8) ^
        ((u32)Td4[(s0 >> 16) & 0xff] << 16) ^
        ((u32)Td4[(s3 >> 24)       ] << 24) ^
        rk[2];
    *(u32*)(out+12) =
        ((u32)Td4[(s3      ) & 0xff])     ^
        ((u32)Td4[(s2 >>  8) & 0xff] <<  8) ^
        ((u32)Td4[(s1 >> 16) & 0xff] << 16) ^
        ((u32)Td4[(s0 >> 24)       ] << 24) ^
        rk[3];
}
