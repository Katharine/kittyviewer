/** 
 * @file llpanelpeoplemenus.h
 * @brief Menus used by the side tray "People" panel
 *
 * $LicenseInfo:firstyear=2009&license=viewergpl$
 * 
 * Copyright (c) 2009-2010, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

// libs
#include "llmenugl.h"
#include "lluictrlfactory.h"

#include "llpanelpeoplemenus.h"

// newview
#include "llagent.h"
#include "llagentdata.h"			// for gAgentID
#include "llavataractions.h"
#include "llcallingcard.h"			// for LLAvatarTracker
#include "llviewermenu.h"			// for gMenuHolder

namespace LLPanelPeopleMenus
{

NearbyMenu gNearbyMenu;

//== NearbyMenu ===============================================================

LLContextMenu* NearbyMenu::createMenu()
{
	// set up the callbacks for all of the avatar menu items
	LLUICtrl::CommitCallbackRegistry::ScopedRegistrar registrar;
	LLUICtrl::EnableCallbackRegistry::ScopedRegistrar enable_registrar;

	if ( mUUIDs.size() == 1 )
	{
		// Set up for one person selected menu

		const LLUUID& id = mUUIDs.front();
		registrar.add("Avatar.Profile",			boost::bind(&LLAvatarActions::showProfile,				id));
		registrar.add("Avatar.AddFriend",		boost::bind(&LLAvatarActions::requestFriendshipDialog,	id));
		registrar.add("Avatar.RemoveFriend",	boost::bind(&LLAvatarActions::removeFriendDialog, 		id));
		registrar.add("Avatar.IM",				boost::bind(&LLAvatarActions::startIM,					id));
		registrar.add("Avatar.Call",			boost::bind(&LLAvatarActions::startCall,				id));
		registrar.add("Avatar.OfferTeleport",	boost::bind(&NearbyMenu::offerTeleport,					this));
		registrar.add("Avatar.ShowOnMap",		boost::bind(&LLAvatarActions::showOnMap,				id));
		registrar.add("Avatar.Share",			boost::bind(&LLAvatarActions::share,					id));
		registrar.add("Avatar.Pay",				boost::bind(&LLAvatarActions::pay,						id));
		registrar.add("Avatar.BlockUnblock",	boost::bind(&LLAvatarActions::toggleBlock,				id));

		enable_registrar.add("Avatar.EnableItem", boost::bind(&NearbyMenu::enableContextMenuItem,	this, _2));
		enable_registrar.add("Avatar.CheckItem",  boost::bind(&NearbyMenu::checkContextMenuItem,	this, _2));

		// create the context menu from the XUI
		return createFromFile("menu_people_nearby.xml");
	}
	else
	{
		// Set up for multi-selected People

		// registrar.add("Avatar.AddFriend",	boost::bind(&LLAvatarActions::requestFriendshipDialog,	mUUIDs)); // *TODO: unimplemented
		registrar.add("Avatar.IM",			boost::bind(&LLAvatarActions::startConference,			mUUIDs));
		registrar.add("Avatar.Call",		boost::bind(&LLAvatarActions::startAdhocCall,			mUUIDs));
		registrar.add("Avatar.RemoveFriend",boost::bind(&LLAvatarActions::removeFriendsDialog,		mUUIDs));
		// registrar.add("Avatar.Share",		boost::bind(&LLAvatarActions::startIM,					mUUIDs)); // *TODO: unimplemented
		// registrar.add("Avatar.Pay",		boost::bind(&LLAvatarActions::pay,						mUUIDs)); // *TODO: unimplemented
		enable_registrar.add("Avatar.EnableItem",	boost::bind(&NearbyMenu::enableContextMenuItem,	this, _2));

		// create the context menu from the XUI
		return createFromFile("menu_people_nearby_multiselect.xml");
	}
}

bool NearbyMenu::enableContextMenuItem(const LLSD& userdata)
{
	std::string item = userdata.asString();

	// Note: can_block and can_delete is used only for one person selected menu
	// so we don't need to go over all uuids.

	if (item == std::string("can_block"))
	{
		const LLUUID& id = mUUIDs.front();
		return LLAvatarActions::canBlock(id);
	}
	else if (item == std::string("can_add"))
	{
		// We can add friends if:
		// - there are selected people
		// - and there are no friends among selection yet.

		//EXT-7389 - disable for more than 1
		if(mUUIDs.size() > 1)
		{
			return false;
		}

		bool result = (mUUIDs.size() > 0);

		uuid_vec_t::const_iterator
			id = mUUIDs.begin(),
			uuids_end = mUUIDs.end();

		for (;id != uuids_end; ++id)
		{
			if ( LLAvatarActions::isFriend(*id) )
			{
				result = false;
				break;
			}
		}

		return result;
	}
	else if (item == std::string("can_delete"))
	{
		// We can remove friends if:
		// - there are selected people
		// - and there are only friends among selection.

		bool result = (mUUIDs.size() > 0);

		uuid_vec_t::const_iterator
			id = mUUIDs.begin(),
			uuids_end = mUUIDs.end();

		for (;id != uuids_end; ++id)
		{
			if ( !LLAvatarActions::isFriend(*id) )
			{
				result = false;
				break;
			}
		}

		return result;
	}
	else if (item == std::string("can_call"))
	{
		return LLAvatarActions::canCall();
	}
	else if (item == std::string("can_show_on_map"))
	{
		const LLUUID& id = mUUIDs.front();

		return (LLAvatarTracker::instance().isBuddyOnline(id) && is_agent_mappable(id))
					|| gAgent.isGodlike();
	}
	else if(item == std::string("can_offer_teleport"))
	{
		const LLUUID& id = mUUIDs.front();
		return LLAvatarActions::canOfferTeleport(id);
	}
	return false;
}

bool NearbyMenu::checkContextMenuItem(const LLSD& userdata)
{
	std::string item = userdata.asString();
	const LLUUID& id = mUUIDs.front();

	if (item == std::string("is_blocked"))
	{
		return LLAvatarActions::isBlocked(id);
	}

	return false;
}

void NearbyMenu::offerTeleport()
{
	// boost::bind cannot recognize overloaded method LLAvatarActions::offerTeleport(),
	// so we have to use a wrapper.
	const LLUUID& id = mUUIDs.front();
	LLAvatarActions::offerTeleport(id);
}

} // namespace LLPanelPeopleMenus
