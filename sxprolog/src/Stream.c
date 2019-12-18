/*
 *   This file is part of the CxProlog system

 *   Stream.c
 *   by A.Miguel Dias - 1989/12/02
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL

 *   CxProlog is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.

 *   CxProlog is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of9
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "CxProlog.h"

ExtraTypePt streamType ;
static StreamPt origIn, origOut, origErr ;
StreamPt userIn, userOut, userErr, userPrt ;
StreamPt currIn, currOut ;

static Str stringStreamData ;
static Pt listStreamData ;

static StreamPt captureOutput = nil, saveUserOut, saveUserErr, saveCurrOut ;


/* PRIVATE FUNCTIONS */

static void StreamInit(StreamPt srm)
{
	StreamLineCount(srm) = 1 ;
	StreamLinePosition(srm) = 0 ;
	StreamCharCount(srm) = 0 ;
	StreamAllowReposition(srm) = false ;
	StreamEofAction(srm) = eofCode ;
	StreamIsATty(srm) = false ;
	StreamEofSeen(srm) = false ;
	StreamIsBinary(srm) = false ;
	StreamIsEdinburgh(srm) = false ;
}

static StreamPt StreamNew(VoidPt chn, Str n, StreamMode m, StreamKind k)
{
	StreamPt srm = ExtraNew(streamType, 0) ;
	StreamKind(srm) = k ;
	StreamMode(srm) = m ;
	StreamChannel(srm) = chn ;
	StreamAtom(srm) = LookupTempAtom(n) ;
	StreamPath(srm) =
		Booting() || !IsFileStream(srm)
				? StreamAtom(srm)
				: AbsoluteFileName(n) ;
	StreamInit(srm) ;
	return srm ;
}

static StreamPt UsingStream(StreamPt srm, StreamMode mode)
{
	if( !XExtraCheck(streamType, TagExtra(streamType, srm)) )
		FileError("Invalid operation over closed stream %s", ExtraAsStr(srm)) ;
	if( mode == mNone )
		return srm ;
	if( (StreamMode(srm)==mRead) == (mode==mRead) )
		return srm ;
	elif( StreamMode(srm) == mRead )
		return FileError("'%s' is a read stream; cannot be used as write stream",
							ExtraAsStr(srm)) ;
	else
		return FileError("'%s' is a write stream; cannot be used as read stream",
							ExtraAsStr(srm)) ;
}

static StreamPt XTestStreamGeneral(Pt t, StreamMode mode, Bool err)
{
	if( Drf(t) == tUserAtom ) {
		switch( mode ) {
			case mRead: return userIn ;
			case mWrite: return userOut ;
			case mAppend: return userOut ;
			default: return userIn ; /* user == user_input by default */
		}
	}
	else {
		StreamPt srm = XTestExtraGen(streamType, t, true, err) ;
		if( srm != nil ) UsingStream(srm, mode) ;
		return srm ;
	}
}

static void EnsureTextStream(StreamPt srm)
{
	if( StreamIsBinary(srm) )
		FileError("This operation is not available for binary streams") ;
}

#if !COMPASS
static void EnsureBinaryStream(StreamPt srm)
{
	if( !StreamIsBinary(srm) )
		FileError("This operation is not available for text streams") ;
}
#endif

static Str StreamModeStr(StreamPt srm)
{
	switch( StreamMode(srm) ) {
		case mRead: return "read" ;
		case mWrite: return "write" ;
		case mAppend: return "append" ;
		default: return InternalError("StreamModeAtStr") ;
	}
}

static StreamMode XTestStreamMode(Pt t)
{
	t = Drf(t) ;
	if( t == tReadAtom ) return mRead ;
	elif( t == tWriteAtom ) return mWrite ;
	elif( t == tAppendAtom ) return mAppend ;
	else { FileError("Invalid stream mode") ; return 0 ; }
}

static Str StreamKindStr(StreamPt srm)
{
	switch( StreamKind(srm) ) {
		case textFileStream: return "file" ;
		case textBufferStream: return "buffer" ;
		case binaryFileStream: return "file" ;
		case binaryBufferStream: return "buffer" ;
		case netStream: return "net" ;
		case threadInputBufferStream: return "thread_input_buffer" ;
		case interactiveFileStream: return "interactive_file" ;
		case nullStream: return "null" ;
		case stringStream: return "string" ;
		case listStream: return "list" ;
		case innerStream: return "inner" ;
		default: return InternalError("StreamKindStr") ;
	}
}

static EofAction XTestEofAction(Pt t)
{
	t = Drf(t) ;
	if( t == tErrorAtom ) return eofError ;
	elif( t == tEofCodeAtom ) return eofCode ;
	elif( t == tResetAtom ) return eofReset ;
	else { FileError("Invalid eof action") ; return 0 ; }
}

static Str StreamEofActionStr(StreamPt srm)
{
	switch( StreamEofAction(srm) ) {
		case eofError: return "error" ;
		case eofCode: return "eof_code" ;
		case eofReset: return "reset" ;
		default: return InternalError("StreamEofActionStr") ;
	}
}

static Size StreamSizeFun(CVoidPt x)
{
	Unused(x) ;
	return WordsOf(Stream) ;
}

static void StreamsBasicGCMarkContents(VoidPt x)
{
	StreamPt srm = cStreamPt(x) ;
	ExtraGCMark(StreamAtom(srm)) ;
	ExtraGCMark(StreamPath(srm)) ;
	if( StreamChannel(srm) != nil )
		ExtraGCMark(StreamChannel(srm)) ;
}

static Bool StreamBasicGCDelete(VoidPt x)
{
	StreamPt srm = cStreamPt(x) ;
	StreamClose(srm, nil) ;
	return true ;
}

void StreamsSetInteractiveSession(void)
{
	if( StreamKind(origIn) == textFileStream )
		StreamKind(origIn) = interactiveFileStream;
}

void StreamsSetUser(StreamPt i, StreamPt o, StreamPt e)
{
	userIn = i;
	userOut = o ;
	userErr = e ;
	AliasSet(tUserInputAtom, userIn) ;
	AliasSet(tUserOutputAtom, userOut) ;
	AliasSet(tUserErrorAtom, userErr) ;
}


/* PRIVATE EDINBURGH FUNCTIONS */

static Bool FindEdinburghStreamAux(CVoidPt x, CVoidPt name)
{
	return StreamIsEdinburgh(cStreamPt(x))
		&& StrEqual(StreamName(cStreamPt(x)), cCharPt(name)) ;
}
static StreamPt FindEdinburghStream(Str name, StreamMode mode)
{
	StreamPt res ;
	if( name[0] == '\0' )
		FileError("'' is an invalid file name") ;
	if( (res = ExtraFindFirst(streamType, 0, FindEdinburghStreamAux, name)) != nil )
		return UsingStream(res, mode) ;
	return nil ;
}

static StreamPt FileEdinburghStreamOpen(Str name, StreamMode mode)
{
	StreamPt srm ;
	if( (srm = FindEdinburghStream(name, mode)) != nil )
		return srm ;
	srm = FileStreamOpen(name, mode, nil) ;
	StreamIsEdinburgh(srm) = true ;
	return srm ;
}

static Size StreamBasicGCMarkAux(CVoidPt x)
{
	StreamPt srm = cStreamPt(x) ;
	if( StreamIsEdinburgh(srm) )
		ExtraGCMark(srm) ;
	return 1 ;
}
static void StreamBasicGCMark()
{
	ExtraForEach(streamType, StreamBasicGCMarkAux) ;
}


/* OPEN & CLOSE OPERATIONS */

StreamPt FileStreamOpen(Str name, StreamMode mode, OpenStreamOptionsPt s)
{
	OpenStreamOptions soDef ;
	OpenStreamOptionsPt so =
				s != nil ? s : StreamPropertyGetDefaultOpenOptions(&soDef) ;
	Bool isBinary = so->type == tBinaryAtom ;
	FilePt f = FileNew(nil, name, mode, EncodingMake(XAtomName(so->encoding)), XTestBool3(so->bom)) ;
	StreamPt srm = StreamNew(f, name, mode,
							isBinary ? binaryFileStream : textFileStream) ;
	StreamIsBinary(srm) = isBinary ;
	StreamEofAction(srm) = XTestEofAction(so->eofAction) ;
	StreamAllowReposition(srm) = so->reposition == tTrueAtom ;
	StreamIsATty(srm) = FileIsATty(f) ;
	if( so->alias != nil )
		AliasSet(so->alias, srm) ;
	return srm ;
}

