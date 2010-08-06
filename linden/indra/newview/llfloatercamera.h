/** 
 * @file llfloatercamera.h
 * @brief Container for camera control buttons (zoom, pan, orbit)
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

#ifndef LLFLOATERCAMERA_H
#define LLFLOATERCAMERA_H

#include "lltransientdockablefloater.h"
#include "lliconctrl.h"
#include "lltextbox.h"
#include "llflatlistview.h"

class LLJoystickCameraRotate;
class LLJoystickCameraTrack;
class LLFloaterReg;
class LLPanelCameraZoom;

enum ECameraControlMode
{
	CAMERA_CTRL_MODE_MODES,
	CAMERA_CTRL_MODE_PAN,
	CAMERA_CTRL_MODE_FREE_CAMERA,
	CAMERA_CTRL_MODE_PRESETS
};

class LLFloaterCamera
	:	public LLTransientDockableFloater
{
	friend class LLFloaterReg;
	
public:

	/* whether in free camera mode */
	static bool inFreeCameraMode();
	/* callback for camera items selection changing */
	static void onClickCameraItem(const LLSD& param);

	static void onLeavingMouseLook();

	/** resets current camera mode to orbit mode */
	static void resetCameraMode();

	/** Called when Avatar is entered/exited editing appearance mode */
	static void onAvatarEditingAppearance(bool editing);

	/* determines actual mode and updates ui */
	void update();

	/*switch to one of the camera presets (front, rear, side)*/
	static void switchToPreset(const std::string& name);

	/* move to CAMERA_CTRL_MODE_PRESETS from CAMERA_CTRL_MODE_FREE_CAMERA if we are on presets panel and
	   are not in free camera mode*/
	void fromFreeToPresets();

	virtual void onOpen(const LLSD& key);
	virtual void onClose(bool app_quitting);

	LLJoystickCameraRotate* mRotate;
	LLPanelCameraZoom*	mZoom;
	LLJoystickCameraTrack*	mTrack;

private:

	LLFloaterCamera(const LLSD& val);
	~LLFloaterCamera() {};

	/* return instance if it exists - created by LLFloaterReg */
	static LLFloaterCamera* findInstance();

	/*virtual*/ BOOL postBuild();

	ECameraControlMode determineMode();

	/* resets to the previous mode */
	void toPrevMode();

	/* sets a new mode and performs related actions */
	void switchMode(ECameraControlMode mode);

	/* sets a new mode preserving previous one and updates ui*/
	void setMode(ECameraControlMode mode);

	/** set title appropriate to passed mode */
	void setModeTitle(const ECameraControlMode mode);

	/* updates the state (UI) according to the current mode */
	void updateState();

	/* update camera modes items selection and camera preset items selection according to the currently selected preset */
	void updateItemsSelection();

	void onClickBtn(ECameraControlMode mode);
	void assignButton2Mode(ECameraControlMode mode, const std::string& button_name);
	
	// fills flatlist with items from given panel
	void fillFlatlistFromPanel (LLFlatListView* list, LLPanel* panel);

	// set to true when free camera mode is selected in modes list
	// remains true until preset camera mode is chosen, or pan button is clicked, or escape pressed
	static bool sFreeCamera;
	BOOL mClosed;
	ECameraControlMode mPrevMode;
	ECameraControlMode mCurrMode;
	std::map<ECameraControlMode, LLButton*> mMode2Button;
};

/**
 * Class used to represent widgets from panel_camera_item.xml- 
 * panels that contain pictures and text. Pictures are different
 * for selected and unselected state (this state is nor stored- icons
 * are changed in setValue()). This class doesn't implement selection logic-
 * it's items are used inside of flatlist.
 */
class LLPanelCameraItem 
	: public LLPanel
{
public:
	struct Params :	public LLInitParam::Block<Params, LLPanel::Params>
	{
		Optional<LLIconCtrl::Params> icon_over;
		Optional<LLIconCtrl::Params> icon_selected;
		Optional<LLIconCtrl::Params> picture;
		Optional<LLIconCtrl::Params> selected_picture;

		Optional<LLTextBox::Params> text;
		Optional<CommitCallbackParam> mousedown_callback;
		Params();
	};
	/*virtual*/ BOOL postBuild();
	/** setting on/off background icon to indicate selected state */
	/*virtual*/ void setValue(const LLSD& value);
	// sends commit signal
	void onAnyMouseClick();
protected:
	friend class LLUICtrlFactory;
	LLPanelCameraItem(const Params&);
	LLIconCtrl* mIconOver;
	LLIconCtrl* mIconSelected;
	LLIconCtrl* mPicture;
	LLIconCtrl* mPictureSelected;
	LLTextBox* mText;
};

#endif
