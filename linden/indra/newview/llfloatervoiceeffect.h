/** 
 * @file llfloatervoiceeffect.h
 * @author Aimee
 * @brief Selection and preview of voice effects.
 *
 * $LicenseInfo:firstyear=2010&license=viewergpl$
 * 
 * Copyright (c) 2010, Linden Research, Inc.
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

#ifndef LL_LLFLOATERVOICEEFFECT_H
#define LL_LLFLOATERVOICEEFFECT_H

#include "llfloater.h"
#include "llvoiceclient.h"

class LLButton;
class LLScrollListCtrl;

class LLFloaterVoiceEffect
	: public LLFloater
	, public LLVoiceEffectObserver
{
public:
	LOG_CLASS(LLFloaterVoiceEffect);

	LLFloaterVoiceEffect(const LLSD& key);
	virtual ~LLFloaterVoiceEffect();

	virtual BOOL postBuild();
	virtual void onClose(bool app_quitting);

private:
	enum ColumnIndex
	{
		NAME_COLUMN = 0,
		DATE_COLUMN = 1,
	};

	void refreshEffectList();
	void updateControls();

	/// Called by voice effect provider when voice effect list is changed.
	virtual void onVoiceEffectChanged(bool effect_list_updated);

	void onClickRecord();
	void onClickPlay();
	void onClickStop();
//	void onClickActivate();

	LLUUID mSelectedID;
	LLScrollListCtrl* mVoiceEffectList;
};

#endif
