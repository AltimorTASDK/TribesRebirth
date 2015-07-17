//------------------------------------------------------------------------------
// Description 
//    Provides simple interface to Windows 95/NT registry
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------


#include <windows.h>
#include <stdio.h>
#include <types.h>
#include "registry.h"


//------------------------------------------------------------------------------
Registry::Registry()
{
   // setup default state
   close();
}


//------------------------------------------------------------------------------
Registry::~Registry()
{
   //close all open keys
   close();
}


//------------------------------------------------------------------------------
// NAME 
//    HKEY Registry::getCurrentKey()
//    
// DESCRIPTION 
//    get last opened HKEY
//    
// ARGUMENTS 
//    none
//    
// RETURNS 
//    last HKEY or HKEY_CURRENT_USER
//    
// NOTES 
//    
//------------------------------------------------------------------------------
HKEY Registry::getCurrentKey()
{
   if ( keyList.size() ) 
      return keyList.last(); 
   return NULL; 
}


const char* Registry::getCurrentKeyName()
{
   if ( keyName.size() ) 
      return keyName.last(); 
   return NULL; 
}


//------------------------------------------------------------------------------
// NAME 
//    bool Registry::open(const char *key, bool create)
//    
// DESCRIPTION 
//    open/create a key
//    
// ARGUMENTS 
//    key      - name of key
//    create   - if true will create and open a key
//    
// RETURNS 
//    true on succcess
//    false otherwise
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::open(const char *regkey, bool create)
{
   HKEY  hKey = NULL;
   int   error;
   DWORD disposition;

   close();
   strcpy(path, regkey);

   // first figure out what root to use
   char *key = strtok(path, "\\/");
   if (!key)
      return (false);

   if (stricmp(key, "HKEY_CURRENT_USER") == 0)
      hKey = HKEY_CURRENT_USER;
   else if (stricmp(key, "HKEY_LOCAL_MACHINE") == 0)
      hKey = HKEY_LOCAL_MACHINE;
   else if (stricmp(key, "HKEY_CLASSES_ROOT") == 0)
      hKey = HKEY_CLASSES_ROOT;
   else if (stricmp(key, "HKEY_USERS") == 0)
      hKey = HKEY_USERS;

   if (!hKey)
      return (false);
   keyList.push_back( hKey );
   keyName.push_back( key );
   key = strtok(NULL, "\\/");
   while (key)
   {
      if ( create )
      {
         error = RegCreateKeyEx(getCurrentKey(),
                              key,
                              0,
                              "via Registry",
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKey,
                              &disposition);
      }
      else
      {
         error = RegOpenKeyEx(getCurrentKey(),
                              key,
                              0,
                              KEY_ALL_ACCESS,
                              &hKey );
      }
      if ( error != ERROR_SUCCESS )
      {
         close();
         return ( false );
      }
      keyList.push_back( hKey );
      keyName.push_back( key );
      key = strtok(NULL, "\\/");
   }
   return ( true );
}


//------------------------------------------------------------------------------
// NAME 
//    bool Registry::close()
//    
// DESCRIPTION 
//    close the last opened key
//    
// ARGUMENTS 
//    none
//    
// RETURNS 
//    true on success
//    false if no key is open
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::close()
{
   while( keyList.size() )
   {
      RegCloseKey( keyList.last() );
      keyList.pop_back();
   }
   keyName.clear();
   return ( true );
}


//------------------------------------------------------------------------------
// NAME 
//    DWORD Registry::remove(HKEY hStartKey, const char *pKeyName )
//    
// DESCRIPTION 
//    remove helper function
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    NT does not have a recursive delete key (like Win95, go figure)
//    so... we must make one ourselves.
//    
//------------------------------------------------------------------------------
DWORD Registry::remove(HKEY hStartKey, const char *pKeyName )
{
   #define MAX_KEY_LENGTH 255

   DWORD   result, subKeyLength;
   char    subKey[MAX_KEY_LENGTH]; 
   HKEY    hKey;
 
   // do not allow NULL or empty key name
   if ( pKeyName && lstrlen(pKeyName))
   {
      if( (result = RegOpenKeyEx(hStartKey, 
                        pKeyName,
                        0, 
                        KEY_ENUMERATE_SUB_KEYS | DELETE, 
                        &hKey )) == ERROR_SUCCESS)
      {
         while (result == ERROR_SUCCESS )
         {
            subKeyLength = MAX_KEY_LENGTH;
            result = RegEnumKeyEx( hKey,
                           0,          // always index zero, since we delete
                           subKey,
                           &subKeyLength,
                           NULL,
                           NULL,
                           NULL,
                           NULL );
 
            if(result == ERROR_NO_MORE_ITEMS)
            {
               result = RegDeleteKey(hStartKey, pKeyName);
               break;
            }
            else 
               if(result == ERROR_SUCCESS)
                  result = remove(hKey, subKey);
         }
         RegCloseKey(hKey);
         // Do not save return code because error
         // has already occurred
      }
   }
   else
      result = ERROR_BADKEY;
   return result;
}



