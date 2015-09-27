/*************************************************************************
    KssuFile.cpp
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


#include "KssuFile.h"
#include "NitroLz.h"
#include <assert.h>
#include <tuple>
const int MetaDataLength[7]={
    1,
    6,
    2,
    28,
    1,
    6,
    4

};


void KfPlt::readFile(const u8* src){
    src+=2;//skip field DataLength
    u8 threadCount=*(src++);
    memcpy(colors,src,512);src+=512;
    memcpy(finalColors,colors,512);

    threads.clear();
    threads.resize(threadCount);
    for(u8 thri=0;thri<threadCount;thri++){
        u8 frameCount;
        frameCount=*(src++);
        threads[thri].unk=frameCount&0x80?true:false;
        frameCount&=0x7F;
        threads[thri].frames.resize(frameCount);
        u8 commandLength;
        commandLength=*(src++);
        threads[thri].command.resize(commandLength);
        src+=2;//skip field totalColorCount
        memcpy(threads[thri].command.data(),src,commandLength);src+=commandLength;
        for(u8 frai=0;frai<frameCount;frai++){
            u8 colorCount;
            colorCount=*(src++);
            threads[thri].frames[frai].aniColors.resize(colorCount==0?256:colorCount);
        }
        for(u8 frai=0;frai<frameCount;frai++){
            threads[thri].frames[frai].colorId=*(src++);
        }
        for(u8 frai=0;frai<frameCount;frai++){
            memcpy(threads[thri].frames[frai].aniColors.data(),src,threads[thri].frames[frai].aniColors.size()*2);
            src+=threads[thri].frames[frai].aniColors.size()*2;
        }
        threads[thri].nextCommand=0;
        threads[thri].time=0;
        threads[thri].shiftState=Thread::NONE;
    }
}
void KfPlt::tick(){
    for(u32 thri=0;thri<threads.size();thri++){
        u8 frameId;
        while(!threads[thri].time){
            switch(threads[thri].command[threads[thri].nextCommand++]){
            case 0:
                threads[thri].nextCommand=0;
                break;
            case 1:
                threads[thri].time=threads[thri].command[threads[thri].nextCommand++];
                break;
            case 2:
                threads[thri].subLoopingCount=threads[thri].command[threads[thri].nextCommand++];
                threads[thri].subLoopingPos=threads[thri].nextCommand;
                break;
            case 3:
                threads[thri].subLoopingCount--;
                if(threads[thri].subLoopingCount){
                    threads[thri].nextCommand=threads[thri].subLoopingPos;
                }
                break;
            case 4:
                frameId=threads[thri].command[threads[thri].nextCommand++];
                memcpy(finalColors+threads[thri].frames[frameId].colorId,
                       threads[thri].frames[frameId].aniColors.data(),
                       threads[thri].frames[frameId].aniColors.size()*2);
                threads[thri].shiftState=Thread::NONE;
                break;
            case 5:
                threads[thri].mixFrameId=frameId=threads[thri].command[threads[thri].nextCommand++];
                threads[thri].beforeMixColor.resize(threads[thri].frames[frameId].aniColors.size());
                memcpy(threads[thri].beforeMixColor.data(),
                       finalColors+threads[thri].frames[frameId].colorId,
                       threads[thri].frames[frameId].aniColors.size()*2);
                /*memcpy(finalColors+threads[thri].frames[frameId].colorId,
                       threads[thri].frames[frameId].aniColors.data(),
                       threads[thri].frames[frameId].aniColors.size()*2);*/
                threads[thri].mixId=threads[thri].frames[frameId].colorId;
                threads[thri].denom=threads[thri].numer=threads[thri].command[threads[thri].nextCommand++];
                break;
            case 6:
                frameId=threads[thri].command[threads[thri].nextCommand++];
                threads[thri].shiftTime=threads[thri].shiftPeriod=threads[thri].command[threads[thri].nextCommand++];
                threads[thri].shiftState=Thread::LEFT;
                threads[thri].shiftId=threads[thri].frames[frameId].colorId;
                threads[thri].shiftCount=threads[thri].frames[frameId].aniColors.size();
                break;
            case 7:
                frameId=threads[thri].command[threads[thri].nextCommand++];
                threads[thri].shiftTime=threads[thri].shiftPeriod=threads[thri].command[threads[thri].nextCommand++];
                threads[thri].shiftState=Thread::RIGHT;
                threads[thri].shiftId=threads[thri].frames[frameId].colorId;
                threads[thri].shiftCount=threads[thri].frames[frameId].aniColors.size();
                break;
            default:
                assert(0);
            }
        }
        threads[thri].time--;
        if(threads[thri].numer){
            threads[thri].numer--;
            for(u32 i=0;i<threads[thri].beforeMixColor.size();i++){
                finalColors[i+threads[thri].mixId]=
                        Color15::lerp(threads[thri].frames[threads[thri].mixFrameId].aniColors[i],
                        threads[thri].beforeMixColor[i],(float)threads[thri].numer/threads[thri].denom);
            }
        }
        if(threads[thri].shiftState!=Thread::NONE){
            threads[thri].shiftTime--;
            if(threads[thri].shiftTime==0){
                threads[thri].shiftTime=threads[thri].shiftPeriod;
                if(threads[thri].shiftState==Thread::LEFT){
                    Color15 temp;
                    temp=finalColors[threads[thri].shiftId];
                    memmove(finalColors+threads[thri].shiftId,
                            finalColors+threads[thri].shiftId+1,
                            (threads[thri].shiftCount-1)*2);
                    finalColors[threads[thri].shiftId+threads[thri].shiftCount-1]=temp;
                }
                else{
                    Color15 temp;
                    temp=finalColors[threads[thri].shiftId+threads[thri].shiftCount-1];
                    memmove(finalColors+threads[thri].shiftId+1,
                            finalColors+threads[thri].shiftId,
                            (threads[thri].shiftCount-1)*2);
                    finalColors[threads[thri].shiftId]=temp;
                }
            }
        }
    }
}
void KfPlt::loadDefault(){
    threads.clear();
    memset(colors,0,512);//ZeroMemory(colors,512);
    colors[1]=Color15(0,0,0);
    colors[2]=Color15(32,32,32);
    memcpy(finalColors,colors,512);
}

