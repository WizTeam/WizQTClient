/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include "CLucene/_ApiHeader.h"
#include "CLucene/util/Misc.h"
#include "CLucene/util/BitSet.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/index/IndexReader.h"
#include "ChainedFilter.h"

CL_NS_DEF(search)
CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(document)


ChainedFilter::ChainedFilter( Filter ** _filters, int _op ):
	Filter(),
	filters(_filters),
	logicArray(NULL),
	logic(_op)
{
}
ChainedFilter::ChainedFilter( Filter** _filters, int* _array ):
	Filter(),
	filters(_filters),
	logicArray(_array),
	logic(-1)
{
}
ChainedFilter::ChainedFilter( const ChainedFilter& copy ) :
	Filter(copy),
	logicArray( copy.logicArray ),
	logic( copy.logic )
{
	filters = copy.filters;
}
ChainedFilter::~ChainedFilter(void)
{

}

Filter* ChainedFilter::clone() const {
	return _CLNEW ChainedFilter(*this );
}

const TCHAR* ChainedFilter::getLogicString(int logic){
	if ( logic == ChainedFilter::OR )
		return _T("OR");
	else if ( logic == ChainedFilter::AND )
		return _T("AND");
	else if ( logic == ChainedFilter::ANDNOT )
		return _T("ANDNOT");
	else if ( logic == ChainedFilter::XOR )
		return _T("XOR");
	else if ( logic >= ChainedFilter::USER ){
		return _T("USER");
	}
	return _T("");
}

TCHAR* ChainedFilter::toString()
{

	Filter** filter = filters;

	StringBuffer buf(_T("ChainedFilter: ["));
	int* la = logicArray;
	while(*filter )
	{
		if ( filter != filters )
			buf.appendChar(' ');
		buf.append(getLogicString(logic==-1?*la:logic));
		buf.appendChar(' ');

		TCHAR* filterstr = (*filter)->toString();
		buf.append(filterstr);
		_CLDELETE_ARRAY( filterstr );

		filter++;
		if ( logic == -1 )
			la++;
	}

	buf.appendChar(']');

	return buf.toString();
}


/** Returns a BitSet with true for documents which should be permitted in
search results, and false for those that should not. */
BitSet* ChainedFilter::bits( IndexReader* reader )
{
	if( logic != -1 )
		return bits( reader, logic );
	else if( logicArray != NULL )
		return bits( reader, logicArray );
	else
		return bits( reader, DEFAULT );
}


BitSet* ChainedFilter::bits( IndexReader* reader, int logic )
{
	BitSet* bts = NULL;

	Filter** filter = filters;

	// see discussion at top of file
	if( *filter ) {
		BitSet* tmp = (*filter)->bits( reader );
		if ( (*filter)->shouldDeleteBitSet(tmp) ) //if we are supposed to delete this BitSet, then
			bts = tmp; //we can safely call it our own
		else if ( tmp == NULL ){
			int32_t len = reader->maxDoc();
			bts = _CLNEW BitSet( len ); //bitset returned null, which means match _all_
			for (int32_t i=0;i<len;i++ )
				bts->set(i);
		}else{
			bts = tmp->clone(); //else it is probably cached, so we need to copy it before using it.
		}
		filter++;
	}
	else
		bts = _CLNEW BitSet( reader->maxDoc() );

	while( *filter ) {
		doChain( bts, reader, logic, *filter );
		filter++;
	}

	return bts;
}


BitSet* ChainedFilter::bits( IndexReader* reader, int* _logicArray )
{
	BitSet* bts = NULL;

	Filter** filter = filters;
	int* logic = _logicArray;

	// see discussion at top of file
	if( *filter ) {
		BitSet* tmp = (*filter)->bits( reader );
		if ( (*filter)->shouldDeleteBitSet(tmp) ) //if we are supposed to delete this BitSet, then
			bts = tmp; //we can safely call it our own
		else if ( tmp == NULL ){
			int32_t len = reader->maxDoc();
			bts = _CLNEW BitSet( len ); //bitset returned null, which means match _all_
			for (int32_t i=0;i<len;i++ )
				bts->set(i); //todo: this could mean that we can skip certain types of filters
		}
		else
		{
			bts = tmp->clone(); //else it is probably cached, so we need to copy it before using it.
		}
		filter++;
		logic++;
	}
	else
		bts = _CLNEW BitSet( reader->maxDoc() );

	while( *filter ) {
		doChain( bts, reader, *logic, *filter );
		filter++;
		logic++;
	}

	return bts;
}

void ChainedFilter::doUserChain( CL_NS(util)::BitSet* /*chain*/, CL_NS(util)::BitSet* /*filter*/, int /*logic*/ ){
	_CLTHROWA(CL_ERR_Runtime,"User chain logic not implemented by superclass");
}

BitSet* ChainedFilter::doChain( BitSet* resultset, IndexReader* reader, int logic, Filter* filter )
{
	BitSet* filterbits = filter->bits( reader );
	int32_t maxDoc = reader->maxDoc();
	int32_t i=0;
	if ( logic >= ChainedFilter::USER ){
		doUserChain(resultset,filterbits,logic);
	}else{
		switch( logic )
		{
		case OR:
			for( i=0; i < maxDoc; i++ )
				resultset->set( i, (resultset->get(i) || (filterbits==NULL || filterbits->get(i) ))?1:0 );
			break;
		case AND:
			for( i=0; i < maxDoc; i++ )
				resultset->set( i, (resultset->get(i) && (filterbits==NULL || filterbits->get(i) ))?1:0 );
			break;
		case ANDNOT:
			for( i=0; i < maxDoc; i++ )
				resultset->set( i, (resultset->get(i) && (filterbits==NULL || filterbits->get(i)))?0:1 );
			break;
		case XOR:
			for( i=0; i < maxDoc; i++ )
				resultset->set( i, resultset->get(i) ^ ((filterbits==NULL || filterbits->get(i) )?1:0) );
			break;
		default:
			doChain( resultset, reader, DEFAULT, filter );
		}
	}

	if ( filter->shouldDeleteBitSet(filterbits) )
		_CLDELETE( filterbits );

	return resultset;
}

CL_NS_END