//------------------------------------------------------------------------------
// NAME 
//    bool Registry::remove()
//    
// DESCRIPTION 
//    remove an entire key and its sub tree
//    
// ARGUMENTS 
//    key   - name of key to remove
//    
// RETURNS 
//    true on success
//    false on failure ot key does not exist
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::remove(const char *subkey)
{
   return (remove( getCurrentKey(), subkey) == ERROR_SUCCESS );
}


//------------------------------------------------------------------------------
// NAME 
//    bool Registry::enumerateKeys(DWORD index, char *name, DWORD len)
//    
// DESCRIPTION 
//    Enumerate through the tags in the current key
//    
// ARGUMENTS 
//    index - tag index to read
//    name  - pointer to buffer to fill with tag name
//    len   - len of buffer pointer to by name
//    
// RETURNS 
//    true on succcess
//    false if enumeration failed
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::enumerateKeys(DWORD index, char *name, DWORD len)
{
   return (RegEnumKeyEx( getCurrentKey(),
                  index,
                  name,
                  &len,
                  NULL, 
                  NULL,
                  NULL,
                  NULL) == ERROR_SUCCESS);
}


//------------------------------------------------------------------------------
// NAME 
//    bool Registry::enumerateTags(DWORD index, char *name, DWORD lenName, BYTE *data, DWORD lenData)
//    
// DESCRIPTION 
//    Enumerate the tags in the current key and optionally extract their data
//    
// ARGUMENTS 
//    index    - tag index to read
//    name     - pointer to buffer to fill with tag name
//    lenName  - len of buffer pointer to by name
//    data     - NULL or pointer to buffer to fill with tag data
//    lenData  - length of buffer pointed to by data
//    
// RETURNS 
//    true on succcess
//       lenData will be updated with actual size of data copied
//       type will contain data-type as specified in the registry REG_*
//    false if enumeration failed
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::enumerateTags(DWORD index, char *name, DWORD lenName, BYTE *data, DWORD *lenData, DWORD *type)
{
   DWORD _type;
   if ( !type ) type = &_type;
   return (RegEnumValue( getCurrentKey(),
                  index,
                  name,
                  &lenName,
                  NULL,
                  type, 
                  data,
                  lenData) == ERROR_SUCCESS);
}


//------------------------------------------------------------------------------
// NAME 
//    bool Registry::readString(const char *tag, char *str, DWORD len)
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    tag   - IN name of tag
//    str   - IN pointer to buffer to fill with string
//    len   - IN size of buffer pointed to  by str
//    
// RETURNS 
//    true on succcess
//    false otherwise
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::readString(const char *tag, char *str, DWORD len)
{
   DWORD type;
   if (RegQueryValueEx(getCurrentKey(), tag, 0, &type, (BYTE*)str, &len) == ERROR_SUCCESS)
   {
      if ( type == REG_SZ )
         return(true);
   }
   return(false);
}

 
//------------------------------------------------------------------------------
// NAME 
//    bool Registry::writeString(const char *tag, const char *str)
//    
// DESCRIPTION 
//    write a string to a registry tag
//    
// ARGUMENTS 
//    tag   - IN name of tag
//    str   - IN pointer to string to write
//    
// RETURNS 
//    true on succcess
//    false otherwise
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::writeString(const char *tag, const char *str)
{
   DWORD size = strlen(str) + 1;
   return ( RegSetValueEx(getCurrentKey(), tag, 0, REG_SZ, (BYTE*)str, size) == ERROR_SUCCESS );
}


//------------------------------------------------------------------------------
// NAME 
//    bool readMultiString(const char *tag, char *str, DWORD len)
//    
// DESCRIPTION 
//    read a multi-part string
//    
// ARGUMENTS 
//    tag   - IN name of tag
//    str   - IN pointer to buffer to fill with multi-part string
//    len   - IN size of buffer pointed to  by str
//    
// RETURNS 
//    true on succcess
//    false otherwise
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::readMultiString(const char *tag, char *str, DWORD len)
{
   DWORD type;
   if (RegQueryValueEx(getCurrentKey(), tag, 0, &type, (BYTE*)str, &len) == ERROR_SUCCESS)
   {
      if ( type == REG_MULTI_SZ )
         return(true);
   }
   return(false);
}   


