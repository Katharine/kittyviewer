/** 
 * @file llstyle.cpp
 * @brief Text style class
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2010, Linden Research, Inc.
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

#include "llstyle.h"

#include "llfontgl.h"
#include "llstring.h"
#include "llui.h"

LLStyle::Params::Params()
:	visible("visible", true),
	drop_shadow("drop_shadow", LLFontGL::NO_SHADOW),
	color("color", LLColor4::black),
	readonly_color("readonly_color", LLColor4::black),
	selected_color("selected_color", LLColor4::black),
	font("font", LLFontGL::getFontMonospace()),
	image("image"),
	link_href("href")
{}


LLStyle::LLStyle(const LLStyle::Params& p)
:	mVisible(p.visible),
	mColor(p.color),
	mReadOnlyColor(p.readonly_color),
	mSelectedColor(p.selected_color),
	mFont(p.font()),
	mLink(p.link_href),
	mDropShadow(p.drop_shadow),
	mImagep(p.image())
{}

void LLStyle::setFont(const LLFontGL* font)
{
	mFont = font;
}


const LLFontGL* LLStyle::getFont() const
{
	return mFont;
}

void LLStyle::setLinkHREF(const std::string& href)
{
	mLink = href;
}

BOOL LLStyle::isLink() const
{
	return mLink.size();
}

BOOL LLStyle::isVisible() const
{
	return mVisible;
}

void LLStyle::setVisible(BOOL is_visible)
{
	mVisible = is_visible;
}

LLUIImagePtr LLStyle::getImage() const
{
	return mImagep;
}

void LLStyle::setImage(const LLUUID& src)
{
	mImagep = LLUI::getUIImageByID(src);
}

void LLStyle::setImage(const std::string& name)
{
	mImagep = LLUI::getUIImage(name);
}
