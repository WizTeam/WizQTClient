
/* This file was generated automatically by the Snowball to ANSI C compiler */

#include "../runtime/header.h"

extern int french_UTF_8_stem(struct SN_env * z);
static int r_un_accent(struct SN_env * z);
static int r_un_double(struct SN_env * z);
static int r_residual_suffix(struct SN_env * z);
static int r_verb_suffix(struct SN_env * z);
static int r_i_verb_suffix(struct SN_env * z);
static int r_standard_suffix(struct SN_env * z);
static int r_R2(struct SN_env * z);
static int r_R1(struct SN_env * z);
static int r_RV(struct SN_env * z);
static int r_mark_regions(struct SN_env * z);
static int r_postlude(struct SN_env * z);
static int r_prelude(struct SN_env * z);

extern struct SN_env * french_UTF_8_create_env(void);
extern void french_UTF_8_close_env(struct SN_env * z);

static symbol s_0_1[1] = { 'I' };
static symbol s_0_2[1] = { 'U' };
static symbol s_0_3[1] = { 'Y' };

static struct among a_0[4] =
{
/*  0 */ { 0, 0, -1, 4, 0},
/*  1 */ { 1, s_0_1, 0, 1, 0},
/*  2 */ { 1, s_0_2, 0, 2, 0},
/*  3 */ { 1, s_0_3, 0, 3, 0}
};

static symbol s_1_0[3] = { 'i', 'q', 'U' };
static symbol s_1_1[3] = { 'a', 'b', 'l' };
static symbol s_1_2[4] = { 'I', 0xC3, 0xA8, 'r' };
static symbol s_1_3[4] = { 'i', 0xC3, 0xA8, 'r' };
static symbol s_1_4[3] = { 'e', 'u', 's' };
static symbol s_1_5[2] = { 'i', 'v' };

static struct among a_1[6] =
{
/*  0 */ { 3, s_1_0, -1, 3, 0},
/*  1 */ { 3, s_1_1, -1, 3, 0},
/*  2 */ { 4, s_1_2, -1, 4, 0},
/*  3 */ { 4, s_1_3, -1, 4, 0},
/*  4 */ { 3, s_1_4, -1, 2, 0},
/*  5 */ { 2, s_1_5, -1, 1, 0}
};

static symbol s_2_0[2] = { 'i', 'c' };
static symbol s_2_1[4] = { 'a', 'b', 'i', 'l' };
static symbol s_2_2[2] = { 'i', 'v' };

static struct among a_2[3] =
{
/*  0 */ { 2, s_2_0, -1, 2, 0},
/*  1 */ { 4, s_2_1, -1, 1, 0},
/*  2 */ { 2, s_2_2, -1, 3, 0}
};

static symbol s_3_0[4] = { 'i', 'q', 'U', 'e' };
static symbol s_3_1[6] = { 'a', 't', 'r', 'i', 'c', 'e' };
static symbol s_3_2[4] = { 'a', 'n', 'c', 'e' };
static symbol s_3_3[4] = { 'e', 'n', 'c', 'e' };
static symbol s_3_4[5] = { 'l', 'o', 'g', 'i', 'e' };
static symbol s_3_5[4] = { 'a', 'b', 'l', 'e' };
static symbol s_3_6[4] = { 'i', 's', 'm', 'e' };
static symbol s_3_7[4] = { 'e', 'u', 's', 'e' };
static symbol s_3_8[4] = { 'i', 's', 't', 'e' };
static symbol s_3_9[3] = { 'i', 'v', 'e' };
static symbol s_3_10[2] = { 'i', 'f' };
static symbol s_3_11[5] = { 'u', 's', 'i', 'o', 'n' };
static symbol s_3_12[5] = { 'a', 't', 'i', 'o', 'n' };
static symbol s_3_13[5] = { 'u', 't', 'i', 'o', 'n' };
static symbol s_3_14[5] = { 'a', 't', 'e', 'u', 'r' };
static symbol s_3_15[5] = { 'i', 'q', 'U', 'e', 's' };
static symbol s_3_16[7] = { 'a', 't', 'r', 'i', 'c', 'e', 's' };
static symbol s_3_17[5] = { 'a', 'n', 'c', 'e', 's' };
static symbol s_3_18[5] = { 'e', 'n', 'c', 'e', 's' };
static symbol s_3_19[6] = { 'l', 'o', 'g', 'i', 'e', 's' };
static symbol s_3_20[5] = { 'a', 'b', 'l', 'e', 's' };
static symbol s_3_21[5] = { 'i', 's', 'm', 'e', 's' };
static symbol s_3_22[5] = { 'e', 'u', 's', 'e', 's' };
static symbol s_3_23[5] = { 'i', 's', 't', 'e', 's' };
static symbol s_3_24[4] = { 'i', 'v', 'e', 's' };
static symbol s_3_25[3] = { 'i', 'f', 's' };
static symbol s_3_26[6] = { 'u', 's', 'i', 'o', 'n', 's' };
static symbol s_3_27[6] = { 'a', 't', 'i', 'o', 'n', 's' };
static symbol s_3_28[6] = { 'u', 't', 'i', 'o', 'n', 's' };
static symbol s_3_29[6] = { 'a', 't', 'e', 'u', 'r', 's' };
static symbol s_3_30[5] = { 'm', 'e', 'n', 't', 's' };
static symbol s_3_31[6] = { 'e', 'm', 'e', 'n', 't', 's' };
static symbol s_3_32[9] = { 'i', 's', 's', 'e', 'm', 'e', 'n', 't', 's' };
static symbol s_3_33[5] = { 'i', 't', 0xC3, 0xA9, 's' };
static symbol s_3_34[4] = { 'm', 'e', 'n', 't' };
static symbol s_3_35[5] = { 'e', 'm', 'e', 'n', 't' };
static symbol s_3_36[8] = { 'i', 's', 's', 'e', 'm', 'e', 'n', 't' };
static symbol s_3_37[6] = { 'a', 'm', 'm', 'e', 'n', 't' };
static symbol s_3_38[6] = { 'e', 'm', 'm', 'e', 'n', 't' };
static symbol s_3_39[3] = { 'a', 'u', 'x' };
static symbol s_3_40[4] = { 'e', 'a', 'u', 'x' };
static symbol s_3_41[3] = { 'e', 'u', 'x' };
static symbol s_3_42[4] = { 'i', 't', 0xC3, 0xA9 };

