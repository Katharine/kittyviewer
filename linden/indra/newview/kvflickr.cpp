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
#include "llhttpclient.h"
#include "llmd5.h"
#include "llsd.h"
#include "llsdserialize.h"
#include "lluri.h"

#include "kvflickr.h"

class KVFlickrResponse : public LLHTTPClient::Responder
{
public:
	KVFlickrResponse(KVFlickrResponseCallback &callback);
	/* virtual */ void completedRaw(
									U32 status,
									const std::string& reason,
									const LLChannelDescriptors& channels,
									const LLIOPipe::buffer_ptr_t& buffer);
private:
	KVFlickrResponseCallback mCallback;
};

void KVFlickrRequest::request(const std::string& method, const LLSD& args, KVFlickrResponseCallback callback) 
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

KVFlickrResponse::KVFlickrResponse(KVFlickrResponseCallback &callback) : 
	mCallback(callback)
{
}

void KVFlickrResponse::completedRaw(
							   U32 status,
							   const std::string& reason,
							   const LLChannelDescriptors& channels,
							   const LLIOPipe::buffer_ptr_t& buffer)
{
	LLSD response;
	LLBufferStream istr(channels, buffer.get());
	LLPointer<LLSDParser> parser = new LLSDNotationParser();
	if(parser->parse(istr, response, LLSDSerialize::SIZE_UNLIMITED) == LLSDParser::PARSE_FAILURE)
	{
		LL_WARNS("FlickrAPI") << "Got invalid JSON." << LL_ENDL;
		mCallback(false, LLSD());
	}
	else
	{
		LL_INFOS("FlickrAPI") << "Got good response: " << response << LL_ENDL;
		mCallback(isGoodStatus(status), response);
	}
}
	