StreamPt BufferStreamOpen(BufferPt buff, StreamMode mode, OpenStreamOptionsPt s)
{
	OpenStreamOptions soDef ;
	OpenStreamOptionsPt so =
				s != nil ? s : StreamPropertyGetDefaultOpenOptions(&soDef) ;
	Bool isBinary = so->type == tBinaryAtom ;
	Str encoding = EncodingMake(XAtomName(so->encoding)) ; /* may fail */
	StreamPt srm ;
	Str name = GStrFormat("_%%buffer_%lx", cWord(buff)) ;

/* open buffer */
	if( mode == mWrite ) BufferRewrite(buff) ;
	elif( mode == mRead ) BufferReset(buff) ;
	elif( mode == mAppend ) BufferAppend(buff) ;
	else InternalError("BufferStreamOpen") ;

/* setup descriptor */
	srm = StreamNew(buff, name, mode,
					isBinary ? binaryBufferStream : textBufferStream) ;
	BufferSetEncoding(buff, encoding) ;
	StreamIsBinary(srm) = isBinary ;
	StreamEofAction(srm) = XTestEofAction(so->eofAction) ;
	StreamAllowReposition(srm) = so->reposition == tTrueAtom ;
	if( so->alias != nil )
		AliasSet(so->alias, srm) ;
	return srm ;
}

StreamPt NullStreamOpen()
{
	static int n = 0 ;
	Str name = GStrFormat("_%%null_%d", n++) ;
	StreamPt srm = StreamNew(nil, name, mWrite, nullStream) ;
	return srm ;
}

StreamPt StringStreamOpen(Str string)
{
	static StreamPt srm = nil ;
	if( srm == nil ) { /* There is only one string stream */
		srm = StreamNew(nil, "_%string", mRead, stringStream) ;
		ExtraHide(srm) ;
		ExtraPermanent(srm) ;
	}
	else
		StreamInit(srm) ;
	stringStreamData = cCharPtz(string) ;
	return srm ;
}

StreamPt ListStreamOpen(Pt list)
{
	static StreamPt srm = nil ;
	if( srm == nil ) { /* There is only one  stream */
		srm = StreamNew(nil, "_%list", mRead, listStream) ;
		ExtraHide(srm) ;
		ExtraPermanent(srm) ;
	}
	listStreamData = Drf(list) ;
	return srm ;
}

StreamPt InnerStreamOpen()
{
	static StreamPt srm = nil ;
	if( srm == nil ) { /* There is only one inner stream */
		srm = StreamNew(nil, "_%inner", mWrite, innerStream) ;
		ExtraHide(srm) ;
		ExtraPermanent(srm) ;
	}
	BigStrOpen() ;
	return srm ;
}

StreamPt FILEToStream(FILE *file, Str n, StreamMode m, StreamKind k)
{
	FilePt f = FileNew(file, n, m, DefaultEncoding(), undefined3) ;
	StreamPt srm = StreamNew(f, n, m, k) ;
	StreamIsATty(srm) = FileIsATty(f) ;
	return srm ;
}

Str StreamClose(StreamPt srm, CloseStreamOptionsPt co)
{
	CloseStreamOptions coDef ;
	Str res = nil ; /* keep nil here */

	if( srm == origIn || srm == origOut || srm == origErr )
		return nil ;
	if( srm == userIn || srm == userOut || srm == userErr )
		return nil ;

	if( co == nil )
		StreamPropertyGetDefaultCloseOptions(co = &coDef) ;

	switch( StreamKind(srm) ) {
		case textFileStream:
		case binaryFileStream:
		case netStream: {
			if( ExtraDisable(srm) )
				FileDelete(StreamChannel(srm), co->force == tTrueAtom) ;
			break ;
		}
		case interactiveFileStream: {	/* userIn */
			InternalError("StreamClose") ;
			break ;
		}
		case threadInputBufferStream:
		case textBufferStream:
		case binaryBufferStream: {
			ExtraDisable(srm) ;
			break ;
		}
		case nullStream: {
			ExtraDisable(srm) ;
			break ;
		}
		case stringStream: {
			/* Never delete the only permanent string stream */
			break ;
		}
		case listStream: {
			/* Never delete the only permanent list stream */
			break ;
		}
		case innerStream: {
			/* Never delete the only permanent inner stream */
			res = BigStrClose() ;
			break ;
		}
		default: InternalError("StreamClose") ;
	}

	if( srm == currIn ) currIn = userIn ;
	elif( srm == currOut ) currOut = userOut ;
	return res ;
}

void StreamRealiasing(Pt atom, StreamPt srm)
{	/* Sync user streams with the aliases user_input, etc. */
	atom = Drf(atom) ;
	if( srm != nil ) {
		if( atom == tUserInputAtom ) {
			if( currIn == userIn ) currIn = srm ;
			userIn = srm ;
			InterLineChangedUserStreams() ;
		}
		elif( atom == tUserOutputAtom ) {
			StreamFlush(userOut) ;
			if( currOut == userOut ) currOut = srm ;
			userOut = srm ;
			InterLineChangedUserStreams() ;
		}
		elif( atom == tUserErrorAtom ) {
			StreamFlush(userErr) ;
			if( currOut == userErr ) currOut = srm ;
			userErr = srm ;
		}
		elif( atom == tUserAtom )
			Error("A stream cannot be aliased 'user'") ;
	}
	else {
		if( atom == tUserInputAtom )
			AliasSet(tUserInputAtom, origIn) ;
		elif( atom == tUserOutputAtom )
			AliasSet(tUserOutputAtom, origOut) ;
		elif( atom == tUserErrorAtom )
			AliasSet(tUserErrorAtom, origErr) ;
	}
}

Str StreamEncodingName(StreamPt srm)
{
	switch( StreamKind(srm) ) {
		case textFileStream:
		case binaryFileStream:
		case netStream:
		case interactiveFileStream:
			return EncodingName(FileGetEncoding(StreamChannel(srm))) ;

		case textBufferStream:
		case binaryBufferStream:
		case threadInputBufferStream:
			return EncodingName(BufferGetEncoding(StreamChannel(srm))) ;

		default:
			return EncodingName(SystemEncoding()) ;
	}
}

Bool StreamBom(StreamPt srm)
{
	switch( StreamKind(srm) ) {
		case textFileStream:
		case binaryFileStream:
		case netStream:
		case interactiveFileStream:
			return FileGetBom(StreamChannel(srm)) ;

		default:
			return false ;
	}
}


/* BASIC INPUT OPERATIONS */

static void CheckDoubleEOF(StreamPt srm, Bool advance)
{
	if( StreamEofAction(srm) == eofError ) {
		if( StreamEofSeen(srm) )
			FileError("Attempt to read beyond the end_of_file") ;
	}
	if( advance )
		StreamEofSeen(srm) = true ;
}

static void StreamUpdateCounters(StreamPt srm, WChar c)
{
	if( c >= ' ' ) {
		StreamCharCount(srm)++ ;	
		StreamLinePosition(srm)++ ;
	}
	 else {
		if( c == '\n' ) {
			StreamCharCount(srm)++ ;	
			StreamLineCount(srm)++ ;
			StreamLinePosition(srm) = 0 ;
		}
		elif( c == '\t' ) {
			StreamCharCount(srm)++ ;	
			StreamLinePosition(srm) += 8 - StreamLinePosition(srm) % 8 ;
		}
		elif( c == '\b' ) {
			StreamCharCount(srm)++ ;	
			if( StreamLinePosition(srm) > 0 )
				StreamLinePosition(srm)-- ;
		}
		elif( c == EOF )
			CheckDoubleEOF(srm, true) ;
	}
}