static struct among a_3[43] =
{
/*  0 */ { 4, s_3_0, -1, 1, 0},
/*  1 */ { 6, s_3_1, -1, 2, 0},
/*  2 */ { 4, s_3_2, -1, 1, 0},
/*  3 */ { 4, s_3_3, -1, 5, 0},
/*  4 */ { 5, s_3_4, -1, 3, 0},
/*  5 */ { 4, s_3_5, -1, 1, 0},
/*  6 */ { 4, s_3_6, -1, 1, 0},
/*  7 */ { 4, s_3_7, -1, 11, 0},
/*  8 */ { 4, s_3_8, -1, 1, 0},
/*  9 */ { 3, s_3_9, -1, 8, 0},
/* 10 */ { 2, s_3_10, -1, 8, 0},
/* 11 */ { 5, s_3_11, -1, 4, 0},
/* 12 */ { 5, s_3_12, -1, 2, 0},
/* 13 */ { 5, s_3_13, -1, 4, 0},
/* 14 */ { 5, s_3_14, -1, 2, 0},
/* 15 */ { 5, s_3_15, -1, 1, 0},
/* 16 */ { 7, s_3_16, -1, 2, 0},
/* 17 */ { 5, s_3_17, -1, 1, 0},
/* 18 */ { 5, s_3_18, -1, 5, 0},
/* 19 */ { 6, s_3_19, -1, 3, 0},
/* 20 */ { 5, s_3_20, -1, 1, 0},
/* 21 */ { 5, s_3_21, -1, 1, 0},
/* 22 */ { 5, s_3_22, -1, 11, 0},
/* 23 */ { 5, s_3_23, -1, 1, 0},
/* 24 */ { 4, s_3_24, -1, 8, 0},
/* 25 */ { 3, s_3_25, -1, 8, 0},
/* 26 */ { 6, s_3_26, -1, 4, 0},
/* 27 */ { 6, s_3_27, -1, 2, 0},
/* 28 */ { 6, s_3_28, -1, 4, 0},
/* 29 */ { 6, s_3_29, -1, 2, 0},
/* 30 */ { 5, s_3_30, -1, 15, 0},
/* 31 */ { 6, s_3_31, 30, 6, 0},
/* 32 */ { 9, s_3_32, 31, 12, 0},
/* 33 */ { 5, s_3_33, -1, 7, 0},
/* 34 */ { 4, s_3_34, -1, 15, 0},
/* 35 */ { 5, s_3_35, 34, 6, 0},
/* 36 */ { 8, s_3_36, 35, 12, 0},
/* 37 */ { 6, s_3_37, 34, 13, 0},
/* 38 */ { 6, s_3_38, 34, 14, 0},
/* 39 */ { 3, s_3_39, -1, 10, 0},
/* 40 */ { 4, s_3_40, 39, 9, 0},
/* 41 */ { 3, s_3_41, -1, 1, 0},
/* 42 */ { 4, s_3_42, -1, 7, 0}
};

static symbol s_4_0[3] = { 'i', 'r', 'a' };
static symbol s_4_1[2] = { 'i', 'e' };
static symbol s_4_2[4] = { 'i', 's', 's', 'e' };
static symbol s_4_3[7] = { 'i', 's', 's', 'a', 'n', 't', 'e' };
static symbol s_4_4[1] = { 'i' };
static symbol s_4_5[4] = { 'i', 'r', 'a', 'i' };
static symbol s_4_6[2] = { 'i', 'r' };
static symbol s_4_7[4] = { 'i', 'r', 'a', 's' };
static symbol s_4_8[3] = { 'i', 'e', 's' };
static symbol s_4_9[5] = { 0xC3, 0xAE, 'm', 'e', 's' };
static symbol s_4_10[5] = { 'i', 's', 's', 'e', 's' };
static symbol s_4_11[8] = { 'i', 's', 's', 'a', 'n', 't', 'e', 's' };
static symbol s_4_12[5] = { 0xC3, 0xAE, 't', 'e', 's' };
static symbol s_4_13[2] = { 'i', 's' };
static symbol s_4_14[5] = { 'i', 'r', 'a', 'i', 's' };
static symbol s_4_15[6] = { 'i', 's', 's', 'a', 'i', 's' };
static symbol s_4_16[6] = { 'i', 'r', 'i', 'o', 'n', 's' };
static symbol s_4_17[7] = { 'i', 's', 's', 'i', 'o', 'n', 's' };
static symbol s_4_18[5] = { 'i', 'r', 'o', 'n', 's' };
static symbol s_4_19[6] = { 'i', 's', 's', 'o', 'n', 's' };
static symbol s_4_20[7] = { 'i', 's', 's', 'a', 'n', 't', 's' };
static symbol s_4_21[2] = { 'i', 't' };
static symbol s_4_22[5] = { 'i', 'r', 'a', 'i', 't' };
static symbol s_4_23[6] = { 'i', 's', 's', 'a', 'i', 't' };
static symbol s_4_24[6] = { 'i', 's', 's', 'a', 'n', 't' };
static symbol s_4_25[7] = { 'i', 'r', 'a', 'I', 'e', 'n', 't' };
static symbol s_4_26[8] = { 'i', 's', 's', 'a', 'I', 'e', 'n', 't' };
static symbol s_4_27[5] = { 'i', 'r', 'e', 'n', 't' };
static symbol s_4_28[6] = { 'i', 's', 's', 'e', 'n', 't' };
static symbol s_4_29[5] = { 'i', 'r', 'o', 'n', 't' };
static symbol s_4_30[3] = { 0xC3, 0xAE, 't' };
static symbol s_4_31[5] = { 'i', 'r', 'i', 'e', 'z' };
static symbol s_4_32[6] = { 'i', 's', 's', 'i', 'e', 'z' };
static symbol s_4_33[4] = { 'i', 'r', 'e', 'z' };
static symbol s_4_34[5] = { 'i', 's', 's', 'e', 'z' };

