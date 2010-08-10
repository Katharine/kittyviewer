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

#include "llviewerprecompiledheaders.h"

#include "kvfloaterflickrupload.h"

#include "llgl.h"

#include "llui.h"
#include "lllineeditor.h"
#include "llbutton.h"
#include "lltexteditor.h"
#include "llfloaterreg.h"
#include "llnotificationsutil.h"
#include "llviewercontrol.h"
#include "llviewernetwork.h"
#include "lluictrlfactory.h"
#include "lluploaddialog.h"
#include "llviewerregion.h"

#include "llimage.h"
#include "llviewertexture.h"
#include "llagentui.h"

KVFloaterFlickrUpload::KVFloaterFlickrUpload(const LLSD& key) : LLFloater(key),
mCompressedImage(NULL),
mViewerImage(NULL)
{
}

KVFloaterFlickrUpload::~KVFloaterFlickrUpload()
{
	mCompressedImage = NULL;
}

// static
KVFloaterFlickrUpload* KVFloaterFlickrUpload::showFromSnapshot(LLImageFormatted *compressed, LLViewerTexture *img, const LLVector2& img_scale, const LLVector3d& pos_taken_global)
{
	// Take the images from the caller
	// It's now our job to clean them up
	KVFloaterFlickrUpload* instance = LLFloaterReg::showTypedInstance<KVFloaterFlickrUpload>("flickr_upload", LLSD(img->getID()));
	
	instance->mCompressedImage = compressed;
	instance->mViewerImage = img;
	instance->mImageScale = img_scale;
	instance->mPosTakenGlobal = pos_taken_global;
	
	return instance;
}

// This function stolen from LLFloaterPostcard
void KVFloaterFlickrUpload::draw()
{
	LLGLSUIDefault gls_ui;
	LLFloater::draw();
	
	if(!isMinimized() && mViewerImage.notNull() && mCompressedImage.notNull()) 
	{
		LLRect rect(getRect());
		
		// first set the max extents of our preview
		rect.translate(-rect.mLeft, -rect.mBottom);
		rect.mLeft += 280;
		rect.mRight -= 10;
		rect.mTop -= 27;
		rect.mBottom = rect.mTop - 130;
		
		// then fix the aspect ratio
		F32 ratio = (F32)mCompressedImage->getWidth() / (F32)mCompressedImage->getHeight();
		if ((F32)rect.getWidth() / (F32)rect.getHeight() >= ratio)
		{
			rect.mRight = LLRect::tCoordType((F32)rect.mLeft + ((F32)rect.getHeight() * ratio));
		}
		else
		{
			rect.mBottom = LLRect::tCoordType((F32)rect.mTop - ((F32)rect.getWidth() / ratio));
		}
		{
			gGL.getTexUnit(0)->unbind(LLTexUnit::TT_TEXTURE);
			gl_rect_2d(rect, LLColor4(0.f, 0.f, 0.f, 1.f));
			rect.stretch(-1);
		}
		{
			
			glMatrixMode(GL_TEXTURE);
			glPushMatrix();
			{
				glScalef(mImageScale.mV[VX], mImageScale.mV[VY], 1.f);
				glMatrixMode(GL_MODELVIEW);
				gl_draw_scaled_image(rect.mLeft,
									 rect.mBottom,
									 rect.getWidth(),
									 rect.getHeight(),
									 mViewerImage.get(), 
									 LLColor4::white);
			}
			glMatrixMode(GL_TEXTURE);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		}
	}
}