void KfTileSet::readFile(const u8* src){
    u32 compressedDataLength;
    memcpy(&compressedDataLength,src,4);src+=4;
    u16 tileCount;
    memcpy(&tileCount,src,2);src+=2;
    u8 compressed=*(src++);
    u8 threadCount=*(src++);
    tiles.resize(tileCount);
    if(compressed){
        uncompressLZ(src,(u8*)tiles.data());
        src+=compressedDataLength;
    }
    else{
        memcpy(tiles.data(),src,tileCount*64);src+=tileCount*64;
    }
    threads.clear();
    aniTiles.clear();
    finalTiles=tiles;
    if(threadCount==0)return;

    const u8 *p=src;
    threads.resize(threadCount);
    u32 aniTilesDataLength;
    memcpy(&aniTilesDataLength,p,4);p+=4;
    aniTiles.resize(aniTilesDataLength/64);
    u16 idInAniTiles=0;
    for(u32 thri=0;thri<threads.size();thri++){
        u8 frameCount;
        u8 commandLength;
        frameCount=*(p++);
        commandLength=*(p++);
        threads[thri].frames.resize(frameCount);
        threads[thri].command.resize(commandLength);
        memcpy(threads[thri].command.data(),p,commandLength);p+=commandLength;
        for(int frai=0;frai<frameCount;frai++){
            memcpy(&threads[thri].frames[frai].tileCount,p,2);p+=2;

        }
        for(int frai=0;frai<frameCount;frai++){
            memcpy(&threads[thri].frames[frai].tileIdInTiles,p,2);p+=2;
            threads[thri].frames[frai].tileIdInAniTiles=idInAniTiles;
            idInAniTiles+=threads[thri].frames[frai].tileCount;
        }
        threads[thri].time=0;
        threads[thri].nextCommand=0;
    }
    u16 partitionLength;
    u32 total=0,t;
    u8* q=(u8*)aniTiles.data();
    while(1){
        memcpy(&partitionLength,p,2);p+=2;
        if(!partitionLength)break;
        t=getLengthLZ(p);
        uncompressLZ(p,q);
        q+=t;
        total+=t;
        p+=partitionLength;

    }
    assert(total==aniTilesDataLength);


}

