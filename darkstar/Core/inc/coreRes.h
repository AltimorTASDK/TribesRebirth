//------------------------------------------------------------------------------
class RawData
{
private:
   bool ownMemory;

public:
   char *data;
   int size;

   RawData()  { ownMemory = false; }
   RawData(StreamIO &s, int sz)  
   {  
      data = (char*)s.lock();
      size = sz;
      ownMemory = (data == NULL); 
      if (ownMemory)
      {
         data = new char[sz];
         if (data) s.read(sz, data);
      }
   }
   ~RawData() { if (ownMemory) delete [] data; }
};   


//-------------------------------------- RawData type
class ResourceTypeRawData : public ResourceType
{
public:   
   ResourceTypeRawData(const char *ext = ".dat"):
      ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream, int size) 
      { return (void*)new RawData(*stream, size); }
   void destruct(void *p) 
      { delete (RawData*)p; }
};   

class ResourceTypeStaticRawData : public ResourceType
{
public:   
   ResourceTypeStaticRawData(const char *ext = ".sdt"):
      ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream, int size) 
      { return (void*)new RawData(*stream, size); }
   void destruct(void *p) 
      { }
};   

