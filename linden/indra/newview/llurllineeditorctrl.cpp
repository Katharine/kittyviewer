/** 
 * @file llurllineeditorctrl.cpp
 * @brief LLURLLineEditor base class
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

#include "llviewerprecompiledheaders.h"

#include "llclipboard.h"
#include "lluictrlfactory.h"

#include "llurllineeditorctrl.h"

#include "llweb.h"
#include "llslurl.h"

//Constructor
LLURLLineEditor::LLURLLineEditor(const LLLineEditor::Params& p)
: LLLineEditor(p){

}

// copy selection to clipboard
void LLURLLineEditor::copy()
{
	if( canCopy() )
	{
		copyEscapedURLToClipboard();
	}
}

// cut selection to clipboard
void LLURLLineEditor::cut()
{
	if( canCut() )
	{
		// Prepare for possible rollback
		LLURLLineEditorRollback rollback( this );

		copyEscapedURLToClipboard();

		deleteSelection();

		// Validate new string and rollback the if needed.
		BOOL need_to_rollback = ( mPrevalidateFunc && !mPrevalidateFunc( mText.getWString() ) );
		if( need_to_rollback )
		{
			rollback.doRollback( this );
			LLUI::reportBadKeystroke();
		}
		else
		if( mKeystrokeCallback )
		{
			mKeystrokeCallback( this );
		}
	}
}
// Copies escaped URL to clipboard
void LLURLLineEditor::copyEscapedURLToClipboard()
{
	S32 left_pos = llmin( mSelectionStart, mSelectionEnd );
	S32 length = llabs( mSelectionStart - mSelectionEnd );

	const std::string unescaped_text = wstring_to_utf8str(mText.getWString().substr(left_pos, length));
	LLWString text_to_copy;
	// *HACK: Because LLSLURL is currently broken we cannot use it to check if unescaped_text is a valid SLURL (see EXT-8335).
	if (LLStringUtil::startsWith(unescaped_text, "http://")) // SLURL
		text_to_copy = utf8str_to_wstring(LLWeb::escapeURL(unescaped_text));
	else // human-readable location
		text_to_copy = utf8str_to_wstring(unescaped_text);
		
	gClipboard.copyFromString( text_to_copy );
}
