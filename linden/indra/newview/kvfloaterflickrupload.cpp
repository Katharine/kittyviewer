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

#include "llagent.h"
#include "llfloaterreg.h"
#include "llgl.h"
#include "llimage.h"
#include "llnotificationsutil.h"
#include "llui.h"
#include "lluploaddialog.h"
#include "llviewercontrol.h"
#include "llviewerregion.h"
#include "llviewertexture.h"
#include "llworld.h"

#include "kvflickr.h"
#include "kvfloaterflickrauth.h"

#include <boost/bind.hpp>

KVFloaterFlickrUpload::KVFloaterFlickrUpload(const LLSD& key) : LLFloater(key),
mCompressedImage(NULL),
mViewerImage(NULL)
{
}

KVFloaterFlickrUpload::~KVFloaterFlickrUpload()
{
	mCompressedImage = NULL;
	mViewerImage = NULL;
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

BOOL KVFloaterFlickrUpload::postBuild()
{
	// Set the various UI fields to their default values.
	childSetValue("rating_combo", gSavedSettings.getLLSD("KittyFlickrLastRating"));
	childSetValue("tags_form", gSavedSettings.getLLSD("KittyFlickrLastTags"));
	childSetValue("show_position_check", gSavedSettings.getLLSD("KittyFlickrShowPosition"));

	// Connect the buttons up
	childSetAction("cancel_btn", onClickCancel, this);
	childSetAction("upload_btn", onClickUpload, this);

	// Check that we actually can do an upload.
	LLSD query;
	query["auth_token"] = gSavedPerAccountSettings.getLLSD("KittyFlickrToken");
	KVFlickrRequest::request("flickr.auth.checkToken", query, boost::bind(&KVFloaterFlickrUpload::confirmToken, this, _1, _2));

	return true;
}

void KVFloaterFlickrUpload::confirmToken(bool success, const LLSD &response)
{
	if(!success)
	{
		LLNotificationsUtil::add("KittyFlickrHTTPFail");
		closeFloater(false);
		return;
	}
	if(response["stat"].asString() == "ok")
	{
		// Just in case the username changed. This can happen.
		std::string username = response["auth"]["user"]["username"];
		gSavedPerAccountSettings.setString("KittyFlickrUsername", username);
		childSetValue("account_name", username);
		childSetEnabled("upload_btn", true);
	}
	else
	{
		// Uh oh.
		if(response["code"].asInteger() == 98) // Invalid auth token
		{
			// Mark the account as invalid
			childSetValue("account_name", getString("no_account"));
			// Need to authenticate.
			gSavedPerAccountSettings.setString("KittyFlickrToken", "");
			gSavedPerAccountSettings.setString("KittyFlickrUsername", "");
			gSavedPerAccountSettings.setString("KittyFlickrNSID", "");
			KVFloaterFlickrAuth *floater = KVFloaterFlickrAuth::showFloater(boost::bind(&KVFloaterFlickrUpload::authCallback, this, _1));
			// Link it to us to protect it from freeze frame mode, if need be.
			if(floater && !gSavedSettings.getBOOL("CloseSnapshotOnKeep"))
			{
				gFloaterView->removeChild(floater);
				addChild(floater);
			}
			LLNotificationsUtil::add("KittyFlickrTokenRejected");
		}
		else
		{
			LLSD args;
			args["CODE"] = response["code"];
			args["ERROR"] = response["message"];
			LLNotificationsUtil::add("KittyFlickrGenericFail", args);
		}
	}
}

void KVFloaterFlickrUpload::authCallback(bool authorised)
{
	if(authorised)
	{
		childSetValue("account_name", gSavedPerAccountSettings.getString("KittyFlickrUsername"));
		childSetEnabled("upload_btn", true);
	}
	else
	{
		LLNotificationsUtil::add("KittyFlickrUploadCancelledAuthRejected");
		closeFloater(false);
	}
}

void KVFloaterFlickrUpload::saveSettings()
{
	gSavedSettings.setS32("KittyFlickrLastRating", childGetValue("rating_combo"));
	gSavedSettings.setString("KittyFlickrLastTags", childGetValue("tags_form"));
	gSavedSettings.setBOOL("KittyFlickrShowPosition", childGetValue("show_position_check"));
}

void KVFloaterFlickrUpload::uploadSnapshot()
{
	mTitle = childGetValue("title_form").asString();
	LLSD params;
	params["title"] = childGetValue("title_form");
	params["description"] = childGetValue("description_form");
	params["safety_level"] = childGetValue("rating_combo");
	std::string tags = childGetValue("tags_form");
	if(childGetValue("show_position_check").asBoolean())
	{
		// Work out where this was taken.
		LLVector3d clamped_global = LLWorld::getInstance()->clipToVisibleRegions(gAgent.getPositionGlobal(), mPosTakenGlobal);
		LLViewerRegion* region = LLWorld::getInstance()->getRegionFromPosGlobal(clamped_global);
		if(!region)
		{
			// Clamping failed? Shouldn't happen.
			// Use the agent's position instead; if the region the agent is in doesn't exist we have some serious issues,
			// and crashing is an entirely reasonable thing to do.
			region = gAgent.getRegion();
			clamped_global = gAgent.getPositionGlobal();
		}
		std::string region_name = region->getName();
		LLVector3 region_pos = region->getPosRegionFromGlobal(clamped_global);
		std::ostringstream region_tags;
		region_tags << " \"secondlife:region=" << region_name << "\"";
		region_tags << " secondlife:x=" << llround(region_pos[VX]);
		region_tags << " secondlife:y=" << llround(region_pos[VY]);
		region_tags << " secondlife:z=" << llround(region_pos[VZ]);
		tags += region_tags.str();
	}
	params["tags"] = tags;
	LL_INFOS("FlickrAPI") << "Uploading snapshot with metadata: " << params << LL_ENDL;

	params["auth_token"] = gSavedPerAccountSettings.getLLSD("KittyFlickrToken");
	LLUploadDialog::modalUploadDialog(getString("uploading"));
	KVFlickrRequest::uploadPhoto(params, mCompressedImage, boost::bind(&KVFloaterFlickrUpload::imageUploaded, this, _1, _2));
}

void KVFloaterFlickrUpload::imageUploaded(bool success, const LLSD& response)
{
	LLUploadDialog::modalUploadFinished();
	LLSD args;
	args["TITLE"] = mTitle;
	if(success)
	{
		args["ID"] = response["photoid"];
		LLNotificationsUtil::add("KittyFlickrUploadComplete", args);
	}
	else if(response.has("stat"))
	{
		args["CODE"] = response["code"];
		args["ERROR"] = response["msg"];
		LLNotificationsUtil::add("KittyFlickrUploadFailed", args);
	}
	else
	{
		LLNotificationsUtil::add("KittyFlickrUploadFailedNoError");
	}

	// We're pretty much done now.
	closeFloater(false);
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

//static
void KVFloaterFlickrUpload::onClickCancel(void* data)
{
	if(data)
	{
		KVFloaterFlickrUpload *self = (KVFloaterFlickrUpload*)data;
		self->closeFloater(false);
	}
}

//static
void KVFloaterFlickrUpload::onClickUpload(void* data)
{
	if(!data)
		return;
	KVFloaterFlickrUpload *self = (KVFloaterFlickrUpload*)data;
	self->uploadSnapshot();
	self->saveSettings();
	self->setVisible(false);
}