static struct among a_4[35] =
{
/*  0 */ { 3, s_4_0, -1, 1, 0},
/*  1 */ { 2, s_4_1, -1, 1, 0},
/*  2 */ { 4, s_4_2, -1, 1, 0},
/*  3 */ { 7, s_4_3, -1, 1, 0},
/*  4 */ { 1, s_4_4, -1, 1, 0},
/*  5 */ { 4, s_4_5, 4, 1, 0},
/*  6 */ { 2, s_4_6, -1, 1, 0},
/*  7 */ { 4, s_4_7, -1, 1, 0},
/*  8 */ { 3, s_4_8, -1, 1, 0},
/*  9 */ { 5, s_4_9, -1, 1, 0},
/* 10 */ { 5, s_4_10, -1, 1, 0},
/* 11 */ { 8, s_4_11, -1, 1, 0},
/* 12 */ { 5, s_4_12, -1, 1, 0},
/* 13 */ { 2, s_4_13, -1, 1, 0},
/* 14 */ { 5, s_4_14, 13, 1, 0},
/* 15 */ { 6, s_4_15, 13, 1, 0},
/* 16 */ { 6, s_4_16, -1, 1, 0},
/* 17 */ { 7, s_4_17, -1, 1, 0},
/* 18 */ { 5, s_4_18, -1, 1, 0},
/* 19 */ { 6, s_4_19, -1, 1, 0},
/* 20 */ { 7, s_4_20, -1, 1, 0},
/* 21 */ { 2, s_4_21, -1, 1, 0},
/* 22 */ { 5, s_4_22, 21, 1, 0},
/* 23 */ { 6, s_4_23, 21, 1, 0},
/* 24 */ { 6, s_4_24, -1, 1, 0},
/* 25 */ { 7, s_4_25, -1, 1, 0},
/* 26 */ { 8, s_4_26, -1, 1, 0},
/* 27 */ { 5, s_4_27, -1, 1, 0},
/* 28 */ { 6, s_4_28, -1, 1, 0},
/* 29 */ { 5, s_4_29, -1, 1, 0},
/* 30 */ { 3, s_4_30, -1, 1, 0},
/* 31 */ { 5, s_4_31, -1, 1, 0},
/* 32 */ { 6, s_4_32, -1, 1, 0},
/* 33 */ { 4, s_4_33, -1, 1, 0},
/* 34 */ { 5, s_4_34, -1, 1, 0}
};

static symbol s_5_0[1] = { 'a' };
static symbol s_5_1[3] = { 'e', 'r', 'a' };
static symbol s_5_2[4] = { 'a', 's', 's', 'e' };
static symbol s_5_3[4] = { 'a', 'n', 't', 'e' };
static symbol s_5_4[3] = { 0xC3, 0xA9, 'e' };
static symbol s_5_5[2] = { 'a', 'i' };
static symbol s_5_6[4] = { 'e', 'r', 'a', 'i' };
static symbol s_5_7[2] = { 'e', 'r' };
static symbol s_5_8[2] = { 'a', 's' };
static symbol s_5_9[4] = { 'e', 'r', 'a', 's' };
static symbol s_5_10[5] = { 0xC3, 0xA2, 'm', 'e', 's' };
static symbol s_5_11[5] = { 'a', 's', 's', 'e', 's' };
static symbol s_5_12[5] = { 'a', 'n', 't', 'e', 's' };
static symbol s_5_13[5] = { 0xC3, 0xA2, 't', 'e', 's' };
static symbol s_5_14[4] = { 0xC3, 0xA9, 'e', 's' };
static symbol s_5_15[3] = { 'a', 'i', 's' };
static symbol s_5_16[5] = { 'e', 'r', 'a', 'i', 's' };
static symbol s_5_17[4] = { 'i', 'o', 'n', 's' };
static symbol s_5_18[6] = { 'e', 'r', 'i', 'o', 'n', 's' };
static symbol s_5_19[7] = { 'a', 's', 's', 'i', 'o', 'n', 's' };
static symbol s_5_20[5] = { 'e', 'r', 'o', 'n', 's' };
static symbol s_5_21[4] = { 'a', 'n', 't', 's' };
static symbol s_5_22[3] = { 0xC3, 0xA9, 's' };
static symbol s_5_23[3] = { 'a', 'i', 't' };
static symbol s_5_24[5] = { 'e', 'r', 'a', 'i', 't' };
static symbol s_5_25[3] = { 'a', 'n', 't' };
static symbol s_5_26[5] = { 'a', 'I', 'e', 'n', 't' };
static symbol s_5_27[7] = { 'e', 'r', 'a', 'I', 'e', 'n', 't' };
static symbol s_5_28[6] = { 0xC3, 0xA8, 'r', 'e', 'n', 't' };
static symbol s_5_29[6] = { 'a', 's', 's', 'e', 'n', 't' };
static symbol s_5_30[5] = { 'e', 'r', 'o', 'n', 't' };
static symbol s_5_31[3] = { 0xC3, 0xA2, 't' };
static symbol s_5_32[2] = { 'e', 'z' };
static symbol s_5_33[3] = { 'i', 'e', 'z' };
static symbol s_5_34[5] = { 'e', 'r', 'i', 'e', 'z' };
static symbol s_5_35[6] = { 'a', 's', 's', 'i', 'e', 'z' };
static symbol s_5_36[4] = { 'e', 'r', 'e', 'z' };
static symbol s_5_37[2] = { 0xC3, 0xA9 };

static struct among a_5[38] =
{
/*  0 */ { 1, s_5_0, -1, 3, 0},
/*  1 */ { 3, s_5_1, 0, 2, 0},
/*  2 */ { 4, s_5_2, -1, 3, 0},
/*  3 */ { 4, s_5_3, -1, 3, 0},
/*  4 */ { 3, s_5_4, -1, 2, 0},
/*  5 */ { 2, s_5_5, -1, 3, 0},
/*  6 */ { 4, s_5_6, 5, 2, 0},
/*  7 */ { 2, s_5_7, -1, 2, 0},
/*  8 */ { 2, s_5_8, -1, 3, 0},
/*  9 */ { 4, s_5_9, 8, 2, 0},
/* 10 */ { 5, s_5_10, -1, 3, 0},
/* 11 */ { 5, s_5_11, -1, 3, 0},
/* 12 */ { 5, s_5_12, -1, 3, 0},
/* 13 */ { 5, s_5_13, -1, 3, 0},
/* 14 */ { 4, s_5_14, -1, 2, 0},
/* 15 */ { 3, s_5_15, -1, 3, 0},
/* 16 */ { 5, s_5_16, 15, 2, 0},
/* 17 */ { 4, s_5_17, -1, 1, 0},
/* 18 */ { 6, s_5_18, 17, 2, 0},
/* 19 */ { 7, s_5_19, 17, 3, 0},
/* 20 */ { 5, s_5_20, -1, 2, 0},
/* 21 */ { 4, s_5_21, -1, 3, 0},
/* 22 */ { 3, s_5_22, -1, 2, 0},
/* 23 */ { 3, s_5_23, -1, 3, 0},
/* 24 */ { 5, s_5_24, 23, 2, 0},
/* 25 */ { 3, s_5_25, -1, 3, 0},
/* 26 */ { 5, s_5_26, -1, 3, 0},
/* 27 */ { 7, s_5_27, 26, 2, 0},
/* 28 */ { 6, s_5_28, -1, 2, 0},
/* 29 */ { 6, s_5_29, -1, 3, 0},
/* 30 */ { 5, s_5_30, -1, 2, 0},
/* 31 */ { 3, s_5_31, -1, 3, 0},
/* 32 */ { 2, s_5_32, -1, 2, 0},
/* 33 */ { 3, s_5_33, 32, 2, 0},
/* 34 */ { 5, s_5_34, 33, 2, 0},
/* 35 */ { 6, s_5_35, 33, 3, 0},
/* 36 */ { 4, s_5_36, 32, 2, 0},
/* 37 */ { 2, s_5_37, -1, 2, 0}
};

