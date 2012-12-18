
/* This file was generated automatically by the Snowball to ANSI C compiler */

#include "../runtime/header.h"

extern int swedish_UTF_8_stem(struct SN_env * z);
static int r_other_suffix(struct SN_env * z);
static int r_consonant_pair(struct SN_env * z);
static int r_main_suffix(struct SN_env * z);
static int r_mark_regions(struct SN_env * z);

extern struct SN_env * swedish_UTF_8_create_env(void);
extern void swedish_UTF_8_close_env(struct SN_env * z);

static symbol s_0_0[1] = { 'a' };
static symbol s_0_1[4] = { 'a', 'r', 'n', 'a' };
static symbol s_0_2[4] = { 'e', 'r', 'n', 'a' };
static symbol s_0_3[7] = { 'h', 'e', 't', 'e', 'r', 'n', 'a' };
static symbol s_0_4[4] = { 'o', 'r', 'n', 'a' };
static symbol s_0_5[2] = { 'a', 'd' };
static symbol s_0_6[1] = { 'e' };
static symbol s_0_7[3] = { 'a', 'd', 'e' };
static symbol s_0_8[4] = { 'a', 'n', 'd', 'e' };
static symbol s_0_9[4] = { 'a', 'r', 'n', 'e' };
static symbol s_0_10[3] = { 'a', 'r', 'e' };
static symbol s_0_11[4] = { 'a', 's', 't', 'e' };
static symbol s_0_12[2] = { 'e', 'n' };
static symbol s_0_13[5] = { 'a', 'n', 'd', 'e', 'n' };
static symbol s_0_14[4] = { 'a', 'r', 'e', 'n' };
static symbol s_0_15[5] = { 'h', 'e', 't', 'e', 'n' };
static symbol s_0_16[3] = { 'e', 'r', 'n' };
static symbol s_0_17[2] = { 'a', 'r' };
static symbol s_0_18[2] = { 'e', 'r' };
static symbol s_0_19[5] = { 'h', 'e', 't', 'e', 'r' };
static symbol s_0_20[2] = { 'o', 'r' };
static symbol s_0_21[1] = { 's' };
static symbol s_0_22[2] = { 'a', 's' };
static symbol s_0_23[5] = { 'a', 'r', 'n', 'a', 's' };
static symbol s_0_24[5] = { 'e', 'r', 'n', 'a', 's' };
static symbol s_0_25[5] = { 'o', 'r', 'n', 'a', 's' };
static symbol s_0_26[2] = { 'e', 's' };
static symbol s_0_27[4] = { 'a', 'd', 'e', 's' };
static symbol s_0_28[5] = { 'a', 'n', 'd', 'e', 's' };
static symbol s_0_29[3] = { 'e', 'n', 's' };
static symbol s_0_30[5] = { 'a', 'r', 'e', 'n', 's' };
static symbol s_0_31[6] = { 'h', 'e', 't', 'e', 'n', 's' };
static symbol s_0_32[4] = { 'e', 'r', 'n', 's' };
static symbol s_0_33[2] = { 'a', 't' };
static symbol s_0_34[5] = { 'a', 'n', 'd', 'e', 't' };
static symbol s_0_35[3] = { 'h', 'e', 't' };
static symbol s_0_36[3] = { 'a', 's', 't' };

