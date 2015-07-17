//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <sim.h>
#include <simPrefs.h>


//------------------------------------------------------------------------------
void SimPreference::Preference::clear()   
{ 
   delete [] key; 
   if (type & (TYPE_STR | TYPE_BLOCK) )
      delete [] pData; 
   pData = NULL; 
}


//------------------------------------------------------------------------------
SimPreference::SimPreference(bool _autoSave)
{
   registryKey = NULL;
   autoSave = _autoSave;
}


//------------------------------------------------------------------------------
SimPreference::~SimPreference()
{
   close();
   delete [] registryKey;
}


//------------------------------------------------------------------------------
SimPreference::PreferenceVector::iterator SimPreference::find(const char *key)
{
   //for now just a linear search 
   VectorPtr<Preference*>::iterator i = preferenceVector.begin();
   for (; i != preferenceVector.end(); i++)
      if ( strcmp((*i)->key, key) == 0 )
         return (i);
   return (NULL);
}  


//------------------------------------------------------------------------------ 
SimPreference::PreferenceVector::iterator SimPreference::build(const char *key, Type type, DWORD dataLen)
{
   PreferenceVector::iterator i = find(key);
   if (i)
   {
      if ( (*i)->type & type )
      {
         if ( (type & (TYPE_STR | TYPE_BLOCK) ) && (*i)->dataLen < dataLen)
         {
            delete [] (*i)->pData;   
            (*i)->pData= new BYTE[dataLen];
         }
         return (i);
      }

      delete (*i);
      preferenceVector.erase(i);
   }
   
   // could not find a match      
   Preference *p = new Preference;
   p->key     = strnew(key);
   p->dataLen = dataLen;
   p->type    = type;
   if (type & (TYPE_STR | TYPE_BLOCK) )
      p->pData= new BYTE[dataLen];
   preferenceVector.push_back( p );

   return (preferenceVector.end()-1);
}   


//------------------------------------------------------------------------------
// string
bool SimPreference::getString(const char *key, char *str, DWORD len)
{
   PreferenceVector::iterator p = find(key);
   if ( p && (*p)->type & TYPE_STR)
   {
      strncpy(str, (char *) (*p)->pData, min(len, (*p)->dataLen));
      return (true);
   }
   return (false);
}


//------------------------------------------------------------------------------
bool SimPreference::setString(const char *key, const char *str)
{
   int len = strlen(str)+1;
   PreferenceVector::iterator p = build(key, TYPE_STR, len);
   if ( p )
      strcpy( (char *) (*p)->pData, str);
   return (p);
}


//------------------------------------------------------------------------------
// multi-string
bool SimPreference::getMultiString(const char *key, char *str, DWORD len)
{
   PreferenceVector::iterator p = find(key);
   if ( p && (*p)->type & TYPE_MULTI_STR)
   {
      strncpy(str, (char *) (*p)->pData, min(len, (*p)->dataLen));
      return (true);
   }
   return (false);
}


//------------------------------------------------------------------------------
bool SimPreference::setMultiString(const char *key, const char *str)
{
   const char *s = str;
   int len = 0;
   while (*s)
   {
      len += strlen(s)+1;
      s   += len;
   }
   len++; // final terminator
   
   PreferenceVector::iterator p = build(key, TYPE_MULTI_STR, len);
   if ( p )
      strcpy( (char *) (*p)->pData, str);
   return (p);
}


//------------------------------------------------------------------------------
// int    
bool SimPreference::get(const char *key, int *data)
{
   PreferenceVector::iterator p = find(key);
   if ( p && (*p)->type & TYPE_INT)
   {
      *data = (*p)->iData;
      return (true);
   }
   return (false);
}


//------------------------------------------------------------------------------
bool SimPreference::set(const char *key, int data)
{
   PreferenceVector::iterator p = build(key, TYPE_INT, sizeof(int));
   if ( p )
      (*p)->iData = data;
   return (p);
}


//------------------------------------------------------------------------------
// float
bool SimPreference::get(const char *key, float *data)
{
   PreferenceVector::iterator p = find(key);
   if ( p && (*p)->type & TYPE_FLOAT)
   {
      *data = (*p)->fData;
      return (true);
   }
   return (false);
}


//------------------------------------------------------------------------------
bool SimPreference::set(const char *key, float data)
{
   PreferenceVector::iterator p = build(key, TYPE_FLOAT, sizeof(float));
   if ( p )
      (*p)->fData = data;
   return (p);
}


