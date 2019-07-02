#pragma once
/*
The MIT License (MIT)

Copyright (c) 2015 Baltasar García Perez-Schofield

https://github.com/Baltasarq/cscrutil
*/
#ifndef SCRUTIL_H_INCLUDED
#define SCRUTIL_H_INCLUDED

#include <stdbool.h>

#ifdef __cplusplus
	extern "C" {
#endif 


		/** Colors Available */
		typedef enum _Color {
			scrBlack, scrBlue, scrRed, scrMagenta,
			scrGreen, scrCyan, scrYellow, scrWhite,
			scrUndefinedColor
		} Color;

		/**
			A pos. on the screen
		*/
		typedef struct _scrPosition {
			unsigned short int row;
			unsigned short int column;
		} scrPosition;

		/*
		to keep the current values
		*/
		typedef struct _scrAttributes {
			Color paper;
			Color ink;
		} scrAttributes;

		/**
			Clear the screen
		*/
		void scrClear();

		/**

			Indicates the colors of the text to be written
			@param ink color
			@param Background color
		*/
		void scrSetColors(Color, Color);

		/**
			Indicates the colors of the text to be written
			@param color Color of ink and paper
			@see scrAttributes
		*/
		void scrSetColorsWithAttr(scrAttributes color);

		/**
			Get the attributes in use
			@return The colors as a structure scrAttributes
			@see scrAttributes
		*/
		scrAttributes scrGetCurrentAttributes();

		/**
			Get the char in a pos.
			@param Pos. as a structure scrPosition
			@return The whole value of the char
			@see scrAttributes
		*/
		int scrGetCharacterAt(scrPosition pos);

		/**
		*/
		void scrMoveCursorTo(unsigned short int fila, unsigned short int columna);

		/**
		*/
		void scrMoveCursorToPos(scrPosition pos);

		/**
			@note in Unix always returns 25x80
		*/
		scrPosition scrGetConsoleSize();

		/**
		In case the functionality is not supported,
		returns -1 in both fields of scrPosition
		*/
		unsigned short int scrGetMaxRows();

		/**
		*/
		unsigned short int scrGetMaxColumns();

		/**
		in Unix always returns -1, -1
		*/
		scrPosition scrGetCursorPosition();

		/**
		*/
		void scrShowCursor(bool see);

#ifdef __cplusplus
	} /* extern "C" {*/
#endif 

#endif // SCRUTIL_H_INCLUDED

