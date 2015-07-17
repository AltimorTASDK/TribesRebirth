#ifndef PERSIST_H_
#define PERSIST_H_

#include <string.h>
#include <stdio.h>
#include <types.h>   // must include this before typeinfo.h for MSVC
#include <typeinfo.h>
#include <blkstrm.h>
#include <memstrm.h>
#include <tVector.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

namespace Persistent
{

//--------------------------------------------------------------------
// classes used to register Persistent classes:
//--------------------------------------------------------------------

const int USER_DEFAULT = 0;

//--------------------------------------------------------------------
// AbstractClass: a base class to implement name unmangling to insure
//                compiler independence.
//--------------------------------------------------------------------

class DLLAPI AbstractClass
{
  public:
   enum constants {
      MaxNameSize = 128,
      FastTableSize = 2048,
   };

   //---------------------------------------------------------------
   // to mangle class names for compiler independence:
   static char *   __cdecl convertClassName( char const *rawClassName, 
                                            char *className );

  protected:

   //---------------------------------------------------------------
   // virtual methods created by template instantiation below:
   
   virtual void * create() const = 0;
   virtual void   dump()   const = 0;
   virtual void   initFields() const = 0;

   //---------------------------------------------------------------
   // each registered class has a a compiler independent
   // mangled class name:
   
   char           name[MaxNameSize];
};

//--------------------------------------------------------------------
// AbstractTaggedClass: classes identified by a block header: id, size
//
// The default id for these classes is PERS.  Classes with this
// id store the class name immediately following the header.
// The class name is stored as: a word containing the length of the
// name; the actual name, padded as necessary to maintain word
// alignment.
//--------------------------------------------------------------------

class DLLAPI AbstractTaggedClass : public AbstractClass
{
  public:

   //---------------------------------------------------------------
   // find a registered class by tag, and return its stuff:
   
	static AbstractTaggedClass* lookupClass(int tag);
   static Bool lookupClass( char const *className, int *tag );
   static const char *lookupClassName(int tag);

   struct Field
   {
      const char *fieldName;
      int type;
      int elementCount;
      int offset;
      int ranking;
   };

   Field *fields;
   int numFields;
   AbstractTaggedClass();
  protected:
   ~AbstractTaggedClass();

   //---------------------------------------------------------------
   // virtual methods created by template instantiation below:
   
   virtual void * create() const = 0;
   virtual void   dump()   const = 0;
	void linkTag();

   //---------------------------------------------------------------
   // maintain a linked list of registered tagged classes:

   static AbstractTaggedClass* fastTable[];   
   static AbstractTaggedClass* head;
   AbstractTaggedClass*        next;

   //---------------------------------------------------------------
   // each registered tagged class has a id tag
   
   int            tag;
   int            tag2;

   //---------------------------------------------------------------
   // 
  public:
   //---------------------------------------------------------------
   // Interface used by Persistent Base class to create objects:
   
   static void* create( char const *className );
   static void* create( int classTag );
   static void  initPersistFields();

   //---------------------------------------------------------------
   // for debugging, a way to see what classes are registered:

   static void dumpClasses();

   int getTag() { return tag2 ? tag2 : tag; }
};

//--------------------------------------------------------------------
// AbstractUntaggedClass: classes not identified by a block header.  
//
// These classes must be identified by a function (of type idFunction) 
// which inspects the object (via the stream) and determines if it is 
// of the correct type.
//--------------------------------------------------------------------

class AbstractUntaggedClass : public AbstractClass
{
  public:
   //---------------------------------------------------------------
   // find a registered class, and return its stuff:
   
   static Bool    lookupClass( char const *className, int user );

  protected:
   ~AbstractUntaggedClass();

   //---------------------------------------------------------------
   // virtual methods created by template instantiation below:
   
   virtual void * create() const = 0;
   virtual void   dump() const = 0;
	void linkTag();

   //---------------------------------------------------------------
   // maintain a linked list of registered classes:
   
