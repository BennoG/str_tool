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
#ifdef _WIN32

#include <Windows.h>
#include <conio.h>

#include "stl_str.h"
#include "stl_Cursor.h"

static HANDLE hConOut=INVALID_HANDLE_VALUE;
static HANDLE hConInp=INVALID_HANDLE_VALUE;

static void _conInit(void)
{
	if (hConOut==INVALID_HANDLE_VALUE)
		hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConInp==INVALID_HANDLE_VALUE)
		hConInp = GetStdHandle(STD_INPUT_HANDLE);
}

void conGotoxy(int x,int y)
{
	COORD coord;
	_conInit();
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(hConOut, coord);
}

int conClrscr(void)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	COORD home = {0, 0};
	DWORD bufsize;
	DWORD num;
	_conInit();
	GetConsoleScreenBufferInfo(hConOut, &info);
	bufsize = info.dwSize.X * info.dwSize.Y;
	FillConsoleOutputCharacter(hConOut, (TCHAR) ' ', bufsize, home, &num);
	GetConsoleScreenBufferInfo(hConOut, &info);
	FillConsoleOutputAttribute(hConOut, info.wAttributes, bufsize, home, &num);
	SetConsoleCursorPosition(hConOut, home);
	return 0;
}
void conGetXY(int *x,int *y)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	_conInit();
	GetConsoleScreenBufferInfo(hConOut, &info);
	if (x) *x=info.dwCursorPosition.X;
	if (y) *y=info.dwCursorPosition.Y;
}
int conGetX(void)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	_conInit();
	GetConsoleScreenBufferInfo(hConOut, &info);
	return info.dwCursorPosition.X;
}
void conSetSize(int iWidth,int iHeight)
{
	SMALL_RECT SR;
	CONSOLE_SCREEN_BUFFER_INFO info;
	_conInit();
	GetConsoleScreenBufferInfo(hConOut, &info);
	if ((iWidth>info.dwSize.X)||(iHeight>info.dwSize.Y))
	{
		COORD bsize=info.dwSize;
		if (iWidth >bsize.X) bsize.X=iWidth;
		if (iHeight>bsize.Y) bsize.Y=iHeight;
		SetConsoleScreenBufferSize(hConOut,bsize);
	}
	SR.Left=0;
	SR.Top =0;
	SR.Right =iWidth-1;
	SR.Bottom=iHeight-1;
	SetConsoleWindowInfo(hConOut,TRUE,&SR);
}

int conGetY(void)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	_conInit();
	GetConsoleScreenBufferInfo(hConOut, &info);
	return info.dwCursorPosition.Y;
}
void conGetSize(int *pWidth,int *pHeight)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	_conInit();
	GetConsoleScreenBufferInfo(hConOut, &info);
	if (pWidth ) *pWidth =(info.srWindow.Right  - info.srWindow.Left)+1;
	if (pHeight) *pHeight=(info.srWindow.Bottom - info.srWindow.Top )+1;
}
int conGetWidth(void)
{
	int iRes=0;
	conGetSize(&iRes,NULL);
	return iRes;
}
int conGetHeight(void)
{
	int iRes=0;
	conGetSize(NULL,&iRes);
	return iRes;
}
void conPutch(char ch)
{
	DWORD num;
	_conInit();
	WriteConsole(hConOut,&ch,1,&num,NULL);
}
void conWrite(char *buf,int len)
{
	DWORD num;
	_conInit();
	if (len<0) len=strlen(buf);
	WriteConsole(hConOut,buf,len,&num,NULL);
}
void conWritef(char *fmt,...)
{
	STP pVA;
	va_list ap;
	va_start(ap,fmt);
	pVA=stlSetSta(fmt,ap);
	va_end(ap);
	if (pVA)
	{
		conWrite(pVA->sBuf,pVA->iLen);
		stlFree(pVA);
	}
}
void conWritefXY(int x,int y,char *fmt,...)
{
	STP pVA;
	va_list ap;
	va_start(ap,fmt);
	pVA=stlSetSta(fmt,ap);
	va_end(ap);
	if (pVA)
	{
		conGotoxy(x,y);
		conWrite(pVA->sBuf,pVA->iLen);
		stlFree(pVA);
	}
}
int conKbhit(void)
{
	INPUT_RECORD buf;
	DWORD num=0;
	_conInit();
	PeekConsoleInput(hConInp,&buf,1,&num);
	return num;
}
void conColorSet(unsigned short wAttr)
{
	WORD attr=wAttr;
	_conInit();
	SetConsoleTextAttribute(hConOut,attr);
}

#endif // _WIN32

