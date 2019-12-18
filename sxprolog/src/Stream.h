/*
 *   This file is part of the CxProlog system

 *   Stream.h
 *   by A.Miguel Dias - 1989/12/02
 *   CITI - Centro de Informatica e Tecnologias da Informacao
 *   Dept. de Informatica, FCT, Universidade Nova de Lisboa.
 *   Copyright (C) 1990-2016 A.Miguel Dias, CITI, DI/FCT/UNL
 */

#ifndef _Stream_
#define _Stream_

/* STREAM */
typedef enum {
	textFileStream, binaryFileStream,
	textBufferStream, binaryBufferStream,
	interactiveFileStream, threadInputBufferStream,
	netStream, nullStream,
/* for internal use */
	stringStream, listStream, innerStream
} StreamKind ;

typedef enum {
	mNone, mRead, mWrite,  mAppend
} StreamMode ;

typedef enum {
	eofError, eofCode, eofReset
} EofAction ;

typedef struct Stream {
	ExtraDef(Stream) ;
	VoidPt channel ;
	AtomPt atom ;
	AtomPt path ;
    Size lineCount, linePosition, charCount ;
	StreamKind kind ;
	StreamMode mode ;
	Bool allowReposition ;
	EofAction eofAction ;
	Bool isATty ;
	Bool eofSeen ;
	Bool isBinary ;
	Bool isEdinburgh ;
} Stream, *StreamPt ;

#define cStreamPt(s)			((StreamPt)(s))

#define StreamChannel(s)		((s)->channel)
#define StreamFILE(s)			FileAsFILE(StreamChannel(s))
#define StreamAtom(s)			((s)->atom)
#define StreamName(s)			AtomName(StreamAtom(s))
#define StreamPath(s)			((s)->path)
#define StreamLineCount(s)		((s)->lineCount)
#define StreamLinePosition(s)	((s)->linePosition)
#define StreamCharCount(s)		((s)->charCount)
#define StreamKind(s)			((s)->kind)
#define StreamMode(s)			((s)->mode)
#define StreamAllowReposition(s) ((s)->allowReposition)
#define StreamEofAction(s)		((s)->eofAction)
#define StreamIsATty(s)			((s)->isATty)
#define StreamEofSeen(s)		((s)->eofSeen)
#define StreamIsBinary(s)		((s)->isBinary)
#define StreamIsEdinburgh(s)	((s)->isEdinburgh)

#define IsFileStream(s)			( (s)->kind <= binaryFileStream )


/* PUBLIC VARS */
extern ExtraTypePt streamType ;
extern StreamPt userIn, userOut, userErr, userPrt ;
extern StreamPt currIn, currOut ;

/* INTERACTIVE SESSION */
void StreamsSetInteractiveSession(void) ;
void StreamsSetUser(StreamPt i, StreamPt o, StreamPt e) ;

/* OPEN & CLOSE OPERATIONS */
StreamPt FileStreamOpen(Str name, StreamMode mode, OpenStreamOptionsPt so) ;
StreamPt BufferStreamOpen(BufferPt buff, StreamMode mode, OpenStreamOptionsPt so) ;
StreamPt NullStreamOpen(void) ;
StreamPt StringStreamOpen(Str string) ;
StreamPt ListStreamOpen(Pt list) ;
StreamPt InnerStreamOpen(void) ;
StreamPt FILEToStream(FILE *file, Str n, StreamMode m, StreamKind k) ;
Str StreamClose(StreamPt srm, CloseStreamOptionsPt co) ;
void StreamRealiasing(Pt atom, StreamPt srm) ;
Str StreamEncodingName(StreamPt srm) ;
Bool StreamBom(StreamPt srm) ;

/* BASIC INPUT OPERATIONS */
WChar StreamGet(StreamPt srm) ;
WChar StreamPeek(StreamPt srm) ;
Size StreamReadBytes(StreamPt srm, VoidPt v, Size n) ;
Bool StreamAtEnd(StreamPt srm) ;

/* BASIC OUTPUT OPERATIONS */
void StreamPut(StreamPt srm, WChar c) ;
void StreamPutStr(StreamPt srm, Str s) ;
void StreamPutStrNl(StreamPt srm, Str s) ;
void StreamPutStrMulti(StreamPt srm, ...) ;
void StreamPutStrSegm(StreamPt srm, Str s, CharPt end) ;
Size StreamWriteBytes(StreamPt srm, VoidPt v, Size n) ;
void StreamWriteV(StreamPt srm, Str fmt, va_list v) ;
void StreamFlush(StreamPt srm) ;

/* MORE INPUT OPERATIONS */
WChar StreamGetSingleChar(void);
WChar StreamGetNonBlank(StreamPt srm) ;
WChar StreamPeekNonBlank(StreamPt srm) ;
CharPt StreamGetLine(StreamPt srm) ;

/* MORE OUTPUT OPERATIONS */
void StreamWrite(StreamPt srm, Str fmt, ...) ;
void Write(Str fmt, ...) ;
void WriteNothing(Str fmt, ...) ;
void WriteStd(Str fmt, ...) ;
void WriteErr(Str fmt, ...) ;
void WriteEOF(void) ;
void Dot(Str fmt, ...) ;
void StreamFlushAll(void) ;

/* CAPTURE OUTPUT */
void CaptureOutputStart(void) ;
Str CaptureOutputEnd(void) ;

/* TEST, EXTRACT & INIT */
Bool IsStream(Pt t) ;
StreamPt XTestStream(Pt t, StreamMode mode) ;
Pt BuildStreamPositionTerm(StreamPt srm) ;
void StreamsSane(void) ;
void StreamsInit2(void) ;
void StreamsInit(void) ;

#endif
