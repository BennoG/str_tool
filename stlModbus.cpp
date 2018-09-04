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


#include "stlModbus.h"
#include <stdlib.h>

using namespace ansStl;

static int _getDataWord(STP sData,int iIdx)
{
	int iAns=0;
	iIdx = iIdx * 2;
	if (sData==NULL) return -1;
	if (iIdx < 0) return -2;
	if (iIdx + 1 >= sData->iLen) return -3;
	iAns = sData->sBuf[iIdx+0] & 0xFF; iAns<<=8;
	iAns|= sData->sBuf[iIdx+1] & 0xFF;
	return iAns;
}
static int _getDataWord(ansStl::cST &sData,int iIdx)
{
	int iAns=0;
	iIdx = iIdx * 2;
	if (iIdx < 0) return -2;
	if (iIdx + 1 >= sData.length()) return -3;
	iAns = sData[iIdx+0] & 0xFF; iAns<<=8;
	iAns|= sData[iIdx+1] & 0xFF;
	return iAns;
}
static int _setDataWord(STP sData,int iIdx,int iVal)
{
	iIdx = iIdx * 2;
	if (sData==NULL) return -1;
	if (iIdx < 0) return -2;
	if (iIdx + 1 >= sData->iLen) return -3;
	sData->sBuf[iIdx+0] = (iVal >> 8)&0xFF;
	sData->sBuf[iIdx+1] = (iVal >> 0)&0xFF;
	return 0;
}
static int _setDataWord(ansStl::cST &sData,int iIdx,int iVal)
{
	iIdx = iIdx * 2;
	if (iIdx < 0) return -2;
	if (iIdx + 1 >= sData.length()) return -3;
	sData[iIdx+0] = (iVal >> 8)&0xFF;
	sData[iIdx+1] = (iVal >> 0)&0xFF;
	return 0;
}



int stlModbus::getDataWord(STP sData,int iIdx)
{
	return _getDataWord(sData,iIdx);
}
int stlModbus::getDataWord(ansStl::cST &sData,int iIdx)
{
	return _getDataWord(sData,iIdx);
}
int stlModbus::setDataWord(STP sData,int iIdx,int iVal)
{
	return _setDataWord(sData,iIdx,iVal);
}
int stlModbus::setDataWord(ansStl::cST &sData,int iIdx,int iVal)
{
	return _setDataWord(sData,iIdx,iVal);
}




/************************************************************************/
/*                                                                      */
/************************************************************************/

stlModbusTcp::stlModbusTcp(const char *sIP,int iTcpPort /* = 502 */)
{
	initVars();
	stlMutexInit(&conMux);
	IP    = sIP;
	iPort = iTcpPort;
	_testReconnect();
}

/* Read data from remote Modbus device
**    iStart  Start adres from where to read
**    iLeng   number of items to read. (bits, bytes, or words)
**    iType   one of the modbusRead* types 
*/
STP stlModbusTcp::modbusReadData(int iStart,int iLeng,mbCmd iType,int iTimeOut /* = 250 */)
{
	if (_testReconnect() < 0) return NULL;
	
	stlMutexLock(&conMux);
	iMsgIdx = ((iMsgIdx + 1) % 1000) + 1000;
	STP sHdr = _generateReadHdr(iMsgIdx,iStart,iLeng,iType);
	int iRes = stlTcpWrite(con,(unsigned char *)sHdr->sBuf,sHdr->iLen,iTimeOut);
	stlFree(sHdr);
	if (iRes < 0) {
		printf("stlModbusTcp::modbusReadData %s:%d timeout",IP.buf(),iPort);
		stlTcpRelease(con); 
		con = NULL; 
		stlMutexUnlock(&conMux); 
		return NULL; 
	}
	int iErrCode = 0;
	STP sRes = _waitReponce(iErrCode,iTimeOut);
	stlMutexUnlock(&conMux);
	return sRes;
}

