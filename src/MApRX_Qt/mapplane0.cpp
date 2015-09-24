/*************************************************************************
    mapplane0.cpp
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

#include "mapplane0.h"
#include <QPainter>
#include <QImage>
#include <QMouseEvent>
MapPlane0::MapPlane0(QWidget *parent) : QWidget(parent)
{
}



void MapPlane0::paintEvent(QPaintEvent *){
    if(!pMainWindow->map.Loaded())return;
    QPainter painter(this);
    if(pMainWindow->showEssence){
        for(u32 x=0;x<pMainWindow->map.GetWidth();x++)
            for(u32 y=0;y<pMainWindow->map.GetHeight();y++){
                BlockEssence e;
                e=pMainWindow->blocks.Essences(pMainWindow->map.At(x,y).blockId);
                painter.drawPixmap(x*24,y*24,pMainWindow->essenceSheet,
                                   (e%16)*24,e/16*24,24,24);
            }
    }
    else{
        QImage image(width,height,QImage::Format_ARGB32);
        if(pMainWindow->showItems){
            pMainWindow->map.Draw([this,&image](int x,int y,const Color15& c15){
                u32 c=c15.ToGrey32();
                image.setPixel(x,y,c);
            },pMainWindow->plt,0,0,pMainWindow->blocks,pMainWindow->tiles);
        }
        else{
            if(pMainWindow->bckScr.Loaded())pMainWindow->bckScr.Draw([this,&image](int x,int y,const Color15& c15){
                u32 c=c15.ToARGB32();
                for(;x<width;x+=pMainWindow->bckScr.GetWidth()*8)for(;y<height;y+=pMainWindow->bckScr.GetHeight()*8)
                    image.setPixel(x,y,c);
            },pMainWindow->bckPlt,0,0,pMainWindow->bckTiles);
            pMainWindow->map.Draw([this,&image](int x,int y,const Color15& c15){
                u32 c=c15.ToARGB32();
                image.setPixel(x,y,c);
            },pMainWindow->plt,0,0,pMainWindow->blocks,pMainWindow->tiles);
        }



        painter.drawPixmap(0,0,QPixmap::fromImage(image));


    }
    if(pMainWindow->showScript){
        if(!pMainWindow->showItems){
            QBrush brushScript(QColor(255,0,255,100),Qt::SolidPattern);
            for(u32 x=0;x<pMainWindow->map.GetWidth();x++)
                for(u32 y=0;y<pMainWindow->map.GetHeight();y++){
                    if(!pMainWindow->map.At(x,y).scripts.empty()){
                        painter.fillRect(x*24,y*24,24,24,brushScript);
                    }
                }
        }

    }
    if(pMainWindow->showItems){
        QPen itemPen;
        itemPen.setColor(QColor(255,255,0));
        itemPen.setWidth(3);
        painter.setPen(itemPen);
        for(u32 i=0;i<pMainWindow->map.metaData.itemCount;i++){
            QString str;
            str.sprintf("%d",i);
            painter.drawEllipse(pMainWindow->map.Items(i).basic.x-8,
                                pMainWindow->map.Items(i).basic.y-8,
                                16,16);
            painter.drawText(pMainWindow->map.Items(i).basic.x-8,
                             pMainWindow->map.Items(i).basic.y-8,
                             16,16,Qt::AlignCenter,str);
        }
    }else{
        if(curX!=-1){
            QPen penCur;
            penCur.setColor(QColor(255,255,0));
            penCur.setWidth(2);
            painter.setPen(penCur);
            painter.drawRect(curX*24,curY*24,
                             24,24);
        }
    }


}

void MapPlane0::reset(){
    width=pMainWindow->map.metaData.width*24;
    height=pMainWindow->map.metaData.height*24;
    setMinimumSize(width,height);
    resize(width,height);

}

void MapPlane0::mouseMoveEvent(QMouseEvent * event){
    if(!pMainWindow->showItems){
        if(event->x()>pMainWindow->map.metaData.width*24||
           event->y()>pMainWindow->map.metaData.height*24||
                event->x()<0||event->y()<0){
            curX=curY=-1;
        }
        else{
            curX=event->x()/24;
            curY=event->y()/24;
        }
    }


    repaint();
}

void MapPlane0::mousePressEvent(QMouseEvent* ){
    if(!pMainWindow->showItems){
        if(curX!=-1){
            if(!pMainWindow->showScript){
                MainWindow::MoEditCell editCell(curX,curY,pMainWindow->selBlock);
                pMainWindow->doOperation(&editCell);
            }
        }
    }
}
void MapPlane0::leaveEvent(QEvent * ){
    curX=curY=-1;
    repaint();
}
