/*  Copyright  2005  jinti jinti.shen@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _LRULIST_HXX_
#define _LRULIST_HXX_
#include <stdint.h>

 
namespace FastCPP {

   class LruList
   {
      public:
	 class iterator;

	 class BaseLruItem
	 {
	    public:
	       BaseLruItem(LruList &list):mRemove(false),
					  mBack(0),
					  mNext(0),
					  mLrulist(list)		  
	       {
		  mLrulist.push_front(this);
	       }
	       virtual ~BaseLruItem()
	       {
		  if (!mRemove)
		  {
		     mLrulist.remove(this);
		  }
	       }
	   protected:
	       bool mRemove;
	       BaseLruItem *mBack;
	       BaseLruItem *mNext;
	       LruList &mLrulist;
	       friend class LruList;
	       friend class iterator;
	 }; 
	 class iterator
	 {
	    public:
	       explicit iterator(BaseLruItem * start)
		  : mPos(start)
	       {}

	       iterator& operator=(const iterator& rhs)
	       {
		  mPos = rhs.mPos;
		  return *this;
	       }

	       iterator& operator++(int)
	       {
		  mPos = mPos->mNext;
		  return *this;
	       }
	       iterator& operator--(int)
	       {
		  mPos = mPos->mBack;
		  return *this;
	       }
	       bool operator==(const iterator& rhs)
	       {
		  return mPos == rhs.mPos;
	       }

	       bool operator!=(const iterator& rhs)
	       {
		  return mPos != rhs.mPos;
	       }

	       BaseLruItem * operator*()
	       {
		  return mPos;
	       }

	    private:
	       BaseLruItem * mPos;
	 };
      public:
	 LruList():
	    mHead(0),
	    mTail(0),
	    mSize(0)
	 {}
	 
	 iterator begin()
	 {
	    return iterator(mHead);
	 }

	 iterator end()
	 {
	    return iterator(mTail);
	 }

	 uint32_t size()
	 {
	    return mSize;
	 }
	 bool empty()
	 {
	    return mSize == 0 ? true:false; 
	 }
	 
	 void remove(BaseLruItem *item)
	 {
	    item->mRemove = true;
	    if(item == mTail)
	    {
	       mTail = item->mBack;
	    }
	    if(item == mHead)
	    {
	       mHead = item->mNext;
	    } 

	    if(item->mBack)
	    {
	       item->mBack->mNext = item->mNext;
	    }
	    if(item->mNext)
	    {
	       item->mNext->mBack = item->mBack;
	    }
	    item->mBack = 0;
	    item->mNext = 0;
	    mSize --;	    
	 }
	 
	 void update(BaseLruItem *item)
	 {

	    if(item == mHead)
	    {
	       return;
	    }

	    if(item == mTail)
	    {
	       mTail = item->mBack;
	    }
	    if(item->mBack)
	    {
	       item->mBack->mNext = item->mNext;
	    }
	    if(item->mNext)
	    {
	       item->mNext->mBack = item->mBack;
	    }
	    item->mBack = 0;
	    item->mNext = 0;
	    if (mSize == 0)
	    {
	       mHead = item;
	       mTail = item;
	    }
	    else
	    {
	       mHead->mBack = item;
	       item->mNext = mHead;	       
	       mHead = item;
	    } 
	 }
	 void push_front(BaseLruItem *item)
	 {
	    if (mHead == 0)
	    {
	       mHead = item;
	       mTail = item;
	    }
	    else
	    {
	       mHead->mBack = item;
	       item->mNext = mHead;	       
	       mHead = item;
	    } 
	    mSize++;
	 }

	 
	 void pop_back()
	 {
	    BaseLruItem *tail = mTail;

	    if (tail)
	    {
	       delete tail;
	    }
	 }
	 BaseLruItem *tail()
	 {
	    return mTail;
	 }
      protected:	 
	 BaseLruItem *mHead;
	 BaseLruItem *mTail;
	 uint32_t mSize;
	 friend class BaseLruItem;
   };
};
#endif //_LRULIST_HXX_
