/*                      S K E T C H . C P P
 * BRL-CAD
 *
 * Copyright (c) 2014 United States Government as represented by
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
/** @file Sketch.cpp
 *
 *  BRL-CAD core C++ interface:
 *      sketch (ID_SKETCH) database object implementation
 */

#include <cstring>
#include <cassert>

#include "raytrace.h"
#include "rt/geom.h"
#include "bu/parallel.h"

#include <brlcad/Sketch.h>


using namespace BRLCAD;


static size_t AddToVerts
(
    const point2d_t&    point,
    rt_sketch_internal& sketch
) {
    size_t ret = 0; // index of the added vertex

    // vertex already there?
    for (; ret < sketch.vert_count; ++ret) {
        if (V2NEAR_EQUAL(point, sketch.verts[ret], VUNITIZE_TOL))
            break;
    }

    if (ret == sketch.vert_count) {
        // add a new vertex
        ++sketch.vert_count;
        sketch.verts = static_cast<point2d_t*>(bu_realloc(sketch.verts,
                                                          sketch.vert_count * sizeof(point2d_t),
                                                          "sketch interface AddToVerts()"));

        V2MOVE(sketch.verts[ret], point);
    }

    return ret;
}


static void RemoveFromVerts
(
    size_t              index,
    rt_sketch_internal& sketch
) {
    assert(index < sketch.vert_count);

    if (index < sketch.vert_count) {
        // is the vertex used elsewhere?
        size_t vertexUsage = 0;

        for (int i = 0; i < sketch.curve.count; ++i) {
            const uint32_t *magic = static_cast<uint32_t*>(sketch.curve.segment[i]);

            switch (*magic) {
                case CURVE_LSEG_MAGIC: {
                        line_seg* line = static_cast<line_seg*>(sketch.curve.segment[i]);

                        if (line->start == index)
                            ++vertexUsage;

                        if (line->end == index)
                            ++vertexUsage;
                    }
                    break;

                case CURVE_CARC_MAGIC: {
                        carc_seg* carc = static_cast<carc_seg*>(sketch.curve.segment[i]);

                        if (carc->start == index)
                            ++vertexUsage;

                        if (carc->end == index)
                            ++vertexUsage;
                    }
                    break;

                case CURVE_NURB_MAGIC: {
                        nurb_seg* nurb = static_cast<nurb_seg*>(sketch.curve.segment[i]);

                        for (size_t j = 0; j < nurb->c_size; ++j) {
                            if (nurb->ctl_points[j] == index)
                                ++vertexUsage;
                        }
                    }
                    break;

                case CURVE_BEZIER_MAGIC: {
                        bezier_seg* bezier = static_cast<bezier_seg*>(sketch.curve.segment[i]);

                        for (size_t j = 0; j <= bezier->degree; ++j) {
                            if (bezier->ctl_points[j] == index)
                                ++vertexUsage;
                        }
                    }

            }
        }

        if (vertexUsage <= 1) {
            // really remove it
            memmove(sketch.verts + index, sketch.verts + index + 1, (sketch.vert_count - index - 1) * sizeof(point2d_t));

            --sketch.vert_count;
            sketch.verts = static_cast<point2d_t*>(bu_realloc(sketch.verts,
                                                              sketch.vert_count * sizeof(point2d_t),
                                                              "sketch interface RemoveFromVerts()"));
        }
    }
}


static size_t SwapVertex
(
    size_t              oldIndex,
    const point2d_t&    newPoint,
    rt_sketch_internal& sketch
) {
    size_t ret = -1; // index of the new vertex

    if (V2NEAR_EQUAL(newPoint, sketch.verts[oldIndex], VUNITIZE_TOL))
        ret = oldIndex;
    else {
        RemoveFromVerts(oldIndex, sketch);

        ret = AddToVerts(newPoint, sketch);
    }

    return ret;
}


static void AppendSegment
(
    void*               segment,
    rt_sketch_internal* sketch
) throw(bad_alloc) {
    if (sketch->curve.count == 0) {
        sketch->curve.reverse = static_cast<int*>(bu_malloc(sizeof(int), "Sketch::AppendLine: reverse"));
        sketch->curve.segment = static_cast<void**>(bu_malloc(sizeof(void*), "Sketch::AppendLine: segment"));
    }
    else {
        sketch->curve.reverse = static_cast<int*>(bu_realloc(sketch->curve.reverse,
                                                                (sketch->curve.count + 1) * sizeof(int),
                                                                "Sketch::AppendLine: reverse"));
        sketch->curve.segment = static_cast<void**>(bu_realloc(sketch->curve.segment,
                                                                    (sketch->curve.count + 1) * sizeof(void*),
                                                                    "Sketch::AppendLine: segment"));
    }

    sketch->curve.reverse[sketch->curve.count] = 0;
    sketch->curve.segment[sketch->curve.count] = segment;
    ++(sketch->curve.count);
};


