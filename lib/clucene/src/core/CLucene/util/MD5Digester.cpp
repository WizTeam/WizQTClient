/////////////////////////////////////////////////////////////////////////
// MD5.cpp
// Implementation file for MD5 class
//
// This C++ Class implementation of the original RSA Data Security, Inc.
// MD5 Message-Digest Algorithm is copyright (c) 2002, Gary McNickle.
// All rights reserved.  This software is a derivative of the "RSA Data
//  Security, Inc. MD5 Message-Digest Algorithm"
//
// You may use this software free of any charge, but without any
// warranty or implied warranty, provided that you follow the terms
// of the original RSA copyright, listed below.
//
// Original RSA Data Security, Inc. Copyright notice
/////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
// rights reserved.
//
// License to copy and use this software is granted provided that it
// is identified as the "RSA Data Security, Inc. MD5 Message-Digest
// Algorithm" in all material mentioning or referencing this software
// or this function.
// License is also granted to make and use derivative works provided
// that such works are identified as "derived from the RSA Data
// Security, Inc. MD5 Message-Digest Algorithm" in all material
// mentioning or referencing the derived work.
// RSA Data Security, Inc. makes no representations concerning either
// the merchantability of this software or the suitability of this
// software for any particular purpose. It is provided "as is"
// without express or implied warranty of any kind.
// These notices must be retained in any copies of any part of this
// documentation and/or software.
/////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include "CLucene/_ApiHeader.h"
#include "_MD5Digester.h"
CL_NS_DEF(util)