WChar StreamGet(StreamPt srm)
{
	WChar c ;
	switch( StreamKind(srm) ) {
		case textFileStream: {
			c = FileGetChar(StreamChannel(srm)) ;
			StreamUpdateCounters(srm, c) ;
			return c ;
		}
		case binaryFileStream:
		case netStream: {
			c = FileGetByte(StreamChannel(srm)) ;
			if( c == EOF ) CheckDoubleEOF(srm, true) ;
			return c ;
		}
		case textBufferStream: {
			c = BufferGetChar(StreamChannel(srm)) ;
			if( c == EOF ) CheckDoubleEOF(srm, true) ;
			return c ;			
		}	
		case threadInputBufferStream: {	/* userIn */
			return ThreadInputGetChar() ;
		}
		case binaryBufferStream: {
			c = BufferGetByte(StreamChannel(srm)) ;
			if( c == EOF ) CheckDoubleEOF(srm, true) ;
			return c ;
		}
		case interactiveFileStream: {	/* userIn */
			c = InterLineGet() ;
			if( c == EOF ) CheckDoubleEOF(srm, true) ;
			return c ;
		}
		case nullStream: {
			FileError("This operation is not available for null streams") ;
			return 0 ;
		}
		case stringStream: {
			if( *stringStreamData == '\0' ) return EOF ;
			c = CharDecode(stringStreamData) ;
			StreamUpdateCounters(srm, c) ;
			return c ;
		}
		case listStream: {
			if( listStreamData == tNilAtom ) return EOF ;
			if( IsList(listStreamData) ) {
				Pt t = Drf(XListHead(listStreamData)) ;
				listStreamData = Drf(XListTail(listStreamData)) ;
				return XTestCode(t) ;
			}
			else TypeError("PROPERLY-TERMINATED-LIST", nil) ;
		}
		case innerStream: {
			FileError("This operation is not available for inner streams") ;
			return 0 ;
		}
		default: return IInternalError("StreamGet") ;
	}
}

WChar StreamPeek(StreamPt srm)
{
	WChar c ;
	switch( StreamKind(srm) ) {
		case textFileStream: {
			c = FilePeekChar(StreamChannel(srm)) ;
			if( c == EOF ) CheckDoubleEOF(srm, false) ;
			return c ;
		}
		case binaryFileStream:
		case netStream: {
			c = FilePeekByte(StreamChannel(srm)) ;
			if( c == EOF ) CheckDoubleEOF(srm, false) ;
			return c ;
		}
		case textBufferStream: {
			c = BufferPeekChar(StreamChannel(srm)) ;
			if( c == EOF ) CheckDoubleEOF(srm, false) ;
			return c ;
		}
		case binaryBufferStream: {
			c = BufferPeekByte(StreamChannel(srm)) ;
			if( c == EOF ) CheckDoubleEOF(srm, false) ;
			return c ;
		}
		case threadInputBufferStream: {	/* userIn */
			c = ThreadInputPeekChar() ;
			if( c == EOF ) CheckDoubleEOF(srm, true) ;
			return c ;
		}
		case interactiveFileStream: {	/* userIn */
			c = InterLinePeek() ;
			if( c == EOF ) CheckDoubleEOF(srm, true) ;
			return c ;
		}
		case nullStream: {
			FileError("This operation is not available for null streams") ;
			return 0 ;
		}
		case stringStream: {
			if( *stringStreamData == '\0' ) return EOF ;
			return CharFirst(stringStreamData) ;
		}
		case listStream: {
			if( listStreamData == tNilAtom ) return EOF ;
			if( IsList(listStreamData) ) {
				Pt t = Drf(XListHead(listStreamData)) ;
				return XTestCode(t) ;
			}
			else TypeError("PROPERLY-TERMINATED-LIST", nil) ;
		}
		case innerStream: {
			FileError("This operation is not available for inner streams") ;
			return 0 ;
		}
		default: return IInternalError("StreamPeek") ;
	}
}

Size StreamReadBytes(StreamPt srm, VoidPt v, Size n)
{
	switch( StreamKind(srm) ) {
		case textFileStream: {
			return FileGetNBytes(StreamChannel(srm), v, n, true) ;
		}
		case binaryFileStream: {
			return FileGetNBytes(StreamChannel(srm), v, n, false) ;
		}
		case textBufferStream:
		case binaryBufferStream: {
			return BufferGetNBytes(StreamChannel(srm), v, n) ;
		}
		case nullStream: {
			return 0 ;
		}
		default: {
			FileError("This operation is not available for this kind of stream") ;
			return -1 ;
		}
	}
}

Bool StreamAtEnd(StreamPt srm)
{
	if( StreamMode(srm) != mRead ) return false ;

	switch( StreamKind(srm) ) {
		case textFileStream: {
			return FilePeekChar(StreamChannel(srm)) == EOF ;
		}
		case binaryFileStream:
		case netStream: {
			return FilePeekByte(StreamChannel(srm)) == EOF ;
		}
		case textBufferStream: {
		case binaryBufferStream:
			return BufferAtEnd(StreamChannel(srm)) ;
		}
		case threadInputBufferStream: {	/* userIn */
			return ThreadInputPeekChar() == EOF ;
		}
		case interactiveFileStream: {	/* userIn */
			return InterLinePeek() == EOF ;
		}
		case nullStream: {
			return false ;
		}
		case stringStream: {
			return *stringStreamData == '\0' ;
		}
		case listStream: {
			return listStreamData == tNilAtom ;
		}
		case innerStream: {
			FileError("This operation is not available for inner streams") ;
			return false ;
		}
		default: return IInternalError("StreamAtEnd") ;
	}
}



/* BASIC OUTPUT OPERATIONS */

void StreamPut(StreamPt srm, WChar c)
{
	switch( StreamKind(srm) ) {
		case textFileStream: {
			if( c < ' ' ) {
				if( c == '\n' ) {
					StreamLineCount(srm)++ ;
					StreamLinePosition(srm) = 0 ;
				}
				elif( c == '\t' ) {
					StreamLinePosition(srm) +=
						8 - StreamLinePosition(srm) % 8 ;
				}
				elif( c == '\b' ) {
					if( StreamLinePosition(srm) > 0 )
						StreamLinePosition(srm)-- ;
				}
				else
					StreamLinePosition(srm)++ ;
			}
			else
				StreamLinePosition(srm)++ ;
			StreamCharCount(srm)++ ;
			FilePutChar(StreamChannel(srm), c) ;
			break ;		
		}
		case binaryFileStream:
		case netStream: {
			FilePutByte(StreamChannel(srm), c) ;
			break ;
		}
		case threadInputBufferStream:
		case textBufferStream: {
			BufferPutChar(StreamChannel(srm), c) ;
			break ;
		}
		case binaryBufferStream: {
			BufferPutByte(StreamChannel(srm), c) ;
			break ;
		}
		case interactiveFileStream: {
			FileError("There are no output interactive file streams") ;
			break ;
		}
		case nullStream: {
			break ;
		}
		case stringStream: {
			FileError("This operation is not available for string streams") ;
			break ;
		}
		case listStream: {
			FileError("This operation is not available for list streams") ;
			break ;
		}
		case innerStream: {
			BigStrAddChar(c) ;
			break ;
		}
		default: InternalError("StreamPut") ;
	}
}

void StreamPutStr(StreamPt srm, Str s)
{
	switch( StreamKind(srm) ) {
		case textFileStream: {
			while( *s )
				StreamPut(srm, CharDecode(s)) ;
			break ;
		}
		case binaryFileStream:
		case netStream: {
			while( *s )
				FilePutByte(StreamChannel(srm), *s++) ;
			break ;
		}
		case threadInputBufferStream:
		case textBufferStream: {
			BufferPutCharStr(StreamChannel(srm), s) ;
			break ;
		}
		case binaryBufferStream: {
			while( *s )
				BufferPutByte(StreamChannel(srm), *s++) ;
			break ;
		}
		case interactiveFileStream: {
			FileError("There are no output interactive file streams") ;
			break ;
		}
		case nullStream: {
			break ;
		}
		case stringStream: {
			FileError("This operation is not available for string streams") ;
			break ;
		}
		case listStream: {
			FileError("This operation is not available for list streams") ;
			break ;
		}
		case innerStream: {
			BigStrAddStr(s) ;
			break ;
		}
		default: InternalError("StreamPutStr") ;
	}
}

