/* ---------------------------------------------------------------------*
*                    Advanced Network Services v.o.f.,                  *
*              Copyright (c) 2002-2011 All Rights reserved              *
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
#include <stdlib.h>
#include <string.h>

#include "stl_str.h"
#include "stl_crypt.h"

/* Initialize encrypt data with configurable data default repetition after 6TB of data
**  cfg   configuration record (all values are optional even record may be NULL)
** Return Crypt struct for use with encryption functions
**        must be released with stlCryptRelease
*/
PSTLCRYPT stlCryptInitEx(PSTLCRYPTCONFIG cfg)
{
	PSTLCRYPT cRES;
	STLCRYPTCONFIG cfgStat;
	int i;
	int iLen[4]={263,1279,3299,6011};	// repetition after 6670453122953 bytes = 6670453122 KB = 6670453 MB = 6670 GB
	unsigned int uiSeed1[4]={0x6E84D594,0xFA376C54,0x71864A67,0xE11C35E4};
	unsigned int uiSeed2[4]={0x21D1508E,0x1AC14358,0x26C578AB,0x2B90D2D7};
	unsigned int uiSeed3[4]={0x2B061893,0xB4E8840C,0x34726601,0x345AC345};
	memset(&cfgStat,0,sizeof(STLCRYPTCONFIG));
	if (cfg==NULL) cfg=&cfgStat;
	// initialize default values if non or invalid values are given
	for (i=0;i<4;i++)
	{
		if ((cfg->iLen[i]<=0)||(cfg->iLen[i]>=64000)) cfg->iLen[i]=iLen[i];
		if (cfg->uiSeed1[i]==0) cfg->uiSeed1[i]=uiSeed1[i];
		if (cfg->uiSeed2[i]==0) cfg->uiSeed2[i]=cfg->uiSeed1[i]^uiSeed2[i];
		if (cfg->uiSeed3[i]==0) cfg->uiSeed3[i]=cfg->uiSeed1[i]^uiSeed3[i];
	}
	// Now it is time to generate the random data
	cRES=malloc(sizeof(STLCRYPT));
	if (cRES==NULL) return NULL;
	memset(cRES,0,sizeof(STLCRYPT));
	for (i=0;i<4;i++)
		cRES->sRnd[i]=stlRandom(cfg->iLen[i],_RandNoZero_,cfg->uiSeed1[i],cfg->uiSeed2[i],cfg->uiSeed3[i]);
	return cRES;
}

PSTLCRYPT stlCryptInit(unsigned int uiSeed1,unsigned int uiSeed2,unsigned int uiSeed3)
{
	STLCRYPTCONFIG cfg;
	memset(&cfg,0,sizeof(cfg));
	cfg.uiSeed1[0]=uiSeed1;
	cfg.uiSeed2[0]=uiSeed2;
	cfg.uiSeed3[0]=uiSeed3;
	return stlCryptInitEx(&cfg);
}
PSTLCRYPT stlCryptInit2(unsigned int uiSeed1,unsigned int uiSeed2,unsigned int uiSeed3,unsigned int uiSeed4)
{
	STLCRYPTCONFIG cfg;
	unsigned int uall = (uiSeed1  & 0x1FFFFFFF) + (uiSeed2  & 0x1FFFFFFF) + (uiSeed3  & 0x1FFFFFFF) + (uiSeed4  & 0x1FFFFFFF);
	memset(&cfg,0,sizeof(cfg));

	cfg.uiSeed1[0]=uiSeed1 ^ uall;
	cfg.uiSeed1[1]=uiSeed2 ^ uall;
	cfg.uiSeed1[2]=uiSeed3 ^ uall;
	cfg.uiSeed1[3]=uiSeed4 ^ uall;

	cfg.uiSeed2[3]=uiSeed1 ^ 0x6E84D594;
	cfg.uiSeed2[0]=uiSeed2 ^ 0xFA376C54;
	cfg.uiSeed2[1]=uiSeed3 ^ 0x71864A67;
	cfg.uiSeed2[2]=uiSeed4 ^ 0xE11C35E4;

	cfg.uiSeed3[2]=uiSeed1 ^ 0x21D1508E;
	cfg.uiSeed3[3]=uiSeed2 ^ 0x1AC14358;
	cfg.uiSeed3[0]=uiSeed3 ^ 0x26C578AB;
	cfg.uiSeed3[1]=uiSeed4 ^ 0x2B90D2D7;

	return stlCryptInitEx(&cfg);
}



/* Release crypt struct with was used to encrypt some data
*/
void stlCryptRelease(PSTLCRYPT cDAT)
{
	int i;
	if (cDAT==NULL) return;
	for (i=0;i<4;i++) stlFree(cDAT->sRnd[i]);
	memset(cDAT,0,sizeof(STLCRYPT));
	free(cDAT);
}

/* Encrypt/decrypt a block of data with the encryption proces
**  pCRP   encryption data struct
**  pData  data to encrypt / decrypt
**  iLeng  number of bytes to encrypt / decrypt
*/
void stlCryptData(PSTLCRYPT pCRP,void *pData,int iLen)
{
	unsigned char *p,c;
	int i,j;
	if (pCRP==NULL) return;
	p=pData;
	for (j=0;j<iLen;j++)
	{
		c=*p;
		for (i=0;i<4;i++)
		{
			if (pCRP->sRnd[i])
			{
				c ^= pCRP->sRnd[i]->sBuf[pCRP->iOfs[i]++];
				if (pCRP->iOfs[i]>=pCRP->sRnd[i]->iLen) pCRP->iOfs[i]=0;
			}
		}
		*p++=c;
	}
}
