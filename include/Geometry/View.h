/*
 * BRL-CAD
 *
 * Copyright (c) 1997-2009 United States Government as represented by
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
/** @file View.h
 *
 *	Description:
 *
 *	$HeadURL: $
 *	$Date$
 *	$Revision$
 *	$Author$ 
 *
 */
 
#ifndef _VIEW_H_
#define _VIEW_H_

#include <iostream>
#include <vector>

#include "Geometry/Triangle.h"
#include "Geometry/View.h"


namespace Geometry {

  class View
  {
  private:
    double _from[3];
    double _at[3];
    double _up[3];
    double _angle;
    double _hither;
    
    double _x;
    double _y;
  protected:
  public:
    View(double from[3], double at[3], double up[3], double angle, double hither);
    ~View();

    void setResolution(unsigned long int x, unsigned long int y) {
      _x = x;
      _y = y;
    }

  };

}

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
