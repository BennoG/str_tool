#include "stl_compressLzw.h"
#include <memory.h>

#define MINCODESIZE 4
#define MAXCODESIZE 10
#define CLEARCODE 16
#define ENDCODE CLEARCODE+1
#define INIFREECODE CLEARCODE+2
#define INIMAXCODE 32

static const unsigned int CodeMask[12] = {
	0x0001, 0x0003, 0x0007, 0x000f,
	0x001f, 0x003f, 0x007f, 0x00ff,
	0x01ff, 0x03ff, 0x07ff, 0x0fff
};



stlCompressLzwAlenco::stlCompressLzwAlenco()
{

}

void stlCompressLzwAlenco::InitCodeTable(void)
{
	lzw.CodeSize = MINCODESIZE+1;
	lzw.MaxCode  = INIMAXCODE;
	lzw.FreeCode = INIFREECODE;
}
int stlCompressLzwAlenco::InputChar(void)
{
	int result;
	if (lzw.rawDataIdx >= lzw.rawData.length()) return -1;
	if(lzw.LowBits) result = (lzw.rawData[lzw.rawDataIdx++])&0x0f;
	else result = (lzw.rawData[lzw.rawDataIdx])>>4;
	lzw.LowBits = !lzw.LowBits;
	return result;
}
int stlCompressLzwAlenco::SearchCode(int Prefix, unsigned char C)
{
	int result;
	result = INIFREECODE;
	while(result<lzw.FreeCode)
	{
		if((CodeTable[result].Prefix == Prefix)&&(CodeTable[result].C==C))
			return result;
		result++;
	}
	return -1;
}
void stlCompressLzwAlenco::OutputCode(int Code)
{
	long temp;
	if(lzw.BitsLeft==0)
	{
		lzw.compressData.append((char)(Code & 0xFF));
		lzw.compressData.append((char)(Code >>   8));
		//*lzw.compressData = Code&0xff;
		//*(lzw.compressData+1) = Code>>8;
	}else
	{
		temp = Code;
		temp <<= lzw.BitsLeft;
		lzw.compressData[-1] |= (unsigned char)temp;
		lzw.compressData.append((char)(temp>> 8));
		lzw.compressData.append((char)(temp>>16));
		//*lzw.compressData |= (unsigned char)temp;
		//*(lzw.compressData+1) |= (unsigned char)(temp>>8);
		//*(lzw.compressData+2) |= (unsigned char)(temp>>16);
	}
	//lzw.compressData += ((lzw.CodeSize+lzw.BitsLeft)/8);
	lzw.BitsLeft = (lzw.CodeSize+lzw.BitsLeft)%8;
}



int stlCompressLzwAlenco::InputCode(void)
{
	int result;
	long temp = lzw.compressData[lzw.compressIdx + 2];
	temp = (temp << 8) | lzw.compressData[lzw.compressIdx + 1];
	temp = (temp << 8) | lzw.compressData[lzw.compressIdx + 0];



	//temp = *(lzw.compressData+2);
	//temp = (temp<<8)|(*(lzw.compressData+1));
	//temp = (temp<<8)|(*lzw.compressData);
	if(lzw.BitsLeft) temp >>= lzw.BitsLeft;
	result = (int)(temp & CodeMask[lzw.CodeSize-1]);
	lzw.compressData += ((lzw.CodeSize+lzw.BitsLeft)/8);
	lzw.BitsLeft = (lzw.CodeSize+lzw.BitsLeft)%8;
	return result;
}

void stlCompressLzwAlenco::OutputChar(unsigned char C)
{
	if(lzw.LowBits)
	{
		lzw.rawData[-1] |= C;
		//*lzw.rawData |= C;
		//lzw.rawData++;
	}else{
		lzw.rawData.append((char)(C << 4));
		//lzw.rawData = C<<4;
	}
	lzw.LowBits = !lzw.LowBits;
}

