/*                         E L L I P T I C A L T O R U S . H
 * BRL-CAD
 *
 * Copyright (c) 2020 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/** @file ellipticaltorus.h
 *
 *  BRL-CAD embedded lua script:
 *      BRLCAD::EllipticalTorus functions
 */

#ifndef ELLIPTICALTORUS_INCLUDED
#define ELLIPTICALTORUS_INCLUDED

#include "lua.hpp"

#include "brlcad/EllipticalTorus.h"


void InitEllipticalTorus
(
    lua_State* luaState
);


int PushEllipticalTorus
(
    lua_State*               luaState,
    BRLCAD::EllipticalTorus* object,
    bool                     takeOwnership
);


BRLCAD::EllipticalTorus* TestEllipticalTorus
(
    lua_State* luaState,
    int        narg
);


#endif // ELLIPTICALTORUS_INCLUDED
