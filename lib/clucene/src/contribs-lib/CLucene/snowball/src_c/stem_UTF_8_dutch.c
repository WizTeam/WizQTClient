
/* This file was generated automatically by the Snowball to ANSI C compiler */

#include "../runtime/header.h"

extern int dutch_UTF_8_stem(struct SN_env * z);
static int r_standard_suffix(struct SN_env * z);
static int r_undouble(struct SN_env * z);
static int r_R2(struct SN_env * z);
static int r_R1(struct SN_env * z);
static int r_mark_regions(struct SN_env * z);
static int r_en_ending(struct SN_env * z);
static int r_e_ending(struct SN_env * z);
static int r_postlude(struct SN_env * z);
static int r_prelude(struct SN_env * z);

extern struct SN_env * dutch_UTF_8_create_env(void);
extern void dutch_UTF_8_close_env(struct SN_env * z);

static symbol s_0_1[2] = { 0xC3, 0xA1 };
static symbol s_0_2[2] = { 0xC3, 0xA4 };
static symbol s_0_3[2] = { 0xC3, 0xA9 };
static symbol s_0_4[2] = { 0xC3, 0xAB };
static symbol s_0_5[2] = { 0xC3, 0xAD };
static symbol s_0_6[2] = { 0xC3, 0xAF };
static symbol s_0_7[2] = { 0xC3, 0xB3 };
static symbol s_0_8[2] = { 0xC3, 0xB6 };
static symbol s_0_9[2] = { 0xC3, 0xBA };
static symbol s_0_10[2] = { 0xC3, 0xBC };

static struct among a_0[11] =
{
/*  0 */ { 0, 0, -1, 6, 0},
/*  1 */ { 2, s_0_1, 0, 1, 0},
/*  2 */ { 2, s_0_2, 0, 1, 0},
/*  3 */ { 2, s_0_3, 0, 2, 0},
/*  4 */ { 2, s_0_4, 0, 2, 0},
/*  5 */ { 2, s_0_5, 0, 3, 0},
/*  6 */ { 2, s_0_6, 0, 3, 0},
/*  7 */ { 2, s_0_7, 0, 4, 0},
/*  8 */ { 2, s_0_8, 0, 4, 0},
/*  9 */ { 2, s_0_9, 0, 5, 0},
/* 10 */ { 2, s_0_10, 0, 5, 0}
};

static symbol s_1_1[1] = { 'I' };
static symbol s_1_2[1] = { 'Y' };

static struct among a_1[3] =
{
/*  0 */ { 0, 0, -1, 3, 0},
/*  1 */ { 1, s_1_1, 0, 2, 0},
/*  2 */ { 1, s_1_2, 0, 1, 0}
};

static symbol s_2_0[2] = { 'd', 'd' };
static symbol s_2_1[2] = { 'k', 'k' };
static symbol s_2_2[2] = { 't', 't' };

static struct among a_2[3] =
{
/*  0 */ { 2, s_2_0, -1, -1, 0},
/*  1 */ { 2, s_2_1, -1, -1, 0},
/*  2 */ { 2, s_2_2, -1, -1, 0}
};

static symbol s_3_0[3] = { 'e', 'n', 'e' };
static symbol s_3_1[2] = { 's', 'e' };
static symbol s_3_2[2] = { 'e', 'n' };
static symbol s_3_3[5] = { 'h', 'e', 'd', 'e', 'n' };
static symbol s_3_4[1] = { 's' };

static struct among a_3[5] =
{
/*  0 */ { 3, s_3_0, -1, 2, 0},
/*  1 */ { 2, s_3_1, -1, 3, 0},
/*  2 */ { 2, s_3_2, -1, 2, 0},
/*  3 */ { 5, s_3_3, 2, 1, 0},
/*  4 */ { 1, s_3_4, -1, 3, 0}
};

static symbol s_4_0[3] = { 'e', 'n', 'd' };
static symbol s_4_1[2] = { 'i', 'g' };
static symbol s_4_2[3] = { 'i', 'n', 'g' };
static symbol s_4_3[4] = { 'l', 'i', 'j', 'k' };
static symbol s_4_4[4] = { 'b', 'a', 'a', 'r' };
static symbol s_4_5[3] = { 'b', 'a', 'r' };

static struct among a_4[6] =
{
/*  0 */ { 3, s_4_0, -1, 1, 0},
/*  1 */ { 2, s_4_1, -1, 2, 0},
/*  2 */ { 3, s_4_2, -1, 1, 0},
/*  3 */ { 4, s_4_3, -1, 3, 0},
/*  4 */ { 4, s_4_4, -1, 4, 0},
/*  5 */ { 3, s_4_5, -1, 5, 0}
};

