
/* This file was generated automatically by the Snowball to ANSI C compiler */

#include "../runtime/header.h"

extern int finnish_ISO_8859_1_stem(struct SN_env * z);
static int r_tidy(struct SN_env * z);
static int r_other_endings(struct SN_env * z);
static int r_t_plural(struct SN_env * z);
static int r_i_plural(struct SN_env * z);
static int r_case_ending(struct SN_env * z);
static int r_VI(struct SN_env * z);
static int r_LONG(struct SN_env * z);
static int r_possessive(struct SN_env * z);
static int r_particle_etc(struct SN_env * z);
static int r_R2(struct SN_env * z);
static int r_mark_regions(struct SN_env * z);

extern struct SN_env * finnish_ISO_8859_1_create_env(void);
extern void finnish_ISO_8859_1_close_env(struct SN_env * z);

static symbol s_0_0[2] = { 'p', 'a' };
static symbol s_0_1[3] = { 's', 't', 'i' };
static symbol s_0_2[4] = { 'k', 'a', 'a', 'n' };
static symbol s_0_3[3] = { 'h', 'a', 'n' };
static symbol s_0_4[3] = { 'k', 'i', 'n' };
static symbol s_0_5[3] = { 'h', 0xE4, 'n' };
static symbol s_0_6[4] = { 'k', 0xE4, 0xE4, 'n' };
static symbol s_0_7[2] = { 'k', 'o' };
static symbol s_0_8[2] = { 'p', 0xE4 };
static symbol s_0_9[2] = { 'k', 0xF6 };

static struct among a_0[10] =
{
/*  0 */ { 2, s_0_0, -1, 1, 0},
/*  1 */ { 3, s_0_1, -1, 2, 0},
/*  2 */ { 4, s_0_2, -1, 1, 0},
/*  3 */ { 3, s_0_3, -1, 1, 0},
/*  4 */ { 3, s_0_4, -1, 1, 0},
/*  5 */ { 3, s_0_5, -1, 1, 0},
/*  6 */ { 4, s_0_6, -1, 1, 0},
/*  7 */ { 2, s_0_7, -1, 1, 0},
/*  8 */ { 2, s_0_8, -1, 1, 0},
/*  9 */ { 2, s_0_9, -1, 1, 0}
};

static symbol s_1_0[3] = { 'l', 'l', 'a' };
static symbol s_1_1[2] = { 'n', 'a' };
static symbol s_1_2[3] = { 's', 's', 'a' };
static symbol s_1_3[2] = { 't', 'a' };
static symbol s_1_4[3] = { 'l', 't', 'a' };
static symbol s_1_5[3] = { 's', 't', 'a' };

static struct among a_1[6] =
{
/*  0 */ { 3, s_1_0, -1, -1, 0},
/*  1 */ { 2, s_1_1, -1, -1, 0},
/*  2 */ { 3, s_1_2, -1, -1, 0},
/*  3 */ { 2, s_1_3, -1, -1, 0},
/*  4 */ { 3, s_1_4, 3, -1, 0},
/*  5 */ { 3, s_1_5, 3, -1, 0}
};

static symbol s_2_0[3] = { 'l', 'l', 0xE4 };
static symbol s_2_1[2] = { 'n', 0xE4 };
static symbol s_2_2[3] = { 's', 's', 0xE4 };
static symbol s_2_3[2] = { 't', 0xE4 };
static symbol s_2_4[3] = { 'l', 't', 0xE4 };
static symbol s_2_5[3] = { 's', 't', 0xE4 };

static struct among a_2[6] =
{
/*  0 */ { 3, s_2_0, -1, -1, 0},
/*  1 */ { 2, s_2_1, -1, -1, 0},
/*  2 */ { 3, s_2_2, -1, -1, 0},
/*  3 */ { 2, s_2_3, -1, -1, 0},
/*  4 */ { 3, s_2_4, 3, -1, 0},
/*  5 */ { 3, s_2_5, 3, -1, 0}
};

static symbol s_3_0[3] = { 'l', 'l', 'e' };
static symbol s_3_1[3] = { 'i', 'n', 'e' };

static struct among a_3[2] =
{
/*  0 */ { 3, s_3_0, -1, -1, 0},
/*  1 */ { 3, s_3_1, -1, -1, 0}
};

