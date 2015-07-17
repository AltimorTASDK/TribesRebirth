%{
#define YYLMAX 4096

#include <stdio.h>
#include "console.h"
#include "ast.h"
#include "gram.h"
#include "filstrm.h"
#include "talgorithm.h"
#include "resManager.h"
#include "consoleInternal.h"

static int Sc_ScanString();
static int Sc_ScanChar();
static int Sc_ScanNum();
static int Sc_ScanVar();
static int Sc_ScanHex();

#ifdef __BORLANDC__
#pragma option -w-rch
#endif

#define FLEX_DEBUG 1

//#undef input
//#undef unput
#undef CMDgetc;
int CMDgetc();
static int lineIndex;

%}

DIGIT	[0-9]
INTEGER	{DIGIT}+
FLOAT	({INTEGER}\.{INTEGER})|({INTEGER}(\.{INTEGER})?[eE][+-]?{INTEGER})
LETTER	[A-Za-z_]
FILECHAR [A-Za-z_\.]
IDTAIL	[:A-Za-z0-9_]
VAR	[$%]{LETTER}{IDTAIL}*
ID	{LETTER}{IDTAIL}*
ILID [$%]{DIGIT}+{LETTER}{IDTAIL}*
FILENAME {FILECHAR}+
SPACE	[ \t\v\f]
HEXDIGIT [a-fA-F0-9]
TTAG "ID"{IDTAIL}+

