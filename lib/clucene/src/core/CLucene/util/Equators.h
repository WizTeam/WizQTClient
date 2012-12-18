/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_Equators_
#define _lucene_util_Equators_

#include <map>
#include <list>
#include <set>
#include <vector>
#include <stdlib.h>
//#include "CLucene/LuceneThreads.h"

CL_NS_DEF(util)

////////////////////////////////////////////////////////////////////////////////
// Equators
////////////////////////////////////////////////////////////////////////////////
/** @internal */
class CLUCENE_INLINE_EXPORT Equals{
public:
	class CLUCENE_INLINE_EXPORT Int32:public CL_NS_STD(binary_function)<const int32_t*,const int32_t*,bool>
	{
	public:
		bool operator()( const int32_t val1, const int32_t val2 ) const;
	};
	
	class CLUCENE_INLINE_EXPORT Char:public CL_NS_STD(binary_function)<const char*,const char*,bool>
	{
	public:
		bool operator()( const char* val1, const char* val2 ) const;
	};
#ifdef _UCS2
	class CLUCENE_INLINE_EXPORT WChar: public CL_NS_STD(binary_function)<const wchar_t*,const wchar_t*,bool>
	{
	public:
		bool operator()( const wchar_t* val1, const wchar_t* val2 ) const;
	};
	class CLUCENE_INLINE_EXPORT TChar: public WChar{
	};
#else
	class CLUCENE_INLINE_EXPORT TChar: public Char{
	};
#endif


    template<typename _cl>
	class CLUCENE_INLINE_EXPORT Void:public CL_NS_STD(binary_function)<const void*,const void*,bool>
	{
	public:
		bool operator()( _cl* val1, _cl* val2 ) const{
			return val1==val2;
		}
	};
};


////////////////////////////////////////////////////////////////////////////////
// Comparors
////////////////////////////////////////////////////////////////////////////////
class CLUCENE_EXPORT NamedObject{
public:
  virtual ~NamedObject();
	virtual const char* getObjectName() const = 0;
  virtual bool instanceOf(const char* otherobject) const;
};
class CLUCENE_EXPORT Comparable:public NamedObject{
public:
   virtual ~Comparable(){
   }
   
	virtual int32_t compareTo(NamedObject* o) = 0;
};

/** @internal */
class CLUCENE_INLINE_EXPORT Compare{
public:
	class CLUCENE_EXPORT _base
	{	// traits class for hash containers
	public:
		enum
		{	// parameters for hash table
			bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8
		};	// min_buckets = 2 ^^ N, 0 < N

		_base()
		{
		}
	};

	class CLUCENE_INLINE_EXPORT Int32:public _base, public Comparable{
		int32_t value;
	public:
		int32_t getValue() const;
		Int32(int32_t val);
		Int32();
		int32_t compareTo(NamedObject* o);
		bool operator()( int32_t t1, int32_t t2 ) const;
		size_t operator()( int32_t t ) const;
		static const char* getClassName();
		const char* getObjectName() const;
	};

	
	class CLUCENE_INLINE_EXPORT Float:public Comparable{
		float_t value;
	public:
		float_t getValue() const;
		Float(float_t val);
		int32_t compareTo(NamedObject* o);
		static const char* getClassName();
		const char* getObjectName() const;
	};


	class CLUCENE_EXPORT Char: public _base, public Comparable //<char*>
	{
	    const char* s;
	public:
    	const char* getValue() const;
    	Char();
    	Char(const char* str);
    	int32_t compareTo(NamedObject* o);

		bool operator()( const char* val1, const char* val2 ) const;
		size_t operator()( const char* val1) const;
		static const char* getClassName();
		const char* getObjectName() const;
	};

#ifdef _UCS2
	class CLUCENE_EXPORT WChar: public _base, public Comparable //<wchar_t*>
	{
	    const wchar_t* s;
	public:
    	const wchar_t* getValue() const;
    	WChar();
    	WChar(const wchar_t* str);
    	int32_t compareTo(NamedObject* o);

