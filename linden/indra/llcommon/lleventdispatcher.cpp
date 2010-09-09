/**
 * @file   lleventdispatcher.cpp
 * @author Nat Goodspeed
 * @date   2009-06-18
 * @brief  Implementation for lleventdispatcher.
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

#if LL_WINDOWS
#pragma warning (disable : 4355) // 'this' used in initializer list: yes, intentionally
#endif

// Precompiled header
#include "linden_common.h"
// associated header
#include "lleventdispatcher.h"
// STL headers
// std headers
// external library headers
// other Linden headers
#include "llevents.h"
#include "llerror.h"
#include "llsdutil.h"

LLEventDispatcher::LLEventDispatcher(const std::string& desc, const std::string& key):
    mDesc(desc),
    mKey(key)
{
}

LLEventDispatcher::~LLEventDispatcher()
{
}

/// Register a callable by name
void LLEventDispatcher::add(const std::string& name, const std::string& desc,
                            const Callable& callable, const LLSD& required)
{
    mDispatch.insert(DispatchMap::value_type(name,
                                             DispatchMap::mapped_type(callable, desc, required)));
}

void LLEventDispatcher::addFail(const std::string& name, const std::string& classname) const
{
    LL_ERRS("LLEventDispatcher") << "LLEventDispatcher(" << mDesc << ")::add(" << name
                                 << "): " << classname << " is not a subclass "
                                 << "of LLEventDispatcher" << LL_ENDL;
}

/// Unregister a callable
bool LLEventDispatcher::remove(const std::string& name)
{
    DispatchMap::iterator found = mDispatch.find(name);
    if (found == mDispatch.end())
    {
        return false;
    }
    mDispatch.erase(found);
    return true;
}

/// Call a registered callable with an explicitly-specified name. If no
/// such callable exists, die with LL_ERRS.
void LLEventDispatcher::operator()(const std::string& name, const LLSD& event) const
{
    if (! attemptCall(name, event))
    {
        LL_ERRS("LLEventDispatcher") << "LLEventDispatcher(" << mDesc << "): '" << name
                                     << "' not found" << LL_ENDL;
    }
}

/// Extract the @a key value from the incoming @a event, and call the
/// callable whose name is specified by that map @a key. If no such
/// callable exists, die with LL_ERRS.
void LLEventDispatcher::operator()(const LLSD& event) const
{
    // This could/should be implemented in terms of the two-arg overload.
    // However -- we can produce a more informative error message.
    std::string name(event[mKey]);
    if (! attemptCall(name, event))
    {
        LL_ERRS("LLEventDispatcher") << "LLEventDispatcher(" << mDesc << "): bad " << mKey
                                     << " value '" << name << "'" << LL_ENDL;
    }
}

bool LLEventDispatcher::attemptCall(const std::string& name, const LLSD& event) const
{
    DispatchMap::const_iterator found = mDispatch.find(name);
    if (found == mDispatch.end())
    {
        // The reason we only return false, leaving it up to our caller to die
        // with LL_ERRS, is that different callers have different amounts of
        // available information.
        return false;
    }
    // Found the name, so it's plausible to even attempt the call. But first,
    // validate the syntax of the event itself.
    std::string mismatch(llsd_matches(found->second.mRequired, event));
    if (! mismatch.empty())
    {
        LL_ERRS("LLEventDispatcher") << "LLEventDispatcher(" << mDesc << ") calling '" << name
                                     << "': bad request: " << mismatch << LL_ENDL;
    }
    // Event syntax looks good, go for it!
    (found->second.mFunc)(event);
    return true;                    // tell caller we were able to call
}

LLEventDispatcher::Callable LLEventDispatcher::get(const std::string& name) const
{
    DispatchMap::const_iterator found = mDispatch.find(name);
    if (found == mDispatch.end())
    {
        return Callable();
    }
    return found->second.mFunc;
}

LLSD LLEventDispatcher::getMetadata(const std::string& name) const
{
    DispatchMap::const_iterator found = mDispatch.find(name);
    if (found == mDispatch.end())
    {
        return LLSD();
    }
    LLSD meta;
    meta["name"] = name;
    meta["desc"] = found->second.mDesc;
    meta["required"] = found->second.mRequired;
    return meta;
}

LLDispatchListener::LLDispatchListener(const std::string& pumpname, const std::string& key):
    LLEventDispatcher(pumpname, key),
    mPump(pumpname, true),          // allow tweaking for uniqueness
    mBoundListener(mPump.listen("self", boost::bind(&LLDispatchListener::process, this, _1)))
{
}

bool LLDispatchListener::process(const LLSD& event)
{
    (*this)(event);
    return false;
}
