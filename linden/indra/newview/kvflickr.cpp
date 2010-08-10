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

#include "llbufferstream.h"
#include "lluri.h"
#include "llmd5.h"
#include "llhttpclient.h"
#include "jsoncpp/reader.h"

#include "kvflickr.h"

class KVFlickrResponse : public LLHTTPClient::Responder
{
public:
	KVFlickrResponse(KVFlickrRequest::response_callback_t &callback);
	/* virtual */ void completedRaw(
									U32 status,
									const std::string& reason,
									const LLChannelDescriptors& channels,
									const LLIOPipe::buffer_ptr_t& buffer);
private:
	KVFlickrRequest::response_callback_t mCallback;
};

void KVFlickrRequest::request(const std::string& method, const LLSD& args, response_callback_t callback) 
{
	LLSD params(args);
	params["format"] = "json";
	params["method"] = method;
	params["api_key"] = FLICKR_API_KEY;
	params["nojsoncallback"] = 1;
	params["api_sig"] = getSignatureForCall(params); // This must be the last one set.
	LLHTTPClient::get("http://flickr.com/services/rest/", params, new KVFlickrResponse(callback));
}

//static
std::string KVFlickrRequest::getSignatureForCall(const LLSD& parameters)
{
	std::vector<std::string> keys;
	for(LLSD::map_const_iterator itr = parameters.beginMap(); itr != parameters.endMap(); ++itr)
	{
		keys.push_back(itr->first);
	}
	std::sort(keys.begin(), keys.end());
	std::string to_hash(FLICKR_API_SECRET);
	for(std::vector<std::string>::const_iterator itr  = keys.begin(); itr != keys.end(); ++itr)
	{
		to_hash += *itr;
		to_hash += LLURI::escapeQueryValue(parameters[*itr]);
	}
	LLMD5 hashed((const unsigned char*)to_hash.c_str());
	char hex_hash[MD5HEX_STR_SIZE];
	hashed.hex_digest(hex_hash);
	return std::string(hex_hash);
}

void JsonToLLSD(const Json::Value &root, LLSD &output)
{
	if(root.isObject())
	{
		Json::Value::Members keys = root.getMemberNames();
		for(Json::Value::Members::const_iterator itr = keys.begin(); itr != keys.end(); ++itr)
		{
			LLSD elem;
			JsonToLLSD(root[*itr], elem);
			output[*itr] = elem;
		}
	}
	else if(root.isArray())
	{
		for(Json::Value::const_iterator itr = root.begin(); itr != root.end(); ++itr)
		{
			LLSD elem;
			JsonToLLSD(*itr, elem);
			output.append(elem);
		}
	}
	else
	{
		switch(root.type())
		{
			case Json::intValue:
				output = root.asInt();
				break;
			case Json::realValue:
			case Json::uintValue:
				output = root.asDouble();
				break;
			case Json::stringValue:
				output = root.asString();
				break;
			case Json::booleanValue:
				output = root.asBool();
			case Json::nullValue:
				output = NULL;
			default:
				break;
		}
	}
}

KVFlickrResponse::KVFlickrResponse(KVFlickrRequest::response_callback_t &callback) : 
	mCallback(callback)
{
}

void KVFlickrResponse::completedRaw(
							   U32 status,
							   const std::string& reason,
							   const LLChannelDescriptors& channels,
							   const LLIOPipe::buffer_ptr_t& buffer)
{
	LLBufferStream istr(channels, buffer.get());
	std::stringstream strstrm;
	strstrm << istr.rdbuf();
	std::string result = std::string(strstrm.str());
	Json::Value root;
	Json::Reader reader;

	bool success = reader.parse(result, root);
	if(!success)
	{
		mCallback(false, LLSD());
		return;
	}
	else
	{
		LL_INFOS("FlickrAPI") << "Got response string: " << result << LL_ENDL;
		LLSD response;
		JsonToLLSD(root, response);
		LL_INFOS("FlickrAPI") << "As LLSD: " << response << LL_ENDL;
		mCallback(isGoodStatus(status), response);
	}
}
	