static symbol s_6_0[1] = { 'e' };
static symbol s_6_1[5] = { 'I', 0xC3, 0xA8, 'r', 'e' };
static symbol s_6_2[5] = { 'i', 0xC3, 0xA8, 'r', 'e' };
static symbol s_6_3[3] = { 'i', 'o', 'n' };
static symbol s_6_4[3] = { 'I', 'e', 'r' };
static symbol s_6_5[3] = { 'i', 'e', 'r' };
static symbol s_6_6[2] = { 0xC3, 0xAB };

static struct among a_6[7] =
{
/*  0 */ { 1, s_6_0, -1, 3, 0},
/*  1 */ { 5, s_6_1, 0, 2, 0},
/*  2 */ { 5, s_6_2, 0, 2, 0},
/*  3 */ { 3, s_6_3, -1, 1, 0},
/*  4 */ { 3, s_6_4, -1, 2, 0},
/*  5 */ { 3, s_6_5, -1, 2, 0},
/*  6 */ { 2, s_6_6, -1, 4, 0}
};

static symbol s_7_0[3] = { 'e', 'l', 'l' };
static symbol s_7_1[4] = { 'e', 'i', 'l', 'l' };
static symbol s_7_2[3] = { 'e', 'n', 'n' };
static symbol s_7_3[3] = { 'o', 'n', 'n' };
static symbol s_7_4[3] = { 'e', 't', 't' };

static struct among a_7[5] =
{
/*  0 */ { 3, s_7_0, -1, -1, 0},
/*  1 */ { 4, s_7_1, -1, -1, 0},
/*  2 */ { 3, s_7_2, -1, -1, 0},
/*  3 */ { 3, s_7_3, -1, -1, 0},
/*  4 */ { 3, s_7_4, -1, -1, 0}
};

static unsigned char g_v[] = { 17, 65, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 130, 103, 8, 5 };

static unsigned char g_keep_with_s[] = { 1, 65, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128 };

static symbol s_0[] = { 'u' };
static symbol s_1[] = { 'U' };
static symbol s_2[] = { 'i' };
static symbol s_3[] = { 'I' };
static symbol s_4[] = { 'y' };
static symbol s_5[] = { 'Y' };
static symbol s_6[] = { 'y' };
static symbol s_7[] = { 'Y' };
static symbol s_8[] = { 'q' };
static symbol s_9[] = { 'u' };
static symbol s_10[] = { 'U' };
static symbol s_11[] = { 'i' };
static symbol s_12[] = { 'u' };
static symbol s_13[] = { 'y' };
static symbol s_14[] = { 'i', 'c' };
static symbol s_15[] = { 'i', 'q', 'U' };
static symbol s_16[] = { 'l', 'o', 'g' };
static symbol s_17[] = { 'u' };
static symbol s_18[] = { 'e', 'n', 't' };
static symbol s_19[] = { 'a', 't' };
static symbol s_20[] = { 'e', 'u', 'x' };
static symbol s_21[] = { 'i' };
static symbol s_22[] = { 'a', 'b', 'l' };
static symbol s_23[] = { 'i', 'q', 'U' };
static symbol s_24[] = { 'a', 't' };
static symbol s_25[] = { 'i', 'c' };
static symbol s_26[] = { 'i', 'q', 'U' };
static symbol s_27[] = { 'e', 'a', 'u' };
static symbol s_28[] = { 'a', 'l' };
static symbol s_29[] = { 'e', 'u', 'x' };
static symbol s_30[] = { 'a', 'n', 't' };
static symbol s_31[] = { 'e', 'n', 't' };
static symbol s_32[] = { 'e' };
static symbol s_33[] = { 's' };
static symbol s_34[] = { 's' };
static symbol s_35[] = { 't' };
static symbol s_36[] = { 'i' };
static symbol s_37[] = { 'g', 'u' };
static symbol s_38[] = { 0xC3, 0xA9 };
static symbol s_39[] = { 0xC3, 0xA8 };
static symbol s_40[] = { 'e' };
static symbol s_41[] = { 'Y' };
static symbol s_42[] = { 'i' };
static symbol s_43[] = { 0xC3, 0xA7 };
static symbol s_44[] = { 'c' };

static int r_prelude(struct SN_env * z) {
    while(1) { /* repeat, line 38 */
        int c = z->c;
        while(1) { /* goto, line 38 */
            int c = z->c;
            {   int c = z->c; /* or, line 44 */
                if (!(in_grouping_U(z, g_v, 97, 251))) goto lab3;
                z->bra = z->c; /* [, line 40 */
                {   int c = z->c; /* or, line 40 */
                    if (!(eq_s(z, 1, s_0))) goto lab5;
                    z->ket = z->c; /* ], line 40 */
                    if (!(in_grouping_U(z, g_v, 97, 251))) goto lab5;
                    {   int ret;
                        ret = slice_from_s(z, 1, s_1); /* <-, line 40 */
                        if (ret < 0) return ret;
                    }
                    goto lab4;
                lab5:
                    z->c = c;
                    if (!(eq_s(z, 1, s_2))) goto lab6;
                    z->ket = z->c; /* ], line 41 */
                    if (!(in_grouping_U(z, g_v, 97, 251))) goto lab6;
                    {   int ret;
                        ret = slice_from_s(z, 1, s_3); /* <-, line 41 */
                        if (ret < 0) return ret;
                    }
                    goto lab4;
                lab6:
                    z->c = c;
                    if (!(eq_s(z, 1, s_4))) goto lab3;
                    z->ket = z->c; /* ], line 42 */
                    {   int ret;
                        ret = slice_from_s(z, 1, s_5); /* <-, line 42 */
                        if (ret < 0) return ret;
                    }
                }
            lab4:
                goto lab2;
            lab3:
                z->c = c;
                z->bra = z->c; /* [, line 45 */
                if (!(eq_s(z, 1, s_6))) goto lab7;
                z->ket = z->c; /* ], line 45 */
                if (!(in_grouping_U(z, g_v, 97, 251))) goto lab7;
                {   int ret;
                    ret = slice_from_s(z, 1, s_7); /* <-, line 45 */
                    if (ret < 0) return ret;
                }
                goto lab2;
            lab7:
                z->c = c;
                if (!(eq_s(z, 1, s_8))) goto lab1;
                z->bra = z->c; /* [, line 47 */
                if (!(eq_s(z, 1, s_9))) goto lab1;
                z->ket = z->c; /* ], line 47 */
                {   int ret;
                    ret = slice_from_s(z, 1, s_10); /* <-, line 47 */
                    if (ret < 0) return ret;
                }
            }
        lab2:
            z->c = c;
            break;
        lab1:
            z->c = c;
            {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
                if (c < 0) goto lab0;
                z->c = c; /* goto, line 38 */
            }
        }
        continue;
    lab0:
        z->c = c;
        break;
    }
    return 1;
}

