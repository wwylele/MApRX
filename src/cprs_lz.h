/*************************************************************************
    cprs_lz.h
    :glue between NitroLz.cpp and cprs_lz.cpp
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

/*
 * Taken from grit/cldib/cldib_core.h
 */
#ifndef _CPRS_LZ_H_
#define _CPRS_LZ_H_
typedef unsigned char   BYTE, uchar, echar;
typedef unsigned short  ushort, eshort;
typedef unsigned int    uint, eint, DWORD;



typedef struct RECORD
{
    int width;		//!< Width of \a data / datatype
    int height;		//!< Height of data / Length of data
    BYTE *data;		//!< Binary data.
} RECORD;

uint lz77gba_compress(RECORD *dst, const RECORD *src);
uint lz77gba_decompress(RECORD *dst, const RECORD *src);

#endif
