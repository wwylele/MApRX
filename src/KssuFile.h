/*************************************************************************
    KssuFile.h
    :Classes for reading/editing KSSU's map data.

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

#ifndef _KSSUFILE_H_
#define _KSSUFILE_H_
#include "Nitro.h"
#include <vector>
#include <memory>
#include <cstring>

extern const int MetaDataLength[7];

typedef std::vector<u8> Command;

//#0,#4
class KfPlt{
protected:
    Color15 colors[256];
    Color15 finalColors[256];
    struct Thread{
        Command command;
        bool unk;
        struct Frame{
            u8 colorId;
            std::vector<Color15> aniColors;

        };
        std::vector<Frame> frames;
        //Ticking
        u16 nextCommand;
        //for command 1
        u8 time;
        //for command 2,3
        u8 subLoopingCount;
        u16 subLoopingPos;
        //for command 5
        u8 numer;
        u8 denom;
        std::vector<Color15> beforeMixColor;
        u8 mixFrameId;
        u8 mixId;
        //for command 6,7
        enum{NONE,LEFT,RIGHT} shiftState;
        u8 shiftId;
        u8 shiftCount;
        u8 shiftPeriod;
        u8 shiftTime;
    };
    std::vector<Thread> threads;
    u8 tickCounter;
public:
    Color15 getColors(u8 i)const{
        return finalColors[i];
    }
    void tick();
    u8 getTickCounter(){ return tickCounter; }
    void clearTickCounter(){ tickCounter=0; }
    void readFile(const u8* src);
    void loadDefault();
};

//#1,#5
class KfTileSet{
protected:
    std::vector<Tile8bpp> tiles;
    std::vector<Tile8bpp> aniTiles;
    std::vector<Tile8bpp> finalTiles;
    struct Thread{
        Command command;
        struct Frame{
            u16 tileCount;
            u16 tileIdInTiles;
            u16 tileIdInAniTiles;
        };
        std::vector<Frame> frames;
        //Ticking
        u16 nextCommand;
        //for command 1
        u8 time;
        //for command 2,3
        u8 subLoopingCount;
        u16 subLoopingPos;
    };
    std::vector<Thread> threads;
    std::vector<u8> tickCounter;
    u8 totalTickCounter;

public:
    static Tile8bpp invalidTile;
    void readFile(const u8* src);
    const Tile8bpp& operator [](u16 tileId)const{
        if(tileId>=finalTiles.size())return invalidTile;
        return finalTiles[tileId];
    }
    void tick();
    u8* getTickCounter(){ return tickCounter.data(); }
    u8 getTotalTickCounter(){ return totalTickCounter; }
    void clearTickCounter(){
        memset(tickCounter.data(),0,tickCounter.size());
        totalTickCounter=0;
    }
};

struct Block{
    CharData data[9];
    const CharData& tileAt(u8 x/*0~2*/,u8 y/*0~2*/)const{
        return data[x+y*3];
    }
};
typedef u8 BlockEssence;

//#2
class KfBlockSet{

protected:
    std::vector<Block> blocks;
    std::vector<BlockEssence> essences;
    bool loaded=false;
public:
    static Block invalidBlock;
    void readFile(const u8* src);
    void loadDefault();
    const Block& operator [](u16 blockId)const{
        if(blockId>=blocks.size())return invalidBlock;
        return blocks[blockId];
    }
    BlockEssence getEssences(u16 blockId)const{
        return essences[blockId];
    }

    u32 blockCount()const{
        return blocks.size();
    }
    bool isLoaded(){
        return loaded;
    }
    
};


//#3
class KfMap{
public:
    typedef std::vector<u8> Script;
    typedef u16 Cell;
    assert_size(Cell,2);
    enum CellFlags{
        BLOCK_ID_MASK=0x7FFF,
        CELL_HAS_SCRIPT=0x8000
    };