static struct among a_0[37] =
{
/*  0 */ { 1, s_0_0, -1, 1, 0},
/*  1 */ { 4, s_0_1, 0, 1, 0},
/*  2 */ { 4, s_0_2, 0, 1, 0},
/*  3 */ { 7, s_0_3, 2, 1, 0},
/*  4 */ { 4, s_0_4, 0, 1, 0},
/*  5 */ { 2, s_0_5, -1, 1, 0},
/*  6 */ { 1, s_0_6, -1, 1, 0},
/*  7 */ { 3, s_0_7, 6, 1, 0},
/*  8 */ { 4, s_0_8, 6, 1, 0},
/*  9 */ { 4, s_0_9, 6, 1, 0},
/* 10 */ { 3, s_0_10, 6, 1, 0},
/* 11 */ { 4, s_0_11, 6, 1, 0},
/* 12 */ { 2, s_0_12, -1, 1, 0},
/* 13 */ { 5, s_0_13, 12, 1, 0},
/* 14 */ { 4, s_0_14, 12, 1, 0},
/* 15 */ { 5, s_0_15, 12, 1, 0},
/* 16 */ { 3, s_0_16, -1, 1, 0},
/* 17 */ { 2, s_0_17, -1, 1, 0},
/* 18 */ { 2, s_0_18, -1, 1, 0},
/* 19 */ { 5, s_0_19, 18, 1, 0},
/* 20 */ { 2, s_0_20, -1, 1, 0},
/* 21 */ { 1, s_0_21, -1, 2, 0},
/* 22 */ { 2, s_0_22, 21, 1, 0},
/* 23 */ { 5, s_0_23, 22, 1, 0},
/* 24 */ { 5, s_0_24, 22, 1, 0},
/* 25 */ { 5, s_0_25, 22, 1, 0},
/* 26 */ { 2, s_0_26, 21, 1, 0},
/* 27 */ { 4, s_0_27, 26, 1, 0},
/* 28 */ { 5, s_0_28, 26, 1, 0},
/* 29 */ { 3, s_0_29, 21, 1, 0},
/* 30 */ { 5, s_0_30, 29, 1, 0},
/* 31 */ { 6, s_0_31, 29, 1, 0},
/* 32 */ { 4, s_0_32, 21, 1, 0},
/* 33 */ { 2, s_0_33, -1, 1, 0},
/* 34 */ { 5, s_0_34, -1, 1, 0},
/* 35 */ { 3, s_0_35, -1, 1, 0},
/* 36 */ { 3, s_0_36, -1, 1, 0}
};

static symbol s_1_0[2] = { 'd', 'd' };
static symbol s_1_1[2] = { 'g', 'd' };
static symbol s_1_2[2] = { 'n', 'n' };
static symbol s_1_3[2] = { 'd', 't' };
static symbol s_1_4[2] = { 'g', 't' };
static symbol s_1_5[2] = { 'k', 't' };
static symbol s_1_6[2] = { 't', 't' };

static struct among a_1[7] =
{
/*  0 */ { 2, s_1_0, -1, -1, 0},
/*  1 */ { 2, s_1_1, -1, -1, 0},
/*  2 */ { 2, s_1_2, -1, -1, 0},
/*  3 */ { 2, s_1_3, -1, -1, 0},
/*  4 */ { 2, s_1_4, -1, -1, 0},
/*  5 */ { 2, s_1_5, -1, -1, 0},
/*  6 */ { 2, s_1_6, -1, -1, 0}
};

static symbol s_2_0[2] = { 'i', 'g' };
static symbol s_2_1[3] = { 'l', 'i', 'g' };
static symbol s_2_2[3] = { 'e', 'l', 's' };
static symbol s_2_3[5] = { 'f', 'u', 'l', 'l', 't' };
static symbol s_2_4[5] = { 'l', 0xC3, 0xB6, 's', 't' };

static struct among a_2[5] =
{
/*  0 */ { 2, s_2_0, -1, 1, 0},
/*  1 */ { 3, s_2_1, 0, 1, 0},
/*  2 */ { 3, s_2_2, -1, 1, 0},
/*  3 */ { 5, s_2_3, -1, 3, 0},
/*  4 */ { 5, s_2_4, -1, 2, 0}
};

static unsigned char g_v[] = { 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 32 };

static unsigned char g_s_ending[] = { 119, 127, 149 };

static symbol s_0[] = { 'l', 0xC3, 0xB6, 's' };
static symbol s_1[] = { 'f', 'u', 'l', 'l' };

static int r_mark_regions(struct SN_env * z) {
    z->I[0] = z->l;
    {   int c_test = z->c; /* test, line 29 */
        {   int c = skip_utf8(z->p, z->c, 0, z->l, + 3);
            if (c < 0) return 0;
            z->c = c; /* hop, line 29 */
        }
        z->I[1] = z->c; /* setmark x, line 29 */
        z->c = c_test;
    }
    while(1) { /* goto, line 30 */
        int c = z->c;
        if (!(in_grouping_U(z, g_v, 97, 246))) goto lab0;
        z->c = c;
        break;
    lab0:
        z->c = c;
        {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
            if (c < 0) return 0;
            z->c = c; /* goto, line 30 */
        }
    }
    while(1) { /* gopast, line 30 */
        if (!(out_grouping_U(z, g_v, 97, 246))) goto lab1;
        break;
    lab1:
        {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
            if (c < 0) return 0;
            z->c = c; /* gopast, line 30 */
        }
    }
    z->I[0] = z->c; /* setmark p1, line 30 */
     /* try, line 31 */
    if (!(z->I[0] < z->I[1])) goto lab2;
    z->I[0] = z->I[1];
lab2:
    return 1;
}

