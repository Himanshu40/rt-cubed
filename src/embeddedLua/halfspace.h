/*                         H A L F S P A C E . H
 * BRL-CAD
 *
 * Copyright (c) 2019 United States Government as represented by
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
/** @file halfspace.h
 *
 *  BRL-CAD embedded lua script:
 *      BRLCAD::Halfspace functions
 */

#ifndef HALFSPACE_INCLUDED
#define HALFSPACE_INCLUDED

#include "lua.hpp"

#include "brlcad/Halfspace.h"


void InitHalfspace
(
    lua_State* luaState
);


int PushHalfspace
(
    lua_State*         luaState,
    BRLCAD::Halfspace* object,
    bool               takeOwnership
);


BRLCAD::Halfspace* TestHalfspace
(
    lua_State* luaState,
    int        narg
);


#endif // HALFSPACE_INCLUDED