static void InsertSegment
(
    void*               segment,
    size_t              index,
    rt_sketch_internal* sketch
) throw(bad_alloc) {
    assert(sketch->curve.count > 0);

    sketch->curve.reverse = static_cast<int*>(bu_realloc(sketch->curve.reverse,
                                                            (sketch->curve.count + 1) * sizeof(int),
                                                            "Sketch::AppendLine: reverse"));
    sketch->curve.segment = static_cast<void**>(bu_realloc(sketch->curve.segment,
                                                                (sketch->curve.count + 1) * sizeof(void*),
                                                                "Sketch::AppendLine: segment"));

    memmove(sketch->curve.segment + index + 1, sketch->curve.segment + index, (sketch->curve.count - index) * sizeof(int));
    memmove(sketch->curve.segment + index + 1, sketch->curve.segment + index, (sketch->curve.count - index) * sizeof(void*));

    sketch->curve.reverse[index] = 0;
    sketch->curve.segment[index] = segment;
    ++(sketch->curve.count);
};


Sketch::Sketch(void) throw(bad_alloc) : Object() {
    if (!BU_SETJUMP) {
        BU_GET(m_internalp, rt_sketch_internal);
        m_internalp->magic = RT_SKETCH_INTERNAL_MAGIC;
        m_internalp->curve.count = 0;
        m_internalp->vert_count = 0;
    }
    else {
        BU_UNSETJUMP;
        throw bad_alloc("BRLCAD::Sphere::Sphere");
    }

    BU_UNSETJUMP;
};


Sketch::Sketch
(
    const Sketch& original
) throw(bad_alloc) {
    if (!BU_SETJUMP)
        m_internalp = rt_copy_sketch(original.Internal());
    else {
        BU_UNSETJUMP;
        throw bad_alloc("BRLCAD::Sketch::Sketch");
    }

    BU_UNSETJUMP;

};


Sketch::~Sketch(void) throw() {
    if (m_internalp != 0) {
        if (m_internalp->verts != 0)
            bu_free(m_internalp->verts, "BRLCAD::Sketch::~Sketch::m_internalp->verts");

        rt_curve_free(&m_internalp->curve);

        BU_PUT(m_internalp, rt_sketch_internal);
    }
};


const Sketch& Sketch::operator=(
    const Sketch& original
) throw(bad_alloc) {
    if (&original != this) {
        Copy(original);

        if (!BU_SETJUMP) {
            rt_sketch_internal*       thisInternal     = Internal();
            const rt_sketch_internal* originalInternal = original.Internal();

            memcpy(thisInternal, originalInternal, sizeof(rt_sketch_internal));

            if (thisInternal->vert_count > 0) {
                thisInternal->verts = static_cast<point2d_t*>(bu_calloc(thisInternal->vert_count, sizeof(point2d_t), "Sketch::operator=::m_internalp->verts"));

                for (size_t i = 0; i < thisInternal->vert_count; ++i)
                    V2MOVE(thisInternal->verts[i], originalInternal->verts[i]);
            }

            rt_copy_curve(&thisInternal->curve, &originalInternal->curve);
        }
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::operator=");
        }

        BU_UNSETJUMP;
    }

    return *this;
};


//
// Segment class
//

void Sketch::Segment::Destroy(void) throw() {
    delete this;
}


//
// Line class
//

Sketch::Segment::SegmentType Sketch::Line::Type(void) const throw() {
     Sketch::Segment::SegmentType ret = Sketch::Segment::Null;

    if ((m_lineSegment != 0) && (m_sketch != 0))
        ret = Sketch::Segment::Line;

    return ret;

};


Sketch::Segment* Sketch::Line::Clone(void) const throw(bad_alloc, std::bad_alloc) {
    return new Line(*this);
};


Vector2D Sketch::Line::StartPoint(void) const throw() {
    assert(m_lineSegment != 0);
    assert(m_sketch != 0);

    Vector2D ret;

    if ((m_lineSegment != 0) && (m_sketch != 0))
        ret = Vector2D(m_sketch->verts[m_lineSegment->start]);

    return ret;
};


void Sketch::Line::SetStartPoint
(
        const Vector2D& startPoint
) throw(bad_alloc) {
    assert(m_lineSegment != 0);
    assert(m_sketch != 0);

    if ((m_lineSegment != 0) && (m_sketch != 0)) {
        if (!BU_SETJUMP)
            SwapVertex(m_lineSegment->start, startPoint.coordinates, *m_sketch);
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::Line::SetStartPoint");
        }

        BU_UNSETJUMP;
    }
};


Vector2D Sketch::Line::EndPoint(void) const throw() {
    assert(m_lineSegment != 0);
    assert(m_sketch != 0);

    Vector2D ret;

    if ((m_lineSegment != 0) && (m_sketch != 0))
        ret = Vector2D(m_sketch->verts[m_lineSegment->end]);

    return ret;
};


