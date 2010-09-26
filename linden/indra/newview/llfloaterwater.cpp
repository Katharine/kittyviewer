/** 
 * @file llfloaterwater.cpp
 * @brief LLFloaterWater class definition
 *
 * $LicenseInfo:firstyear=2007&license=viewerlgpl$
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

#include "llviewerprecompiledheaders.h"

#include "llfloaterwater.h"

#include "pipeline.h"
#include "llsky.h"

#include "llfloaterreg.h"
#include "llsliderctrl.h"
#include "llspinctrl.h"
#include "llcolorswatch.h"
#include "llcheckboxctrl.h"
#include "lltexturectrl.h"
#include "lluictrlfactory.h"
#include "llviewercamera.h"
#include "llcombobox.h"
#include "lllineeditor.h"
#include "llnotificationsutil.h"
#include "llfloaterdaycycle.h"
#include "llboost.h"
#include "llmultisliderctrl.h"

#include "llagent.h"
#include "llinventorymodel.h"
#include "llviewerinventory.h"

#include "v4math.h"
#include "llviewerdisplay.h"
#include "llviewercontrol.h"
#include "llviewerwindow.h"
#include "llsavedsettingsglue.h"

#include "llwaterparamset.h"
#include "llwaterparammanager.h"
#include "llpostprocess.h"

#undef max

std::set<std::string> LLFloaterWater::sDefaultPresets;

LLFloaterWater::LLFloaterWater(const LLSD& key)
  : LLFloater(key)
{
}

LLFloaterWater::~LLFloaterWater()
{
}
BOOL LLFloaterWater::postBuild()
{

	std::string def_water = getString("WLDefaultWaterNames");

	// no editing or deleting of the blank string
	sDefaultPresets.insert("");
	boost_tokenizer tokens(def_water, boost::char_separator<char>(":"));
	for (boost_tokenizer::iterator token_iter = tokens.begin(); token_iter != tokens.end(); ++token_iter)
	{
		std::string tok(*token_iter);
		sDefaultPresets.insert(tok);
	}
	
	// add the combo boxes
	LLComboBox* comboBox = getChild<LLComboBox>("WaterPresetsCombo");

	if(comboBox != NULL) {

		std::map<std::string, LLWaterParamSet>::iterator mIt = 
			LLWaterParamManager::instance()->mParamList.begin();
		for(; mIt != LLWaterParamManager::instance()->mParamList.end(); mIt++) 
		{
			comboBox->add(mIt->first);
		}

		// set defaults on combo boxes
		comboBox->selectByValue(LLSD("Default"));
	}
	// load it up
	initCallbacks();
	syncMenu();
	return TRUE;
}
void LLFloaterWater::initCallbacks(void) {

	LLWaterParamManager * param_mgr = LLWaterParamManager::instance();

	getChild<LLUICtrl>("WaterFogColor")->setCommitCallback(boost::bind(&LLFloaterWater::onWaterFogColorMoved, this, _1, &param_mgr->mFogColor));

	// 
	getChild<LLUICtrl>("WaterGlow")->setCommitCallback(boost::bind(&LLFloaterWater::onColorControlAMoved, this, _1, &param_mgr->mFogColor));

	// fog density
	getChild<LLUICtrl>("WaterFogDensity")->setCommitCallback(boost::bind(&LLFloaterWater::onExpFloatControlMoved, this, _1, &param_mgr->mFogDensity));
	getChild<LLUICtrl>("WaterUnderWaterFogMod")->setCommitCallback(boost::bind(&LLFloaterWater::onFloatControlMoved, this, _1, &param_mgr->mUnderWaterFogMod));

	// blue density
	getChild<LLUICtrl>("WaterNormalScaleX")->setCommitCallback(boost::bind(&LLFloaterWater::onVector3ControlXMoved, this, _1, &param_mgr->mNormalScale));
	getChild<LLUICtrl>("WaterNormalScaleY")->setCommitCallback(boost::bind(&LLFloaterWater::onVector3ControlYMoved, this, _1, &param_mgr->mNormalScale));
	getChild<LLUICtrl>("WaterNormalScaleZ")->setCommitCallback(boost::bind(&LLFloaterWater::onVector3ControlZMoved, this, _1, &param_mgr->mNormalScale));

	// fresnel
	getChild<LLUICtrl>("WaterFresnelScale")->setCommitCallback(boost::bind(&LLFloaterWater::onFloatControlMoved, this, _1, &param_mgr->mFresnelScale));
	getChild<LLUICtrl>("WaterFresnelOffset")->setCommitCallback(boost::bind(&LLFloaterWater::onFloatControlMoved, this, _1, &param_mgr->mFresnelOffset));

	// scale above/below
	getChild<LLUICtrl>("WaterScaleAbove")->setCommitCallback(boost::bind(&LLFloaterWater::onFloatControlMoved, this, _1, &param_mgr->mScaleAbove));
	getChild<LLUICtrl>("WaterScaleBelow")->setCommitCallback(boost::bind(&LLFloaterWater::onFloatControlMoved, this, _1, &param_mgr->mScaleBelow));

	// blur mult
	getChild<LLUICtrl>("WaterBlurMult")->setCommitCallback(boost::bind(&LLFloaterWater::onFloatControlMoved, this, _1, &param_mgr->mBlurMultiplier));

	// Load/save
// 	getChild<LLUICtrl>("WaterLoadPreset")->setCommitCallback(boost::bind(&LLFloaterWater::onLoadPreset, this));
	getChild<LLUICtrl>("WaterNewPreset")->setCommitCallback(boost::bind(&LLFloaterWater::onNewPreset, this));
	getChild<LLUICtrl>("WaterSavePreset")->setCommitCallback(boost::bind(&LLFloaterWater::onSavePreset, this));
	getChild<LLUICtrl>("WaterDeletePreset")->setCommitCallback(boost::bind(&LLFloaterWater::onDeletePreset, this));
	getChild<LLUICtrl>("WaterSaveNotecard")->setCommitCallback(boost::bind(&LLFloaterWater::onSaveNotecard, this));

	// wave direction
	getChild<LLUICtrl>("WaterWave1DirX")->setCommitCallback(boost::bind(&LLFloaterWater::onVector2ControlXMoved, this, _1, &param_mgr->mWave1Dir));
	getChild<LLUICtrl>("WaterWave1DirY")->setCommitCallback(boost::bind(&LLFloaterWater::onVector2ControlYMoved, this, _1, &param_mgr->mWave1Dir));
	getChild<LLUICtrl>("WaterWave2DirX")->setCommitCallback(boost::bind(&LLFloaterWater::onVector2ControlXMoved, this, _1, &param_mgr->mWave2Dir));
	getChild<LLUICtrl>("WaterWave2DirY")->setCommitCallback(boost::bind(&LLFloaterWater::onVector2ControlYMoved, this, _1, &param_mgr->mWave2Dir));

	getChild<LLUICtrl>("WaterPresetsCombo")->setCommitCallback(boost::bind(&LLFloaterWater::onChangePresetName, this, _1));

	LLTextureCtrl* textCtrl = getChild<LLTextureCtrl>("WaterNormalMap");
	textCtrl->setDefaultImageAssetID(DEFAULT_WATER_NORMAL);
	getChild<LLUICtrl>("WaterNormalMap")->setCommitCallback(boost::bind(&LLFloaterWater::onNormalMapPicked, this, _1));
}

bool LLFloaterWater::newPromptCallback(const LLSD& notification, const LLSD& response)
{
	std::string text = response["message"].asString();
	S32 option = LLNotificationsUtil::getSelectedOption(notification, response);

	if(text == "")
	{
		return false;
	}

	if(option == 0) {
		LLComboBox* comboBox = getChild<LLComboBox>( "WaterPresetsCombo");

		LLWaterParamManager * param_mgr = LLWaterParamManager::instance();

		// add the current parameters to the list
		// see if it's there first
		std::map<std::string, LLWaterParamSet>::iterator mIt = 
			param_mgr->mParamList.find(text);

		// if not there, add a new one
		if(mIt == param_mgr->mParamList.end()) 
		{
			param_mgr->addParamSet(text, param_mgr->mCurParams);
			comboBox->add(text);
			comboBox->sortByName();

			comboBox->setSelectedByValue(text, true);

			param_mgr->savePreset(text);

		// otherwise, send a message to the user
		} 
		else 
		{
			LLNotificationsUtil::add("ExistsWaterPresetAlert");
		}
	}
	return false;
}

void LLFloaterWater::syncMenu()
{
	bool err;

	LLWaterParamManager * param_mgr = LLWaterParamManager::instance();

	LLWaterParamSet & current_params = param_mgr->mCurParams;

	// blue horizon
	param_mgr->mFogColor = current_params.getVector4(param_mgr->mFogColor.mName, err);

	LLColor4 col = param_mgr->getFogColor();
	getChild<LLUICtrl>("WaterGlow")->setValue(col.mV[3]);
	col.mV[3] = 1.0f;
	LLColorSwatchCtrl* colCtrl = getChild<LLColorSwatchCtrl>("WaterFogColor");

	colCtrl->set(col);

	// fog and wavelets
	param_mgr->mFogDensity.mExp = 
		log(current_params.getFloat(param_mgr->mFogDensity.mName, err)) / 
		log(param_mgr->mFogDensity.mBase);
	param_mgr->setDensitySliderValue(param_mgr->mFogDensity.mExp);
	getChild<LLUICtrl>("WaterFogDensity")->setValue(param_mgr->mFogDensity.mExp);
	
	param_mgr->mUnderWaterFogMod.mX = 
		current_params.getFloat(param_mgr->mUnderWaterFogMod.mName, err);
	getChild<LLUICtrl>("WaterUnderWaterFogMod")->setValue(param_mgr->mUnderWaterFogMod.mX);

	param_mgr->mNormalScale = current_params.getVector3(param_mgr->mNormalScale.mName, err);
	getChild<LLUICtrl>("WaterNormalScaleX")->setValue(param_mgr->mNormalScale.mX);
	getChild<LLUICtrl>("WaterNormalScaleY")->setValue(param_mgr->mNormalScale.mY);
	getChild<LLUICtrl>("WaterNormalScaleZ")->setValue(param_mgr->mNormalScale.mZ);

	// Fresnel
	param_mgr->mFresnelScale.mX = current_params.getFloat(param_mgr->mFresnelScale.mName, err);
	getChild<LLUICtrl>("WaterFresnelScale")->setValue(param_mgr->mFresnelScale.mX);
	param_mgr->mFresnelOffset.mX = current_params.getFloat(param_mgr->mFresnelOffset.mName, err);
	getChild<LLUICtrl>("WaterFresnelOffset")->setValue(param_mgr->mFresnelOffset.mX);

	// Scale Above/Below
	param_mgr->mScaleAbove.mX = current_params.getFloat(param_mgr->mScaleAbove.mName, err);
	getChild<LLUICtrl>("WaterScaleAbove")->setValue(param_mgr->mScaleAbove.mX);
	param_mgr->mScaleBelow.mX = current_params.getFloat(param_mgr->mScaleBelow.mName, err);
	getChild<LLUICtrl>("WaterScaleBelow")->setValue(param_mgr->mScaleBelow.mX);

	// blur mult
	param_mgr->mBlurMultiplier.mX = current_params.getFloat(param_mgr->mBlurMultiplier.mName, err);
	getChild<LLUICtrl>("WaterBlurMult")->setValue(param_mgr->mBlurMultiplier.mX);

	// wave directions
	param_mgr->mWave1Dir = current_params.getVector2(param_mgr->mWave1Dir.mName, err);
	getChild<LLUICtrl>("WaterWave1DirX")->setValue(param_mgr->mWave1Dir.mX);
	getChild<LLUICtrl>("WaterWave1DirY")->setValue(param_mgr->mWave1Dir.mY);

	param_mgr->mWave2Dir = current_params.getVector2(param_mgr->mWave2Dir.mName, err);
	getChild<LLUICtrl>("WaterWave2DirX")->setValue(param_mgr->mWave2Dir.mX);
	getChild<LLUICtrl>("WaterWave2DirY")->setValue(param_mgr->mWave2Dir.mY);

	LLTextureCtrl* textCtrl = getChild<LLTextureCtrl>("WaterNormalMap");
	textCtrl->setImageAssetID(param_mgr->getNormalMapID());
}


// vector control callbacks
void LLFloaterWater::onVector3ControlXMoved(LLUICtrl* ctrl, WaterVector3Control* vectorControl)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);

	vectorControl->mX = sldrCtrl->getValueF32();

	vectorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

// vector control callbacks
void LLFloaterWater::onVector3ControlYMoved(LLUICtrl* ctrl, WaterVector3Control* vectorControl)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);

	vectorControl->mY = sldrCtrl->getValueF32();

	vectorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

// vector control callbacks
void LLFloaterWater::onVector3ControlZMoved(LLUICtrl* ctrl, WaterVector3Control* vectorControl)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);

	vectorControl->mZ = sldrCtrl->getValueF32();

	vectorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}


// vector control callbacks
void LLFloaterWater::onVector2ControlXMoved(LLUICtrl* ctrl, WaterVector2Control* vectorControl)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);

	vectorControl->mX = sldrCtrl->getValueF32();

	vectorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

// vector control callbacks
void LLFloaterWater::onVector2ControlYMoved(LLUICtrl* ctrl, WaterVector2Control* vectorControl)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);

	vectorControl->mY = sldrCtrl->getValueF32();

	vectorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

// color control callbacks
void LLFloaterWater::onColorControlRMoved(LLUICtrl* ctrl, WaterColorControl* colorControl)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);

	colorControl->mR = sldrCtrl->getValueF32();

	// move i if it's the max
	if(colorControl->mR >= colorControl->mG 
		&& colorControl->mR >= colorControl->mB 
		&& colorControl->mHasSliderName)
	{
		colorControl->mI = colorControl->mR;
		std::string name = colorControl->mSliderName;
		name.append("I");
		
		getChild<LLUICtrl>(name)->setValue(colorControl->mR);
	}

	colorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

void LLFloaterWater::onColorControlGMoved(LLUICtrl* ctrl, WaterColorControl* colorControl)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);

	colorControl->mG = sldrCtrl->getValueF32();

	// move i if it's the max
	if(colorControl->mG >= colorControl->mR 
		&& colorControl->mG >= colorControl->mB
		&& colorControl->mHasSliderName)
	{
		colorControl->mI = colorControl->mG;
		std::string name = colorControl->mSliderName;
		name.append("I");

		getChild<LLUICtrl>(name)->setValue(colorControl->mG);

	}

	colorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

void LLFloaterWater::onColorControlBMoved(LLUICtrl* ctrl, WaterColorControl* colorControl)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);

	colorControl->mB = sldrCtrl->getValueF32();

	// move i if it's the max
	if(colorControl->mB >= colorControl->mR
		&& colorControl->mB >= colorControl->mG
		&& colorControl->mHasSliderName)
	{
		colorControl->mI = colorControl->mB;
		std::string name = colorControl->mSliderName;
		name.append("I");

		getChild<LLUICtrl>(name)->setValue(colorControl->mB);
	}

	colorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}

void LLFloaterWater::onColorControlAMoved(LLUICtrl* ctrl, WaterColorControl* colorControl)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);

	colorControl->mA = sldrCtrl->getValueF32();

	colorControl->update(LLWaterParamManager::instance()->mCurParams);

	LLWaterParamManager::instance()->propagateParameters();
}


void LLFloaterWater::onColorControlIMoved(LLUICtrl* ctrl, WaterColorControl* colorControl)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);

	colorControl->mI = sldrCtrl->getValueF32();
	
	// only for sliders where we pass a name
	if(colorControl->mHasSliderName) 
	{
		// set it to the top
		F32 maxVal = std::max(std::max(colorControl->mR, colorControl->mG), colorControl->mB);
		F32 iVal;

		iVal = colorControl->mI;

		// get the names of the other sliders
		std::string rName = colorControl->mSliderName;
		rName.append("R");
		std::string gName = colorControl->mSliderName;
		gName.append("G");
		std::string bName = colorControl->mSliderName;
		bName.append("B");

		// handle if at 0
		if(iVal == 0)
		{
			colorControl->mR = 0;
			colorControl->mG = 0;
			colorControl->mB = 0;
		
		// if all at the start
		// set them all to the intensity
		}
		else if (maxVal == 0)
		{
			colorControl->mR = iVal;
			colorControl->mG = iVal;
			colorControl->mB = iVal;
		}
		else
		{
			// add delta amounts to each
			F32 delta = (iVal - maxVal) / maxVal;
			colorControl->mR *= (1.0f + delta);
			colorControl->mG *= (1.0f + delta);
			colorControl->mB *= (1.0f + delta);
		}

		// set the sliders to the new vals
		getChild<LLUICtrl>(rName)->setValue(colorControl->mR);
		getChild<LLUICtrl>(gName)->setValue(colorControl->mG);
		getChild<LLUICtrl>(bName)->setValue(colorControl->mB);
	}

	// now update the current parameters and send them to shaders
	colorControl->update(LLWaterParamManager::instance()->mCurParams);
	LLWaterParamManager::instance()->propagateParameters();
}

void LLFloaterWater::onExpFloatControlMoved(LLUICtrl* ctrl, WaterExpFloatControl* expFloatControl)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);

	F32 val = sldrCtrl->getValueF32();
	expFloatControl->mExp = val;
	LLWaterParamManager::instance()->setDensitySliderValue(val);

	expFloatControl->update(LLWaterParamManager::instance()->mCurParams);
	LLWaterParamManager::instance()->propagateParameters();
}

void LLFloaterWater::onFloatControlMoved(LLUICtrl* ctrl, WaterFloatControl* floatControl)
{
	LLSliderCtrl* sldrCtrl = static_cast<LLSliderCtrl*>(ctrl);

	floatControl->mX = sldrCtrl->getValueF32() / floatControl->mMult;

	floatControl->update(LLWaterParamManager::instance()->mCurParams);
	LLWaterParamManager::instance()->propagateParameters();
}
void LLFloaterWater::onWaterFogColorMoved(LLUICtrl* ctrl, WaterColorControl* colorControl)
{
	LLColorSwatchCtrl* swatch = static_cast<LLColorSwatchCtrl*>(ctrl);
	*colorControl = swatch->get();

	colorControl->update(LLWaterParamManager::instance()->mCurParams);
	LLWaterParamManager::instance()->propagateParameters();
}

void LLFloaterWater::onNormalMapPicked(LLUICtrl* ctrl)
{
	LLTextureCtrl* textCtrl = static_cast<LLTextureCtrl*>(ctrl);
	LLUUID textID = textCtrl->getImageAssetID();
	LLWaterParamManager::instance()->setNormalMapID(textID);
}

void LLFloaterWater::onNewPreset()
{
	LLNotificationsUtil::add("NewWaterPreset", LLSD(),  LLSD(), boost::bind(&LLFloaterWater::newPromptCallback, this, _1, _2));
}

void LLFloaterWater::onSavePreset()
{
	// get the name
	LLComboBox* comboBox = getChild<LLComboBox>("WaterPresetsCombo");

	// don't save the empty name
	if(comboBox->getSelectedItemLabel() == "")
	{
		return;
	}

	LLWaterParamManager::instance()->mCurParams.mName = 
		comboBox->getSelectedItemLabel();

	// check to see if it's a default and shouldn't be overwritten
	std::set<std::string>::iterator sIt = sDefaultPresets.find(
		comboBox->getSelectedItemLabel());
	if(sIt != sDefaultPresets.end() && !gSavedSettings.getBOOL("WaterEditPresets")) 
	{
		LLNotificationsUtil::add("WLNoEditDefault");
		return;
	}

	LLNotificationsUtil::add("WLSavePresetAlert", LLSD(), LLSD(), boost::bind(&LLFloaterWater::saveAlertCallback, this, _1, _2));
}

class KVFloaterWaterNotecardCreatedCallback : public LLInventoryCallback
{
public:
	void fire(const LLUUID& inv_item);
};

void LLFloaterWater::onSaveNotecard()
{
	// get the name
	LLComboBox* comboBox = getChild<LLComboBox>("WaterPresetsCombo");
	
	// don't save the empty name
	if(comboBox->getSelectedItemLabel() == "")
		return;
	
	// Check if this is already a notecard.
	if(LLWaterParamManager::instance()->mCurParams.mInventoryID.notNull())
	{
		LLNotificationsUtil::add("KittyWLSaveNotecardAlert", LLSD(), LLSD(), boost::bind(&LLFloaterWater::saveNotecardCallback, this, _1, _2));
	}
	else
	{
		// Make sure we have a ".ww" extension.
		std::string name = comboBox->getSelectedItemLabel();
		if(name.length() > 2 && name.compare(name.length() - 3, 3, ".ww") != 0)
		{
			name += ".ww";
		}
		LLPointer<KVFloaterWaterNotecardCreatedCallback> cb = new KVFloaterWaterNotecardCreatedCallback();
		// Create a notecard and then save it.
		create_inventory_item(gAgent.getID(), 
							  gAgent.getSessionID(), 
							  gInventory.findCategoryUUIDForType(LLFolderType::FT_NOTECARD),
							  LLTransactionID::tnull,
							  name,
							  "Water settings (Kitty Viewer compatible)",
							  LLAssetType::AT_NOTECARD,
							  LLInventoryType::IT_NOTECARD,
							  NOT_WEARABLE,
							  PERM_ITEM_UNRESTRICTED,
							  cb);
		
	}
}

void KVFloaterWaterNotecardCreatedCallback::fire(const LLUUID& inv_item)
{
	LLWaterParamManager * param_mgr = LLWaterParamManager::instance();
	param_mgr->setParamSet(param_mgr->mCurParams.mName, param_mgr->mCurParams);
	param_mgr->mParamList[param_mgr->mCurParams.mName].mInventoryID = inv_item;
	param_mgr->mCurParams.mInventoryID = inv_item;
	LL_INFOS("WindLight") << "Created inventory item " << inv_item << LL_ENDL;
	param_mgr->savePresetToNotecard(param_mgr->mCurParams.mName);
}

bool LLFloaterWater::saveNotecardCallback(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotificationsUtil::getSelectedOption(notification, response);
	// if they choose save, do it.  Otherwise, don't do anything
	if(option == 0) 
	{
		LLWaterParamManager * param_mgr = LLWaterParamManager::instance();
		param_mgr->setParamSet(param_mgr->mCurParams.mName, param_mgr->mCurParams);
		param_mgr->savePresetToNotecard(param_mgr->mCurParams.mName);
	}
	return false;
}

bool LLFloaterWater::saveAlertCallback(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotificationsUtil::getSelectedOption(notification, response);
	// if they choose save, do it.  Otherwise, don't do anything
	if(option == 0) 
	{
		LLWaterParamManager * param_mgr = LLWaterParamManager::instance();

		param_mgr->setParamSet(
			param_mgr->mCurParams.mName, 
			param_mgr->mCurParams);

		// comment this back in to save to file
		param_mgr->savePreset(param_mgr->mCurParams.mName);
	}
	return false;
}

void LLFloaterWater::onDeletePreset()
{
	LLComboBox* combo_box = getChild<LLComboBox>("WaterPresetsCombo");

	if(combo_box->getSelectedValue().asString() == "")
	{
		return;
	}

	LLSD args;
	args["SKY"] = combo_box->getSelectedValue().asString();
	LLNotificationsUtil::add("WLDeletePresetAlert", args, LLSD(), boost::bind(&LLFloaterWater::deleteAlertCallback, this, _1, _2));
}

bool LLFloaterWater::deleteAlertCallback(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotificationsUtil::getSelectedOption(notification, response);
	// if they choose delete, do it.  Otherwise, don't do anything
	if(option == 0) 
	{
		LLComboBox* combo_box = getChild<LLComboBox>("WaterPresetsCombo");
		LLFloaterDayCycle* day_cycle = LLFloaterReg::findTypedInstance<LLFloaterDayCycle>("env_day_cycle");
		LLComboBox* key_combo = NULL;

		if (day_cycle) 
		{
			key_combo = day_cycle->getChild<LLComboBox>("WaterKeyPresets");
		}

		std::string name = combo_box->getSelectedValue().asString();

		// check to see if it's a default and shouldn't be deleted
		std::set<std::string>::iterator sIt = sDefaultPresets.find(name);
		if(sIt != sDefaultPresets.end()) 
		{
			LLNotificationsUtil::add("WaterNoEditDefault");
			return false;
		}

		LLWaterParamManager::instance()->removeParamSet(name, true);
		
		// remove and choose another
		S32 new_index = combo_box->getCurrentIndex();

		combo_box->remove(name);

		if(key_combo != NULL) 
		{
			key_combo->remove(name);

			// remove from slider, as well
			day_cycle->deletePreset(name);
		}

		// pick the previously selected index after delete
		if(new_index > 0) 
		{
			new_index--;
		}
		
		if(combo_box->getItemCount() > 0) 
		{
			combo_box->setCurrentByIndex(new_index);
		}
	}
	return false;
}


void LLFloaterWater::onChangePresetName(LLUICtrl* ctrl)
{
	std::string data = ctrl->getValue().asString();
	if(!data.empty())
	{
		LLWaterParamManager::instance()->loadPreset(data);
		syncMenu();
	}
}

