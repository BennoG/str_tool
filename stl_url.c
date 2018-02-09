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


STP stlUrlEncode(const char *src)
{
	STP sAns = NULL;
	if (src)
	{
		sAns = stlSetSt("");
		while (*src)
		{
			char ch = *src++;
			if (((ch>='a') && (ch <= 'z')) ||
				((ch>='A') && (ch <= 'Z')) ||
				((ch>='0') && (ch <= '9')) ){ stlAppendCh(sAns,ch); continue; }
			if ((ch == '-') || (ch == '_') || (ch == '.') || (ch == '~')) {stlAppendCh(sAns,ch); continue;}
			if (ch == ' ') {stlAppendCh(sAns,'+'); continue;}
			stlAppendStf(sAns,"%%%02X",ch & 0xFF);
		}
	}
	return sAns;
}

STP stlUrlDecode(const char *src)
{
	STP sAns = NULL;
	if (src)
	{
		sAns = stlSetSt("");
		while (*src)
		{
			char ch = *src++;
			if (ch == '+')
			{
				ch = ' ';
			}else if ((ch == '%') && (src[0]) && (src[1]))
			{
				ch = ((src[0] & 0xF) + ((src[0] > '9') ? 9 : 0)) << 4;
				ch |= (src[1] & 0xF) + ((src[1] > '9') ? 9 : 0);
				src+=2;
			}
			stlAppendCh(sAns,ch);
		}
	}
	return sAns;
}

