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
#ifndef __D3D10RENDERWINDOW_H__
#define __D3D10RENDERWINDOW_H__

#include "OgreD3D10Prerequisites.h"
#include "OgreRenderWindow.h"


namespace Ogre 
{
	class D3D10RenderWindow : public RenderWindow
	{
	public:
		/** Constructor.
		@param instance The application instance
		@param driver The root driver
		@param deviceIfSwapChain The existing D3D device to create an additional swap chain from, if this is not
		the first window.
		*/
		D3D10RenderWindow(HINSTANCE instance, D3D10Device & device);
		~D3D10RenderWindow();
		void create(const String& name, unsigned int width, unsigned int height,
			bool fullScreen, const NameValuePairList *miscParams);
		void setFullscreen(bool fullScreen, unsigned int width, unsigned int height);
		void destroy(void);
		bool isVisible() const;
		bool isClosed() const { return mClosed; }
		void reposition(int left, int top);
		void resize(unsigned int width, unsigned int height);
		void swapBuffers( bool waitForVSync = true );
		HWND getWindowHandle() const;

		void getCustomAttribute( const String& name, void* pData );
		/** Overridden - see RenderTarget.
		*/
		virtual void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);
		bool requiresTextureFlipping() const;

		// Method for dealing with resize / move & 3d library
		void windowMovedOrResized();

		/// Get the presentation parameters used with this window
		DXGI_SWAP_CHAIN_DESC* getPresentationParameters(void);

		/// @copydoc RenderTarget::update
		void update(bool swap);

		/** Create (or recreate) the D3D device or SwapChain for this window.
		*/
		void createD3DResources();

		/** Destroy the D3D device or SwapChain for this window.
		*/
		void destroyD3DResources();

		/// Accessor for render surface
		//	IDXGISurface * getRenderSurface() { return mpRenderSurface; }

		/// Are we in the middle of switching between fullscreen and windowed
		bool _getSwitchingFullscreen() const;
		/// Indicate that fullscreen / windowed switching has finished
		void _finishSwitchingFullscreen();


	protected:
		HINSTANCE mInstance;			// Process instance
		D3D10Device & mDevice;			// D3D10 driver
		HWND	mHWnd;					// Win32 Window handle
		bool	mIsExternal;			// window not created by Ogre
		bool	mSizing;
		bool	mClosed;
		bool	mIsSwapChain;			// Is this a secondary window?
		bool	mSwitchingFullscreen;	// Are we switching from fullscreen to windowed or vice versa

		// -------------------------------------------------------
		// DirectX-specific
		// -------------------------------------------------------

		// Pointer to swap chain, only valid if mIsSwapChain
		IDXGISwapChain * mpSwapChain;
		DXGI_SWAP_CHAIN_DESC md3dpp;
		//	IDXGISurface * mpRenderSurface;
		//	IDXGISurface * mpRenderZBuffer;
		DXGI_SAMPLE_DESC mFSAAType;
		//DWORD mFSAAQuality;
		UINT mDisplayFrequency;
		bool mVSync;
		unsigned int mVSyncInterval;
		bool mUseNVPerfHUD;
		ID3D10RenderTargetView*		mRenderTargetView;
		ID3D10DepthStencilView*		mDepthStencilView;
		ID3D10Texture2D*			mpBackBuffer;

		// just check if the multisampling requested is supported by the device
		bool _checkMultiSampleQuality(UINT SampleCount, UINT *outQuality, DXGI_FORMAT format);

	};
}
#endif
