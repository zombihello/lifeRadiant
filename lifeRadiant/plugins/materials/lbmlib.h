//////////////////////////////////////////////////////////////////////////
//
//				  *** lifeRadiant ***
//				Copyright (C) 2018-2020
//
// Репозиторий движка:  https://github.com/zombihello/lifeRadiant
// Авторы:				Егор Погуляка (zombiHello)
//
//////////////////////////////////////////////////////////////////////////

#ifndef LBMLIB_H
#define LBMLIB_H

// LoadImage will rely on file extension to call LoadTGA loadHLW
void LoadImage( const char* name, byte** pic, int* width, int* height );

#endif // !LBMLIB_H