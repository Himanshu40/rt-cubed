/*                      C O N S T D A T A B A S E . H
 * BRL-CAD
 *
 * Copyright (c) 2008-2011 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file ConstDatabase.h
 *
 *  BRL-CAD core C++ interface:
 *      declares a handle on a read-only database
 *
 *  Origin -
 *      TNO (Netherlands)
 *      IABG mbH (Germany)
 */

#ifndef BRLCAD_CONSTDATABASE_INCLUDED
#define BRLCAD_CONSTDATABASE_INCLUDED

#include <brlcad/cicommon.h>
#include <brlcad/Object.h>
#include <brlcad/VectorList.h>


struct rt_i;
struct resource;
struct directory;


namespace BRLCAD {
    class NonManifoldGeometry;


    class BRLCAD_COREINTERFACE_EXPORT ConstDatabase {
    public:
        ConstDatabase(void);
        virtual ~ConstDatabase(void);

        /// associates the handle with a BRL-CAD database file (*.g)
        /** If the handle was already associated with a database file this association will be discarded.
            The file will be opened for reading only. */
        virtual bool         Load(const char* fileName);

        /// @name Parsing the database
        //@{
        /// title string read from the database
        const char*          Title(void) const;


        class BRLCAD_COREINTERFACE_EXPORT TopObjectIterator {
        public:
            TopObjectIterator(const TopObjectIterator& original) : m_hashTablePosition(original.m_hashTablePosition),
                                                                           m_pDir(original.m_pDir),
                                                                           m_rtip(original.m_rtip) {}

            ~TopObjectIterator(void) {}

            const TopObjectIterator& operator=(const TopObjectIterator& original) {
                m_hashTablePosition = original.m_hashTablePosition;
                m_pDir              = original.m_pDir;
                m_rtip              = original.m_rtip;

                return *this;
            }

            const TopObjectIterator& operator++(void);
            bool                     Good(void) const;
            const char*              Name(void) const;

        private:
            size_t           m_hashTablePosition;
            const directory* m_pDir;
            const rt_i*      m_rtip;

            TopObjectIterator(size_t           hashTablePosition,
                              const directory* pDir,
                              const rt_i*      rtip);

            friend class ConstDatabase;

            TopObjectIterator(void);
        };


        /// returns the first of the top level objects via an iterator object
        /** To get a list of all top level objects you have to use the iterator returned by this function. */
        TopObjectIterator    FirstTopObject(void) const;
        //@}

        /// @name Accessing objects
        //@{
        class ObjectCallback {
        public:
            virtual ~ObjectCallback(void) {}

            /// the user has to implement this object method to evaluate the object
            virtual void operator()(const Object& object) = 0;

        protected:
            ObjectCallback(void) {}
            ObjectCallback(const ObjectCallback&) {}
            const ObjectCallback& operator=(const ObjectCallback&) {return *this;}
        };

        /// selects a single object and hand it over to an ObjectCallback (read only)
        void                 Get(const char*     objectName,
                                 ObjectCallback& callback) const;

        /// overloaded member function, provided for convenience: selects a single object and and returns a copy of it
        /** Do not forget to BRLCAD::Object::Destroy() the copy when you are finished with it! */
        Object*              Get(const char* objectName) const;
        //@}

        /// @name Generating alternative representations
        /// facetizes a single object's tree and returns it as a non-manifold geometry
        /** Do not forget to BRLCAD::Object::Destroy() the non-manifold geometry when you are finished with it! */
        NonManifoldGeometry* Facetize(const char* objectName) const;

        /// plot a single object's tree and write the resulting wireframe to a vector list
        void                 Plot(const char* objectName,
                                  VectorList& vectorList) const;
        //@}

        /// @name Active set functions
        //@{
        /// add the database object \a objectName to the active set
        /** The function accepts a space separated list of object names too,
            but it is not sure that this behaviour will be kept in future versions. */
        void                 Select(const char* objectName);
        /// clear the active set
        void                 UnSelectAll(void);

        bool                 SelectionIsEmpty(void) const;
        Vector3D             BoundingBoxMinima(void) const;
        Vector3D             BoundingBoxMaxima(void) const;

        /// ray trace
        class Hit {
        public:
            virtual ~Hit(void) {}

            virtual const char* Name(void) const                = 0;
            virtual double      DistanceIn(void) const          = 0;
            virtual double      DistanceOut(void) const         = 0;
            virtual Vector3D    PointIn(void) const             = 0;
            virtual Vector3D    PointOut(void) const            = 0;
            virtual Vector3D    SurfaceNormalIn(void) const     = 0;
            virtual Vector3D    SurfaceNormalOut(void) const    = 0;
            virtual Curvature3D SurfaceCurvatureIn(void) const  = 0;
            virtual Curvature3D SurfaceCurvatureOut(void) const = 0;
            virtual Mapping2D   Surface2DMappingIn(void) const  = 0;
            virtual Mapping2D   Surface2DMappingOut(void) const = 0;

            /// some extracted data (originally from the region)
            virtual bool        HasColor(void) const            = 0;
            virtual double      Red(void) const                 = 0;
            virtual double      Green(void) const               = 0;
            virtual double      Blue(void) const                = 0;

        protected:
            Hit(void) {}
            Hit(const Hit&) {}
            const Hit& operator=(const Hit&) {return *this;}
        };


        class HitCallback {
        public:
            virtual ~HitCallback(void) {}

            /** return true: go on; false: stop
                The return value gives the calling function the possibility to optimize.
                However be aware the return value may be ignored. */
            /** Do not throw en exception here.
                This method will be called from deep inside the brlcad libraries.
                The status of the program after an exception will be undetermined. */
            virtual bool operator()(const Hit& hit) = 0;

        protected:
            HitCallback(void) {}
            HitCallback(const HitCallback&) {}
            const HitCallback& operator=(const HitCallback&) {return *this;}
        };


        void                 ShootRay(const Ray3D& ray,
                                      HitCallback& callback) const;

        static const int StopAfterFirstHit = 1;
        static const int WithOverlaps      = 2;

        void                 ShootRay(const Ray3D& ray,
                                      HitCallback& callback,
                                      int          flags) const;
        //@}

    protected:
        rt_i*     m_rtip;
        resource* m_resp;

    private:
        ConstDatabase(const ConstDatabase&);                  // not implemented
        const ConstDatabase& operator=(const ConstDatabase&); // not implemented
    };
}


#endif // BRLCAD_CONSTDATABASE_INCLUDED
