/** 
 * @file llassoclist.h
 * @brief LLAssocList class header file
 *
 * $LicenseInfo:firstyear=2001&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#ifndef LL_LLASSOCLIST_H
#define LL_LLASSOCLIST_H

//------------------------------------------------------------------------
// LLAssocList is an associative list container class.
//
// The implementation is a single linked list.
// Both index and value objects are stored by value (not reference).
// If pointer values are specified for index and/or value, this
// container does NOT assume ownership of the referenced objects,
// and does NOT delete() them on removal or destruction of the container.
//
// Note that operations are generally not optimized, and may of them
// are O(n) complexity.
//------------------------------------------------------------------------

#include <iostream>

template<class INDEX_TYPE, class VALUE_TYPE>
class LLAssocList
{
private:
	// internal list node type
	class Node
	{
	public:
		Node(const INDEX_TYPE &index, const VALUE_TYPE &value, Node *next)
		{
			mIndex = index;
			mValue = value;
			mNext = next;
		}
		~Node() { }
		INDEX_TYPE	mIndex;
		VALUE_TYPE	mValue;
		Node		*mNext;
	};

	// head of the linked list
	Node *mHead;

public:
	// Constructor
	LLAssocList()
	{
		mHead = NULL;
	}

	// Destructor
	~LLAssocList()
	{
		removeAll();
	}

	// Returns TRUE if list is empty.
	BOOL isEmpty()
	{
		return (mHead == NULL);
	}

	// Returns the number of items in the list.
	U32 length()
	{
		U32 count = 0;
		for (	Node *node = mHead;
				node;
				node = node->mNext )
		{
			count++;
		}
		return count;
	}

	// Removes item with the specified index.
	BOOL remove( const INDEX_TYPE &index )
	{
		if (!mHead)
			return FALSE;

		if (mHead->mIndex == index)
		{
			Node *node = mHead;
			mHead = mHead->mNext;
			delete node;
			return TRUE;
		}

		for (	Node *prev = mHead;
				prev->mNext;
				prev = prev->mNext )
		{
			if (prev->mNext->mIndex == index)
			{
				Node *node = prev->mNext;
				prev->mNext = prev->mNext->mNext;
				delete node;
				return TRUE;
			}
		}
		return FALSE;
	}

	// Removes all items from the list.
	void removeAll()
	{
		while ( mHead )
		{
			Node *node = mHead;
			mHead = mHead->mNext;
			delete node;
		}
	}

	// Adds a new item to the head of the list,
	// removing any existing item with same index.
	void addToHead( const INDEX_TYPE &index, const VALUE_TYPE &value )
	{
		remove(index);
		Node *node = new Node(index, value, mHead);
		mHead = node;
	}

	// Adds a new item to the end of the list,
	// removing any existing item with the same index.
	void addToTail( const INDEX_TYPE &index, const VALUE_TYPE &value )
	{
		remove(index);
		Node *node = new Node(index, value, NULL);
		if (!mHead)
		{
			mHead = node;
			return;
		}
		for (	Node *prev=mHead;
				prev;
				prev=prev->mNext )
		{
			if (!prev->mNext)
			{
				prev->mNext=node;
				return;
			}
		}
	}

	// Sets the value of a specified index.
	// If index does not exist, a new value will be added only if
	// 'addIfNotFound' is set to TRUE.
	// Returns TRUE if successful.
	BOOL setValue( const INDEX_TYPE &index, const VALUE_TYPE &value, BOOL addIfNotFound=FALSE )
	{
		VALUE_TYPE *valueP = getValue(index);
		if (valueP)
		{
			*valueP = value;
			return TRUE;
		}
		if (!addIfNotFound)
			return FALSE;
		addToTail(index, value);
		return TRUE;
	}

	// Sets the ith value in the list.
	// A new value will NOT be addded, if the ith value does not exist.
	// Returns TRUE if successful.
	BOOL setValueAt( U32 i, const VALUE_TYPE &value )
	{
		VALUE_TYPE *valueP = getValueAt(i);
		if (valueP)
		{
			*valueP = value;
			return TRUE;
		}
		return FALSE;
	}

	// Returns a pointer to the value for the specified index,
	// or NULL if no item found.
	VALUE_TYPE *getValue( const INDEX_TYPE &index )
	{
		for (	Node *node = mHead;
				node;
				node = node->mNext )
		{
			if (node->mIndex == index)
				return &node->mValue;
		}
		return NULL;
	}

	// Returns a pointer to the ith value in the list, or
	// NULL if i is not valid.
	VALUE_TYPE *getValueAt( U32 i )
	{
		U32 count = 0;
		for (	Node *node = mHead;
				node;
				node = node->mNext )
		{
			if (count == i)
				return &node->mValue;
			count++;
		}
		return NULL;
	}

	// Returns a pointer to the index for the specified index,
	// or NULL if no item found.
	INDEX_TYPE *getIndex( const INDEX_TYPE &index )
	{
		for (	Node *node = mHead;
				node;
				node = node->mNext )
		{
			if (node->mIndex == index)
				return &node->mIndex;
		}
		return NULL;
	}

	// Returns a pointer to the ith index in the list, or
	// NULL if i is not valid.
	INDEX_TYPE *getIndexAt( U32 i )
	{
		U32 count = 0;
		for (	Node *node = mHead;
				node;
				node = node->mNext )
		{
			if (count == i)
				return &node->mIndex;
			count++;
		}
		return NULL;
	}

	// Returns a pointer to the value for the specified index,
	// or NULL if no item found.
	VALUE_TYPE *operator[](const INDEX_TYPE &index)
	{
		return getValue(index);
	}

	// Returns a pointer to the ith value in the list, or
	// NULL if i is not valid.
	VALUE_TYPE *operator[](U32 i)
	{
		return getValueAt(i);
	}

	// Prints the list contents to the specified stream.
	friend std::ostream &operator<<( std::ostream &os, LLAssocList &map )
	{
		os << "{";
		for (	Node *node = map.mHead;
				node;
				node = node->mNext )
		{
			os << "<" << node->mIndex << ", " << node->mValue << ">";
			if (node->mNext)
				os << ", ";
		}
		os << "}";

		return os;
	}
};

#endif // LL_LLASSOCLIST_H