STP stlModbusTcp::modbusReadWrite(int iRdStart, int iRdLen, int iWrStart, ansStl::cST &sData, int iTimeOut /* = 250 */)
{
	if (_testReconnect() < 0) return NULL;

	stlMutexLock(&conMux);
	iMsgIdx = ((iMsgIdx + 1) % 1000) + 1000;
	STP sHdr = _generateReadHdr(iMsgIdx, iRdStart, iRdLen, _modbusReadWriteRegisters);
	int iWrLen = sData.length() / 2;
	stlAppendCh(sHdr, (iWrStart >> 8) & 0xFF);
	stlAppendCh(sHdr, (iWrStart >> 0) & 0xFF);
	stlAppendCh(sHdr, (iWrLen >> 8) & 0xFF);
	stlAppendCh(sHdr, (iWrLen >> 0) & 0xFF);
	iWrLen = iWrLen * 2;
	stlAppendCh(sHdr, iWrLen);
	for (int i = 0; i < iWrLen; i++)
		stlAppendCh(sHdr, sData[i]);

	sHdr->sBuf[5] = sHdr->iLen - 6;
	int iRes = stlTcpWrite(con, (unsigned char *)sHdr->sBuf, sHdr->iLen, iTimeOut);
	stlFree(sHdr);
	if (iRes < 0) {
		printf("stlModbusTcp::modbusReadWriteData %s:%d timeout", IP.buf(), iPort);
		stlTcpRelease(con);
		con = NULL;
		stlMutexUnlock(&conMux);
		return NULL;
	}
	int iErrCode = 0;
	STP sRes = _waitReponce(iErrCode, iTimeOut);
	stlMutexUnlock(&conMux);
	return sRes;
}


/* Write data to remote Modbus device
**    iStart  Start adres from where to read
**    sData   Data to write
**    iType   one of the modbusRead* types 
*/
int stlModbusTcp::modbusWriteData(int iStart,STP sData,mbCmd iType,int iTimeOut /* =250 */)
{
	if (_testReconnect() < 0) return -1;
	if ((sData) && (sData->iLen & 1)) return -2;		// altijd even aantal bytes

	stlMutexLock(&conMux);
	iMsgIdx = ((iMsgIdx + 1) % 1000) + 1000;

	int iLeng = sData->iLen / 2;						// aantal woorden dat we gaan schrijven
	if ((iType == _modbusWriteSingleCoil)||(iType == _modbusWriteMultipleCoils)) iLeng=sData->iLen * 8;
	STP sHdr = _generateWriteHdr(iMsgIdx,iStart,iLeng,iType);
	if (sData) stlAppendStp(sHdr,sData);
	int iRes = stlTcpWrite(con,(unsigned char *)sHdr->sBuf,sHdr->iLen,iTimeOut);
	stlFree(sHdr);

	if (iRes < 0) {
		printf("stlModbusTcp::modbusWriteData %s:%d timeout %d",IP.buf(),iPort,iRes);
		stlTcpRelease(con); 
		con = NULL; 
		stlMutexUnlock(&conMux); 
		return -2; 
	}
	int iErrCode = 0;
	STP sRes = _waitReponce(iErrCode,iTimeOut);
	stlMutexUnlock(&conMux);
	stlFree(sRes);
	return 0;
}

int stlModbusTcp::modbusWriteData(int iStart, ansStl::cST &sData, mbCmd iType, int iTimeOut/* =250 */)
{
	STP da = sData.getStp();
	int iRes = modbusWriteData(iStart, da, iType, iTimeOut);
	stlFree(da);
	return iRes;
}


/************************************************************************/
/*                                                                      */
/************************************************************************/

int stlModbusTcp::_readDataBlock(ansStl::cST &data,int iTimeOut)
{
	int iRdLen = 0;
	int iRes = stlTcpReadMax(con,(char*)data.buf(),data.length(),iTimeOut,10,NULL,NULL,&iRdLen);
	if (iRes < 0)
	{
		printf("stlModbusTcp::_readDataBlock %s:%d timeout %d",IP.buf(),iPort,iRes);
		stlTcpRelease(con); 
		con = NULL; 
		return iRes;
	}
	if (iRdLen != data.length())
	{
		printf("stlModbusTcp::_waitReponce %s:%d header lengt error",IP.buf(),iPort);
		stlTcpRelease(con); 
		con = NULL; 
		return -21; 
	}
	return 0;
}


