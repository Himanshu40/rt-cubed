/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef __Ogre_Terrain_Prereq_H__
#define __Ogre_Terrain_Prereq_H__

#include "OgrePrerequisites.h"

namespace Ogre
{
	// forward decls
	class Terrain;
	class TerrainPageContent;
	class TerrainPageContentFactory;
	class TerrainQuadTreeNode;


	typedef GeneralAllocatedObject TerrainAlloc;


}

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32) && !defined(OGRE_STATIC_LIB)
#	ifdef OGRE_TERRAIN_EXPORTS
#		define _OgreTerrainExport __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define _OgreTerrainExport
#       else
#    		define _OgreTerrainExport __declspec(dllimport)
#       endif
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#    define _OgreTerrainExport  __attribute__ ((visibility("default")))
#else
#	define _OgreTerrainExport
#endif	// OGRE_WIN32

#endif 
