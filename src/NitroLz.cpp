/*************************************************************************
    NitroLz.cpp
    :A wrapper for cprs_lz.cpp, just like DS a wrapper for GBA
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

#include "NitroLz.h"
#include <cstring>
#include <cstdlib>
#include "cprs_lz.h"

u32 compressLZ(const u8 *srcp,u32 size,u8 *dstp)
{
    RECORD src,dst;
    src.data=(BYTE*)srcp;
    src.width=1;
    src.height=size;
    dst.data=0;
    lz77gba_compress(&dst,&src);
    std::memcpy(dstp,dst.data,dst.height);
    std::free(dst.data);
    return dst.height;
}
void uncompressLZ(const  u8 *srcp,u8 *destp)
{
    RECORD src,dst;
    src.data=(BYTE*)srcp;
    dst.data=0;
    lz77gba_decompress(&dst,&src);
    std::memcpy(destp,dst.data,dst.height);
    std::free(dst.data);
}

u32 getLengthLZ(const u8* srcp){
    u32 length;
    memcpy(&length,srcp,4);
    return length>>8;
}
