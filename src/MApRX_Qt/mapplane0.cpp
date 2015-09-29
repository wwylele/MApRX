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
#include "dialogscripts.h"
MapPlane0::MapPlane0(QWidget *parent) : QWidget(parent)
{
}



void MapPlane0::paintEvent(QPaintEvent *){
    if(!pMainWindow->map.Loaded())return;
    QPainter painter(this);
    if(pMainWindow->showEssence){
        for(u32 x=0;x<pMainWindow->map.getWidth();x++)
            for(u32 y=0;y<pMainWindow->map.getHeight();y++){
                BlockEssence e;
                e=pMainWindow->blocks.Essences(pMainWindow->map.at(x,y).blockId);
                painter.drawPixmap(x*24,y*24,pMainWindow->essenceSheet,
                                   (e%16)*24,e/16*24,24,24);
            }
    }
    else{
        QImage image(width,height,QImage::Format_ARGB32);
        if(pMainWindow->showItems){
            pMainWindow->map.draw([this,&image](int x,int y,const Color15& c15){
                u32 c=c15.toGray32();
                image.setPixel(x,y,c);
            },pMainWindow->plt,0,0,pMainWindow->blocks,pMainWindow->tiles);
        }
        else{
            if(pMainWindow->showBackground &&pMainWindow->bckScr.Loaded())
                pMainWindow->bckScr.draw([this,&image](int x,int y,const Color15& c15){
                u32 c=c15.toARGB32();
                for(;x<width;x+=pMainWindow->bckScr.getWidth()*8)for(;y<height;y+=pMainWindow->bckScr.getHeight()*8)
                    image.setPixel(x,y,c);
            },pMainWindow->bckPlt,0,0,pMainWindow->bckTiles);
            pMainWindow->map.draw([this,&image](int x,int y,const Color15& c15){
                u32 c=c15.toARGB32();
                image.setPixel(x,y,c);
            },pMainWindow->plt,0,0,pMainWindow->blocks,pMainWindow->tiles);
        }



        painter.drawPixmap(0,0,QPixmap::fromImage(image));


    }
    if(pMainWindow->showScript){
        if(!pMainWindow->showItems){
            QBrush brushScript(QColor(255,0,255,100),Qt::SolidPattern);
            for(u32 x=0;x<pMainWindow->map.getWidth();x++)
                for(u32 y=0;y<pMainWindow->map.getHeight();y++){
                    if(!pMainWindow->map.at(x,y).scripts.empty()){
                        painter.fillRect(x*24,y*24,24,24,brushScript);
                    }
                }
        }

    }
    if(pMainWindow->showItems){
        extern QBrush itemBackground[13];

        for(u32 i=0;i<pMainWindow->map.metaData.itemCount;i++){
            QString str;
            str.sprintf("%d",i);
            u8 catagory=pMainWindow->map.Items(i).basic.catagory;
            if(catagory>=13)catagory=0;
            painter.setBrush(itemBackground[catagory]);
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
    if(!pMainWindow->map.Loaded()){
        curX=curY=-1;
        return;
    }
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

void MapPlane0::mousePressEvent(QMouseEvent* event){
    if(!pMainWindow->map.Loaded()){
        return;
    }
    if(!pMainWindow->showItems){
        if(curX!=-1){
            if(event->button()==Qt::LeftButton){
                MainWindow::MoEditCell editCell(curX,curY,pMainWindow->selBlock);
                editCell.toolTip=QString("Edit cell(%1,%2)").arg(curX).arg(curY);
                pMainWindow->doOperation(&editCell);
            }else if(event->button()==Qt::MidButton){
                pMainWindow->selBlock=pMainWindow->map.at(curX,curY).blockId;
                pBlockStore->repaint();
            }else if(event->button()==Qt::RightButton){
                DialogScripts dlg(pMainWindow->map.at(curX,curY).scripts,pMainWindow);
                dlg.setWindowTitle(QString("Scripts for cell(%1,%2)").arg(curX).arg(curY));
                dlg.exec();
            }

        }
    }
}
void MapPlane0::leaveEvent(QEvent * ){
    curX=curY=-1;
    repaint();
}
