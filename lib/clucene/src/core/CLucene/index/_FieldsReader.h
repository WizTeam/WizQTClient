/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_FieldsReader_
#define _lucene_index_FieldsReader_

#include "CLucene/util/_ThreadLocal.h"
CL_CLASS_DEF(store,Directory)
CL_CLASS_DEF(document,Document)
#include "CLucene/document/Field.h"
CL_CLASS_DEF(document,FieldSelector)
CL_CLASS_DEF(index, FieldInfo)
CL_CLASS_DEF(index, FieldInfos)
CL_CLASS_DEF(store,IndexInput)

CL_NS_DEF(index)

	/**
	* Class responsible for access to stored document fields.
  * <p/>
	* It uses &lt;segment&gt;.fdt and &lt;segment&gt;.fdx; files.
	*/
	class FieldsReader :LUCENE_BASE{
	private:
		const FieldInfos* fieldInfos;

		// The main fieldStream, used only for cloning.
		CL_NS(store)::IndexInput* cloneableFieldsStream;

		// This is a clone of cloneableFieldsStream used for reading documents.
		// It should not be cloned outside of a synchronized context. 
		CL_NS(store)::IndexInput* fieldsStream;

		CL_NS(store)::IndexInput* indexStream;
		int32_t numTotalDocs;
		int32_t _size;
		bool closed;

		// The docID offset where our docs begin in the index
		// file.  This will be 0 if we have our own private file.
		int32_t docStoreOffset;

		DEFINE_MUTEX(THIS_LOCK)
		CL_NS(util)::ThreadLocal<CL_NS(store)::IndexInput*, CL_NS(util)::Deletor::Object<CL_NS(store)::IndexInput> > fieldsStreamTL;
    static void uncompress(const CL_NS(util)::ValueArray<uint8_t>& input, CL_NS(util)::ValueArray<uint8_t>& output);
	public:
		FieldsReader(CL_NS(store)::Directory* d, const char* segment, FieldInfos* fn,
			int32_t readBufferSize = CL_NS(store)::BufferedIndexInput::BUFFER_SIZE, int32_t docStoreOffset = -1, int32_t size = 0);
		virtual ~FieldsReader();

	//protected:
		/**
		* @throws an exception (CL_ERR_IllegalState) if this FieldsReader is closed
		*/
		void ensureOpen();

		/**
		* Closes the underlying {@link org.apache.lucene.store.IndexInput} streams, including any ones associated with a
		* lazy implementation of a Field.  This means that the Fields values will not be accessible.
		*
		* @throws IOException
		*/
		void close();

		int32_t size() const;
		
		/** Loads the fields from n'th document into doc. returns true on success. */
		bool doc(int32_t n, CL_NS(document)::Document& doc, const CL_NS(document)::FieldSelector* fieldSelector = NULL);

	protected:
		/** Returns the length in bytes of each raw document in a
		*  contiguous range of length numDocs starting with
		*  startDocID.  Returns the IndexInput (the fieldStream),
		*  already seeked to the starting point for startDocID.*/
		CL_NS(store)::IndexInput* rawDocs(int32_t* lengths, const int32_t startDocID, const int32_t numDocs);

	private:
		/**
		* Skip the field.  We still have to read some of the information about the field, but can skip past the actual content.
		* This will have the most payoff on large fields.
		*/
		void skipField(const bool binary, const bool compressed);
		void skipField(const bool binary, const bool compressed, const int32_t toRead);

		void addFieldLazy(CL_NS(document)::Document& doc, const FieldInfo* fi, const bool binary, const bool compressed, const bool tokenize);

		/** Add the size of field as a byte[] containing the 4 bytes of the integer byte size (high order byte first; char = 2 bytes)
		* Read just the size -- caller must skip the field content to continue reading fields
		* Return the size in bytes or chars, depending on field type
		*/
		int32_t addFieldSize(CL_NS(document)::Document& doc, const FieldInfo* fi, const bool binary, const bool compressed);

		// in merge mode we don't uncompress the data of a compressed field
		void addFieldForMerge(CL_NS(document)::Document& doc, const FieldInfo* fi, const bool binary, const bool compressed, const bool tokenize);

		void addField(CL_NS(document)::Document& doc, const FieldInfo* fi, const bool binary, const bool compressed, const bool tokenize);

		CL_NS(document)::Field::TermVector getTermVectorType(const FieldInfo* fi);
		CL_NS(document)::Field::Index getIndexType(const FieldInfo* fi, const bool tokenize);
	
	private:
		/**
		* A Lazy implementation of Field that differs loading of fields until asked for, instead of when the Document is
		* loaded.
		*/
		class LazyField : public CL_NS(document)::Field {
		private:
			int32_t toRead;
			int64_t pointer;
			FieldsReader* parent;

		public:
            LazyField(FieldsReader* _parent, const TCHAR* _name, int config, const int32_t _toRead, const int64_t _pointer);
            virtual ~LazyField();
		private:
			CL_NS(store)::IndexInput* getFieldStream();

		public:
			/** The value of the field in Binary, or null.  If null, the Reader value,
			* String value, or TokenStream value is used. Exactly one of stringValue(), 
			* readerValue(), binaryValue(), and tokenStreamValue() must be set. */
			virtual const CL_NS(util)::ValueArray<uint8_t>* binaryValue();

			/** The value of the field as a Reader, or null.  If null, the String value,
			* binary value, or TokenStream value is used.  Exactly one of stringValue(), 
			* readerValue(), binaryValue(), and tokenStreamValue() must be set. */
			virtual CL_NS(util)::Reader* readerValue();

			/** The value of the field as a String, or null.  If null, the Reader value,
			* binary value, or TokenStream value is used.  Exactly one of stringValue(), 
			* readerValue(), binaryValue(), and tokenStreamValue() must be set. */
			virtual const TCHAR* stringValue();

			/** The value of the field as a TokesStream, or null.  If null, the Reader value,
			* String value, or binary value is used. Exactly one of stringValue(), 
			* readerValue(), binaryValue(), and tokenStreamValue() must be set. */
			virtual CL_NS(analysis)::TokenStream* tokenStreamValue();

			int64_t getPointer() const;
			void setPointer(const int64_t _pointer);

			int32_t getToRead() const;
			void setToRead(const int32_t _toRead);
		};
		friend class LazyField;
    friend class SegmentMerger;
    friend class FieldsWriter;

		// Instances of this class hold field properties and data
		// for merge
		class FieldForMerge : public CL_NS(document)::Field {
		public:
			const TCHAR* stringValue() const;
			CL_NS(util)::Reader* readerValue() const;
			const CL_NS(util)::ValueArray<uint8_t>* binaryValue();
			CL_NS(analysis)::TokenStream* tokenStreamValue() const;

			FieldForMerge(void* _value, ValueType _type, const FieldInfo* fi, const bool binary, const bool compressed, const bool tokenize);
      virtual ~FieldForMerge();

      virtual const char* getObjectName() const;
      static const char* getClassName();
		};
	};
CL_NS_END
#endif
