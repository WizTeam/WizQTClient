
/* This file was generated automatically by the Snowball to ANSI C compiler */

#include "../runtime/header.h"

extern int german_ISO_8859_1_stem(struct SN_env * z);
static int r_standard_suffix(struct SN_env * z);
static int r_R2(struct SN_env * z);
static int r_R1(struct SN_env * z);
static int r_mark_regions(struct SN_env * z);
static int r_postlude(struct SN_env * z);
static int r_prelude(struct SN_env * z);

extern struct SN_env * german_ISO_8859_1_create_env(void);
extern void german_ISO_8859_1_close_env(struct SN_env * z);

static symbol s_0_1[1] = { 'U' };
static symbol s_0_2[1] = { 'Y' };
static symbol s_0_3[1] = { 0xE4 };
static symbol s_0_4[1] = { 0xF6 };
static symbol s_0_5[1] = { 0xFC };

static struct among a_0[6] =
{
/*  0 */ { 0, 0, -1, 6, 0},
/*  1 */ { 1, s_0_1, 0, 2, 0},
/*  2 */ { 1, s_0_2, 0, 1, 0},
/*  3 */ { 1, s_0_3, 0, 3, 0},
/*  4 */ { 1, s_0_4, 0, 4, 0},
/*  5 */ { 1, s_0_5, 0, 5, 0}
};

static symbol s_1_0[1] = { 'e' };
static symbol s_1_1[2] = { 'e', 'm' };
static symbol s_1_2[2] = { 'e', 'n' };
static symbol s_1_3[3] = { 'e', 'r', 'n' };
static symbol s_1_4[2] = { 'e', 'r' };
static symbol s_1_5[1] = { 's' };
static symbol s_1_6[2] = { 'e', 's' };

static struct among a_1[7] =
{
/*  0 */ { 1, s_1_0, -1, 1, 0},
/*  1 */ { 2, s_1_1, -1, 1, 0},
/*  2 */ { 2, s_1_2, -1, 1, 0},
/*  3 */ { 3, s_1_3, -1, 1, 0},
/*  4 */ { 2, s_1_4, -1, 1, 0},
/*  5 */ { 1, s_1_5, -1, 2, 0},
/*  6 */ { 2, s_1_6, 5, 1, 0}
};

static symbol s_2_0[2] = { 'e', 'n' };
static symbol s_2_1[2] = { 'e', 'r' };
static symbol s_2_2[2] = { 's', 't' };
static symbol s_2_3[3] = { 'e', 's', 't' };

static struct among a_2[4] =
{
/*  0 */ { 2, s_2_0, -1, 1, 0},
/*  1 */ { 2, s_2_1, -1, 1, 0},
/*  2 */ { 2, s_2_2, -1, 2, 0},
/*  3 */ { 3, s_2_3, 2, 1, 0}
};

static symbol s_3_0[2] = { 'i', 'g' };
static symbol s_3_1[4] = { 'l', 'i', 'c', 'h' };

static struct among a_3[2] =
{
/*  0 */ { 2, s_3_0, -1, 1, 0},
/*  1 */ { 4, s_3_1, -1, 1, 0}
};

static symbol s_4_0[3] = { 'e', 'n', 'd' };
static symbol s_4_1[2] = { 'i', 'g' };
static symbol s_4_2[3] = { 'u', 'n', 'g' };
static symbol s_4_3[4] = { 'l', 'i', 'c', 'h' };
static symbol s_4_4[4] = { 'i', 's', 'c', 'h' };
static symbol s_4_5[2] = { 'i', 'k' };
static symbol s_4_6[4] = { 'h', 'e', 'i', 't' };
static symbol s_4_7[4] = { 'k', 'e', 'i', 't' };

static struct among a_4[8] =
{
/*  0 */ { 3, s_4_0, -1, 1, 0},
/*  1 */ { 2, s_4_1, -1, 2, 0},
/*  2 */ { 3, s_4_2, -1, 1, 0},
/*  3 */ { 4, s_4_3, -1, 3, 0},
/*  4 */ { 4, s_4_4, -1, 2, 0},
/*  5 */ { 2, s_4_5, -1, 2, 0},
/*  6 */ { 4, s_4_6, -1, 3, 0},
/*  7 */ { 4, s_4_7, -1, 4, 0}
};

static unsigned char g_v[] = { 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 32, 8 };

static unsigned char g_s_ending[] = { 117, 30, 5 };

