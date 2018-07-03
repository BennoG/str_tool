/* ---------------------------------------------------------------------*
*                    Advanced Network Services v.o.f.,                  *
*              Copyright (c) 2002-2017 All Rights reserved              *
*                                                                       *
* This Library is licensed under the GNU general public license (GPL).  *
* As such, all software using it must also be released as open source   *
* under the same license. Read more about the GPL at www.gnu.org.       *
*                                                                       *
* With the alternative license agreement presented below it will        *
* be possible to use the library in your closed-source application.     *
*                                                                       *
* This file and its contents are protected by Dutch and                 *
* International copyright laws.  Unauthorized reproduction and/or       *
* distribution of all or any portion of the code contained herein       *
* is strictly prohibited and will result in severe civil and criminal   *
* penalties.  Any violations of this copyright will be prosecuted       *
* to the fullest extent possible under law.                             *
*                                                                       *
* THE SOURCE CODE CONTAINED HEREIN AND IN RELATED FILES IS PROVIDED     *
* TO THE REGISTERED DEVELOPER FOR THE PURPOSES OF EDUCATION AND         *
* TROUBLESHOOTING. UNDER NO CIRCUMSTANCES MAY ANY PORTION OF THE SOURCE *
* CODE BE DISTRIBUTED, DISCLOSED OR OTHERWISE MADE AVAILABLE TO ANY     *
* THIRD PARTY WITHOUT THE EXPRESS WRITTEN CONSENT OF THE OWNER          *
*                                                                       *
* THE REGISTERED DEVELOPER ACKNOWLEDGES THAT THIS SOURCE CODE           *
* CONTAINS VALUABLE AND PROPRIETARY TRADE SECRETS OF                    *
* ADVANCED NETWORK SERVICES THE REGISTERED DEVELOPER AGREES TO          *
* EXPEND EVERY EFFORT TO INSURE ITS CONFIDENTIALITY.                    *
*                                                                       *
* THE END USER LICENSE AGREEMENT (EULA) ACCOMPANYING THE PRODUCT        *
* PERMITS THE REGISTERED DEVELOPER TO REDISTRIBUTE THE PRODUCT IN       *
* EXECUTABLE FORM ONLY IN SUPPORT OF APPLICATIONS WRITTEN USING         *
* THE PRODUCT.  IT DOES NOT PROVIDE ANY RIGHTS REGARDING THE            *
* SOURCE CODE CONTAINED HEREIN.                                         *
*                                                                       *
* THIS COPYRIGHT NOTICE MAY NOT BE REMOVED FROM THIS FILE.              *
* --------------------------------------------------------------------- *
*/



#ifndef _STL_MODBUS_H_
#define _STL_MODBUS_H_

#include "stl_rs232.h"
#include "stl_str.h"
#include "stl_tcp.h"

namespace ansStl
{

	/* modbus device welke op poort 502 luistert
	*/
	class stlModbusDevTcp
	{
	private:
		int iTcpPort;
	public:
		stlModbusDevTcp(int iPort = 502);
		~stlModbusDevTcp();
	private:
		static void tcpServerEntry(struct stlTcpConInfo *CI);
		void tcpServer(struct stlTcpConInfo *CI);
	protected:
		virtual void onRxData(int iRegStart,STP sData) = 0;
		virtual STP  onRequestData(int iRegStart,int iRegCount) = 0;
	};


	class stlModbus
	{
	public:
//		enum ds301State {S_OFFLINE = 0, S_INIT_HW, S_READY};

		enum mbCmd {
			_modbusReadCoil				= 1,	// read output IO bits
			_modbusReadDiscreteInputs	= 2,	// read input IO bits
			_modbusReadHoldingRegister	= 3,	// NVRAM soort van scratch pad
			_modbusReadInputRegister		= 4,	// NVRAM ? communicatie met intelligente modules in woorden ? http://www.simplymodbus.ca/FC04.htm
			_modbusWriteSingleCoil		= 5,	// 
			_modbusWriteSingleRegister	= 6,	// NVRAM
			_modbusWriteMultipleCoils	= 15,	// deze is IO out bits
			_modbusWriteMultipleRegister	= 16,
			_modbusWriteMaskedRegister	= 22,	// WAGO only ?   (zie kopstation documentatie FC22)
			_modbusReadWriteRegisters	= 23	// WAGO only ? Reading and Writing several registers
		};

	public:
		stlModbus(){};
		virtual ~stlModbus(){};

		/* Read data from remote Modbus device
		**    iStart  Start adres from where to read
		**    iLeng   number of items to read. (bits, bytes, or words)
		**    iType   one of the modbusRead* types 
		*/
		virtual STP modbusReadData(int iStart,int iLeng,mbCmd iType,int iTimeOut=250) = 0;

