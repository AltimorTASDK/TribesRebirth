//
// bmlist.h
//

#ifndef _BMLIST_H_
#define _BMLIST_H_

class BitmapListNode
{
  public:
	GFXBitmap      *bitmap;
	BitmapListNode *prev, *next;

	BitmapListNode() { prev = next = NULL;  bitmap = NULL; }
};

class BitmapList
{
  private:
	BitmapListNode *head, *tail, *current;

  public:
	int width, height, count;

	BitmapList();
	~BitmapList();

	void       Zap();
	void       SetBitmapArray(GFXBitmapArray *bitmapArray);
	GFXBitmap *SetCurrent(int index);
	GFXBitmap *Skip();
	void       Insert(GFXBitmap *newBitmap);
	void       Append(GFXBitmap *newBitmap);
	GFXBitmap *GetCurrent();
	GFXBitmap *GetItem(int index);
	int        DeleteCurrent();
	void       Rewind() { current = head; }	
	void       ReEvaluateDimensions();
	void       Promote(int index);
	void       Demote(int index);
};

#endif