static int r_main_suffix(struct SN_env * z) {
    int among_var;
    {   int m3; /* setlimit, line 37 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 37 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 37 */
        among_var = find_among_b(z, a_0, 37); /* substring, line 37 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 37 */
        z->lb = m3;
    }
    switch(among_var) {
        case 0: return 0;
        case 1:
            {   int ret;
                ret = slice_del(z); /* delete, line 44 */
                if (ret < 0) return ret;
            }
            break;
        case 2:
            if (!(in_grouping_b_U(z, g_s_ending, 98, 121))) return 0;
            {   int ret;
                ret = slice_del(z); /* delete, line 46 */
                if (ret < 0) return ret;
            }
            break;
    }
    return 1;
}

static int r_consonant_pair(struct SN_env * z) {
    {   int m3; /* setlimit, line 50 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 50 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        {   int m = z->l - z->c; (void) m; /* and, line 52 */
            if (!(find_among_b(z, a_1, 7))) { z->lb = m3; return 0; } /* among, line 51 */
            z->c = z->l - m;
            z->ket = z->c; /* [, line 52 */
            {   int c = skip_utf8(z->p, z->c, z->lb, 0, -1);
                if (c < 0) { z->lb = m3; return 0; }
                z->c = c; /* next, line 52 */
            }
            z->bra = z->c; /* ], line 52 */
            {   int ret;
                ret = slice_del(z); /* delete, line 52 */
                if (ret < 0) return ret;
            }
        }
        z->lb = m3;
    }
    return 1;
}

static int r_other_suffix(struct SN_env * z) {
    int among_var;
    {   int m3; /* setlimit, line 55 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 55 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 56 */
        among_var = find_among_b(z, a_2, 5); /* substring, line 56 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 56 */
        switch(among_var) {
            case 0: { z->lb = m3; return 0; }
            case 1:
                {   int ret;
                    ret = slice_del(z); /* delete, line 57 */
                    if (ret < 0) return ret;
                }
                break;
            case 2:
                {   int ret;
                    ret = slice_from_s(z, 4, s_0); /* <-, line 58 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                {   int ret;
                    ret = slice_from_s(z, 4, s_1); /* <-, line 59 */
                    if (ret < 0) return ret;
                }
                break;
        }
        z->lb = m3;
    }
    return 1;
}

extern int swedish_UTF_8_stem(struct SN_env * z) {
    {   int c = z->c; /* do, line 66 */
        {   int ret = r_mark_regions(z);
            if (ret == 0) goto lab0; /* call mark_regions, line 66 */
            if (ret < 0) return ret;
        }
    lab0:
        z->c = c;
    }
    z->lb = z->c; z->c = z->l; /* backwards, line 67 */

    {   int m = z->l - z->c; (void) m; /* do, line 68 */
        {   int ret = r_main_suffix(z);
            if (ret == 0) goto lab1; /* call main_suffix, line 68 */
            if (ret < 0) return ret;
        }
    lab1:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* do, line 69 */
        {   int ret = r_consonant_pair(z);
            if (ret == 0) goto lab2; /* call consonant_pair, line 69 */
            if (ret < 0) return ret;
        }
    lab2:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* do, line 70 */
        {   int ret = r_other_suffix(z);
            if (ret == 0) goto lab3; /* call other_suffix, line 70 */
            if (ret < 0) return ret;
        }
    lab3:
        z->c = z->l - m;
    }
    z->c = z->lb;
    return 1;
}

extern struct SN_env * swedish_UTF_8_create_env(void) { return SN_create_env(0, 2, 0); }

extern void swedish_UTF_8_close_env(struct SN_env * z) { SN_close_env(z); }

