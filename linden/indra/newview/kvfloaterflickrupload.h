/* Copyright (c) 2010 Katharine Berry All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *   3. Neither the name Katharine Berry nor the names of any contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY KATHARINE BERRY AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KATHARINE BERRY OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KV_KVFLOATERFLICKRUPLOAD_H
#define KV_KVFLOATERFLICKRUPLOAD_H

#include "llfloater.h"
#include "llcheckboxctrl.h"

#include "llpointer.h"

class LLViewerTexture;
class LLImageFormatted;

class KVFloaterFlickrUpload : public LLFloater
{
public:
	KVFloaterFlickrUpload(const LLSD& key);
	~KVFloaterFlickrUpload();

	BOOL postBuild();
	void draw();
	void saveSettings();
	void uploadSnapshot();

	void confirmToken(bool success, const LLSD &response);
	void authCallback(bool authorised);
	void imageUploaded(bool success, const LLSD& response);

	static void onClickCancel(void* data);
	static void onClickUpload(void* data);
	static KVFloaterFlickrUpload* showFromSnapshot(LLImageFormatted *compressed, LLViewerTexture *img, const LLVector2& img_scale, const LLVector3d& pos_taken_global);

private:
	LLPointer<LLImageFormatted> mCompressedImage;
	LLPointer<LLViewerTexture> mViewerImage;
	LLVector2 mImageScale;
	LLVector3d mPosTakenGlobal;
};


#endif // KV_KVFLOATERFLICKRUPLOAD_H
