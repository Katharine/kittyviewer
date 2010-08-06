/**
 * @file llrendertype.h
 * @brief LLRenderType class definition
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

#ifndef LL_LLRENDERTYPE_H
#define LL_LLRENDERTYPE_H

enum render_pool_type
{
	POOL_SIMPLE = 1,
	POOL_TERRAIN,
	POOL_TREE,
	POOL_SKY,
	POOL_WL_SKY,
	POOL_GROUND,
	POOL_GRASS,
	POOL_FULLBRIGHT,
	POOL_BUMP,
	POOL_INVISIBLE, // see below *
	POOL_AVATAR,
	POOL_VOIDWATER,
	POOL_WATER,
	POOL_GLOW,
	POOL_ALPHA,
	END_POOL_TYPES,
	// * invisiprims work by rendering to the depth buffer but not the color buffer, occluding anything rendered after them
	// - and the LLDrawPool types enum controls what order things are rendered in
	// - so, it has absolute control over what invisprims block
	// ...invisiprims being rendered in pool_invisible
	// ...shiny/bump mapped objects in rendered in POOL_BUMP
};

enum render_pass_type
{
	PASS_SIMPLE = END_POOL_TYPES,
	PASS_GRASS,
	PASS_FULLBRIGHT,
	PASS_INVISIBLE,
	PASS_INVISI_SHINY,
	PASS_FULLBRIGHT_SHINY,
	PASS_SHINY,
	PASS_BUMP,
	PASS_POST_BUMP,
	PASS_GLOW,
	PASS_ALPHA,
	PASS_ALPHA_MASK,
	PASS_FULLBRIGHT_ALPHA_MASK,
	PASS_ALPHA_SHADOW,
	END_PASS_TYPES,
};

enum render_object_type
{
	OBJECT_HUD = END_PASS_TYPES,
	OBJECT_VOLUME,
	OBJECT_PARTICLES,
	OBJECT_CLOUDS,
	OBJECT_HUD_PARTICLES,
	END_RENDER_TYPES,
};

// Constructor-less 'Plain Old Data' struct, so that compiler optimization can turn
// constructs like (1 << RENDER_TYPE_POOL_SIMPLE) | (1 << RENDER_TYPE_POOL_TERRAIN) | ...
// into a constant. For the same reason, mIndex has to be public.
//
// Therefore: don't use this type anywhere but in this header file.
// Use LLRenderType instead.
struct LLRenderTypePOD {
  int mIndex;				// The index is used for switch() and as array index.

  friend bool operator==(LLRenderTypePOD const& rt1, LLRenderTypePOD const& rt2) { return rt1.mIndex == rt2.mIndex; }
  friend bool operator!=(LLRenderTypePOD const& rt1, LLRenderTypePOD const& rt2) { return rt1.mIndex != rt2.mIndex; }
  friend bool operator<(LLRenderTypePOD const& rt1, LLRenderTypePOD const& rt2) { return rt1.mIndex < rt2.mIndex; }
  friend bool operator>(LLRenderTypePOD const& rt1, LLRenderTypePOD const& rt2) { return rt1.mIndex > rt2.mIndex; }
  friend std::ostream& operator<<(std::ostream& os, LLRenderTypePOD const& rt) { return os << rt.mIndex; }
};

// RENDER_TYPE_* constants.
static LLRenderTypePOD const RENDER_TYPE_NONE = { 0 };
static LLRenderTypePOD const RENDER_TYPE_POOL_SIMPLE = { POOL_SIMPLE };
static LLRenderTypePOD const RENDER_TYPE_POOL_TERRAIN = { POOL_TERRAIN };
static LLRenderTypePOD const RENDER_TYPE_POOL_TREE = { POOL_TREE };
static LLRenderTypePOD const RENDER_TYPE_POOL_SKY = { POOL_SKY };
static LLRenderTypePOD const RENDER_TYPE_POOL_WL_SKY = { POOL_WL_SKY };
static LLRenderTypePOD const RENDER_TYPE_POOL_GROUND = { POOL_GROUND };
static LLRenderTypePOD const RENDER_TYPE_POOL_GRASS = { POOL_GRASS };
static LLRenderTypePOD const RENDER_TYPE_POOL_FULLBRIGHT = { POOL_FULLBRIGHT };
static LLRenderTypePOD const RENDER_TYPE_POOL_BUMP = { POOL_BUMP };
static LLRenderTypePOD const RENDER_TYPE_POOL_INVISIBLE = { POOL_INVISIBLE };
static LLRenderTypePOD const RENDER_TYPE_POOL_AVATAR = { POOL_AVATAR };
static LLRenderTypePOD const RENDER_TYPE_POOL_VOIDWATER = { POOL_VOIDWATER };
static LLRenderTypePOD const RENDER_TYPE_POOL_WATER = { POOL_WATER };
static LLRenderTypePOD const RENDER_TYPE_POOL_GLOW = { POOL_GLOW };
static LLRenderTypePOD const RENDER_TYPE_POOL_ALPHA = { POOL_ALPHA };
static LLRenderTypePOD const RENDER_TYPE_PASS_SIMPLE = { PASS_SIMPLE };
static LLRenderTypePOD const RENDER_TYPE_PASS_GRASS = { PASS_GRASS };
static LLRenderTypePOD const RENDER_TYPE_PASS_FULLBRIGHT = { PASS_FULLBRIGHT };
static LLRenderTypePOD const RENDER_TYPE_PASS_INVISIBLE = { PASS_INVISIBLE };
static LLRenderTypePOD const RENDER_TYPE_PASS_INVISI_SHINY = { PASS_INVISI_SHINY };
static LLRenderTypePOD const RENDER_TYPE_PASS_FULLBRIGHT_SHINY = { PASS_FULLBRIGHT_SHINY };
static LLRenderTypePOD const RENDER_TYPE_PASS_SHINY = { PASS_SHINY };
static LLRenderTypePOD const RENDER_TYPE_PASS_BUMP = { PASS_BUMP };
static LLRenderTypePOD const RENDER_TYPE_PASS_POST_BUMP = { PASS_POST_BUMP };
static LLRenderTypePOD const RENDER_TYPE_PASS_GLOW = { PASS_GLOW };
static LLRenderTypePOD const RENDER_TYPE_PASS_ALPHA = { PASS_ALPHA };
static LLRenderTypePOD const RENDER_TYPE_PASS_ALPHA_MASK = { PASS_ALPHA_MASK };
static LLRenderTypePOD const RENDER_TYPE_PASS_FULLBRIGHT_ALPHA_MASK = { PASS_FULLBRIGHT_ALPHA_MASK };
static LLRenderTypePOD const RENDER_TYPE_PASS_ALPHA_SHADOW = { PASS_ALPHA_SHADOW };
static LLRenderTypePOD const RENDER_TYPE_HUD = { OBJECT_HUD };
static LLRenderTypePOD const RENDER_TYPE_VOLUME = { OBJECT_VOLUME };
static LLRenderTypePOD const RENDER_TYPE_PARTICLES = { OBJECT_PARTICLES };
static LLRenderTypePOD const RENDER_TYPE_CLOUDS = { OBJECT_CLOUDS };
static LLRenderTypePOD const RENDER_TYPE_HUD_PARTICLES = { OBJECT_HUD_PARTICLES };
static LLRenderTypePOD const RENDER_TYPE_END_RENDER_TYPES = { END_RENDER_TYPES };

// To protect access to mIndex, the access to the base class is protected.
class LLRenderType : protected LLRenderTypePOD
{
public:
  LLRenderType(LLRenderTypePOD const& type) { mIndex = type.mIndex; }
  explicit LLRenderType(int index) { mIndex = index; }

  // Read access to the under laying 'index'; use only for temporaries (ie switch arguments and array index).
  int index() const { return mIndex; }

  // Because the base class is inaccessible elsewhere, we have to repeat these functions
  // here, overloaded for LLRenderType itself.
  friend bool operator==(LLRenderType const& rt1, LLRenderTypePOD const& rt2) { return rt1.mIndex == rt2.mIndex; }
  friend bool operator==(LLRenderTypePOD const& rt1, LLRenderType const& rt2) { return rt1.mIndex == rt2.mIndex; }
  friend bool operator==(LLRenderType const& rt1, LLRenderType const& rt2) { return rt1.mIndex == rt2.mIndex; }
  friend bool operator!=(LLRenderType const& rt1, LLRenderTypePOD const& rt2) { return rt1.mIndex != rt2.mIndex; }
  friend bool operator!=(LLRenderTypePOD const& rt1, LLRenderType const& rt2) { return rt1.mIndex != rt2.mIndex; }
  friend bool operator!=(LLRenderType const& rt1, LLRenderType const& rt2) { return rt1.mIndex != rt2.mIndex; }
  friend bool operator<(LLRenderType const& rt1, LLRenderTypePOD const& rt2) { return rt1.mIndex < rt2.mIndex; }
  friend bool operator<(LLRenderTypePOD const& rt1, LLRenderType const& rt2) { return rt1.mIndex < rt2.mIndex; }
  friend bool operator<(LLRenderType const& rt1, LLRenderType const& rt2) { return rt1.mIndex < rt2.mIndex; }
  friend bool operator>(LLRenderType const& rt1, LLRenderTypePOD const& rt2) { return rt1.mIndex > rt2.mIndex; }
  friend bool operator>(LLRenderTypePOD const& rt1, LLRenderType const& rt2) { return rt1.mIndex > rt2.mIndex; }
  friend bool operator>(LLRenderType const& rt1, LLRenderType const& rt2) { return rt1.mIndex > rt2.mIndex; }
  friend std::ostream& operator<<(std::ostream& os, LLRenderType const& rt) { return os << rt.mIndex; }
};

#endif //LL_LLRENDERTYPE_H
