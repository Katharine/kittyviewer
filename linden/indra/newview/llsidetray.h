/** 
 * @file LLSideTray.h
 * @brief SideBar header file
 *
 * $LicenseInfo:firstyear=2004&license=viewerlgpl$
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

#ifndef LL_LLSIDETRAY_H_
#define LL_LLSIDETRAY_H_

#include "llpanel.h"
#include "string"

class LLAccordionCtrl;
class LLSideTrayTab;

// added inheritance from LLDestroyClass<LLSideTray> to enable Side Tray perform necessary actions 
// while disconnecting viewer in LLAppViewer::disconnectViewer().
// LLDestroyClassList::instance().fireCallbacks() calls destroyClass method. See EXT-245.
class LLSideTray : public LLPanel, private LLDestroyClass<LLSideTray>
{
	friend class LLUICtrlFactory;
	friend class LLDestroyClass<LLSideTray>;
public:

	LOG_CLASS(LLSideTray);

	struct Params 
	:	public LLInitParam::Block<Params, LLPanel::Params>
	{
		// initial state
		Optional<bool>				collapsed;
		Optional<LLUIImage*>		tab_btn_image_normal,
									tab_btn_image_selected;
		
		Optional<S32>				default_button_width,
									default_button_height,
									default_button_margin;
		
		Params();
	};

	static LLSideTray*	getInstance		();
	static bool			instanceCreated	();
protected:
	LLSideTray(const Params& params);
	typedef std::vector<LLSideTrayTab*> child_vector_t;
	typedef child_vector_t::iterator					child_vector_iter_t;
	typedef child_vector_t::const_iterator  			child_vector_const_iter_t;
	typedef child_vector_t::reverse_iterator 			child_vector_reverse_iter_t;
	typedef child_vector_t::const_reverse_iterator 		child_vector_const_reverse_iter_t;
	typedef std::vector<std::string>					tab_order_vector_t;
	typedef tab_order_vector_t::const_iterator			tab_order_vector_const_iter_t;

public:

	// interface functions
	    
	/**
     * Select tab with specific name and set it active    
     */
	bool 		selectTabByName	(const std::string& name);
	
	/**
     * Select tab with specific index and set it active    
     */
	bool		selectTabByIndex(size_t index);

	/**
	 * Activate tab with "panel_name" panel
	 * if no such tab - return NULL, otherwise a pointer to the panel
	 * Pass params as array, or they may be overwritten(example - params["name"]="nearby")
	 */
	LLPanel*	showPanel		(const std::string& panel_name, const LLSD& params = LLSD());

	/**
	 * Toggling Side Tray tab which contains "sub_panel" child of "panel_name" panel.
	 * If "sub_panel" is not visible Side Tray is opened to display it,
	 * otherwise Side Tray is collapsed.
	 * params are passed to "panel_name" panel onOpen().
	 */
	void		togglePanel		(LLPanel* &sub_panel, const std::string& panel_name, const LLSD& params = LLSD());

	/*
	 * get the panel (don't show it or do anything else with it)
	 */
    LLPanel*	getPanel		(const std::string& panel_name);
    LLPanel*	getActivePanel	();
    bool		isPanelActive	(const std::string& panel_name);
	/*
	 * get currently active tab
	 */
    const LLSideTrayTab*	getActiveTab() const { return mActiveTab; }

	/*
     * collapse SideBar, hiding visible tab and moving tab buttons
     * to the right corner of the screen
     */
	void		collapseSideBar	();
    
	/*
     * expand SideBar
     */
	void		expandSideBar	();


	/**
	 *hightlight if focused. manly copypaste from highlightFocusedFloater
	 */
	void		highlightFocused();

	void		setVisible(BOOL visible)
	{
		if (getParent()) getParent()->setVisible(visible);
	}

	LLPanel*	getButtonsPanel() { return mButtonsPanel; }

	bool		getCollapsed() { return mCollapsed; }

public:
	virtual ~LLSideTray(){};

    virtual BOOL postBuild();

	void		onTabButtonClick(std::string name);
	void		onToggleCollapse();

	bool		addChild		(LLView* view, S32 tab_group);
	bool		removeTab		(LLSideTrayTab* tab); // Used to detach tabs temporarily
	bool		addTab			(LLSideTrayTab* tab); // Used to re-attach tabs

	BOOL		handleMouseDown	(S32 x, S32 y, MASK mask);
	
	void		reshape			(S32 width, S32 height, BOOL called_from_parent = TRUE);

	void		processTriState ();
	
	void		updateSidetrayVisibility();

	commit_signal_t& getCollapseSignal() { return mCollapseSignal; }

	void		handleLoginComplete();

	LLSideTrayTab* getTab		(const std::string& name);

	bool 		isTabAttached	(const std::string& name);

protected:
	bool		hasTabs			();

	void		createButtons	();

	LLButton*	createButton	(const std::string& name,const std::string& image,const std::string& tooltip,
									LLUICtrl::commit_callback_t callback);
	void		arrange			();
	void		detachTabs		();
	void		reflectCollapseChange();

	void		toggleTabButton	(LLSideTrayTab* tab);

	LLPanel*	openChildPanel	(LLSideTrayTab* tab, const std::string& panel_name, const LLSD& params);

private:
	// Implementation of LLDestroyClass<LLSideTray>
	static void destroyClass()
	{
		// Disable SideTray to avoid crashes. EXT-245
		if (LLSideTray::instanceCreated())
			LLSideTray::getInstance()->setEnabled(FALSE);
	}
	
private:
	LLPanel*						mButtonsPanel;
	typedef std::map<std::string,LLButton*> button_map_t;
	button_map_t					mTabButtons;
	child_vector_t					mTabs;
	child_vector_t					mDetachedTabs;
	tab_order_vector_t				mOriginalTabOrder;
	LLSideTrayTab*					mActiveTab;	
	
	commit_signal_t					mCollapseSignal;

	LLButton*						mCollapseButton;
	bool							mCollapsed;
	
	static LLSideTray*				sInstance;
};

#endif