static int r_mark_regions(struct SN_env * z) {
    z->I[0] = z->l;
    z->I[1] = z->l;
    z->I[2] = z->l;
    {   int c = z->c; /* do, line 56 */
        {   int c = z->c; /* or, line 57 */
            if (!(in_grouping_U(z, g_v, 97, 251))) goto lab2;
            if (!(in_grouping_U(z, g_v, 97, 251))) goto lab2;
            {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
                if (c < 0) goto lab2;
                z->c = c; /* next, line 57 */
            }
            goto lab1;
        lab2:
            z->c = c;
            {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
                if (c < 0) goto lab0;
                z->c = c; /* next, line 57 */
            }
            while(1) { /* gopast, line 57 */
                if (!(in_grouping_U(z, g_v, 97, 251))) goto lab3;
                break;
            lab3:
                {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
                    if (c < 0) goto lab0;
                    z->c = c; /* gopast, line 57 */
                }
            }
        }
    lab1:
        z->I[0] = z->c; /* setmark pV, line 58 */
    lab0:
        z->c = c;
    }
    {   int c = z->c; /* do, line 60 */
        while(1) { /* gopast, line 61 */
            if (!(in_grouping_U(z, g_v, 97, 251))) goto lab5;
            break;
        lab5:
            {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
                if (c < 0) goto lab4;
                z->c = c; /* gopast, line 61 */
            }
        }
        while(1) { /* gopast, line 61 */
            if (!(out_grouping_U(z, g_v, 97, 251))) goto lab6;
            break;
        lab6:
            {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
                if (c < 0) goto lab4;
                z->c = c; /* gopast, line 61 */
            }
        }
        z->I[1] = z->c; /* setmark p1, line 61 */
        while(1) { /* gopast, line 62 */
            if (!(in_grouping_U(z, g_v, 97, 251))) goto lab7;
            break;
        lab7:
            {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
                if (c < 0) goto lab4;
                z->c = c; /* gopast, line 62 */
            }
        }
        while(1) { /* gopast, line 62 */
            if (!(out_grouping_U(z, g_v, 97, 251))) goto lab8;
            break;
        lab8:
            {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
                if (c < 0) goto lab4;
                z->c = c; /* gopast, line 62 */
            }
        }
        z->I[2] = z->c; /* setmark p2, line 62 */
    lab4:
        z->c = c;
    }
    return 1;
}

