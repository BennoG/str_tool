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
#include <time.h>


#include "stl_mailUtil.h"
#include "stl_str.h"


#ifdef __linux__
static char *tzoffset(time_t *now)
/* calculate timezone offset */
{
    static char offset_string[6];
    struct tm gmt, *lt;
    int off;
    char sign = '+';
	
    gmt = *gmtime(now);
    lt = localtime(now);
    off = (lt->tm_hour - gmt.tm_hour) * 60 + lt->tm_min - gmt.tm_min;
    if (lt->tm_year < gmt.tm_year)
		off -= 24 * 60;
    else if (lt->tm_year > gmt.tm_year)
		off += 24 * 60;
    else if (lt->tm_yday < gmt.tm_yday)
		off -= 24 * 60;
    else if (lt->tm_yday > gmt.tm_yday)
		off += 24 * 60;
    if (off < 0) {
		sign = '-';
		off = -off;
    }
    if (off >= 24 * 60)			/* should be impossible */
		off = 23 * 60 + 59;		/* if not, insert silly value */
    sprintf(offset_string, "%c%02d%02d", sign, off / 60, off % 60);
    return (offset_string);
}
#endif




/* The two currently defined translation tables.  The first is the
standard uuencoding, the second is base64 encoding.  */
/*
static const char uu_std[64] =
{
	'`', '!', '"', '#', '$', '%', '&', '\'',
	'(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
	'X', 'Y', 'Z', '[', '\\', ']', '^', '_'
};
*/

static const char uu_base64[64] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};



int stlBase64Encode(STP sData)
{
	unsigned char *buf;
	int i,ch1,ch2,ch3,ch4;
	STP aRes;
	if (sData==NULL) return -1;
	aRes=stlInitLen((sData->iLen * 4 /3) + 10,0);
	if (aRes==NULL) return -2;
	aRes->iLen=0;

	buf=(unsigned char *)sData->sBuf;
	for (i=0;i<sData->iLen;i+=3){
		ch1=(buf[i]&0xFC)>>2;
		ch2=(buf[i]&0x03)<<4;
		if (i+1<sData->iLen){
			ch2|=(buf[i+1]&0xF0)>>4;
			ch3 =(buf[i+1]&0x0F)<<2;
		}else{
			ch3=-1;
		}
		if (i+2<sData->iLen){
			ch3|=(buf[i+2]&0xC0)>>6;
			ch4 =(buf[i+2]&0x3F)>>0;
		}else{
			ch4=-1;
		}
		if (ch3>=0) ch3=uu_base64[ch3&0x3F]; else ch3='=';
		if (ch4>=0) ch4=uu_base64[ch4&0x3F]; else ch4='=';
		aRes->sBuf[aRes->iLen++]=uu_base64[ch1&0x3F];
		aRes->sBuf[aRes->iLen++]=uu_base64[ch2&0x3F];
		aRes->sBuf[aRes->iLen++]=ch3;
		aRes->sBuf[aRes->iLen++]=ch4;
	}
	stlExchange(sData,aRes);
	stlFree(aRes);
	return 0;
}



int stlBase64Decode(STP sData)
{
	STP sRes;
	unsigned char *p;
	static const char b64_tab[256] ={
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*000-007*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*010-017*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*020-027*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*030-037*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*040-047*/
		'\177', '\177', '\177', '\76',  '\177', '\177', '\177', '\77',  /*050-057*/
		'\64',  '\65',  '\66',  '\67',  '\70',  '\71',  '\72',  '\73',  /*060-067*/
		'\74',  '\75',  '\177', '\177', '\177', '\100', '\177', '\177', /*070-077*/
		'\177', '\0',   '\1',   '\2',   '\3',   '\4',   '\5',   '\6',   /*100-107*/
		'\7',   '\10',  '\11',  '\12',  '\13',  '\14',  '\15',  '\16',  /*110-117*/
		'\17',  '\20',  '\21',  '\22',  '\23',  '\24',  '\25',  '\26',  /*120-127*/
		'\27',  '\30',  '\31',  '\177', '\177', '\177', '\177', '\177', /*130-137*/
		'\177', '\32',  '\33',  '\34',  '\35',  '\36',  '\37',  '\40',  /*140-147*/
		'\41',  '\42',  '\43',  '\44',  '\45',  '\46',  '\47',  '\50',  /*150-157*/
		'\51',  '\52',  '\53',  '\54',  '\55',  '\56',  '\57',  '\60',  /*160-167*/
		'\61',  '\62',  '\63',  '\177', '\177', '\177', '\177', '\177', /*170-177*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*200-207*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*210-217*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*220-227*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*230-237*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*240-247*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*250-257*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*260-267*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*270-277*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*300-307*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*310-317*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*320-327*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*330-337*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*340-347*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*350-357*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*360-367*/
		'\177', '\177', '\177', '\177', '\177', '\177', '\177', '\177', /*370-377*/
	};

	if (sData==NULL) return -1;
	sRes=stlInitLen(sData->iLen + 10,0);
	if (sRes==NULL) return -2;
	sRes->iLen=0;
	p=(unsigned char *)sData->sBuf;

	while (*p){
		char c1, c2, c3;
		while ((b64_tab[*p] & '\100') != 0)     // negeer alle chars niet in tabel
			if (*p == 0 || *p++ == '=')         // end of line.
				break;
		if (*p == 0) break;     /* This leaves the loop.  */

		c1 = b64_tab[*p++];

		while ((b64_tab[*p] & '\100') != 0)
		{
			if (*p == 0 || *p++ == '='){
				printf("error illegal base 64 line\n");
				stlExchange(sRes,sData);
				stlFree(sRes);
				return -3;
			}
		}

		c2 = b64_tab[*p++];
		while (b64_tab[*p] == '\177')
		{
			if (*p++ == 0){
				printf("error illegal base 64 line\n");
				stlExchange(sRes,sData);
				stlFree(sRes);
				return -4;
			}
		}

		if (*p == '='){
			stlAppendCh(sRes,(c1 << 2 | c2 >> 4));
			break;
		}

		c3 = b64_tab[*p++];
		while (b64_tab[*p] == '\177')
		{
			if (*p++ == 0){
				printf("error illegal base 64 line\n");
				stlExchange(sRes,sData);
				stlFree(sRes);
				return -5;
			}
		}
		stlAppendCh(sRes,(c1 << 2 | c2 >> 4));
		stlAppendCh(sRes,(c2 << 4 | c3 >> 2));
		if (*p == '=') break;
		stlAppendCh(sRes,(c3 << 6 | b64_tab[*p++]));

	}
	stlExchange(sRes,sData);
	return sData->iLen;
}

/* return a timestamp in RFC822 form */
STP stlRfc822timestamp(void)
{
	time_t      now;
	STP Tim;

	Tim=stlInitLen(80,0);
	if (Tim==NULL) return NULL;
	time(&now);
	/*
	* Conform to RFC822.  We generate a 4-digit year here, avoiding
	* Y2K hassles.  Max length of this timestamp in an English locale
	* should be 29 chars.  The only things that should vary by locale
	* are the day and month abbreviations.
	*/
	strftime(Tim->sBuf,Tim->iLen,"%a, %d %b %Y %H:%M:%S XXXXX (%Z)", localtime(&now));
#ifdef __linux__
	strncpy(strstr(Tim->sBuf,"XXXXX"), tzoffset(&now), 5);
#else
	strncpy(strstr(Tim->sBuf,"XXXXX"), "+0100", 5);
#endif
	Tim->iLen=(int)strlen(Tim->sBuf);
	return Tim;
}


