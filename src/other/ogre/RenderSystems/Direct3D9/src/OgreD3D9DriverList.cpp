/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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
#include "OgreD3D9DriverList.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreD3D9RenderSystem.h"

namespace Ogre 
{
	D3D9DriverList::D3D9DriverList()
	{
		enumerate();
	}

	D3D9DriverList::~D3D9DriverList(void)
	{
		mDriverList.clear();
	}

	BOOL D3D9DriverList::enumerate()
	{
		IDirect3D9* lpD3D9 = D3D9RenderSystem::getDirect3D9();

		LogManager::getSingleton().logMessage( "D3D9: Driver Detection Starts" );
		for( UINT iAdapter=0; iAdapter < lpD3D9->GetAdapterCount(); ++iAdapter )
		{
			D3DADAPTER_IDENTIFIER9 adapterIdentifier;
			D3DDISPLAYMODE d3ddm;
			D3DCAPS9 d3dcaps9;
			
			lpD3D9->GetAdapterIdentifier( iAdapter, 0, &adapterIdentifier );
			lpD3D9->GetAdapterDisplayMode( iAdapter, &d3ddm );
			lpD3D9->GetDeviceCaps( iAdapter, D3DDEVTYPE_HAL, &d3dcaps9 );

			mDriverList.push_back( D3D9Driver( iAdapter, d3dcaps9, adapterIdentifier, d3ddm ) );
		}

		LogManager::getSingleton().logMessage( "D3D9: Driver Detection Ends" );

		return TRUE;
	}

	size_t D3D9DriverList::count() const 
	{
		return mDriverList.size();
	}

	D3D9Driver* D3D9DriverList::item( size_t index )
	{
		return &mDriverList.at( index );
	}

	D3D9Driver* D3D9DriverList::item( const String &name )
	{
		vector<D3D9Driver>::type::iterator it = mDriverList.begin();
		if (it == mDriverList.end())
			return NULL;

		for (;it != mDriverList.end(); ++it)
		{
			if (it->DriverDescription() == name)
				return &(*it);
		}

		return NULL;
	}
}
