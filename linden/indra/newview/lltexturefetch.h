/** 
 * @file lltexturefetch.h
 * @brief Object for managing texture fetches.
 *
 * $LicenseInfo:firstyear=2000&license=viewerlgpl$
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

#ifndef LL_LLTEXTUREFETCH_H
#define LL_LLTEXTUREFETCH_H

#include "lldir.h"
#include "llimage.h"
#include "lluuid.h"
#include "llworkerthread.h"
#include "llcurl.h"
#include "lltextureinfo.h"

class LLViewerTexture;
class LLTextureFetchWorker;
class HTTPGetResponder;
class LLTextureCache;
class LLImageDecodeThread;
class LLHost;

// Interface class
class LLTextureFetch : public LLWorkerThread
{
	friend class LLTextureFetchWorker;
	friend class HTTPGetResponder;
	
public:
	LLTextureFetch(LLTextureCache* cache, LLImageDecodeThread* imagedecodethread, bool threaded);
	~LLTextureFetch();

	/*virtual*/ S32 update(U32 max_time_ms);	
	void shutDownTextureCacheThread() ; //called in the main thread after the TextureCacheThread shuts down.
	void shutDownImageDecodeThread() ;  //called in the main thread after the ImageDecodeThread shuts down.

	bool createRequest(const std::string& url, const LLUUID& id, const LLHost& host, F32 priority,
					   S32 w, S32 h, S32 c, S32 discard, bool needs_aux, bool can_use_http);
	void deleteRequest(const LLUUID& id, bool cancel);
	bool getRequestFinished(const LLUUID& id, S32& discard_level,
							LLPointer<LLImageRaw>& raw, LLPointer<LLImageRaw>& aux);
	bool updateRequestPriority(const LLUUID& id, F32 priority);

	bool receiveImageHeader(const LLHost& host, const LLUUID& id, U8 codec, U16 packets, U32 totalbytes, U16 data_size, U8* data);
	bool receiveImagePacket(const LLHost& host, const LLUUID& id, U16 packet_num, U16 data_size, U8* data);

	void setTextureBandwidth(F32 bandwidth) { mTextureBandwidth = bandwidth; }
	F32 getTextureBandwidth() { return mTextureBandwidth; }
	
	// Debug
	BOOL isFromLocalCache(const LLUUID& id);
	S32 getFetchState(const LLUUID& id, F32& decode_progress_p, F32& requested_priority_p,
					  U32& fetch_priority_p, F32& fetch_dtime_p, F32& request_dtime_p, bool& can_use_http);
	void dump();
	S32 getNumRequests() ;
	S32 getNumHTTPRequests() ;
	
	// Public for access by callbacks
	void lockQueue() { mQueueMutex.lock(); }
	void unlockQueue() { mQueueMutex.unlock(); }
	LLTextureFetchWorker* getWorker(const LLUUID& id);
	LLTextureFetchWorker* getWorkerAfterLock(const LLUUID& id);

	LLTextureInfo* getTextureInfo() { return &mTextureInfo; }
	
protected:
	void addToNetworkQueue(LLTextureFetchWorker* worker);
	void removeFromNetworkQueue(LLTextureFetchWorker* worker, bool cancel);
	void addToHTTPQueue(const LLUUID& id);
	void removeFromHTTPQueue(const LLUUID& id, S32 received_size = 0);
	void removeRequest(LLTextureFetchWorker* worker, bool cancel);
	// Called from worker thread (during doWork)
	void processCurlRequests();	

private:
	void sendRequestListToSimulators();
	/*virtual*/ void startThread(void);
	/*virtual*/ void endThread(void);
	/*virtual*/ void threadedUpdate(void);

public:
	LLUUID mDebugID;
	S32 mDebugCount;
	BOOL mDebugPause;
	S32 mPacketCount;
	S32 mBadPacketCount;
	
private:
	LLMutex mQueueMutex;        //to protect mRequestMap only
	LLMutex mNetworkQueueMutex; //to protect mNetworkQueue, mHTTPTextureQueue and mCancelQueue.

	LLTextureCache* mTextureCache;
	LLImageDecodeThread* mImageDecodeThread;
	LLCurlRequest* mCurlGetRequest;
	
	// Map of all requests by UUID
	typedef std::map<LLUUID,LLTextureFetchWorker*> map_t;
	map_t mRequestMap;

	// Set of requests that require network data
	typedef std::set<LLUUID> queue_t;
	queue_t mNetworkQueue;
	queue_t mHTTPTextureQueue;
	typedef std::map<LLHost,std::set<LLUUID> > cancel_queue_t;
	cancel_queue_t mCancelQueue;
	F32 mTextureBandwidth;
	F32 mMaxBandwidth;
	LLTextureInfo mTextureInfo;

	U32 mHTTPTextureBits;
};

#endif // LL_LLTEXTUREFETCH_H

