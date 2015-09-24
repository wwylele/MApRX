/*************************************************************************
    NitroLz.h
    Copyright (C) 2015 wwylele

    This file is part of MApRX.

    MApRX is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MApRX is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MApRX.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef _NITROLZ_H_
#define _NITROLZ_H_
#include "Nitro.h"

void uncompressLZ(const u8* srcp,u8 *destp);
u32 compressLZ(const u8* srcp,u32 size,u8* destp);
u32 getLengthLZ(const u8* srcp);
#endif