static unsigned char g_st_ending[] = { 117, 30, 4 };

static symbol s_0[] = { 0xDF };
static symbol s_1[] = { 's', 's' };
static symbol s_2[] = { 'u' };
static symbol s_3[] = { 'U' };
static symbol s_4[] = { 'y' };
static symbol s_5[] = { 'Y' };
static symbol s_6[] = { 'y' };
static symbol s_7[] = { 'u' };
static symbol s_8[] = { 'a' };
static symbol s_9[] = { 'o' };
static symbol s_10[] = { 'u' };
static symbol s_11[] = { 'i', 'g' };
static symbol s_12[] = { 'e' };
static symbol s_13[] = { 'e' };
static symbol s_14[] = { 'e', 'r' };
static symbol s_15[] = { 'e', 'n' };

static int r_prelude(struct SN_env * z) {
    {   int c_test = z->c; /* test, line 30 */
        while(1) { /* repeat, line 30 */
            int c = z->c;
            {   int c = z->c; /* or, line 33 */
                z->bra = z->c; /* [, line 32 */
                if (!(eq_s(z, 1, s_0))) goto lab2;
                z->ket = z->c; /* ], line 32 */
                {   int ret;
                    ret = slice_from_s(z, 2, s_1); /* <-, line 32 */
                    if (ret < 0) return ret;
                }
                goto lab1;
            lab2:
                z->c = c;
                if (z->c >= z->l) goto lab0;
                z->c++; /* next, line 33 */
            }
        lab1:
            continue;
        lab0:
            z->c = c;
            break;
        }
        z->c = c_test;
    }
    while(1) { /* repeat, line 36 */
        int c = z->c;
        while(1) { /* goto, line 36 */
            int c = z->c;
            if (!(in_grouping(z, g_v, 97, 252))) goto lab4;
            z->bra = z->c; /* [, line 37 */
            {   int c = z->c; /* or, line 37 */
                if (!(eq_s(z, 1, s_2))) goto lab6;
                z->ket = z->c; /* ], line 37 */
                if (!(in_grouping(z, g_v, 97, 252))) goto lab6;
                {   int ret;
                    ret = slice_from_s(z, 1, s_3); /* <-, line 37 */
                    if (ret < 0) return ret;
                }
                goto lab5;
            lab6:
                z->c = c;
                if (!(eq_s(z, 1, s_4))) goto lab4;
                z->ket = z->c; /* ], line 38 */
                if (!(in_grouping(z, g_v, 97, 252))) goto lab4;
                {   int ret;
                    ret = slice_from_s(z, 1, s_5); /* <-, line 38 */
                    if (ret < 0) return ret;
                }
            }
        lab5:
            z->c = c;
            break;
        lab4:
            z->c = c;
            if (z->c >= z->l) goto lab3;
            z->c++; /* goto, line 36 */
        }
        continue;
    lab3:
        z->c = c;
        break;
    }
    return 1;
}

static int r_mark_regions(struct SN_env * z) {
    z->I[0] = z->l;
    z->I[1] = z->l;
    {   int c_test = z->c; /* test, line 47 */
        {   int c = z->c + 3;
            if (0 > c || c > z->l) return 0;
            z->c = c; /* hop, line 47 */
        }
        z->I[2] = z->c; /* setmark x, line 47 */
        z->c = c_test;
    }
    while(1) { /* gopast, line 49 */
        if (!(in_grouping(z, g_v, 97, 252))) goto lab0;
        break;
    lab0:
        if (z->c >= z->l) return 0;
        z->c++; /* gopast, line 49 */
    }
    while(1) { /* gopast, line 49 */
        if (!(out_grouping(z, g_v, 97, 252))) goto lab1;
        break;
    lab1:
        if (z->c >= z->l) return 0;
        z->c++; /* gopast, line 49 */
    }
    z->I[0] = z->c; /* setmark p1, line 49 */
     /* try, line 50 */
    if (!(z->I[0] < z->I[2])) goto lab2;
    z->I[0] = z->I[2];
lab2:
    while(1) { /* gopast, line 51 */
        if (!(in_grouping(z, g_v, 97, 252))) goto lab3;
        break;
    lab3:
        if (z->c >= z->l) return 0;
        z->c++; /* gopast, line 51 */
    }
    while(1) { /* gopast, line 51 */
        if (!(out_grouping(z, g_v, 97, 252))) goto lab4;
        break;
    lab4:
        if (z->c >= z->l) return 0;
        z->c++; /* gopast, line 51 */
    }
    z->I[1] = z->c; /* setmark p2, line 51 */
    return 1;
}