		/* Write data to remote Modbus device
		**    iStart  Start adres from where to read
		**    sData   Data to write
		**    iType   one of the modbusRead* types 
		*/
		virtual int modbusWriteData(int iStart,STP sData,mbCmd iType,int iTimeOut=250) = 0;

		virtual bool isConnectd(){ return true; }

		int getDataWord(STP sData,int iIdx);
		int getDataWord(ansStl::cST &sData,int iIdx);

		int setDataWord(STP sData,int iIdx,int iVal);
		int setDataWord(ansStl::cST &sData,int iIdx,int iVal);

	};

	class stlModbusTcp: public stlModbus
	{
	private:
		void initVars(){ iPort = 502; con = NULL; iMsgIdx = 1000; }
		ansStl::cST IP;
		int iPort;
		stlTcpConn *con;
		stlMutex conMux;
		int iMsgIdx;
	public:
		stlModbusTcp(const char *IP,int iPort = 502);

		/* Read data from remote Modbus device
		**    iStart  Start adres from where to read
		**    iLeng   number of items to read. (bits, bytes, or words)
		**    iType   one of the modbusRead* types 
		*/
		virtual STP modbusReadData(int iStart,int iLeng,mbCmd iType,int iTimeOut=250);

		/* Write data to remote Modbus device
		**    iStart  Start adres from where to read
		**    sData   Data to write
		**    iType   one of the modbusRead* types 
		*/
		virtual int modbusWriteData(int iStart, STP sData, mbCmd iType, int iTimeOut = 250);
		virtual int modbusWriteData(int iStart, ansStl::cST &sData,mbCmd iType,int iTimeOut=250);
		virtual STP modbusReadWrite(int iRdStart, int iRdLen, int iWrStart, ansStl::cST &sData, int iTimeOut = 250);

		virtual bool isConnectd(){return (con != NULL);}
	private:
		int _testReconnect();

		/* Genereer header voor modbus bericht
		*/
		STP _generateReadHdr (int transID,int iStart,int iNumBytes,mbCmd iType);
		STP _generateWriteHdr(int transID,int iStart,int iNumItems,mbCmd iType);

		/* wacht op antwoord met timeout
		*/
		STP _waitReponce(int &iError,int iTimeOut=250);

		int _readDataBlock(ansStl::cST &data,int iTimeOut);
	};


/*
	class plcComm
	{
	private:
		void initVar(){com = NULL; comCleanup = false;}
	protected:
		bool comCleanup;
		ansStl::rs232 *com;
		ansStl::cST errCode;
	public:
		plcComm();
		virtual ~plcComm();
		virtual int open(const char *portName,int baud = 9600,int parity = 'n' ,int databits = 8,int stopbits = 1);
		virtual ansStl::cST readWord(char chType = 'D',int start = 0,int count = 1) = 0;
		virtual int writeWord(ansStl::cST data,char chType = 'D',int start = 0) = 0;
		virtual int writeWord(int dwValue,char chType = 'D',int start = 0);
		virtual int writeWord(int *dwValue,int count,char chType = 'D',int start = 0);
		virtual int writeWord(STP data,char chType = 'D',int start = 0);
		int lastError();
	protected:
		virtual int plcSendData(ansStl::cST &data);
		virtual ansStl::cST plcReadData(int timeoutMs = 200);
	};
	// modbus RTU is de rs232 binaire variant van modbus
	class modbusRtu : public plcComm
	{
	public:
		modbusRtu();

	protected:
		// Get LOW byte of int 
		unsigned char getLOWbyte(unsigned int input);
		// Get HIGH byte of int 
		unsigned char getHIGHbyte(unsigned int input);
		// Combine HIGH and LOW bytes 
		unsigned int combineBytes(unsigned char high, unsigned char low);
		// Generate CRC 
		unsigned int generateCRC(unsigned char * data, unsigned char length);
		// Check for exception 
		char checkException(unsigned char * data);
		// Answer to the Master device 
		void prepareOutputArray(unsigned char * data);

	};


	class plcFX3 : public plcComm
	{
	public:
		plcFX3();
		plcFX3(ansStl::rs232 *com);
		plcFX3(stlSerial *ser);
		plcFX3(const char *portName,int baud = 9600,int parity = 'n' ,int databits = 8,int stopbits = 1);
	public:
		virtual ansStl::cST readWord(char chType = 'D',int start = 0,int count = 1);
		virtual int writeWord(ansStl::cST data,char chType = 'D',int start = 0);
	};

	class plcQ : public plcComm
	{
	public:
		plcQ();
		plcQ(ansStl::rs232 *com);
		plcQ(stlSerial *ser);
		plcQ(const char *portName,int baud = 115200,int parity = 'n' ,int databits = 8,int stopbits = 1);
	public:
		virtual ansStl::cST readWord(char chType = 'D',int start = 0,int count = 1);
		virtual int writeWord(ansStl::cST data,char chType = 'D',int start = 0);
	};
	*/

};


#endif