static symbol s_4_0[3] = { 'n', 's', 'a' };
static symbol s_4_1[3] = { 'm', 'm', 'e' };
static symbol s_4_2[3] = { 'n', 'n', 'e' };
static symbol s_4_3[2] = { 'n', 'i' };
static symbol s_4_4[2] = { 's', 'i' };
static symbol s_4_5[2] = { 'a', 'n' };
static symbol s_4_6[2] = { 'e', 'n' };
static symbol s_4_7[2] = { 0xE4, 'n' };
static symbol s_4_8[3] = { 'n', 's', 0xE4 };

static struct among a_4[9] =
{
/*  0 */ { 3, s_4_0, -1, 3, 0},
/*  1 */ { 3, s_4_1, -1, 3, 0},
/*  2 */ { 3, s_4_2, -1, 3, 0},
/*  3 */ { 2, s_4_3, -1, 2, 0},
/*  4 */ { 2, s_4_4, -1, 1, 0},
/*  5 */ { 2, s_4_5, -1, 4, 0},
/*  6 */ { 2, s_4_6, -1, 6, 0},
/*  7 */ { 2, s_4_7, -1, 5, 0},
/*  8 */ { 3, s_4_8, -1, 3, 0}
};

static symbol s_5_0[2] = { 'a', 'a' };
static symbol s_5_1[2] = { 'e', 'e' };
static symbol s_5_2[2] = { 'i', 'i' };
static symbol s_5_3[2] = { 'o', 'o' };
static symbol s_5_4[2] = { 'u', 'u' };
static symbol s_5_5[2] = { 0xE4, 0xE4 };
static symbol s_5_6[2] = { 0xF6, 0xF6 };

static struct among a_5[7] =
{
/*  0 */ { 2, s_5_0, -1, -1, 0},
/*  1 */ { 2, s_5_1, -1, -1, 0},
/*  2 */ { 2, s_5_2, -1, -1, 0},
/*  3 */ { 2, s_5_3, -1, -1, 0},
/*  4 */ { 2, s_5_4, -1, -1, 0},
/*  5 */ { 2, s_5_5, -1, -1, 0},
/*  6 */ { 2, s_5_6, -1, -1, 0}
};

static symbol s_6_0[1] = { 'a' };
static symbol s_6_1[3] = { 'l', 'l', 'a' };
static symbol s_6_2[2] = { 'n', 'a' };
static symbol s_6_3[3] = { 's', 's', 'a' };
static symbol s_6_4[2] = { 't', 'a' };
static symbol s_6_5[3] = { 'l', 't', 'a' };
static symbol s_6_6[3] = { 's', 't', 'a' };
static symbol s_6_7[3] = { 't', 't', 'a' };
static symbol s_6_8[3] = { 'l', 'l', 'e' };
static symbol s_6_9[3] = { 'i', 'n', 'e' };
static symbol s_6_10[3] = { 'k', 's', 'i' };
static symbol s_6_11[1] = { 'n' };
static symbol s_6_12[3] = { 'h', 'a', 'n' };
static symbol s_6_13[3] = { 'd', 'e', 'n' };
static symbol s_6_14[4] = { 's', 'e', 'e', 'n' };
static symbol s_6_15[3] = { 'h', 'e', 'n' };
static symbol s_6_16[4] = { 't', 't', 'e', 'n' };
static symbol s_6_17[3] = { 'h', 'i', 'n' };
static symbol s_6_18[4] = { 's', 'i', 'i', 'n' };
static symbol s_6_19[3] = { 'h', 'o', 'n' };
static symbol s_6_20[3] = { 'h', 0xE4, 'n' };
static symbol s_6_21[3] = { 'h', 0xF6, 'n' };
static symbol s_6_22[1] = { 0xE4 };
static symbol s_6_23[3] = { 'l', 'l', 0xE4 };
static symbol s_6_24[2] = { 'n', 0xE4 };
static symbol s_6_25[3] = { 's', 's', 0xE4 };
static symbol s_6_26[2] = { 't', 0xE4 };
static symbol s_6_27[3] = { 'l', 't', 0xE4 };
static symbol s_6_28[3] = { 's', 't', 0xE4 };
static symbol s_6_29[3] = { 't', 't', 0xE4 };