static int r_postlude(struct SN_env * z) {
    int among_var;
    while(1) { /* repeat, line 55 */
        int c = z->c;
        z->bra = z->c; /* [, line 57 */
        among_var = find_among(z, a_0, 6); /* substring, line 57 */
        if (!(among_var)) goto lab0;
        z->ket = z->c; /* ], line 57 */
        switch(among_var) {
            case 0: goto lab0;
            case 1:
                {   int ret;
                    ret = slice_from_s(z, 1, s_6); /* <-, line 58 */
                    if (ret < 0) return ret;
                }
                break;
            case 2:
                {   int ret;
                    ret = slice_from_s(z, 1, s_7); /* <-, line 59 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                {   int ret;
                    ret = slice_from_s(z, 1, s_8); /* <-, line 60 */
                    if (ret < 0) return ret;
                }
                break;
            case 4:
                {   int ret;
                    ret = slice_from_s(z, 1, s_9); /* <-, line 61 */
                    if (ret < 0) return ret;
                }
                break;
            case 5:
                {   int ret;
                    ret = slice_from_s(z, 1, s_10); /* <-, line 62 */
                    if (ret < 0) return ret;
                }
                break;
            case 6:
                if (z->c >= z->l) goto lab0;
                z->c++; /* next, line 63 */
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

static int r_standard_suffix(struct SN_env * z) {
    int among_var;
    {   int m = z->l - z->c; (void) m; /* do, line 74 */
        z->ket = z->c; /* [, line 75 */
        among_var = find_among_b(z, a_1, 7); /* substring, line 75 */
        if (!(among_var)) goto lab0;
        z->bra = z->c; /* ], line 75 */
        {   int ret = r_R1(z);
            if (ret == 0) goto lab0; /* call R1, line 75 */
            if (ret < 0) return ret;
        }
        switch(among_var) {
            case 0: goto lab0;
            case 1:
                {   int ret;
                    ret = slice_del(z); /* delete, line 77 */
                    if (ret < 0) return ret;
                }
                break;
            case 2:
                if (!(in_grouping_b(z, g_s_ending, 98, 116))) goto lab0;
                {   int ret;
                    ret = slice_del(z); /* delete, line 80 */
                    if (ret < 0) return ret;
                }
                break;
        }
    lab0:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* do, line 84 */
        z->ket = z->c; /* [, line 85 */
        among_var = find_among_b(z, a_2, 4); /* substring, line 85 */
        if (!(among_var)) goto lab1;
        z->bra = z->c; /* ], line 85 */
        {   int ret = r_R1(z);
            if (ret == 0) goto lab1; /* call R1, line 85 */
            if (ret < 0) return ret;
        }
        switch(among_var) {
            case 0: goto lab1;
            case 1:
                {   int ret;
                    ret = slice_del(z); /* delete, line 87 */
                    if (ret < 0) return ret;
                }
                break;
            case 2:
                if (!(in_grouping_b(z, g_st_ending, 98, 116))) goto lab1;
                {   int c = z->c - 3;
                    if (z->lb > c || c > z->l) goto lab1;
                    z->c = c; /* hop, line 90 */
                }
                {   int ret;
                    ret = slice_del(z); /* delete, line 90 */
                    if (ret < 0) return ret;
                }
                break;
        }
    lab1:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* do, line 94 */
        z->ket = z->c; /* [, line 95 */
        among_var = find_among_b(z, a_4, 8); /* substring, line 95 */
        if (!(among_var)) goto lab2;
        z->bra = z->c; /* ], line 95 */
        {   int ret = r_R2(z);
            if (ret == 0) goto lab2; /* call R2, line 95 */
            if (ret < 0) return ret;
        }
        switch(among_var) {
            case 0: goto lab2;
            case 1:
                {   int ret;
                    ret = slice_del(z); /* delete, line 97 */
                    if (ret < 0) return ret;
                }
                {   int m = z->l - z->c; (void) m; /* try, line 98 */
                    z->ket = z->c; /* [, line 98 */
                    if (!(eq_s_b(z, 2, s_11))) { z->c = z->l - m; goto lab3; }
                    z->bra = z->c; /* ], line 98 */
                    {   int m = z->l - z->c; (void) m; /* not, line 98 */
                        if (!(eq_s_b(z, 1, s_12))) goto lab4;
                        { z->c = z->l - m; goto lab3; }
                    lab4:
                        z->c = z->l - m;
                    }
                    {   int ret = r_R2(z);
                        if (ret == 0) { z->c = z->l - m; goto lab3; } /* call R2, line 98 */
                        if (ret < 0) return ret;
                    }
                    {   int ret;
                        ret = slice_del(z); /* delete, line 98 */
                        if (ret < 0) return ret;
                    }
                lab3:
                    ;
                }
                break;
            case 2:
                {   int m = z->l - z->c; (void) m; /* not, line 101 */
                    if (!(eq_s_b(z, 1, s_13))) goto lab5;
                    goto lab2;
                lab5:
                    z->c = z->l - m;
                }
                {   int ret;
                    ret = slice_del(z); /* delete, line 101 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                {   int ret;
                    ret = slice_del(z); /* delete, line 104 */
                    if (ret < 0) return ret;
                }
                {   int m = z->l - z->c; (void) m; /* try, line 105 */
                    z->ket = z->c; /* [, line 106 */
                    {   int m = z->l - z->c; (void) m; /* or, line 106 */
                        if (!(eq_s_b(z, 2, s_14))) goto lab8;
                        goto lab7;
                    lab8:
                        z->c = z->l - m;
                        if (!(eq_s_b(z, 2, s_15))) { z->c = z->l - m; goto lab6; }
                    }
                lab7:
                    z->bra = z->c; /* ], line 106 */
                    {   int ret = r_R1(z);
                        if (ret == 0) { z->c = z->l - m; goto lab6; } /* call R1, line 106 */
                        if (ret < 0) return ret;
                    }
                    {   int ret;
                        ret = slice_del(z); /* delete, line 106 */
                        if (ret < 0) return ret;
                    }
                lab6:
                    ;
                }
                break;
            case 4:
                {   int ret;
                    ret = slice_del(z); /* delete, line 110 */
                    if (ret < 0) return ret;
                }
                {   int m = z->l - z->c; (void) m; /* try, line 111 */
                    z->ket = z->c; /* [, line 112 */
                    among_var = find_among_b(z, a_3, 2); /* substring, line 112 */
                    if (!(among_var)) { z->c = z->l - m; goto lab9; }
                    z->bra = z->c; /* ], line 112 */
                    {   int ret = r_R2(z);
                        if (ret == 0) { z->c = z->l - m; goto lab9; } /* call R2, line 112 */
                        if (ret < 0) return ret;
                    }
                    switch(among_var) {
                        case 0: { z->c = z->l - m; goto lab9; }
                        case 1:
                            {   int ret;
                                ret = slice_del(z); /* delete, line 114 */
                                if (ret < 0) return ret;
                            }
                            break;
                    }
                lab9:
                    ;
                }
                break;
        }
    lab2:
        z->c = z->l - m;
    }
    return 1;
}

extern int german_ISO_8859_1_stem(struct SN_env * z) {
    {   int c = z->c; /* do, line 125 */
        {   int ret = r_prelude(z);
            if (ret == 0) goto lab0; /* call prelude, line 125 */
            if (ret < 0) return ret;
        }
    lab0:
        z->c = c;
    }
    {   int c = z->c; /* do, line 126 */
        {   int ret = r_mark_regions(z);
            if (ret == 0) goto lab1; /* call mark_regions, line 126 */
            if (ret < 0) return ret;
        }
    lab1:
        z->c = c;
    }
    z->lb = z->c; z->c = z->l; /* backwards, line 127 */

    {   int m = z->l - z->c; (void) m; /* do, line 128 */
        {   int ret = r_standard_suffix(z);
            if (ret == 0) goto lab2; /* call standard_suffix, line 128 */
            if (ret < 0) return ret;
        }
    lab2:
        z->c = z->l - m;
    }
    z->c = z->lb;
    {   int c = z->c; /* do, line 129 */
        {   int ret = r_postlude(z);
            if (ret == 0) goto lab3; /* call postlude, line 129 */
            if (ret < 0) return ret;
        }
    lab3:
        z->c = c;
    }
    return 1;
}

extern struct SN_env * german_ISO_8859_1_create_env(void) { return SN_create_env(0, 3, 0); }

extern void german_ISO_8859_1_close_env(struct SN_env * z) { SN_close_env(z); }

