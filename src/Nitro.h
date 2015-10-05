/*************************************************************************
    Nitro.h
    :Utility for DS ROM

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

#ifndef _NITRO_H_
#define _NITRO_H_
#include <cstdio>
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef signed char s8;
typedef signed short s16;
typedef signed long s32;

struct Color15
{
    u16 data;
    inline Color15(){}
    inline Color15(u16 tr,u16 tg,u16 tb){
        data=tr|(tg<<5)|(tb<<10);
    }
    inline u16 r()const{
        return data&31;
    }
    inline u16 g()const{
        return (data>>5)&31;
    }
    inline u16 b()const{
        return (data>>10)&31;
    }

    inline u32 toARGB32()const {
        u8 tr,tg,tb;
        tr=(r()*255+15)/31;
        tg=(g()*255+15)/31;
        tb=(b()*255+15)/31;
        return 0xFF000000|(tr<<16)|(tg<<8)|tb;
    }
    inline u32 toGray32()const {
        u8 t;
        t=((r()+g()+b())/3*255+15)/31;
        return 0xFF000000|(t<<16)|(t<<8)|t;
    }
    static inline Color15 lerp(Color15 Zero,Color15 One,float v){
        return Color15((u16)(Zero.r()*(1-v)+One.r()*v),
                       (u16)(Zero.g()*(1-v)+One.g()*v),
                       (u16)(Zero.b()*(1-v)+One.b()*v)
                       );
    }
    
};

typedef u16 CharData;
enum CharDataFlags{
    TILE_ID_MASK=1023,
    FLIP_X=1024,
    FLIP_Y=2048,
};

struct Tile8bpp
{
    u8 data[64];
    inline u8& pixel(u8 x/*0~7*/,u8 y/*0~7*/){
        return data[x|(y<<3)];
    }
    inline u8 pixel(u8 x/*0~7*/,u8 y/*0~7*/)const{
        return data[x|(y<<3)];
    }
    template<typename T/* [](int x,int y,const Color15&) */,typename U/*KfPlt or sth else */>
    void draw(T fSetPixel,const U& plt,int dx,int dy,bool flipX,bool flipY)const{
        for(int x=0;x<8;x++)for(int y=0;y<8;y++){
            u8 c;
            c=pixel(flipX?7-x:x,flipY?7-y:y);
            if(c)fSetPixel(dx+x,dy+y,plt.Colors(c));
        }
    }
};


struct ROM_HEADER
{

    u8 name[12];
    u8 id[4];
    u8 maker_code[2];
    u8 product_code;
    u8 device_type;
    u8 device_caps;
    u8 rom_version;
    u16 rom_ctrl_info[5];

    u32    main_rom_offset;    // Transmit source ROM offset
    u32    main_entry_address;    // Execution start address (not implemented)
    u32    main_ram_address;    // Transmit destination RAM address
    u32    main_size;    // Transmit size
    //    ARM7
    u32    sub_rom_offset;    // Transmit source ROM offset
    u32    sub_entry_address;    // Execution start address (not implemented)
    u32    sub_ram_address;    // Transmit destination RAM address
    u32    sub_size;    // Transmit size

    u32    fnt_offset;    // Top ROM offset
    u32    fnt_size;    // Table size

    u32    fat_offset;    // Top ROM offset
    u32    fat_size;    // Table size

    //    ARM9
    u32    main_ovt_offset;    // Top ROM offset
    u32    main_ovt_size;    // Table size

    //    ARM7
    u32    sub_ovt_offset;    // Top ROM offset
    u32    sub_ovt_size;    // Table size


    u32 reserved_B1;
    u32 reserved_B2;
    u32 title_offset;
    u16 CRC16;
    u16 ROMtimeout;

    u32     ARM9unk;
    u32     ARM7unk;

    u8      unknown3c[8];
    u32     ROMSize;
    u32     HeaderSize;
    u8      unknown5[56];
    u8      logo[156];
    u16     logoCRC16;
    u16     headerCRC16;
    u8      reserved[160];

};
struct ROM_FNTDir
{
    u32    entry_start;    // Reference location of entry name
    u16    entry_file_id;    // File ID of top entry
    u16    parent_id;    // ID of parent directory
};
struct ROM_FAT
{
    u32    top;    // Top ROM address of file
    u32    bottom;    // Bottom ROM address of file
};


u16 nitroGetSubFileId(std::FILE *file, const char* subfilename);

u32 nitroGetSubFileOffset(std::FILE *file, u16 Id, u32* getlen=0);
void nitroSetSubFileOffset(std::FILE *file, u16 Id, u32 from, u32 len);

u16 nitroCrc16(void *buf,u32 length);


#endif