    struct RipeCell{
        u16 blockId;
        std::vector<Script> scripts;
        RipeCell():blockId(0),scripts(){}
    };

    struct Item{
        u16 param0;
        u16 param1;
        u16 x;
        u16 y;
        enum Param0Flags{
            SPECIES_MASK=0x00FF,
            BEHAVIOR_MASK=0x3F00,
            FLAG_A=0x4000,
            FLAG_B=0x8000
        };
        enum Param1Flags{
            PARAM_MASK=0x00FF,
            CATAGORY_MASK=0x7F00,
            HAS_SCRIPT=0x8000
        };
        Item():param0(1),param1(0x200),x(24),y(24){}

        u8 species(){
            return param0&SPECIES_MASK;
        }
        void setSpecies(u8 v){
            param0&=~SPECIES_MASK;
            param0|=v;
        }
        u8 param(){
            return param1&PARAM_MASK;
        }
        void setParam(u8 v){
            param1&=~PARAM_MASK;
            param1|=v;
        }

        u8 behavior(){
            return (param0&BEHAVIOR_MASK)>>8;
        }
        void setBehavior(u8 v){
            param0&=~BEHAVIOR_MASK;
            param0|=v<<8;
        }
        u8 catagory(){
            return (param1&CATAGORY_MASK)>>8;
        }
        void setCatagory(u8 v){
            param1&=~CATAGORY_MASK;
            param1|=v<<8;
        }

        bool flagA(){
            return (param0&FLAG_A) !=0;
        }
        void setFlagA(bool v){
            if(v)param0|=FLAG_A;
            else param0&=~FLAG_A;
        }
        bool flagB(){
            return (param0&FLAG_B) !=0;
        }
        void setFlagB(bool v){
            if(v)param0|=FLAG_B;
            else param0&=~FLAG_B;
        }
    };
    assert_size(Item,8);

    struct RipeItem{
        Item basic;
        std::vector<Script> scripts;
        RipeItem():basic(),scripts(){
        }
    };

protected:
    bool loaded=false;
    std::vector<RipeCell> cells;
    std::vector<RipeItem> items;
public:
    bool isLoaded(){return loaded;}
    void unload();

    void readFile(const u8* src);

    u8* generateFile(u32 *length);

    struct MetaData_Struct{
        u16 width; // Should not access directly
        u16 height; // Should not access directly
        u16 clipTop;
        u16 clipBottom;
        u16 clipLeft;
        u16 clipRight;
        s16 bckOffsetHori;
        s16 bckOffsetVert;
        s16 bckSpeedHori;
        s16 bckSpeedVert;
        u8 globalEffect;
        u8 bgm;
        u8 itemCount; // Should not access directly
        u8 itemPlts[5];
    }metaData;
    assert_size(MetaData_Struct,28);

    
    /*struct Script_{
        u32 code:8;
        u32 hostId:15;
        u32 hostIsAItem:1;
        u32 data0:8;
        u8 data[1];
    };*/

    static u32 getScripteLength(u8 *pScript);

    RipeCell& cellAt(u16 x,u16 y){
        return cells[x+y*metaData.width];
    }
    RipeItem& itemAt(u8 i){
        return items[i];
    }
    u16 getWidth(){ return metaData.width; }
    u16 getHeight(){ return metaData.height; }
    u8 getItemCount(){ return metaData.itemCount; }

    template <class T/* void (*)(u8& itemIdReference) */>
    void forEachItemReferenceInScripts(T doWhat){
        auto fetrisl=[&doWhat](std::vector<Script> &scripts){
            for(Script& script:scripts){
                if(script[0]==4){
                    doWhat(script[3]);
                }
                //no I think this is my fault
                /*else if(script[0]==6){
                    u8 size;
                    size=script[3];
                    for(int i=0;i<size;i++){
                        doWhat(script[4+i*2]);
                    }
                }*/

            }
        };
        for(RipeCell & cell:cells){
            fetrisl(cell.scripts);
        }
        for(RipeItem & item:items){
            fetrisl(item.scripts);
        }
    }