static symbol s_5_0[2] = { 'a', 'a' };
static symbol s_5_1[2] = { 'e', 'e' };
static symbol s_5_2[2] = { 'o', 'o' };
static symbol s_5_3[2] = { 'u', 'u' };

static struct among a_5[4] =
{
/*  0 */ { 2, s_5_0, -1, -1, 0},
/*  1 */ { 2, s_5_1, -1, -1, 0},
/*  2 */ { 2, s_5_2, -1, -1, 0},
/*  3 */ { 2, s_5_3, -1, -1, 0}
};

static unsigned char g_v[] = { 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128 };

static unsigned char g_v_I[] = { 1, 0, 0, 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128 };

static unsigned char g_v_j[] = { 17, 67, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128 };

static symbol s_0[] = { 'a' };
static symbol s_1[] = { 'e' };
static symbol s_2[] = { 'i' };
static symbol s_3[] = { 'o' };
static symbol s_4[] = { 'u' };
static symbol s_5[] = { 'y' };
static symbol s_6[] = { 'Y' };
static symbol s_7[] = { 'i' };
static symbol s_8[] = { 'I' };
static symbol s_9[] = { 'y' };
static symbol s_10[] = { 'Y' };
static symbol s_11[] = { 'y' };
static symbol s_12[] = { 'i' };
static symbol s_13[] = { 'e' };
static symbol s_14[] = { 'g', 'e', 'm' };
static symbol s_15[] = { 'h', 'e', 'i', 'd' };
static symbol s_16[] = { 'h', 'e', 'i', 'd' };
static symbol s_17[] = { 'c' };
static symbol s_18[] = { 'e', 'n' };
static symbol s_19[] = { 'i', 'g' };
static symbol s_20[] = { 'e' };
static symbol s_21[] = { 'e' };

static int r_prelude(struct SN_env * z) {
    int among_var;
    {   int c_test = z->c; /* test, line 42 */
        while(1) { /* repeat, line 42 */
            int c = z->c;
            z->bra = z->c; /* [, line 43 */
            among_var = find_among(z, a_0, 11); /* substring, line 43 */
            if (!(among_var)) goto lab0;
            z->ket = z->c; /* ], line 43 */
            switch(among_var) {
                case 0: goto lab0;
                case 1:
                    {   int ret;
                        ret = slice_from_s(z, 1, s_0); /* <-, line 45 */
                        if (ret < 0) return ret;
                    }
                    break;
                case 2:
                    {   int ret;
                        ret = slice_from_s(z, 1, s_1); /* <-, line 47 */
                        if (ret < 0) return ret;
                    }
                    break;
                case 3:
                    {   int ret;
                        ret = slice_from_s(z, 1, s_2); /* <-, line 49 */
                        if (ret < 0) return ret;
                    }
                    break;
                case 4:
                    {   int ret;
                        ret = slice_from_s(z, 1, s_3); /* <-, line 51 */
                        if (ret < 0) return ret;
                    }
                    break;
                case 5:
                    {   int ret;
                        ret = slice_from_s(z, 1, s_4); /* <-, line 53 */
                        if (ret < 0) return ret;
                    }
                    break;
                case 6:
                    {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
                        if (c < 0) goto lab0;
                        z->c = c; /* next, line 54 */
                    }
                    break;
            }
            continue;
        lab0:
            z->c = c;
            break;
        }
        z->c = c_test;
    }
    {   int c = z->c; /* try, line 57 */
        z->bra = z->c; /* [, line 57 */
        if (!(eq_s(z, 1, s_5))) { z->c = c; goto lab1; }
        z->ket = z->c; /* ], line 57 */
        {   int ret;
            ret = slice_from_s(z, 1, s_6); /* <-, line 57 */
            if (ret < 0) return ret;
        }
    lab1:
        ;
    }
    while(1) { /* repeat, line 58 */
        int c = z->c;
        while(1) { /* goto, line 58 */
            int c = z->c;
            if (!(in_grouping_U(z, g_v, 97, 232))) goto lab3;
            z->bra = z->c; /* [, line 59 */
            {   int c = z->c; /* or, line 59 */
                if (!(eq_s(z, 1, s_7))) goto lab5;
                z->ket = z->c; /* ], line 59 */
                if (!(in_grouping_U(z, g_v, 97, 232))) goto lab5;
                {   int ret;
                    ret = slice_from_s(z, 1, s_8); /* <-, line 59 */
                    if (ret < 0) return ret;
                }
                goto lab4;
            lab5:
                z->c = c;
                if (!(eq_s(z, 1, s_9))) goto lab3;
                z->ket = z->c; /* ], line 60 */
                {   int ret;
                    ret = slice_from_s(z, 1, s_10); /* <-, line 60 */
                    if (ret < 0) return ret;
                }
            }
        lab4:
            z->c = c;
            break;
        lab3:
            z->c = c;
            {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
                if (c < 0) goto lab2;
                z->c = c; /* goto, line 58 */
            }
        }
        continue;
    lab2:
        z->c = c;
        break;
    }
    return 1;
}

