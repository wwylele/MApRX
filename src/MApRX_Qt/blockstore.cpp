/*************************************************************************
    blockstore.cpp
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

#include "blockstore.h"
#include <QPainter>
BlockStore::BlockStore(QWidget *parent) : QWidget(parent)
{
}

const int blockStoreColumnCount=16;

void BlockStore::paintEvent(QPaintEvent *){
    if(!pMainWindow->blocks.Loaded())return;
    QPainter painter(this);

    if(pMainWindow->showEssence){
        for(u32 i=0;i<pMainWindow->blocks.blockCount();i++){
            painter.drawPixmap((i%blockStoreColumnCount)*24,(i/blockStoreColumnCount)*24,
                    pMainWindow->essenceSheet,
                    (pMainWindow->blocks.Essences(i)%16)*24,
                    pMainWindow->blocks.Essences(i)/16*24,24,24);
        }
    }
    else{
        QImage image(width,height,QImage::Format_ARGB32);
        for(u32 i=0;i<pMainWindow->blocks.blockCount();i++){
            pMainWindow->blocks[i].draw([this,&image](int x,int y,const Color15& c15){
                u32 c=c15.toARGB32();
                image.setPixel(x,y,c);
            },pMainWindow->plt,(i%blockStoreColumnCount)*24,(i/blockStoreColumnCount)*24,pMainWindow->tiles);
        }

        painter.drawPixmap(0,0,QPixmap::fromImage(image));
    }
    QPen penCur,penSel;
    penCur.setColor(QColor(255,255,0));
    penCur.setWidth(2);
    penSel.setColor(QColor(255,0,0));
    penSel.setWidth(2);
    painter.setPen(penCur);
    if(curBlock!=-1)painter.drawRect((curBlock%blockStoreColumnCount)*24,
                     (curBlock/blockStoreColumnCount)*24,
                     24,24);
    painter.setPen(penSel);
    painter.drawRect((pMainWindow->selBlock%blockStoreColumnCount)*24,
                     (pMainWindow->selBlock/blockStoreColumnCount)*24,
                     24,24);


}

void BlockStore::reset(){
    width=blockStoreColumnCount*24;
    height=(pMainWindow->blocks.blockCount()/blockStoreColumnCount+1)*24;
    setMinimumSize(width,height);
    resize(width,height);
    curBlock=-1;
    pMainWindow->selBlock=0;
}
void BlockStore::mouseMoveEvent(QMouseEvent * event){
    if(event->x()>blockStoreColumnCount*24 || event->x()<0||event->y()<0){
        curBlock=-1;
    }
    else{
        int i=event->x()/24 + event->y()/24*blockStoreColumnCount;
        if(i<0 || (u32)i>pMainWindow->blocks.blockCount()){
            curBlock=-1;
        }
        else{
            curBlock=i;
        }
    }

    repaint();
}

void BlockStore::mousePressEvent(QMouseEvent* ){
    if(curBlock!=-1)pMainWindow->selBlock=curBlock;
    repaint();
}
void BlockStore::leaveEvent(QEvent * ){
    curBlock=-1;
    repaint();
}