static unsigned char PADDING[64] =
{
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

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


// PrintMD5: Converts a completed md5 digest into a char* string.
char* PrintMD5(uint8_t md5Digest[16])
{
	char chBuffer[256];
	char chEach[10];
	int nCount;

	memset(chBuffer,0,256);
	memset(chEach, 0, 10);

	for (nCount = 0; nCount < 16; nCount++)
	{
		cl_sprintf(chEach, 10, "%02x", md5Digest[nCount]);
		strncat(chBuffer, chEach, sizeof(chEach));
	}

	return STRDUP_AtoA(chBuffer);
}

// MD5String: Performs the MD5 algorithm on a char* string, returning
// the results as a char*.
char* MD5String(char* szString)
{
	int nLen = strlen(szString);
	md5 alg;

	alg.Update((unsigned char*)szString, (unsigned int)nLen);
	alg.Finalize();

	return PrintMD5(alg.Digest());

}

// MD5File: Performs the MD5 algorithm on a file (binar or text),
// returning the results as a char*.  Returns NULL if it fails.
char* MD5File(char* szFilename)
{
	FILE* file;
	md5 alg;
	int nLen;
	unsigned char chBuffer[1024];

	try
	{
		memset(chBuffer, 0, 1024);

		if ((file = fopen (szFilename, "rb")) != NULL)
		{
			while ((nLen = fread (chBuffer, 1, 1024, file)))
				alg.Update(chBuffer, nLen);

			alg.Finalize();

			fclose (file);

			return PrintMD5(alg.Digest());
		}
	}
	catch(...) //todo: only catch IO Err???
	{

	}

	return NULL; // failed
}


// md5::Init
// Initializes a new context.
void md5::Init()
{
	memset(m_Count, 0, 2 * sizeof(uint32_t));

	m_State[0] = 0x67452301;
	m_State[1] = 0xefcdab89;
	m_State[2] = 0x98badcfe;
	m_State[3] = 0x10325476;
}

// md5::Update
// MD5 block update operation. Continues an MD5 message-digest
// operation, processing another message block, and updating the
// context.
void md5::Update(uint8_t* chInput, uint32_t nInputLen)
{
	uint32_t i, index, partLen;

	// Compute number of bytes mod 64
	index = (unsigned int)((m_Count[0] >> 3) & 0x3F);

	// Update number of bits
	if ((m_Count[0] += (nInputLen << 3)) < (nInputLen << 3))
		m_Count[1]++;

	m_Count[1] += (nInputLen >> 29);

	partLen = 64 - index;

	// Transform as many times as possible.
	if (nInputLen >= partLen)
	{
		memcpy( &m_Buffer[index], chInput, partLen );
		Transform(m_Buffer);

		for (i = partLen; i + 63 < nInputLen; i += 64)
			Transform(&chInput[i]);

		index = 0;
	}
	else
		i = 0;

  // Buffer remaining input
  memcpy( &m_Buffer[index], &chInput[i], nInputLen-i );
}

// md5::Finalize
// MD5 finalization. Ends an MD5 message-digest operation, writing
// the message digest and zeroizing the context.
void md5::Finalize()
{
	uint8_t bits[8];
	uint32_t index, padLen;

	// Save number of bits
	Encode (bits, m_Count, 8);

	// Pad out to 56 mod 64
	index = (unsigned int)((m_Count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	Update(PADDING, padLen);

	// Append length (before padding)
	Update (bits, 8);

	// Store state in digest
	Encode (m_Digest, m_State, 16);

	memset(m_Count, 0, 2 * sizeof(uint32_t));
	memset(m_State, 0, 4 * sizeof(uint32_t));
	memset(m_Buffer,0, 64 * sizeof(uint8_t));
}

// md5::Transform
// MD5 basic transformation. Transforms state based on block.
void md5::Transform (uint8_t* block)
{
  uint32_t a = m_State[0], b = m_State[1], c = m_State[2], d = m_State[3], x[16];

  Decode (x, block, 64);

  // Round 1
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478);
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756);
  FF (c, d, a, b, x[ 2], S13, 0x242070db);
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee);
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf);
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a);
  FF (c, d, a, b, x[ 6], S13, 0xa8304613);
  FF (b, c, d, a, x[ 7], S14, 0xfd469501);
  FF (a, b, c, d, x[ 8], S11, 0x698098d8);
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af);
  FF (c, d, a, b, x[10], S13, 0xffff5bb1);
  FF (b, c, d, a, x[11], S14, 0x895cd7be);
  FF (a, b, c, d, x[12], S11, 0x6b901122);
  FF (d, a, b, c, x[13], S12, 0xfd987193);
  FF (c, d, a, b, x[14], S13, 0xa679438e);
  FF (b, c, d, a, x[15], S14, 0x49b40821);

 // Round 2
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562);
  GG (d, a, b, c, x[ 6], S22, 0xc040b340);
  GG (c, d, a, b, x[11], S23, 0x265e5a51);
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa);
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d);
  GG (d, a, b, c, x[10], S22,  0x2441453);
  GG (c, d, a, b, x[15], S23, 0xd8a1e681);
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8);
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6);
  GG (d, a, b, c, x[14], S22, 0xc33707d6);
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87);
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed);
  GG (a, b, c, d, x[13], S21, 0xa9e3e905);
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8);
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9);
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a);

  // Round 3
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942);
  HH (d, a, b, c, x[ 8], S32, 0x8771f681);
  HH (c, d, a, b, x[11], S33, 0x6d9d6122);
  HH (b, c, d, a, x[14], S34, 0xfde5380c);
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44);
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9);
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60);
  HH (b, c, d, a, x[10], S34, 0xbebfbc70);
  HH (a, b, c, d, x[13], S31, 0x289b7ec6);
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa);
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085);
  HH (b, c, d, a, x[ 6], S34,  0x4881d05);
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039);
  HH (d, a, b, c, x[12], S32, 0xe6db99e5);
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8);
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665);

  // Round 4
  II (a, b, c, d, x[ 0], S41, 0xf4292244);
  II (d, a, b, c, x[ 7], S42, 0x432aff97);
  II (c, d, a, b, x[14], S43, 0xab9423a7);
  II (b, c, d, a, x[ 5], S44, 0xfc93a039);
  II (a, b, c, d, x[12], S41, 0x655b59c3);
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92);
  II (c, d, a, b, x[10], S43, 0xffeff47d);
  II (b, c, d, a, x[ 1], S44, 0x85845dd1);
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f);
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0);
  II (c, d, a, b, x[ 6], S43, 0xa3014314);
  II (b, c, d, a, x[13], S44, 0x4e0811a1);
  II (a, b, c, d, x[ 4], S41, 0xf7537e82);
  II (d, a, b, c, x[11], S42, 0xbd3af235);
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb);
  II (b, c, d, a, x[ 9], S44, 0xeb86d391);

  m_State[0] += a;
  m_State[1] += b;
  m_State[2] += c;
  m_State[3] += d;

  memset(x, 0, sizeof(x));
}

// md5::Encode
// Encodes input (uint32_t) into output (uint8_t). Assumes nLength is
// a multiple of 4.
void md5::Encode(uint8_t* dest, uint32_t* src, uint32_t nLength)
{
	uint32_t i, j;

	CND_PRECONDITION(nLength % 4 == 0,"nLength % 4 != 0")

	for (i = 0, j = 0; j < nLength; i++, j += 4)
	{
		dest[j] = (uint8_t)(src[i] & 0xff);
		dest[j+1] = (uint8_t)((src[i] >> 8) & 0xff);
		dest[j+2] = (uint8_t)((src[i] >> 16) & 0xff);
		dest[j+3] = (uint8_t)((src[i] >> 24) & 0xff);
	}
}

// md5::Decode
// Decodes input (uint8_t) into output (uint32_t). Assumes nLength is
// a multiple of 4.
void md5::Decode(uint32_t* dest, uint8_t* src, uint32_t nLength)
{
	uint32_t i, j;

	CND_PRECONDITION(nLength % 4 == 0, "nLength % 4 != 0");

	for (i = 0, j = 0; j < nLength; i++, j += 4)
	{
		dest[i] = ((uint32_t)src[j]) | (((uint32_t)src[j+1])<<8) |
			      (((uint32_t)src[j+2])<<16) | (((uint32_t)src[j+3])<<24);
	}
}

CL_NS_END
