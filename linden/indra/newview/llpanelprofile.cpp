/** 
* @file llpanelprofile.cpp
* @brief Profile panel implementation
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
#include "llpanelprofile.h"

#include "llagent.h"
#include "llavataractions.h"
#include "llfloaterreg.h"
#include "llcommandhandler.h"
#include "llpanelpicks.h"
#include "lltabcontainer.h"

static const std::string PANEL_PICKS = "panel_picks";
static const std::string PANEL_PROFILE = "panel_profile";

class LLAgentHandler : public LLCommandHandler
{
public:
	// requires trusted browser to trigger
	LLAgentHandler() : LLCommandHandler("agent", UNTRUSTED_THROTTLE) { }

	bool handle(const LLSD& params, const LLSD& query_map,
		LLMediaCtrl* web)
	{
		if (params.size() < 2) return false;
		LLUUID avatar_id;
		if (!avatar_id.set(params[0], FALSE))
		{
			return false;
		}

		const std::string verb = params[1].asString();
		if (verb == "about")
		{
			LLAvatarActions::showProfile(avatar_id);
			return true;
		}

		if (verb == "inspect")
		{
			LLFloaterReg::showInstance("inspect_avatar", LLSD().with("avatar_id", avatar_id));
			return true;
		}

		if (verb == "im")
		{
			LLAvatarActions::startIM(avatar_id);
			return true;
		}

		if (verb == "pay")
		{
			LLAvatarActions::pay(avatar_id);
			return true;
		}

		if (verb == "offerteleport")
		{
			LLAvatarActions::offerTeleport(avatar_id);
			return true;
		}

		if (verb == "requestfriend")
		{
			LLAvatarActions::requestFriendshipDialog(avatar_id);
			return true;
		}

		if (verb == "mute")
		{
			if (! LLAvatarActions::isBlocked(avatar_id))
			{
				LLAvatarActions::toggleBlock(avatar_id);
			}
			return true;
		}

		if (verb == "unmute")
		{
			if (LLAvatarActions::isBlocked(avatar_id))
			{
				LLAvatarActions::toggleBlock(avatar_id);
			}
			return true;
		}

		return false;
	}
};
LLAgentHandler gAgentHandler;


LLPanelProfile::LLPanelProfile()
 : LLPanel()
 , mTabCtrl(NULL)
 , mAvatarId(LLUUID::null)
{
}

BOOL LLPanelProfile::postBuild()
{
	mTabCtrl = getChild<LLTabContainer>("tabs");

	getTabCtrl()->setCommitCallback(boost::bind(&LLPanelProfile::onTabSelected, this, _2));

	LLPanelPicks* panel_picks = getChild<LLPanelPicks>(PANEL_PICKS);
	panel_picks->setProfilePanel(this);

	getTabContainer()[PANEL_PICKS] = panel_picks;
	getTabContainer()[PANEL_PROFILE] = getChild<LLPanelAvatarProfile>(PANEL_PROFILE);

	return TRUE;
}

void LLPanelProfile::onOpen(const LLSD& key)
{
	// open the desired panel
	if (key.has("open_tab_name"))
	{
		getTabContainer()[PANEL_PICKS]->onClosePanel();

		// onOpen from selected panel will be called from onTabSelected callback
		getTabCtrl()->selectTabByName(key["open_tab_name"]);
	}
	else
	{
		getTabCtrl()->getCurrentPanel()->onOpen(getAvatarId());
	}

	// support commands to open further pieces of UI
	if (key.has("show_tab_panel"))
	{
		std::string panel = key["show_tab_panel"].asString();
		if (panel == "create_classified")
		{
			LLPanelPicks* picks = dynamic_cast<LLPanelPicks *>(getTabContainer()[PANEL_PICKS]);
			if (picks)
			{
				picks->createNewClassified();
			}
		}
		else if (panel == "classified_details")
		{
			LLPanelPicks* picks = dynamic_cast<LLPanelPicks *>(getTabContainer()[PANEL_PICKS]);
			if (picks)
			{
				LLSD params = key;
				params.erase("show_tab_panel");
				params.erase("open_tab_name");
				picks->openClassifiedInfo(params);
			}
		}
	}
}

//*TODO redo panel toggling
void LLPanelProfile::togglePanel(LLPanel* panel, const LLSD& key)
{
	// TRUE - we need to open/expand "panel"
	bool expand = getChildList()->front() != panel;  // mTabCtrl->getVisible();

	if (expand)
	{
		openPanel(panel, key);
	}
	else 
	{
		closePanel(panel);

		getTabCtrl()->getCurrentPanel()->onOpen(getAvatarId());
	}
}

void LLPanelProfile::onTabSelected(const LLSD& param)
{
	std::string tab_name = param.asString();
	if (NULL != getTabContainer()[tab_name])
	{
		getTabContainer()[tab_name]->onOpen(getAvatarId());
	}
}

void LLPanelProfile::setAllChildrenVisible(BOOL visible)
{
	const child_list_t* child_list = getChildList();
	child_list_const_iter_t child_it = child_list->begin();
	for (; child_it != child_list->end(); ++child_it)
	{
		LLView* viewp = *child_it;
		viewp->setVisible(visible);
	}
}

void LLPanelProfile::openPanel(LLPanel* panel, const LLSD& params)
{
	if (panel->getParent() != this)
	{
		addChild(panel);
	}
	else
	{
		sendChildToFront(panel);
	}

	panel->setVisible(TRUE);

	panel->onOpen(params);

	LLRect new_rect = getRect();
	panel->reshape(new_rect.getWidth(), new_rect.getHeight());
	new_rect.setLeftTopAndSize(0, new_rect.getHeight(), new_rect.getWidth(), new_rect.getHeight());
	panel->setRect(new_rect);
}

void LLPanelProfile::closePanel(LLPanel* panel)
{
	panel->setVisible(FALSE);

	if (panel->getParent() == this) 
	{
		removeChild(panel);
	}
}

S32 LLPanelProfile::notifyParent(const LLSD& info)
{
	std::string action = info["action"];
	// lets update Picks list after Pick was saved
	if("save_new_pick" == action)
	{
		onOpen(info);
		return 1;
	}

	return LLPanel::notifyParent(info);
}