void Sketch::Line::SetEndPoint
(
        const Vector2D& endPoint
) throw(bad_alloc) {
    assert(m_lineSegment != 0);
    assert(m_sketch != 0);

    if ((m_lineSegment != 0) && (m_sketch != 0)) {
        if (!BU_SETJUMP)
            SwapVertex(m_lineSegment->end, endPoint.coordinates, *m_sketch);
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::Line::SetEndPoint");
        }

        BU_UNSETJUMP;
    }
};


//
// CircularArc class
//

Vector3D Sketch::CircularArc::Center(void) const throw() {
    Vector3D ret;

    assert(m_circularArcSegment != 0);
    assert(m_sketch != 0);

    if ((m_circularArcSegment != 0) && (m_sketch != 0)) {
        if(m_circularArcSegment->radius <= 0.0) {
            point_t center;

            VJOIN2(center, m_sketch->V, m_sketch->verts[m_circularArcSegment->end][0],
            m_sketch->u_vec, m_sketch->verts[m_circularArcSegment->end][1], m_sketch->v_vec);

            ret = Vector3D(center);
        }
        else
            ret = Vector3D(m_sketch->verts[m_circularArcSegment->center]);
    }

    return ret;

};


void Sketch::CircularArc::SetCenter(
    Vector2D c
) throw() {
    assert(m_circularArcSegment != 0);
    assert(m_sketch != 0);

    if ((m_circularArcSegment != 0) && (m_sketch != 0)) {

        if (!BU_SETJUMP)
            SwapVertex(m_circularArcSegment->center, c.coordinates, *m_sketch);
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::CircularArc::SetCenter");
        }

        BU_UNSETJUMP;
    };
}


Sketch::Segment::SegmentType Sketch::CircularArc::Type(void) const throw() {
     Sketch::Segment::SegmentType ret = Sketch::Segment::Null;

    if ((m_circularArcSegment != 0) && (m_sketch != 0))
        ret = Sketch::Segment::CircularArc;

    return ret;

};

Sketch::Segment* Sketch::CircularArc::Clone(void) const throw(bad_alloc, std::bad_alloc) {
    return new CircularArc(*this);
};


Vector2D Sketch::CircularArc::StartPoint(void) const throw() {
    assert(m_circularArcSegment != 0);
    assert(m_sketch != 0);

    Vector2D ret;

    if ((m_circularArcSegment != 0) && (m_sketch != 0))
        ret = Vector2D(m_sketch->verts[m_circularArcSegment->start]);

    return ret;
};


void Sketch::CircularArc::SetStartPoint
(
        const Vector2D& startPoint
) throw(bad_alloc) {
    assert(m_circularArcSegment != 0);
    assert(m_sketch != 0);

    if ((m_circularArcSegment != 0) && (m_sketch != 0)) {

        if (!BU_SETJUMP)
            SwapVertex(m_circularArcSegment->start, startPoint.coordinates, *m_sketch);
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::CircularArc::SetStartPoint");
        }

        BU_UNSETJUMP;
    };
};


Vector2D Sketch::CircularArc::EndPoint(void) const throw() {
    return Vector2D(m_sketch->verts[m_circularArcSegment->end]);
};


void Sketch::CircularArc::SetEndPoint
(
        const Vector2D& endPoint
) throw(bad_alloc) {
    assert(m_circularArcSegment != 0);
    assert(m_sketch != 0);

    if ((m_circularArcSegment != 0) && (m_sketch != 0)) {

        if (!BU_SETJUMP)
            SwapVertex(m_circularArcSegment->end, endPoint.coordinates, *m_sketch);
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::CircularArc::SetEndPoint");
        }

        BU_UNSETJUMP;
    }
};


double Sketch::CircularArc::Radius(void) const throw() {
   assert(m_circularArcSegment != 0);
   assert(m_sketch != 0);
   return m_circularArcSegment->radius;
};


void Sketch::CircularArc::SetRadius
(
    double Radius
) throw() {
    assert(m_circularArcSegment != 0);
    assert(m_sketch != 0);
    m_circularArcSegment->radius =  Radius;
};


bool Sketch::CircularArc::CenterIsLeft(void) const throw() {
    assert(m_circularArcSegment != 0);
    assert(m_sketch != 0);
    return m_circularArcSegment->center_is_left;
};


void Sketch::CircularArc::SetCenterIsLeft
(
    bool centerIsLeft
    ) throw() {
    assert(m_circularArcSegment != 0);
    assert(m_sketch != 0);
    m_circularArcSegment->center_is_left = centerIsLeft;
};


bool Sketch::CircularArc::ClockwiseOriented(void) const throw() {
    assert(m_circularArcSegment != 0);
    assert(m_sketch != 0);
    return this->m_circularArcSegment->orientation;
};


void Sketch::CircularArc::SetClockwiseOriented
(
    bool clockwiseOriented
) throw() {
    assert(m_circularArcSegment != 0);
    assert(m_sketch != 0);
    m_circularArcSegment->orientation = clockwiseOriented;
};


//
// Nurb class
//