void StreamPutStrNl(StreamPt srm, Str s)
{
	StreamPutStr(srm, s) ;
	StreamPut(srm, '\n') ;
}

void StreamPutStrMulti(StreamPt srm, ...)
{
	Str s ;
	va_list v ;
	va_start(v, srm) ;
	while( (s = va_arg(v, CharPt)) != nil )
		StreamPutStr(srm, s) ;
	va_end(v) ;
}

void StreamPutStrSegm(StreamPt srm, Str s, CharPt end)
{
	Char save = *end ;
	*end = '\0' ;
	StreamPutStr(srm, s) ;
	*end = save ;
}

Size StreamWriteBytes(StreamPt srm, VoidPt v, Size n)
{
	switch( StreamKind(srm) ) {
		case textFileStream: {
			return FilePutNBytes(StreamChannel(srm), v, n, true) ;
		}
		case binaryFileStream: {
			return FilePutNBytes(StreamChannel(srm), v, n, false) ;
		}
		case textBufferStream:
		case binaryBufferStream: {
			return BufferPutNBytes(StreamChannel(srm), v, n) ;
		}
		case nullStream: {
			return 0 ;
		}
		default:
			FileError("This operation is not available for this kind of stream") ;
			return -1 ;
	}
}

void StreamWriteV(StreamPt srm, Str fmt, va_list v)
{
	if( srm == nil ) {
		/* Streams have not been initialized yet.
		    Also the memory manager may also not been initialized yet. */
		vfprintf(stderr, fmt, v) ;
	}
	else {
		/* Don't relly on GStrFormat. This may be an emergency message. */
		Str1K s ;
		vsnprintf(s, 1000, fmt, v) ;
		if( srm == userErr ) {
			StreamFlush(userOut) ;
			StreamPutStr(userErr, s) ;
			StreamFlush(userErr) ;
			SysTraceWrite(s) ;
		}
		else StreamPutStr(srm, s) ;
	}
}

void StreamFlush(StreamPt srm)
{
	switch( StreamKind(srm) ) {
		case textFileStream:
		case binaryFileStream:
		case netStream: {
			FileFlush(StreamChannel(srm)) ;
			break ;
		}
		case threadInputBufferStream:
		case textBufferStream:
		case binaryBufferStream:
		case nullStream: {
			break ;
		}
		case interactiveFileStream: {
			FileError("There are no output interactive file streams") ;
			break ;
		}
		case stringStream: {
			FileError("This operation is not available for string streams") ;
			break ;
		}
		case listStream: {
			FileError("This operation is not available for list streams") ;
			break ;
		}
		case innerStream: {
			break ;
		}
		default: InternalError("StreamFlush") ;
	}
}


/* MORE INPUT OPERATIONS */

WChar StreamGetSingleChar() // AMD check buffer case
{
	if( SetRawInput(StreamFILE(userIn)) ) {
		WChar c = FileGetChar(StreamChannel(userIn));
		UnsetRawInput() ;
		return c ;
	}
	else {
		Str s = StreamGetLine(userIn) ;
		if( s == nil )
			return -1 ;
		else {
			while( *s == ' ' ) s++ ;
			return *s == '\0' ? 10 : *s ;
		}
	}		
}

WChar StreamGetNonBlank(StreamPt srm)
{
	for(;;) {
		WChar c = StreamGet(srm) ;
		if( cx_isspace(c) )
			/* continue */ ;
		else
			return c ;
	}
}

WChar StreamPeekNonBlank(StreamPt srm)
{
	for(;;) {
		WChar c = StreamPeek(srm) ;
		if( cx_isspace(c) )
			StreamGet(srm) ;
		else
			return c ;
	}
}

CharPt StreamGetLine(StreamPt srm)
{
	register WChar c = StreamGet(srm) ;
	if( c == EOF ) return nil ;
	GStrOpen() ;
	while( c != '\n' && c != EOF ) {
		GStrAddChar(c) ;
		c = StreamGet(srm) ;
	}
	return GStrClose() ;
}


/* MORE OUTPUT OPERATIONS */

void StreamWrite(StreamPt srm, Str fmt, ...)
{
	va_list v ;
	va_start(v, fmt) ;
	StreamWriteV(srm, fmt, v) ;
	va_end(v) ;
}

void Write(Str fmt, ...)
{
	va_list v ;
	va_start(v, fmt) ;
	StreamWriteV(currOut, fmt, v) ;
	va_end(v) ;
}

void WriteNothing(Str fmt, ...)
{
	va_list v ;
	va_start(v, fmt) ;
	va_end(v) ;
}

void WriteStd(Str fmt, ...)
{
	va_list v ;
	va_start(v, fmt) ;
	StreamWriteV(userOut, fmt, v) ;
	va_end(v) ;
}

void WriteErr(Str fmt, ...)
{
	va_list v ;
	va_start(v, fmt) ;
	StreamWriteV(userErr, fmt, v) ;
	va_end(v) ;
}

void WriteEOF()
{
	StreamWrite(userOut, "EOF\n") ;
}

void Dot(Str fmt, ...)
{
	va_list v ;
	va_start(v, fmt) ;
	StreamWriteV(userErr, fmt, v) ;
	va_end(v) ;
}

static Size FlushAllAux(CVoidPt x)
{
	StreamPt srm = cStreamPt(x) ;
	if( StreamMode(srm) == mWrite || StreamMode(srm) == mAppend )
		StreamFlush(srm) ;
	return 1 ;
}
void StreamFlushAll()
{
	ExtraForEach(streamType, FlushAllAux) ;
}

/* INCOMPLETE */
static Pt XGetListHead(Pt *list)	/* pre: *list already Drf */
{
	if( IsList(*list) ) {
		Pt head = XListHead(*list) ;
		*list = Drf(XListTail(*list)) ;
		return head ;
	}
	return FileError("Missing arguments") ;
}

static void StreamFormat(StreamPt srm, Str fmt, Pt list)
{
	Str s = cCharPtz(fmt) ;
	WChar c ;
	list = Drf(list) ;
	if( !IsList(list) && list != tNilAtom )
		list = MakeList(list, tNilAtom) ;
	while( (c = CharDecode(s)) != '\0' ) {
		if( c == '~' ) {
			Bool hasArg = false ;
			int i, arg = 0 ;
			for(;;) {
				c = CharDecode(s) ;
				if( InRange(c, '0', '9') ) {
					hasArg = true ;
					arg = arg * 10 + c - '0' ;
				}
				else break ;
			}
			if( !hasArg ) arg = 1 ;
			switch( c ) {
				case '@': {
					Pt t = XGetListHead(&list) ;
					Size lvrLevel = LvrPush(&list) ;
					dotimes(i, arg)
						CallProlog(t) ;
					LvrRestore(lvrLevel) ;
					break ;
				}
				case '~': {
					dotimes(i, arg)
						StreamPut(srm, '~') ;
					break ;
				}
				case 'a': {
					Str str = XTestAtomName(XGetListHead(&list)) ;
					dotimes(i, arg)
						StreamPutStr(srm, str) ;
					break ;
				}
				case 'c': {
					PInt n = XTestInt(XGetListHead(&list)) ;
					dotimes(i, arg)
						StreamPut(srm, n) ;
					break ;
				}
				case 'd': {
					PInt n = XTestInt(XGetListHead(&list)) ;
					if( hasArg )
						{/* TODO */ } ;
					StreamWrite(srm, "%ld", n) ;
					break ;
				}
				case 'D': {
					StreamWrite(srm, "%ld", XTestInt(XGetListHead(&list))) ; /* Will change */
					break ;
				}
				case 'e':
				case 'E':
				case 'f':
				case 'F':
				case 'g':
				case 'G': {
					StreamPutStr(srm, FloatAsStr(c, XTestFloat(XGetListHead(&list)))) ;
					break ;
				}
				case 'i': {
					dotimes(i, arg)
						XGetListHead(&list) ;
					break ;
				}
				case 'k': {
					Pt t = XGetListHead(&list) ;
					dotimes(i, arg)
						TermWriteC(srm, t) ;
					break ;
				}
				case 'n': {
					dotimes(i, arg)
						StreamPut(srm, '\n') ;
					break ;
				}
				case 'p': {
					Pt t = XGetListHead(&list) ;
					dotimes(i, arg)
						TermWriteP(srm, t) ;
					break ;
				}
				case 'q': {
					Pt t = XGetListHead(&list) ;
					dotimes(i, arg)
						TermWriteQ(srm, t) ;
					break ;
				}
				case 's': {
					Str s = PStringToString(XGetListHead(&list)) ;
					dotimes(i, arg)
						StreamPutStr(srm, s) ;
					break ;
				}
				case 'w': {
					Pt t = XGetListHead(&list) ;
					dotimes(i, arg)
						TermWriteN(srm, t) ;
					break ;
				}
				default: {
					FileError("Invalid ~%c format option", c) ;
				}
			}
		}
		elif( c == '\\' ) {
			switch( (c = CharDecode(s)) ) {
				case 'n':
					StreamPut(srm, '\n') ;
					break ;
				default:
					StreamPut(srm, c) ;
					break ;
			}
		}
		else StreamPut(srm, c) ;
	}
}


