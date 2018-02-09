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

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


/* Get status of RAS connection
**	   <1>  0=Ok -99=No info
**     <2>  2=Calling 5=Logging in 8192=Connected
**     <3>  600=Calling/logging in   0=Logged in
**     <4>  "modem" ....
**     <5>  "Descriptoin of modem"
**     <6,1> "My IP adres"
**     <6,2> "Remote IP adres"
**     <7,1> bytes transmitted
**     <7,2> bytes received
**     <7,3> frames transmitted
**     <7,4> frames received
*/
STP stlRasStatus(void);


int stlRasHangup(void);

/* Create dial-out modem connection
**	sCfg configuration record
**       <1>   Phone book entry naam
**       <2>   Phone number
**       <3,1> User name
**       <3,2> User password
**       <4>   Domain name
**       <5>   My IP address
** Return 0 Ok
**       -1 Out of memory
**       -2 Invalid entry name
**       -3 No valid modem found to use
**       -4 Error updating phone book
**       -5 dial error (no connection, dial-tone, etc)
*/
int stlRasDial(STP sCfg);

/* Get message from ras message fifo
 * Return STP (message)
 *        NULL (no messages in fifo)
 */
STP stlRasGetMsgFiFo(void);

/* Get message from ras message fifo (non destructive)
 * Return STP (message)
 *        NULL (no messages in fifo)
 */
STP stlRasPeekMsgFiFo(void);

#ifdef __cplusplus
}
#endif // __cplusplus