static struct among a_6[30] =
{
/*  0 */ { 1, s_6_0, -1, 8, 0},
/*  1 */ { 3, s_6_1, 0, -1, 0},
/*  2 */ { 2, s_6_2, 0, -1, 0},
/*  3 */ { 3, s_6_3, 0, -1, 0},
/*  4 */ { 2, s_6_4, 0, -1, 0},
/*  5 */ { 3, s_6_5, 4, -1, 0},
/*  6 */ { 3, s_6_6, 4, -1, 0},
/*  7 */ { 3, s_6_7, 4, 9, 0},
/*  8 */ { 3, s_6_8, -1, -1, 0},
/*  9 */ { 3, s_6_9, -1, -1, 0},
/* 10 */ { 3, s_6_10, -1, -1, 0},
/* 11 */ { 1, s_6_11, -1, 7, 0},
/* 12 */ { 3, s_6_12, 11, 1, 0},
/* 13 */ { 3, s_6_13, 11, -1, r_VI},
/* 14 */ { 4, s_6_14, 11, -1, r_LONG},
/* 15 */ { 3, s_6_15, 11, 2, 0},
/* 16 */ { 4, s_6_16, 11, -1, r_VI},
/* 17 */ { 3, s_6_17, 11, 3, 0},
/* 18 */ { 4, s_6_18, 11, -1, r_VI},
/* 19 */ { 3, s_6_19, 11, 4, 0},
/* 20 */ { 3, s_6_20, 11, 5, 0},
/* 21 */ { 3, s_6_21, 11, 6, 0},
/* 22 */ { 1, s_6_22, -1, 8, 0},
/* 23 */ { 3, s_6_23, 22, -1, 0},
/* 24 */ { 2, s_6_24, 22, -1, 0},
/* 25 */ { 3, s_6_25, 22, -1, 0},
/* 26 */ { 2, s_6_26, 22, -1, 0},
/* 27 */ { 3, s_6_27, 26, -1, 0},
/* 28 */ { 3, s_6_28, 26, -1, 0},
/* 29 */ { 3, s_6_29, 26, 9, 0}
};

static symbol s_7_0[3] = { 'e', 'j', 'a' };
static symbol s_7_1[3] = { 'm', 'm', 'a' };
static symbol s_7_2[4] = { 'i', 'm', 'm', 'a' };
static symbol s_7_3[3] = { 'm', 'p', 'a' };
static symbol s_7_4[4] = { 'i', 'm', 'p', 'a' };
static symbol s_7_5[3] = { 'm', 'm', 'i' };
static symbol s_7_6[4] = { 'i', 'm', 'm', 'i' };
static symbol s_7_7[3] = { 'm', 'p', 'i' };
static symbol s_7_8[4] = { 'i', 'm', 'p', 'i' };
static symbol s_7_9[3] = { 'e', 'j', 0xE4 };
static symbol s_7_10[3] = { 'm', 'm', 0xE4 };
static symbol s_7_11[4] = { 'i', 'm', 'm', 0xE4 };
static symbol s_7_12[3] = { 'm', 'p', 0xE4 };
static symbol s_7_13[4] = { 'i', 'm', 'p', 0xE4 };

static struct among a_7[14] =
{
/*  0 */ { 3, s_7_0, -1, -1, 0},
/*  1 */ { 3, s_7_1, -1, 1, 0},
/*  2 */ { 4, s_7_2, 1, -1, 0},
/*  3 */ { 3, s_7_3, -1, 1, 0},
/*  4 */ { 4, s_7_4, 3, -1, 0},
/*  5 */ { 3, s_7_5, -1, 1, 0},
/*  6 */ { 4, s_7_6, 5, -1, 0},
/*  7 */ { 3, s_7_7, -1, 1, 0},
/*  8 */ { 4, s_7_8, 7, -1, 0},
/*  9 */ { 3, s_7_9, -1, -1, 0},
/* 10 */ { 3, s_7_10, -1, 1, 0},
/* 11 */ { 4, s_7_11, 10, -1, 0},
/* 12 */ { 3, s_7_12, -1, 1, 0},
/* 13 */ { 4, s_7_13, 12, -1, 0}
};

static symbol s_8_0[1] = { 'i' };
static symbol s_8_1[1] = { 'j' };

