#include "stl_str.h"

#define LARGESTCODE 1023

class stlCompressLzwAlenco
{
private:
	struct{
		int rawDataIdx ,compressIdx;
		ansStl::cST rawData, compressData;
		int CodeSize;
		int MaxCode;
		int FreeCode;
		int LowBits;
		int BitsLeft;
	}lzw;
	struct{
		int Prefix;
		unsigned char C;
	}CodeTable[LARGESTCODE+1];
	unsigned char StrBuf[LARGESTCODE+1];


public:
	stlCompressLzwAlenco();
	//STP compress(void *src,unsigned long srcLen);
	unsigned int compress(int inputSize, int outputSize, unsigned char *inputBuffer, unsigned char *outputBuffer);
	void ExpandDot(int inputSize, int outputSize, unsigned char *pDotStream, unsigned char *pCodeStream);

private:
	void InitCodeTable(void);
	int InputChar(void);
	int SearchCode(int Prefix, unsigned char C);
	void OutputCode(int Code);
	int InputCode(void);
	void OutputChar(unsigned char C);
	unsigned char OutputString(int Code);
};

class stlCompressLzw
{
private:
	void initVars(){ code_value = NULL; prefix_code = NULL; append_character = NULL; fInput = fOutput = NULL; inputDataIdx = inputCodeIdx = 0; output_bit_count = 0; output_bit_buffer = 0; }
private:
	int *code_value;                  /* This is the code value array        */
	unsigned int *prefix_code;        /* This array holds the prefix codes   */
	unsigned char *append_character;  /* This array holds the appended chars */
	unsigned char decode_stack[4000]; /* This array holds the decoded string */
	FILE *fInput;
	ansStl::cST sInputCode;
	int inputCodeIdx;
	// gebruikt voor code output stream
	FILE *fOutput;
	int output_bit_count;
	unsigned long output_bit_buffer;

	ansStl::cST sInputData,sOutputCode,sOutputData;
	int inputDataIdx;					// index into sInputData
public:
	stlCompressLzw();
	~stlCompressLzw();
	void compress(FILE *input,FILE *output);
	STP  compress(const char *data);
	STP  compress(const unsigned char *data);
	STP  compress(STP data);

	STP  expand(STP data);
private:
	void compress();
	void expand();
	unsigned char *decode_string(unsigned char *buffer,unsigned int code);
	int find_match(int hash_prefix,unsigned int hash_character);
	unsigned int input_code();				// compressed data stream reader used by expand
	void output_code(unsigned int code);	// compressed data stream writer used by compress
	// stream simulator uncompressed input
	int input_data();						// uncompressed data stream reader used by compress
	void output_data(unsigned char data);	// uncompressed data stream writer used by expand
};


STP stlCompressZlib(STP src,int level = 6);
STP stlCompressZlib(void *src,unsigned long srcLen,int level = 6);
STP stlUnompressZlib(STP src);