Sketch::Segment::SegmentType Sketch::Nurb::Type(void) const throw() {
     Sketch::Segment::SegmentType ret = Sketch::Segment::Null;

    if ((m_nurbSegment != 0) && (m_sketch != 0))
        ret = Sketch::Segment::Nurb;
    return ret;
};


Sketch::Segment* Sketch::Nurb::Clone(void) const throw(bad_alloc, std::bad_alloc) {
    return new Nurb(*this);
};


Vector2D Sketch::Nurb::StartPoint(void) const throw() {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);
    Vector2D ret;
    if ((m_nurbSegment != 0) && (m_sketch != 0))
        ret = Vector2D(m_sketch->verts[m_nurbSegment->ctl_points[0]]);

    return ret;
};


void Sketch::Nurb::SetStartPoint
(
        const Vector2D& startPoint
) throw(bad_alloc) {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);

    if ((m_nurbSegment != 0) && (m_sketch != 0)) {
        if (!BU_SETJUMP)
            SwapVertex(m_nurbSegment->ctl_points[0], startPoint.coordinates, *m_sketch);
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::Nurb::SetStartPoint");
        }

        BU_UNSETJUMP;
    }
};


Vector2D Sketch::Nurb::EndPoint(void) const throw() {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);
    Vector2D ret;
    if ((m_nurbSegment != 0) && (m_sketch != 0))
        ret = Vector2D(m_sketch->verts[m_nurbSegment->ctl_points[m_nurbSegment->c_size]]);
    return ret;
};


void Sketch::Nurb::SetEndPoint
(
        const Vector2D& endPoint
) throw(bad_alloc) {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);

    if ((m_nurbSegment != 0) && (m_sketch != 0)) {
        if (!BU_SETJUMP)
            SwapVertex(m_nurbSegment->ctl_points[m_nurbSegment->c_size], endPoint.coordinates, *m_sketch);
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::Nurb::SetEndPoint");
        }

        BU_UNSETJUMP;
    }
};


size_t Sketch::Nurb::Order(void) const throw() {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);
    return m_nurbSegment->order;
};


bool  Sketch::Nurb::IsRational(void) const throw() {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);
    if(m_nurbSegment->weights)
        return true;
    return false;
};


size_t Sketch::Nurb::NumberOfKnots(void) const throw() {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);
    return m_nurbSegment->k.k_size;
};


double Sketch::Nurb::Knot
(
    size_t index
) const throw() {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);
    return m_nurbSegment->k.knots[index];
};


size_t Sketch::Nurb::NumberOfControlPoints(void)const throw() {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);
    return this->m_nurbSegment->c_size;
};


Vector2D Sketch::Nurb::ControlPoint
(
    size_t index
) const throw() {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);
    Vector2D ret;
    if ((m_nurbSegment != 0) && (m_sketch != 0))
        ret = Vector2D(m_sketch->verts[this->m_nurbSegment->ctl_points[index]]);
    return ret;
};


double Sketch::Nurb::ControlPointWeight
(
    size_t index
) const throw() {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);
    if(!IsRational())
        return 0;
    return this->m_nurbSegment->weights[index];
};


void Sketch::Nurb::SetOrder
(
    size_t order
) throw() {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);
    m_nurbSegment->order = order;
};


void Sketch::Nurb::AddKnot
(
    double knot
) throw() {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);

    m_nurbSegment->k.k_size++;
    m_nurbSegment->k.knots = (double*) bu_realloc(m_nurbSegment->k.knots,
            m_nurbSegment->k.k_size * sizeof(double), "Sketch::Nurb::AddKnot");
    m_nurbSegment->k.knots[m_nurbSegment->k.k_size - 1] = knot;

};


void Sketch::Nurb::AddControlPoint
(
    const Vector2D& point
) throw() {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);

    int vcount, ret;
    vcount = m_sketch->vert_count;
    ret = AddToVerts(point.coordinates, *m_sketch);
    if(vcount < ret) {
        m_nurbSegment->c_size++;
        m_nurbSegment->ctl_points = (int*) bu_realloc(m_nurbSegment->ctl_points,
            m_nurbSegment->c_size * sizeof(int), "Sketch::Nurb::AddControlPoint");
        m_nurbSegment->ctl_points[m_nurbSegment->c_size - 1] = ret;
        if(!m_nurbSegment->weights)
            m_nurbSegment->weights = (fastf_t*) bu_realloc(m_nurbSegment->weights,
            m_nurbSegment->c_size * sizeof(fastf_t), "Sketch::Nurb::AddControlPoint - weights");

    }
};


void Sketch::Nurb::AddControlPointWeight
(
    const Vector2D& point,
    double          weight
) throw() {
    assert(m_nurbSegment != 0);
    assert(m_sketch != 0);

    int ret = AddToVerts(point.coordinates, *m_sketch);

    if(!m_nurbSegment->weights) {
        m_nurbSegment->weights = (fastf_t*) bu_calloc(m_nurbSegment->c_size, sizeof(fastf_t), "Sketch::Nurb::AddControlPointWeight");
        m_nurbSegment->weights[ret] = weight;
    }
};