static int r_mark_regions(struct SN_env * z) {
    z->I[0] = z->l;
    z->I[1] = z->l;
    while(1) { /* gopast, line 69 */
        if (!(in_grouping_U(z, g_v, 97, 232))) goto lab0;
        break;
    lab0:
        {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
            if (c < 0) return 0;
            z->c = c; /* gopast, line 69 */
        }
    }
    while(1) { /* gopast, line 69 */
        if (!(out_grouping_U(z, g_v, 97, 232))) goto lab1;
        break;
    lab1:
        {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
            if (c < 0) return 0;
            z->c = c; /* gopast, line 69 */
        }
    }
    z->I[0] = z->c; /* setmark p1, line 69 */
     /* try, line 70 */
    if (!(z->I[0] < 3)) goto lab2;
    z->I[0] = 3;
lab2:
    while(1) { /* gopast, line 71 */
        if (!(in_grouping_U(z, g_v, 97, 232))) goto lab3;
        break;
    lab3:
        {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
            if (c < 0) return 0;
            z->c = c; /* gopast, line 71 */
        }
    }
    while(1) { /* gopast, line 71 */
        if (!(out_grouping_U(z, g_v, 97, 232))) goto lab4;
        break;
    lab4:
        {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
            if (c < 0) return 0;
            z->c = c; /* gopast, line 71 */
        }
    }
    z->I[1] = z->c; /* setmark p2, line 71 */
    return 1;
}

static int r_postlude(struct SN_env * z) {
    int among_var;
    while(1) { /* repeat, line 75 */
        int c = z->c;
        z->bra = z->c; /* [, line 77 */
        among_var = find_among(z, a_1, 3); /* substring, line 77 */
        if (!(among_var)) goto lab0;
        z->ket = z->c; /* ], line 77 */
        switch(among_var) {
            case 0: goto lab0;
            case 1:
                {   int ret;
                    ret = slice_from_s(z, 1, s_11); /* <-, line 78 */
                    if (ret < 0) return ret;
                }
                break;
            case 2:
                {   int ret;
                    ret = slice_from_s(z, 1, s_12); /* <-, line 79 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
                    if (c < 0) goto lab0;
                    z->c = c; /* next, line 80 */
                }
                break;
        }
        continue;
    lab0:
        z->c = c;
        break;
    }
    return 1;
}

static int r_R1(struct SN_env * z) {
    if (!(z->I[0] <= z->c)) return 0;
    return 1;
}

static int r_R2(struct SN_env * z) {
    if (!(z->I[1] <= z->c)) return 0;
    return 1;
}

static int r_undouble(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 91 */
        if (!(find_among_b(z, a_2, 3))) return 0; /* among, line 91 */
        z->c = z->l - m_test;
    }
    z->ket = z->c; /* [, line 91 */
    {   int c = skip_utf8(z->p, z->c, z->lb, 0, -1);
        if (c < 0) return 0;
        z->c = c; /* next, line 91 */
    }
    z->bra = z->c; /* ], line 91 */
    {   int ret;
        ret = slice_del(z); /* delete, line 91 */
        if (ret < 0) return ret;
    }
    return 1;
}

static int r_e_ending(struct SN_env * z) {
    z->B[0] = 0; /* unset e_found, line 95 */
    z->ket = z->c; /* [, line 96 */
    if (!(eq_s_b(z, 1, s_13))) return 0;
    z->bra = z->c; /* ], line 96 */
    {   int ret = r_R1(z);
        if (ret == 0) return 0; /* call R1, line 96 */
        if (ret < 0) return ret;
    }
    {   int m_test = z->l - z->c; /* test, line 96 */
        if (!(out_grouping_b_U(z, g_v, 97, 232))) return 0;
        z->c = z->l - m_test;
    }
    {   int ret;
        ret = slice_del(z); /* delete, line 96 */
        if (ret < 0) return ret;
    }
    z->B[0] = 1; /* set e_found, line 97 */
    {   int ret = r_undouble(z);
        if (ret == 0) return 0; /* call undouble, line 98 */
        if (ret < 0) return ret;
    }
    return 1;
}

