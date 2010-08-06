/** 
 * @file llinventoryfunctions.h
 * @brief Miscellaneous inventory-related functions and classes
 * class definition
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

#ifndef LL_LLINVENTORYFUNCTIONS_H
#define LL_LLINVENTORYFUNCTIONS_H

#include "llinventorytype.h"
#include "llfolderview.h"
#include "llfolderviewitem.h"

/********************************************************************************
 **                                                                            **
 **                    MISCELLANEOUS GLOBAL FUNCTIONS
 **/

// Is this item or its baseitem is worn, attached, etc...
BOOL get_is_item_worn(const LLUUID& id);

// Could this item be worn (correct type + not already being worn)
BOOL get_can_item_be_worn(const LLUUID& id);

BOOL get_is_item_removable(const LLInventoryModel* model, const LLUUID& id);

BOOL get_is_category_removable(const LLInventoryModel* model, const LLUUID& id);

BOOL get_is_category_renameable(const LLInventoryModel* model, const LLUUID& id);

void show_item_profile(const LLUUID& item_uuid);
void show_task_item_profile(const LLUUID& item_uuid, const LLUUID& object_id);

void show_item_original(const LLUUID& item_uuid);

void change_item_parent(LLInventoryModel* model,
									 LLViewerInventoryItem* item,
									 const LLUUID& new_parent_id,
									 BOOL restamp);

void change_category_parent(LLInventoryModel* model,
	LLViewerInventoryCategory* cat,
	const LLUUID& new_parent_id,
	BOOL restamp);

void remove_category(LLInventoryModel* model, const LLUUID& cat_id);

void rename_category(LLInventoryModel* model, const LLUUID& cat_id, const std::string& new_name);

// Generates a string containing the path to the item specified by item_id.
void append_path(const LLUUID& id, std::string& path);

/**                    Miscellaneous global functions
 **                                                                            **
 *******************************************************************************/

/********************************************************************************
 **                                                                            **
 **                    INVENTORY COLLECTOR FUNCTIONS
 **/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLInventoryCollectFunctor
