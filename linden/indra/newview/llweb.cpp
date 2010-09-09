/** 
 * @file llweb.cpp
 * @brief Functions dealing with web browsers
 * @author James Cook
 *
 * $LicenseInfo:firstyear=2006&license=viewerlgpl$
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

#include "llweb.h"

// Library includes
#include "llwindow.h"	// spawnWebBrowser()

#include "llagent.h"
#include "llappviewer.h"
#include "llfloatermediabrowser.h"
#include "llfloaterreg.h"
#include "lllogininstance.h"
#include "llparcel.h"
#include "llsd.h"
#include "lltoastalertpanel.h"
#include "llui.h"
#include "lluri.h"
#include "llversioninfo.h"
#include "llviewercontrol.h"
#include "llviewernetwork.h"
#include "llviewerparcelmgr.h"
#include "llviewerregion.h"
#include "llviewerwindow.h"
#include "llnotificationsutil.h"

bool on_load_url_external_response(const LLSD& notification, const LLSD& response, bool async );


class URLLoader : public LLToastAlertPanel::URLLoader
{
	virtual void load(const std::string& url , bool force_open_externally)
	{
		if (force_open_externally)
		{
			LLWeb::loadURLExternal(url);
		}
		else
		{
			LLWeb::loadURL(url);
		}
	}
};
static URLLoader sAlertURLLoader;


// static
void LLWeb::initClass()
{
	LLToastAlertPanel::setURLLoader(&sAlertURLLoader);
}


// static
void LLWeb::loadURL(const std::string& url)
{
	if (gSavedSettings.getBOOL("UseExternalBrowser"))
	{
		loadURLExternal(url);
	}
	else
	{
		loadURLInternal(url);
	}
}


// static
void LLWeb::loadURLInternal(const std::string &url)
{
	LLFloaterReg::showInstance("media_browser", url);
}


// static
void LLWeb::loadURLExternal(const std::string& url)
{
	loadURLExternal(url, true);
}


// static
void LLWeb::loadURLExternal(const std::string& url, bool async)
{
	LLSD payload;
	payload["url"] = url;
	LLNotificationsUtil::add( "WebLaunchExternalTarget", LLSD(), payload, boost::bind(on_load_url_external_response, _1, _2, async));
}

// static 
bool on_load_url_external_response(const LLSD& notification, const LLSD& response, bool async )
{
	S32 option = LLNotificationsUtil::getSelectedOption(notification, response);
	if ( 0 == option )
	{
		LLSD payload = notification["payload"];
		std::string url = payload["url"].asString();
		std::string escaped_url = LLWeb::escapeURL(url);
		if (gViewerWindow)
		{
			gViewerWindow->getWindow()->spawnWebBrowser(escaped_url, async);
		}
	}
	return false;
}


// static
std::string LLWeb::escapeURL(const std::string& url)
{
	// The CURL curl_escape() function escapes colons, slashes,
	// and all characters but A-Z and 0-9.  Do a cheesy mini-escape.
	std::string escaped_url;
	S32 len = url.length();
	for (S32 i = 0; i < len; i++)
	{
		char c = url[i];
		if (c == ' ')
		{
			escaped_url += "%20";
		}
		else if (c == '\\')
		{
			escaped_url += "%5C";
		}
		else
		{
			escaped_url += c;
		}
	}
	return escaped_url;
}

//static
std::string LLWeb::expandURLSubstitutions(const std::string &url,
										  const LLSD &default_subs)
{
	LLSD substitution = default_subs;
	substitution["VERSION"] = LLVersionInfo::getVersion();
	substitution["VERSION_MAJOR"] = LLVersionInfo::getMajor();
	substitution["VERSION_MINOR"] = LLVersionInfo::getMinor();
	substitution["VERSION_PATCH"] = LLVersionInfo::getPatch();
	substitution["VERSION_BUILD"] = LLVersionInfo::getBuild();
	substitution["CHANNEL"] = LLVersionInfo::getChannel();
	substitution["GRID"] = LLGridManager::getInstance()->getGridLabel();
	substitution["OS"] = LLAppViewer::instance()->getOSInfo().getOSStringSimple();
	substitution["SESSION_ID"] = gAgent.getSessionID();
	substitution["FIRST_LOGIN"] = gAgent.isFirstLogin();

	// work out the current language
	std::string lang = LLUI::getLanguage();
	if (lang == "en-us")
	{
		// *HACK: the correct fix is to change English.lproj/language.txt,
		// but we're late in the release cycle and this is a less risky fix
		lang = "en";
	}
	substitution["LANGUAGE"] = lang;

	// find the region ID
	LLUUID region_id;
	LLViewerRegion *region = gAgent.getRegion();
	if (region)
	{
		region_id = region->getRegionID();
	}
	substitution["REGION_ID"] = region_id;

	// find the parcel local ID
	S32 parcel_id = 0;
	LLParcel* parcel = LLViewerParcelMgr::getInstance()->getAgentParcel();
	if (parcel)
	{
		parcel_id = parcel->getLocalID();
	}
	substitution["PARCEL_ID"] = llformat("%d", parcel_id);

	// expand all of the substitution strings and escape the url
	std::string expanded_url = url;
	LLStringUtil::format(expanded_url, substitution);

	return LLWeb::escapeURL(expanded_url);
}
