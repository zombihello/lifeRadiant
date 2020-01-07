//////////////////////////////////////////////////////////////////////////
//
//				  *** lifeRadiant ***
//				Copyright (C) 2018-2020
//
// Репозиторий движка:  https://github.com/zombihello/lifeRadiant
// Авторы:				Егор Погуляка (zombiHello)
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "plugin.h"
#include "lbmlib.h"

// =============================================================================
// static variables

_QERFuncTable_1			g_FuncTable; // Radiant function table
_QERFileSystemTable		g_FileSystemTable;

// =============================================================================
// SYNAPSE

CSynapseServer*					g_pSynapseServer = NULL;
CSynapseClientMaterials			g_SynapseClient;

static const XMLConfigEntry_t entries[] =
{
	{ VFS_MAJOR, SYN_REQUIRE, sizeof( g_FileSystemTable ), &g_FileSystemTable },
	{ NULL, SYN_UNKNOWN, 0, NULL }
};

#if __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif
extern "C" CSynapseClient * SYNAPSE_DLL_EXPORT Synapse_EnumerateInterfaces( const char* version, CSynapseServer * pServer )
{
#if __GNUC__ >= 4
#pragma GCC visibility pop
#endif
	if ( strcmp( version, SYNAPSE_VERSION ) )
	{
		Syn_Printf( "ERROR: synapse API version mismatch: should be '" SYNAPSE_VERSION "', got '%s'\n", version );
		return NULL;
	}
	g_pSynapseServer = pServer;
	g_pSynapseServer->IncRef();
	Set_Syn_Printf( g_pSynapseServer->Get_Syn_Printf() );

	g_SynapseClient.AddAPI( IMAGE_MAJOR, "lmt", sizeof( _QERPlugImageTable ) );
	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( _QERFuncTable_1 ), SYN_REQUIRE, &g_FuncTable );

	if ( !g_SynapseClient.ConfigXML( pServer, NULL, entries ) )
	{
		return NULL;
	}

	return &g_SynapseClient;
}

bool CSynapseClientMaterials::RequestAPI( APIDescriptor_t* pAPI )
{
	if ( !strcmp( pAPI->major_name, "image" ) )
	{
		_QERPlugImageTable* pTable = static_cast< _QERPlugImageTable* >( pAPI->mpTable );
		if ( !strcmp( pAPI->minor_name, "lmt" ) )
		{
			pTable->m_pfnLoadImage = &LoadImage;
			return true;
		}
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}
bool CSynapseClientMaterials::OnActivate()
{
	if ( !g_FileSystemTable.m_nSize )
	{
		Syn_Printf( "ERROR: VFS_MAJOR table was not initialized before OnActivate in '%s' - incomplete synapse.config?\n", GetInfo() );
		return false;
	}
	return true;
}
#include "version.h"

const char* CSynapseClientMaterials::GetInfo()
{
	return "lifeengine materials formats module built " __DATE__ " " RADIANT_VERSION;
}