%%
			;
{SPACE}+	{ }
"//"[^\n\r]*	;
"#"[^\n\r]*   ;
[\r]        ;
[\n]		{lineIndex++;}
\"(\\.|[^\\"\n\r])*\"		{ return(Sc_ScanString()); }
'([^'\n]|("\\".))*[']?	{ return(Sc_ScanChar()); }
"=="        return(CMDlval.i = opEQ);
"!="        return(CMDlval.i = opNE);
">="        return(CMDlval.i = opGE);
"<="        return(CMDlval.i = opLE);
"&&"        return(CMDlval.i = opAND);
"||"        return(CMDlval.i = opOR);
"::"        return(CMDlval.i = opCOLONCOLON);
"--"        return(CMDlval.i = opMINUSMINUS);
"++"        return(CMDlval.i = opPLUSPLUS);
"$+"        return(CMDlval.i = opCAT);
"<<"        return(CMDlval.i = opSHL);
">>"        return(CMDlval.i = opSHR);
"+="        return(CMDlval.i = opPLASN);
"-="        return(CMDlval.i = opMIASN);
"*="        return(CMDlval.i = opMLASN);
"/="        return(CMDlval.i = opDVASN);
"%="        return(CMDlval.i = opMODASN);
"&="        return(CMDlval.i = opANDASN);
"^="        return(CMDlval.i = opXORASN);
"|="        return(CMDlval.i = opORASN);
"<<="       return(CMDlval.i = opSLASN);
">>="       return(CMDlval.i = opSRASN);
"@" |
"[" |
"]" |
"(" | 
")" | 
"+" | 
"-" | 
"*" | 
"/" |
"<" | 
">" | 
"|" | 
"." | 
"!" |
":" | 
";" |
"{" | 
"}" | 
"," |
"&" |
"%" |
"^" |
"~" |
"=" {       return(CMDlval.i = CMDtext[0]); }

"break"     { CMDlval.i = lineIndex; return(rwBREAK); }
"nil"       { CMDlval.i = lineIndex; return(rwNIL); }
"class"     { CMDlval.i = lineIndex; return(rwCLASS); }
"return"    { CMDlval.i = lineIndex; return(rwRETURN); }
"else"      { CMDlval.i = lineIndex; return(rwELSE); }
"global"    { CMDlval.i = lineIndex; return(rwGLOBAL); }
"while"     { CMDlval.i = lineIndex; return(rwWHILE); }
"if"        { CMDlval.i = lineIndex; return(rwIF); }
"for"       { CMDlval.i = lineIndex; return(rwFOR); }
"continue"  { CMDlval.i = lineIndex; return(rwCONTINUE); }
"function"  { CMDlval.i = lineIndex; return(rwDEFINE); }
"instant"   { CMDlval.i = lineIndex; return(rwDECLARE); }
{VAR}       return(Sc_ScanVar());
{TTAG}      { CMDtext[CMDleng] = 0; CMDlval.s = stringTable.insert(CMDtext); return(TTAG); }
{ID}        { CMDtext[CMDleng] = 0; CMDlval.s = stringTable.insert(CMDtext); return(IDENT); }
0[xX]{HEXDIGIT}+ return(Sc_ScanHex());
{INTEGER}   { CMDtext[CMDleng] = 0; CMDlval.i = atoi(CMDtext); return INTCONST; }
{FLOAT}     return Sc_ScanNum();
{ILID}      return(ILLEGAL_TOKEN);
.           return(ILLEGAL_TOKEN);
%%

/*
 * Scan character constant.
 */

/*
 * Scan identifier.
 */

static const char *scanBuffer;
static const char *fileName;
static int scanIndex;
 
void CMDerror(char * s, ...)
{
   s;
   CMDConsole::getLocked()->printf("%s Line: %d - Syntax error.",
      fileName, lineIndex);
}

void SetScanBuffer(const char *sb, const char *fn)
{
   scanBuffer = sb;
   fileName = fn;
   scanIndex = 0;
   lineIndex = 1;
}

int CMDgetc()
{
   int ret = scanBuffer[scanIndex];
   if(ret)
      scanIndex++;
   else
      ret = -1;
   return ret;
}

int CMDwrap()
{
   return 1;
}

static int Sc_ScanVar()
{
   CMDtext[CMDleng] = 0;
	CMDlval.s = stringTable.insert(CMDtext);
	return(VAR);
}
/*
 * Scan string constant.
 */

static int charConv(int in)
{
   switch(in)
   {
      case 'n':
         return '\n';
      case 't':
         return '\t';
      default:
         return in;
   }
}

static int getHexDigit(char c)
{
   if(c >= '0' && c <= '9')
      return c - '0';
   if(c >= 'A' && c <= 'F')
      return c - 'A' + 10;
   if(c >= 'a' && c <= 'f')
      return c - 'a' + 10;
   return -1;
}

static int Sc_ScanString()
{
	CMDtext[CMDleng - 1] = 0;
   int len = CMDleng - 1;
   int i;
   for(i = 1; i < len;)
   {
      if(CMDtext[i] == '\\')
      {
         if(CMDtext[i+1] == 'x')
         {
            int dig1 = getHexDigit(CMDtext[i+2]);
            if(dig1 == -1)
               return -1;

            int dig2 = getHexDigit(CMDtext[i+3]);
            if(dig2 == -1)
               return -1;
            CMDtext[i] = dig1 * 16 + dig2;
            memmove(CMDtext + i + 1, CMDtext + i + 4, len - i - 3);
            len -= 3;
            i++;
         }
         else
         {
            CMDtext[i] = charConv(CMDtext[i+1]);
            memmove(CMDtext + i + 1, CMDtext + i + 2, len - i - 1);
            len--;
            i++;
         }
      }
      else
         i++;
   }


	CMDlval.str = new char[strlen(CMDtext)];
   strcpy(CMDlval.str, CMDtext + 1);
	return(STRATOM);
}

static int Sc_ScanChar()
{
   CMDtext[CMDleng-1] = 0;
	CMDlval.str = new char[strlen(CMDtext)];
   strcpy(CMDlval.str, CMDtext + 1);
	return(STRATOM);
}

static int Sc_ScanNum()
{
   CMDtext[CMDleng] = 0;
	CMDlval.f = atof(CMDtext);
	return(FLTCONST);
}

static int Sc_ScanHex()
{
   int val = 0;
   sscanf(CMDtext, "%x", &val);
   CMDlval.i = val;
   return INTCONST;
}