    template <class T/* void (*)(u16& cellXReference,u16& cellYReference) */>
    void forEachCellReferenceInScripts(T doWhat){
        auto f=[&doWhat](std::vector<Script> &scripts){
            for(Script& script:scripts){
                if(script[0]==2){
                    u16 x,y;
                    std::memcpy(&x,script.data()+2,2);
                    std::memcpy(&y,script.data()+4,2);
                    doWhat(x,y);
                    std::memcpy(script.data()+2,&x,2);
                    std::memcpy(script.data()+4,&y,2);
                }
            }
        };
        for(RipeCell & cell:cells){
            f(cell.scripts);
        }
        for(RipeItem & item:items){
            f(item.scripts);
        }
    }

    void swapItem(u8 firstItemId);
    void deleteItem(u8 itemId);
    void newItem(u8 before_itemId,const RipeItem& item);

    void resizeMap(u16 width,u16 height,
                   int x0,int y0//Where to put old cell(0,0) on new map
                   );
    enum Align{
        BEGIN=-1,
        CENTER=0,
        END=1
    };

    void resizeMap(u16 width,u16 height,Align hAlign,Align vAlign);

    static u8 itemCatagory[256/*species*/];
};

//#6
class KfBckScr{
protected:
    u16 width;
    u16 height;
    std::vector<CharData> chars;
    bool loaded=false;
public:
    void readFile(const u8* src);
    bool isLoaded(){return loaded;}
    void unload(){
        loaded=false;
        chars.clear();
    }
    CharData at(u16 x,u16 y){
        return chars[x+y*width];
    }


    u16 getWidth(){ return width; }
    u16 getHeight(){ return height; }
};

#define MAP_COUNT 548

struct RoomInfo{
    union{
        u32 subFileId[7];
        struct SUB_FILE_ID_SLOTS{
            u32 rawFrtPltId;
            u32 rawFrtTileSetId;
            u32 rawFrtBlockSetId;
            u32 rawMapId;//should be always equal to map index
            u32 rawBckPltId;
            u32 rawBckTileSetId;
            u32 rawBckScrId;
        }subFileIdSlots;
    }subFileIdData;
    assert_size(subFileIdData,28);
    static const u32 invalidId;
};
class Kf_mapdata{
private:
    struct rawFile{
        std::unique_ptr<u8[]> ptr;
        u32 length;
        rawFile(){}
        rawFile(rawFile&& c):length(c.length){
            ptr.reset(c.ptr.release());
        }
    };
public:
    std::vector<rawFile> rawSubFiles[7];
public:
    RoomInfo roomInfos[MAP_COUNT];
    
    void fromFile(std::FILE* file);
    void toFile(std::FILE* file);

    u8* rawFrtPlts(u32 i){
        return rawSubFiles[0][i].ptr.get();
    }
    u8* rawFrtTileSets(u32 i){
        return rawSubFiles[1][i].ptr.get();
    }
    u8* rawFrtBlockSets(u32 i){
        return rawSubFiles[2][i].ptr.get();
    }
    u8* rawMaps(u32 i){
        return rawSubFiles[3][i].ptr.get();
    }
    u8* rawBckPlts(u32 i){
        return rawSubFiles[4][i].ptr.get();
    }
    u8* rawBckTileSets(u32 i){
        return rawSubFiles[5][i].ptr.get();
    }
    u8* rawBckScrs(u32 i){
        return rawSubFiles[6][i].ptr.get();
    }

    void writeMap(u32 i,const u8* p,u32 len){
        rawSubFiles[3][i].ptr.reset(new u8[len]);
        rawSubFiles[3][i].length=len;
        std::memcpy(rawSubFiles[3][i].ptr.get(),p,len);
    }
};


#endif
