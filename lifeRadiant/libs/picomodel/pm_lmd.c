
/* marker */
#define PM_LMD_C

/* dependencies */
#include "picointernal.h"

#define LMD_MAGIC "LMD"
#define LMD_VERSION 2

/* md3 model file md3 structure */
typedef struct lmd_s
{
	char				magic[ 3 ];            /* MD3_MAGIC */
	unsigned short		version;
} lmd_t;

typedef struct lmdMaterials_s
{
	unsigned int		size;
	char*				route;
} lmdMaterials_t;

typedef struct lmdVertex_s
{
	float		position[ 3 ];
	float		normal[ 3 ];
	float		texcoords[ 2 ];
	float		tangent[ 3 ];
	float		bitangent[ 3 ];
} lmdVertex_t;

typedef struct lmdSurface_s
{
	unsigned int		materialID;
	unsigned int		startIndex;
	unsigned int		countIndeces;
} lmdSurface_t;

/*
   _lmd_canload()
   validates a quake3 arena md3 model file. btw, i use the
   preceding underscore cause it's a static func referenced
   by one structure only.
 */

static int _lmd_canload( PM_PARAMS_CANLOAD )
{
	lmd_t		lmd;

	/* sanity check */
	if ( ( size_t ) bufSize < ( sizeof( lmd_t ) ) )
	{
		return PICO_PMV_ERROR_SIZE;
	}

	memcpy( &lmd.magic, ( char* ) buffer, 3 );
	memcpy( &lmd.version, ( char* ) buffer+3, sizeof( unsigned short ) );
	
	/* check lmd magic */
	if ( strncmp( lmd.magic, LMD_MAGIC, 3 ) != 0 )
	{
		return PICO_PMV_ERROR_IDENT;
	}

	/* check lmd version */
	if ( lmd.version != LMD_VERSION )
	{
		return PICO_PMV_ERROR_VERSION;
	}

	/* file seems to be a valid lmd */
	return PICO_PMV_OK;
}

/*
   _lmd_load()
   loads a quake3 arena md3 model file.
 */

