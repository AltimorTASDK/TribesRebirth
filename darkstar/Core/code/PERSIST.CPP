#include "persist.h"
#include <filstrm.h>
#include "stringTable.h"
namespace Persistent
{
   //--------------------------------------------------------------------
   // AbstractClass:
   //--------------------------------------------------------------------

char * __cdecl AbstractClass::convertClassName( char const *rawClassName, 
                                                char *className )
{
   // check for class names that are too long
   // and convert to a standard (compiler independent)
   // class name:

   AssertFatal( strlen( rawClassName ) < AbstractClass::MaxNameSize,
      "class name too long for persistent class" );
   strcpy( className, rawClassName );

   #ifdef _MSC_VER
   char *pos = strstr( className, "class " );
   if( pos )
      strcpy( pos, pos + 6 );
   #endif

   return className;
}

//--------------------------------------------------------------------
// AbstractTaggedClass:
//--------------------------------------------------------------------

AbstractTaggedClass * AbstractTaggedClass::head = 0;
AbstractTaggedClass * AbstractTaggedClass::fastTable[AbstractClass::FastTableSize];

Vector<AbstractTaggedClass::Field> fieldList;

void Base::addField(const char *fieldName, int type, int offset, int count)
{
   AbstractTaggedClass::Field f;
   f.fieldName = stringTable.insert(fieldName);
   f.type = type;
   f.offset = offset;
   f.elementCount = count;
   fieldList.push_back(f);
}

void AbstractTaggedClass::initPersistFields()
{
   AbstractTaggedClass *walk = head;
   while(walk)
   {
      fieldList.setSize(0);
      walk->initFields();
      if(fieldList.size())
      {
         walk->fields = new Field[fieldList.size()];
         walk->numFields = fieldList.size();
         int i;
         for(i = 0; i < walk->numFields;i++)
            walk->fields[i] = fieldList[i];
      }
      walk = walk->next;
   }
}

void AbstractTaggedClass::linkTag()
{
   if(!head)
      for(int i = 0; i < AbstractClass::FastTableSize; i++)
         fastTable[i] = 0;

   if(tag < AbstractClass::FastTableSize && tag > 0)
      fastTable[tag] = this;
   else if(tag2 < AbstractClass::FastTableSize && tag2 > 0)
      fastTable[tag2] = this;

	next = head;
	head = this;
}

void * AbstractTaggedClass::create( const char *className )
{
   // search for registered class with this name and, if found
   // use it to create the object:

   char mangledName[AbstractClass::MaxNameSize];
   for( const AbstractTaggedClass *cur = head; cur; cur = cur->next )
      if( !stricmp( className, cur->name ) )
         return cur->create();
   return 0;
}

void * AbstractTaggedClass::create( int classTag )
{
   // search for registered class with this tag and, if found
   // use it to create the object:
   if(classTag > 0 && classTag < AbstractClass::FastTableSize)
   {
      if(!fastTable[classTag])
         return NULL;
      return fastTable[classTag]->create();
   }

   for( const AbstractTaggedClass *cur = head; cur; cur = cur->next )
      if( classTag == cur->tag )
         return cur->create();
   return 0;
}

AbstractTaggedClass* AbstractTaggedClass::lookupClass( int tag )
{
   if(tag > 0 && tag < AbstractClass::FastTableSize)
      return fastTable[tag];
   
   for( AbstractTaggedClass *cur = head; cur; cur = cur->next )
	   if (cur->tag == tag)
		   return cur;
	return 0;
}

const char *AbstractTaggedClass::lookupClassName(int tag)
{
   AbstractTaggedClass *aclass = lookupClass(tag);
   if (aclass)
      return aclass->name;
   return NULL;
}

Bool AbstractTaggedClass::lookupClass( char const *className, int *tag )
{
   // search for registered class with this name and, if found
   // return its tag and name:

   for( const AbstractTaggedClass *cur = head; cur; cur = cur->next )
      if( !strcmp( className, cur->name ) )
         {
         *tag = cur->tag;
         return TRUE;
         }
   return FALSE;
}

void AbstractTaggedClass::dumpClasses()
{
   for( const AbstractTaggedClass *cur = head; cur; cur = cur->next )
      cur->dump();
}

AbstractTaggedClass::AbstractTaggedClass()
{
   fields = NULL;
}

AbstractTaggedClass::~AbstractTaggedClass()
   {
   delete[] fields;

   // unlink from the list of registered classes:
   if(tag < AbstractClass::FastTableSize && tag > 0)
      fastTable[tag] = NULL;
   else if(tag2 < AbstractClass::FastTableSize && tag2 > 0)
      fastTable[tag2] = NULL;

   for( AbstractTaggedClass ** pcur = &head;
        *pcur; pcur = &(*pcur)->next )
      if( *pcur == this )
         {
         *pcur = (*pcur)->next;
         break;
         }
   }

//--------------------------------------------------------------------
// AbstractUntaggedClass:
//--------------------------------------------------------------------

AbstractUntaggedClass * AbstractUntaggedClass::head = 0;

void AbstractUntaggedClass::linkTag()
{
	next = head;
	head = this;
}

void * AbstractUntaggedClass::create( StreamIO &stream, int *user )
{
   // search for registered class with this stream and, if found
   // use it to create the object:

   Int32 startPos = stream.getPosition();
   for( const AbstractUntaggedClass *cur = head; cur; cur = cur->next )
      {
      if( cur->idFunc( stream ) )
      {
			*user = cur->user;
         return cur->create();
      }
      stream.setPosition( startPos );
      }
   return 0;
}

Bool AbstractUntaggedClass::lookupClass( char const *className,
                                             int user )
{
   // search for registered class with this name and, if found
   // return its tag and name:

   for( const AbstractUntaggedClass *cur = head; cur; cur = cur->next )
      if( user == cur->user && !strcmp( className, cur->name ) )
         {
         return TRUE;
         }
   return FALSE;
   }

void AbstractUntaggedClass::dumpClasses()
{
   for( const AbstractUntaggedClass *cur = head; cur; cur = cur->next )
      cur->dump();
}

AbstractUntaggedClass::~AbstractUntaggedClass()
{
   // unlink from the list of registered classes:

   for( AbstractUntaggedClass ** pcur = &head;
        *pcur; pcur = &(*pcur)->next )
      if( *pcur == this )
         {
         *pcur = (*pcur)->next;
         break;
         }
}

//--------------------------------------------------------------------
// Base class:
//--------------------------------------------------------------------

VectorPtr<void*> *Base::dictionary=0;

void  Base::openDictionary()
{
   AssertWarn( !dictionary, "Base::openDictionary:  Dictionary previously open" );
   delete dictionary;
   dictionary = new VectorPtr<void*>;
}

void  Base::closeDictionary()
{
   AssertWarn( dictionary, "Base::closeDictionary:  Dictionary not open" );
   if ( !dictionary ) 
      return;
   dictionary->clear();
   delete dictionary;
   dictionary = NULL;
}

Base * Base::fileLoad( const char *name, Error *err )
{
   FileRStream sio;
   if (sio.open(name))
      return (load(sio, err));   
   return (NULL);
}

Base::Error Base::read( StreamIO &, int version, int user )
{
   version;
   user;
   return Ok;
}

Base::Error Base::write( StreamIO &, int version, int user )
{
   version;
   user;
   return Ok;
}

void Base::initPersistFields()
{
}

Base::Error Base::fileStore( const char *name, int user ) const
{
   FileWStream sio;
   if (sio.open(name))
      return (store(sio, user));   
   return WriteError;
}


Base* Base::load( StreamIO& stream, Error *err )
{
   // load a persistent object from the stream
	int 		user = USER_DEFAULT;
   Base *   object = 0;
   Int32    startPosition = stream.getPosition();

   *err = Ok;

   // determine the type of the object and attempt to create one:
   // first, assume it's a registered Tagged type and try to open
   // a block object.  

   BlockRStream bs;
   bs.open( stream );
	if (bs.getStatus() != STRM_OK)
		return 0;

   // is it a dictionary object?
   if( bs.getID() == Int32(FOURCC('D','I','C','T')) )
   {
      AssertWarn( dictionary, "Base::load:  Attempt to load dictionary object with no dictionary" );
      int index;
      if( bs.read(&index) && index < dictionary->size() )
         return ( (Base*)(*dictionary)[index] );   // return ptr to dictionary object
      else 
      {
         AssertWarn( 0, "Base::load:  Attempt to load object not in dictionary" );
         *err = IncorrectTypeFound;
         return 0;
      }
   }

   // Is it a PERS type block?

   if( bs.getID() == Int32(FOURCC('P','E','R','S')) )
      {
      // PERS blocks have the class name after the block header:

      short namesize;
      if( !bs.read(&namesize) )
         *err = ReadError;
      if( !*err )
         {
         if( namesize > AbstractClass::MaxNameSize )
            *err = InvalidName;
         }
      if( !*err )
         {
         char name[AbstractClass::MaxNameSize];
         if( !bs.read( (namesize + 1) & (~1), name ) )
            *err = ReadError;
         else
            {
            name[namesize] = 0;
            object = (Base*) create( name );
            }
         }
      }
   else
      {
      // not a PERS block, assume it's a non-PERS block:
      object = (Base*) create( bs.getID() );
      }

   StreamIO *readStream = &bs;   // may not be reading object from block

   // if create failed, it wasn't a block type what we know about.
   // So, assume it's a registered Untagged type and try id functions:
   
   if( !*err && !object )
      {
      // start id function at start of block:
      stream.setPosition(startPosition);
      object = (Base*) create( stream, &user );

      if( object )
         {
			// We didn't use the block stream afterall, gracefully
			// close it so that it doesn't set the real stream position
			// to garbage
         // NOTE: this also, resets the stream start read function at start of block:
			bs.undoOpen();
         //stream.setPosition(startPosition);
         AssertFatal( !dynamic_cast<VersionedBase*>(object),
            "Error:VersionedBase used as base class for non-block class" );
         readStream = &stream;
         }
      else
         {
         // if we haven't identified it yet, we won't:
         *err = UnregisteredClassError;
         }
      }

   // if using a dictionary, add object to dictionary
   if ( dictionary )
      dictionary->push_back( (void*)object );

   if( !*err )
   {
      // read version# (if versioning used):
      int version = object->readVersion( *readStream, err );

      // finally, read the object:
      if( !*err )
      {
         *err = object->read( *readStream, version, user );

         // get rid of object if read failed:
         if( *err )
         {
            delete object;
            object = 0;
         }
      }
   }

   return object;
}

AbstractTaggedClass *Base::getClass()
{
   return NULL;
}

//--------------------------------------------------------------------
// get the class name for this object
const char * Base::getClassName() const
{
   const char * name = typeid(*this).name();
   
   #ifdef _MSC_VER
   char * pos = strstr(name, "class ");
   if(pos)
      name += 6;
   #endif
   
   AssertFatal(strlen(name) < AbstractClass::MaxNameSize,
      "class name too long for persistent class");
      
   return(name);
}

//--------------------------------------------------------------------
// NOTE:  This const-callable function calls two non-const functions,
//  write() and version().  Currently, to get past this without breaking
//  user code, const on this is casted away to call these functions.  At
//  some point, version and write should probably be declared const as
//  well.  DMM 1/20/97
//
Base::Error Base::store( StreamIO& stream, int user ) const
   {
   // make sure the class was registered and
   // get the class tag and name:

   int tag;
   char const *name;

   // if using dictionary, check to see if object has already been written
   if ( dictionary )
   {
      for( int index=0; index<dictionary->size(); index++ )
         if ( (void *) this == (*dictionary)[index] ) 
         {
            tag = FOURCC('D','I','C','T');
            BlockRWStream bs;
            if( !bs.open( stream, tag, 0, FALSE ) )
               return OpenBlockError;
            if ( !bs.write( index ) )
               return WriteError;
            return Ok;
         }
      dictionary->push_back( (void*)this );
   }

   name = getClassName();
   if( !user && AbstractTaggedClass::lookupClass( name, &tag ) )
      {
      // a standard block object:
      // open the block stream:

      BlockRWStream bs;
      if( !bs.open( stream, tag, 0, FALSE ) )
         return OpenBlockError;

      // for PERS tag items, write the class name:

      if( tag == FOURCC('P','E','R','S') )
         {
         if( !bs.write( (short)strlen( name ) ) )
            return WriteError;
         else if( !bs.write( (strlen( name ) + 1) & (~1), name ) )
            return WriteError;
         }

      // write the version # (if versioning used):

      Error err = const_cast<Base *>(this)->writeVersion( bs );

      // write the actual object:

      if( !err )
         err = const_cast<Base *>(this)->write( bs, 
                                                const_cast<Base *>(this)->version(),
                                                USER_DEFAULT );

      return err;
      }
   else if( AbstractUntaggedClass::lookupClass( name, user ) )
      {
      // This is an untagged object, so don't write block header 
      // or version numbers!
      return const_cast<Base *>(this)->write( stream, 
                                              const_cast<Base *>(this)->version(),
                                              user );
      }
   else
      {
      // a class we don't know about!
      return UnregisteredClassError;
      }
   }

#pragma argsused  // stream is not used
int Base::readVersion( StreamIO& stream, Error *err )
{
   // default: don't read & write version; it's always 0
   *err = Ok;
   return 0;
}

#pragma argsused  // stream is not used
Base::Error Base::writeVersion( StreamIO& stream )
{
   // default: don't read & write version; it's always 0
   return Ok;
}

int Base::version() 
{
   // if no version method is provided by the derived class,
   // it will always be 0.
   return 0;
}

//--------------------------------------------------------------------
// VersionedBase class:
//--------------------------------------------------------------------

int VersionedBase::readVersion( StreamIO& stream, Error *err )
{
   *err = Ok;
   int version;
   if( !stream.read(&version) )
      *err = ReadError;
   return version;
}

Base::Error VersionedBase::writeVersion( StreamIO& stream )
{
   if( !stream.write(version()) )
      return WriteError;
   return Ok;
}

}; // namespace Persistent

DataFieldManager dataFieldManager;

void DataFieldManager::registerType(int type, int size, GetDataFunction gdf, SetDataFunction sdf)
{
   fnGetData[type] = gdf;
   fnSetData[type] = sdf;
   typeSizes[type] = size;
}

void DataFieldManager::setData(int type, void *dptr, int index, int argc, const char **argv)
{
   SetDataFunction fn = fnSetData[type];
   fn((void *) (DWORD(dptr) + index * typeSizes[type]),argc, argv);
}

const char *DataFieldManager::getData(int type, void *dptr, int index)
{
   GetDataFunction fn = fnGetData[type];
   return fn((void *) (DWORD(dptr) + index * typeSizes[type]));
}