static int r_postlude(struct SN_env * z) {
    int among_var;
    while(1) { /* repeat, line 66 */
        int c = z->c;
        z->bra = z->c; /* [, line 68 */
        among_var = find_among(z, a_0, 4); /* substring, line 68 */
        if (!(among_var)) goto lab0;
        z->ket = z->c; /* ], line 68 */
        switch(among_var) {
            case 0: goto lab0;
            case 1:
                {   int ret;
                    ret = slice_from_s(z, 1, s_11); /* <-, line 69 */
                    if (ret < 0) return ret;
                }
                break;
            case 2:
                {   int ret;
                    ret = slice_from_s(z, 1, s_12); /* <-, line 70 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                {   int ret;
                    ret = slice_from_s(z, 1, s_13); /* <-, line 71 */
                    if (ret < 0) return ret;
                }
                break;
            case 4:
                {   int c = skip_utf8(z->p, z->c, 0, z->l, 1);
                    if (c < 0) goto lab0;
                    z->c = c; /* next, line 72 */
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

static int r_RV(struct SN_env * z) {
    if (!(z->I[0] <= z->c)) return 0;
    return 1;
}

static int r_R1(struct SN_env * z) {
    if (!(z->I[1] <= z->c)) return 0;
    return 1;
}

static int r_R2(struct SN_env * z) {
    if (!(z->I[2] <= z->c)) return 0;
    return 1;
}

static int r_standard_suffix(struct SN_env * z) {
    int among_var;
    z->ket = z->c; /* [, line 83 */
    among_var = find_among_b(z, a_3, 43); /* substring, line 83 */
    if (!(among_var)) return 0;
    z->bra = z->c; /* ], line 83 */
    switch(among_var) {
        case 0: return 0;
        case 1:
            {   int ret = r_R2(z);
                if (ret == 0) return 0; /* call R2, line 87 */
                if (ret < 0) return ret;
            }
            {   int ret;
                ret = slice_del(z); /* delete, line 87 */
                if (ret < 0) return ret;
            }
            break;
        case 2:
            {   int ret = r_R2(z);
                if (ret == 0) return 0; /* call R2, line 90 */
                if (ret < 0) return ret;
            }
            {   int ret;
                ret = slice_del(z); /* delete, line 90 */
                if (ret < 0) return ret;
            }
            {   int m = z->l - z->c; (void) m; /* try, line 91 */
                z->ket = z->c; /* [, line 91 */
                if (!(eq_s_b(z, 2, s_14))) { z->c = z->l - m; goto lab0; }
                z->bra = z->c; /* ], line 91 */
                {   int m = z->l - z->c; (void) m; /* or, line 91 */
                    {   int ret = r_R2(z);
                        if (ret == 0) goto lab2; /* call R2, line 91 */
                        if (ret < 0) return ret;
                    }
                    {   int ret;
                        ret = slice_del(z); /* delete, line 91 */
                        if (ret < 0) return ret;
                    }
                    goto lab1;
                lab2:
                    z->c = z->l - m;
                    {   int ret;
                        ret = slice_from_s(z, 3, s_15); /* <-, line 91 */
                        if (ret < 0) return ret;
                    }
                }
            lab1:
            lab0:
                ;
            }
            break;
        case 3:
            {   int ret = r_R2(z);
                if (ret == 0) return 0; /* call R2, line 95 */
                if (ret < 0) return ret;
            }
            {   int ret;
                ret = slice_from_s(z, 3, s_16); /* <-, line 95 */
                if (ret < 0) return ret;
            }
            break;
        case 4:
            {   int ret = r_R2(z);
                if (ret == 0) return 0; /* call R2, line 98 */
                if (ret < 0) return ret;
            }
            {   int ret;
                ret = slice_from_s(z, 1, s_17); /* <-, line 98 */
                if (ret < 0) return ret;
            }
            break;
        case 5:
            {   int ret = r_R2(z);
                if (ret == 0) return 0; /* call R2, line 101 */
                if (ret < 0) return ret;
            }
            {   int ret;
                ret = slice_from_s(z, 3, s_18); /* <-, line 101 */
                if (ret < 0) return ret;
            }
            break;
        case 6:
            {   int ret = r_RV(z);
                if (ret == 0) return 0; /* call RV, line 105 */
                if (ret < 0) return ret;
            }
            {   int ret;
                ret = slice_del(z); /* delete, line 105 */
                if (ret < 0) return ret;
            }
            {   int m = z->l - z->c; (void) m; /* try, line 106 */
                z->ket = z->c; /* [, line 107 */
                among_var = find_among_b(z, a_1, 6); /* substring, line 107 */
                if (!(among_var)) { z->c = z->l - m; goto lab3; }
                z->bra = z->c; /* ], line 107 */
                switch(among_var) {
                    case 0: { z->c = z->l - m; goto lab3; }
                    case 1:
                        {   int ret = r_R2(z);
                            if (ret == 0) { z->c = z->l - m; goto lab3; } /* call R2, line 108 */
                            if (ret < 0) return ret;
                        }
                        {   int ret;
                            ret = slice_del(z); /* delete, line 108 */
                            if (ret < 0) return ret;
                        }
                        z->ket = z->c; /* [, line 108 */
                        if (!(eq_s_b(z, 2, s_19))) { z->c = z->l - m; goto lab3; }
                        z->bra = z->c; /* ], line 108 */
                        {   int ret = r_R2(z);
                            if (ret == 0) { z->c = z->l - m; goto lab3; } /* call R2, line 108 */
                            if (ret < 0) return ret;
                        }
                        {   int ret;
                            ret = slice_del(z); /* delete, line 108 */
                            if (ret < 0) return ret;
                        }
                        break;
                    case 2:
                        {   int m = z->l - z->c; (void) m; /* or, line 109 */
                            {   int ret = r_R2(z);
                                if (ret == 0) goto lab5; /* call R2, line 109 */
                                if (ret < 0) return ret;
                            }
                            {   int ret;
                                ret = slice_del(z); /* delete, line 109 */
                                if (ret < 0) return ret;
                            }
                            goto lab4;
                        lab5:
                            z->c = z->l - m;
                            {   int ret = r_R1(z);
                                if (ret == 0) { z->c = z->l - m; goto lab3; } /* call R1, line 109 */
                                if (ret < 0) return ret;
                            }
                            {   int ret;
                                ret = slice_from_s(z, 3, s_20); /* <-, line 109 */
                                if (ret < 0) return ret;
                            }
                        }
                    lab4:
                        break;
                    case 3:
                        {   int ret = r_R2(z);
                            if (ret == 0) { z->c = z->l - m; goto lab3; } /* call R2, line 111 */
                            if (ret < 0) return ret;
                        }
                        {   int ret;
                            ret = slice_del(z); /* delete, line 111 */
                            if (ret < 0) return ret;
                        }
                        break;
                    case 4:
                        {   int ret = r_RV(z);
                            if (ret == 0) { z->c = z->l - m; goto lab3; } /* call RV, line 113 */
                            if (ret < 0) return ret;
                        }
                        {   int ret;
                            ret = slice_from_s(z, 1, s_21); /* <-, line 113 */
                            if (ret < 0) return ret;
                        }
                        break;
                }
            lab3:
                ;
            }
            break;
        case 7:
            {   int ret = r_R2(z);
                if (ret == 0) return 0; /* call R2, line 120 */
                if (ret < 0) return ret;
            }
            {   int ret;
                ret = slice_del(z); /* delete, line 120 */
                if (ret < 0) return ret;
            }
            {   int m = z->l - z->c; (void) m; /* try, line 121 */
                z->ket = z->c; /* [, line 122 */
                among_var = find_among_b(z, a_2, 3); /* substring, line 122 */
                if (!(among_var)) { z->c = z->l - m; goto lab6; }
                z->bra = z->c; /* ], line 122 */
                switch(among_var) {
                    case 0: { z->c = z->l - m; goto lab6; }
                    case 1:
                        {   int m = z->l - z->c; (void) m; /* or, line 123 */
                            {   int ret = r_R2(z);
                                if (ret == 0) goto lab8; /* call R2, line 123 */
                                if (ret < 0) return ret;
                            }
                            {   int ret;
                                ret = slice_del(z); /* delete, line 123 */
                                if (ret < 0) return ret;
                            }
                            goto lab7;
                        lab8:
                            z->c = z->l - m;
                            {   int ret;
                                ret = slice_from_s(z, 3, s_22); /* <-, line 123 */
                                if (ret < 0) return ret;
                            }
                        }
                    lab7:
                        break;
                    case 2:
                        {   int m = z->l - z->c; (void) m; /* or, line 124 */
                            {   int ret = r_R2(z);
                                if (ret == 0) goto lab10; /* call R2, line 124 */
                                if (ret < 0) return ret;
                            }
                            {   int ret;
                                ret = slice_del(z); /* delete, line 124 */
                                if (ret < 0) return ret;
                            }
                            goto lab9;
                        lab10:
                            z->c = z->l - m;
                            {   int ret;
                                ret = slice_from_s(z, 3, s_23); /* <-, line 124 */
                                if (ret < 0) return ret;
                            }
                        }
                    lab9:
                        break;
                    case 3:
                        {   int ret = r_R2(z);
                            if (ret == 0) { z->c = z->l - m; goto lab6; } /* call R2, line 125 */
                            if (ret < 0) return ret;
                        }
                        {   int ret;
                            ret = slice_del(z); /* delete, line 125 */
                            if (ret < 0) return ret;
                        }
                        break;
                }
            lab6:
                ;
            }
            break;
        case 8:
            {   int ret = r_R2(z);
                if (ret == 0) return 0; /* call R2, line 132 */
                if (ret < 0) return ret;
            }
            {   int ret;
                ret = slice_del(z); /* delete, line 132 */
                if (ret < 0) return ret;
            }
            {   int m = z->l - z->c; (void) m; /* try, line 133 */
                z->ket = z->c; /* [, line 133 */
                if (!(eq_s_b(z, 2, s_24))) { z->c = z->l - m; goto lab11; }
                z->bra = z->c; /* ], line 133 */
                {   int ret = r_R2(z);
                    if (ret == 0) { z->c = z->l - m; goto lab11; } /* call R2, line 133 */
                    if (ret < 0) return ret;
                }
                {   int ret;
                    ret = slice_del(z); /* delete, line 133 */
                    if (ret < 0) return ret;
                }
                z->ket = z->c; /* [, line 133 */
                if (!(eq_s_b(z, 2, s_25))) { z->c = z->l - m; goto lab11; }
                z->bra = z->c; /* ], line 133 */
                {   int m = z->l - z->c; (void) m; /* or, line 133 */
                    {   int ret = r_R2(z);
                        if (ret == 0) goto lab13; /* call R2, line 133 */
                        if (ret < 0) return ret;
                    }
                    {   int ret;
                        ret = slice_del(z); /* delete, line 133 */
                        if (ret < 0) return ret;
                    }
                    goto lab12;
                lab13:
                    z->c = z->l - m;
                    {   int ret;
                        ret = slice_from_s(z, 3, s_26); /* <-, line 133 */
                        if (ret < 0) return ret;
                    }
                }
            lab12:
            lab11:
                ;
            }
            break;
        case 9:
            {   int ret;
                ret = slice_from_s(z, 3, s_27); /* <-, line 135 */
                if (ret < 0) return ret;
            }
            break;
        case 10:
            {   int ret = r_R1(z);
                if (ret == 0) return 0; /* call R1, line 136 */
                if (ret < 0) return ret;
            }
            {   int ret;
                ret = slice_from_s(z, 2, s_28); /* <-, line 136 */
                if (ret < 0) return ret;
            }
            break;
        case 11:
            {   int m = z->l - z->c; (void) m; /* or, line 138 */
                {   int ret = r_R2(z);
                    if (ret == 0) goto lab15; /* call R2, line 138 */
                    if (ret < 0) return ret;
                }
                {   int ret;
                    ret = slice_del(z); /* delete, line 138 */
                    if (ret < 0) return ret;
                }
                goto lab14;
            lab15:
                z->c = z->l - m;
                {   int ret = r_R1(z);
                    if (ret == 0) return 0; /* call R1, line 138 */
                    if (ret < 0) return ret;
                }
                {   int ret;
                    ret = slice_from_s(z, 3, s_29); /* <-, line 138 */
                    if (ret < 0) return ret;
                }
            }
        lab14:
            break;
        case 12:
            {   int ret = r_R1(z);
                if (ret == 0) return 0; /* call R1, line 141 */
                if (ret < 0) return ret;
            }
            if (!(out_grouping_b_U(z, g_v, 97, 251))) return 0;
            {   int ret;
                ret = slice_del(z); /* delete, line 141 */
                if (ret < 0) return ret;
            }
            break;
        case 13:
            {   int ret = r_RV(z);
                if (ret == 0) return 0; /* call RV, line 146 */
                if (ret < 0) return ret;
            }
            {   int ret;
                ret = slice_from_s(z, 3, s_30); /* <-, line 146 */
                if (ret < 0) return ret;
            }
            return 0; /* fail, line 146 */
            break;
        case 14:
            {   int ret = r_RV(z);
                if (ret == 0) return 0; /* call RV, line 147 */
                if (ret < 0) return ret;
            }
            {   int ret;
                ret = slice_from_s(z, 3, s_31); /* <-, line 147 */
                if (ret < 0) return ret;
            }
            return 0; /* fail, line 147 */
            break;
        case 15:
            {   int m_test = z->l - z->c; /* test, line 149 */
                if (!(in_grouping_b_U(z, g_v, 97, 251))) return 0;
                {   int ret = r_RV(z);
                    if (ret == 0) return 0; /* call RV, line 149 */
                    if (ret < 0) return ret;
                }
                z->c = z->l - m_test;
            }
            {   int ret;
                ret = slice_del(z); /* delete, line 149 */
                if (ret < 0) return ret;
            }
            return 0; /* fail, line 149 */
            break;
    }
    return 1;
}

static int r_i_verb_suffix(struct SN_env * z) {
    int among_var;
    {   int m3; /* setlimit, line 154 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 154 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 155 */
        among_var = find_among_b(z, a_4, 35); /* substring, line 155 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 155 */
        switch(among_var) {
            case 0: { z->lb = m3; return 0; }
            case 1:
                if (!(out_grouping_b_U(z, g_v, 97, 251))) { z->lb = m3; return 0; }
                {   int ret;
                    ret = slice_del(z); /* delete, line 161 */
                    if (ret < 0) return ret;
                }
                break;
        }
        z->lb = m3;
    }
    return 1;
}

static int r_verb_suffix(struct SN_env * z) {
    int among_var;
    {   int m3; /* setlimit, line 165 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 165 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 166 */
        among_var = find_among_b(z, a_5, 38); /* substring, line 166 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 166 */
        switch(among_var) {
            case 0: { z->lb = m3; return 0; }
            case 1:
                {   int ret = r_R2(z);
                    if (ret == 0) { z->lb = m3; return 0; } /* call R2, line 168 */
                    if (ret < 0) return ret;
                }
                {   int ret;
                    ret = slice_del(z); /* delete, line 168 */
                    if (ret < 0) return ret;
                }
                break;
            case 2:
                {   int ret;
                    ret = slice_del(z); /* delete, line 176 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                {   int ret;
                    ret = slice_del(z); /* delete, line 181 */
                    if (ret < 0) return ret;
                }
                {   int m = z->l - z->c; (void) m; /* try, line 182 */
                    z->ket = z->c; /* [, line 182 */
                    if (!(eq_s_b(z, 1, s_32))) { z->c = z->l - m; goto lab0; }
                    z->bra = z->c; /* ], line 182 */
                    {   int ret;
                        ret = slice_del(z); /* delete, line 182 */
                        if (ret < 0) return ret;
                    }
                lab0:
                    ;
                }
                break;
        }
        z->lb = m3;
    }
    return 1;
}

static int r_residual_suffix(struct SN_env * z) {
    int among_var;
    {   int m = z->l - z->c; (void) m; /* try, line 190 */
        z->ket = z->c; /* [, line 190 */
        if (!(eq_s_b(z, 1, s_33))) { z->c = z->l - m; goto lab0; }
        z->bra = z->c; /* ], line 190 */
        {   int m_test = z->l - z->c; /* test, line 190 */
            if (!(out_grouping_b_U(z, g_keep_with_s, 97, 232))) { z->c = z->l - m; goto lab0; }
            z->c = z->l - m_test;
        }
        {   int ret;
            ret = slice_del(z); /* delete, line 190 */
            if (ret < 0) return ret;
        }
    lab0:
        ;
    }
    {   int m3; /* setlimit, line 191 */
        int m = z->l - z->c; (void) m;
        if (z->c < z->I[0]) return 0;
        z->c = z->I[0]; /* tomark, line 191 */
        m3 = z->lb; z->lb = z->c;
        z->c = z->l - m;
        z->ket = z->c; /* [, line 192 */
        among_var = find_among_b(z, a_6, 7); /* substring, line 192 */
        if (!(among_var)) { z->lb = m3; return 0; }
        z->bra = z->c; /* ], line 192 */
        switch(among_var) {
            case 0: { z->lb = m3; return 0; }
            case 1:
                {   int ret = r_R2(z);
                    if (ret == 0) { z->lb = m3; return 0; } /* call R2, line 193 */
                    if (ret < 0) return ret;
                }
                {   int m = z->l - z->c; (void) m; /* or, line 193 */
                    if (!(eq_s_b(z, 1, s_34))) goto lab2;
                    goto lab1;
                lab2:
                    z->c = z->l - m;
                    if (!(eq_s_b(z, 1, s_35))) { z->lb = m3; return 0; }
                }
            lab1:
                {   int ret;
                    ret = slice_del(z); /* delete, line 193 */
                    if (ret < 0) return ret;
                }
                break;
            case 2:
                {   int ret;
                    ret = slice_from_s(z, 1, s_36); /* <-, line 195 */
                    if (ret < 0) return ret;
                }
                break;
            case 3:
                {   int ret;
                    ret = slice_del(z); /* delete, line 196 */
                    if (ret < 0) return ret;
                }
                break;
            case 4:
                if (!(eq_s_b(z, 2, s_37))) { z->lb = m3; return 0; }
                {   int ret;
                    ret = slice_del(z); /* delete, line 197 */
                    if (ret < 0) return ret;
                }
                break;
        }
        z->lb = m3;
    }
    return 1;
}

static int r_un_double(struct SN_env * z) {
    {   int m_test = z->l - z->c; /* test, line 203 */
        if (!(find_among_b(z, a_7, 5))) return 0; /* among, line 203 */
        z->c = z->l - m_test;
    }
    z->ket = z->c; /* [, line 203 */
    {   int c = skip_utf8(z->p, z->c, z->lb, 0, -1);
        if (c < 0) return 0;
        z->c = c; /* next, line 203 */
    }
    z->bra = z->c; /* ], line 203 */
    {   int ret;
        ret = slice_del(z); /* delete, line 203 */
        if (ret < 0) return ret;
    }
    return 1;
}

static int r_un_accent(struct SN_env * z) {
    {   int i = 1;
        while(1) { /* atleast, line 207 */
            if (!(out_grouping_b_U(z, g_v, 97, 251))) goto lab0;
            i--;
            continue;
        lab0:
            break;
        }
        if (i > 0) return 0;
    }
    z->ket = z->c; /* [, line 208 */
    {   int m = z->l - z->c; (void) m; /* or, line 208 */
        if (!(eq_s_b(z, 2, s_38))) goto lab2;
        goto lab1;
    lab2:
        z->c = z->l - m;
        if (!(eq_s_b(z, 2, s_39))) return 0;
    }
lab1:
    z->bra = z->c; /* ], line 208 */
    {   int ret;
        ret = slice_from_s(z, 1, s_40); /* <-, line 208 */
        if (ret < 0) return ret;
    }
    return 1;
}

extern int french_UTF_8_stem(struct SN_env * z) {
    {   int c = z->c; /* do, line 214 */
        {   int ret = r_prelude(z);
            if (ret == 0) goto lab0; /* call prelude, line 214 */
            if (ret < 0) return ret;
        }
    lab0:
        z->c = c;
    }
    {   int c = z->c; /* do, line 215 */
        {   int ret = r_mark_regions(z);
            if (ret == 0) goto lab1; /* call mark_regions, line 215 */
            if (ret < 0) return ret;
        }
    lab1:
        z->c = c;
    }
    z->lb = z->c; z->c = z->l; /* backwards, line 216 */

    {   int m = z->l - z->c; (void) m; /* do, line 218 */
        {   int m = z->l - z->c; (void) m; /* or, line 228 */
            {   int m = z->l - z->c; (void) m; /* and, line 224 */
                {   int m = z->l - z->c; (void) m; /* or, line 220 */
                    {   int ret = r_standard_suffix(z);
                        if (ret == 0) goto lab6; /* call standard_suffix, line 220 */
                        if (ret < 0) return ret;
                    }
                    goto lab5;
                lab6:
                    z->c = z->l - m;
                    {   int ret = r_i_verb_suffix(z);
                        if (ret == 0) goto lab7; /* call i_verb_suffix, line 221 */
                        if (ret < 0) return ret;
                    }
                    goto lab5;
                lab7:
                    z->c = z->l - m;
                    {   int ret = r_verb_suffix(z);
                        if (ret == 0) goto lab4; /* call verb_suffix, line 222 */
                        if (ret < 0) return ret;
                    }
                }
            lab5:
                z->c = z->l - m;
                {   int m = z->l - z->c; (void) m; /* try, line 225 */
                    z->ket = z->c; /* [, line 225 */
                    {   int m = z->l - z->c; (void) m; /* or, line 225 */
                        if (!(eq_s_b(z, 1, s_41))) goto lab10;
                        z->bra = z->c; /* ], line 225 */
                        {   int ret;
                            ret = slice_from_s(z, 1, s_42); /* <-, line 225 */
                            if (ret < 0) return ret;
                        }
                        goto lab9;
                    lab10:
                        z->c = z->l - m;
                        if (!(eq_s_b(z, 2, s_43))) { z->c = z->l - m; goto lab8; }
                        z->bra = z->c; /* ], line 226 */
                        {   int ret;
                            ret = slice_from_s(z, 1, s_44); /* <-, line 226 */
                            if (ret < 0) return ret;
                        }
                    }
                lab9:
                lab8:
                    ;
                }
            }
            goto lab3;
        lab4:
            z->c = z->l - m;
            {   int ret = r_residual_suffix(z);
                if (ret == 0) goto lab2; /* call residual_suffix, line 229 */
                if (ret < 0) return ret;
            }
        }
    lab3:
    lab2:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* do, line 234 */
        {   int ret = r_un_double(z);
            if (ret == 0) goto lab11; /* call un_double, line 234 */
            if (ret < 0) return ret;
        }
    lab11:
        z->c = z->l - m;
    }
    {   int m = z->l - z->c; (void) m; /* do, line 235 */
        {   int ret = r_un_accent(z);
            if (ret == 0) goto lab12; /* call un_accent, line 235 */
            if (ret < 0) return ret;
        }
    lab12:
        z->c = z->l - m;
    }
    z->c = z->lb;
    {   int c = z->c; /* do, line 237 */
        {   int ret = r_postlude(z);
            if (ret == 0) goto lab13; /* call postlude, line 237 */
            if (ret < 0) return ret;
        }
    lab13:
        z->c = c;
    }
    return 1;
}

extern struct SN_env * french_UTF_8_create_env(void) { return SN_create_env(0, 3, 0); }

extern void french_UTF_8_close_env(struct SN_env * z) { SN_close_env(z); }