/* CAPTURE OUTPUT */

void CaptureOutputStart()
{
	BufferPt buff ;
	if( captureOutput == nil ) {
		buff = BufferNew() ;
		ExtraPermanent(buff) ;
		ExtraHide(buff) ;	
		captureOutput = BufferStreamOpen(buff, mWrite, nil) ;
		ExtraPermanent(captureOutput) ;
		ExtraHide(captureOutput) ;
#if USE_UTF8_AS_INTERNAL_ENCODING
		BufferSetEncoding(buff, EncodingMake("utf8")) ; /* Must be last */
#else
		BufferSetEncoding(buff, EncodingMake("iso_latin_1")) ; /* Must be last */
#endif
		
	}
	buff = StreamChannel(captureOutput) ;
	BufferRewrite(buff) ;
	saveUserOut = userOut ;
	saveUserErr = userErr ;
	saveCurrOut = currOut ;
	userOut = userErr = currOut = captureOutput ;
}

Str CaptureOutputEnd()
{
	if( userOut == captureOutput ) {
		BufferPt buff = StreamChannel(captureOutput) ;
		userOut = saveUserOut ;
		userErr = saveUserErr ;
		currOut = saveCurrOut ;
		BufferPutChar(buff, '\0') ;
		return cCharPt(BufferContents(buff)) ;
	}
	else return nil ;
}


/* OTHER */

static Size StreamByteCount(StreamPt srm) {
	switch( StreamKind(srm) ) {
		case textFileStream:
			return ftell(StreamFILE(srm)) ;
		case binaryFileStream:
		case netStream:
			return 0 ;
		case textBufferStream:
			return 0 ;
		case binaryBufferStream:
			return 0 ;
		case interactiveFileStream:
			return 0 ;
		case nullStream:
			return 0 ;
		case stringStream:
			return 0 ;
		case listStream:
			return 0 ;
		case innerStream:
			return 0 ;
		default:
			return IInternalError("StreamByteCount") ;
	}
}

Pt BuildStreamPositionTerm(StreamPt srm) {
	Pt args[4] ;
	args[0] = MakeInt(StreamCharCount(srm)) ;
	args[1] = MakeInt(StreamLineCount(srm)) ;
	args[2] = MakeInt(StreamLinePosition(srm)) ;
	args[3] = MakeInt(StreamByteCount(srm)) ;
	return MakeStruct(streamPositionFunctor, args) ;
}


/* CXPROLOG C'BUILTINS */

static void PStreamCheck()
{
	MustBe( XExtraCheck(streamType, X0) ) ;
}

static void POpenFileStream3()
{
	StreamPt srm = FileStreamOpen(XTestFileName(X0), XTestStreamMode(X1), nil) ;
	BindVarWithExtra(X2, srm) ;
	JumpNext() ;
}

#if COMPASS
static void POpenFileStream4()
{
	OpenStreamOptions so ;
	StreamPt srm = FileStreamOpen(XTestFileName(X0), XTestStreamMode(X1),
								StreamPropertyGetOpenOptions(X2, &so)) ;
	if( so.alias != nil ) BindVarWithExtra(so.alias, srm) ;
	BindVarWithExtra(X3, srm) ;
	JumpNext() ;
}
#else
static void POpenFileStream4()
{
	OpenStreamOptions so ;
	StreamPt srm = FileStreamOpen(XTestFileName(X0), XTestStreamMode(X1),
								StreamPropertyGetOpenOptions(X3, &so)) ;
	if( so.alias != nil ) BindVarWithExtra(so.alias, srm) ;
	BindVarWithExtra(X2, srm) ;
	JumpNext() ;
}
#endif

static void POpenBufferStream3()
{
	StreamPt srm = BufferStreamOpen(XTestBuffer(X0), XTestStreamMode(X1), nil) ;
	BindVarWithExtra(X2, srm) ;
	JumpNext() ;
}

#if COMPASS
static void POpenBufferStream4()
{
	OpenStreamOptions so ;
	StreamPt srm = BufferStreamOpen(XTestBuffer(X0), XTestStreamMode(X1),
								StreamPropertyGetOpenOptions(X2, &so)) ;
	if( so.alias != nil ) BindVarWithExtra(so.alias, srm) ;
	BindVarWithExtra(X3, srm) ;
	JumpNext() ;
}
#else
static void POpenBufferStream4()
{
	OpenStreamOptions so ;
	StreamPt srm = BufferStreamOpen(XTestBuffer(X0), XTestStreamMode(X1),
								StreamPropertyGetOpenOptions(X3, &so)) ;
	if( so.alias != nil ) BindVarWithExtra(so.alias, srm) ;
	BindVarWithExtra(X2, srm) ;
	JumpNext() ;
}
#endif

static void POpenNull()
{
	BindVarWithExtra(X0, NullStreamOpen()) ;
	JumpNext() ;
}

static void PClose1()
{
	StreamPt srm = XTestStream(X0, mNone) ;
	StreamClose(srm, nil) ;
	JumpNext() ;
}

static void PClose2()
{
	CloseStreamOptions co ;
	StreamPt srm = XTestStream(X0, mNone) ;
	StreamClose(srm, StreamPropertyGetCloseOptions(X1, &co)) ;
	JumpNext() ;
}

static Bool CurrentStreamHandle(VoidPt x)
{
	register StreamPt srm = cStreamPt(x) ;
	return UnifyWithAtomic(X0, MakeTempAtom(StreamName(srm)))
		&& UnifyWithAtomic(X1, MakeTempAtom(StreamModeStr(srm))) ;
}
static void PNDCurrentStream3()
{
	ExtraPNDCurrent(streamType, CurrentStreamHandle, 3, 2) ;
	JumpNext() ;
}

static Bool CurrentStreamHandle4(VoidPt x)
{
	StreamPt srm = cStreamPt(x) ;
	Pt t = IsAbsoluteFileName(AtomName(StreamPath(srm)))
				? ZPushTerm(PathNameListFromAtom(StreamPath(srm)))
				: TagAtom(StreamPath(srm)) ;
	return UnifyWithAtomic(X0, TagAtom(StreamAtom(srm)))
		&& UnifyWithAtomic(X1, MakeTempAtom(StreamModeStr(srm)))
		&& Unify(X2, t) ;
}
static void PNDCurrentStream4()
{
	ExtraPNDCurrent(streamType, CurrentStreamHandle4, 4, 3) ;
	JumpNext() ;
}

