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

#include "llfloaterreg.h"
#include "llsd.h"
#include "llslurl.h"
#include "lluri.h"
#include "llurlaction.h"
#include "llviewercontrol.h"

#include "kvflickr.h"
#include "kvflickrauthfloater.h"

KVFlickrAuthFloater::KVFlickrAuthFloater(const LLSD& key) :
	LLFloater(key)
{
}

BOOL KVFlickrAuthFloater::postBuild()
{
	mBrowser = getChild<LLMediaCtrl>("browser");
	mBrowser->addObserver(this);

	// Work out URL.
	LLSD query;
	query["api_key"] = std::string(FLICKR_API_KEY);
	query["perms"] = "write";
	query["api_sig"] = KVFlickrRequest::getSignatureForCall(query);
	std::string query_string = LLURI::mapToQueryString(query);
	LL_INFOS("FlickrAPI") << "Auth query: " << query_string << LL_ENDL;
	std::string full_url = "http://www.flickr.com/services/auth/" + query_string;
	mBrowser->navigateTo(full_url, "text/html");

	return true;
}

void KVFlickrAuthFloater::onClose(bool app_quitting)
{
	destroy(); // Die die die!
}

void KVFlickrAuthFloater::handleMediaEvent(LLPluginClassMedia* media, EMediaEvent event)
{
	if(event == MEDIA_EVENT_LOCATION_CHANGED)
	{
		std::string uri_string = media->getLocation();
		LLURI uri(uri_string);
		// We use this moronic data: hack because the internal browser crashes on
		// secondlife:/// redirects, doesn't raise any events on nonexistent links,
		// and gets confused by about:blank. At least this is prettier.
		if(uri.scheme() == "data")
		{
			// Turns out we have to parse query string out ourselves because LLURI won't do it
			// unless it's a http(s), ftp, secondlife or x-grid-location-info link.
			std::string::size_type q = uri_string.find('?');
			if(q != std::string::npos)
			{
				std::string query_string = uri_string.substr(q + 1);
				LLSD query = LLURI::queryMap(query_string);
				if(query.has("frob"))
				{
					std::string frob = query["frob"];
					LLSD params;
					params["frob"] = frob;
					KVFlickrRequest::request("flickr.auth.getToken", params, boost::bind(&KVFlickrAuthFloater::gotToken, this, _1, _2));
				}
			}
		}
		// We don't get anything if authentication is rejected; they're just redirected to the
		// home page. This is mildly problematic, given the restricted view they're in.
		// Therefore, if they click outside where we want them to be, we close the view.
		// If they go to the homepage (because they clicked "Home", the logo, or (most importantly)
		// the "Do not allow" button), it is noted that they refused permission. Otherwise,
		// we open the link they clicked in a standard browser. In either case we close
		// our floater.
		else if(uri.hostName() == "www.flickr.com" && uri.path() != "/services/auth/")
		{
			if(uri.path() == "/")
			{
				LL_WARNS("FlickrAPI") << "API permission refused." << LL_ENDL;
			}
			else
			{
				LLUrlAction::openURL(uri_string);
			}
			closeFloater(false);
		}
	}
}

void KVFlickrAuthFloater::gotToken(bool success, const LLSD& response)
{
	std::string token = response["auth"]["token"]["_content"];
	std::string username = response["auth"]["user"]["username"];
	std::string nsid = response["auth"]["user"]["nsid"];
	LL_INFOS("FlickrAPI") << "Got token " << token << " for user " << username << " (" << nsid << ")." << LL_ENDL;
	gSavedPerAccountSettings.setString("KittyFlickrToken", token);
	gSavedPerAccountSettings.setString("KittyFlickrUsername", username);
	gSavedPerAccountSettings.setString("KittyFlickrNSID", nsid);
	closeFloater(false);
}

//static
void KVFlickrAuthFloater::showFloater()
{
	KVFlickrAuthFloater *floater = dynamic_cast<KVFlickrAuthFloater*>(LLFloaterReg::getInstance("flickr_auth"));
	if(floater)
	{
		floater->setVisible(true);
		floater->setFrontmost(true);
		floater->center();
	}
	else
	{
		LL_WARNS("FlickrAPI") << "Can't find flickr auth!" << LL_ENDL;
	}
}