//------------------------------------------------------------------------------
// NAME 
//    bool writeMultiString(const char *tag, const char *str)
//    
// DESCRIPTION 
//    write a multi-part string 
//    
// ARGUMENTS 
//    tag - IN name of tag
//    str - IN point to  str1\0str2\0str3\0\0
//    
// RETURNS 
//    true on succcess
//    false otherwise
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::writeMultiString(const char *tag, const char *str)
{
   const char *s = str;
   int len = 0;
   while (*s)
   {
      len += strlen(s)+1;
      s   += len;
   }
   len++; // final terminator
   
   return ( RegSetValueEx(getCurrentKey(), tag, 0, REG_MULTI_SZ, (BYTE*)str, len) == ERROR_SUCCESS );
}   


//------------------------------------------------------------------------------
// NAME 
//    bool Registry::read(const char *tag, int *data)
//    
// DESCRIPTION 
//    read a DWORD from a registry tag
//    
// ARGUMENTS 
//    tag   - IN name of tag
//    data  - IN/OUT pointer to a DWORD to be filled
//    
// RETURNS 
//    true on succcess
//    false otherwise
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::read(const char *tag, int *data)
{
   DWORD type, size = sizeof(DWORD);
   if (RegQueryValueEx(getCurrentKey(), tag, 0, &type, (BYTE*)data, &size) == ERROR_SUCCESS)
   {
      if ( type == REG_DWORD )
         return(true);
   }
   return(false);
}


//------------------------------------------------------------------------------
// NAME 
//    bool Registry::write(const char *tag, int data)
//    
// DESCRIPTION 
//    write a DWORD to ta registry tag
//    
// ARGUMENTS 
//    tag   - IN name of tag
//    data  - IN DWORD to write out
//    
// RETURNS 
//    true on succcess
//    false otherwise
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::write(const char *tag, int data)
{
   if (RegSetValueEx(getCurrentKey(), tag, 0, REG_DWORD, (BYTE*)&data, sizeof(data)) == ERROR_SUCCESS)
      return(true);
   return(false);
}



//------------------------------------------------------------------------------
// NAME 
//    bool Registry::read(const char *tag, float *data)
//    
// DESCRIPTION 
//    read a FLOAT from a registry tag
//    
// ARGUMENTS 
//    tag   - IN name of tag
//    data  - IN/OUT pointer to a FLOAT to be filled
//    
// RETURNS 
//    true on succcess
//    false otherwise
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::read(const char *tag, float *data)
{
   char buff[100];
   DWORD type, size = 100;

   if (RegQueryValueEx(getCurrentKey(), tag, 0, &type, (BYTE*)buff, &size) == ERROR_SUCCESS)
   {
      if ( type == REG_SZ )
         if ( sscanf(buff, "%f", data) == 1 )
            return(true);
   }
   return(false);
}


//------------------------------------------------------------------------------
// NAME 
//    bool Registry::write(const char *tag, float data)
//    
// DESCRIPTION 
//    write a FLOAT to ta registry tag
//    
// ARGUMENTS 
//    tag   - IN name of tag
//    data  - IN FLOAT to write out
//    
// RETURNS 
//    true on succcess
//    false otherwise
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::write(const char *tag, float data)
{
   char buff[100];
   sprintf(buff, "%f", data);
   DWORD size = strlen(buff) + 1;
   return ( RegSetValueEx(getCurrentKey(), tag, 0, REG_SZ, (BYTE*)buff, size) == ERROR_SUCCESS );
}



//------------------------------------------------------------------------------
// NAME 
//    bool Registry::read(const char *tag, BYTE *data, DWORD *len)
//    
// DESCRIPTION 
//    read a block of BINARY data from a registry tag
//    
// ARGUMENTS 
//    tag   - IN name of tag
//    data  - IN pointer to block of memory to fill
//    len   - IN/OUT size of memory pointed to by data
//             on success is filled with bytes actually read
//    
// RETURNS 
//    true on succcess
//    false otherwise
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::read(const char *tag, BYTE *data, DWORD *len)
{
   DWORD type;
   if (RegQueryValueEx(getCurrentKey(), tag, 0, &type, data, len) == ERROR_SUCCESS)
   {
      if ( type == REG_BINARY )
         return(true);
   }
   return(false);
}


//------------------------------------------------------------------------------
// NAME 
//    bool Registry::write(const char *tag, BYTE *data, DWORD len)
//    
// DESCRIPTION 
//    write a block of BINARY data to a registry tag
//    
// ARGUMENTS 
//    tag   - IN name of tag
//    data  - IN pointer to block of memory to fill
//    len   - INT size of memory pointed to by data
//    
// RETURNS 
//    true on succcess
//    false otherwise
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool Registry::write(const char *tag, BYTE *data, DWORD len)
{
   if (RegSetValueEx(getCurrentKey(), tag, 0, REG_BINARY, data, len) == ERROR_SUCCESS)
      return(true);
   return(false);
}


