// ThredParser.h : header file
//
#ifndef _THREDPARSER_H_
#define _THREDPARSER_H_

#include "thredprimitives.h"
#include "transformmatrix.h"
/////////////////////////////////////////////////////////////////////////////
// CThredParser window

class CThredParser
{
// Construction
public:
	int ReadMatrix(const char *Key, CTransformMatrix& Matrix );
	int WriteMatrix(const char *String, CTransformMatrix& Matrix );
	CString mEndString;
	int ReadPolygon(ThredPolygon& Poly);
	void WritePolygon(ThredPolygon& Poly);
	static const char* WhiteSpace;
	CString OutString;
	int IsStoring;
	CString Filename;
	CThredParser(CArchive* archive, char* EndType) 
	{	
		CFile *AFile;

		ar = archive; 
		IsStoring = ar->IsStoring();
		ar->Flush();
		AFile = ar->GetFile(); 
		if(!IsStoring ) {
			Filename = AFile->GetFilePath();
			AFile->Close();
			File.Open(Filename, CFile::modeRead);
		}
		mEndString = EndType;
	}
	~CThredParser()
	{
		if(!IsStoring ) {
			CFile *AFile;
			File.Close();
			ar->Flush();
			AFile = ar->GetFile(); 
			AFile->Open(Filename, CFile::modeRead);
		}

	}


	void SetEnd(char *EndType)
	{
		mEndString = EndType;
	}
	void GotoEnd()
	{
		GetKey(mEndString);
	}

