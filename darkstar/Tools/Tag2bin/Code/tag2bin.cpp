//
// tag2bin.cpp
//
// Converts ASCII tag dictionaries to bin format
//

#include <stdio.h>
#include <core.h>
#include <tagDictionary.h>
#include <console.h>
#include <persist.h>
CMDConsole *c;
ResourceManager rm;

////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////

void changeFilenameExtension(char *s, char *e)
{
    char *c = &s[strlen(s)];
    while (*c != '.') c--;
    strcpy(c, e);
}

////////////////////////////////////////////////////////////////////////////

class ConOut : public CMDConsumer
{
   virtual void consoleCallback(CMDConsole *, const char *consoleLine);
};

void ConOut::consoleCallback(CMDConsole *, const char *consoleLine)
{
   fprintf(stderr, "%s\n", consoleLine);
}

int main(int argc, char **argv)
{
   c = new CMDConsole();
   ConOut co;
   c->setResourceManager(&rm);
   c->addConsumer(&co);

   if (argc < 2)
   {
      fprintf(stderr, "usage: [-bpath] [-hpath] tag2bin <tagFile0> [<tagFile1> ... <tagFileN>]\n\n");
      fprintf(stderr, 
            "Reads an ASCII tag file (.ttag) and writes a binary version\n"
            "(.btag) and a header file (.h).\n"
            "-s specifies directory where .ttag is located.\n"
            "-i specifies directory where .h    is stored.\n\n");
      return(1);    
   }

   char headerName[1024];
   char hPath[1024] = ".\\";
   char sPath[1024] = ".\\";

   TagDictionary *tag;
   FileWStream    fws;

   for (int i = 1; i < argc; i++)
   {
      char *pChr = argv[i];
      if ( *pChr=='-' )
      {
         pChr++;
         if( *pChr=='i' )
         {
            strcpy( hPath, ++pChr );
            fprintf(stderr, "Header output to:  %s\n", pChr);
         }
         else if( *pChr=='s' )
         {
            strcpy( sPath, ++pChr );
            fprintf(stderr, "Source from:  %s\n", pChr);
         }
         else
            fprintf(stderr, "unknown flag \"%c\"\n", *pChr);
      }
      else
      {
         // Read the ASCII tag
         fprintf(stderr, "Reading \"%s\"...\n", pChr);
         strcat(sPath, "\\");
         strcat(sPath, pChr);
         Console->executef(2, "exec", sPath);

         strcpy(headerName, pChr);
         changeFilenameExtension(headerName, ".h");

         // Write the header
         fprintf(stderr, "Writing \"%s\"...",headerName);
         if (fws.open(avar("%s\\%s",hPath,headerName)))
         {
            if (tagDictionary.writeHeader(fws) == Persistent::Base::Ok)
               fprintf(stderr, "Ok\n");
            else
               fprintf(stderr, "ERROR\n");
            fws.close();
         }
         else
            fprintf(stderr, "ERROR\n");
      }
   }
   delete c;
   return(0);
}
