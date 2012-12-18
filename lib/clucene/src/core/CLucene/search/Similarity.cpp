/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Similarity.h"

#include "CLucene/index/Term.h"
#include "SearchHeader.h"
#include "Searchable.h"

CL_NS_USE(index)
CL_NS_DEF(search)

#ifdef _CL_HAVE_NO_FLOAT_BYTE
	#if defined(_LUCENE_PRAGMA_WARNINGS)
	 #pragma message ("==================Using fallback float<->byte encodings!!!==================")
	#else
	 #warning "==================Using fallback float<->byte encodings!!!=================="
	#endif

    //if the autoconf figured out that we can't do the conversions properly, then
	//we fall back on the old, inaccurate way of doing things.
	float_t NORM_TABLE[]  = {
	0.0,5.820766E-10,6.9849193E-10,8.1490725E-10,9.313226E-10,1.1641532E-9,1.3969839E-9,
	1.6298145E-9,1.8626451E-9,2.3283064E-9,2.7939677E-9,3.259629E-9,3.7252903E-9,
	4.656613E-9,5.5879354E-9,6.519258E-9,7.4505806E-9,9.313226E-9,1.1175871E-8,1.3038516E-8,
	1.4901161E-8,1.8626451E-8,2.2351742E-8,2.6077032E-8,2.9802322E-8,3.7252903E-8,4.4703484E-8,
	5.2154064E-8,5.9604645E-8,7.4505806E-8,8.940697E-8,1.0430813E-7,1.1920929E-7,1.4901161E-7,
	1.7881393E-7,2.0861626E-7,2.3841858E-7,2.9802322E-7,3.5762787E-7,4.172325E-7,4.7683716E-7,
	5.9604645E-7,7.1525574E-7,8.34465E-7,9.536743E-7,1.1920929E-6,1.4305115E-6,1.66893E-6,
	1.9073486E-6,2.3841858E-6,2.861023E-6,3.33786E-6,3.8146973E-6,4.7683716E-6,5.722046E-6,
	6.67572E-6,7.6293945E-6,9.536743E-6,1.1444092E-5,1.335144E-5,1.5258789E-5,1.9073486E-5,
	2.2888184E-5,2.670288E-5,3.0517578E-5,3.8146973E-5,4.5776367E-5,5.340576E-5,6.1035156E-5,
	7.6293945E-5,9.1552734E-5,1.0681152E-4,1.2207031E-4,1.5258789E-4,1.8310547E-4,2.1362305E-4,
	2.4414062E-4,3.0517578E-4,3.6621094E-4,4.272461E-4,4.8828125E-4,6.1035156E-4,7.324219E-4,
	8.544922E-4,9.765625E-4,0.0012207031,0.0014648438,0.0017089844,0.001953125,0.0024414062,
	0.0029296875,0.0034179688,0.00390625,0.0048828125,0.005859375,0.0068359375,
	0.0078125,0.009765625,0.01171875,0.013671875,0.015625,0.01953125,0.0234375,
	0.02734375,0.03125,0.0390625,0.046875,0.0546875,0.0625,0.078125,0.09375,0.109375,
	0.125,0.15625,0.1875,0.21875,0.25,0.3125,0.375,0.4375,0.5,0.625,0.75,
	0.875,1.0,1.25,1.5,1.75,2,2.5,3,3.5,4.0,5.0,6.0,7.0,8.0,10.0,12.0,14.0,16.0,20.0,24.0,28.0,32.0,40.0,48.0,56.0,
	64.0,80.0,96.0,112.0,128.0,160.0,192.0,224.0,256.0,320.0,384.0,448.0,512.0,640.0,768.0,896.0,1024.0,1280.0,1536.0,1792.0,
	2048.0,2560.0,3072.0,3584.0,4096.0,5120.0,6144.0,7168.0,8192.0,10240.0,12288.0,14336.0,16384.0,20480.0,24576.0,
	28672.0,32768.0,40960.0,49152.0,57344.0,65536.0,81920.0,98304.0,114688.0,131072.0,163840.0,196608.0,
	229376.0,262144.0,327680.0,393216.0,458752.0,524288.0,655360.0,786432.0,917504.0,1048576.0,1310720.0,
	1572864.0,1835008.0,2097152.0,2621440.0,3145728.0,3670016.0,4194304.0,5242880.0,6291456.0,7340032.0,
	8388608.0,10485760.0,12582912.0,14680064.0,16777216.0,20971520.0,25165824.0,29360128.0,33554432.0,
	41943040.0,50331648.0,58720256.0,67108864.0,83886080.0,100663296.0,117440512.0,134217728.0,
	167772160.0,201326592.0,234881024.0,268435456.0,335544320.0,402653184.0,469762048.0,536870912.0,
	671088640.0,805306368.0,939524096.0,1073741824.0,1342177280.0,1610612736.0,1879048192.0,
	2147483648.0,2684354560.0,3221225472.0,3758096384.0,4294967296.0,5368709120.0,6442450944.0,7516192768.0
	};

	float_t Similarity::byteToFloat(uint8_t b) {
		return NORM_TABLE[b];
    }

   uint8_t Similarity::floatToByte(float_t f) {
		return Similarity::encodeNorm(f);
   }