unsigned char stlCompressLzwAlenco::OutputString(int Code)
{
	int Count;
	Count = 0;
	while(Code>=INIFREECODE)
	{
		StrBuf[Count++] = CodeTable[Code].C;
		Code = CodeTable[Code].Prefix;
	}
	StrBuf[Count] = Code;
	while(Count>=0) OutputChar(StrBuf[Count--]);
	return Code;
}




unsigned int stlCompressLzwAlenco::compress(int inputSize, int outputSize, unsigned char *pDotStream, unsigned char *pCodeStream)
{
	int Prefix,C,Code;
	lzw.rawData      = stlSetSt((char*)pDotStream);
	lzw.compressData = stlSetSt("");
	lzw.BitsLeft = lzw.CodeSize = lzw.compressIdx = lzw.FreeCode = lzw.LowBits = lzw.MaxCode = lzw.rawDataIdx = 0;
	InitCodeTable();
	C = InputChar();
	Prefix = C;
	for(;;)
	{
		C = InputChar();
		if(C==-1) break;
		Code = SearchCode(Prefix,C);
		if(Code==-1)
		{
			OutputCode(Prefix);
			if(lzw.FreeCode==lzw.MaxCode)
			{
				if(lzw.CodeSize==MAXCODESIZE)
				{
					OutputCode(CLEARCODE);
					InitCodeTable();
					Prefix = C;
					continue;
				}else
				{
					lzw.CodeSize++;
					lzw.MaxCode <<= 1;
				}
			}
			CodeTable[lzw.FreeCode].Prefix = Prefix;
			CodeTable[lzw.FreeCode].C = C;
			lzw.FreeCode++;
			Prefix = C;
		}else Prefix = Code;
	}
	OutputCode(Prefix);
	OutputCode(ENDCODE);
	//if(lzw.BitsLeft) lzw.compressData++;
	//return (unsigned int)(lzw.compressData-pCodeStream);
	return 0;

}

void stlCompressLzwAlenco::ExpandDot(int DotWidth, int DotHeight, unsigned char *pDotStream, unsigned char *pCodeStream)
{
	int Code,OldCode;
	unsigned char C;
	lzw.rawData = "";
	lzw.compressData = (char*)pCodeStream;
	lzw.BitsLeft = lzw.CodeSize = lzw.compressIdx = lzw.FreeCode = lzw.LowBits = lzw.MaxCode = lzw.rawDataIdx = 0;
	InitCodeTable();
	Code = InputCode();
	C = Code&0xff;
	OutputChar(C);
	OldCode = Code;
	for(;;)
	{
		Code = InputCode();
		if(Code==ENDCODE) break;
		if(Code==CLEARCODE)
		{
			InitCodeTable();
			Code = InputCode();
			C = Code&0xff;
			OutputChar(C);
		}else
		{
			if(Code<lzw.FreeCode) C = OutputString(Code);	
			else
			{
				C = OutputString(OldCode);
				OutputChar(C);
			}
			CodeTable[lzw.FreeCode].Prefix = OldCode;
			CodeTable[lzw.FreeCode].C = C;
			lzw.FreeCode++;
			if(lzw.FreeCode==lzw.MaxCode)
			{
				if(lzw.CodeSize<MAXCODESIZE)
				{
					lzw.CodeSize++;
					lzw.MaxCode <<= 1;
				}
			}
		}
		OldCode = Code;
	}
}



#define BITS 12                   /* Setting the number of bits to 12, 13 or 14 affects several constants.  */
#define HASHING_SHIFT (BITS-8) 
#define MAX_VALUE (1 << BITS) - 1 
#define MAX_CODE MAX_VALUE - 1    

#if BITS == 14
#  define TABLE_SIZE 18041        /* The string table size needs to be a */
#endif                            /* prime number that is somewhat larger*/
#if BITS == 13                    /* than 2**BITS.                       */
#  define TABLE_SIZE 9029
#endif
#if BITS <= 12
#  define TABLE_SIZE 5021
#endif

stlCompressLzw::stlCompressLzw()
{
	initVars();
	code_value = new int[TABLE_SIZE];
	prefix_code = new unsigned int[TABLE_SIZE];
	append_character = new unsigned char[TABLE_SIZE];
}

