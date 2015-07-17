//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMPREFS_H_
#define _SIMPREFS_H_


#include <sim.h>
#include <registry.h>
#include <tVector.h>


class SimPreference;

//--------------------------------------------------------------------------- 
struct SimPreferenceEvent: public SimEvent
{
   SimPreference *prefObject;
	SimPreferenceEvent(SimPreference *p) 
	{ 
	   type = SimPreferenceEventType; 
      prefObject = p;
	}
};



//------------------------------------------------------------------------------
class SimPreference: public SimSet
{
public:
      enum Type
      {
         TYPE_STR       = (1<<0),
         TYPE_MULTI_STR = (1<<1),
         TYPE_INT       = (1<<2),
         TYPE_FLOAT     = (1<<3),
         TYPE_BLOCK     = (1<<4),
         TYPE_VOLATILE  = (1<<31),    // if set, will not save to registry
      };

private:
   enum constants {
      MAX_TAG_LENGTH = 32,
   };

   struct Preference
   {
      char  *key;
      DWORD dataLen;
      DWORD type;
      union {
         BYTE  *pData;  
         int   iData;
         float fData;         
      };
      Preference()   { key = NULL; pData = NULL; dataLen = 0; } 
      ~Preference()  { clear(); }
      void clear();
   };

   typedef VectorPtr<Preference*> PreferenceVector;
   PreferenceVector preferenceVector;
   Registry registry;

   PreferenceVector::iterator find(const char *key);
   PreferenceVector::iterator build(const char *key, Type type, DWORD dataLen);

   char *registryKey;
   bool autoSave;

   void clear();

public:   
   SimPreference(bool autoSave=false);
   ~SimPreference();

   // string
   bool getString(const char *key, char *str, DWORD len);
   bool setString(const char *key, const char *str);

   // multi-string
   bool getMultiString(const char *key, char *str, DWORD len);
   bool setMultiString(const char *key, const char *str);

   // int    
   bool get(const char *key, int *data);
   bool set(const char *key, int data);

   // float
   bool get(const char *key, float *data);
   bool set(const char *key, float data);

   // raw BYTE block
   bool get(const char *key, BYTE *data, DWORD *len);
   bool set(const char *key, BYTE *data, DWORD len);

   void makeVolatile(const char *key, bool tf );

   //--------------------------------------
   void setRoot(const char *registryKey);
   bool open();
   void close();
   void save();
   bool restore();

   void notify();
};

   

#endif //_SIMPREFS_H_
