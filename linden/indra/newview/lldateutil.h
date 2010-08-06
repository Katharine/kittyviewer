/** 
* @file lldateutil.h
*
* $LicenseInfo:firstyear=2009&license=viewergpl$
* 
* Copyright (c) 2009-2010, Linden Research, Inc.
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

#ifndef LLDATEUTIL_H
#define LLDATEUTIL_H

class LLDate;

namespace LLDateUtil
{
	/**
	 * Convert a date provided by the server into seconds since the Epoch.
	 * 
	 * @param[out] date Number of seconds since 01/01/1970 UTC.
	 * @param[in]  str  Date string (MM/DD/YYYY) in PDT time zone.
	 * 
	 * @return true on success, false on parse error
	 */
	bool dateFromPDTString(LLDate& date, const std::string& str);

	/**
	 * Get human-readable avatar age.
	 * 
	 * Used for avatar inspectors and profiles.
	 * 
	 * @param born_date Date an avatar was born on.
	 * @param now       Current date.
	 * 
	 * @return human-readable localized string like "1 year, 2 months",
	 *         or "???" on error.
	 */
	std::string ageFromDate(const LLDate& born_date, const LLDate& now);

	// Convert a date provided by the server (MM/DD/YYYY) into a localized,
	// human-readable age (1 year, 2 months) using translation strings.
	// Pass LLDate::now() for now.
	// Used for avatar inspectors and profiles.
	std::string ageFromDate(const std::string& date_string, const LLDate& now);

	// Calls the above with LLDate::now()
	std::string ageFromDate(const std::string& date_string);
}

#endif
