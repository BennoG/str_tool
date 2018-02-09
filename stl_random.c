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

#include "stl_str.h"

#define _RandCapital_	0x01	// A-Z
#define _RandLower_		0x02	// a-z
#define _RandNumber_	0x04	// 0-9
#define _RandPrint_		0x08	// char(32-127)
#define _RandNoZero_	0x10	// char(1-255)
#define _RandAllChar_	0x20	// char(0-255)

/* Generate reproducible random data
**  iLen    number of data bytes to generate
**  iMode   Type of data to generate (flags can be or'd together)
**  uiSeed1 random seed number 1
**  uiSeed2 random seed number 2
**  uiSeed3 random seed number 3
** Return Resulting random data
*/
STP stlRandom(int iLen,int iMode,unsigned int uiSeed1,unsigned int uiSeed2,unsigned int uiSeed3)
{
	int i,iOfs=0;
	unsigned int uiR1,uiR2,uiR3;
	STP sRes;
	sRes=stlInitLen(iLen,0);
	if ((iMode & 0x3F)==0) iMode=_RandCapital_;

	while (1)
	{
		// Random nummer 1
		uiSeed1 = uiSeed1 * 1103515245L + 12345;
		uiR1 =(uiSeed1/65536L)%2048;
		uiR1<<=11;
		uiSeed1 = uiSeed1 * 1103515245L + 12345;
		uiR1^=(uiSeed1/65536L)%2048;
		uiR1<<=10;
		uiSeed1 = uiSeed1 * 1103515245L + 12345;
		uiR1^=(uiSeed1/65536L)%1024;

		uiSeed2 = uiSeed2 * 1103515245L + 12345;
		uiR2 =(uiSeed2/65536L)%2048;
		uiR2<<=11;
		uiSeed2 = uiSeed2 * 1103515245L + 12345;
		uiR2^=(uiSeed2/65536L)%2048;
		uiR2<<=10;
		uiSeed2 = uiSeed2 * 1103515245L + 12345;
		uiR2^=(uiSeed2/65536L)%1024;

		uiSeed3 = uiSeed3 * 1103515245L + 12345;
		uiR3 =(uiSeed3/65536L)%2048;
		uiR3<<=11;
		uiSeed3 = uiSeed3 * 1103515245L + 12345;
		uiR3^=(uiSeed3/65536L)%2048;
		uiR3<<=10;
		uiSeed3 = uiSeed3 * 1103515245L + 12345;
		uiR3^=(uiSeed3/65536L)%1024;

		uiR1 += uiR2;
		uiR1 += uiR3;

		for (i=0;i<4;i++)
		{
			if (iOfs>=sRes->iLen) return sRes;
			uiR2=uiR1 & 0xFF;
			uiR1>>=8;
			// 'A' - 'Z'
			if ((iMode & _RandCapital_)&&(uiR2>='A')&&(uiR2<='Z'))
			{
				sRes->sBuf[iOfs++]=uiR2; 
				continue;
			}
			// 'a' - 'z'
			if ((iMode & _RandLower_)&&(uiR2>='a')&&(uiR2<='z'))
			{
				sRes->sBuf[iOfs++]=uiR2; 
				continue;
			}
			// '0' - '9'
			if ((iMode & _RandNumber_)&&(uiR2>='0')&&(uiR2<='9'))
			{
				sRes->sBuf[iOfs++]=uiR2; 
				continue;
			}
			// char(32-127)
			if ((iMode & _RandPrint_)&&(uiR2>=32)&&(uiR2<=127))
			{
				sRes->sBuf[iOfs++]=uiR2; 
				continue;
			}
			// char(1-255)
			if ((iMode & _RandNoZero_)&&(uiR2>=1)&&(uiR2<=255))
			{
				sRes->sBuf[iOfs++]=uiR2; 
				continue;
			}
			// 0 - 255
			if (iMode & _RandAllChar_){
				sRes->sBuf[iOfs++]=uiR2; 
				continue;
			}
		}
	}
	return sRes;
}