void KfTileSet::tick(){
    for(u32 thri=0;thri<threads.size();thri++){

        while(!threads[thri].time){
            switch(threads[thri].command[threads[thri].nextCommand++]){
            case 0:
                threads[thri].nextCommand=0;
                break;
            case 1:
                threads[thri].time=threads[thri].command[threads[thri].nextCommand++];
                break;
            case 2:
                threads[thri].subLoopingCount=threads[thri].command[threads[thri].nextCommand++];
                threads[thri].subLoopingPos=threads[thri].nextCommand;
                break;
            case 3:
                threads[thri].subLoopingCount--;
                if(threads[thri].subLoopingCount){
                    threads[thri].nextCommand=threads[thri].subLoopingPos;
                }
                break;
            case 4:{
                u8 frameId;
                frameId=threads[thri].command[threads[thri].nextCommand++];
                memcpy(finalTiles.data()+threads[thri].frames[frameId].tileIdInTiles,
                       aniTiles.data()+threads[thri].frames[frameId].tileIdInAniTiles,
                       threads[thri].frames[frameId].tileCount*64);
                break;
            }
            default:
                assert(0);
            }
        }
        threads[thri].time--;
    }
}
void KfBlockSet::loadDefault(){
    const u16 count=4096;
    blocks.resize(count);
    essences.resize(count,0);
    for(u16 i=0;i<count;i++){
        memset(blocks[i].data,0,18);//ZeroMemory(blocks[i].data,18);
        blocks[i].data[4].tileId=(i>>12)|0x10;
        blocks[i].data[5].tileId=((i>>8)&0xF)|0x10;
        blocks[i].data[7].tileId=((i>>4)&0xF)|0x10;
        blocks[i].data[8].tileId=(i&0xF)|0x10;
    }
    loaded=true;
}

void KfBlockSet::readFile(const u8* src){
    src+=2;//field DataLength
    u16 blockCount;
    memcpy(&blockCount,src,2);src+=2;
    blocks.resize(blockCount);
    essences.resize(blockCount);
    std::unique_ptr<u8[]> buf(new u8[getLengthLZ(src)]);
    uncompressLZ(src,buf.get());
    memcpy(blocks.data(),buf.get(),blocks.size()*sizeof(Block));
    memcpy(essences.data(),buf.get()+blocks.size()*sizeof(Block),blocks.size());
    loaded=true;
}

u32 KfMap::getScripteLength(u8 *pScript){
    s16 temp;
    switch(*pScript){
    case 1:case 6:
        return 4+2*pScript[3];break;
    case 2:
        return 7;break;
    case 3:
        return 9;break;
    case 4:
        return 4;break;
    case 5:
        memcpy(&temp,pScript+3,2);
        return temp>=0?7:5;
        break;
    case 0:return 1;//eod
    default:;assert(0);return 0;
    }
}
void KfMap::readFile(const u8* src){
    src+=2;//field DataLength
    
    memcpy(&metaData,src,28);
    cells.clear();
    items.clear();
    cells.resize(metaData.width*metaData.height);
    items.resize(metaData.itemCount);
    src+=28;

    u32 length=getLengthLZ(src);
    u8* p;
    std::unique_ptr<u8[]> buf(p=new u8[length]);
    uncompressLZ(src,buf.get());
    for(u16 i=0;i<metaData.width*metaData.height;i++){
        memcpy(&cells[i].blockId,p,2);p+=2;
        cells[i].blockId&=0x7FFF;//Erase the field hasScript
    }
    length-=metaData.width*metaData.height*2;

    for(u8 i=0;i<metaData.itemCount;i++){
        memcpy(&items[i].basic,p,8);p+=8;
    }
    length-=metaData.itemCount*8;

    std::unique_ptr<u8[]> rawScripts;
    u32 rawScriptsLength;
    rawScripts.reset(new u8[rawScriptsLength=length]);
    memcpy(rawScripts.get(),p,rawScriptsLength);

    p=rawScripts.get();
    u16 tid=0,ntid;
    int len;
    std::vector<Script>*pScriptList;
    u16 CCCC=0xCCCC;
    while(*p){
        assert(p<rawScripts.get()+rawScriptsLength);
        memcpy(&ntid,p+1,2);
        assert(ntid>=tid);
        if(ntid<0x8000){
            pScriptList=&cells[ntid].scripts;
        }
        else{
            pScriptList=&items[ntid&0x7FFF].scripts;
        }
        memcpy(p+1,&CCCC,2);//Erase field hostID
        tid=ntid;
        len=getScripteLength(p);
        pScriptList->emplace_back(len);
        memcpy(pScriptList->back().data(),p,len);
        p+=len;
    }
    assert(p-rawScripts.get()==rawScriptsLength-1);
    loaded=true;

#ifdef _DEBUG
    for(u8 i=0;i<metaData.itemCount;i++){
        assert(!(Items(i).basic.behavior&0x30));
    }
#endif

}