		bool operator()( const wchar_t* val1, const wchar_t* val2 ) const;
		size_t operator()( const wchar_t* val1) const;
		static const char* getClassName();
		const char* getObjectName() const;
	};
	typedef WChar TChar;
#else
	typedef Char TChar;
#endif

	template<typename _cl>
	class CLUCENE_INLINE_EXPORT Void:public _base //<const void*,const void*,bool>
	{
	public:
		int32_t compareTo(_cl* o){
			if ( this == o )
				return o;
			else
				return this > o ? 1 : -1;
		}
		bool operator()( _cl* t1, _cl* t2 ) const{
			return t1 > t2 ? true : false;
		}
		size_t operator()( _cl* t ) const{
			return (size_t)t;
		}
	};
};




int32_t compare(Comparable* o1, Comparable* o2);

////////////////////////////////////////////////////////////////////////////////
// allocators
////////////////////////////////////////////////////////////////////////////////
/** @internal */
class CLUCENE_INLINE_EXPORT AbstractDeletor{
public:
	virtual void Delete(void*) = 0;
    virtual ~AbstractDeletor();
};
class CLUCENE_INLINE_EXPORT Deletor{
public:
    class CLUCENE_INLINE_EXPORT tcArray: public AbstractDeletor{
    public:
    	void Delete(void* _arr){
    		doDelete((TCHAR*)_arr);
    	}
    	static void doDelete(TCHAR* arr){
    		_CLDELETE_CARRAY(arr);
    	}
    };

	template<typename _kt>
	class CLUCENE_INLINE_EXPORT vArray: public AbstractDeletor{
	public:
		void Delete(void* arr){
			doDelete((_kt*)arr);
		}
		static void doDelete(_kt* arr){
			_CLDELETE_LARRAY(arr);
		}
	};
	class CLUCENE_INLINE_EXPORT acArray: public AbstractDeletor{
	public:
		void Delete(void* arr){
			doDelete((char*)arr);
		}
		static void doDelete(char* arr){
			_CLDELETE_CaARRAY(arr);
		}
	};
	
	template<typename _kt>
	class CLUCENE_INLINE_EXPORT Object: public AbstractDeletor{
	public:
		void Delete(void* obj){
			doDelete((_kt*)obj);
		}
		static void doDelete(_kt* obj){
			_CLDELETE(obj);
		}
	};
	template<typename _kt>
	class CLUCENE_INLINE_EXPORT Void: public AbstractDeletor{
	public:
		void Delete(void* obj){
			doDelete((_kt*)obj);
		}
		static void doDelete(_kt* obj){
			_CLVDELETE(obj);
		}
	};
	class CLUCENE_INLINE_EXPORT Dummy: public AbstractDeletor{
	public:
		void Delete(void*){}
		static void doDelete(const void*){
			//todo: remove all occurances where it hits this point
			//CND_WARNING(false,"Deletor::Dummy::doDelete run, set deleteKey or deleteValue to false");
		}
	};
	class CLUCENE_INLINE_EXPORT DummyInt32: public AbstractDeletor{
	public:
		void Delete(void*){}
		static void doDelete(const int32_t){
		}
	};
	class CLUCENE_INLINE_EXPORT DummyFloat: public AbstractDeletor{
	public:
		void Delete(void*){}
		static void doDelete(const float_t){
		}
	};
	template <typename _type>
	class CLUCENE_INLINE_EXPORT ConstNullVal: public AbstractDeletor{
	public:
		void Delete(void*){}
		static void doDelete(const _type){
			//todo: remove all occurances where it hits this point
			//CND_WARNING(false,"Deletor::Dummy::doDelete run, set deleteKey or deleteValue to false");
		}
	};
	
	template <typename _type>
	class CLUCENE_INLINE_EXPORT NullVal: public AbstractDeletor{
	public:
		void Delete(void*){}
		static void doDelete(_type){
			//todo: remove all occurances where it hits this point
			//CND_WARNING(false,"Deletor::Dummy::doDelete run, set deleteKey or deleteValue to false");
		}
	};
};
////////////////////////////////////////////////////////////////////////////////

CL_NS_END
#endif
