/** 
 * @file llstyle.h
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

#ifndef LL_LLSTYLE_H
#define LL_LLSTYLE_H

#include "v4color.h"
#include "llui.h"
#include "llinitparam.h"

class LLFontGL;

class LLStyle : public LLRefCount
{
public:
	struct Params : public LLInitParam::Block<Params>
	{
		Optional<bool>					visible;
		Optional<LLFontGL::ShadowType>	drop_shadow;
		Optional<LLUIColor>				color,
										readonly_color,
										selected_color;
		Optional<const LLFontGL*>		font;
		Optional<LLUIImage*>			image;
		Optional<std::string>			link_href;
		Params();
	};
	LLStyle(const Params& p = Params());
public:
	const LLUIColor& getColor() const { return mColor; }
	void setColor(const LLUIColor &color) { mColor = color; }

	const LLUIColor& getReadOnlyColor() const { return mReadOnlyColor; }
	void setReadOnlyColor(const LLUIColor& color) { mReadOnlyColor = color; }

	const LLUIColor& getSelectedColor() const { return mSelectedColor; }
	void setSelectedColor(const LLUIColor& color) { mSelectedColor = color; }

	BOOL isVisible() const;
	void setVisible(BOOL is_visible);

	LLFontGL::ShadowType getShadowType() const { return mDropShadow; }

	void setFont(const LLFontGL* font);
	const LLFontGL* getFont() const;

	const std::string& getLinkHREF() const { return mLink; }
	void setLinkHREF(const std::string& href);
	BOOL isLink() const;

	LLUIImagePtr getImage() const;
	void setImage(const LLUUID& src);
	void setImage(const std::string& name);

	BOOL isImage() const { return mImagep.notNull(); }

	bool operator==(const LLStyle &rhs) const
	{
		return 
			mVisible == rhs.mVisible
			&& mColor == rhs.mColor
			&& mReadOnlyColor == rhs.mReadOnlyColor
			&& mSelectedColor == rhs.mSelectedColor
			&& mFont == rhs.mFont
			&& mLink == rhs.mLink
			&& mImagep == rhs.mImagep
			&& mDropShadow == rhs.mDropShadow;
	}

	bool operator!=(const LLStyle& rhs) const { return !(*this == rhs); }

public:	
	LLFontGL::ShadowType		mDropShadow;

protected:
	~LLStyle() { }

private:
	BOOL				mVisible;
	LLUIColor			mColor;
	LLUIColor   		mReadOnlyColor;
	LLUIColor			mSelectedColor;
	std::string			mFontName;
	const LLFontGL*		mFont;
	std::string			mLink;
	LLUIImagePtr		mImagep;
};

typedef LLPointer<LLStyle> LLStyleSP;
typedef LLPointer<const LLStyle> LLStyleConstSP;

#endif  // LL_LLSTYLE_H