static struct among a_8[2] =
{
/*  0 */ { 1, s_8_0, -1, -1, 0},
/*  1 */ { 1, s_8_1, -1, -1, 0}
};

static symbol s_9_0[3] = { 'm', 'm', 'a' };
static symbol s_9_1[4] = { 'i', 'm', 'm', 'a' };

static struct among a_9[2] =
{
/*  0 */ { 3, s_9_0, -1, 1, 0},
/*  1 */ { 4, s_9_1, 0, -1, 0}
};

static unsigned char g_AEI[] = { 17, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8 };

static unsigned char g_V1[] = { 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32 };

static unsigned char g_V2[] = { 17, 65, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32 };

static unsigned char g_particle_end[] = { 17, 97, 24, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32 };

static symbol s_0[] = { 'k' };
static symbol s_1[] = { 'k', 's', 'e' };
static symbol s_2[] = { 'k', 's', 'i' };
static symbol s_3[] = { 'i' };
static symbol s_4[] = { 'a' };
static symbol s_5[] = { 'e' };
static symbol s_6[] = { 'i' };
static symbol s_7[] = { 'o' };
static symbol s_8[] = { 0xE4 };
static symbol s_9[] = { 0xF6 };
static symbol s_10[] = { 'i', 'e' };
static symbol s_11[] = { 'e' };
static symbol s_12[] = { 'p', 'o' };
static symbol s_13[] = { 't' };
static symbol s_14[] = { 'p', 'o' };
static symbol s_15[] = { 'j' };
static symbol s_16[] = { 'o' };
static symbol s_17[] = { 'u' };
static symbol s_18[] = { 'o' };
static symbol s_19[] = { 'j' };

static int r_mark_regions(struct SN_env * z) {
    z->I[0] = z->l;
    z->I[1] = z->l;
    while(1) { /* goto, line 46 */
        int c = z->c;
        if (!(in_grouping(z, g_V1, 97, 246))) goto lab0;
        z->c = c;
        break;
    lab0:
        z->c = c;
        if (z->c >= z->l) return 0;
        z->c++; /* goto, line 46 */
    }
    while(1) { /* gopast, line 46 */
        if (!(out_grouping(z, g_V1, 97, 246))) goto lab1;
        break;
    lab1:
        if (z->c >= z->l) return 0;
        z->c++; /* gopast, line 46 */
    }
    z->I[0] = z->c; /* setmark p1, line 46 */
    while(1) { /* goto, line 47 */
        int c = z->c;
        if (!(in_grouping(z, g_V1, 97, 246))) goto lab2;
        z->c = c;
        break;
    lab2:
        z->c = c;
        if (z->c >= z->l) return 0;
        z->c++; /* goto, line 47 */
    }
    while(1) { /* gopast, line 47 */
        if (!(out_grouping(z, g_V1, 97, 246))) goto lab3;
        break;
    lab3:
        if (z->c >= z->l) return 0;
        z->c++; /* gopast, line 47 */
    }
    z->I[1] = z->c; /* setmark p2, line 47 */
    return 1;
}

static int r_R2(struct SN_env * z) {
    if (!(z->I[1] <= z->c)) return 0;
    return 1;
}

static int r_particle_etc(struct SN_env * z) {
    int among_var;
    {   int m3; /* setlimit, line 55 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 55 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 55 */
        among_var = find_among_b(z, a_0, 10); /* substring, line 55 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 55 */
        z->lb = m3;
    }
    switch(among_var) {
        case 0: return 0;
        case 1:
            if (!(in_grouping_b(z, g_particle_end, 97, 246))) return 0;
            break;
        case 2:
            {   int ret = r_R2(z);
                if (ret == 0) return 0; /* call R2, line 64 */
                if (ret < 0) return ret;
            }
            break;
    }
    {   int ret;
        ret = slice_del(z); /* delete, line 66 */
        if (ret < 0) return ret;
    }
    return 1;
}