static picoModel_t* _lmd_load( PM_PARAMS_LOAD )
{
	lmd_t			lmd;
	unsigned int	offset = 0;

	memcpy( &lmd.magic, ( char* ) buffer, 3 );
	offset += 3;

	memcpy( &lmd.version, ( char* ) buffer + offset, sizeof( unsigned short ) );
	offset += sizeof( unsigned short );

	/* -------------------------------------------------
	   lmd loading
	   ------------------------------------------------- */


	/* check ident and version */
	if ( strncmp( lmd.magic, LMD_MAGIC, 3 ) != 0 ||
		 lmd.version != LMD_VERSION )
	{
		/* not an lmd file (todo: set error) */
		return NULL;
	}

	unsigned int			countMaterials = 0;
	lmdMaterials_t*			materials = NULL;

	memcpy( &countMaterials, ( char* ) buffer + offset, sizeof( unsigned int ) );
	offset += sizeof( unsigned int );

	/* if exist materials in model thene we loading */
	if ( countMaterials > 0 )
	{
		materials = ( lmdMaterials_t* ) malloc( countMaterials * sizeof( lmdMaterials_t ) );
		
		for ( unsigned int index = 0; index < countMaterials; ++index )
		{
			unsigned int		sizeRoute = 0;		
			memcpy( &sizeRoute, ( char* ) buffer + offset, sizeof( unsigned int ) );
			offset += sizeof( unsigned int );

			if ( sizeRoute <= 0 ) continue;

			materials[ index ].size = sizeRoute + 1;
			materials[ index ].route = ( char* ) malloc( materials[ index ].size * sizeof( char ) );

			memcpy( materials[ index ].route, ( char* ) buffer + offset, sizeRoute * sizeof( char ) );
			materials[ index ].route[ sizeRoute ] = '\0';
			offset += sizeRoute * sizeof( char );
		}
	}

	unsigned int		countVerteces = 0;
	lmdVertex_t*		verteces = NULL;

	memcpy( &countVerteces, ( char* ) buffer + offset, sizeof( unsigned int ) );
	offset += sizeof( unsigned int );

	/* if exist verteces in model then we loading */
	if ( countVerteces > 0 )
	{
		verteces = ( lmdVertex_t* ) malloc( countVerteces * sizeof( lmdVertex_t ) );
		memcpy( verteces, ( char* ) buffer + offset, countVerteces * sizeof( lmdVertex_t ) );
		offset += countVerteces * sizeof( lmdVertex_t );
	}

	unsigned int		countIndeces = 0;
	unsigned int*		indeces = NULL;

	memcpy( &countIndeces, ( char* ) buffer + offset, sizeof( unsigned int ) );
	offset += sizeof( unsigned int );

	if ( countIndeces > 0 )
	{
		indeces = ( unsigned int* ) malloc( countIndeces * sizeof( unsigned int ) );
		memcpy( indeces, ( char* ) buffer + offset, countIndeces * sizeof( unsigned int ) );
		offset += countIndeces * sizeof( unsigned int );
	}

	unsigned int		countSurfaces = 0;
	lmdSurface_t*		surfaces = NULL;

	memcpy( &countSurfaces, ( char* ) buffer + offset, sizeof( unsigned int ) );
	offset += sizeof( unsigned int );

	if ( countSurfaces > 0 )
	{
		surfaces = ( lmdSurface_t* ) malloc( countSurfaces * sizeof( lmdSurface_t ) );
		memcpy( surfaces, ( char* ) buffer + offset, countSurfaces * sizeof( lmdSurface_t ) );
		offset += countSurfaces * sizeof( lmdSurface_t );
	}
	
	picoModel_t*		picoModel = PicoNewModel();
	if ( !picoModel )	return NULL;

	/* do model setup */
	PicoSetModelFrameNum( picoModel, frameNum );
	PicoSetModelNumFrames( picoModel, 0 ); /* sea */
	PicoSetModelName( picoModel, fileName );
	PicoSetModelFileName( picoModel, fileName );
	
	unsigned int curIndex = 0;
	unsigned int curVertex = 0;

	for ( unsigned int index = 0; index < countSurfaces; ++index )
	{
		picoSurface_t*		picoSurface = PicoNewSurface( picoModel );
		if ( !picoSurface )	return NULL;

		PicoSetSurfaceType( picoSurface, PICO_TRIANGLES );
		PicoSetSurfaceName( picoSurface, "this_is_fucked_surface" );

		picoShader_t*		picoShader = PicoNewShader( picoModel );
		if ( !picoShader )	return NULL;
		
		PicoSetShaderName( picoShader, materials[ surfaces[ index ].materialID ].route );
		PicoSetShaderMapName( picoShader, materials[ surfaces[ index ].materialID ].route );
		PicoSetSurfaceShader( picoSurface, picoShader );

		for ( unsigned int i = surfaces[ index ].startIndex, j = surfaces[ index ].startIndex + surfaces[ index ].countIndeces; i < j; ++i )
		{
			lmdVertex_t			vertex = verteces[ indeces[ i ] ];
			
			picoVec3_t			xyz;
			xyz[ 0 ] = vertex.position[ 0 ];
			xyz[ 1 ] = vertex.position[ 1 ];
			xyz[ 2 ] = vertex.position[ 2 ];
			PicoSetSurfaceXYZ( picoSurface, curVertex, xyz );

			picoVec3_t			normal;
			normal[ 0 ] = vertex.normal[ 0 ];
			normal[ 1 ] = vertex.normal[ 1 ];
			normal[ 2 ] = vertex.normal[ 2 ];
			PicoSetSurfaceNormal( picoSurface, curVertex, normal );
			
			picoVec2_t			st;
			st[ 0 ] = vertex.texcoords[ 0 ];
			st[ 1 ] = vertex.texcoords[ 1 ];
			PicoSetSurfaceST( picoSurface, 0, curVertex, st );

			PicoSetSurfaceIndex( picoSurface, curIndex, curVertex );
			curIndex++;
			curVertex++;
		}
	}

	for ( int i = 0; i < countMaterials; ++i )
		free( materials[ i ].route );

	if ( materials ) free( materials );
	if ( verteces ) free( verteces );
	if ( indeces ) free( indeces );
	if ( surfaces ) free( surfaces );

	return picoModel;
}

/* pico file format module definition */
const picoModule_t picoModuleLMD =
{
	"1.0",                      /* module version string */
	"lifeEngine models",         /* module display name */
	"Egor Pogulyaka",            /* author's name */
	"2020 Egor Pogulyaka",        /* module copyright */
	{
		"lmd", NULL, NULL, NULL /* default extensions to use */
	},
	_lmd_canload,               /* validation routine */
	_lmd_load,                  /* load routine */
	NULL,                       /* save validation routine */
	NULL                        /* save routine */
};
