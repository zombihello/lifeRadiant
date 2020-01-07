//////////////////////////////////////////////////////////////////////////
//
//				  *** lifeRadiant ***
//				Copyright (C) 2018-2020
//
// Репозиторий движка:  https://github.com/zombihello/lifeRadiant
// Авторы:				Егор Погуляка (zombiHello)
//
//////////////////////////////////////////////////////////////////////////

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>

#include "plugin.h"
#include "lbmlib.h"

#include <FreeImage/FreeImage.h>


void LoadImage( const char* name, byte** pic, int* width, int* height )
{
	char*		pBuff;
	int			nSize = vfsLoadFile( name, reinterpret_cast < void** >( &pBuff ), 0 );

	if ( nSize <= 0 )
	{
		Syn_Printf( "ERROR: Unable to read file %s\n", name );
		*pic = nullptr;
		return;
	}

	rapidjson::Document			document;
	document.Parse( pBuff );
	if ( document.HasParseError() )
	{
		Syn_Printf( "ERROR: not correct json syntax %s\n", name );
		*pic = nullptr;
		return;
	}

	Str				baseDir = g_FuncTable.m_pfnGetGamePath();

	for ( auto it = document.MemberBegin(), itEnd = document.MemberEnd(); it != itEnd; ++it )
	{
		if ( strcmp( it->name.GetString(), "editor" ) == 0 && it->value.IsString() )
		{
			bool					isError = false;
			Str						path = baseDir;
			path += "/";
			path += it->value.GetString();
			FREE_IMAGE_FORMAT		imageFormat = FIF_UNKNOWN;
			
			imageFormat = FreeImage_GetFileType( path.GetBuffer(), 0 );

			if ( imageFormat == FIF_UNKNOWN )
				imageFormat = FreeImage_GetFIFFromFilename( path.GetBuffer() );
	
			FIBITMAP* bitmap = FreeImage_Load( imageFormat, path.GetBuffer(), 0 );
			if ( !bitmap )
			{
				Syn_Printf( "ERROR: bitmap not loaded %s\n", path.GetBuffer() );
				isError = true;
				return;
			}

			FIBITMAP*  bitmap_32bits = FreeImage_ConvertTo32Bits( bitmap );
			FreeImage_Unload( bitmap );
			FreeImage_FlipVertical( bitmap_32bits );

			auto		red = FreeImage_GetChannel( bitmap_32bits, FREE_IMAGE_COLOR_CHANNEL::FICC_RED );
			auto		blue = FreeImage_GetChannel( bitmap_32bits, FREE_IMAGE_COLOR_CHANNEL::FICC_BLUE );

			FreeImage_SetChannel( bitmap_32bits, blue, FREE_IMAGE_COLOR_CHANNEL::FICC_RED );
			FreeImage_SetChannel( bitmap_32bits, red, FREE_IMAGE_COLOR_CHANNEL::FICC_BLUE );

			FreeImage_Unload( red );
			FreeImage_Unload( blue );

			byte*	tempData = FreeImage_GetBits( bitmap_32bits );
			*width = FreeImage_GetWidth( bitmap_32bits );
			*height = FreeImage_GetHeight( bitmap_32bits );

			*pic = ( byte* ) g_malloc( FreeImage_GetPitch( bitmap_32bits ) * (*height) );
			memcpy( *pic, tempData, FreeImage_GetPitch( bitmap_32bits ) * ( *height ) );

			Syn_Printf( "LOADED: material %s\n", name );
			
			FreeImage_Unload( bitmap_32bits );
			vfsFreeFile( pBuff );
			return;
		}
	}

	vfsFreeFile( pBuff );
	*pic = nullptr;
}