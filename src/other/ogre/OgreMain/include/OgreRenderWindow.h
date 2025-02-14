/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as 
published by the Free Software Foundation; either version 2.1 of the 
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
License for more details.

You should have received a copy of the GNU Lesser General Public License 
along with this library; if not, write to the Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/
#ifndef __RenderWindow_H__
#define __RenderWindow_H__

#include "OgrePrerequisites.h"

#include "OgreRenderTarget.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup RenderSystem
	*  @{
	*/
	/** Manages the target rendering window.
        @remarks
            This class handles a window into which the contents
            of a scene are rendered. There is a many-to-1 relationship
            between instances of this class an instance of RenderSystem
            which controls the rendering of the scene. There may be
            more than one window in the case of level editor tools etc.
            This class is abstract since there may be
            different implementations for different windowing systems.
        @remarks
            Instances are created and communicated with by the render system
            although client programs can get a reference to it from
            the render system if required for resizing or moving.
            Note that you can have multiple viewpoints
            in the window for effects like rear-view mirrors and
            picture-in-picture views (see Viewport and Camera).
        @author
            Steven Streeting
        @version
            1.0
    */
    class _OgreExport RenderWindow : public RenderTarget
    {

    public:
        /** Default constructor.
        */
        RenderWindow();

        /** Creates & displays the new window.
            @param
                width The width of the window in pixels.
            @param
                height The height of the window in pixels.
            @param
                colourDepth The colour depth in bits. Ignored if
                fullScreen is false since the desktop depth is used.
            @param
                fullScreen If true, the window fills the screen,
                with no title bar or border.
            @param
                left The x-position of the window. Ignored if
                fullScreen = true.
            @param
                top The y-position of the window. Ignored if
                fullScreen = true.
            @param
                depthBuffer Specify true to include a depth-buffer.
            @param
                miscParam A variable number of pointers to platform-specific arguments. The
                actual requirements must be defined by the implementing subclasses.
        */
		virtual void create(const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams) = 0;

		/** Alter fullscreen mode options. 
		@note Nothing will happen unless the settings here are different from the
			current settings.
		@param fullScreen Whether to use fullscreen mode or not. 
		@param width The new width to use
		@param height The new height to use
		*/
		virtual void setFullscreen(bool fullScreen, unsigned int width, unsigned int height) {}
        
        /** Destroys the window.
        */
        virtual void destroy(void) = 0;

        /** Alter the size of the window.
        */
        virtual void resize(unsigned int width, unsigned int height) = 0;

	/** Notify that the window has been resized
	@remarks
		You don't need to call this unless you created the window externally.
	*/
	virtual void windowMovedOrResized() {}

        /** Reposition the window.
        */
        virtual void reposition(int left, int top) = 0;

        /** Indicates whether the window is visible (not minimized or obscured)
        */
        virtual bool isVisible(void) const { return true; }

        /** Set the visibility state
        */
        virtual void setVisible(bool visible) {}

        /** Overridden from RenderTarget, flags invisible windows as inactive
        */
        virtual bool isActive(void) const { return mActive && isVisible(); }

        /** Indicates whether the window has been closed by the user.
        */
        virtual bool isClosed(void) const = 0;
        
        /** Indicates whether the window is the primary window. The
        	primary window is special in that it is destroyed when 
        	ogre is shut down, and cannot be destroyed directly.
        	This is the case because it holds the context for vertex,
        	index buffers and textures.
        */
        virtual bool isPrimary(void) const;

        /** Returns true if window is running in fullscreen mode.
        */
        virtual bool isFullScreen(void) const;

        /** Overloaded version of getMetrics from RenderTarget, including extra details
            specific to windowing systems.
        */
        virtual void getMetrics(unsigned int& width, unsigned int& height, unsigned int& colourDepth, 
			int& left, int& top);

		/// Override since windows don't usually have alpha
		PixelFormat suggestPixelFormat() const { return PF_BYTE_RGB; }

        /** Returns true if the window will automatically de-activate itself when it loses focus.
        */
        bool isDeactivatedOnFocusChange() const;

        /** Indicates whether the window will automatically deactivate itself when it loses focus.
          * \param deactivate a value of 'true' will cause the window to deactivate itself when it loses focus.  'false' will allow it to continue to render even when window focus is lost.
          * \note 'true' is the default behavior.
          */
        void setDeactivateOnFocusChange(bool deactivate);

    protected:
        bool mIsFullScreen;
        bool mIsPrimary;
        bool mAutoDeactivatedOnFocusChange;
        int mLeft;
        int mTop;
        
        /** Indicates that this is the primary window. Only to be called by
            Ogre::Root
        */
        void _setPrimary() { mIsPrimary = true; }
        
        friend class Root;
    };
	/** @} */
	/** @} */

} // Namespace
#endif