stlCompressLzw::~stlCompressLzw()
{
	if (code_value ) delete code_value;
	if (prefix_code) delete prefix_code;
	if (append_character) delete append_character;
}

STP stlCompressLzw::compress(const char *data)
{
	fInput = fOutput = NULL;
	sInputData  = data;
	sOutputCode = "";
	inputDataIdx = 0;
	compress();
	return sOutputCode.getStp();
}
STP stlCompressLzw::compress(const unsigned char *data)
{
	return compress((const char *)data);
}


STP stlCompressLzw::compress(STP data)
{
	fInput = fOutput = NULL;
	sInputData  = data;
	sOutputCode = "";
	inputDataIdx = 0;
	compress();
	return sOutputCode.getStp();
}

void stlCompressLzw::compress(FILE *input,FILE *output)
{
	fInput  = input;
	fOutput = output;
	sInputData = "";
	compress();
	fInput = fOutput = NULL;
}

/*
** This is the compression routine.  The code should be a fairly close
** match to the algorithm accompanying the article.
**
*/
void stlCompressLzw::compress()
{
	unsigned int next_code;
	unsigned int character;
	unsigned int string_code;
	unsigned int index;

	next_code=256;							/* Next code is the next available string code*/
	for (int i = 0; i < TABLE_SIZE; i++)	/* Clear out the string table before starting */
		code_value[i] = -1;

	string_code=input_data();				//  Get the first code
	/*
	** This is the main loop where it all happens.  This loop runs until all of
	** the input has been exhausted.  Note that it stops adding codes to the
	** table after all of the possible codes have been defined.
	*/
	//while ((character=getc(input)) != (unsigned)EOF)
	while ((character=input_data()) != (unsigned)EOF)
	{
		index=find_match(string_code,character);/* See if the string is in */
		if (code_value[index] != -1)			/* the table.  If it is,   */
			string_code=code_value[index];		/* get the code value.  If */
		else									/* the string is not in the*/
		{										/* table, try to add it.   */
			if (next_code <= MAX_CODE)
			{
				code_value[index]=next_code++;
				prefix_code[index]=string_code;
				append_character[index]=character;
			}
			output_code(string_code);			/* When a string is found  */
			string_code=character;				/* that is not in the table*/
		}										/* I output the last string*/
	}											/* after adding the new one*/
	/*
	** End of the main loop.
	*/
	output_code(string_code); /* Output the last code               */
	output_code(MAX_VALUE);   /* Output the end of buffer code      */
	output_code(0);           /* This code flushes the output buffer*/
	printf("\n");
}

STP stlCompressLzw::expand(STP data)
{
	fInput = fOutput = NULL;
	sInputCode   = data;
	inputCodeIdx = 0;
	sOutputData  = "";
	expand();
	return sOutputData.getStp();
}

/*
**  This is the expansion routine.  It takes an LZW format file, and expands
**  it to an output file.  The code here should be a fairly close match to
**  the algorithm in the accompanying article.
*/
void stlCompressLzw::expand()
{
	unsigned int next_code;
	unsigned int new_code;
	unsigned int old_code;
	int character;
	unsigned char *string;

	next_code=256;           /* This is the next available code to define */

	old_code=input_code();  /* Read in the first code, initialize the */
	character=old_code;     /* character variable, and send the first */
	output_data(old_code);	/* code to the output file                */
	/*
	**  This is the main expansion loop.  It reads in characters from the LZW file
	**  until it sees the special code used to inidicate the end of the data.
	*/
	while ((new_code=input_code()) != (MAX_VALUE))
	{
		/*
		** This code checks for the special STRING+CHARACTER+STRING+CHARACTER+STRING
		** case which generates an undefined code.  It handles it by decoding
		** the last code, and adding a single character to the end of the decode string.
		*/
		if (new_code>=next_code)
		{
			*decode_stack=character;
			string=decode_string(decode_stack+1,old_code);
		}
		/*
		** Otherwise we do a straight decode of the new code.
		*/
		else
			string=decode_string(decode_stack,new_code);
		/*
		** Now we output the decoded string in reverse order.
		*/
		character=*string;
		while (string >= decode_stack)
			output_data(*string--);
		/*
		** Finally, if possible, add a new code to the string table.
		*/
		if (next_code <= MAX_CODE)
		{
			prefix_code[next_code]=old_code;
			append_character[next_code]=character;
			next_code++;
		}
		old_code=new_code;
	}
}