//
// Bezier class
//

 Sketch::Segment::SegmentType Sketch::Bezier::Type(void) const throw() {
     Sketch::Segment::SegmentType ret = Sketch::Segment::Null;

    if ((m_bezierSegment != 0) && (m_sketch != 0))
        ret = Sketch::Segment::Bezier;

    return ret;

};


Sketch::Segment* Sketch::Bezier::Clone(void) const throw(bad_alloc, std::bad_alloc) {
    return new Bezier(*this);
};


Vector2D Sketch::Bezier::StartPoint(void) const throw() {
    assert(m_bezierSegment != 0);
    assert(m_sketch != 0);

    Vector2D ret;
    if ((m_bezierSegment != 0) && (m_sketch != 0))
        ret = Vector2D(m_sketch->verts[m_bezierSegment->ctl_points[0]]);
    return ret;
};


void Sketch::Bezier::SetStartPoint
(
        const Vector2D& startPoint
) throw(bad_alloc) {
    assert(m_bezierSegment != 0);
    assert(m_sketch != 0);

    if ((m_bezierSegment != 0) && (m_sketch != 0)) {
        if (!BU_SETJUMP)
            SwapVertex(m_bezierSegment->ctl_points[0], startPoint.coordinates, *m_sketch);
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::Bezier::SetStartPoint");
        }

        BU_UNSETJUMP;
    }
};


Vector2D Sketch::Bezier::EndPoint(void) const throw() {
    return Vector2D(m_sketch->verts[m_bezierSegment->ctl_points[m_bezierSegment->degree]]);
};


void Sketch::Bezier::SetEndPoint
(
        const Vector2D& endPoint
) throw(bad_alloc) {
    assert(m_bezierSegment != 0);
    assert(m_sketch != 0);

    if ((m_bezierSegment != 0) && (m_sketch != 0)) {
        if (!BU_SETJUMP)
            SwapVertex(m_bezierSegment->ctl_points[m_bezierSegment->degree], endPoint.coordinates, *m_sketch);
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::Bezier::SetEndPoint");
        }

        BU_UNSETJUMP;
    }
};


size_t Sketch::Bezier::Degree(void) const throw() {
    assert(m_bezierSegment != 0);
    assert(m_sketch != 0);

    return m_bezierSegment->degree;
};


Vector2D Sketch::Bezier::ControlPoint(
    size_t index
) const throw() {
    assert(m_bezierSegment != 0);
    assert(m_sketch != 0);

    if(index >= 0 && index <= m_bezierSegment->degree)
    return Vector2D(m_sketch->verts[m_bezierSegment->ctl_points[index]]);
    return NULL;
};



void Sketch::Bezier::AddControlPoint
(
    const Vector2D& Point
) throw() {
    assert(m_bezierSegment != 0);
    assert(m_sketch != 0);

    int vcount, ret;
    vcount = m_sketch->vert_count;
    ret = AddToVerts(Point.coordinates, *m_sketch);
    if(vcount < ret) {
        m_bezierSegment->degree++;
        m_bezierSegment->ctl_points = (int*) bu_realloc(m_bezierSegment->ctl_points,
            m_bezierSegment->degree * sizeof(int), "Sketch::Bezier::AddControlPoint");
        m_bezierSegment->ctl_points[m_bezierSegment->degree] = ret;

    }
};


//
// Sketch class
//

void Sketch::Get
(
    size_t                index,
    ConstSegmentCallback& callback
) const throw() {
    if(Internal() != 0) {
        if(!BU_SETJUMP) {
            const uint32_t *magic = reinterpret_cast<uint32_t*>(Internal()->curve.segment[index]);
            switch (*magic) {
                case CURVE_LSEG_MAGIC: {
                    line_seg* line = reinterpret_cast<line_seg*>(Internal()->curve.segment[index]);
                    Line lineClass(line, const_cast<rt_sketch_internal*>(Internal()));
                    callback(lineClass);
                    }
                    break;

                case CURVE_CARC_MAGIC: {
                    carc_seg* carc = reinterpret_cast<carc_seg*>(Internal()->curve.segment[index]);
                    CircularArc arcClass(carc, const_cast<rt_sketch_internal*>(Internal()));
                    callback(arcClass);
                    }
                    break;

                case CURVE_NURB_MAGIC: {
                    nurb_seg* nurb = reinterpret_cast<nurb_seg*>(Internal()->curve.segment[index]);
                    Nurb nurbClass(nurb, const_cast<rt_sketch_internal*>(Internal()));
                    callback(nurbClass);
                    }
                    break;

                case CURVE_BEZIER_MAGIC: {
                    bezier_seg* bezier = reinterpret_cast<bezier_seg*>(Internal()->curve.segment[index]);
                    Bezier bezierClass(bezier, const_cast<rt_sketch_internal*>(Internal()));
                    callback(bezierClass);
                    }
                    break;
            };
        };
    }
};


