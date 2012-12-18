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

#ifndef _lucene_util_MD5Digester_H
#define _lucene_util_MD5Digester_H


CL_NS_DEF(util)

typedef unsigned short int uint2;

char* PrintMD5(uint8_t md5Digest[16]);
char* MD5String(char* szString);
char* MD5File(char* szFilename);

class md5
{
// Methods
public:
	md5() { Init(); }
	void	Init();
	void	Update(uint8_t* chInput, uint32_t nInputLen);
	void	Finalize();
	uint8_t*	Digest() { return m_Digest; }

private:

	void	Transform(uint8_t* block);
	void	Encode(uint8_t* dest, uint32_t* src, uint32_t nLength);
	void	Decode(uint32_t* dest, uint8_t* src, uint32_t nLength);


	inline	uint32_t	rotate_left(uint32_t x, uint32_t n)
	                 { return ((x << n) | (x >> (32-n))); }

	inline	uint32_t	F(uint32_t x, uint32_t y, uint32_t z)
	                 { return ((x & y) | (~x & z)); }

	inline  uint32_t	G(uint32_t x, uint32_t y, uint32_t z)
	                 { return ((x & z) | (y & ~z)); }

	inline  uint32_t	H(uint32_t x, uint32_t y, uint32_t z)
	                 { return (x ^ y ^ z); }

	inline  uint32_t	I(uint32_t x, uint32_t y, uint32_t z)
	                 { return (y ^ (x | ~z)); }

	inline	void	FF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
	                 { a += F(b, c, d) + x + ac; a = rotate_left(a, s); a += b; }

	inline	void	GG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
                     { a += G(b, c, d) + x + ac; a = rotate_left(a, s); a += b; }

	inline	void	HH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
                     { a += H(b, c, d) + x + ac; a = rotate_left(a, s); a += b; }

	inline	void	II(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
                     { a += I(b, c, d) + x + ac; a = rotate_left(a, s); a += b; }

// Data
private:
	uint32_t		m_State[4];
	uint32_t		m_Count[2];
	uint8_t		m_Buffer[64];
	uint8_t		m_Digest[16];
	uint8_t		m_Finalized;

};

CL_NS_END
#endif