static int r_en_ending(struct SN_env * z) {
    {   int ret = r_R1(z);
        if (ret == 0) return 0; /* call R1, line 102 */
        if (ret < 0) return ret;
    }
    {   int m = z->l - z->c; (void) m; /* and, line 102 */
        if (!(out_grouping_b_U(z, g_v, 97, 232))) return 0;
        z->c = z->l - m;
        {   int m = z->l - z->c; (void) m; /* not, line 102 */
            if (!(eq_s_b(z, 3, s_14))) goto lab0;
            return 0;
        lab0:
            z->c = z->l - m;
        }
    }
    {   int ret;
        ret = slice_del(z); /* delete, line 102 */
        if (ret < 0) return ret;
    }
    {   int ret = r_undouble(z);
        if (ret == 0) return 0; /* call undouble, line 103 */
        if (ret < 0) return ret;
    }
    return 1;
}

static int r_standard_suffix(struct SN_env * z) {
    int among_var;
    {   int m = z->l - z->c; (void) m; /* do, line 107 */
        z->ket = z->c; /* [, line 108 */
        among_var = find_among_b(z, a_3, 5); /* substring, line 108 */
        if (!(among_var)) goto lab0;
        z->bra = z->c; /* ], line 108 */
        switch(among_var) {
            case 0: goto lab0;
            case 1:
                {   int ret = r_R1(z);
                    if (ret == 0) goto lab0; /* call R1, line 110 */
                    if (ret < 0) return ret;
                }
                {   int ret;
                    ret = slice_from_s(z, 4, s_15); /* <-, line 110 */
                    if (ret < 0) return ret;
                }
                break;
            case 2:
                {   int ret = r_en_ending(z);
                    if (ret == 0) goto lab0; /* call en_ending, line 113 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                {   int ret = r_R1(z);
                    if (ret == 0) goto lab0; /* call R1, line 116 */
                    if (ret < 0) return ret;
                }
                if (!(out_grouping_b_U(z, g_v_j, 97, 232))) goto lab0;
                {   int ret;
                    ret = slice_del(z); /* delete, line 116 */
                    if (ret < 0) return ret;
                }
                break;
        }
    lab0:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* do, line 120 */
        {   int ret = r_e_ending(z);
            if (ret == 0) goto lab1; /* call e_ending, line 120 */
            if (ret < 0) return ret;
        }
    lab1:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* do, line 122 */
        z->ket = z->c; /* [, line 122 */
        if (!(eq_s_b(z, 4, s_16))) goto lab2;
        z->bra = z->c; /* ], line 122 */
        {   int ret = r_R2(z);
            if (ret == 0) goto lab2; /* call R2, line 122 */
            if (ret < 0) return ret;
        }
        {   int m = z->l - z->c; (void) m; /* not, line 122 */
            if (!(eq_s_b(z, 1, s_17))) goto lab3;
            goto lab2;
        lab3:
            z->c = z->l - m;
        }
        {   int ret;
            ret = slice_del(z); /* delete, line 122 */
            if (ret < 0) return ret;
        }
        z->ket = z->c; /* [, line 123 */
        if (!(eq_s_b(z, 2, s_18))) goto lab2;
        z->bra = z->c; /* ], line 123 */
        {   int ret = r_en_ending(z);
            if (ret == 0) goto lab2; /* call en_ending, line 123 */
            if (ret < 0) return ret;
        }
    lab2:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* do, line 126 */
        z->ket = z->c; /* [, line 127 */
        among_var = find_among_b(z, a_4, 6); /* substring, line 127 */
        if (!(among_var)) goto lab4;
        z->bra = z->c; /* ], line 127 */
        switch(among_var) {
            case 0: goto lab4;
            case 1:
                {   int ret = r_R2(z);
                    if (ret == 0) goto lab4; /* call R2, line 129 */
                    if (ret < 0) return ret;
                }
                {   int ret;
                    ret = slice_del(z); /* delete, line 129 */
                    if (ret < 0) return ret;
                }
                {   int m = z->l - z->c; (void) m; /* or, line 130 */
                    z->ket = z->c; /* [, line 130 */
                    if (!(eq_s_b(z, 2, s_19))) goto lab6;
                    z->bra = z->c; /* ], line 130 */
                    {   int ret = r_R2(z);
                        if (ret == 0) goto lab6; /* call R2, line 130 */
                        if (ret < 0) return ret;
                    }
                    {   int m = z->l - z->c; (void) m; /* not, line 130 */
                        if (!(eq_s_b(z, 1, s_20))) goto lab7;
                        goto lab6;
                    lab7:
                        z->c = z->l - m;
                    }
                    {   int ret;
                        ret = slice_del(z); /* delete, line 130 */
                        if (ret < 0) return ret;
                    }
                    goto lab5;
                lab6:
                    z->c = z->l - m;
                    {   int ret = r_undouble(z);
                        if (ret == 0) goto lab4; /* call undouble, line 130 */
                        if (ret < 0) return ret;
                    }
                }
            lab5:
                break;
            case 2:
                {   int ret = r_R2(z);
                    if (ret == 0) goto lab4; /* call R2, line 133 */
                    if (ret < 0) return ret;
                }
                {   int m = z->l - z->c; (void) m; /* not, line 133 */
                    if (!(eq_s_b(z, 1, s_21))) goto lab8;
                    goto lab4;
                lab8:
                    z->c = z->l - m;
                }
                {   int ret;
                    ret = slice_del(z); /* delete, line 133 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                {   int ret = r_R2(z);
                    if (ret == 0) goto lab4; /* call R2, line 136 */
                    if (ret < 0) return ret;
                }
                {   int ret;
                    ret = slice_del(z); /* delete, line 136 */
                    if (ret < 0) return ret;
                }
                {   int ret = r_e_ending(z);
                    if (ret == 0) goto lab4; /* call e_ending, line 136 */
                    if (ret < 0) return ret;
                }
                break;
            case 4:
                {   int ret = r_R2(z);
                    if (ret == 0) goto lab4; /* call R2, line 139 */
                    if (ret < 0) return ret;
                }
                {   int ret;
                    ret = slice_del(z); /* delete, line 139 */
                    if (ret < 0) return ret;
                }
                break;
            case 5:
                {   int ret = r_R2(z);
                    if (ret == 0) goto lab4; /* call R2, line 142 */
                    if (ret < 0) return ret;
                }
                if (!(z->B[0])) goto lab4; /* Boolean test e_found, line 142 */
                {   int ret;
                    ret = slice_del(z); /* delete, line 142 */
                    if (ret < 0) return ret;
                }
                break;
        }
    lab4:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* do, line 146 */
        if (!(out_grouping_b_U(z, g_v_I, 73, 232))) goto lab9;
        {   int m_test = z->l - z->c; /* test, line 148 */
            if (!(find_among_b(z, a_5, 4))) goto lab9; /* among, line 149 */
            if (!(out_grouping_b_U(z, g_v, 97, 232))) goto lab9;
            z->c = z->l - m_test;
        }
        z->ket = z->c; /* [, line 152 */
        {   int c = skip_utf8(z->p, z->c, z->lb, 0, -1);
            if (c < 0) goto lab9;
            z->c = c; /* next, line 152 */
        }
        z->bra = z->c; /* ], line 152 */
        {   int ret;
            ret = slice_del(z); /* delete, line 152 */
            if (ret < 0) return ret;
        }
    lab9:
        z->c = z->l - m;
    }
    return 1;
}