static Size PStreamsAux(CVoidPt x)
{
	register StreamPt srm = cStreamPt(x) ;
	if( IsAbsoluteFileName(AtomName(StreamPath(srm))) )
		Write("absolute_file_name('%s')", AtomName(StreamPath(srm))) ;
	Write("\n%10s", "") ;
	Write("name('%s'), ", StreamName(srm)) ;
	Write("kind(%s), ", StreamKindStr(srm)) ;
	Write("type(%s), ", StreamIsBinary(srm) ? "binary" : "text") ;
	Write("\n%10s", "") ;
	Write("mode(%s), ", StreamModeStr(srm)) ;
	Write("encoding(%s), ", StreamEncodingName(srm)) ;
	Write("bom(%s), ", StreamBom(srm) ? "true" : "false") ;
	Write("tty(%s), ", StreamIsATty(srm) ? "true" : "false") ;
	Write("\n%10s", "") ;
	Write("current(%s), ", (srm == currIn || srm == currOut) ? "true" : "false") ;
	Write("reposition(%s), ", StreamAllowReposition(srm) ? "true" : "false") ;
	Write("eof_action(%s), ", StreamEofActionStr(srm)) ;
	Write("\n%10s", "") ;
	if( srm == origIn || (StreamIsATty(srm) && StreamMode(srm) == mRead) )
		Write("at_eof(false)") ;	/* avoids waiting for input */
	else
		Write("at_eof(%s)", StreamAtEnd(srm) ? "true" : "false") ;
	return 1 ;
}
static void PStreams()
{
	ExtraShow(streamType, PStreamsAux) ;
	JumpNext() ;
}

static void PSetUserStreams()
{
	StreamsSetUser(XTestStream(X0, mRead),
				XTestStream(X1, mWrite),
				XTestStream(X2, mWrite)) ;
	JumpNext() ;
}

static void PSetInput()
{
	currIn = XTestStream(X0, mRead) ;
	JumpNext() ;
}

static void PCurrentInput()
{
	MustBe( UnifyWithAtomic(X0, TagExtra(streamType, currIn)) ) ;
}

static void PSetOutput()
{
	currOut = XTestStream(X0, mWrite) ;
	JumpNext() ;
}

static void PCurrentOutput()
{
	MustBe( UnifyWithAtomic(X0, TagExtra(streamType, currOut)) ) ;
}

static void PSee()
{
	StreamPt srm ;
	if( (srm = XTestStreamGeneral(X0, mRead, false)) != nil )
		currIn = srm ;
	elif( IsAtom(Drf(X0)) )
		currIn = FileEdinburghStreamOpen(XTestFileName(X0), mRead) ;
	else ExtraTypeError(streamType, "FILENAME", X0) ;
	JumpNext() ;
}

static void PSeeing()
{
	if( currIn == userIn )
		MustBe( UnifyWithAtomic(X0, tUserAtom ) ) ;
	elif( oldFashionedSeeing_flag )
		MustBe( UnifyWithAtomic(X0, MakeTempAtom(StreamName(currIn))) ) ;
	else
		MustBe( UnifyWithAtomic(X0, TagExtra(streamType, currIn)) ) ;
}

static void PSeen()
{
	StreamClose(currIn, nil) ;
	currIn = userIn ;
	JumpNext() ;
}

static void PTell()
{
	StreamPt srm ;
	if( (srm = XTestStreamGeneral(X0, mWrite, false)) != nil )
		currOut = srm ;
	elif( IsAtom(Drf(X0)) )
		currOut = FileEdinburghStreamOpen(XTestFileName(X0), mWrite) ;
	else ExtraTypeError(streamType, "FILENAME", X0) ;
	JumpNext() ;
}

static void PTelling()
{
	if( currOut == userOut )
		MustBe( UnifyWithAtomic(X0, tUserAtom ) ) ;
	elif( oldFashionedSeeing_flag )
		MustBe( UnifyWithAtomic(X0, MakeTempAtom(StreamName(currOut))) ) ;
	else
		MustBe( UnifyWithAtomic(X0, TagExtra(streamType, currOut)) ) ;
}

static void PTold()
{
	StreamClose(currOut, nil) ;
	currOut = userOut ;
	JumpNext() ;
}

#if !COMPASS
static void PGetChar()
{
	EnsureTextStream(currIn) ;
	Mesg("wwwwwwwwwwwwwwwww %p", currIn) ;
	MustBe( UnifyWithAtomic(X0, MakeChar(StreamGet(currIn))) ) ;
}

static void PSGetChar()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	EnsureTextStream(srm) ;
	MustBe( UnifyWithAtomic(X1, MakeChar(StreamGet(srm))) ) ;
}

static void PGetCode()
{
	EnsureTextStream(currIn) ;
	MustBe( UnifyWithAtomic(X0, MakeCode(StreamGet(currIn))) ) ;
}

static void PSGetCode()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	EnsureTextStream(srm) ;
	MustBe( UnifyWithAtomic(X1, MakeCode(StreamGet(srm))) ) ;
}
#endif

static void PGet()
{
	EnsureTextStream(currIn) ;
	MustBe( UnifyWithNumber(X0, MakeCode(StreamGetNonBlank(currIn))) ) ;
}

static void PSGet()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	EnsureTextStream(srm) ;
	MustBe( UnifyWithNumber(X1, MakeCode(StreamGetNonBlank(srm))) ) ;
}

static void PGetLine()
{
	Str str ;
	EnsureTextStream(currIn) ;
	if( (str = StreamGetLine(currIn)) == nil )
		MustBe( UnifyWithAtomic(X0, MakeCode(EOF)) ) ;
	else
		MustBe( UnifyWithAtomic(X0, MakeTempAtom(str)) ) ;
}

static void PSGetLine()
{
	Str str ;
	StreamPt srm = XTestStream(X0, mRead) ;
	EnsureTextStream(srm) ;
	if( (str = StreamGetLine(srm)) == nil )
		MustBe( UnifyWithAtomic(X1, MakeCode(EOF)) ) ;
	else
		MustBe( UnifyWithAtomic(X1, MakeTempAtom(str)) ) ;
}

static void PGetSingleChar()
{
	MustBe( UnifyWithNumber(X0, MakeCode(StreamGetSingleChar())) ) ;
}

static void PGetCharWithPrompt()
{
	WChar c ;
	int arg ;
	c = InterLineGetCommand(XTestAtomName(X0), &arg) ;
	MustBe( UnifyWithAtomic(X1, MakeChar(c))
		&& UnifyWithNumber(X2, MakeInt(arg)) ) ;
}

#if !COMPASS
static void PPeekChar()
{
	EnsureTextStream(currIn) ;
	MustBe( UnifyWithAtomic(X0, MakeChar(StreamPeek(currIn))) ) ;
}

static void PSPeekChar()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	EnsureTextStream(srm) ;
	MustBe( UnifyWithAtomic(X1, MakeChar(StreamPeek(srm))) ) ;
}

static void PPeekCode()
{
	EnsureTextStream(currIn) ;
	MustBe( UnifyWithAtomic(X0, MakeCode(StreamPeek(currIn))) ) ;
}

static void PSPeekCode()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	EnsureTextStream(srm) ;
	MustBe( UnifyWithAtomic(X1, MakeCode(StreamPeek(srm))) ) ;
}
#endif

static void PPeek()
{
	EnsureTextStream(currIn) ;
	MustBe( UnifyWithNumber(X0, MakeCode(StreamPeekNonBlank(currIn))) ) ;
}

static void PSPeek()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	EnsureTextStream(srm) ;
	MustBe( UnifyWithNumber(X1, MakeCode(StreamPeekNonBlank(srm))) ) ;
}

#if !COMPASS
static void PPutChar()
{
	EnsureTextStream(currOut) ;
	StreamPut(currOut, XTestChar(X0)) ;
	JumpNext() ;
}

static void PSPutChar()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	EnsureTextStream(srm) ;
	StreamPut(srm, XTestChar(X1)) ;
	JumpNext() ;
}

static void PPutCode()
{
	EnsureTextStream(currOut) ;
	StreamPut(currOut, XTestCode(X0)) ;
	JumpNext() ;
}

static void PSPutCode()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	EnsureTextStream(srm) ;
	StreamPut(srm, XTestCode(X1)) ;
	JumpNext() ;
}
#endif

static void PNl()
{
	EnsureTextStream(currOut) ;
	StreamPut(currOut, '\n') ;
	JumpNext() ;
}

static void PSNl()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	EnsureTextStream(srm) ;
	StreamPut(srm, '\n') ;
	JumpNext() ;
}

static void PTab()
{
	Size n = XTestInt(X0) ;
	EnsureTextStream(currOut) ;
	while( n-- )
		StreamPut(currOut, ' ') ;
	JumpNext() ;
}