static int r_possessive(struct SN_env * z) {
    int among_var;
    {   int m3; /* setlimit, line 69 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 69 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 69 */
        among_var = find_among_b(z, a_4, 9); /* substring, line 69 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 69 */
        z->lb = m3;
    }
    switch(among_var) {
        case 0: return 0;
        case 1:
            {   int m = z->l - z->c; (void) m; /* not, line 72 */
                if (!(eq_s_b(z, 1, s_0))) goto lab0;
                return 0;
            lab0:
                z->c = z->l - m;
            }
            {   int ret;
                ret = slice_del(z); /* delete, line 72 */
                if (ret < 0) return ret;
            }
            break;
        case 2:
            {   int ret;
                ret = slice_del(z); /* delete, line 74 */
                if (ret < 0) return ret;
            }
            z->ket = z->c; /* [, line 74 */
            if (!(eq_s_b(z, 3, s_1))) return 0;
            z->bra = z->c; /* ], line 74 */
            {   int ret;
                ret = slice_from_s(z, 3, s_2); /* <-, line 74 */
                if (ret < 0) return ret;
            }
            break;
        case 3:
            {   int ret;
                ret = slice_del(z); /* delete, line 78 */
                if (ret < 0) return ret;
            }
            break;
        case 4:
            if (!(find_among_b(z, a_1, 6))) return 0; /* among, line 81 */
            {   int ret;
                ret = slice_del(z); /* delete, line 81 */
                if (ret < 0) return ret;
            }
            break;
        case 5:
            if (!(find_among_b(z, a_2, 6))) return 0; /* among, line 83 */
            {   int ret;
                ret = slice_del(z); /* delete, line 84 */
                if (ret < 0) return ret;
            }
            break;
        case 6:
            if (!(find_among_b(z, a_3, 2))) return 0; /* among, line 86 */
            {   int ret;
                ret = slice_del(z); /* delete, line 86 */
                if (ret < 0) return ret;
            }
            break;
    }
    return 1;
}

static int r_LONG(struct SN_env * z) {
    if (!(find_among_b(z, a_5, 7))) return 0; /* among, line 91 */
    return 1;
}

static int r_VI(struct SN_env * z) {
    if (!(eq_s_b(z, 1, s_3))) return 0;
    if (!(in_grouping_b(z, g_V2, 97, 246))) return 0;
    return 1;
}

static int r_case_ending(struct SN_env * z) {
    int among_var;
    {   int m3; /* setlimit, line 96 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 96 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 96 */
        among_var = find_among_b(z, a_6, 30); /* substring, line 96 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 96 */
        z->lb = m3;
    }
    switch(among_var) {
        case 0: return 0;
        case 1:
            if (!(eq_s_b(z, 1, s_4))) return 0;
            break;
        case 2:
            if (!(eq_s_b(z, 1, s_5))) return 0;
            break;
        case 3:
            if (!(eq_s_b(z, 1, s_6))) return 0;
            break;
        case 4:
            if (!(eq_s_b(z, 1, s_7))) return 0;
            break;
        case 5:
            if (!(eq_s_b(z, 1, s_8))) return 0;
            break;
        case 6:
            if (!(eq_s_b(z, 1, s_9))) return 0;
            break;
        case 7:
            {   int m = z->l - z->c; (void) m; /* try, line 111 */
                {   int m = z->l - z->c; (void) m; /* and, line 113 */
                    {   int m = z->l - z->c; (void) m; /* or, line 112 */
                        {   int ret = r_LONG(z);
                            if (ret == 0) goto lab2; /* call LONG, line 111 */
                            if (ret < 0) return ret;
                        }
                        goto lab1;
                    lab2:
                        z->c = z->l - m;
                        if (!(eq_s_b(z, 2, s_10))) { z->c = z->l - m; goto lab0; }
                    }
                lab1:
                    z->c = z->l - m;
                    if (z->c <= z->lb) { z->c = z->l - m; goto lab0; }
                    z->c--; /* next, line 113 */
                }
                z->bra = z->c; /* ], line 113 */
            lab0:
                ;
            }
            break;
        case 8:
            if (!(in_grouping_b(z, g_V1, 97, 246))) return 0;
            if (!(out_grouping_b(z, g_V1, 97, 246))) return 0;
            break;
        case 9:
            if (!(eq_s_b(z, 1, s_11))) return 0;
            break;
    }
    {   int ret;
        ret = slice_del(z); /* delete, line 138 */
        if (ret < 0) return ret;
    }
    z->B[0] = 1; /* set ending_removed, line 139 */
    return 1;
}

