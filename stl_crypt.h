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
#ifndef _STL_CRYPT_H_
#define _STL_CRYPT_H_

#include "stl_str.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef struct{
	STP sRnd[4];
	int iOfs[4];
}STLCRYPT,*PSTLCRYPT;

typedef struct
{
	int iLen[4];
	unsigned int uiSeed1[4];
	unsigned int uiSeed2[4];
	unsigned int uiSeed3[4];
}STLCRYPTCONFIG,*PSTLCRYPTCONFIG;

/* Initialize encrypt data with configurable data default repetition after 6TB of data
**  cfg   configuration record (all values are optional even record may be NULL)
** Return Crypt struct for use with encryption functions
**        must be released with stlCryptRelease
*/
PSTLCRYPT stlCryptInitEx(PSTLCRYPTCONFIG cfg);
PSTLCRYPT stlCryptInit  (unsigned int uiSeed1,unsigned int uiSeed2,unsigned int uiSeed3);
PSTLCRYPT stlCryptInit2 (unsigned int uiSeed1,unsigned int uiSeed2,unsigned int uiSeed3,unsigned int uiSeed4);

/* Release crypt struct with was used to encrypt some data
*/
void stlCryptRelease(PSTLCRYPT cDAT);

/* Encrypt/decrypt a block of data with the encryption proces
**  pCRP   encryption data struct
**  pData  data to encrypt / decrypt
**  iLeng  number of bytes to encrypt / decrypt
*/
void stlCryptData(PSTLCRYPT pCRP,void *pData,int iLen);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* _STL_CRYPT_H_ */