/*
** This routine simply decodes a string from the string table, storing
** it in a buffer.  The buffer can then be output in reverse order by
** the expansion program.
*/
unsigned char *stlCompressLzw::decode_string(unsigned char *buffer,unsigned int code)
{
	int i;

	i=0;
	while (code > 255)
	{
		*buffer++ = append_character[code];
		code=prefix_code[code];
		if (i++>=MAX_CODE)
		{
			printf("#90# stlCompressLzw::decode_string Fatal error during code expansion.\n");
			return NULL;
		}
	}
	*buffer=code;
	return(buffer);
}


/*
** This is the hashing routine.  It tries to find a match for the prefix+char
** string in the string table.  If it finds it, the index is returned.  If
** the string is not found, the first available index in the string table is
** returned instead.
*/

int stlCompressLzw::find_match(int hash_prefix,unsigned int hash_character)
{

	int index;
	int offset;

	index = (hash_character << HASHING_SHIFT) ^ hash_prefix;
	if (index == 0)
		offset = 1;
	else
		offset = TABLE_SIZE - index;
	while (1)
	{
		if (code_value[index] == -1)
			return(index);
		if (prefix_code[index] == (unsigned int)(hash_prefix) && 
			append_character[index] == hash_character)
			return(index);
		index -= offset;
		if (index < 0)
			index += TABLE_SIZE;
	}


/*

	int offset = 1;

	int index = (hash_character << HASHING_SHIFT) ^ hash_prefix;
	if (index != 0) offset = TABLE_SIZE - index;
	while (1)
	{
		if (code_value[index] == -1)
			return(index);
		if (prefix_code[index] == hash_prefix && 
			append_character[index] == hash_character)
			return(index);
		index -= offset;
		if (index < 0)
			index += TABLE_SIZE;
	}
	*/
}


/*
** The following two routines are used to output variable length
** codes.  They are written strictly for clarity, and are not
** particularly efficient.
*/
unsigned int stlCompressLzw::input_code()
{
	unsigned int return_value;
	static int input_bit_count=0;
	static unsigned long input_bit_buffer=0L;

	while (input_bit_count <= 24)
	{
		int ch = EOF;
		if (fInput) ch = getc(fInput);
		else if (inputCodeIdx >= sInputCode.length()) ch = (sInputCode[inputCodeIdx++] & 0xFF);

		input_bit_buffer |= (unsigned long) ch << (24 - input_bit_count);
		input_bit_count += 8;
	}
	return_value=input_bit_buffer >> (32-BITS);
	input_bit_buffer <<= BITS;
	input_bit_count -= BITS;
	return(return_value);
}

void stlCompressLzw::output_code(unsigned int code)
{
	output_bit_buffer |= (unsigned long) code << (32-BITS-output_bit_count);
	output_bit_count += BITS;
	while (output_bit_count >= 8)
	{
		if (fOutput) putc(output_bit_buffer >> 24,fOutput);
		else sOutputCode.append((char)(output_bit_buffer >> 24));
		output_bit_buffer <<= 8;
		output_bit_count -= 8;
	}
}

int stlCompressLzw::input_data()
{
	if (fInput) return getc(fInput);
	if (inputDataIdx >= sInputData.length()) return EOF;
	return (sInputData[inputDataIdx++] & 0xFF);
}

void stlCompressLzw::output_data(unsigned char data)	// uncompressed data stream writer used by expand
{
	if (fOutput){ putc(data,fOutput); return; }
	sOutputData.append((char)data);
}