static int r_other_endings(struct SN_env * z) {
    int among_var;
    {   int m3; /* setlimit, line 142 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[1]) return 0;
        z->c = z->I[1]; /* tomark, line 142 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 142 */
        among_var = find_among_b(z, a_7, 14); /* substring, line 142 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 142 */
        z->lb = m3;
    }
    switch(among_var) {
        case 0: return 0;
        case 1:
            {   int m = z->l - z->c; (void) m; /* not, line 146 */
                if (!(eq_s_b(z, 2, s_12))) goto lab0;
                return 0;
            lab0:
                z->c = z->l - m;
            }
            break;
    }
    {   int ret;
        ret = slice_del(z); /* delete, line 151 */
        if (ret < 0) return ret;
    }
    return 1;
}

static int r_i_plural(struct SN_env * z) {
    {   int m3; /* setlimit, line 154 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 154 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 154 */
        if (!(find_among_b(z, a_8, 2))) { z->lb = m3; return 0; } /* substring, line 154 */
        z->bra = z->c; /* ], line 154 */
        z->lb = m3;
    }
    {   int ret;
        ret = slice_del(z); /* delete, line 158 */
        if (ret < 0) return ret;
    }
    return 1;
}

static int r_t_plural(struct SN_env * z) {
    int among_var;
    {   int m3; /* setlimit, line 161 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 161 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 162 */
        if (!(eq_s_b(z, 1, s_13))) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 162 */
        {   int m_test = z->l - z->c; /* test, line 162 */
            if (!(in_grouping_b(z, g_V1, 97, 246))) { z->lb = m3; return 0; }
            z->c = z->l - m_test;
        }
        {   int ret;
            ret = slice_del(z); /* delete, line 163 */
            if (ret < 0) return ret;
        }
        z->lb = m3;
    }
    {   int m3; /* setlimit, line 165 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[1]) return 0;
        z->c = z->I[1]; /* tomark, line 165 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 165 */
        among_var = find_among_b(z, a_9, 2); /* substring, line 165 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 165 */
        z->lb = m3;
    }
    switch(among_var) {
        case 0: return 0;
        case 1:
            {   int m = z->l - z->c; (void) m; /* not, line 167 */
                if (!(eq_s_b(z, 2, s_14))) goto lab0;
                return 0;
            lab0:
                z->c = z->l - m;
            }
            break;
    }
    {   int ret;
        ret = slice_del(z); /* delete, line 170 */
        if (ret < 0) return ret;
    }
    return 1;
}

static int r_tidy(struct SN_env * z) {
    {   int m3; /* setlimit, line 173 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 173 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        {   int m = z->l - z->c; (void) m; /* do, line 174 */
            {   int m = z->l - z->c; (void) m; /* and, line 174 */
                {   int ret = r_LONG(z);
                    if (ret == 0) goto lab0; /* call LONG, line 174 */
                    if (ret < 0) return ret;
                }
                z->c = z->l - m;
                z->ket = z->c; /* [, line 174 */
                if (z->c <= z->lb) goto lab0;
                z->c--; /* next, line 174 */
                z->bra = z->c; /* ], line 174 */
                {   int ret;
                    ret = slice_del(z); /* delete, line 174 */
                    if (ret < 0) return ret;
                }
            }
        lab0:
            z->c = z->l - m;
        }
        {   int m = z->l - z->c; (void) m; /* do, line 175 */
            z->ket = z->c; /* [, line 175 */
            if (!(in_grouping_b(z, g_AEI, 97, 228))) goto lab1;
            z->bra = z->c; /* ], line 175 */
            if (!(out_grouping_b(z, g_V1, 97, 246))) goto lab1;
            {   int ret;
                ret = slice_del(z); /* delete, line 175 */
                if (ret < 0) return ret;
            }
        lab1:
            z->c = z->l - m;
        }
        {   int m = z->l - z->c; (void) m; /* do, line 176 */
            z->ket = z->c; /* [, line 176 */
            if (!(eq_s_b(z, 1, s_15))) goto lab2;
            z->bra = z->c; /* ], line 176 */
            {   int m = z->l - z->c; (void) m; /* or, line 176 */
                if (!(eq_s_b(z, 1, s_16))) goto lab4;
                goto lab3;
            lab4:
                z->c = z->l - m;
                if (!(eq_s_b(z, 1, s_17))) goto lab2;
            }
        lab3:
            {   int ret;
                ret = slice_del(z); /* delete, line 176 */
                if (ret < 0) return ret;
            }
        lab2:
            z->c = z->l - m;
        }
        {   int m = z->l - z->c; (void) m; /* do, line 177 */
            z->ket = z->c; /* [, line 177 */
            if (!(eq_s_b(z, 1, s_18))) goto lab5;
            z->bra = z->c; /* ], line 177 */
            if (!(eq_s_b(z, 1, s_19))) goto lab5;
            {   int ret;
                ret = slice_del(z); /* delete, line 177 */
                if (ret < 0) return ret;
            }
        lab5:
            z->c = z->l - m;
        }
        z->lb = m3;
    }
    while(1) { /* goto, line 179 */
        int m = z->l - z->c; (void) m;
        if (!(out_grouping_b(z, g_V1, 97, 246))) goto lab6;
        z->c = z->l - m;
        break;
    lab6:
        z->c = z->l - m;
        if (z->c <= z->lb) return 0;
        z->c--; /* goto, line 179 */
    }
    z->ket = z->c; /* [, line 179 */
    if (z->c <= z->lb) return 0;
    z->c--; /* next, line 179 */
    z->bra = z->c; /* ], line 179 */
    z->S[0] = slice_to(z, z->S[0]); /* -> x, line 179 */
    if (z->S[0] == 0) return -1; /* -> x, line 179 */
    if (!(eq_v_b(z, z->S[0]))) return 0; /* name x, line 179 */
    {   int ret;
        ret = slice_del(z); /* delete, line 179 */
        if (ret < 0) return ret;
    }
    return 1;
}