static void PSTab()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	Size n = XTestInt(X1) ;
	EnsureTextStream(srm) ;
	while( n-- )
		StreamPut(srm, ' ') ;
	JumpNext() ;
}

static void PFormat1()
{
	StreamFormat(currOut, XTestAtomNameOrPString(X0), tNilAtom) ;
	JumpNext() ;
}

static void PFormat()
{
	StreamFormat(currOut, XTestAtomNameOrPString(X0), X1) ;
	JumpNext() ;
}

static void PSFormat()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	StreamFormat(srm, XTestAtomNameOrPString(X1), X2) ;
	JumpNext() ;
}


#if !COMPASS
static void PGetByte()
{
	EnsureBinaryStream(currIn) ;
	MustBe( UnifyWithAtomic(X0, MakeByte(StreamGet(currIn))) ) ;
}

static void PSGetByte()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	EnsureBinaryStream(srm) ;
	MustBe( UnifyWithAtomic(X1, MakeByte(StreamGet(srm))) ) ;
}

static void PPeekByte()
{
	EnsureBinaryStream(currIn) ;
	MustBe( UnifyWithAtomic(X0, MakeByte(StreamPeek(currIn))) ) ;
}

static void PSPeekByte()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	EnsureBinaryStream(srm) ;
	MustBe( UnifyWithAtomic(X1, MakeByte(StreamPeek(srm))) ) ;
}

static void PPutByte()
{
	if( StreamKind(currOut) != nullStream )
		EnsureBinaryStream(currOut) ;
	StreamPut(currOut, XTestByte(X0)) ;
	JumpNext() ;
}

static void PSPutByte()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	if( StreamKind(srm) != nullStream )
		EnsureBinaryStream(srm) ;
	StreamPut(srm, XTestByte(X1)) ;
	JumpNext() ;
}
#endif

static void PAtEndOfStream()
{
	MustBe( StreamAtEnd(currIn) ) ;
}

static void PSAtEndOfStream()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	MustBe( StreamAtEnd(srm) ) ;
}

static void PFlushOutput()
{
	StreamFlush(currOut) ;
	JumpNext() ;
}

static void PSFlushOutput()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	StreamFlush(srm) ;
	JumpNext() ;
}

static void PSFlushOutputAll()
{
	StreamFlushAll() ;
	JumpNext() ;
}

static void PGet0()
{
	WChar c = StreamGet(currIn) ;
	Pt t = StreamIsBinary(currIn) ? MakeByte(c) : MakeCode(c) ;
	MustBe( UnifyWithNumber(X0, t) ) ;
}

static void PSGet0()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	WChar c = StreamGet(srm) ;
	Pt t = StreamIsBinary(srm) ? MakeByte(c) : MakeCode(c) ;
	MustBe( UnifyWithNumber(X1, t) ) ;
}

static void PSkip()
{
	WChar c = StreamIsBinary(currIn) ? XTestByte(X0) : XTestCode(X0) ;
	while( StreamGet(currIn) != c ) ;
	JumpNext() ;
}

static void PSSkip()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	WChar c = StreamIsBinary(srm) ? XTestByte(X1) : XTestCode(X1) ;
	while( StreamGet(srm) != c ) ;
	JumpNext() ;
}

static void PPeek0()
{
	WChar c = StreamPeek(currIn) ;
	Pt t = StreamIsBinary(currIn) ? MakeByte(c) : MakeCode(c) ;
	MustBe( UnifyWithNumber(X0, t) ) ;
}

static void PSPeek0()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	WChar c = StreamPeek(srm) ;
	Pt t = StreamIsBinary(srm) ? MakeByte(c) : MakeCode(c) ;
	MustBe( UnifyWithNumber(X1, t) ) ;
}

static void PPut()
{
	WChar c = StreamIsBinary(currOut) ? XTestByte(X0) : XTestCode(X0) ;
	StreamPut(currOut, c) ;
	JumpNext() ;
}

static void PSPut()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	WChar c = StreamIsBinary(srm) ? XTestByte(X1) : XTestCode(X1) ;
	StreamPut(srm, c) ;
	JumpNext() ;
}

static void PSGetBlock2()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	BufferPt b = XTestBuffer(X1) ;
	Size n ;
	BufferRewrite(b) ;
	do {
		BufferEnsureFreeSpace(b, 256) ;
		n = StreamReadBytes(srm, BufferContents(b) + BufferSize(b), 256) ;
		BufferSetSizeUnsafe(b, BufferSize(b) + n) ;
	} while( n == 256 ) ;
	JumpNext() ;
}

static void PSGetBlock3()
{
	StreamPt srm = XTestStream(X0, mRead) ;
	BufferPt b = XTestBuffer(X1) ;
	PInt n = XTestPosInt(X2) ;
	BufferRewrite(b) ;
	BufferEnsureFreeSpace(b, n) ;
	n = StreamReadBytes(srm, BufferContents(b), n) ;	/* may read less */
	BufferSetSizeUnsafe(b, n) ;
	JumpNext() ;
}

static void PSPutBlock()
{
	StreamPt srm = XTestStream(X0, mWrite) ;
	BufferPt b = XTestBuffer(X1) ;
	if( BufferSize(b) > 0 ) {
		Size n = StreamWriteBytes(srm, BufferContents(b), BufferSize(b)) ;
		if( n == 0 ) FileError("Could not write block") ;
		elif( n < BufferSize(b) ) FileError("Could not write entire block") ;
	}
	JumpNext() ;
}

static void PCharacterCount()
{
	StreamPt srm = XTestStream(X0, mNone) ;
	MustBe( UnifyWithAtomic(X1, MakeInt(StreamCharCount(srm))) ) ;
}

static void PLineCount()
{
	StreamPt srm = XTestStream(X0, mNone) ;
	MustBe( UnifyWithAtomic(X1, MakeInt(StreamLineCount(srm))) ) ;
}

static void PLinePosition()
{
	StreamPt srm = XTestStream(X0, mNone) ;
	MustBe( UnifyWithAtomic(X1, MakeInt(StreamLinePosition(srm))) ) ;
}

static void PByteCount()
{
	StreamPt srm = XTestStream(X0, mNone) ;
	MustBe( UnifyWithAtomic(X1, MakeInt(StreamByteCount(srm))) ) ;
}

static void PStreamPositionData()
{
	Str op = XTestAtomName(X0) ;
	Pt spos = Drf(X1) ;
	Ensure( IsThisStruct(spos, streamPositionFunctor) ) ;
	if( strcmp(op, "char_count") == 0 )
		MustBe( UnifyWithAtomic(X2, XStructArg(spos, 0)) ) ;
	elif( strcmp(op, "line_count") == 0 )
		MustBe( UnifyWithAtomic(X2, XStructArg(spos, 1)) ) ;
	elif( strcmp(op, "line_position") == 0 )
		MustBe( UnifyWithAtomic(X2, XStructArg(spos, 2)) ) ;
	elif( strcmp(op, "byte_count") == 0 )
		MustBe( UnifyWithAtomic(X2, XStructArg(spos, 3)) ) ;
	else
		DomainError("STREAM POSITION SELECTOR", X0) ;
}

static void PSetInteractiveSession(void)
{
	StreamsSetInteractiveSession();
	JumpNext() ;
}

static void PTTTest()
{
/* 7.84 - 9.42 - 32.32 */
/* 2.37 - 3.59 - 9.57 - 172 */
	StreamPt srm ;
	double start = CpuTime() ;
	int i ;
	srm = FileStreamOpen(WithTmpDirPath("tttest", "log"), mWrite, nil) ;
	dotimes(i, 30000000) {
		StreamPut(srm, 'a') ;
	}
	StreamClose( srm, nil) ;
	Mesg("time = %lf", CpuTime() - start) ;
	JumpNext() ;
}


/* TEST, EXTRACT & INIT */

void StreamsSane()
{
	currIn = userIn ;
	currOut = userOut ;
}

Bool IsStream(Pt t)
{
	return IsThisExtra(streamType, t) ;
}

StreamPt XTestStream(Pt t, StreamMode mode)
{
	return XTestStreamGeneral(t, mode, true) ;
}