STP stlModbusTcp::_waitReponce(int &iError,int iTimeOut /* = 250 */)
{
	// eerst de header lezen van TCP bus
	cST sHdr(7,(char*)NULL);
	iError = _readDataBlock(sHdr,iTimeOut);
	if (iError < 0) return NULL;

	int iDaLen = getDataWord(sHdr,2);
	cST sData(iDaLen - 1,(char *)NULL);
	iError = _readDataBlock(sData,iTimeOut);
	if (iError == -21)
	{
		printf("stlModbusTcp::_waitReponce exeption %d function 0x%X",sData[1],sData[0]);
		return NULL;
	}

	if (iError < 0) return NULL;

	int iDaIdx = getDataWord(sHdr,0);
	if (iDaIdx != iMsgIdx) return _waitReponce(iError,iTimeOut);		// verkeerde volgnummer probeer volgende datagram
	if (sData[0] == _modbusReadWriteRegisters ) sData.insDel(0, -2);
	if (sData[0] == _modbusReadInputRegister  ) sData.insDel(0, -2);		// 1e 2 bytes weg halen
	if (sData[0] == _modbusWriteSingleRegister) sData.insDel(0, -3);		// 1e 3 bytes weg halen
	return sData.getStp();
}


int stlModbusTcp::_testReconnect()
{
	if (con) return 0;
	if (IP.length() < 3) return -1;
	con = stlTcpInit(stlTcpDnsConnectTout(IP,iPort,1000));
	if (con) {
		printf("stlModbusTcp::modbusConnect %s:%d ok", IP.buf(), iPort);
		return 0;
	}
	printf("stlModbusTcp::modbusConnect %s:%d error", IP.buf(), iPort);
	return -2;
}


/* Genereer header voor modbus bericht
*/
STP stlModbusTcp::_generateReadHdr(int transID,int iStart,int iLeng,mbCmd iType)
{
	STP sHDR;
	sHDR=stlInitLen(12,0);
	sHDR->sBuf[0]  = (transID>>8) & 0xFF;	// transaction id
	sHDR->sBuf[1]  = (transID>>0) & 0xFF;	// transaction id
	sHDR->sBuf[5]  = 6;						// Message size
	sHDR->sBuf[6]  = 0;						// Slave address
	sHDR->sBuf[7]  = iType;					// Function code
	sHDR->sBuf[8]  = (iStart>>8)&0xFF;		// Start address
	sHDR->sBuf[9]  = (iStart>>0)&0xFF;		// Start address
	sHDR->sBuf[10] = (iLeng>>8)&0xFF;		// Number of data to read
	sHDR->sBuf[11] = (iLeng>>0)&0xFF;		// Number of data to read
	return sHDR;
}

