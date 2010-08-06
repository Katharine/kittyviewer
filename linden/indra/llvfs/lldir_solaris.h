/** 
 * @file fmodwrapper.cpp
 * @brief dummy source file for building a shared library to wrap libfmod.a
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2010, Linden Research, Inc.
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

#ifndef LL_LLDIR_SOLARIS_H
#define LL_LLDIR_SOLARIS_H

#include "lldir.h"

#include <dirent.h>
#include <errno.h>

class LLDir_Solaris : public LLDir
{
public:
	LLDir_Solaris();
	virtual ~LLDir_Solaris();

	/*virtual*/ void initAppDirs(const std::string &app_name,
		const std::string& app_read_only_data_dir);
public:	
	virtual std::string getCurPath();
	virtual U32 countFilesInDir(const std::string &dirname, const std::string &mask);
	virtual BOOL getNextFileInDir(const std::string &dirname, const std::string &mask, std::string &fname, BOOL wrap);
	virtual void getRandomFileInDir(const std::string &dirname, const std::string &mask, std::string &fname);
	/*virtual*/ BOOL fileExists(const std::string &filename) const;

private:
	DIR *mDirp;
	int mCurrentDirIndex;
	int mCurrentDirCount;
	std::string mCurrentDir;	
};

#endif // LL_LLDIR_SOLARIS_H


