#ifndef _STL_MATH_H_
#define _STL_MATH_H_

#include "stl_str.h"
#include <vector>

#ifdef  __cplusplus

namespace ansStl
{
	class mathEvaluate
	{
	private:
		void initVars(){sFormulla = NULL;}
		STP sFormulla;
		std::vector <double> valStack;
		std::vector <char> operStack;
		std::vector <STP> nameTab;		// gebruikers parameter namen
		std::vector <double> nameVal;	// gebruikers parameter waardes
	public:
		mathEvaluate(){initVars();}
		mathEvaluate(const char *fx){initVars(); setFormulla(fx);}
		int calculate();
		int calculate(const char *fx){setFormulla(fx); return calculate();}
		void setFormulla(const char *fx){stlFree(sFormulla); sFormulla = stlSetSt(fx);}
	public:
		void setUpdatVar(const char *varName,double varVal);
		void resetAllVars();
		void reset();
		STP  varDump(const char *postVar = NULL);
		STP  varNames(const char *postVar = NULL);
		STP  varValues();
	private:
		bool isAlphaNum(STP s,int iOfs){return (isAlpha(s,iOfs) || isNum(s,iOfs));}
		bool isAlpha(STP s,int iOfs);
		bool isNum(STP s,int iOfs);
		bool isOp(STP s,int iOfs);
		int  getNum(STP s,int iOfs);	// returns the new offset
		int  getVar(STP s,int iOfs);	// returns the new offset
		int  doOp  (STP s,int iOfs);	// returns the new offset
		double getVar(const char *varName);
		void doOp();
		double evaluate(double a, double b, char op);
		inline char charAt(STP s,int iOfs){return ((s != NULL) && (iOfs >= 0) && (iOfs < s->iLen)) ? s->sBuf[iOfs] : 0; }
		int prior(char op);
	};
}

#endif

#endif	// _STL_MATH_H_