STP stlModbusTcp::_generateWriteHdr(int transID,int iStart,int iNumItems,mbCmd iType)
{
	int iNumBytes = (iNumItems+7)/8;
	STP sHDR;
	if (iType >= _modbusWriteMultipleCoils)
		sHDR = stlInitLen(13,0);
	else 
		sHDR = stlInitLen(10,0);

	if (iType == _modbusWriteMultipleRegister) iNumBytes = iNumItems * 2;	// dit zijn woorden

	sHDR->sBuf[0] = (transID>>8) & 0xFF;		// transaction id
	sHDR->sBuf[1] = (transID>>0) & 0xFF;		// transaction id
	sHDR->sBuf[4] = ((iNumBytes+7)>>8) & 0xFF;	// Complete message size in bytes
	sHDR->sBuf[5] = ((iNumBytes+7)>>0) & 0xFF;	// Complete message size in bytes
	// vanaf hier gelijk aan rs232
	sHDR->sBuf[6] = 0;							// Slave address
	sHDR->sBuf[7] = iType;						// Function code
	sHDR->sBuf[8] = (iStart>>8)&0xFF;			// Start address
	sHDR->sBuf[9] = (iStart>>0)&0xFF;			// Start address
	if (iType >= _modbusWriteMultipleCoils)
	{
		sHDR->sBuf[10] = (iNumItems>>8)&0xFF;	// Number of data to write
		sHDR->sBuf[11] = (iNumItems>>0)&0xFF;	// Number of data to write
		sHDR->sBuf[12] = iNumBytes;
	}
	return sHDR;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

stlModbusDevTcp::stlModbusDevTcp(int iPort /* = 502 */)
{
	iTcpPort = iPort;
	stlTcpAddServiceEx(iTcpPort,this,tcpServerEntry,_stlTcpSrvReuseSock_);
}
stlModbusDevTcp::~stlModbusDevTcp()
{
	stlTcpRemoveService(iTcpPort);
}

void stlModbusDevTcp::tcpServerEntry(struct stlTcpConInfo *CI)
{
	stlModbusDevTcp *p = NULL;
	if (CI) p = static_cast<stlModbusDevTcp*>(CI->pConfig);
	if (p) p->tcpServer(CI);
	if (CI) free(CI);
}

// 0  1  2  3  4  5  6  7  8  9 10 11
//C3 88 00 00 00 06 FF 03 00 07 00 01 
//                              +++++---length
//                        +++++---------start adres
//                     ++---------------message type
//                  ++------------------slave adres
//            +++++--------------------- packet lengt
//+++++--------------------------------- packet seqnr


void stlModbusDevTcp::tcpServer(struct stlTcpConInfo *CI)
{
	STP sMsgHdr = stlInitLen(12,0);
	stlTcpConn *con = stlTcpInit(CI->sockFd);
	while (true)
	{
		int iRxLen = 0;
		int iRes = stlTcpReadMax(con,sMsgHdr->sBuf,sMsgHdr->iLen,-1,100,NULL,NULL,&iRxLen);
		if (iRes < 0) break;
		if (iRxLen != sMsgHdr->iLen) break;
		//int iPktLen = _getDataWord(sMsgHdr,2);
		int iRegNr  = _getDataWord(sMsgHdr,4);
		int iRegCnt = _getDataWord(sMsgHdr,5);
		int iCmdNr  = sMsgHdr->sBuf[7] & 0xFF;

		STP sMsgAns  = NULL;
		STP sMsgData = NULL;
		switch (iCmdNr)
		{
		case 0x03:		// Read Multiple Registers
		case 0x04:		// Read Input Registers
			sMsgData = onRequestData(iRegNr,iRegCnt);
			break;
		case 0x06:		// Write Single Register
			{
				STP sVal = stlSetSt("  ");
				_setDataWord(sVal,0,iRegCnt);
				onRxData(iRegNr,sVal);
				sMsgAns = stlCopy(sMsgHdr);
				stlFree(sVal);
			}
			break;
		case 0x10:		// Write Multiple Registers
			{
				iRxLen = 0;
				int iBytes = stlTcpGetChar(con,250);
				STP sVal = stlInitLen(iBytes,0);
				stlTcpReadMax(con,sVal->sBuf,sVal->iLen,250,100,NULL,NULL,&iRxLen);
				if (iRxLen == iBytes)
				{
					onRxData(iRegNr,sVal);
					sMsgAns = stlCopy(sMsgHdr);
				}
				stlFree(sVal);
			}
			break;
		}
		if ((sMsgData) && (sMsgAns == NULL))
		{
			sMsgAns = stlCopy(sMsgHdr);
			sMsgAns->sBuf[8] = sMsgData->iLen;
			sMsgAns->iLen = 9;
			stlAppendStp(sMsgAns,sMsgData);
		}
		if (sMsgAns == NULL)
		{
			sMsgAns = stlCopy(sMsgHdr);
			sMsgAns->sBuf[7] |= 0x80;		// error frame
			sMsgAns->sBuf[8] =  0x01;		// illegal function
			sMsgAns->iLen = 9;
		}
		_setDataWord(sMsgAns,2,sMsgAns->iLen - 6);		// lengte in header zetten.
		stlTcpWrite(con,(unsigned char*)(sMsgAns->sBuf),sMsgAns->iLen,250);
		stlFree(sMsgAns,sMsgData);
	}
	stlTcpRelease(con);
	stlFree(sMsgHdr);
}



#if 0

#define STX		0x02	// start of text
#define ETX		0x03	// End of text
#define EOT     0x04	// End of transmission
#define ENQ     ((char)0x05)	// Enquiry
#define ACK		0x06	// Acknowledge
#define NAK		0x15	// Negative Acknowledge

plcComm::plcComm()
{
	initVar();
}

plcComm::~plcComm()
{
	if ((comCleanup) && (com)) delete com;
	initVar();
}

int plcComm::open(const char *portName,int baud /* = 9600 */,int parity /* = 'n'  */,int databits /* = 8 */,int stopbits /* = 1 */)
{
	ansStl::rs232 *oldCom = com;
	if (com) new ansStl::rs232(portName,baud,parity,databits,stopbits);
	if (oldCom){stlMsWait(100); delete oldCom; }
	if (com) return 0;
	return -1;
}
int plcComm::writeWord(int dwValue,char chType /* = 'D' */,int start /* = 0 */)
{
	cST data;
	data.append("%04X",dwValue & 0xFFFF);
	return writeWord(data,chType,start);
}
int plcComm::writeWord(int *dwValue,int count,char chType /* = 'D' */,int start /* = 0 */)
{
	cST data;
	for (int i = 0; i < count; i++)
		data.append("%04X",dwValue[i] & 0xFFFF);
	return writeWord(data,chType,start);
}
int plcComm::writeWord(STP data,char chType /* = 'D' */,int start /* = 0 */)
{
	cST nData = data;
	return writeWord(nData,chType,start);
}


/* low level zend data naar PLC (word wel checksum aan toegevoegd)
** Return 0 Ok
**       -1 invallid communication data
**       -2 write error
*/
int plcComm::plcSendData(ansStl::cST &data)
{
	if (com == NULL) return -1;
	cST sCmd;
	sCmd.append(ENQ);
	sCmd.append(data);
	int iSum = 0, iLen = sCmd.length();
	for (int i = 1; i < iLen; i++)
		iSum += sCmd[i] & 0xFF;
	sCmd.append("%02X",iSum & 0xFF);
	while (com->getc(0) >= 0);					// ontvangst buffer leeg maken voor verzenden
	int iRes = com->write(sCmd);
	if (iRes < 0) return iRes;
	return 0;
}

ansStl::cST plcComm::plcReadData(int timeoutMs /* = 200 */)
{
	cST sAns;
	errCode.set("");		// reset error code
	if (com == NULL) return sAns;

	int iSum=0;
	while (1)
	{
		int ch = com->getc(timeoutMs);
		if (ch == STX)
		{
			sAns.set("");
			iSum = 0;
			continue;
		}
		if ((ch==ACK)||(ch==NAK))
		{
			while ((ch = com->getc(5)) >= 0)
				errCode.append((char) ch);
			sAns = (ch == ACK) ? "0" : "1";
			break;
		}
		if (ch==ETX)
		{
			// End of transmission read checksum data bytes
			iSum += ch;
			ch = com->getc(5);
			int iSum2 = (ch > '9') ? (ch+9)&0xF : ch&0xF; 
			iSum2<<=4;
			ch = com->getc(5);
			iSum2|= (ch > '9') ? (ch+9)&0xF : ch&0xF;
			if (ch>=0)
			{
				if ((iSum ^ iSum2)&0xFF)
				{
					// checksum error
					printf("RS232 RX checksum error %02X %02X\n",iSum,iSum2);
					sAns = "";
					break;
				}
				break;
			}
			// hier hoeft geen break want ch<0 geeft hieronder dan automatisch een timeout
		}
		if (ch<0)	// timeout
		{
			sAns = "";
			printf("RS232 RX timeout error\n");
			break;
		}
		iSum += ch;
		sAns.append((char)ch);
	}
	/*
	if (iQPlc)
	{
		if ((sAns)&&(sAns->iLen>_Q_HDR_LEN_)) stlInsDel(sAns,0,-_Q_HDR_LEN_);
	}else
	{
		if ((sAns)&&(sAns->iLen>_FX3_HDR_LEN_)) stlInsDel(sAns,0,-_FX3_HDR_LEN_);
	}
	*/
	return sAns;

}
/************************************************************************/
/*                                                                      */
/************************************************************************/
/*
    DATA 01 03 
	[0]   device ID
	[1]   function code
	[2]   number of data bytes
	[3..x]  data bytes
	[x+1] CRC LOW
	[x+2] CRC HIGH
*/


modbusRtu::modbusRtu()
{

}

/* Get LOW byte of int */
unsigned char modbusRtu::getLOWbyte(unsigned int input)
{
	return input & 255;
}

/* Get HIGH byte of int */
unsigned char modbusRtu::getHIGHbyte(unsigned int input)
{
	return (input >> 8) & 255;
}

/* Combine HIGH and LOW bytes */
unsigned int modbusRtu::combineBytes(unsigned char high, unsigned char low)
{
	return (int) (high << 8) + low;
}

/* CRC algorithm */
static unsigned int CRC16(unsigned int crc, unsigned int data)
{
	crc = ((crc ^ data) | 0xFF00) & (crc | 0x00FF);
	for (int i = 0; i < 8; i++) 
	{
		bool LSB = ((crc & 0x0001) != 0);
		crc >>= 1;
		if (LSB) crc ^= 0xA001;
	}
	return(crc);
}

/* Generate CRC */
unsigned int modbusRtu::generateCRC(unsigned char * data, unsigned char length)
{
	unsigned int crc = 0xFFFF;
	for (int i = 0; i < length; i++)
		crc = CRC16 (crc, data[i] );
	return crc;
}


/* Check for exception */
char modbusRtu::checkException(unsigned char * data)
{
	/* Start register value */
	unsigned int start_register = combineBytes(data[2], data[3]);
	/* Quantity of registers value */
	unsigned int quantity = combineBytes(data[4], data[5]);
	/* Selection by function code */
	switch(data[1])
	{
		/**** Function Code 03 - Read Holding Registers ****/
	case 3:
		if(start_register + quantity - 1 < 20)
		{
			return 0x00;
		}
		else
		{
			/* Illegal Data Address */
			return 0x02;
		}
		break;
	}
	return 0;
}

/* Answer to the Master device */
void modbusRtu::prepareOutputArray(unsigned char * data)
{
	unsigned short REG[50];
	memset(REG,0,sizeof(REG));
	unsigned char tx_buffer[100];
	memset(tx_buffer,0,sizeof(tx_buffer));

	/* Check for exception */
	char exception = checkException(data);

	/* Temporary counters */
	int temp_cnt_bits = 0;
	int temp_cnt_reg = 0;

	/* Check if exception exist */
	if(exception == 0x00)
	{
		/* Exception doesn't exist */
		switch(data[1])
		{
			/**** Function Code 03 - Read Holding Registers ****/
		case 3:
			tx_buffer[0] = data[0];
			tx_buffer[1] = data[1];
			/* Number of output registers related to requested registers */
			tx_buffer[2] = data[5] * 2;
			/* Temp register */
			temp_cnt_bits = 0;
			temp_cnt_reg = data[3];
			/* Populate register */
			for(int k = 0; k < data[5]; k++)
			{
				tx_buffer[3 + temp_cnt_bits] = getHIGHbyte(REG[data[3] + k]);
				temp_cnt_bits++;
				tx_buffer[3 + temp_cnt_bits] = getLOWbyte(REG[data[3] + k]);
				temp_cnt_bits++;

			}
			/* Prepare CRC at the end */
			tx_buffer[tx_buffer[2] + 3] = getLOWbyte(generateCRC(tx_buffer, tx_buffer[2] + 3));
			tx_buffer[tx_buffer[2] + 4] = getHIGHbyte(generateCRC(tx_buffer, tx_buffer[2] + 3));

			/* Send array to output */
			if (com) com->write(tx_buffer,tx_buffer[2] + 5);
			/*
			for(int i = 0; i < tx_buffer[2] + 5; i++)
			{
				// Custom function for writing a byte to the output
				SendByteToOutput(tx_buffer[i]);
			}
			*/
			break;
		}
	}
	else
	{
		/* Exception exist */
		tx_buffer[0] = (data[1] + 0x80);
		tx_buffer[1] = exception;
		tx_buffer[2] = getLOWbyte(generateCRC(tx_buffer, 2));
		tx_buffer[3] = getHIGHbyte(generateCRC(tx_buffer, 2));

		if (com) com->write(tx_buffer,4);
		/*
		for(int i = 0; i < 4; i++)
		{
			// Custom function for writing a byte to the output
			SendByteToOutput(tx_buffer[i]);
		}
		*/
	}	
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

#define _FX3_HDR_LEN_	4		// number of bytes in _FX3_HDR_
#define _FX3_HDR_     "00FF"

plcFX3::plcFX3(rs232 *com)
{
	this->com = com;
}
plcFX3::plcFX3(stlSerial *ser)
{
	this->com = new rs232(ser);
	comCleanup = true;
}
plcFX3::plcFX3(const char *portName,int baud /* = 9600 */,int parity /* = 'n'  */,int databits /* = 8 */,int stopbits /* = 1 */)
{
	this->com = new rs232(portName,baud,parity,databits,stopbits);
	comCleanup = true;
}

ansStl::cST plcFX3::readWord(char chType /* = 'D' */,int start /* = 0 */,int count /* = 1 */)
{
	cST cmd,res;
	cmd.setf(_FX3_HDR_ "WR0%c%04d%02X",chType,start&0x7FFF,count & 0x7F);
	if (plcSendData(cmd) == 0)						// enkel als data goed is verzonden
		res = plcReadData();
	if (res.length() > _FX3_HDR_LEN_) res.insDel(0,-_FX3_HDR_LEN_);		// header weg halen.
	if (res.length() != count * 4) res = "";		// verkeerde lengte data reset
	return res;
}
int plcFX3::writeWord(ansStl::cST data,char chType /* = 'D' */,int start /* = 0 */)
{
	cST cmd,res;
	int count = data.length() / 4;
	cmd.setf(_FX3_HDR_ "WW0%c%04d%02X",chType,start&0x7FFF,count & 0x7F);
	cmd.append(data);
	if (plcSendData(cmd) == 0)						// enkel als data goed is verzonden
		res = plcReadData();
	if (res.length() > _FX3_HDR_LEN_) res.insDel(0,-_FX3_HDR_LEN_);		// header weg halen.
	if (res.length() > 0) return (res[0] & 1);
	return -1;
}
/************************************************************************/
/*                                                                      */
/************************************************************************/

#define _Q_HDR_LEN_		10		// number of bytes in _Q_HDR_
#define _Q_HDR_	"F90000FF00"
#define _Q_CMD_READ_  "0401"
#define _Q_CMD_WRITE_ "1401"
#define _Q_UNIT_WORD_ "0000"
#define _Q_UNIT_BIT_  "0001"

static const char *getQType(char cType)
{
	if ((cType=='d')||(cType=='D')) return "D*";
	if ((cType=='w')||(cType=='W')) return "W*";
	if ((cType=='r')||(cType=='R')) return "R*";
	if ((cType=='x')||(cType=='X')) return "X*";
	if ((cType=='y')||(cType=='Y')) return "Y*";
	if ((cType=='m')||(cType=='M')) return "M*";
	return "D*";
}

plcQ::plcQ(rs232 *com)
{
	this->com = com;
}
plcQ::plcQ(stlSerial *ser)
{
	this->com = new rs232(ser);
	comCleanup = true;
}
plcQ::plcQ(const char *portName,int baud /* = 115200 */,int parity /* = 'n'  */,int databits /* = 8 */,int stopbits /* = 1 */)
{
	this->com = new rs232(portName,baud,parity,databits,stopbits);
	comCleanup = true;
}

ansStl::cST plcQ::readWord(char chType /* = 'D' */,int start /* = 0 */,int count /* = 1 */)
{
	cST cmd,res;
	cmd.setf(_Q_HDR_ _Q_CMD_READ_ _Q_UNIT_WORD_ "%s%06d%04X",getQType(chType),start&0x7FFFFF,count & 0x7FFF);
	if (plcSendData(cmd) == 0)						// enkel als data goed is verzonden
		res = plcReadData();
	if (res.length() > _FX3_HDR_LEN_) res.insDel(0,-_FX3_HDR_LEN_);		// header weg halen.
	if (res.length() != count * 4) res = "";		// verkeerde lengte data reset
	return res;
}
int plcQ::writeWord(ansStl::cST data,char chType /* = 'D' */,int start /* = 0 */)
{
	cST cmd,res;
	int count = data.length() / 4;
	cmd.setf(_Q_HDR_ _Q_CMD_WRITE_ _Q_UNIT_WORD_ "%s%06d%04X",getQType(chType),start&0x7FFFFF,count & 0x7FFF);
	cmd.append(data);
	if (plcSendData(cmd) == 0)						// enkel als data goed is verzonden
		res = plcReadData();
	if (res.length() > _FX3_HDR_LEN_) res.insDel(0,-_FX3_HDR_LEN_);		// header weg halen.
	if (res.length() > 0) return (res[0] & 1);
	return -1;
}

#endif



