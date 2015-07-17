// Simple FIFO linked-list class

#ifndef _H_TFIFO
#define _H_TFIFO

template <typename T> class FIFO
{
   private :
      T   *head, *tail;
      int  iSize;

   public :
      FIFO()
      {
         iSize = 0;
         head  = tail = NULL;
      }

      inline const T *getHead() 
      {
         return (head);
      }
      inline const T *getTail()
      {
         return (tail);
      }

      inline int size()
      {
         return (iSize);
      }

      void push(T *lpeNew)
      {
         if (tail)
         {
            tail->next = lpeNew;
            tail = tail->next;
         }

         else
         {
            head = tail = lpeNew;
         }

         iSize ++;
      }

      void pop()
      {
         T *tmp;

         if (iSize > 0)
         {
            if (tail == head)
            {
               tail = head->next;
            }

            tmp  = head;
            head = head->next;
            delete tmp;

            iSize --;
         }
      }
};

#endif