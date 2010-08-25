/* Copyright (c) 2010 Katharine Berry. All rights reserved.
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
#include "llbottomtray.h"
#include "llcombobox.h"
#include "lldir.h"
#include "llfile.h"
#include "llfloaterpreference.h"
#include "llfloaterreg.h"
#include "llnotificationsutil.h"
#include "lluictrl.h"
#include "llversioninfo.h"
#include "llviewercontrol.h"

#include "kvfloaterquicksettings.h"

KVFloaterQuickSettings::KVFloaterQuickSettings(const LLSD& key)
: LLTransientDockableFloater(NULL, true, key)
{
}

//virtual
BOOL KVFloaterQuickSettings::postBuild()
{
	setIsChrome(true);
	setTitleVisible(true);

	// Populate the settings combo
	LLComboBox *settings_list = getChild<LLComboBox>("profiles_combo");
	if(settings_list)
	{
		// Include the default profile.
		settings_list->add("Default");

		std::string path_name = gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "profiles", "");
		while(true)
		{
			std::string name;
			if(!gDirUtilp->getNextFileInDir(path_name, "*.xml", name, false))
				break;
			name = name.erase(name.length()-4);
			settings_list->add(name);
		}

		settings_list->setSelectedByValue(gSavedSettings.getLLSD("KittyCurrentSettingsProfile"), true);
		settings_list->setCommitCallback(boost::bind(&KVFloaterQuickSettings::onChangeProfile, this, _1));
	}
	else
	{
		LL_WARNS("QuickSettings") << "No profiles combo. Fix the XUI and try again." << LL_ENDL;
	}

	getChild<LLUICtrl>("new_profile_btn")->setCommitCallback(boost::bind(&KVFloaterQuickSettings::onNewProfile, this));

	LLDockableFloater::postBuild();

	return true;
}

void KVFloaterQuickSettings::onOpen(const LLSD& key)
{
	LLButton *anchor_panel = LLBottomTray::getInstance()->getChild<LLButton>("quick_settings_btn");

	setDockControl(new LLDockControl(
									 anchor_panel, this,
									 getDockTongue(), LLDockControl::TOP));
}

void KVFloaterQuickSettings::onNewProfile()
{
	LLNotificationsUtil::add("KittyNewSettingsProfile", LLSD(), LLSD(), boost::bind(&KVFloaterQuickSettings::newPromptCallback, this, _1, _2));
}

bool KVFloaterQuickSettings::newPromptCallback(const LLSD& notification, const LLSD& response)
{
	std::string profile = response["message"].asString();
	S32 option = LLNotificationsUtil::getSelectedOption(notification, response);

	if(option == 0 && profile != "")
	{
		std::string path = getPathForProfile(profile);
		// Make sure it doesn't already exist. "Default" is reserved.
		if(profile != "Default" && !gDirUtilp->fileExists(path))
		{
			// Save back the current settings first.
			gSavedSettings.saveToFile(gSavedSettings.getString("ClientSettingsFile"), true);
			// Make sure we have the folder.
			LLFile::mkdir(gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "profiles", ""));
			// Make it exist!
			gSavedSettings.setString("KittyCurrentSettingsProfile", profile);
			gSavedSettings.saveToFile(path, true);

			// Add it to the combo box.
			LLComboBox *settings_list = getChild<LLComboBox>("profiles_combo");
			settings_list->add(profile);
			settings_list->sortByName();
			settings_list->setSelectedByValue(profile, true);
			// I think that's it.
		}
		else
		{
			LLSD args;
			args["PROFILE"] = profile;
			LLNotificationsUtil::add("KittyProfileAlreadyExists", args);
		}
	}

	return false;
}

void KVFloaterQuickSettings::onChangeProfile(LLUICtrl *ctrl)
{
	std::string profile = ctrl->getValue().asString();
	// First we must save our current set of settings,
	// because it's not done automatically.
	LL_INFOS("QuickSettings") << "Saving settings to " << gSavedSettings.getString("ClientSettingsFile") << LL_ENDL;
	gSavedSettings.saveToFile(gSavedSettings.getString("ClientSettingsFile"), true);
	// Then we work out the correct path and load a new file!
	std::string file_path = getPathForProfile(profile);
	if(loadSettings(file_path))
	{
		LL_INFOS("QuickSettings") << "Loaded '" << profile << "' successfully." << LL_ENDL;
		// We need this for setting the default value of this floater.
		gSavedSettings.setString("KittyCurrentSettingsProfile", profile);
	}
	else
	{
		// This should never happen. So, of course, it will.
		getChild<LLComboBox>("profiles_combo")->setSelectedByValue(gSavedSettings.getString("KittyCurrentSettingsProfile"), true);
		LLSD args;
		args["PROFILE"] = profile;
		LLNotificationsUtil::add("KittyMissingSettingsProfile", args);
	}
}

//static
std::string KVFloaterQuickSettings::getPathForProfile(const std::string& profile)
{
	if(profile != "Default")
	{
		return gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, "profiles", profile + ".xml");
	}
	else
	{
		return gDirUtilp->getExpandedFilename(LL_PATH_USER_SETTINGS, LLAppViewer::instance()->getSettingsFilename("User", "Global"));
	}
}

bool KVFloaterQuickSettings::loadSettings(const std::string& file)
{
	// *HACK: Close preferences floater, if it's open.
	// Otherwise it'll override our settings changes when it closes.
	// We also need to save its rect, because we're wiping the settings,
	// and it may be moved.
	LLFloaterPreference* prefs = LLFloaterReg::findTypedInstance<LLFloaterPreference>("preferences");
	LLRect prefs_pos;
	bool reopen_prefs = false;
	bool success;
	if(prefs && prefs->getVisible())
	{
		prefs_pos = prefs->getRect();
		reopen_prefs = prefs->isShown();
		prefs->closeFloater(false);
		LL_INFOS("QuickSettings") << "Closed prefs floater." << LL_ENDL;
	}

	// Actually load the settings.
	if(gSavedSettings.loadFromFile(file, false, true))
	{
		LL_INFOS("QuickSettings") << "Loaded settings from " << file << LL_ENDL;
		// Reset "procedural settings". ClientSettingsFile is particularly important.
		gSavedSettings.setString("ClientSettingsFile", file);
		gSavedSettings.setString("VersionChannelName", LLVersionInfo::getChannel());
		success = true;
	}
	else 
	{
		LL_WARNS("QuickSettings") << "Settings load from " << file << " failed." << LL_ENDL;
		success = false;
	}

	// Reopen the prefs floater if we closed it and it wasn't minimised.
	if(reopen_prefs && prefs)
	{
		prefs->openFloater(LLSD());
		prefs->setRect(prefs_pos);
		LL_INFOS("QuickSettings") << "Opened prefs floater." << LL_ENDL;
	}
	return success;
}