void StreamsInit2()
{
	InstallCBuiltinPred("stream", 1, PStreamCheck) ;

	InstallCBuiltinPred("open", 3, POpenFileStream3) ;
	InstallCBuiltinPred("open", 4, POpenFileStream4) ;
	InstallCBuiltinPred("open_buffer_stream", 3, POpenBufferStream3) ;
	InstallCBuiltinPred("open_buffer_stream", 4, POpenBufferStream4) ;
	InstallCBuiltinPred("open_null_stream", 1, POpenNull) ;
	InstallCBuiltinPred("close", 1, PClose1) ;
	InstallCBuiltinPred("close", 2, PClose2) ;
	InstallGNDeterCBuiltinPred("current_stream", 3, 2, PNDCurrentStream3) ;
	InstallGNDeterCBuiltinPred("current_stream", 4, 2, PNDCurrentStream4) ;
	InstallCBuiltinPred("streams", 0, PStreams) ;
	InstallCBuiltinPred("$set_user_streams", 3, PSetUserStreams) ;

	InstallCBuiltinPred("set_input", 1, PSetInput) ;
	InstallCBuiltinPred("current_input", 1, PCurrentInput) ;
	InstallCBuiltinPred("set_output", 1, PSetOutput) ;
	InstallCBuiltinPred("current_output", 1, PCurrentOutput) ;

	InstallCBuiltinPred("see", 1, PSee) ;
	InstallCBuiltinPred("seeing", 1, PSeeing) ;
	InstallCBuiltinPred("seen", 0, PSeen) ;
	InstallCBuiltinPred("tell", 1, PTell) ;
	InstallCBuiltinPred("telling", 1, PTelling) ;
	InstallCBuiltinPred("told", 0, PTold) ;
		
#if !COMPASS
	InstallCBuiltinPred("get_char", 1, PGetChar) ;
	InstallCBuiltinPred("get_char", 2, PSGetChar) ;
	InstallCBuiltinPred("get_code", 1, PGetCode) ;
	InstallCBuiltinPred("get_code", 2, PSGetCode) ;
#endif
	InstallCBuiltinPred("get", 1, PGet) ;
	InstallCBuiltinPred("get", 2, PSGet) ;
	InstallCBuiltinPred("get_line", 1, PGetLine) ;
	InstallCBuiltinPred("get_line", 2, PSGetLine) ;
	InstallCBuiltinPred("get_single_char", 1, PGetSingleChar) ;
	InstallCBuiltinPred("get_char_with_prompt", 3, PGetCharWithPrompt) ;

#if !COMPASS
	InstallCBuiltinPred("peek_char", 1, PPeekChar) ;
	InstallCBuiltinPred("peek_char", 2, PSPeekChar) ;
	InstallCBuiltinPred("peek_code", 1, PPeekCode) ;
	InstallCBuiltinPred("peek_code", 2, PSPeekCode) ;
#endif
	InstallCBuiltinPred("peek", 1, PPeek) ;
	InstallCBuiltinPred("peek", 2, PSPeek) ;

#if !COMPASS
	InstallCBuiltinPred("put_char", 1, PPutChar) ;
	InstallCBuiltinPred("put_char", 2, PSPutChar) ;
	InstallCBuiltinPred("put_code", 1, PPutCode) ;
	InstallCBuiltinPred("put_code", 2, PSPutCode) ;
#endif
	InstallCBuiltinPred("nl", 0, PNl) ;
	InstallCBuiltinPred("nl", 1, PSNl) ;
	InstallCBuiltinPred("tab", 1, PTab) ;
	InstallCBuiltinPred("tab", 2, PSTab) ;
	InstallCBuiltinPred("format", 1, PFormat1) ;
	InstallCBuiltinPred("format", 2, PFormat) ;
	InstallCBuiltinPred("format", 3, PSFormat) ;

#if !COMPASS
	InstallCBuiltinPred("get_byte", 1, PGetByte) ;
	InstallCBuiltinPred("get_byte", 2, PSGetByte) ;
	InstallCBuiltinPred("peek_byte", 1, PPeekByte) ;
	InstallCBuiltinPred("peek_byte", 2, PSPeekByte) ;
	InstallCBuiltinPred("put_byte", 1, PPutByte) ;
	InstallCBuiltinPred("put_byte", 2, PSPutByte) ;
#endif

	InstallCBuiltinPred("at_end_of_stream", 0, PAtEndOfStream) ;
	InstallCBuiltinPred("at_end_of_stream", 1, PSAtEndOfStream) ;
	InstallCBuiltinPred("flush_output", 0, PFlushOutput) ;
	InstallCBuiltinPred("flush_output", 1, PSFlushOutput) ;
	InstallCBuiltinPred("flush_output_all", 0, PSFlushOutputAll) ;
#if COMPASS
	InstallCBuiltinPred("flush", 0, PFlushOutput) ;
	InstallCBuiltinPred("flush", 1, PSFlushOutput) ;
	InstallCBuiltinPred("flushall", 0, PSFlushOutputAll) ;
#endif

	InstallCBuiltinPred("get0", 1, PGet0) ;
	InstallCBuiltinPred("get0", 2, PSGet0) ;
	InstallCBuiltinPred("skip", 1, PSkip) ;
	InstallCBuiltinPred("skip", 2, PSSkip) ;
	InstallCBuiltinPred("peek0", 1, PPeek0) ;
	InstallCBuiltinPred("peek0", 2, PSPeek0) ;
	InstallCBuiltinPred("put", 1, PPut) ;
	InstallCBuiltinPred("put", 2, PSPut) ;

	InstallCBuiltinPred("get_block", 2, PSGetBlock2) ;
	InstallCBuiltinPred("get_block", 3, PSGetBlock3) ;
	InstallCBuiltinPred("put_block", 2, PSPutBlock) ;
	
	InstallCBuiltinPred("byte_count", 2, PByteCount) ;
	InstallCBuiltinPred("character_count", 2, PCharacterCount) ;
	InstallCBuiltinPred("line_count", 2, PLineCount) ;
	InstallCBuiltinPred("line_position", 2, PLinePosition) ;
	InstallCBuiltinPred("stream_position_data", 3, PStreamPositionData) ;
	// ?- stream_property(user_input,position(X)).  X = '$stream_position'(1288, 46, 0, 1288).
	
	InstallCBuiltinPred("set_interactive_session", 0, PSetInteractiveSession) ;

	InstallCBuiltinPred("ttttt", 0, PTTTest) ;
}

void StreamsInit()
{
	if( sizeof(WChar) < 4  )
		FatalError("Size of WChar is too small to contain Unicode characters %d", sizeof(WChar)) ;

	streamType = ExtraTypeNew("STREAM", StreamSizeFun, StreamsBasicGCMarkContents, StreamBasicGCDelete, 1) ;
	ExtraGCHandlerInstall("STREAM", StreamBasicGCMark) ;

	ExtraSpecial(XTestAtom(tUserInputAtom)) ;
	ExtraSpecial(XTestAtom(tUserOutputAtom)) ;
	ExtraSpecial(XTestAtom(tUserErrorAtom)) ;
	ExtraSpecial(XTestAtom(tUserAtom)) ;

	currIn = origIn =
		FILEToStream(stdin, "original_user_input", mRead, textFileStream) ;
	currOut = origOut =
		FILEToStream(stdout, "original_user_output", mWrite, textFileStream) ;
	origErr =
		FILEToStream(stderr, "original_user_error", mWrite, textFileStream) ;
	StreamEofAction(origIn) = eofReset ;

	ExtraPermanent(origIn) ;
	ExtraPermanent(origOut) ;
	ExtraPermanent(origErr) ;
	
	StreamsSetUser(origIn, origOut, origErr) ;
	
	// In case of remote usage...
	setvbuf(stdout, nil, _IOLBF, 0); /* line buffering */
	setvbuf(stderr, nil, _IONBF, 0); /* no buffering */

	#define supressPrompts		0
	Bool isConsole = StreamIsATty(origIn) && StreamIsATty(origOut) ;
	if( !supressPrompts && isConsole )
		StreamsSetInteractiveSession();

	/* FileInfo(stdin); */
}
