/**
 * @file llagentlanguage.cpp
 * @brief Transmit language information to server
 *
 * $LicenseInfo:firstyear=2006&license=viewergpl$
 * 
 * Copyright (c) 2006-2010, Linden Research, Inc.
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
#include "llagentlanguage.h"
// viewer includes
#include "llagent.h"
#include "llviewercontrol.h"
#include "llviewerregion.h"
// library includes
#include "llui.h"					// getLanguage()

// static
void LLAgentLanguage::init()
{
	gSavedSettings.getControl("Language")->getSignal()->connect(boost::bind(&onChange));
	gSavedSettings.getControl("InstallLanguage")->getSignal()->connect(boost::bind(&onChange));
	gSavedSettings.getControl("SystemLanguage")->getSignal()->connect(boost::bind(&onChange));
	gSavedSettings.getControl("LanguageIsPublic")->getSignal()->connect(boost::bind(&onChange));
}

// static
void LLAgentLanguage::onChange()
{
	// Clear inventory cache so that default names of inventory items
	// appear retranslated (EXT-8308).
	gSavedSettings.setBOOL("PurgeCacheOnNextStartup", TRUE);
}

// send language settings to the sim
// static
bool LLAgentLanguage::update()
{
	LLSD body;
	std::string url;

	if (gAgent.getRegion())
	{
		url = gAgent.getRegion()->getCapability("UpdateAgentLanguage");
	}

	if (!url.empty())
	{
		std::string language = LLUI::getLanguage();
		
		body["language"] = language;
		body["language_is_public"] = gSavedSettings.getBOOL("LanguageIsPublic");
		
		LLHTTPClient::post(url, body, new LLHTTPClient::Responder);
	}
    return true;
}