//
// Base class for LLInventoryModel::collectDescendentsIf() method
// which accepts an instance of one of these objects to use as the
// function to determine if it should be added. Derive from this class
// and override the () operator to return TRUE if you want to collect
// the category or item passed in.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLInventoryCollectFunctor
{
public:
	virtual ~LLInventoryCollectFunctor(){};
	virtual bool operator()(LLInventoryCategory* cat, LLInventoryItem* item) = 0;

	static bool itemTransferCommonlyAllowed(const LLInventoryItem* item);
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLAssetIDMatches
//
// This functor finds inventory items pointing to the specified asset
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLViewerInventoryItem;

class LLAssetIDMatches : public LLInventoryCollectFunctor
{
public:
	LLAssetIDMatches(const LLUUID& asset_id) : mAssetID(asset_id) {}
	virtual ~LLAssetIDMatches() {}
	bool operator()(LLInventoryCategory* cat, LLInventoryItem* item);
	
protected:
	LLUUID mAssetID;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLLinkedItemIDMatches
//
// This functor finds inventory items linked to the specific inventory id.
// Assumes the inventory id is itself not a linked item.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLLinkedItemIDMatches : public LLInventoryCollectFunctor
{
public:
	LLLinkedItemIDMatches(const LLUUID& item_id) : mBaseItemID(item_id) {}
	virtual ~LLLinkedItemIDMatches() {}
	bool operator()(LLInventoryCategory* cat, LLInventoryItem* item);
	
protected:
	LLUUID mBaseItemID;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLIsType
//
// Implementation of a LLInventoryCollectFunctor which returns TRUE if
// the type is the type passed in during construction.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLIsType : public LLInventoryCollectFunctor
{
public:
	LLIsType(LLAssetType::EType type) : mType(type) {}
	virtual ~LLIsType() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
protected:
	LLAssetType::EType mType;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLIsNotType
//
// Implementation of a LLInventoryCollectFunctor which returns FALSE if the
// type is the type passed in during construction, otherwise false.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLIsNotType : public LLInventoryCollectFunctor
{
public:
	LLIsNotType(LLAssetType::EType type) : mType(type) {}
	virtual ~LLIsNotType() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
protected:
	LLAssetType::EType mType;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLIsOfAssetType
//
// Implementation of a LLInventoryCollectFunctor which returns TRUE if
// the item or category is of asset type passed in during construction.
// Link types are treated as links, not as the types they point to.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLIsOfAssetType : public LLInventoryCollectFunctor
{
public:
	LLIsOfAssetType(LLAssetType::EType type) : mType(type) {}
	virtual ~LLIsOfAssetType() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
protected:
	LLAssetType::EType mType;
};

class LLIsTypeWithPermissions : public LLInventoryCollectFunctor
{
public:
	LLIsTypeWithPermissions(LLAssetType::EType type, const PermissionBit perms, const LLUUID &agent_id, const LLUUID &group_id) 
		: mType(type), mPerm(perms), mAgentID(agent_id), mGroupID(group_id) {}
	virtual ~LLIsTypeWithPermissions() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
protected:
	LLAssetType::EType mType;
	PermissionBit mPerm;
	LLUUID			mAgentID;
	LLUUID			mGroupID;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLBuddyCollector
//
// Simple class that collects calling cards that are not null, and not
// the agent. Duplicates are possible.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLBuddyCollector : public LLInventoryCollectFunctor
{
public:
	LLBuddyCollector() {}
	virtual ~LLBuddyCollector() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLUniqueBuddyCollector
//
// Simple class that collects calling cards that are not null, and not
// the agent. Duplicates are discarded.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLUniqueBuddyCollector : public LLInventoryCollectFunctor
{
public:
	LLUniqueBuddyCollector() {}
	virtual ~LLUniqueBuddyCollector() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);

protected:
	std::set<LLUUID> mSeen;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLParticularBuddyCollector
//
// Simple class that collects calling cards that match a particular uuid
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LLParticularBuddyCollector : public LLInventoryCollectFunctor
{
public:
	LLParticularBuddyCollector(const LLUUID& id) : mBuddyID(id) {}
	virtual ~LLParticularBuddyCollector() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
protected:
	LLUUID mBuddyID;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLNameCategoryCollector
//
// Collects categories based on case-insensitive match of prefix
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLNameCategoryCollector : public LLInventoryCollectFunctor
{
public:
	LLNameCategoryCollector(const std::string& name) : mName(name) {}
	virtual ~LLNameCategoryCollector() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
protected:
	std::string mName;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLFindCOFValidItems
//
// Collects items that can be legitimately linked to in the COF.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLFindCOFValidItems : public LLInventoryCollectFunctor
{
public:
	LLFindCOFValidItems() {}
	virtual ~LLFindCOFValidItems() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
	
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLFindByMask
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLFindByMask : public LLInventoryCollectFunctor
{
public:
	LLFindByMask(U64 mask)
		: mFilterMask(mask)
	{}

	virtual bool operator()(LLInventoryCategory* cat, LLInventoryItem* item)
	{
		if(item && (mFilterMask & (1LL << item->getInventoryType())) )
		{
			return true;
		}

		return false;
	}

private:
	U64 mFilterMask;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLFindNonLinksByMask
//
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLFindNonLinksByMask : public LLInventoryCollectFunctor
{
public:
	LLFindNonLinksByMask(U64 mask)
		: mFilterMask(mask)
	{}

	virtual bool operator()(LLInventoryCategory* cat, LLInventoryItem* item)
	{
		if(item && !item->getIsLinkType() && (mFilterMask & (1LL << item->getInventoryType())) )
		{
			return true;
		}

		return false;
	}

	void setFilterMask(U64 mask)
	{
		mFilterMask = mask;
	}

private:
	U64 mFilterMask;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLFindWearables
//
// Collects wearables based on item type.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLFindWearables : public LLInventoryCollectFunctor
{
public:
	LLFindWearables() {}
	virtual ~LLFindWearables() {}
	virtual bool operator()(LLInventoryCategory* cat,
							LLInventoryItem* item);
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Class LLFindWearablesEx
//
// Collects wearables based on given criteria.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LLFindWearablesEx : public LLInventoryCollectFunctor
{
public:
	LLFindWearablesEx(bool is_worn, bool include_body_parts = true);
	virtual bool operator()(LLInventoryCategory* cat, LLInventoryItem* item);
private:
	bool mIncludeBodyParts;
	bool mIsWorn;
};

//Inventory collect functor collecting wearables of a specific wearable type
class LLFindWearablesOfType : public LLInventoryCollectFunctor
{
public:
	LLFindWearablesOfType(LLWearableType::EType type) : mWearableType(type) {}
	virtual ~LLFindWearablesOfType() {}
	virtual bool operator()(LLInventoryCategory* cat, LLInventoryItem* item);
	void setType(LLWearableType::EType type);

private:
	LLWearableType::EType mWearableType;
};

/** Filter out wearables-links */
class LLFindActualWearablesOfType : public LLFindWearablesOfType
{
public:
	LLFindActualWearablesOfType(LLWearableType::EType type) : LLFindWearablesOfType(type) {}
	virtual ~LLFindActualWearablesOfType() {}
	virtual bool operator()(LLInventoryCategory* cat, LLInventoryItem* item)
	{
		if (item && item->getIsLinkType()) return false;
		return LLFindWearablesOfType::operator()(cat, item);
	}
};

/* Filters out items of a particular asset type */
class LLIsTypeActual : public LLIsType
{
public:
	LLIsTypeActual(LLAssetType::EType type) : LLIsType(type) {}
	virtual ~LLIsTypeActual() {}
	virtual bool operator()(LLInventoryCategory* cat, LLInventoryItem* item)
	{
		if (item && item->getIsLinkType()) return false;
		return LLIsType::operator()(cat, item);
	}
};

// Collect non-removable folders and items.
class LLFindNonRemovableObjects : public LLInventoryCollectFunctor
{
public:
	virtual bool operator()(LLInventoryCategory* cat, LLInventoryItem* item);
};

/**                    Inventory Collector Functions
 **                                                                            **
 *******************************************************************************/

class LLInventoryState
{
public:
	// HACK: Until we can route this info through the instant message hierarchy
	static BOOL sWearNewClothing;
	static LLUUID sWearNewClothingTransactionID;	// wear all clothing in this transaction	
};

class LLSelectFirstFilteredItem : public LLFolderViewFunctor
{
public:
	LLSelectFirstFilteredItem() : mItemSelected(FALSE) {}
	virtual ~LLSelectFirstFilteredItem() {}
	virtual void doFolder(LLFolderViewFolder* folder);
	virtual void doItem(LLFolderViewItem* item);
	BOOL wasItemSelected() { return mItemSelected; }
protected:
	BOOL mItemSelected;
};

class LLOpenFilteredFolders : public LLFolderViewFunctor
{
public:
	LLOpenFilteredFolders()  {}
	virtual ~LLOpenFilteredFolders() {}
	virtual void doFolder(LLFolderViewFolder* folder);
	virtual void doItem(LLFolderViewItem* item);
};

class LLSaveFolderState : public LLFolderViewFunctor
{
public:
	LLSaveFolderState() : mApply(FALSE) {}
	virtual ~LLSaveFolderState() {}
	virtual void doFolder(LLFolderViewFolder* folder);
	virtual void doItem(LLFolderViewItem* item) {}
	void setApply(BOOL apply);
	void clearOpenFolders() { mOpenFolders.clear(); }
protected:
	std::set<LLUUID> mOpenFolders;
	BOOL mApply;
};

class LLOpenFoldersWithSelection : public LLFolderViewFunctor
{
public:
	LLOpenFoldersWithSelection() {}
	virtual ~LLOpenFoldersWithSelection() {}
	virtual void doFolder(LLFolderViewFolder* folder);
	virtual void doItem(LLFolderViewItem* item);
};

#endif // LL_LLINVENTORYFUNCTIONS_H



