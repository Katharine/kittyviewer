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

#include "llappviewer.h"
#include "lldir.h"
#include "llfile.h"
#include "llfocusmgr.h"
#include "llimview.h"
#include "llnotifications.h"
#include "llsd.h"
#include "llsdserialize.h"
#include "llstartup.h"
#include "llviewercontrol.h"
#include "llviewerwindow.h"
#include "llwindow.h"

#include "kvgrowlmanager.h"
#include "kvgrowlnotifier.h"

// Platform-specific includes
// (None yet)

KVGrowlManager *gGrowlManager = NULL;

KVGrowlManager::KVGrowlManager() : LLEventTimer(GROWL_THROTTLE_CLEANUP_PERIOD)
{
	// Create a notifier appropriate to the platform.
#if 1
	this->mNotifier = new KVGrowlNotifier();
	LL_INFOS("GrowlInit") << "Created generic KVGrowlNotifier." << LL_ENDL;
#endif
	
	// Don't do anything more if Growl isn't usable.
	if(!mNotifier->isUsable())
	{
		LL_WARNS("GrowlInit") << "Growl is unusable; bailing out." << LL_ENDL;
		return;
	}
	
	// Hook into LLNotifications...
	// We hook into all of them, even though (at the time of writing) nothing uses "alert", so more notifications can be added easily.
	LLNotificationChannel::buildChannel("KVGrowlNotifications", "Visible", LLNotificationFilters::includeEverything);
	LLNotifications::instance().getChannel("KVGrowlNotifications")->connectChanged(&KVGrowlManager::onLLNotification);
	
	// Also hook into IM notifications.
	LLIMModel::instance().mNewMsgSignal.connect(&KVGrowlManager::onInstantMessage);
	
	this->loadConfig();
}

void KVGrowlManager::loadConfig()
{
	std::string config_file = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS, "growl_notifications.xml");
	if(config_file == "")
	{
		LL_WARNS("GrowlConfig") << "Couldn't find growl_notifications.xml" << LL_ENDL;
		return;
	}
	LL_INFOS("GrowlConfig") << "Loading growl notification config from " << config_file << LL_ENDL;
	llifstream configs(config_file);
	LLSD notificationLLSD;
	std::set<std::string> notificationTypes;
	notificationTypes.insert(GROWL_IM_MESSAGE_TYPE);
	if(configs.is_open())
	{
		LLSDSerialize::fromXML(notificationLLSD, configs);
		for(LLSD::map_iterator itr = notificationLLSD.beginMap(); itr != notificationLLSD.endMap(); ++itr)
		{
			KVGrowlNotification ntype;
			ntype.growlName = itr->second.get("GrowlName").asString();
			notificationTypes.insert(ntype.growlName);
			
			if(itr->second.has("GrowlTitle"))
				ntype.growlTitle = itr->second.get("GrowlTitle").asString();			
			if(itr->second.has("GrowlBody"))
				ntype.growlBody = itr->second.get("GrowlBody").asString();
			if(itr->second.has("UseDefaultTextForTitle"))
				ntype.useDefaultTextForTitle = itr->second.get("UseDefaultTextForTitle").asBoolean();
			else
				ntype.useDefaultTextForTitle = false;
			if(itr->second.has("UseDefaultTextForBody"))
				ntype.useDefaultTextForBody = itr->second.get("UseDefaultTextForBody").asBoolean();
			else
				ntype.useDefaultTextForBody = false;
			if(ntype.useDefaultTextForBody == false && ntype.useDefaultTextForTitle == false && 
			   ntype.growlBody == "" && ntype.growlTitle == "")
			{
				ntype.useDefaultTextForBody = true;
			}
			this->mNotifications[itr->first] = ntype;
		}
		configs.close();
		
		this->mNotifier->registerApplication(LLAppViewer::instance()->getSecondLifeTitle(), notificationTypes);
	}
	else
	{
		LL_WARNS("GrowlConfig") << "Couldn't open growl config file." << LL_ENDL;
	}
	
}

void KVGrowlManager::notify(const std::string& notification_title, const std::string& notification_message, const std::string& notification_type)
{
	if(!shouldNotify())
		return;
	
	if(!gSavedSettings.getBOOL("KittyEnableGrowl"))
		return;
	
	if(this->mNotifier->needsThrottle())
	{
		U64 now = LLTimer::getTotalTime();
		if(mTitleTimers.find(notification_title) != mTitleTimers.end())
		{
			if(mTitleTimers[notification_title] > now - GROWL_THROTTLE_TIME)
			{
				LL_WARNS("GrowlNotify") << "Discarded notification with title '" << notification_title << "' - spam." << LL_ENDL;
				mTitleTimers[notification_title] = now;
				return;
			}
		}
		mTitleTimers[notification_title] = now;
	}
	this->mNotifier->showNotification(notification_title, notification_message.substr(0, GROWL_MAX_BODY_LENGTH), notification_type);
}

BOOL KVGrowlManager::tick()
{
	mTitleTimers.clear();
	return false;
}

//static
bool KVGrowlManager::onLLNotification(const LLSD& notice)
{
	if(notice["sigtype"].asString() != "add")
		return false;
	if(!shouldNotify())
		return false;
	LLNotificationPtr notification = LLNotifications::instance().find(notice["id"].asUUID());
	std::string name = notification->getName();
	LLSD substitutions = notification->getSubstitutions();
	if(LLStartUp::getStartupState() < STATE_STARTED)
	{
		LL_WARNS("GrowlLLNotification") << "GrowlManager discarded a notification (" << name << ") - too early." << LL_ENDL;
		return false;
	}
	if(gGrowlManager->mNotifications.find(name) != gGrowlManager->mNotifications.end())
	{
		KVGrowlNotification* growl_notification = &gGrowlManager->mNotifications[name];
		std::string body = "";
		std::string title = "";
		if(growl_notification->useDefaultTextForTitle)
			title = notification->getMessage();
		else if(growl_notification->growlTitle != "")
			title = growl_notification->growlTitle;
			LLStringUtil::format(title, substitutions);
		if(growl_notification->useDefaultTextForBody)
			body = notification->getMessage();
		else if(growl_notification->growlBody != "")
			body = growl_notification->growlBody;
			LLStringUtil::format(body, substitutions);
		gGrowlManager->notify(title, body, growl_notification->growlName);
	}
	return false;
}

//static
void KVGrowlManager::onInstantMessage(const LLSD& im)
{
	std::string message = im["message"];
	std::string prefix = message.substr(0, 4);
	if(prefix == "/me " || prefix == "/me'")
	{
		message = message.substr(3);
	}
	gGrowlManager->notify(im["from"], message, GROWL_IM_MESSAGE_TYPE);
}


//static
bool KVGrowlManager::shouldNotify()
{
	// This magic stolen from llappviewer.cpp. LLViewerWindow::getActive lies.
	return ((!gViewerWindow->mWindow->getVisible() || !gFocusMgr.getAppHasFocus()) || gSavedSettings.getBOOL("KittyGrowlWhenActive"));
}

//static
void KVGrowlManager::InitiateManager()
{
	gGrowlManager = new KVGrowlManager();
}

