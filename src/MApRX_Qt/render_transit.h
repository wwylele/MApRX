/*************************************************************************
    render_transit.h
    :helper for rendering map

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

#ifndef RENDER_TRANSIT
#define RENDER_TRANSIT
#include <QPainter>
#include <QVector>
#include <QPixmap>
#include <QImage>
#include <memory>
class PltTransit:public QVector<QRgb>{
public:
    PltTransit():QVector<QRgb>(256){data()[0]=0x00000000;}
    void doTransit(const KfPlt& plt){
        for(int i=1;i<256;i++){
            data()[i]=plt.getColors(i).toARGB32();
        }
    }
};
class BlockSetTransit{
    std::unique_ptr<std::unique_ptr<QPixmap>[]> pImages;
    u32 size;
    void adjustSize(u32 blockCount){
        if(size!=blockCount){
            size=blockCount;
            pImages.reset(new std::unique_ptr<QPixmap>[size]);
            for(u32 i=0;i<size;i++){
                pImages[i].reset(new QPixmap(24,24));
            }
        }
    }
    void doTransitOne(KfBlockSet& blocks,KfTileSet& tiles,QVector<QRgb> colors,int id){
        pImages[id].get()->fill(Qt::transparent);
        QPainter painter;
        painter.begin(pImages[id].get());
        for(int j=0;j<9;j++){
            QImage tile(tiles[
                        blocks[id].data[j]&TILE_ID_MASK
                    ].data,8,8,QImage::Format_Indexed8);
            tile.setColorTable(colors);
            painter.drawImage((j%3)*8,j/3*8,
                              tile.mirrored
                              (blocks[id].data[j]&FLIP_X?true:false,
                               blocks[id].data[j]&FLIP_Y?true:false));


        }
        painter.end();
    }

public:
    BlockSetTransit():size(0){}
    QPixmap& operator[](int i){
        return *pImages[i].get();
    }

    void doTransit(KfBlockSet& blocks,KfTileSet& tiles,QVector<QRgb> colors){
        adjustSize(blocks.blockCount());
        for(u32 i=0;i<size;i++){
            for(int j=0;j<9;j++){
                if(tiles.getTickCounter()[blocks[i].data[j]&TILE_ID_MASK]){
                    doTransitOne(blocks,tiles,colors,i);
                    break;
                }
            }
        }
    }
    void doAllTransit(KfBlockSet& blocks,KfTileSet& tiles,QVector<QRgb> colors){
        adjustSize(blocks.blockCount());
        for(u32 i=0;i<size;i++){
            doTransitOne(blocks,tiles,colors,i);
        }
    }
};
#endif // RENDER_TRANSIT

