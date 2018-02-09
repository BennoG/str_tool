
#include "stlTftp.h"
#include "stl_tcp.h"

using namespace ansStl;

stlTftp::stlTftp(const char *host)
{
	initVars();
	if (host) sHost = host;
}
void stlTftp::mode(transMode newMode)
{
	iTransMode = newMode;
}

STP stlTftp::get(const char *fn)
{
	stlUdpConn *con = stlUdpConnect();
	cST file = "";
	cST pkt = "12";
	pkt[0] = 0;	pkt[1] = 1;		// set read request
	pkt.append(fn);
	pkt.append((char)0);
	if (iTransMode == TFTP_ascii)
		pkt.append("netascii");
	else
		pkt.append("octet");
	pkt.append((char)0);
	stlUdpSendExv(con,(char*)sHost.buf(),69,(void*)pkt.buf(),pkt.length());		// include trailing 0

	bool bNeedData = true;	// er komt nog meer data
	int  iPktWait  = 1;		// blok nummer waar we op wachten
	int  iToCnt    = 0;		// aantal timeouts
	while (bNeedData)
	{
		pkt.set(stlUdpRecv(con,1000,NULL),true);
		if (pkt.length() > 5)
		{
			int pktType = ((pkt[0] & 0xFF) << 8) | (pkt[1] & 0xFF);	// 1 = read file 3 = data 4 = ack
			int pktBlk  = ((pkt[2] & 0xFF) << 8) | (pkt[3] & 0xFF);	// 1 based block number
			//printf("Rx type(%d) blk(%d) wait(%d)",pktType,pktBlk,iPktWait);
			if (pktBlk != iPktWait) continue;						// niet verwacht data ignore
			iPktWait++;												// volgende blok
			if (pktType == 3)			// data type
			{
				pkt.insDel(0,-4);
				file.append(pkt);
				bNeedData = (pkt.length() == 512);
			}
			pkt = "1234";
			pkt[0] = 0;	pkt[1] = 4;		// ack
			pkt[2] = ((pktBlk >> 8) & 0xFF);
			pkt[3] = ((pktBlk >> 0) & 0xFF);
			stlUdpSendExv(con,con->sRemoteHost,con->uTxPort,(void *)pkt.buf(),pkt.length());
			iToCnt = 0;
		}else iToCnt++;
		if (iToCnt > 5) break;
	}
	stlUdpRelease(con);
	if (iToCnt) return NULL;
	return file.getStp();
}

int stlTftp::put(const char *fn,cST &data)
{
	STP sData = data.getStp();
	int iRes = put(fn,sData);
	stlFree(sData);
	return iRes;
}

int stlTftp::put(const char *fn,STP data)
{
	if ((data == NULL) || (data->iLen == 0)) return -2;
	stlUdpConn *con = stlUdpConnect();
	cST pktTx = "12";
	pktTx[0] = 0;	pktTx[1] = 2;		// set write request
	pktTx.append(fn);
	pktTx.append((char)0);
	if (iTransMode == TFTP_ascii)
		pktTx.append("netascii");
	else
		pktTx.append("octet");
	pktTx.append((char)0);
	stlUdpSendExv(con,(char*)sHost.buf(),69,(void*)pktTx.buf(),pktTx.length());	
	int iError = 0;		// error status.
	int iToCnt = 0;		// timeout counter
	int iPktWait = 0;	// ACK nummer waarop we wachten
	while (iError == 0)
	{
		cST pktRx(stlUdpRecv(con,1000,NULL),true);
		if (pktRx.length() >= 4)
		{
			iToCnt = 0;
			int pktType = ((pktRx[0] & 0xFF) << 8) | (pktRx[1] & 0xFF);	// 1 = read file 3 = data 4 = ack
			int pktBlk  = ((pktRx[2] & 0xFF) << 8) | (pktRx[3] & 0xFF);	// 1 based block number
			//printf("Rx type(%d) blk(%d) wait(%d)",pktType,pktBlk,iPktWait);
			if (pktType == 5){ iError = 1; break; }					// error van remote site
			if (pktBlk != iPktWait) continue;						// niet verwacht data ignore
			iPktWait++;												// volgende blok
			if (pktType == 4)					// ACK stuur volgende data blok
			{
				int iOfs = pktBlk * 512;
				if (iOfs > data->iLen) break;	// klaar met verzenden van data
				int iLen = data->iLen - iOfs;	// hoeveel bytes nog ?
				if (iLen > 512) iLen = 512;
				pktTx.set(iLen,data->sBuf + iOfs);
				pktTx.insDel(0,4);					// maak ruimte voor header
				pktTx[0] = 0;	pktTx[1] = 3;		// data
				pktTx[2] = ((iPktWait >> 8) & 0xFF);
				pktTx[3] = ((iPktWait >> 0) & 0xFF);
			}
		}else{
			iToCnt++;
			if (iToCnt > 5) break;
		}
		stlUdpSendExv(con,(char*)sHost.buf(),con->uTxPort ? con->uTxPort : 69,(void*)pktTx.buf(),pktTx.length());		// (re)transmit packet
	}
	if (iToCnt) return -3;
	if (iError) return -4;
	return 0;
}