u8* KfMap::generateFile(u32 *length){
    assert(loaded);
    assert(metaData.itemCount==items.size());

    u32 scriptsLen=1,dataLen;
    for(RipeCell& cell:cells){
        for(Script& s:cell.scripts)scriptsLen+=getScripteLength(s.data());
    }
    for(RipeItem& item:items){
        for(Script& s:item.scripts)scriptsLen+=getScripteLength(s.data());
    }
    std::unique_ptr<u8[]> buf(new u8[dataLen=
        metaData.width*metaData.height*2+
        metaData.itemCount*8+
        scriptsLen]);

    u8* p=buf.get();

    std::vector<Script> scripts;
    u32 slen;

    for(u16 i=0;i<metaData.width*metaData.height;i++){
        Cell rawCell;
        rawCell.blockId=cells[i].blockId;
        rawCell.hasScript=cells[i].scripts.empty()?0:1;
        memcpy(p,&rawCell,2);p+=2;
        for(u32 j=0;j<cells[i].scripts.size();j++){
            slen=getScripteLength(cells[i].scripts[j].data());
            scripts.emplace_back(slen);
            memcpy(scripts.back().data(),cells[i].scripts[j].data(),slen);
            memcpy(scripts.back().data()+1,&i,2);//fill hostID
        }
    }
    for(u8 i=0;i<items.size();i++){
        items[i].basic.hasScript=items[i].scripts.empty()?0:1;//Not sure
        memcpy(p,&items[i].basic,8);p+=8;
        for(u32 j=0;j<items[i].scripts.size();j++){
            slen=getScripteLength(items[i].scripts[j].data());
            scripts.emplace_back(slen);
            memcpy(scripts.back().data(),items[i].scripts[j].data(),slen);
            u16 hostID;
            hostID=i|0x8000;
            memcpy(scripts.back().data()+1,&hostID,2);//fill hostID
        }
    }

    for(u32 i=0;i<scripts.size();i++){
        
        slen=getScripteLength(scripts[i].data());
        memcpy(p,scripts[i].data(),slen);
        p+=slen;
    }
    *p=0;

    u8* pAlloc=new u8[(2+MetaDataLength[3]+dataLen)*2];
    memcpy(pAlloc+2,&metaData,MetaDataLength[3]);
    dataLen=compressLZ(buf.get(),dataLen,pAlloc+2+MetaDataLength[3]);
    if(!dataLen){
        fprintf(stderr,"CompressLZ-FAILED\n");
        delete pAlloc;return 0;
    }
    *(u16*)pAlloc=(u16)dataLen;
    if(length)*length=2+MetaDataLength[3]+dataLen;
    return pAlloc;
}

void KfMap::unload(){
    cells.clear();
    items.clear();
    loaded=false;
}


void KfMap::swapItem(u8 firstItemId){
    assert(firstItemId<items.size()-1);
    std::swap(items[firstItemId],items[firstItemId+1]);
    forEachItemReferenceInScripts([firstItemId](u8& itemId){
        if(itemId==firstItemId){
            itemId++;
        }else if(itemId==firstItemId+1){
            itemId--;
        }
    });
}

void KfMap::deleteItem(u8 itemId){
    assert(itemId<items.size());
    items.erase(items.begin()+itemId);
    metaData.itemCount--;
    forEachItemReferenceInScripts([itemId](u8& itemIdR){
        if(itemIdR==itemId){
            itemIdR=0;
        }else if(itemIdR>itemId){
            itemIdR--;
        }
    });
}

void KfMap::newItem(u8 before_itemId,const RipeItem& item){
    assert(items.size()<256);
    assert(before_itemId<=items.size());

    forEachItemReferenceInScripts([before_itemId](u8& itemIdR){
        if(itemIdR>=before_itemId){
            itemIdR++;
        }
    });
    items.insert(items.begin()+before_itemId,item);

    metaData.itemCount++;
}

void KfBckScr::readFile(const u8 *src){
    src+=2;
    memcpy(&width,src,2);src+=2;
    memcpy(&height,src,2);src+=2;
    chars.resize(width*height);
    uncompressLZ(src,(u8*)chars.data());
    loaded=true;
}

template<typename T> u32 GetIdInVector(std::vector<T>& vector,const T& item_to_find){
    for(size_t i=0;i<vector.size();i++){
        if(vector[i]==item_to_find)return i;
    }
    vector.push_back(item_to_find);
    return vector.size()-1;
}

