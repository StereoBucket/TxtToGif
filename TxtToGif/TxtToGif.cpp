// TxtToImg.cpp : Defines the entry point for the console application.
//
extern "C"{
#include "gifenc.h"
}
#include "stdafx.h"

#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

//#define WINDOWDEBUG
#define NUM_OF_CHARS 95

SDL_Surface** init_glyphs(TTF_Font* font, SDL_Color fg)
{
	SDL_Surface** glyphs = new SDL_Surface*[NUM_OF_CHARS];
	char ch[2] = " ";
	for (int i = 0; i < NUM_OF_CHARS; i++)
	{
		ch[0] = i + ' ';
		glyphs[i] = TTF_RenderText_Solid(font, ch, fg);
		if (glyphs[i] == NULL)
		{
			std::cerr << std::endl << "Glyph " << i << " couldn't be created" << std::endl << TTF_GetError() << " " << ch;
		}
	}


	return glyphs;
}
void del_glyphs(SDL_Surface** glyphs)
{
	for (int i = 0; i < NUM_OF_CHARS; i++)
		SDL_FreeSurface(glyphs[i]);
	delete[] glyphs;
}
int main(int argn, char** args)
{
	std::ifstream text(".\\input.txt");
	if (!text.is_open())
	{
		std::cout << "Error opening input.txt";
		exit(EXIT_FAILURE);
	}
	std::string prefix;
	std::cout << "gif name:";
	std::cin >> prefix;
	if (prefix.empty())
		prefix = "output";
	std::string str_text;
	int scroll_width = 6;
	std::cout << "Scroll width:";
	std::cin >> scroll_width;
	text.seekg(0, std::ios::end);
	str_text.reserve(text.tellg());
	text.seekg(0, std::ios::beg);
	str_text.assign((std::istreambuf_iterator<char>(text)), std::istreambuf_iterator<char>());
	str_text = ' ' + str_text;

	if (TTF_Init()!=0)
	{
		std::cerr << "TTF init error:" << TTF_GetError();
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	TTF_Font *font = TTF_OpenFont("C:\\Windows\\Fonts\\consola.ttf", 28);
	if (font == NULL)
	{
		std::cerr << "OpenFont error:" << TTF_GetError();
		TTF_Quit();
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	SDL_Color font_fg = { 0,255,0 };
	SDL_Surface** glyphs = init_glyphs(font,font_fg);

	SDL_Surface* bg2bpal = SDL_CreateRGBSurface(0, 32, 32, 8, 0, 0, 0, 0); 
	SDL_Color colors[2] = { {0,0,0},{0,255,0} };
	if (SDL_SetPaletteColors(bg2bpal->format->palette, colors, 0, 2) != 0)
		std::cerr << "Uh Oh";
#ifdef WINDOWDEBUG	//Display part, testing only. Just to see how letters render out in a window.
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cerr << "Init error:" << SDL_GetError();
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	SDL_Window *window = SDL_CreateWindow("Text to img", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, NULL);
	SDL_Surface* win_surface = SDL_GetWindowSurface(window);
	SDL_Rect pos_rect = { 0 };
	for (int i = 0; (pos_rect.y < win_surface->h) && i<NUM_OF_CHARS; pos_rect.y += 32)
	{
		pos_rect.x = 0;
		for (; i<NUM_OF_CHARS&&(pos_rect.x+glyphs[i]->w < win_surface->w); pos_rect.x += 32, i++)
		{
			SDL_FillRect(bg2bpal, NULL, 0);
			if (SDL_BlitSurface(glyphs[i], 0, bg2bpal, 0))
				std::cerr << "Blit failed:" << SDL_GetError();
			SDL_BlitSurface(bg2bpal, NULL, win_surface, &pos_rect);
			SDL_UpdateWindowSurface(window);
			SDL_LockSurface(bg2bpal);
			for (int i = 0; i < 32; i++)
			{
				for (int j = 0; j < 32; j++)
					std::cout << (int)((char*)(bg2bpal->pixels))[j + i * 32]<<" ";
				std::cout << std::endl;
			}
			SDL_UnlockSurface(bg2bpal);
		}
	}
	
	SDL_UpdateWindowSurface(window);
	SDL_Delay(1000);
	SDL_DestroyWindow(window);
#endif //End of display
	uint8_t palette[6] = { 0,0,0,0,255,0 };
	
	
	
	
	for (int index = 0; index < scroll_width; index++)
	{
		std::string filename = ".\\"+prefix + std::to_string(index) + ".gif";
		ge_GIF* gif = ge_new_gif(
			filename.c_str(),
			32, 32,
			palette,
			1, 0
		);
		if (!gif)
		{
			std::cout << "Could not create gif file. Skipping"<<std::endl;
			continue;
		}
		
		for (int i = 0; i <str_text.length();i++)
		{
			int ind = (i + index) % str_text.length();
			int glyph = (str_text[ind] >= ' '&&str_text[ind] < 127) ? str_text[ind] - ' ' : 0;
			SDL_Rect center_align = { 0 };
			center_align.x = 16 - glyphs[glyph]->w / 2;
			center_align.y = 16 - glyphs[glyph]->h / 2;
			SDL_FillRect(bg2bpal, 0, 0);
			SDL_BlitSurface(glyphs[glyph], 0, bg2bpal, &center_align);
			SDL_LockSurface(bg2bpal);
			std::memcpy(gif->frame, bg2bpal->pixels, 32 * 32);
			ge_add_frame(gif, 25);
			SDL_UnlockSurface(bg2bpal);
		}
		if(gif)
			ge_close_gif(gif);
		
	}

	//for (int i = 0; i < NUM_OF_CHARS; i++)
	//{
	//	SDL_Rect center_align = { 0 };
	//	center_align.x = 16 - glyphs[i]->w / 2;
	//	center_align.y = 16 - glyphs[i]->h / 2;
	//	SDL_FillRect(bg2bpal, 0, 0);
	//	SDL_BlitSurface(glyphs[i], 0, bg2bpal,&center_align);
	//	SDL_LockSurface(bg2bpal);
	//	std::memcpy(gif->frame, bg2bpal->pixels, 32 * 32);
	//	ge_add_frame(gif, 25);
	//	SDL_UnlockSurface(bg2bpal);
	//}
	text.close();
	del_glyphs(glyphs);
	TTF_CloseFont(font);
	
	TTF_Quit();
	SDL_Quit();
    return 0;
}

