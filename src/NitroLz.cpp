/*
 * NitroLz.cpp
 * This file is part of MApRX.
 * Taken from an unknown source code.
 *
 */

#include "NitroLz.h"
#include <cstring>

#define LZ_COMPRESS_WORK_SIZE   10000

u8 lz77_work[LZ_COMPRESS_WORK_SIZE];

typedef struct
{
    u16     windowPos;
    u16     windowLen;

    s16    *LZOffsetTable;
    s16    *LZByteTable;
    s16    *LZEndTable;
}LZCompressInfo;
static void LZInitTable(LZCompressInfo * info,void *work)
{
    u16     i;

    info->LZOffsetTable=(s16*)work;
    info->LZByteTable=(s16*)((u32)work + 4096 * sizeof(s16));
    info->LZEndTable=(s16*)((u32)work + (4096 + 256) * sizeof(s16));

    for(i=0; i < 256; i++)
    {
        info->LZByteTable[i]=-1;
        info->LZEndTable[i]=-1;
    }
    info->windowPos=0;
    info->windowLen=0;
}
static u8 SearchLZ(LZCompressInfo * info,const u8 *nextp,u32 remainSize,u16 *offset)
{
    const u8 *searchp;
    const u8 *headp,*searchHeadp;
    u16     maxOffset;
    u8      maxLength=2;
    u8      tmpLength;
    s32     w_offset;
    s16    *const LZOffsetTable=info->LZOffsetTable;
    const u16 windowPos=info->windowPos;
    const u16 windowLen=info->windowLen;

    if(remainSize < 3)
    {
        return 0;
    }

    w_offset=info->LZByteTable[*nextp];

    while(w_offset != -1)
    {
        if(w_offset < windowPos)
        {
            searchp=nextp - windowPos + w_offset;
        }
        else
        {
            searchp=nextp - windowLen - windowPos + w_offset;
        }

        if(*(searchp + 1) != *(nextp + 1) || *(searchp + 2) != *(nextp + 2))
        {
            w_offset=LZOffsetTable[w_offset];
            continue;
        }

        if(nextp - searchp < 2)
        {
            break;
        }
        tmpLength=3;
        searchHeadp=searchp + 3;
        headp=nextp + 3;

        while(((u32)(headp - nextp) < remainSize) && (*headp == *searchHeadp))
        {
            headp++;
            searchHeadp++;
            tmpLength++;

            if(tmpLength == (0xF + 3))
            {
                break;
            }
        }

        if(tmpLength > maxLength)
        {
            maxLength=tmpLength;
            maxOffset=(u16)(nextp - searchp);
            if(maxLength == (0xF + 3))
            {
                break;
            }
        }
        w_offset=LZOffsetTable[w_offset];
    }

    if(maxLength < 3)
    {
        return 0;
    }
    *offset=maxOffset;
    return maxLength;
}

static void SlideByte(LZCompressInfo * info,const u8 *srcp)
{
    s16     offset;
    u8      in_data=*srcp;
    u16     insert_offset;

    s16    *const LZByteTable=info->LZByteTable;
    s16    *const LZOffsetTable=info->LZOffsetTable;
    s16    *const LZEndTable=info->LZEndTable;
    const u16 windowPos=info->windowPos;
    const u16 windowLen=info->windowLen;

    if(windowLen == 4096)
    {
        u8      out_data=*(srcp - 4096);
        if((LZByteTable[out_data]=LZOffsetTable[LZByteTable[out_data]]) == -1)
        {
            LZEndTable[out_data]=-1;
        }
        insert_offset=windowPos;
    }
    else
    {
        insert_offset=windowLen;
    }

    offset=LZEndTable[in_data];
    if(offset == -1)
    {
        LZByteTable[in_data]=(s16)insert_offset;
    }
    else
    {
        LZOffsetTable[offset]=(s16)insert_offset;
    }
    LZEndTable[in_data]=(s16)insert_offset;
    LZOffsetTable[insert_offset]=-1;

    if(windowLen == 4096)
    {
        info->windowPos=(u16)((windowPos + 1) % 0x1000);
    }
    else
    {
        info->windowLen++;
    }
}


static inline void LZSlide(LZCompressInfo * info,const u8 *srcp,u32 n)
{
    u32     i;

    for(i=0; i < n; i++)
    {
        SlideByte(info,srcp++);
    }
}


u32 compressLZ(const u8 *srcp,u32 size,u8 *dstp)
{
    void* work=lz77_work;
    std::memset(work,0,LZ_COMPRESS_WORK_SIZE);

    u32     LZDstCount;
    u8      LZCompFlags;
    u8     *LZCompFlagsp;
    u16     lastOffset;
    u8      lastLength;
    u8      i;
    u32     dstMax;
    LZCompressInfo info;

    if(size<=4)return 0;

    *(u32 *)dstp=size << 8 | 0x10;
    dstp+=4;
    LZDstCount=4;
    dstMax=size;
    LZInitTable(&info,work);

    while(size > 0)
    {
        LZCompFlags=0;
        LZCompFlagsp=dstp++;
        LZDstCount++;


        for(i=0; i < 8; i++)
        {
            LZCompFlags<<=1;
            if(size <= 0)
            {

                continue;
            }

            if((lastLength=SearchLZ(&info,srcp,size,&lastOffset)) != 0)
            {

                LZCompFlags|=0x1;

                if(LZDstCount + 2 >= dstMax)
                {
                    return 0;
                }

                *dstp++=(u8)((lastLength - 3) << 4 | (lastOffset - 1) >> 8);
                *dstp++=(u8)((lastOffset - 1) & 0xff);
                LZDstCount+=2;
                LZSlide(&info,srcp,lastLength);
                srcp+=lastLength;
                size-=lastLength;
            }
            else
            {
                LZSlide(&info,srcp,1);
                *dstp++=*srcp++;
                size--;
                LZDstCount++;
            }
        }
        *LZCompFlagsp=LZCompFlags;
    }

    i=0;
    while((LZDstCount + i) & 0x3)
    {
        *dstp++=0;
        i++;
    }

    return LZDstCount;
}
void uncompressLZ(const  u8 *srcp,u8 *destp)
{
    const u8* pSrc=srcp;
    u8*       pDst=destp;
    s32       destCount=*(u32 *)pSrc >> 8;

    pSrc+=4;
    while(destCount > 0)
    {
        u32 i;
        u32 flags=*pSrc++;
        for(i=0; i < 8; ++i)
        {
            if(!(flags & 0x80))
            {
                *pDst++=*pSrc++;
                destCount--;
            }
            else
            {
                s32 length=(*pSrc >> 4) + 3;
                s32 offset=(*pSrc++ & 0x0f) << 8;
                offset=(offset | *pSrc++) + 1;
                destCount-=length;
                do
                {
                    *pDst=pDst[-offset];
                    ++pDst;
                } while(--length > 0);
            }
            if(destCount <= 0)
            {
                break;
            }
            flags<<=1;
        }
    }
}

u32 getLengthLZ(const u8* srcp){
    u32 length=*(u32*)srcp;
    return length>>8;
}