   static AbstractUntaggedClass* head;
   AbstractUntaggedClass*        next;

   //---------------------------------------------------------------
   // each registered class has an id function which reads from the
   // stream as much as necessary to determine if an object of the
   // correct type is present, and returns TRUE if so, else FALSE.

   typedef bool (*idFunction) (StreamIO &);
   idFunction     idFunc;
	int				user;

  public:
   //---------------------------------------------------------------
   // Interface used by Persistent Base class to create objects:
   
   static void*  create( StreamIO &stream, int *user );

   //---------------------------------------------------------------
   // for debugging, a way to see what classes are registered:

   static void  dumpClasses();
   };

//--------------------------------------------------------------------
// functions for creating registered Persistent classes:
//--------------------------------------------------------------------

inline void* create( const char *className )
{
   return AbstractTaggedClass::create( className );
}

inline void* create( int classTag )
{ 
   return AbstractTaggedClass::create( classTag );
}

inline void* create( StreamIO& stream, int *user )
{ 
   return AbstractUntaggedClass::create( stream, user );
}

//--------------------------------------------------------------------
// a template to specialize TaggedClass to specific classes:
//--------------------------------------------------------------------

template <class t>
class TaggedClass : public AbstractTaggedClass
{
  public:
   TaggedClass( int theTag = FOURCC('P','E','R','S'), int theTag2 = 0 )
   {
      AssertFatal( theTag, "Attempt to register persistent class with zero tag" );
	   AssertFatal( (theTag == FOURCC('P','E','R','S') ||
	   	           lookupClass( theTag ) == 0),
                   "Tagged persist class already defined");
      tag = theTag;
      tag2 = theTag2;
	   linkTag();
      convertClassName( typeid( t ).name(), name );
   }
   void initFields() const
   {
#ifdef _MSC_VER      
      // microsoft compiler error
      void (*fp)() = t::initPersistFields;
      (*fp)();
#else
      t::initPersistFields();
#endif
   }
   void* create() const
   {
      return (void *)(new t);
   }
   void  dump()   const
   {
      printf( "%8d %s\n", tag, name );
   }
};

//--------------------------------------------------------------------
// a template to specialize UntaggedClass to specific classes:
//--------------------------------------------------------------------

template <class t>
class UntaggedClass : public AbstractUntaggedClass
{
  public:
   UntaggedClass( idFunction theIdFunc, int userval )
   {
      AssertFatal( theIdFunc, "Attempt to register persistent class with null id function" );
      idFunc = theIdFunc;
		user = userval;
		linkTag();
      convertClassName( typeid( t ).name(), name );
   }

   void* create() const
   {
      return (void *)(new t);
   }
   void  dump()   const
   {
      printf( "NoTag %s\n", name );
   }
};

//--------------------------------------------------------------------
// Base: Persistent object Base class:
//--------------------------------------------------------------------

class DLLAPI Base
{
  public:
   static VectorPtr<void*> *dictionary;
   enum Error {
      Ok = 0,
      OpenBlockError,
      UnregisteredClassError,
      ReadError,
      WriteError,
      NotBase,
      InvalidName,
      IncorrectTypeFound,  // for use with nested objects
   };

   //---------------------------------------------------------------
   // Objects may be loaded from or stored to a stream.

  public:
   //---------------------------------------------------------------
   // These two methods turn on/off object dictionary construction 
   // while writing persistant objects.

   static void     openDictionary();
   static void     closeDictionary();

   static Base *   fileLoad( const char *name, Error *err );
   Error           fileStore( const char *name, int user = 0 ) const;

   static Base *   load( StreamIO& stream, Error *err );
   Error           store( StreamIO& stream, int user = 0 ) const;

   //---------------------------------------------------------------
   // These two methods need to be defined by derived classes:
   // that are written to disk

  protected:

   virtual Error read( StreamIO &, int version, int user = 0 );
   virtual Error write( StreamIO &, int version, int user = 0 );

   static void addField(const char *fieldName, int type, int offset, int count=1);

