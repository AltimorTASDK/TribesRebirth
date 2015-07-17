//
// bmlist.cpp
//

#include <gw.h>
#include "extras.h"
#include "bmlist.h"
#include "g_bitmap.h"
#include "g_barray.h"

////////////////////////////////////////////////////////////////////////

BitmapList::BitmapList()
{
	head  = tail  = current = NULL;
	count = width = height  = 0;
}

////////////////////////////////////////////////////////////////////////

BitmapList::~BitmapList()
{
	Zap();
}

////////////////////////////////////////////////////////////////////////

void BitmapList::Zap()
{
	Rewind();
	while (current)
	{
		delete current->bitmap;
		DeleteCurrent();
	}
	width = height = 0;
}

////////////////////////////////////////////////////////////////////////

void BitmapList::SetBitmapArray(GFXBitmapArray *bitmapArray)
{
	Zap();
	for (int i = 0; i < bitmapArray->numBitmaps; i++)
		Append(bitmapArray->array[i]);

	// clear numBitmaps in the array object so that the client can
	// destroy the array w/o us losing all the bitmaps
	bitmapArray->numBitmaps = 0;
}

////////////////////////////////////////////////////////////////////////

GFXBitmap *BitmapList::GetCurrent()
{
   	if (current)
		return(current->bitmap);
	else
		return(NULL);
}

////////////////////////////////////////////////////////////////////////

void BitmapList::SetCurrent(int index)
{
	Rewind();
	for (int i = 0; i < index; i++)	Skip();
}

////////////////////////////////////////////////////////////////////////

GFXBitmap *BitmapList::Skip()
{
	if (current)
		current = current->next;
	return(GetCurrent());
}

////////////////////////////////////////////////////////////////////////

void BitmapList::Insert(GFXBitmap *newBitmap)
{
	BitmapListNode *newNode = new BitmapListNode;
	newNode->bitmap = newBitmap;

	if (!head)
	{
		// inserting into empty list
		head = tail = current = newNode;
		count = 1;
	}
	else
	{
		newNode->next = current;
		if (current)
		{
			// inserting into the middle or front
			newNode->prev = current->prev;
			newNode->next = current;
			if (current == head)
			{
				// inserting at the very front
				head = newNode;
				current->prev = newNode;
			}
			else
				current->prev->next = newNode;
			current->prev = newNode;
		}
		else
		{
			// appending onto the very end
			newNode->prev = tail;
			tail->next    = newNode;
			tail          = newNode;
		}
		current = newNode;
		count++;
	}
	width  = max(width,  newBitmap->width);
	height = max(height, newBitmap->height);
}

////////////////////////////////////////////////////////////////////////

void BitmapList::Append(GFXBitmap *newBitmap)
{
	// append to the end by setting current to null
	current = NULL;
	Insert(newBitmap);
}

////////////////////////////////////////////////////////////////////////

int BitmapList::DeleteCurrent()
{
	// Remove the current node from the list but DO NOT delete the bitmap
	if (!current)
		return(0);

	BitmapListNode *tmp = current;
	if (current == head)
		head = current->next;
	else
		current->prev->next = current->next;

	if (current == tail)
		tail = current->prev;
	else
		current->next->prev = current->prev;
	
	current = current->next;
	delete tmp;
	count--;

	return(count);
}

////////////////////////////////////////////////////////////////////////

GFXBitmap *BitmapList::GetItem(int index)
{
	SetCurrent(index);
	return(GetCurrent());
}

////////////////////////////////////////////////////////////////////////

void BitmapList::ReEvaluateDimensions()
{
	width = height = -1;
	Rewind();
	while (current)
	{
		width  = max(width,  current->bitmap->width);
		height = max(height, current->bitmap->height);	
		Skip();
	}
}	

////////////////////////////////////////////////////////////////////////

void BitmapList::Promote(int index)
{
	// Swap the indexed item with its predecessor, inefficiencies due
	// to absolute indexing on the LL, could be fixed with uglier code
	if (index <= 0)
		return;

	SetCurrent(index);
	GFXBitmap *tmp = current->bitmap;
	DeleteCurrent();
	SetCurrent(index - 1);
	Insert(tmp);
}

////////////////////////////////////////////////////////////////////////

void BitmapList::Demote(int index)
{
	if ((index == -1) || (index == count - 1))
		return;

	SetCurrent(index);
	GFXBitmap *tmp = current->bitmap;
	DeleteCurrent();
	if (index == count - 1)
		Append(tmp);
	else
	{
		Skip();
		Insert(tmp);
	}
}
