/** 
 * @file lllistcontextmenu.h
 * @brief Base class of misc lists' context menus
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

#ifndef LL_LLLISTCONTEXTMENU_H
#define LL_LLLISTCONTEXTMENU_H

#include "llhandle.h"
#include "lluuid.h"
#include "llview.h"

class LLView;
class LLContextMenu;

/**
 * Context menu for single or multiple list items.
 * 
 * Derived classes must implement contextMenu().
 * 
 * Typical usage:
 * <code>
 * my_context_menu->show(parent_view, selected_list_items_ids, x, y);
 * </code>
 */
class LLListContextMenu
{
public:
	LLListContextMenu();
	virtual ~LLListContextMenu();

	/**
	 * Show the menu at specified coordinates.
	 *
	 * @param spawning_view View to spawn at.
	 * @param uuids An array of list items ids.
	 * @param x Horizontal coordinate in the spawn_view's coordinate frame.
	 * @param y Vertical coordinate in the spawn_view's coordinate frame.
	 */
	virtual void show(LLView* spawning_view, const uuid_vec_t& uuids, S32 x, S32 y);

	virtual void hide();

protected:
	typedef boost::function<void (const LLUUID& id)> functor_t;

	virtual LLContextMenu* createMenu() = 0;

	static LLContextMenu* createFromFile(const std::string& filename);
	static void handleMultiple(functor_t functor, const uuid_vec_t& ids);

	uuid_vec_t			mUUIDs;
	LLContextMenu*		mMenu;
	LLHandle<LLView>	mMenuHandle;
};

#endif // LL_LLLISTCONTEXTMENU_H
