/** 
 * @file lldlinked.h
 * @brief Declaration of the LLDLinked class.
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
#ifndef LL_LLDLINKED_H
#define LL_LLDLINKED_H

template <class Type> class LLDLinked
{
	LLDLinked* mNextp;
	LLDLinked* mPrevp;
public:

	Type*   getNext()  { return (Type*)mNextp; }
	Type*   getPrev()  { return (Type*)mPrevp; }
	Type*   getFirst() { return (Type*)mNextp; }

	void    init()
	{
		mNextp = mPrevp = NULL;
	}

	void    unlink()
	{
		if (mPrevp) mPrevp->mNextp = mNextp;
		if (mNextp) mNextp->mPrevp = mPrevp;
	}

	 LLDLinked() { mNextp = mPrevp = NULL; }
	virtual ~LLDLinked() { unlink(); }

	virtual void    deleteAll()
	{
		Type *curp = getFirst();
		while(curp)
		{
			Type *nextp = curp->getNext();
			curp->unlink();
			delete curp;
			curp = nextp;
		}
	}

	void relink(Type &after)
	{
		LLDLinked *afterp = (LLDLinked*)&after;
		afterp->mPrevp = this;
		mNextp = afterp;
	}

	virtual void    append(Type& after)
	{
		LLDLinked *afterp = (LLDLinked*)&after;
		afterp->mPrevp    = this;
		afterp->mNextp    = mNextp;
		if (mNextp) mNextp->mPrevp = afterp;
		mNextp            = afterp;
	}

	virtual void    insert(Type& before)
	{
		LLDLinked *beforep = (LLDLinked*)&before;
		beforep->mNextp    = this;
		beforep->mPrevp    = mPrevp;
		if (mPrevp) mPrevp->mNextp = beforep;
		mPrevp             = beforep;
	}

	virtual void    put(Type& obj) { append(obj); }
};

#endif