void Sketch::Get
(
    size_t           index,
    SegmentCallback& callback
) throw() {
    if(Internal() != 0) {
        if(!BU_SETJUMP) {
            const uint32_t *magic = reinterpret_cast<uint32_t*>(Internal()->curve.segment[index]);
            switch (*magic) {
                case CURVE_LSEG_MAGIC: {
                    line_seg* line = reinterpret_cast<line_seg*>(Internal()->curve.segment[index]);
                    Line lineClass(line, static_cast<rt_sketch_internal*>(Internal()));
                    callback(lineClass);
                    }
                    break;

                case CURVE_CARC_MAGIC: {
                    carc_seg* carc = reinterpret_cast<carc_seg*>(Internal()->curve.segment[index]);
                    CircularArc arcClass(carc, static_cast<rt_sketch_internal*>(Internal()));
                    callback(arcClass);
                    }
                    break;

                case CURVE_NURB_MAGIC: {
                    nurb_seg* nurb = reinterpret_cast<nurb_seg*>(Internal()->curve.segment[index]);
                    Nurb nurbClass(nurb, static_cast<rt_sketch_internal*>(Internal()));
                    callback(nurbClass);
                    }
                    break;

                case CURVE_BEZIER_MAGIC: {
                    bezier_seg* bezier = reinterpret_cast<bezier_seg*>(Internal()->curve.segment[index]);
                    Bezier bezierClass(bezier, static_cast<rt_sketch_internal*>(Internal()));
                    callback(bezierClass);
                    }
            };
        };
    }
};


Sketch::Segment* Sketch::Get
(
    size_t index
) const throw(bad_alloc, std::bad_alloc) {
    class SegmentCallbackIntern: public ConstSegmentCallback {
    public:
        SegmentCallbackIntern(void) : ConstSegmentCallback(), m_segment(0) {}
        virtual ~SegmentCallbackIntern(void) throw() {}
        virtual void operator()(const Segment& segment) {
            try {
                m_segment = segment.Clone();
            }
            catch(std::bad_alloc&) {}
        }
        Sketch::Segment* GetSegment(void) const throw() {
            return m_segment;
        };

    private:
        Segment* m_segment;
    } segCallbackIntern;

    Get(index, segCallbackIntern);

    return segCallbackIntern.GetSegment();
};


Sketch::Line* Sketch::AppendLine(void) throw(bad_alloc, std::bad_alloc) {
    Sketch::Line* ret = 0;

    if (!BU_SETJUMP) {
        rt_sketch_internal* sketch = Internal();
        line_seg*           line   = static_cast<line_seg*>(bu_calloc(1, sizeof(line_seg), "Sketch::AppendLine: line_seg"));

        line->magic = CURVE_LSEG_MAGIC;

        // start and end point are arbitrary (but valid: index 0)
        if (sketch->vert_count == 0) {
            point2d_t zero = {0.};

            AddToVerts(zero, *sketch);
        }

        AppendSegment(line, sketch);
        ret = new Line(line, sketch);
    }
    else {
        BU_UNSETJUMP;
        throw bad_alloc("BRLCAD::Sketch::AppendLine");
    }

    BU_UNSETJUMP;

    return ret;
};


Sketch::Line* Sketch::InsertLine
(
        size_t index
) throw(bad_alloc, std::bad_alloc) {
    Sketch::Line*       ret    = 0;
    rt_sketch_internal* sketch = Internal();

    if (index < sketch->curve.count) {
        if (!BU_SETJUMP) {
            line_seg* line = static_cast<line_seg*>(bu_calloc(1, sizeof(line_seg), "Sketch::InsertLine: line_seg"));

            line->magic = CURVE_LSEG_MAGIC;

            // start and end point are arbitrary (but valid: index 0)
            if (sketch->vert_count == 0) {
                point2d_t zero = {0.};

                AddToVerts(zero, *sketch);
            }

            InsertSegment(line, index, sketch);
            ret = new Line(line, sketch);
        }
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::InsertLine");
        }

        BU_UNSETJUMP;
    }

    return ret;
};


Sketch::CircularArc* Sketch::AppendArc(void) throw(bad_alloc, std::bad_alloc) {
    Sketch::CircularArc* ret = 0;

    if (!BU_SETJUMP) {
        rt_sketch_internal* sketch = Internal();
        carc_seg* carc =  static_cast<carc_seg*>(bu_calloc(1, sizeof(carc_seg), "Sketch::AppendArc: carc_seg"));
        carc->magic = CURVE_CARC_MAGIC;

        // start and end point are arbitrary (but valid: index 0)
        if (sketch->vert_count == 0) {
            point2d_t zero = {0.};

            AddToVerts(zero, *sketch);
        }

        AppendSegment(carc, sketch);
        ret = new CircularArc(carc, sketch);

        }
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::AppendArc");
        }

        BU_UNSETJUMP;

        return ret;
};


