/** 
 * @file llnotificationtiphandler.cpp
 * @brief Notification Handler Class for Notification Tips
 *
 * $LicenseInfo:firstyear=2000&license=viewergpl$
 * 
 * Copyright (c) 2000-2010, Linden Research, Inc.
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


#include "llviewerprecompiledheaders.h" // must be first include

#include "llfloaterreg.h"
#include "llnearbychat.h"
#include "llnotificationhandler.h"
#include "llnotifications.h"
#include "lltoastnotifypanel.h"
#include "llviewercontrol.h"
#include "llviewerwindow.h"
#include "llnotificationmanager.h"
#include "llpaneltiptoast.h"

using namespace LLNotificationsUI;

//--------------------------------------------------------------------------
LLTipHandler::LLTipHandler(e_notification_type type, const LLSD& id)
{
	mType = type;	

	// Getting a Channel for our notifications
	mChannel = LLChannelManager::getInstance()->createNotificationChannel();

	LLScreenChannel* channel = dynamic_cast<LLScreenChannel*>(mChannel);
	if(channel)
		channel->setOnRejectToastCallback(boost::bind(&LLTipHandler::onRejectToast, this, _1));
}

//--------------------------------------------------------------------------
LLTipHandler::~LLTipHandler()
{
}

//--------------------------------------------------------------------------
void LLTipHandler::initChannel()
{
	S32 channel_right_bound = gViewerWindow->getWorldViewRectScaled().mRight - gSavedSettings.getS32("NotificationChannelRightMargin"); 
	S32 channel_width = gSavedSettings.getS32("NotifyBoxWidth");
	mChannel->init(channel_right_bound - channel_width, channel_right_bound);
}

//--------------------------------------------------------------------------
bool LLTipHandler::processNotification(const LLSD& notify)
{
	if(!mChannel)
	{
		return false;
	}

	LLNotificationPtr notification = LLNotifications::instance().find(notify["id"].asUUID());

	if(!notification)
		return false;	

	// arrange a channel on a screen
	if(!mChannel->getVisible())
	{
		initChannel();
	}

	if(notify["sigtype"].asString() == "add" || notify["sigtype"].asString() == "change")
	{
		// archive message in nearby chat
		if (LLHandlerUtil::canLogToNearbyChat(notification))
		{
			LLHandlerUtil::logToNearbyChat(notification, CHAT_SOURCE_SYSTEM);

			// don't show toast if Nearby Chat is opened
			LLNearbyChat* nearby_chat = LLFloaterReg::getTypedInstance<
					LLNearbyChat>("nearby_chat", LLSD());
			if (nearby_chat->getVisible())
			{
				return true;
			}
		}

		std::string session_name = notification->getPayload()["SESSION_NAME"];
		const std::string name = notification->getSubstitutions()["NAME"];
		if (session_name.empty())
		{
			session_name = name;
		}
		LLUUID from_id = notification->getPayload()["from_id"];
		if (LLHandlerUtil::canLogToIM(notification))
		{
			LLHandlerUtil::logToIM(IM_NOTHING_SPECIAL, session_name, name,
					notification->getMessage(), from_id, from_id);
		}

		if (LLHandlerUtil::canSpawnIMSession(notification))
		{
			LLHandlerUtil::spawnIMSession(name, from_id);
		}

		// don't spawn toast for inventory accepted/declined offers if respective IM window is open (EXT-5909)
		if (!LLHandlerUtil::canSpawnToast(notification))
		{
			return true;
		}

		LLToastPanel* notify_box = LLToastPanel::buidPanelFromNotification(notification);

		LLToast::Params p;
		p.notif_id = notification->getID();
		p.notification = notification;
		p.lifetime_secs = gSavedSettings.getS32("NotificationTipToastLifeTime");
		p.panel = notify_box;
		p.is_tip = true;
		p.can_be_stored = false;
		
		removeExclusiveNotifications(notification);

		LLScreenChannel* channel = dynamic_cast<LLScreenChannel*>(mChannel);
		if(channel)
			channel->addToast(p);
	}
	else if (notify["sigtype"].asString() == "delete")
	{
		mChannel->killToastByNotificationID(notification->getID());
	}
	return true;
}

//--------------------------------------------------------------------------
void LLTipHandler::onDeleteToast(LLToast* toast)
{
}

//--------------------------------------------------------------------------

void LLTipHandler::onRejectToast(const LLUUID& id)
{
	LLNotificationPtr notification = LLNotifications::instance().find(id);

	if (notification
			&& LLNotificationManager::getInstance()->getHandlerForNotification(
					notification->getType()) == this)
	{
		LLNotifications::instance().cancel(notification);
	}
}