	CArchive* ar;
	CStdioFile File;
void Init()
{
	//ar->Flush();
	//File = ar->GetFile();
}
void WriteString(const char* String, const char* EndString=NULL) 
{ 
	Init();
	if(EndString ) {
		OutString.Format("%s %s\r\n", String, EndString);
		ar->WriteString(OutString);
	}
	else {
		ar->WriteString(String); 
		ar->WriteString("\r\n"); 
	}
}


void WriteBool( const char* String, bool Val )
{
	Init();
	OutString.Format("%s %d\r\n", String, Val ? 1 : 0 );
	ar->WriteString( OutString );
} 
int GetKeyBool( const char * Key, bool &Val )
{
	long	Tmp;

	const char *IntString;
	if( !GetKey( Key ) )
		return( 0 );

	IntString = ( OutString.FindOneOf( WhiteSpace ) ) + ( LPCTSTR )OutString;
	sscanf( IntString, "%d", &Tmp );

	if( Tmp )
		Val = TRUE;
	else
		Val = FALSE;

	return( 1 );
}

void WriteFloat( const char* String, float Val )
{
	Init();
//	OutString.Format( "%s %f\r\n", String, Val );
   OutString.Format( "%s %x\r\n", String, *((int *)&Val) );
	ar->WriteString( OutString );

}
int GetKeyFloat( const char * Key, float &Val )
{
	const char * IntString;
	if( !GetKey( Key ) )
		return( 0 );

	IntString = ( OutString.FindOneOf( WhiteSpace ) ) + ( LPCTSTR )OutString;
	
   if(theApp.m_fileVersion < makeVersion(1,93))
      sscanf( IntString, "%f", &Val );
   else
      sscanf( IntString, "%x", &Val);
   
	return( 1 );
}

void WriteInt(const char* String, int Number)
{
	Init();
	OutString.Format("%s %d\r\n", String, Number);
	ar->WriteString(OutString);
}

void WriteLong(const char* String, long Number)
{
	Init();
	OutString.Format("%s %ld\r\n", String, Number);
	ar->WriteString(OutString);
}

void WriteULong(const char* String, unsigned long Number)
{
	Init();
	OutString.Format("%s %lu\r\n", String, Number);
	ar->WriteString(OutString);
}

void WritePoint(const char* String, ThredPoint& Point)
{
	Init();
//		OutString.Format("%s %lf %lf %lf\r\n", String, Point.X, Point.Y, Point.Z);
//	OutString.Format("%s %g %g %g\r\n", String, Point.X, Point.Y, Point.Z);
   OutString.Format("%s %x %x %x %x %x %x\r\n", String,
      *((int *)&Point.X), *(((int *)&Point.X) + 1),
      *((int *)&Point.Y), *(((int *)&Point.Y) + 1),
      *((int *)&Point.Z), *(((int *)&Point.Z) + 1));
	ar->WriteString(OutString);
}
																												 
void WritePoint2F(const char* String, Point2F& Point)
	{
		Init();
//		OutString.Format("%s %lf %lf %lf\r\n", String, Point.X, Point.Y, Point.Z);
//		OutString.Format("%s %g %g\r\n", String, Point.x, Point.y);
      OutString.Format("%s %x %x\r\n", String, *((int *)&Point.x), *((int *)&Point.y));
		ar->WriteString(OutString);
	}

																												 
int Trim() { OutString.TrimRight(); OutString.TrimLeft();  return OutString.GetLength(); }

// try to get our key line
int GetKey(const char* Key) 
	{
		// get our position in the file
		CString CompareString;
		int EndLength = mEndString.GetLength();
		DWORD FileCount = File.GetPosition();

		
		// now we want to go through the file
		// reading until we find the key
		while(File.ReadString(OutString) ) {
			// we have nothing left if trim returns 0
			if(!Trim() )
				continue;			

			// get a compare string which matches
			// the length of the key
			CompareString = OutString.Left(strlen(Key) );

			// otherwise compare with key
			if(!CompareString.CompareNoCase(Key ) )
				return 1;

			// comapre to our end string and if it
			// matches then stop					
			if(EndLength ) {
				CompareString = OutString.Left(EndLength );
				if(!CompareString.CompareNoCase(mEndString ) )
					break;
			} 
		}

				   
		// we didn't find our damn string so we have
		// to reset the file pointer and return 0
		File.Seek(FileCount, CFile::begin);
		FileCount = File.GetPosition();


		return 0;
}

int GetKeyInt(const char* Key, int& Value) 
{
	const char *IntString;
	// try to get our key line
	if(!GetKey(Key) )
		return 0;

	// we know have our key line in OutString
	IntString = (OutString.FindOneOf(WhiteSpace )) + (LPCTSTR)OutString;

	// now read the int
	sscanf(IntString, "%d", &Value);

	return 1;
}

long GetKeyLong(const char* Key, long& Value) 
{
	const char *LongString;
	// try to get our key line
	if(!GetKey(Key) )
		return 0;

	// we know have our key line in OutString
	LongString = (OutString.FindOneOf(WhiteSpace )) + (LPCTSTR)OutString;

	// now read the int
	sscanf(LongString, "%ld", &Value);

	return 1;
}

long GetKeyULong(const char* Key, unsigned long& Value) 
{
	const char *LongString;
	// try to get our key line
	if(!GetKey(Key) )
		return 0;

	// we know have our key line in OutString
	LongString = (OutString.FindOneOf(WhiteSpace )) + (LPCTSTR)OutString;

	// now read the int
	sscanf(LongString, "%lu", &Value);

	return 1;
}

int GetKeyPoint(const char* Key, ThredPoint& Value) 
{
	const char *IntString;
	// try to get our key line
	if(!GetKey(Key) )
		return 0;

	// we now have our key line in OutString
	IntString = (OutString.FindOneOf(WhiteSpace )) + (LPCTSTR)OutString;

   // check the version
   if(theApp.m_fileVersion < makeVersion(1,93))
   	sscanf(IntString, "%lf %lf %lf", &Value.X, &Value.Y, &Value.Z);
   else
   {
   	sscanf(IntString, "%x %x %x %x %x %x", 
         &Value.X, ((int *)&Value.X) + 1,
         &Value.Y, ((int *)&Value.Y) + 1,
         &Value.Z, ((int *)&Value.Z) + 1);
   }

	return 1;
}

int GetKeyPoint2F(const char* Key, Point2F& Value) 
{
	const char *IntString;
	// try to get our key line
	if(!GetKey(Key) )
		return 0;

	// we now have our key line in OutString
	IntString = (OutString.FindOneOf(WhiteSpace )) + (LPCTSTR)OutString;

   if(theApp.m_fileVersion < makeVersion(1,93))
	   sscanf(IntString, "%f %f", &Value.x, &Value.y);
   else
	   sscanf(IntString, "%x %x", &Value.x, &Value.y);

	return 1;
}

int GetKeyString(const char* Key, CString& InString) 
{
	// try to get our key line
	if(!GetKey(Key) )
		return 0;

	// we know have our key line in OutString
   int index = OutString.FindOneOf(WhiteSpace) + 1;
   if(!index)
      return(0);

   InString = (LPCTSTR)OutString + index;
	return 1;
}

};

/////////////////////////////////////////////////////////////////////////////

#endif