Sketch::CircularArc* Sketch::InsertArc
(
        size_t index
) throw(bad_alloc, std::bad_alloc) {
    Sketch::CircularArc*       ret    = 0;
    rt_sketch_internal* sketch = Internal();

    if (index < sketch->curve.count) {
        if (!BU_SETJUMP) {
            carc_seg* carc = static_cast<carc_seg*>(bu_calloc(1, sizeof(carc_seg), "Sketch::InsertArc: carc_seg"));

            carc->magic = CURVE_CARC_MAGIC;

            // start and end point are arbitrary (but valid: index 0)
            if (sketch->vert_count == 0) {
                point2d_t zero = {0.};

                AddToVerts(zero, *sketch);
            }

            InsertSegment(carc, index, sketch);
            ret = new CircularArc(carc, sketch);
        }
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::InsertLine");
        }

        BU_UNSETJUMP;
    }

    return ret;
};


Sketch::Nurb* Sketch::AppendNurb(void) throw(bad_alloc, std::bad_alloc) {
    Sketch::Nurb* ret = 0;
    if (!BU_SETJUMP) {
        rt_sketch_internal* sketch = Internal();
        nurb_seg*           nurb   = static_cast<nurb_seg*>(bu_calloc(1, sizeof(nurb_seg), "Sketch::AppendNurb: nurb_seg"));
        nurb->magic = CURVE_NURB_MAGIC;

        // start and end point are arbitrary (but valid: index 0)
        if (sketch->vert_count == 0) {
            point2d_t zero = {0.};

            AddToVerts(zero, *sketch);
        }

        AppendSegment(nurb, sketch);
        ret = new Nurb(nurb, sketch);

    }
    else {
        BU_UNSETJUMP;
        throw bad_alloc("BRLCAD::Sketch::AppendLine");
    }

    BU_UNSETJUMP;

    return ret;
};


Sketch::Nurb* Sketch::InsertNurb
(
        size_t index
) throw(bad_alloc, std::bad_alloc) {
    Sketch::Nurb*       ret    = 0;
    rt_sketch_internal* sketch = Internal();
    if (index < sketch->curve.count) {
        if (!BU_SETJUMP) {
            nurb_seg* nurb = static_cast<nurb_seg*>(bu_calloc(1, sizeof(nurb_seg), "Sketch::InsertNurb: nurb_seg"));

            nurb->magic = CURVE_NURB_MAGIC;

            // start and end point are arbitrary (but valid: index 0)
            if (sketch->vert_count == 0) {
                point2d_t zero = {0.};

                AddToVerts(zero, *sketch);
            }

            InsertSegment(nurb, index, sketch);
            ret = new Nurb(nurb, sketch);
        }
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::InsertNurb");
        }

        BU_UNSETJUMP;
    }

    return ret;
};


Sketch::Bezier* Sketch::AppendBezier(void) throw(bad_alloc, std::bad_alloc) {
    Sketch::Bezier* ret = 0;
    if (!BU_SETJUMP) {
    rt_sketch_internal* sketch = Internal();
    bezier_seg*           bezier   = static_cast<bezier_seg*>(bu_calloc(1, sizeof(bezier_seg), "Sketch::AppendBezier: bezier_seg"));
    bezier->magic = CURVE_BEZIER_MAGIC;

    // start and end point are arbitrary (but valid: index 0)
    if (sketch->vert_count == 0) {
        point2d_t zero = {0.};

        AddToVerts(zero, *sketch);
    }

    AppendSegment(bezier, sketch);
    ret = new Bezier(bezier, sketch);
    }
    else {
        BU_UNSETJUMP;
        throw bad_alloc("BRLCAD::Sketch::AppendLine");
    }

    BU_UNSETJUMP;

    return ret;
};


Sketch::Bezier* Sketch::InsertBezier
(
    size_t index
) throw(bad_alloc, std::bad_alloc) {
    Sketch::Bezier*       ret    = 0;
    rt_sketch_internal* sketch = Internal();
    if (index < sketch->curve.count) {
        if (!BU_SETJUMP) {
            bezier_seg* bezier = static_cast<bezier_seg*>(bu_calloc(1, sizeof(bezier_seg), "Sketch::InsertBezier: bezier_seg"));

            bezier->magic = CURVE_BEZIER_MAGIC;

            // start and end point are arbitrary (but valid: index 0)
            if (sketch->vert_count == 0) {
                point2d_t zero = {0.};

                AddToVerts(zero, *sketch);
            }

            InsertSegment(bezier, index, sketch);
            ret = new Bezier(bezier, sketch);
        }
        else {
            BU_UNSETJUMP;
            throw bad_alloc("BRLCAD::Sketch::InsertNurb");
        }
        BU_UNSETJUMP;
    }
    return ret;
};


