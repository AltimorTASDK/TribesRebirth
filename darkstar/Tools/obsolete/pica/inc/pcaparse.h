//---------------------------------------------------------------------------
//
//
//  Filename   : pcaparse.h
//
//
//  Description: The definition of the parse class - basically the parser
//               is there to create the flaglist for each image file, i.e.
//               any targa, bmp or pal file. The parser returns the number
//               of flags necessary (basically a file count) and a buffer
//               of the imageFlags is created, then a call to MakeImageFlags
//               fills the structures, parsing the token list.
//               
//      
//
//  Author     : Joel Franklin 
//
//
//  Date       : Friday 13th 1997
//
//
//
//---------------------------------------------------------------------------

#ifndef _PCAPARSE_H
#define _PCAPARSE_H

struct keywordtable
{
    char *keywordname;
    FLAG (*keywordfunction)(void *, char *, int);
};


class pca_parse
{
    private:
        
        COUNTER bufSize;
        char *TokList;
        int tokIndex;
        COUNTER TokListSize;
        COUNTER FlagNumber;
        
        keywordtable *KeyWordTable;
        void GetNextLine(char *);    
        COUNTER GetWeight(char *);
        void FindFlagNumber(void);
        COUNTER ExpandWildCard(void);
        COUNTER StringValue(char *);
        
        void InitKeywordTable(void);
        void RegisterKeyword(const char *, FLAG (*)(void *, char *, int));
        
        void IsBitmap(char *,allFlags *,COUNTER);
        
        
    public:
    
        pca_parse(char *, COUNTER inSize = 50000);
        ~pca_parse(void);
        void MakeImageFlags(allFlags *);
        COUNTER GetFlagNumber(void);
        COUNTER UpdateFlags(allFlags *, COUNTER);

};

#endif // _PCAPARSE_H