//------------------------------------------------------------------------------
// raw BYTE block
bool SimPreference::get(const char *key, BYTE *data, DWORD *len)
{
   PreferenceVector::iterator p = find(key);
   if ( p && (*p)->type & TYPE_BLOCK )
   {
      *len = min(*len, (*p)->dataLen);
      memcpy( data, (*p)->pData, *len);
      return (true);
   }
   return (false);
}


//------------------------------------------------------------------------------
bool SimPreference::set(const char *key, BYTE *data, DWORD len)
{
   PreferenceVector::iterator p = build(key, TYPE_BLOCK, len);
   if ( p )
      memcpy( data, (*p)->pData, len);
   return (p);
}


//------------------------------------------------------------------------------
void SimPreference::makeVolatile(const char *key, bool tf )
{
   PreferenceVector::iterator p = find(key);
   if ( p )
   {
      if (tf) (*p)->type |= TYPE_VOLATILE;
      else    (*p)->type &= ~TYPE_VOLATILE;
   }
}   


//------------------------------------------------------------------------------
void SimPreference::notify()
{
   SimPreferenceEvent notify(this);
	for (iterator ptr = begin(); ptr != end(); ptr++)
		(*ptr)->processEvent(&notify);
}


//------------------------------------------------------------------------------
void SimPreference::setRoot(const char *regKey)
{
   close();
   delete [] registryKey;
   registryKey = strnew(regKey);
}  


//------------------------------------------------------------------------------
bool SimPreference::open()
{
   close();
   return (restore());
}   


//------------------------------------------------------------------------------
bool SimPreference::restore()
{
   clear();
   if (registryKey && registry.open( registryKey, true ))
   {
      #define MAX_TAG_LENGTH (255)
      char buff[MAX_TAG_LENGTH];
      DWORD dataLen;
      DWORD type;
      DWORD index = 0;

      while (registry.enumerateTags(index, buff, MAX_TAG_LENGTH, NULL, &dataLen, &type))
      {
         Preference *p= new Preference;
         p->key      = strnew(buff);
         switch ( type )
         {
            case REG_BINARY:
               p->pData   = new BYTE[dataLen];
               p->dataLen = dataLen;
               p->type    = TYPE_BLOCK;
               registry.read(p->key, p->pData, &dataLen);
               break;

            case REG_SZ:
               if ( registry.read(p->key, &p->fData) )
               {
                  p->dataLen = sizeof(float);
                  p->type    = TYPE_FLOAT;
               }
               else
               {
                  p->pData   = new BYTE[dataLen];
                  p->dataLen = dataLen;
                  p->type    = TYPE_STR;
                  registry.readString(p->key, (char *) p->pData, p->dataLen);
               }
               break;

            case REG_MULTI_SZ:
               p->pData   = new BYTE[dataLen];
               p->dataLen = dataLen;
               p->type    = TYPE_STR;
               registry.readMultiString(p->key, (char *) p->pData, p->dataLen);
               break;

            case REG_DWORD:
               p->dataLen = sizeof(int);
               p->type    = TYPE_INT;
               registry.read(p->key, &p->iData);
               break;
         }
         preferenceVector.push_back(p);
         index++;
      }
      return (true);
   }
   return (false);
}   


//------------------------------------------------------------------------------
void SimPreference::close()
{
   if (autoSave) save();
   clear();
}   


//------------------------------------------------------------------------------
void SimPreference::clear()
{
   VectorPtr<Preference*>::iterator i = preferenceVector.begin();
   for (; i != preferenceVector.end(); i++)
      delete (*i);
   preferenceVector.clear();
   registry.close();
}   


//------------------------------------------------------------------------------
void SimPreference::save()
{
   VectorPtr<Preference*>::iterator i = preferenceVector.begin();
   for (; i != preferenceVector.end(); i++)
   {
      // don't save if volatile
      if ( !((*i)->type & TYPE_VOLATILE) )
         switch ( (*i)->type & ~TYPE_VOLATILE)
         {
            case TYPE_STR:       registry.writeString((*i)->key, (char *) (*i)->pData);   break;
            case TYPE_MULTI_STR: registry.writeMultiString((*i)->key, (char *) (*i)->pData);   break;
            case TYPE_INT:       registry.write((*i)->key, (*i)->iData); break;
            case TYPE_FLOAT:     registry.write((*i)->key, (*i)->fData); break;
            case TYPE_BLOCK:     registry.write((*i)->key, (*i)->pData, (*i)->dataLen); break;
         }
   }
}


