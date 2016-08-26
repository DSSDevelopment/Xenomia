
// Moved from sdl/i_system.cpp

#include <string.h>

#include <SDL.h>

#include "bitmap.h"
#include "v_palette.h"
#include "textures.h"

bool I_SetCursor(FTexture *cursorpic)
{
	static SDL_Cursor *cursor;
	static SDL_Surface *cursorSurface;

	if (cursorpic != NULL && cursorpic->UseType != FTexture::TEX_Null)
	{
		// Must be no larger than 64x64.
		if (cursorpic->GetWidth() > 64 || cursorpic->GetHeight() > 64)
		{
			return false;
		}

		if (cursorSurface == NULL)
			cursorSurface = SDL_CreateRGBSurface (0, 64, 64, 32, MAKEARGB(0,255,0,0), MAKEARGB(0,0,255,0), MAKEARGB(0,0,0,255), MAKEARGB(255,0,0,0));

		SDL_LockSurface(cursorSurface);
		BYTE buffer[64*64*4];
		memset(buffer, 0, 64*64*4);
		FBitmap bmp(buffer, 64*4, 64, 64);
		cursorpic->CopyTrueColorPixels(&bmp, 0, 0);
		memcpy(cursorSurface->pixels, bmp.GetPixels(), 64*64*4);
		SDL_UnlockSurface(cursorSurface);

		if (cursor)
			SDL_FreeCursor (cursor);
		cursor = SDL_CreateColorCursor (cursorSurface, 0, 0);
		SDL_SetCursor (cursor);
	}
	else
	{
		if (cursor)
		{
			SDL_SetCursor (NULL);
			SDL_FreeCursor (cursor);
			cursor = NULL;
		}
		if (cursorSurface != NULL)
		{
			SDL_FreeSurface(cursorSurface);
			cursorSurface = NULL;
		}
	}
	return true;
}

void I_SetMainWindowVisible(bool visible)
{

}
