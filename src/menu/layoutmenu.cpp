/*
** layoutmenu.cpp
** Layout menu implementation
**
**---------------------------------------------------------------------------
** Copyright 2016 Derek Sanchez
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

//=============================================================================
//
// base class for menu items
//
//=============================================================================

#include "v_video.h"
#include "v_font.h"
#include "cmdlib.h"
#include "gstrings.h"
#include "g_level.h"
#include "gi.h"
#include "v_palette.h"
#include "d_gui.h"
#include "d_event.h"
#include "c_dispatch.h"
#include "c_console.h"
#include "c_cvars.h"
#include "c_bind.h"
#include "gameconfigfile.h"
#include "menu/menu.h"
#include "layoutmenuitems.h"


IMPLEMENT_CLASS(DLayoutMenu)

DLayoutMenu::DLayoutMenu(DMenu *parent, FLayoutMenuDescriptor *desc)
	: DMenu(parent)
{
	mDesc = NULL;
	if (desc != NULL) Init(parent, desc);
}

void DLayoutMenu::Init(DMenu *parent, FLayoutMenuDescriptor *desc)
{
	mParentMenu = parent;
	GC::WriteBarrier(this, parent);
	mDesc = desc;
	if (desc->mCenter)
	{
		int center = 160;
		for (unsigned i = 0; i<mDesc->mItems.Size(); i++)
		{
			int xpos = mDesc->mItems[i]->GetX();
			int width = mDesc->mItems[i]->GetWidth();
			int curx = mDesc->mSelectOfsX;

			if (width > 0 && mDesc->mItems[i]->Selectable())
			{
				int left = 160 - (width - curx) / 2 - curx;
				if (left < center) center = left;
			}
		}
		for (unsigned i = 0; i<mDesc->mItems.Size(); i++)
		{
			int width = mDesc->mItems[i]->GetWidth();

			if (width > 0)
			{
				mDesc->mItems[i]->SetX(center);
			}
		}
	}
}

FLayoutMenuItem *DLayoutMenu::GetItem(FName name)
{
	for (unsigned i = 0; i<mDesc->mItems.Size(); i++)
	{
		FName nm = mDesc->mItems[i]->GetAction(NULL);
		if (nm == name) return mDesc->mItems[i];
	}
	return NULL;
}

bool DLayoutMenu::Responder(event_t *ev)
{
	return Super::Responder(ev);
}

bool DLayoutMenu::MenuEvent(int mkey, bool fromcontroller)
{
	return Super::MenuEvent(mkey, fromcontroller);
}

bool DLayoutMenu::MouseEvent(int type, int x, int y)
{
	return Super::MouseEvent(type, x, y);
}


void DLayoutMenu::Ticker()
{
	Super::Ticker();
	for (unsigned i = 0; i<mDesc->mItems.Size(); i++)
	{
		mDesc->mItems[i]->Ticker();
	}
}


void DLayoutMenu::Drawer()
{
	for (unsigned i = 0; i<mDesc->mItems.Size(); i++)
	{
		if (mDesc->mItems[i]->mEnabled) mDesc->mItems[i]->Drawer(mDesc->mSelectedItem == (int)i);
	}
	if (mDesc->mSelectedItem >= 0 && mDesc->mSelectedItem < (int)mDesc->mItems.Size())
		mDesc->mItems[mDesc->mSelectedItem]->DrawSelector(mDesc->mSelectOfsX, mDesc->mSelectOfsY, mDesc->mSelector);
	Super::Drawer();
}


// Static Items

//=============================================================================
//
// static patch
//
//=============================================================================

FLayoutMenuItemStaticPatch::FLayoutMenuItemStaticPatch(int x, int y, FTextureID patch, bool centered)
	: FLayoutMenuItem(x, y)
{
	mTexture = patch;
	mCentered = centered;
}

void FLayoutMenuItemStaticPatch::Drawer(bool selected)
{
	int x = mXpos;
	FTexture *tex = TexMan(mTexture);
	if (mYpos >= 0)
	{
		if (mCentered) x -= tex->GetScaledWidth() / 2;
		screen->DrawTexture(tex, x, mYpos, DTA_Clean, true, TAG_DONE);
	}
	else
	{
		int x = (mXpos - 160) * CleanXfac + (SCREENWIDTH >> 1);
		if (mCentered) x -= (tex->GetScaledWidth()*CleanXfac) / 2;
		screen->DrawTexture(tex, x, -mYpos*CleanYfac, DTA_CleanNoMove, true, TAG_DONE);
	}
}

//=============================================================================
//
// static text
//
//=============================================================================

FLayoutMenuItemStaticText::FLayoutMenuItemStaticText(int x, int y, const char *text, FFont *font, EColorRange color, bool centered)
	: FLayoutMenuItem(x, y)
{
	mText = ncopystring(text);
	mFont = font;
	mColor = color;
	mCentered = centered;
}

void FLayoutMenuItemStaticText::Drawer(bool selected)
{
	const char *text = mText;
	if (text != NULL)
	{
		if (*text == '$') text = GStrings(text + 1);
		if (mYpos >= 0)
		{
			int x = mXpos;
			if (mCentered) x -= mFont->StringWidth(text) / 2;
			screen->DrawText(mFont, mColor, x, mYpos, text, DTA_Clean, true, TAG_DONE);
		}
		else
		{
			int x = (mXpos - 160) * CleanXfac + (SCREENWIDTH >> 1);
			if (mCentered) x -= (mFont->StringWidth(text)*CleanXfac) / 2;
			screen->DrawText(mFont, mColor, x, -mYpos*CleanYfac, text, DTA_CleanNoMove, true, TAG_DONE);
		}
	}
}

FLayoutMenuItemStaticText::~FLayoutMenuItemStaticText()
{
	if (mText != NULL) delete[] mText;
}


// Selectable Items

//=============================================================================
//
// base class for selectable items
//
//=============================================================================

FLayoutMenuItemSelectable::FLayoutMenuItemSelectable(int x, int y, FName action, int param)
	: FLayoutMenuItem(x, y, action)
{
	mWidth = 1;
	mHeight = 1;
	mParam = param;
	mHotkey = 0;
}

bool FLayoutMenuItemSelectable::CheckCoordinate(int x, int y)
{
	return mEnabled && y >= mYpos && y < mYpos + mHeight && x >= mXpos && x < mXpos + mWidth;	// added x check.
}

bool FLayoutMenuItemSelectable::Selectable()
{
	return mEnabled;
}

bool FLayoutMenuItemSelectable::Activate()
{
	M_SetMenu(mAction, mParam);
	return true;
}

FName FLayoutMenuItemSelectable::GetAction(int *pparam)
{
	if (pparam != NULL) *pparam = mParam;
	return mAction;
}

bool FLayoutMenuItemSelectable::CheckHotkey(int c)
{
	return c == tolower(mHotkey);
}

bool FLayoutMenuItemSelectable::MouseEvent(int type, int x, int y)
{
	if (type == DMenu::MOUSE_Release)
	{
		if (DMenu::CurrentMenu->MenuEvent(MKEY_Enter, true))
		{
			return true;
		}
	}
	return false;
}

//=============================================================================
//
// text item
//
//=============================================================================

FLayoutMenuItemText::FLayoutMenuItemText(int x, int y, int hotkey, const char *text, FFont *font, EColorRange color, EColorRange color2, FName child, int param)
	: FLayoutMenuItemSelectable(x, y, child, param)
{
	mText = ncopystring(text);
	mFont = font;
	mColor = color;
	mColorSelected = color2;
	mHotkey = hotkey;
	mWidth = this->GetWidth();
	mHeight = mFont->GetHeight();
}

FLayoutMenuItemText::~FLayoutMenuItemText()
{
	if (mText != NULL)
	{
		delete[] mText;
	}
}

void FLayoutMenuItemText::Drawer(bool selected)
{
	const char *text = mText;
	if (text != NULL)
	{
		if (*text == '$') text = GStrings(text + 1);
		screen->DrawText(mFont, selected ? mColorSelected : mColor, mXpos, mYpos, text, DTA_Clean, true, TAG_DONE);
	}
}

int FLayoutMenuItemText::GetWidth()
{
	const char *text = mText;
	if (text != NULL)
	{
		if (*text == '$') text = GStrings(text + 1);
		return mFont->StringWidth(text);
	}
	return 1;
}


//=============================================================================
//
// patch item
//
//=============================================================================

FLayoutMenuItemPatch::FLayoutMenuItemPatch(int x, int y, int width, int height, int hotkey, FTextureID patch, FName child, int param)
	: FLayoutMenuItemSelectable(x, y, child, param)
{
	mWidth = width;
	mHeight = height;
	mHotkey = hotkey;
	mTexture = patch;
}

void FLayoutMenuItemPatch::Drawer(bool selected)
{
	screen->DrawTexture(TexMan(mTexture), mXpos, mYpos, DTA_Clean, true, TAG_DONE);
}

bool FLayoutMenuItemPatch::CheckCoordinate(int x, int y)
{
	return mEnabled && y >= mYpos - mHeight && y < mYpos && x >= mXpos - mWidth/2 && x < mXpos + mWidth/2;	// added x check.
}

int FLayoutMenuItemPatch::GetWidth()
{
	return mTexture.isValid()
		? TexMan[mTexture]->GetScaledWidth()
		: 0;
}


