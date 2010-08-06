/** 
 * @file llsaveoutfitcombobtn.cpp
 * @brief Represents outfit save/save as combo button.
 *
 * $LicenseInfo:firstyear=2010&license=viewergpl$
 * 
 * Copyright (c) 2010, Linden Research, Inc.
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

#include "llappearancemgr.h"
#include "llpaneloutfitsinventory.h"
#include "llsidepanelappearance.h"
#include "llsaveoutfitcombobtn.h"
#include "llviewermenu.h"

static const std::string SAVE_BTN("save_btn");
static const std::string SAVE_FLYOUT_BTN("save_flyout_btn");

LLSaveOutfitComboBtn::LLSaveOutfitComboBtn(LLPanel* parent, bool saveAsDefaultAction):
	mParent(parent), mSaveAsDefaultAction(saveAsDefaultAction)
{
	// register action mapping before creating menu
	LLUICtrl::CommitCallbackRegistry::ScopedRegistrar save_registar;
	save_registar.add("Outfit.Save.Action", boost::bind(
			&LLSaveOutfitComboBtn::saveOutfit, this, false));
	save_registar.add("Outfit.SaveAs.Action", boost::bind(
			&LLSaveOutfitComboBtn::saveOutfit, this, true));

	mParent->childSetAction(SAVE_BTN, boost::bind(&LLSaveOutfitComboBtn::saveOutfit, this, mSaveAsDefaultAction));
	mParent->childSetAction(SAVE_FLYOUT_BTN, boost::bind(&LLSaveOutfitComboBtn::showSaveMenu, this));

	mSaveMenu = LLUICtrlFactory::getInstance()->createFromFile<
			LLToggleableMenu> ("menu_save_outfit.xml", gMenuHolder,
			LLViewerMenuHolderGL::child_registry_t::instance());
}

void LLSaveOutfitComboBtn::showSaveMenu()
{
	S32 x, y;
	LLUI::getMousePositionLocal(mParent, &x, &y);

	mSaveMenu->updateParent(LLMenuGL::sMenuContainer);
	LLMenuGL::showPopup(mParent, mSaveMenu, x, y);
}

void LLSaveOutfitComboBtn::saveOutfit(bool as_new)
{
	if (!as_new && LLAppearanceMgr::getInstance()->updateBaseOutfit())
	{
		// we don't need to ask for an outfit name, and updateBaseOutfit() successfully saved.
		// If updateBaseOutfit fails, ask for an outfit name anyways
		return;
	}

	LLPanelOutfitsInventory* panel_outfits_inventory =
			LLPanelOutfitsInventory::findInstance();
	if (panel_outfits_inventory)
	{
		panel_outfits_inventory->onSave();
	}

	//*TODO how to get to know when base outfit is updated or new outfit is created?
}

void LLSaveOutfitComboBtn::setMenuItemEnabled(const std::string& item, bool enabled)
{
	mSaveMenu->setItemEnabled("save_outfit", enabled);
}

void LLSaveOutfitComboBtn::setSaveBtnEnabled(bool enabled)
{
	mParent->childSetEnabled(SAVE_BTN, enabled);
}