const u32 RoomInfo::invalidId=0xFFFFFFFFUL;
void Kf_mapdata::fromFile(FILE* file){
    std::vector<u16> subFileShortAddr[7];
    u16 shortAddr;
    fseek(file,0,SEEK_SET);
    for(int i=0;i<MAP_COUNT;i++){
        for(int j=0;j<7;j++){
            fread(&shortAddr,2,1,file);
            if(shortAddr){
                roomInfos[i].subFileId[j]=GetIdInVector(subFileShortAddr[j],shortAddr);
            }
            else{
                roomInfos[i].subFileId[j]=RoomInfo::invalidId;
            }
        }
    }
    u32 longAddr;
    u32 length; 
    for(int j=0;j<7;j++){
        rawSubFiles[j]=std::vector<rawFile>(subFileShortAddr[j].size());
        for(size_t i=0;i<subFileShortAddr[j].size();i++){
            fseek(file,subFileShortAddr[j][i],SEEK_SET);
            fread(&longAddr,4,1,file);
            fseek(file,longAddr,SEEK_SET);
            length=0;
            fread(&length,2,1,file);
            if(length==0)length=0x10000;//for some non-compressed bcktileset file
            length+=MetaDataLength[j]+2;

            //Special treatment for palette
            if(j==0 || j==4){
                u8 AnimationThreadCount;
                fread(&AnimationThreadCount,1,1,file);
                fseek(file,0x200,SEEK_CUR);
                for(int t=0;t<AnimationThreadCount;t++){
                    u8 FrameCount;
                    u8 CommandLength;
                    u16 ColorCount;
                    fread(&FrameCount,1,1,file);FrameCount&=0x7F;length+=1;
                    fread(&CommandLength,1,1,file);length+=1;
                    fread(&ColorCount,2,1,file);length+=2;
                    fseek(file,CommandLength+FrameCount*2+ColorCount*2,SEEK_CUR);
                    length+=CommandLength+FrameCount*2+ColorCount*2;
                }
            }

            //Special treatment for tileset
            if(j==1 || j==5){
                u8 AnimationThreadCount;
                fseek(file,5,SEEK_CUR);
                fread(&AnimationThreadCount,1,1,file);
                if(AnimationThreadCount){
                    fseek(file,length-8,SEEK_CUR);
                    fseek(file,4,SEEK_CUR);length+=4;
                    for(int t=0;t<AnimationThreadCount;t++){
                        u8 FrameCount;
                        u8 CommandLength;
                        fread(&FrameCount,1,1,file);length+=1;
                        fread(&CommandLength,1,1,file);length+=1;
                        fseek(file,CommandLength,SEEK_CUR);length+=CommandLength;
                        fseek(file,FrameCount*4,SEEK_CUR);length+=FrameCount*4;
                    }
                    while(1){
                        u16 partitionLength;
                        fread(&partitionLength,2,1,file);length+=2;
                        if(!partitionLength)break;
                        fseek(file,partitionLength,SEEK_CUR);length+=partitionLength;
                    }
                }
            }
            rawSubFiles[j][i].length=length;
            rawSubFiles[j][i].ptr.reset(new u8[length]);
            fseek(file,longAddr,SEEK_SET);
            fread(rawSubFiles[j][i].ptr.get(),length,1,file);
        }
    }

}
void Kf_mapdata::toFile(FILE* file){
    u16 subFileAddrSlots[MAP_COUNT][7];
    using idPair=std::tuple<u32/*0~6*/,u32/*id*/>;
    std::vector<idPair> idList;
    for(u32 i=0;i<MAP_COUNT;i++){
        for(u32 j=0;j<7;j++){
            if(roomInfos[i].subFileId[j]==RoomInfo::invalidId){
                subFileAddrSlots[i][j]=0;
            }
            else{
                u32 index;
                index=GetIdInVector(idList,idPair(j,roomInfos[i].subFileId[j]));
                subFileAddrSlots[i][j]=(u16)(index*4+MAP_COUNT*7*2);
            }
        }
    }

    fseek(file,0,SEEK_SET);
    fwrite(subFileAddrSlots,2,MAP_COUNT*7,file);

    long tableP=ftell(file);
    long allocP=tableP+idList.size()*4;
    long len;
    u8* p;
    for(u32 i=0;i<idList.size();i++){
        fseek(file,tableP,SEEK_SET);
        fwrite(&allocP,4,1,file);tableP+=4;
        fseek(file,allocP,SEEK_SET);
        auto rawf=&rawSubFiles[std::get<0>(idList[i])][std::get<1>(idList[i])];
        p=rawf->ptr.get();
        len=rawf->length;
        fwrite(p,1,len,file);allocP+=len;
    }
}
