//---------------------------------------------------------------------------

//	$Workfile:   cdump.cpp  $
//	$Version$
//	$Revision:   1.0  $
//	$Date:   04 Dec 1995 12:12:40  $

//	Dump out the contents of classio data files. All items that have
// been declared have their comment field printed out as well as their
// memory requirements. For undeclared items, the header type (in hex)
// and the header size are displayed.


//---------------------------------------------------------------------------

#include <iostream.h>

#include <assert.h>
#include <streamio.h>
#include <classio.h>
#include <streams.h>


//---------------------------------------------------------------------------
// Header files of libraries we want to initialize.

#include <ts.h>
//#include <anim.h>

//---------------------------------------------------------------------------

void __cdecl main(int argc,char *argv[])
{
	// Initialize the libraries that we want to be able
	// to recognize.

	TSInit();
   //ANInit();

	// Check arguments, should have at least one.

	if (argc < 2)
	{
		cout << "usage: file1 [file2] [...fileN]" << endl;
		cout << "The header information for the ClassItems contained in" << endl;
		cout << "the files will get printed to stdout." << endl;
		return;
	}

	// Loop through all the files on the command line.

	for (int i = 1; i < argc; ++i)
	{
		// Print out the file name if there is more than one.

		if (argc > 2)
			cout << argv[i] << ":" << endl;

		// Open the file and make there was no trouble.

		FileRStream s(argv[i]);

		if (s.getStatus() != STRM_OK)
		{
			cout << "Error openning file" << endl;
			continue;
		}

		// Loop while there are items to load.
		//

		ClassHeader header;

		for (int b = 0; ClassItem::peekItem(s,&header); b++)
		{
			cout << "[" << b << "] ";

			if(ClassItem::getTableEntry(header.type) != 0)
			{
				// Load the whole thing as one block and create
				// a memory stream (much faster than using the 
				// current FileRWStream).

				Int32 size = ClassItem::classItemSize(&header);
				char* buf = new char[size];
				AssertFatal(buf,"Memory allocation failure");

				s.read(size,buf);
				MemRWStream ms(size,buf);

				ClassItem* ip = ClassItem::loadItem(ms);
				ip->classDump(10,0);

				delete ip;
				delete buf;
			}
			else
			{
				// Print out the header information and skip over
				// the data in the stream.

				ClassItem::skipItem(s);
				cout << hex << header.type << " Size: " << dec <<
						header.size << endl;
			}
		}
	}
}


