//////////////////////////////////////////////////////////////////////////
//
//				  *** lifeRadiant ***
//				Copyright (C) 2018-2020
//
// Репозиторий движка:  https://github.com/zombihello/lifeRadiant
// Авторы:				Егор Погуляка (zombiHello)
//
//////////////////////////////////////////////////////////////////////////

#ifndef PLUGIN_H
#define PLUGIN_H

#include "synapse.h"
#include "qerplugin.h"
#include "ifilesystem.h"
#include "iimage.h"

extern _QERFuncTable_1			g_FuncTable;
extern _QERFileSystemTable		g_FileSystemTable;

#define Error					g_FuncTable.m_pfnError
#define vfsLoadFile				g_FileSystemTable.m_pfnLoadFile
#define vfsFreeFile				g_FileSystemTable.m_pfnFreeFile

#define LittleLong( a )			GINT32_FROM_LE( a )
#define LittleShort( a )		GINT16_FROM_LE( a )

class CSynapseClientMaterials : public CSynapseClient
{
public:
	// CSynapseClient API
	bool			RequestAPI( APIDescriptor_t* pAPI );
	const char*		GetInfo();
	bool			OnActivate();
	const char*		GetName() { return "image"; }

	CSynapseClientMaterials() {}
	virtual ~CSynapseClientMaterials() {}
};

#endif // !PLUGIN_H