extern int finnish_ISO_8859_1_stem(struct SN_env * z) {
    {   int c = z->c; /* do, line 185 */
        {   int ret = r_mark_regions(z);
            if (ret == 0) goto lab0; /* call mark_regions, line 185 */
            if (ret < 0) return ret;
        }
    lab0:
        z->c = c;
    }
    z->B[0] = 0; /* unset ending_removed, line 186 */
    z->lb = z->c; z->c = z->l; /* backwards, line 187 */

    {   int m = z->l - z->c; (void) m; /* do, line 188 */
        {   int ret = r_particle_etc(z);
            if (ret == 0) goto lab1; /* call particle_etc, line 188 */
            if (ret < 0) return ret;
        }
    lab1:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* do, line 189 */
        {   int ret = r_possessive(z);
            if (ret == 0) goto lab2; /* call possessive, line 189 */
            if (ret < 0) return ret;
        }
    lab2:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* do, line 190 */
        {   int ret = r_case_ending(z);
            if (ret == 0) goto lab3; /* call case_ending, line 190 */
            if (ret < 0) return ret;
        }
    lab3:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* do, line 191 */
        {   int ret = r_other_endings(z);
            if (ret == 0) goto lab4; /* call other_endings, line 191 */
            if (ret < 0) return ret;
        }
    lab4:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* or, line 192 */
        if (!(z->B[0])) goto lab6; /* Boolean test ending_removed, line 192 */
        {   int m = z->l - z->c; (void) m; /* do, line 192 */
            {   int ret = r_i_plural(z);
                if (ret == 0) goto lab7; /* call i_plural, line 192 */
                if (ret < 0) return ret;
            }
        lab7:
            z->c = z->l - m;
        }
        goto lab5;
    lab6:
        z->c = z->l - m;
        {   int m = z->l - z->c; (void) m; /* do, line 192 */
            {   int ret = r_t_plural(z);
                if (ret == 0) goto lab8; /* call t_plural, line 192 */
                if (ret < 0) return ret;
            }
        lab8:
            z->c = z->l - m;
        }
    }
lab5:
    {   int m = z->l - z->c; (void) m; /* do, line 193 */
        {   int ret = r_tidy(z);
            if (ret == 0) goto lab9; /* call tidy, line 193 */
            if (ret < 0) return ret;
        }
    lab9:
        z->c = z->l - m;
    }
    z->c = z->lb;
    return 1;
}

extern struct SN_env * finnish_ISO_8859_1_create_env(void) { return SN_create_env(1, 2, 1); }

extern void finnish_ISO_8859_1_close_env(struct SN_env * z) { SN_close_env(z); }

