#include <stdlib.h>
#include <math.h>

#include "stl_math.h"
#include "stl_str.h"

using namespace ansStl;

int mathEvaluate::calculate()
{
	if (sFormulla == NULL) return 0;
	stlRemove(sFormulla,' ');
	stlRemove(sFormulla,'\t');
	valStack.clear();
	operStack.clear();
	int i=0;
	while (i < sFormulla->iLen)
	{
		if (isAlpha(sFormulla,i)){
			i = getVar(sFormulla,i);
		}else if (isNum(sFormulla,i)){
			i = getNum(sFormulla,i);
		}else if (isOp(sFormulla,i))
			i = doOp(sFormulla,i);
		else i++;
	}
	doOp();
	if (valStack.size() > 0) return (int)(valStack[0] + 0.5);
	return 0;
}
bool mathEvaluate::isAlpha(STP s,int iOfs)
{
	char c = charAt(s,iOfs);
	return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_'));
}
bool mathEvaluate::isNum(STP s,int iOfs)
{	
	char c = charAt(s,iOfs);
	return (((c >= '0') && (c <= '9')) || (c == '.'));
}
int mathEvaluate::getNum(STP s, int iOfs)
{
	int sign=1;
	if(charAt(s,iOfs)=='-' || charAt(s,iOfs)=='+')
		sign = charAt(s,iOfs++)=='-' ? -1 : 1;
	int iSta = iOfs;
	while (isNum(s,iOfs)) iOfs++;
	STP sVal = stlGetSect(s,iSta,iOfs - iSta);
	double val = atof(sVal->sBuf);
	stlFree(sVal);
	valStack.insert(valStack.begin(),sign * val);
	return iOfs;
}
int mathEvaluate::getVar(STP s,int iOfs)
{
	int iSta = iOfs;
	while (isAlphaNum(s,iOfs)) iOfs++;
	STP sVar = stlGetSect(s,iSta,iOfs - iSta);
	double val = getVar(sVar->sBuf);
	stlFree(sVar);
	valStack.insert(valStack.begin(),val);
	return iOfs;
}
void mathEvaluate::setUpdatVar(const char *varName,double varVal)
{
	int iLen = (int)nameTab.size();
	for (int i = 0; i < iLen; i++)
	{
		if (strcmp(varName,nameTab[i]->sBuf) == 0)
		{ 
			nameVal[i] = varVal; 
			return; 
		}
	}
	nameTab.push_back(stlSetSt(varName));
	nameVal.push_back(varVal);
}
void mathEvaluate::resetAllVars()
{
	int iLen = (int)nameTab.size();
	for (int i = 0; i < iLen; i++)
	{
		stlFree(nameTab[i]);
		nameTab[i] = NULL;
	}
	nameTab.clear();
	nameVal.clear();
}
STP mathEvaluate::varDump(const char *postVar /* = NULL */)
{
	if (postVar == NULL) postVar = "";
	STP sRes = stlSetSt("");
	int iLen = (int)nameTab.size();
	for (int i = 0; i < iLen; i++)
	{
		if (nameTab[i])
		{
			if (sRes->iLen) stlAppendCh(sRes,';');
			stlAppendStf(sRes,"%s%s;%d",postVar,nameTab[i]->sBuf,(int)(nameVal[i] + 0.5));
		}
	}
	return sRes;
}
STP  mathEvaluate::varNames(const char *postVar /* = NULL */)
{
	if (postVar == NULL) postVar = "";
	STP sRes = stlSetSt("");
	int iLen = (int)nameTab.size();
	for (int i = 0; i < iLen; i++)
	{
		if (nameTab[i])
		{
			if (sRes->iLen) stlAppendCh(sRes,';');
			stlAppendStf(sRes,"%s%s",postVar,nameTab[i]->sBuf);
		}
	}
	return sRes;
}
STP  mathEvaluate::varValues()
{
	STP sRes = stlSetSt("");
	int iLen = (int)nameTab.size();
	for (int i = 0; i < iLen; i++)
	{
		if (nameTab[i])
		{
			if (sRes->iLen) stlAppendCh(sRes,';');
			stlAppendStf(sRes,"%d",(int)(nameVal[i] + 0.5));
		}
	}
	return sRes;
}

void mathEvaluate::reset()
{
	resetAllVars();
	valStack.clear();
	operStack.clear();
}

double mathEvaluate::getVar(const char *varName)
{
	int iLen = (int)nameTab.size();
	for (int i = 0; i < iLen; i++)
		if (strcmp(varName,nameTab[i]->sBuf) == 0)
			return nameVal[i];
	return 0;
}
bool mathEvaluate::isOp(STP s,int iOfs)
{
	char c = charAt(s,iOfs);
	return (strchr("()+-*/^",c) != NULL);
}
int mathEvaluate::prior(char op)
{
	switch(op){
			case '+':
			case '-': return 1;
			case '*':
			case '/': return 2;
			case '^': return 4;
	}
	return 0;
}
int mathEvaluate::doOp(STP s, int i)
{
	char op=charAt(s,i);
	if(op=='(')
		operStack.insert(operStack.begin(),op);
	else
	{
		if(op==')'){
			while((operStack.size() > 0) && operStack[0]!='(')
				doOp();
			operStack.erase(operStack.begin()); //   pop_back();	// pop = removeFirst
		}
		else{
			while((operStack.size() > 0) && (prior(op) <= prior(operStack[0])))
				doOp();
			operStack.insert(operStack.begin(),op); //   .push_back(op);	// push = insertFirst
		}
	}
	return i+1;
}
void mathEvaluate::doOp()
{
	double b = 0, a = 0;
	if (valStack.size() > 0){ b = valStack[0]; valStack.erase(valStack.begin()); }
	if (valStack.size() > 0){ a = valStack[0]; valStack.erase(valStack.begin()); }
	char op = operStack[0]; operStack.erase(operStack.begin());
	double res=evaluate(a,b,op);
	valStack.insert(valStack.begin(),res);
}
double mathEvaluate::evaluate(double a, double b, char op)
{
	switch(op){
			case '+': return a+b;
			case '-': return a-b;
			case '/': return a/b;
			case '*': return a*b;
			case '^': return (int)pow(a,b);
	}
	return 0;
}