static void FreeSegment
(
    size_t              index,
    rt_sketch_internal& sketch
){
    const uint32_t *magic = static_cast<uint32_t*>(sketch.curve.segment[index]);
    switch (*magic) {
        case CURVE_LSEG_MAGIC: {
            line_seg* line = static_cast<line_seg*>(sketch.curve.segment[index]);
            RemoveFromVerts(line->start, sketch);
            RemoveFromVerts(line->end, sketch);
            bu_free(line,"Sketch::DeleteSegment::bu_free");
        }
        break;

        case CURVE_CARC_MAGIC: {
            carc_seg* carc = static_cast<carc_seg*>(sketch.curve.segment[index]);
            RemoveFromVerts(carc->start, sketch);
            RemoveFromVerts(carc->end, sketch);
            bu_free(carc,"Sketch::DeleteSegment::bu_free");
        }
        break;

        case CURVE_NURB_MAGIC: {
            nurb_seg* nurb = static_cast<nurb_seg*>(sketch.curve.segment[index]);
            for(int i = 0; i < nurb->c_size; i++) {
                RemoveFromVerts(nurb->ctl_points[i], sketch);
            };
            bu_free(nurb->ctl_points,"Sketch::DeleteSegment::bu_free");
            bu_free(nurb->weights,"Sketch::DeleteSegment::bu_free");
            bu_free(nurb,"Sketch::DeleteSegment::bu_free");

        }
        break;

        case CURVE_BEZIER_MAGIC: {
                bezier_seg* bezier = static_cast<bezier_seg*>(sketch.curve.segment[index]);
                for(int i = 0; i < bezier->degree - 1; i++) {
                    RemoveFromVerts(bezier->ctl_points[i], sketch);
                };
                bu_free(bezier->ctl_points,"Sketch::DeleteSegment::bu_free");
                bu_free(bezier,"Sketch::DeleteSegment::bu_free");
        };
    }

};


void Sketch::DeleteSegment
(
    size_t index
) throw(bad_alloc) {
    assert(index < Internal()->curve.count);

    if(index < Internal()->curve.count)
        return;

    void** temp;
    int tempItr = 0;
    temp = (void**) bu_calloc(Internal()->curve.count - 1, sizeof(void*),
        "Sketch::DeleteSegment");
    for(int i = 0; i < Internal()->curve.count; i++) {
        if(i != index) {
            temp[tempItr] = Internal()->curve.segment[i];
            tempItr++;
        };
    };
    FreeSegment(index, *Internal());

    bu_free(Internal()->curve.segment, "Sketch::DeleteSegment::bu_free");
    Internal()->curve.segment = temp;
    Internal()->curve.count--;
};


int Sketch::NumberOfSegments(void) const throw() {
    return Internal()->curve.count;
};


Vector3D Sketch::EmbeddingPlaneX(void) const {
    return Vector3D(Internal()->u_vec);
};


Vector3D Sketch::EmbeddingPlaneY(void) const  {
    return Vector3D(Internal()->v_vec);
};


void Sketch::SetEmbeddingPlaneX
(
    Vector3D& u
) {
    for(int i = 0; i < 3; i++) {
        Internal()->u_vec[i] = u.coordinates[i];
    };
};


void Sketch::SetEmbeddingPlaneY
(
    Vector3D& v
) {
    for(int i = 0; i < 3; i++) {
        Internal()->v_vec[i] = v.coordinates[i];
    };
};


Vector3D Sketch::EmbeddingPlaneOrigin(void) const {
    return Vector3D(Internal()->V);
};


void Sketch::SetEmbeddingPlaneOrigin
(
    Vector3D& p
) {
    Internal()->V[0] = p.coordinates[0];
    Internal()->V[1] = p.coordinates[1];
    Internal()->V[2] = p.coordinates[2];
};


const Object& Sketch::operator=
(
    const Object& original
) throw(bad_alloc) {
    const Sketch* sketch = dynamic_cast<const Sketch*>(&original);
    assert(sketch != 0);

    if (sketch != 0)
        *this = *sketch;

    return *this;
};


Object* Sketch::Clone(void) const throw(bad_alloc, std::bad_alloc) {
    return new Sketch(*this);
};


const char* Sketch::ClassName(void) throw() {
    return "Sketch";
};


const char* Sketch::Type(void) const throw() {
    return ClassName();
};


rt_sketch_internal* Sketch::Internal(void) throw() {
    rt_sketch_internal* ret;

    if (m_ip != 0)
        ret = static_cast<rt_sketch_internal*>(m_ip->idb_ptr);
    else
        ret = m_internalp;

    RT_SKETCH_CK_MAGIC(ret);

    return ret;
};


const rt_sketch_internal* Sketch::Internal(void) const throw() {
    const rt_sketch_internal* ret;

    if (m_ip != 0)
        ret = static_cast<const rt_sketch_internal*>(m_ip->idb_ptr);
    else
        ret = m_internalp;

    RT_SKETCH_CK_MAGIC(ret);

    return ret;
}


bool Sketch::IsValid(void) const throw() {
    if (rt_check_curve(&Internal()->curve, Internal(), 1))
        return false;
    return true;
};


Sketch::Sketch
(
    resource*       resp,
    directory*      pDir,
    rt_db_internal* ip,
    db_i*           dbip
) throw() : Object(resp, pDir, ip, dbip), m_internalp(0) {}
