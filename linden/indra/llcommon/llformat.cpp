/** 
 * @file llformat.cpp
 * @date   January 2007
 * @brief string formatting utility
 *
 * $LicenseInfo:firstyear=2007&license=viewergpl$
 * 
 * Copyright (c) 2007-2010, Linden Research, Inc.
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

#include "linden_common.h"

#include "llformat.h"

#include <cstdarg>

// common used function with va_list argument
// wrapper for vsnprintf to be called from llformatXXX functions.
static void va_format(std::string& out, const char *fmt, va_list va)
{
	char tstr[1024];	/* Flawfinder: ignore */
#if LL_WINDOWS
	_vsnprintf(tstr, 1024, fmt, va);
#else
	vsnprintf(tstr, 1024, fmt, va);	/* Flawfinder: ignore */
#endif
	out.assign(tstr);
}

std::string llformat(const char *fmt, ...)
{
	std::string res;
	va_list va;
	va_start(va, fmt);
	va_format(res, fmt, va);
	va_end(va);
	return res;
}

std::string llformat_to_utf8(const char *fmt, ...)
{
	std::string res;
	va_list va;
	va_start(va, fmt);
	va_format(res, fmt, va);
	va_end(va);

#if LL_WINDOWS
	// made converting to utf8. See EXT-8318.
	res = ll_convert_string_to_utf8_string(res);
#endif
	return res;
}
