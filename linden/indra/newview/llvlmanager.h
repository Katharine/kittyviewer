/** 
 * @file llvlmanager.h
 * @brief LLVLManager class definition
 *
 * $LicenseInfo:firstyear=2002&license=viewergpl$
 * 
 * Copyright (c) 2002-2010, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 * 
 */

#ifndef LL_LLVLMANAGER_H
#define LL_LLVLMANAGER_H

// This class manages the data coming in for viewer layers from the network.

#include "stdtypes.h"
#include "lldarray.h"

class LLVLData;
class LLViewerRegion;

class LLVLManager
{
public:
	~LLVLManager();

	void addLayerData(LLVLData *vl_datap, const S32 mesg_size);

	void unpackData(const S32 num_packets = 10);

	S32 getTotalBytes() const;

	S32 getLandBits() const;
	S32 getWindBits() const;
	S32 getCloudBits() const;

	void resetBitCounts();

	void cleanupData(LLViewerRegion *regionp);
protected:

	LLDynamicArray<LLVLData *> mPacketData;
	U32 mLandBits;
	U32 mWindBits;
	U32 mCloudBits;
};

class LLVLData
{
public:
	LLVLData(LLViewerRegion *regionp,
			 const S8 type, U8 *data, const S32 size);
	~LLVLData();

	S8 mType;
	U8 *mData;
	S32 mSize;
	LLViewerRegion *mRegionp;
};

extern LLVLManager gVLManager;

#endif // LL_LLVIEWERLAYERMANAGER_H