   //---------------------------------------------------------------
   // This method should be defined by derived classes if versioning
   // is desired:

  protected:
   virtual int version();  // default always returns 0

   // 
   //---------------------------------------------------------------

   //---------------------------------------------------------------
   // some helper functions.  ReadVersion and WriteVersion may be
   // overridden in derived classes if a different method of versioning
   // is needed, as with the class VersionedBase.
public:
   const char * getClassName() const;
   int getPersistTag();
   void getFieldList(int &fieldCount, AbstractTaggedClass::Field *& fields);
   static void initPersistFields();

protected:
   virtual int   readVersion( StreamIO& stream, Error *err );
   virtual Error writeVersion( StreamIO& stream );
   virtual AbstractTaggedClass *getClass();

   // 
   //---------------------------------------------------------------
};

inline void Base::getFieldList(int &fieldCount, AbstractTaggedClass::Field *& fields)
{
   fields = getClass()->fields;
   fieldCount = getClass()->numFields;
}

inline int Base::getPersistTag()
{
   AssertFatal(getClass() != NULL, "Cannot get the persist tag on a non-persist declared class.");
   return getClass()->getTag();
}

//--------------------------------------------------------------------
// VersionedBase: Persistent object base class, with versioning:
//
// This class extends the Base class by storing a version number, as
// an integer, following the header (standard block header, plus class
// name if present).  Most persistent classes should be derived from
// VersionedBase, rather than from Base.
//--------------------------------------------------------------------

class DLLAPI VersionedBase : public Base
{
  protected:
   int   readVersion( StreamIO& stream, Error *err );
   Error writeVersion( StreamIO& stream );
};

//--------------------------------------------------------------------
// some macros to make life easier:
//--------------------------------------------------------------------

#define DECLARE_PERSISTENT(aClass) \
   static Persistent::TaggedClass<aClass> t; \
   virtual Persistent::AbstractTaggedClass *getClass()
#define IMPLEMENT_PERSISTENT(aClass) \
   Persistent::AbstractTaggedClass * aClass::getClass() { return &aClass::t; } \
   Persistent::TaggedClass<aClass> aClass::t
#define IMPLEMENT_PERSISTENT_TAG(aClass,tag) \
   Persistent::AbstractTaggedClass * aClass::getClass() { return &aClass::t; } \
   Persistent::TaggedClass<aClass> aClass::t(tag)
#define IMPLEMENT_PERSISTENT_TAGS(aClass,tag1,tag2) \
   Persistent::AbstractTaggedClass * aClass::getClass() { return &aClass::t; } \
   Persistent::TaggedClass<aClass> aClass::t(tag1,tag2)
#define DECLARE_PERSISTENT_IDFUNC(aClass) \
   static Persistent::UntaggedClass<aClass> tf
#define IMPLEMENT_PERSISTENT_IDFUNC(aClass,func, user) \
   Persistent::UntaggedClass<aClass> aClass::tf(func, user)

#define	Offset(x,cls) (int)&(((cls *)0)->x)
//--------------------------------------------------------------------
}; // namespace Persistent

class DataFieldManager
{
public:
   typedef const char *(*GetDataFunction)(void *dptr);
   typedef void (*SetDataFunction)(void *dptr, int argc, const char **argv);
private:
   enum
   {
      MaxTypes = 256,
   };

   int typeSizes[MaxTypes];
   GetDataFunction fnGetData[MaxTypes];
   SetDataFunction fnSetData[MaxTypes];
public:
   void registerType(int type, int size, GetDataFunction gdf, SetDataFunction sdf);
   void setData(int type, void *dptr, int index, int argc, const char **argv);
   const char *getData(int type, void *dptr, int index);
};

extern DataFieldManager dataFieldManager;

enum CoreTypes
{
   TypeString,
   TypeCaseString,
   TypeInt,
   TypeFloat,
   TypeBool,
   TypePoint3F,
   TypeColorF,
   TypeTMat3F,
   TypeBox3F,
   NumCoreTypes,
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif
