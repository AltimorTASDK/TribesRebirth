//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMINTERIORGROUPING_H_
#define _SIMINTERIORGROUPING_H_

//Includes
#include <simBase.h>
#include <resManager.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

struct InteriorGroupingScript
{
  private:
   static const char* const sm_pIGScriptBeginTag;
   static const char* const sm_pIGScriptEndTag;

   int  curPos;
   bool valid;

   bool isEnd();
  public:
   int   size;
   char* data;
   InteriorGroupingScript()  { size = 0; data = NULL; curPos = 0; valid = false;}
   ~InteriorGroupingScript() { delete[] data; }
   
   bool getNextLine(char* out_pLine);
   bool isValid();
   void reset() { curPos = 0; }
};

class SimInteriorGrouping : public SimGroup
{
   typedef SimGroup Parent;

  private:
   bool parseIGScript(Resource<InteriorGroupingScript>&);

   SimGroup* parseGroupName(const char* in_pGroupClass,
                                    char*       io_pGroupSpec,
                                    const bool  in_createGroup = true);
   bool      parseObject(Resource<InteriorGroupingScript>& io_igScript);
   
  
  public:
   bool processArguments(int argc, const char **argv);

   DECLARE_PERSISTENT(SimInteriorGrouping);
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_SIMINTERIORGROUPING_H_














