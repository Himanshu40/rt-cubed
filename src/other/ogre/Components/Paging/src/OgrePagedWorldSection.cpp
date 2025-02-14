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
#include "OgrePagedWorldSection.h"
#include "OgrePageStrategy.h"
#include "OgreStreamSerialiser.h"
#include "OgreException.h"
#include "OgrePagedWorld.h"
#include "OgrePageManager.h"
#include "OgrePage.h"
#include "OgrePageRequestQueue.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

namespace Ogre
{
	//---------------------------------------------------------------------
	const uint32 PagedWorldSection::CHUNK_ID = StreamSerialiser::makeIdentifier("PWSC");
	const uint16 PagedWorldSection::CHUNK_VERSION = 1;
	//---------------------------------------------------------------------
	PagedWorldSection::PagedWorldSection(PagedWorld* parent)
		: mParent(parent), mStrategy(0), mPageProvider(0), mSceneMgr(0)
	{

	}
	//---------------------------------------------------------------------
	PagedWorldSection::PagedWorldSection(const String& name, PagedWorld* parent, 
		PageStrategy* strategy, SceneManager* sm)
		: mName(name), mParent(parent), mStrategy(0), mPageProvider(0), mSceneMgr(sm)
	{
		setStrategy(strategy);
	}
	//---------------------------------------------------------------------
	PagedWorldSection::~PagedWorldSection()
	{
		if (mStrategy)
		{
			mStrategy->destroyData(mStrategyData);
			mStrategyData = 0;
		}

		removeAllPages();
	}
	//---------------------------------------------------------------------
	PageManager* PagedWorldSection::getManager() const
	{
		return mParent->getManager();
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::setBoundingBox(const AxisAlignedBox& box)
	{
		mAABB = box;
	}
	//---------------------------------------------------------------------
	const AxisAlignedBox& PagedWorldSection::getBoundingBox() const
	{
		return mAABB;
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::setStrategy(PageStrategy* strat)
	{
		if (strat != mStrategy)
		{
			if (mStrategy)
			{
				mStrategy->destroyData(mStrategyData);
				mStrategy = 0;
				mStrategyData = 0;
			}

			mStrategy = strat;
			if (mStrategy)
				mStrategyData = mStrategy->createData();

			removeAllPages();
		}

	}
	//---------------------------------------------------------------------
	void PagedWorldSection::setStrategy(const String& stratName)
	{
		setStrategy(getManager()->getStrategy(stratName));
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::setSceneManager(SceneManager* sm)
	{
		if (sm != mSceneMgr)
		{
			mSceneMgr = sm;
			removeAllPages();
		}
		
		
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::setSceneManager(const String& smName)
	{
		setSceneManager(Root::getSingleton().getSceneManager(smName));
	}
	//---------------------------------------------------------------------
	bool PagedWorldSection::load(StreamSerialiser& ser)
	{
		if (!ser.readChunkBegin(CHUNK_ID, CHUNK_VERSION, "PagedWorldSection"))
			return false;

		// Name
		ser.read(&mName);
		// AABB
		ser.read(&mAABB);
		// Page Strategy Name
		String stratname;
		ser.read(&stratname);
		setStrategy(stratname);
		// Page Strategy Data
		bool strategyDataOk = mStrategyData->load(ser);
		if (!strategyDataOk)
			LogManager::getSingleton().stream() << "Error: PageStrategyData for section '"
			<< mName << "' was not loaded correctly, check file contents";

		ser.readChunkEnd(CHUNK_ID);

		return true;

	}
	//---------------------------------------------------------------------
	void PagedWorldSection::save(StreamSerialiser& ser)
	{
		ser.writeChunkBegin(CHUNK_ID, CHUNK_VERSION);

		// Name
		ser.write(&mName);
		// AABB
		ser.write(&mAABB);
		// Page Strategy Name
		ser.write(&mStrategy->getName());
		// Page Strategy Data
		mStrategyData->save(ser);

		// save all pages
		// TODO

		ser.writeChunkEnd(CHUNK_ID);

	}
	//---------------------------------------------------------------------
	PageID PagedWorldSection::getPageID(const Vector3& worldPos)
	{
		return mStrategy->getPageID(worldPos, this);
	}
	//---------------------------------------------------------------------
	Page* PagedWorldSection::loadOrCreatePage(const Vector3& worldPos)
	{
		PageID id = getPageID(worldPos);
		// this will create a Page instance no matter what, even if load fails
		// we force the load attempt to happen immediately (forceSynchronous)
		loadPage(id, true);
		return getPage(id);
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::loadPage(PageID pageID, bool sync)
	{
		PageMap::iterator i = mPages.find(pageID);
		if (i == mPages.end())
		{
			Page* page = OGRE_NEW Page(pageID);
			// attach page immediately, but notice that it's not loaded yet
			attachPage(page);
			getManager()->getQueue()->loadPage(page, this, sync);
		}
		else
			i->second->touch();
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::unloadPage(PageID pageID, bool sync)
	{
		PageMap::iterator i = mPages.find(pageID);
		if (i != mPages.end())
		{
			Page* page = i->second;
			mPages.erase(i);
			page->_notifyAttached(0);

			getManager()->getQueue()->unloadPage(page, this, sync);
			
		}
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::unloadPage(Page* p, bool sync)
	{
		unloadPage(p->getID(), sync);
	}
	//---------------------------------------------------------------------
	bool PagedWorldSection::_prepareProceduralPage(Page* page)
	{
		bool generated = false;
		if (mPageProvider)
			generated = mPageProvider->prepareProceduralPage(page, this);
		if (!generated)
			generated = mParent->_prepareProceduralPage(page, this);
		return generated;

	}
	//---------------------------------------------------------------------
	bool PagedWorldSection::_loadProceduralPage(Page* page)
	{
		bool generated = false;
		if (mPageProvider)
			generated = mPageProvider->loadProceduralPage(page, this);
		if (!generated)
			generated = mParent->_loadProceduralPage(page, this);
		return generated;

	}
	//---------------------------------------------------------------------
	bool PagedWorldSection::_unloadProceduralPage(Page* page)
	{
		bool generated = false;
		if (mPageProvider)
			generated = mPageProvider->unloadProceduralPage(page, this);
		if (!generated)
			generated = mParent->_unloadProceduralPage(page, this);
		return generated;

	}
	//---------------------------------------------------------------------
	bool PagedWorldSection::_unprepareProceduralPage(Page* page)
	{
		bool generated = false;
		if (mPageProvider)
			generated = mPageProvider->unprepareProceduralPage(page, this);
		if (!generated)
			generated = mParent->_unprepareProceduralPage(page, this);
		return generated;

	}
	//---------------------------------------------------------------------
	void PagedWorldSection::holdPage(PageID pageID)
	{
		PageMap::iterator i = mPages.find(pageID);
		if (i != mPages.end())
			i->second->touch();
	}
	//---------------------------------------------------------------------
	Page* PagedWorldSection::getPage(PageID pageID)
	{
		PageMap::iterator i = mPages.find(pageID);
		if (i != mPages.end())
			return i->second;
		else
			return 0;
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::attachPage(Page* page)
	{
		// try to insert
		std::pair<PageMap::iterator, bool> ret = mPages.insert(
			PageMap::value_type(page->getID(), page));

		if (!ret.second)
		{
			// page with this ID already in map
			if (ret.first->second != page)
			{
				// replacing a page, delete the old one
				getManager()->getQueue()->cancelOperationsForPage(ret.first->second);
				OGRE_DELETE ret.first->second;
				ret.first->second = page;
			}
		}
		page->_notifyAttached(this);
			
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::detachPage(Page* page)
	{
		PageMap::iterator i = mPages.find(page->getID());
		if (i != mPages.end() && i->second == page)
		{
			mPages.erase(i);
			page->_notifyAttached(0);
		}

	}
	//---------------------------------------------------------------------
	void PagedWorldSection::removeAllPages()
	{
		for (PageMap::iterator i= mPages.begin(); i != mPages.end(); ++i)
		{
			getManager()->getQueue()->cancelOperationsForPage(i->second);
			OGRE_DELETE i->second;
		}
		mPages.clear();

	}
	//---------------------------------------------------------------------
	void PagedWorldSection::frameStart(Real timeSinceLastFrame)
	{
		mStrategy->frameStart(timeSinceLastFrame, this);

		for (PageMap::iterator i = mPages.begin(); i != mPages.end(); ++i)
			i->second->frameStart(timeSinceLastFrame);
	}
	//---------------------------------------------------------------------
	void PagedWorldSection::frameEnd(Real timeElapsed)
	{
		mStrategy->frameEnd(timeElapsed, this);

		for (PageMap::iterator i = mPages.begin(); i != mPages.end(); )
		{
			// if this page wasn't used, unload
			Page* p = i->second;
			// pre-increment since unloading will remove it
			++i;
			if (!p->isHeld())
				unloadPage(p);
			else
				p->frameEnd(timeElapsed);
		}

	}
	//---------------------------------------------------------------------
	void PagedWorldSection::notifyCamera(Camera* cam)
	{
		mStrategy->notifyCamera(cam, this);

		for (PageMap::iterator i = mPages.begin(); i != mPages.end(); ++i)
			i->second->notifyCamera(cam);
	}
	//---------------------------------------------------------------------
	StreamSerialiser* PagedWorldSection::_readPageStream(PageID pageID)
	{
		StreamSerialiser* ser = 0;
		if (mPageProvider)
			ser = mPageProvider->readPageStream(pageID, this);
		if (!ser)
			ser = mParent->_readPageStream(pageID, this);
		return ser;

	}
	//---------------------------------------------------------------------
	StreamSerialiser* PagedWorldSection::_writePageStream(PageID pageID)
	{
		StreamSerialiser* ser = 0;
		if (mPageProvider)
			ser = mPageProvider->writePageStream(pageID, this);
		if (!ser)
			ser = mParent->_writePageStream(pageID, this);
		return ser;

	}
	//---------------------------------------------------------------------
	std::ostream& operator <<( std::ostream& o, const PagedWorldSection& p )
	{
		o << "PagedWorldSection(" << p.getName() << ", world:" << p.getWorld()->getName() << ")";
		return o;
	}





}

