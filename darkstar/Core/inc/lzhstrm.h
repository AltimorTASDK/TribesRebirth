//================================================================
//
// $Workfile:   lzhstrm.h  $
// $Version$
// $Revision:   1.2  $
//
// DESCRIPTION:
//
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _LZHSTRM_H_
#define _LZHSTRM_H_


#include <streamio.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif


//---------------------------------------------------------------------------

#define LZH_BUFF_SIZE	4096						// buffer size
#define LZH_LOOKAHEAD	60							// lookahead buffer size
#define LZH_THRESHOLD	2							// Match len threshhold
#define LZH_NIL			LZH_BUFF_SIZE			// leaf of tree

// Huffman coding

#define LZH_NCHAR  		(256 - LZH_THRESHOLD + LZH_LOOKAHEAD)
															// kinds of char (code = 0..LZH_NCHAR-1)
#define LZH_TABLE_SIZE 	(LZH_NCHAR * 2 - 1)	// size of table
#define LZH_ROOT 			(LZH_TABLE_SIZE - 1)	// position of root
#define LZH_MAX_FREQ		0x8000					// updates tree when the root
															// frequency comes to this value.


//---------------------------------------------------------------------------

class LZHuffman
{
	// Tables for encoding and decoding the upper 6 bits
	// of position.

	static unsigned char p_len[64];
	static unsigned char p_code[64];
	static unsigned char d_code[256];
	static unsigned char d_len[256];

	//	freq:	frequency table
	// prnt:	pointers to parent nodes, except for the
	// 		elements [LZH_TABLE_SIZE..LZH_TABLE_SIZE + LZH_NCHAR - 1] which
	//			are used to get the positions of leaves corresponding to the
	//			codes.
	// son:	pointers to child nodes (son[], son[] + 1)

	StreamIO*	d_stream;
	UInt16*		freq;
	Int16* 		prnt;
	Int16* 		son;

	UInt16		d_getbuf;
	UInt8			d_getlen;
	UInt16		d_putbuf;
	UInt8			d_putlen;

private:

	int 	getBit(void);
	int 	getByte(void);
	void 	putCode(int l, unsigned c);
	void 	reconst(void);
	void 	update(int c);


public:

	LZHuffman();
	~LZHuffman();

	void   attach(StreamIO &s);
   void 	flush();
	void 	encodeChar(unsigned int c);
	void 	encodePosition(unsigned c);
	int    decodeChar(void);
	int 	decodePosition(void);
};



//---------------------------------------------------------------------------

class LZHWStream: public StreamIO
{
private:
	StreamIO*		d_stream;
	LZHuffman*		d_huff;
   int            iFileSize;
	int				ER,ES,ELEN,EFULL,ELM,ELF;

	int				match_position;
	int				match_length;
	Int32				curPos;

	unsigned char*	text_buf;
	short*			lson;
	short*			rson;
	short*			dad;

	void 	insertNode(int r);
	void 	deleteNode(int p);

   void   initialize();

public:

	LZHWStream();
	LZHWStream(StreamIO& s);

   void   attach(StreamIO &s);
   void   detach();
   void   close();

	// Virtual methods:

	~LZHWStream();
	Int32 	getPosition() const;
	Bool  	setPosition(Int32 pos);
	Int32		getCapabilities(StreamCap) const;
   Bool  	flush();

	using StreamIO::write;
	using StreamIO::read;

	Bool 	write(int size,const void* d);
	Bool 	read(int size,void* d);
};


//---------------------------------------------------------------------------

class LZHRStream: public StreamIO
{
private:
	StreamIO*		d_stream;
	LZHuffman*		d_huff;
	unsigned char*	text_buf;
	int				DI,DJ,DK,DR;
	Int32				curPos;

   void  initialize();

public:

	LZHRStream();
	LZHRStream(StreamIO& s);

   void   attach(StreamIO &s);
   void   detach();
   void   close();

	// Virtual methods:

	~LZHRStream();
	Int32 	getPosition() const;
	Bool  	setPosition(Int32 pos);
	Int32		getCapabilities(StreamCap) const;
   Bool  	flush();

	using StreamIO::write;
	using StreamIO::read;

	Bool 	write(int size,const void* d);
	Bool 	read(int size,void* d);
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif //_LZHSTRM_H_