extern int dutch_UTF_8_stem(struct SN_env * z) {
    {   int c = z->c; /* do, line 159 */
        {   int ret = r_prelude(z);
            if (ret == 0) goto lab0; /* call prelude, line 159 */
            if (ret < 0) return ret;
        }
    lab0:
        z->c = c;
    }
    {   int c = z->c; /* do, line 160 */
        {   int ret = r_mark_regions(z);
            if (ret == 0) goto lab1; /* call mark_regions, line 160 */
            if (ret < 0) return ret;
        }
    lab1:
        z->c = c;
    }
    z->lb = z->c; z->c = z->l; /* backwards, line 161 */

    {   int m = z->l - z->c; (void) m; /* do, line 162 */
        {   int ret = r_standard_suffix(z);
            if (ret == 0) goto lab2; /* call standard_suffix, line 162 */
            if (ret < 0) return ret;
        }
    lab2:
        z->c = z->l - m;
    }
    z->c = z->lb;
    {   int c = z->c; /* do, line 163 */
        {   int ret = r_postlude(z);
            if (ret == 0) goto lab3; /* call postlude, line 163 */
            if (ret < 0) return ret;
        }
    lab3:
        z->c = c;
    }
    return 1;
}

extern struct SN_env * dutch_UTF_8_create_env(void) { return SN_create_env(0, 2, 1); }

extern void dutch_UTF_8_close_env(struct SN_env * z) { SN_close_env(z); }

