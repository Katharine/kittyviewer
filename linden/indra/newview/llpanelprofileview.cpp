/** 
* @file llpanelprofileview.cpp
* @brief Side tray "Profile View" panel
*
* $LicenseInfo:firstyear=2009&license=viewerlgpl$
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

#include "llviewerprecompiledheaders.h"

#include "llavatarconstants.h"
#include "lluserrelations.h"

#include "llpanelprofileview.h"

#include "llavatarpropertiesprocessor.h"
#include "llcallingcard.h"
#include "llpanelavatar.h"
#include "llpanelpicks.h"
#include "llpanelprofile.h"
#include "llsidetraypanelcontainer.h"

static LLRegisterPanelClassWrapper<LLPanelProfileView> t_panel_target_profile("panel_profile_view");

static std::string PANEL_NOTES = "panel_notes";
static const std::string PANEL_PROFILE = "panel_profile";
static const std::string PANEL_PICKS = "panel_picks";


class AvatarStatusObserver : public LLAvatarPropertiesObserver
{
public:
	AvatarStatusObserver(LLPanelProfileView* profile_view)
	{
		mProfileView = profile_view;
	}

	void processProperties(void* data, EAvatarProcessorType type)
	{
		if(APT_PROPERTIES != type) return;
		const LLAvatarData* avatar_data = static_cast<const LLAvatarData*>(data);
		if(avatar_data && mProfileView->getAvatarId() == avatar_data->avatar_id)
		{
			mProfileView->processOnlineStatus(avatar_data->flags & AVATAR_ONLINE);
			LLAvatarPropertiesProcessor::instance().removeObserver(mProfileView->getAvatarId(), this);
		}
	}

	void subscribe()
	{
		LLAvatarPropertiesProcessor::instance().addObserver(mProfileView->getAvatarId(), this);
	}

private:
	LLPanelProfileView* mProfileView;
};

LLPanelProfileView::LLPanelProfileView()
:	LLPanelProfile()
,	mStatusText(NULL)
,	mAvatarStatusObserver(NULL)
{
	mAvatarStatusObserver = new AvatarStatusObserver(this);
}

LLPanelProfileView::~LLPanelProfileView(void)
{
	delete mAvatarStatusObserver;
}

/*virtual*/ 
void LLPanelProfileView::onOpen(const LLSD& key)
{
	LLUUID id;
	if(key.has("id"))
	{
		id = key["id"];
	}

	if(id.notNull() && getAvatarId() != id)
	{
		setAvatarId(id);
		childSetValue("agent_id", id);
	}

	// Update the avatar name.
	gCacheName->get(getAvatarId(), FALSE,
		boost::bind(&LLPanelProfileView::onAvatarNameCached, this, _1, _2, _3, _4));

	updateOnlineStatus();


	LLPanelProfile::onOpen(key);
}

BOOL LLPanelProfileView::postBuild()
{
	LLPanelProfile::postBuild();

	getTabContainer()[PANEL_NOTES] = getChild<LLPanelAvatarNotes>(PANEL_NOTES);
	
	//*TODO remove this, according to style guide we don't use status combobox
	getTabContainer()[PANEL_PROFILE]->getChildView("online_me_status_text")->setVisible( FALSE);
	getTabContainer()[PANEL_PROFILE]->getChildView("status_combo")->setVisible( FALSE);

	mStatusText = getChild<LLTextBox>("status");
	mStatusText->setVisible(false);

	childSetCommitCallback("back",boost::bind(&LLPanelProfileView::onBackBtnClick,this),NULL);
	
	return TRUE;
}


//private

void LLPanelProfileView::onBackBtnClick()
{
	// Set dummy value to make picks panel dirty, 
	// This will make Picks reload on next open.
	getTabContainer()[PANEL_PICKS]->setValue(LLSD());

	LLSideTrayPanelContainer* parent = dynamic_cast<LLSideTrayPanelContainer*>(getParent());
	if(parent)
	{
		parent->openPreviousPanel();
	}
}

bool LLPanelProfileView::isGrantedToSeeOnlineStatus()
{
	const LLRelationship* relationship = LLAvatarTracker::instance().getBuddyInfo(getAvatarId());
	if (NULL == relationship)
		return false;

	// *NOTE: GRANT_ONLINE_STATUS is always set to false while changing any other status.
	// When avatar disallow me to see her online status processOfflineNotification Message is received by the viewer
	// see comments for ChangeUserRights template message. EXT-453.
	// If GRANT_ONLINE_STATUS flag is changed it will be applied when viewer restarts. EXT-3880
	return relationship->isRightGrantedFrom(LLRelationship::GRANT_ONLINE_STATUS);
}

// method was disabled according to EXT-2022. Re-enabled & improved according to EXT-3880
void LLPanelProfileView::updateOnlineStatus()
{
	// set text box visible to show online status for non-friends who has not set in Preferences
	// "Only Friends & Groups can see when I am online"
	mStatusText->setVisible(TRUE);

	const LLRelationship* relationship = LLAvatarTracker::instance().getBuddyInfo(getAvatarId());
	if (NULL == relationship)
	{
		// this is non-friend avatar. Status will be updated from LLAvatarPropertiesProcessor.
		// in LLPanelProfileView::processOnlineStatus()

		// subscribe observer to get online status. Request will be sent by LLPanelAvatarProfile itself.
		// do not subscribe for friend avatar because online status can be wrong overridden
		// via LLAvatarData::flags if Preferences: "Only Friends & Groups can see when I am online" is set.
		mAvatarStatusObserver->subscribe();
		return;
	}
	// For friend let check if he allowed me to see his status

	// status should only show if viewer has permission to view online/offline. EXT-453, EXT-3880
	mStatusText->setVisible(isGrantedToSeeOnlineStatus());

	bool online = relationship->isOnline();
	processOnlineStatus(online);
}

void LLPanelProfileView::processOnlineStatus(bool online)
{
	std::string status = getString(online ? "status_online" : "status_offline");

	mStatusText->setValue(status);
}

void LLPanelProfileView::onAvatarNameCached(const LLUUID& id, const std::string& first_name, const std::string& last_name, BOOL is_group)
{
	llassert(getAvatarId() == id);
	getChild<LLUICtrl>("user_name", FALSE)->setValue(first_name + " " + last_name);
}

// EOF
