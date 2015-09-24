/*
 * NitroLz.cpp
 * This file is part of MApRX.
 * Taken from an unknown source code.
 *
 */

#include "NitroLz.h"
#include <string.h>

#define CX_LZ_COMPRESS_WORK_SIZE   ( (4096 + 256 + 256) * sizeof(s16) )

u8 lz77_work[CX_LZ_COMPRESS_WORK_SIZE];

typedef struct
{
    u16     windowPos;                 // Initial position of the history window.
    u16     windowLen;                 // Length of the history window.

    s16    *LZOffsetTable;             // Offset buffer of the history window.
    s16    *LZByteTable;               // Pointer to the most recent character history
    s16    *LZEndTable;                // Pointer to the oldest character history
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

        /* This isn't needed, but it seems to make it a little faster.*/
        if(*(searchp + 1) != *(nextp + 1) || *(searchp + 2) != *(nextp + 2))
        {
            w_offset=LZOffsetTable[w_offset];
            continue;
        }

        if(nextp - searchp < 2)
        {
            // VRAM is accessed in units of 2 bytes (since sometimes data is read from VRAM),
            // so the search must start 2 bytes prior to the search target.
            // 
            // Since the offset is stored in 12 bits, the value is 4096 or less
            break;
        }
        tmpLength=3;
        searchHeadp=searchp + 3;
        headp=nextp + 3;

        // Increments the compression size until the data ends or different data is encountered.
        while(((u32)(headp - nextp) < remainSize) && (*headp == *searchHeadp))
        {
            headp++;
            searchHeadp++;
            tmpLength++;

            // Since the data length is stored in 4 bits, the value is 18 or less (3 is added)
            if(tmpLength == (0xF + 3))
            {
                break;
            }
        }

        if(tmpLength > maxLength)
        {
            // Update the maximum-length offset
            maxLength=tmpLength;
            maxOffset=(u16)(nextp - searchp);
            if(maxLength == (0xF + 3))
            {
                // This is the largest matching length, so end search.
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


u32 CompressLZ(const u8 *srcp,u32 size,u8 *dstp)
{
    void* work=lz77_work;
    memset(work,0,CX_LZ_COMPRESS_WORK_SIZE);

    u32     LZDstCount;                // Number of bytes of compressed data
    u8      LZCompFlags;               // Flag series indicating whether there is a compression
    u8     *LZCompFlagsp;              // Point to memory regions storing LZCompFlags
    u16     lastOffset;                // Offset to matching data (the longest matching data at the time)
    u8      lastLength;                // Length of matching data (the longest matching data at the time)
    u8      i;
    u32     dstMax;
    LZCompressInfo info;               // Temporary LZ compression information

    if(size<=4)return 0;

    *(u32 *)dstp=size << 8 | 0x10;// CXiConvertEndian_( size << 8 | CX_COMPRESSION_LZ );  // data header
    dstp+=4;
    LZDstCount=4;
    dstMax=size;
    LZInitTable(&info,work);

    while(size > 0)
    {
        LZCompFlags=0;
        LZCompFlagsp=dstp++;         // Designation for storing flag series
        LZDstCount++;

        // Since flag series is stored as 8-bit data, loop eight times
        for(i=0; i < 8; i++)
        {
            LZCompFlags<<=1;         // No meaning for the first time (i=0)
            if(size <= 0)
            {
                // When reached the end, quit after shifting flag to the end. 
                continue;
            }

            if((lastLength=SearchLZ(&info,srcp,size,&lastOffset)) != 0)
            {
                // Enabled Flag if compression is possible 
                LZCompFlags|=0x1;

                if(LZDstCount + 2 >= dstMax)   // Quit on error if size becomes larger than source
                {
                    return 0;
                }
                // Divide offset into upper 4 bits and lower 8 bits and store
                *dstp++=(u8)((lastLength - 3) << 4 | (lastOffset - 1) >> 8);
                *dstp++=(u8)((lastOffset - 1) & 0xff);
                LZDstCount+=2;
                LZSlide(&info,srcp,lastLength);
                srcp+=lastLength;
                size-=lastLength;
            }
            else
            {
                // No compression
                /*if(LZDstCount + 1 >= dstMax)       // Quit on error if size becomes larger than source
                {
                    return 0;
                }*/ // KSSU: No, we don't need this! We just do it!
                LZSlide(&info,srcp,1);
                *dstp++=*srcp++;
                size--;
                LZDstCount++;
            }
        }                              // Complete eight loops
        *LZCompFlagsp=LZCompFlags;   // Store flag series
    }

    // Align to 4-byte boundary
    //   Does not include Data0 used for alignment as data size
    i=0;
    while((LZDstCount + i) & 0x3)
    {
        *dstp++=0;
        i++;
    }

    return LZDstCount;
}
void UncompressLZ(const  u8 *srcp,u8 *destp)
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
                    *pDst++=pDst[-offset];
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

u32 GetLengthLZ(const u8* srcp){
    u32 length=*(u32*)srcp;
    return length>>8;
}
