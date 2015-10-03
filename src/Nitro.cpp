/*************************************************************************
    Nitro.cpp
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

#include "Nitro.h"
#include <cstring>
#include <cctype>

static void strLower(char* p){
    while(*p){
        *p=std::tolower(*p);
        ++p;
    }
}

u16 nitroGetSubFileId(std::FILE *file,const char* pathname)
{
    std::fseek(file,0,SEEK_SET);//file.Seek(0,CFile::begin);
    ROM_HEADER rom_header;
    std::fread(&rom_header,sizeof(rom_header),1,file);//file.Read(&rom_header,sizeof(rom_header));

    u32 fntdir_offset=0;
    ROM_FNTDir fntdir;
    while(1)
    {
        std::fseek(file,rom_header.fnt_offset+fntdir_offset,SEEK_SET);//file.Seek(rom_header.fnt_offset+fntdir_offset,CFile::begin);
        std::fread(&fntdir,sizeof(fntdir),1,file);//file.Read(&fntdir,sizeof(fntdir));
        std::fseek(file,fntdir.entry_start+rom_header.fnt_offset,SEEK_SET);//file.Seek(fntdir.entry_start+rom_header.fnt_offset,CFile::begin);

        u8 fnamelen=0;
        bool isdir;
        for(fnamelen=0;;fnamelen++)
        {
            if(*(pathname+fnamelen)=='\0')
            {
                isdir=false;
                break;
            }
            if(*(pathname+fnamelen)=='/')
            {
                isdir=true;
                break;
            }
        }
        char* namebuf;
        namebuf=new char[fnamelen+1];
        std::memcpy(namebuf,pathname,fnamelen+1);
        namebuf[fnamelen]='\0';
        pathname+=fnamelen+1;

        u8 fh;
        u8 name_length;
        bool is_dir;
        u16 index_d;
        index_d=0;
        while(1)
        {
            std::fread(&fh,sizeof(fh),1,file);//file.Read(&fh,sizeof(fh));
            name_length=fh&0x7F;
            is_dir=fh&0x80?true:false;
            if(!name_length){delete[]namebuf;return 0xFFFF;}
            char* name_buf;
            name_buf=new char[name_length+1];
            std::fread(name_buf,name_length,1,file);//file.Read(name_buf,name_length);
            name_buf[name_length]=0;
            if(is_dir==isdir && name_length==fnamelen)
            {
                strLower(name_buf);
                strLower(namebuf);
                if(!std::strcmp(name_buf,namebuf))
                {
                    delete[] name_buf;
                    break;
                }


            }
            delete[] name_buf;
            if(is_dir)
            {
                std::fseek(file,2,SEEK_CUR);//file.Seek(2,CFile::current);
            }
            else
            {
                index_d++;
            }
        }
        if(isdir)
        {
            u16 child_dir_id;
            std::fread(&child_dir_id,sizeof(child_dir_id),1,file);//file.Read(&child_dir_id,sizeof(child_dir_id));
            fntdir_offset=(child_dir_id-0xF000)*sizeof(ROM_FNTDir);
        }
        else
        {
            u16 fileid;
            fileid=fntdir.entry_file_id+index_d;
            delete[] namebuf;
            return fileid;
        }

        delete[] namebuf;
    }

    return 0xFFFF;

}
u32 nitroGetSubFileOffset(std::FILE* file,u16 id,u32* getlen)
{

    std::fseek(file,0,SEEK_SET);//file.Seek(0,CFile::begin);
    ROM_HEADER rom_header;
    std::fread(&rom_header,sizeof(rom_header),1,file);//file.Read(&rom_header,sizeof(rom_header));
    if(id>=rom_header.fat_size/sizeof(ROM_FAT))return 0;

    std::fseek(file,rom_header.fat_offset+sizeof(ROM_FAT)*id,SEEK_SET);//file.Seek(rom_header.fat_offset+sizeof(ROM_FAT)*id,CFile::begin);
    ROM_FAT fat;
    std::fread(&fat,sizeof(fat),1,file);//file.Read(&fat,sizeof(fat));
    if(getlen)*getlen=fat.bottom-fat.top;
    return fat.top;

}
void nitroSetSubFileOffset(std::FILE *file, u16 id, u32 from, u32 len){
    std::fseek(file,0,SEEK_SET);
    ROM_HEADER rom_header;
    std::fread(&rom_header,sizeof(rom_header),1,file);
    if(id>=rom_header.fat_size/sizeof(ROM_FAT))return;

    std::fseek(file,rom_header.fat_offset+sizeof(ROM_FAT)*id,SEEK_SET);
    ROM_FAT fat;
    fat.top=from;
    fat.bottom=fat.top+len;
    std::fwrite(&fat,sizeof(fat),1,file);
}

// Crc16()
// Taken from DeSmuME
u16 nitroCrc16(void *buf,u32 len)
{
    u16 crc=0xFFFF;
    u16 currVal=0;
    const u16 val[] = {
        0x0000,0xCC01,0xD801,0x1400,
        0xF001,0x3C00,0x2800,0xE401,
        0xA001,0x6C00,0x7800,0xB401,
        0x5000,0x9C01,0x8801,0x4400};
    len>>=1;
    for(u32 i=0;i<len;++i)
    {
        currVal=((u16*)buf)[i];
        for(int j=0;j<4;++j)
        {
            u16 tabVal=val[crc&0xF];
            crc>>=4;
            crc^=tabVal;
            u16 tempVal=currVal>>(j<<2);
            tabVal=val[tempVal&0xF];
            crc^=tabVal;
        }
    }
    return crc;
}