#else

	/** Cache of decoded bytes. */
	float_t NORM_TABLE[256];
	bool NORM_TABLE_initd=false;

	//float to bits conversion utilities...
	union clvalue {
		int32_t     i;
		float		f; //must use a float type, else types dont match up
	};

	int32_t floatToIntBits(float_t value)
	{
		clvalue u;
		int32_t e, f;
		u.f = (float)value;
		e = u.i & 0x7f800000;
		f = u.i & 0x007fffff;

		if (e == 0x7f800000 && f != 0)
		u.i = 0x7fc00000;

		return u.i;
	}

	float_t intBitsToFloat(int32_t bits)
	{
		clvalue u;
		u.i = bits;
		return u.f;
	}


   float_t Similarity::byteToFloat(uint8_t b) {
      if (b == 0)                                   // zero is a special case
         return 0.0f;
      int32_t mantissa = b & 7;
      int32_t exponent = (b >> 3) & 31;
      int32_t bits = ((exponent+(63-15)) << 24) | (mantissa << 21);
      return intBitsToFloat(bits);
   }

   uint8_t Similarity::floatToByte(float_t f) {
      if (f < 0.0f)                                 // round negatives up to zero
         f = 0.0f;

      if (f == 0.0f)                                // zero is a special case
         return 0;

      int32_t bits = floatToIntBits(f);           // parse float_t into parts
      int32_t mantissa = (bits & 0xffffff) >> 21;
      int32_t exponent = (((bits >> 24) & 0x7f) - 63) + 15;

      if (exponent > 31) {                          // overflow: use max value
         exponent = 31;
         mantissa = 7;
      }

      if (exponent < 0) {                           // underflow: use min value
         exponent = 0;
         mantissa = 1;
      }

      return (uint8_t)((exponent << 3) | mantissa);    // pack into a uint8_t
   }
#endif

   /** The Similarity implementation used by default. */
   Similarity* Similarity_defaultImpl=NULL;

   void Similarity::setDefault(Similarity* similarity) {
    _CLDELETE(Similarity_defaultImpl);
    Similarity_defaultImpl = similarity;
   }

   Similarity* Similarity::getDefault() {
	   if ( Similarity_defaultImpl == NULL ){
			Similarity_defaultImpl = _CLNEW DefaultSimilarity();
	   }
      return Similarity_defaultImpl;
   }
   void Similarity::_shutdown(){
    _CLDELETE(Similarity_defaultImpl);
   }

   float_t Similarity::decodeNorm(uint8_t b) {
#ifndef _CL_HAVE_NO_FLOAT_BYTE
	   if ( !NORM_TABLE_initd ){
		for (int i = 0; i < 256; i++)
			NORM_TABLE[i] = byteToFloat(i);
		NORM_TABLE_initd=true;
	   }
#endif
      return NORM_TABLE[b];
   }

   uint8_t Similarity::encodeNorm(float_t f) {
#ifdef _CL_HAVE_NO_FLOAT_BYTE
	   int32_t i=0;
	   if ( f <= 0 )
		   return 0;

	   while ( i<256 && f > NORM_TABLE[i] ){
			i++;
	   }
	   if ( i == 0 )
		   return 0;
	   else if ( i == 255 && f>NORM_TABLE[255] )
		   return 255;
	   else
		   return i;
#else
	   return floatToByte(f);
#endif
   }


   float_t Similarity::idf(Term* term, Searcher* searcher) {
      return idf(searcher->docFreq(term), searcher->maxDoc());
   }


   float_t Similarity::idf(CL_NS(util)::CLVector<Term*>* terms, Searcher* searcher) {
      float_t _idf = 0.0f;
      for (CL_NS(util)::CLVector<Term*>::iterator i = terms->begin(); i != terms->end(); i++ ) {
         _idf += idf((Term*)*i, searcher);
      }
      return _idf;
   }

   Similarity::~Similarity(){
  }




  DefaultSimilarity::DefaultSimilarity(){
	}
	DefaultSimilarity::~DefaultSimilarity(){
	}

  float_t DefaultSimilarity::lengthNorm(const TCHAR* /*fieldName*/, int32_t numTerms) {
    if ( numTerms == 0 ) //prevent div by zero
        return 0;
    return (1.0 / sqrt((float_t)numTerms));
  }

  float_t DefaultSimilarity::queryNorm(float_t sumOfSquaredWeights) {
    if ( sumOfSquaredWeights == 0 ) //prevent div by zero
        return 0.0f;
	  return (float_t)(1.0 / sqrt(sumOfSquaredWeights));
  }

  float_t DefaultSimilarity::tf(float_t freq) {
    return sqrt(freq);
  }

  float_t DefaultSimilarity::sloppyFreq(int32_t distance) {
    return 1.0f / (distance + 1);
  }

  float_t DefaultSimilarity::idf(int32_t docFreq, int32_t numDocs) {
    return (float_t)(log(numDocs/(float_t)(docFreq+1)) + 1.0);
  }

  float_t DefaultSimilarity::coord(int32_t overlap, int32_t maxOverlap) {
  	if ( maxOverlap == 0 )
  		return 0.0f;
    return overlap / (float_t)maxOverlap;
  }
CL_NS_END
