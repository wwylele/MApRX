/*************************************************************************
    mapview.cpp
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

#include "mapview.h"
#include <QPainter>
#include <QImage>
#include <QMouseEvent>
#include <cassert>
#include "main.h"
#include "dialogscripts.h"
MapView::MapView(QWidget *parent) :
    QWidget(parent),
    transparentPattern(TRAN_PAT_GRID_SIZE*2,
                       TRAN_PAT_GRID_SIZE*2,
                       QImage::Format_ARGB32)
{
    for(int x=0;x<TRAN_PAT_GRID_SIZE;x++)
        for(int y=0;y<TRAN_PAT_GRID_SIZE;y++){
            uint white=0xFFFFFFFFUL,gray=0xFFEEEEEEUL;
            transparentPattern.setPixel(x,y,white);
            transparentPattern.setPixel(x+TRAN_PAT_GRID_SIZE,y,gray);
            transparentPattern.setPixel(x,y+TRAN_PAT_GRID_SIZE,gray);
            transparentPattern.setPixel(x+TRAN_PAT_GRID_SIZE,
                                        y+TRAN_PAT_GRID_SIZE,white);
        }
}



void MapView::paintEvent(QPaintEvent *){
    if(!pMainWindow->map.isLoaded())return;
    QPainter painter(this);
    if(pMainWindow->showEssence){
        for(u32 x=0;x<pMainWindow->map.getWidth();x++)
            for(u32 y=0;y<pMainWindow->map.getHeight();y++){
                BlockEssence e;
                e=pMainWindow->blocks.getEssences(pMainWindow->map.cellAt(x,y).blockId);
                painter.drawPixmap(x*24,y*24,res->essenceSheet,
                                   (e%16)*24,e/16*24,24,24);
            }
    }
    else{

        if(pMainWindow->showBackground &&pMainWindow->bckScr.isLoaded()){
            for(u16 x=0;x<pMainWindow->bckScr.getWidth();x++)for(u16 y=0;y<pMainWindow->bckScr.getHeight();y++){
                CharData chard=pMainWindow->bckScr.at(x,y);
                QImage tile(pMainWindow->bckTiles[
                            chard&TILE_ID_MASK
                            ].data,8,8,QImage::Format_Indexed8);
                tile.setColorTable(pMainWindow->bckPltTransit);
                for(int px=x*8;px<width;px+=pMainWindow->bckScr.getWidth()*8)
                    for(int py=y*8;py<height;py+=pMainWindow->bckScr.getHeight()*8)
                        painter.drawImage(px,py,
                                  tile.mirrored
                                  (chard&FLIP_X?true:false,
                                   chard&FLIP_Y?true:false));
            }
        }else{
            painter.fillRect(0,0,width,height,QBrush(
                                 transparentPattern));
        }

        for(int x=0;x<pMainWindow->map.getWidth();x++)
            for(int y=0;y<pMainWindow->map.getHeight();y++){
                painter.drawPixmap(x*24,y*24,
                     pMainWindow->blocksTransit[pMainWindow->map.cellAt(x,y).blockId]);
            }


    }
    if(pMainWindow->showItems){
        painter.fillRect(0,0,width,height,QBrush(QColor(255,255,255,128)));
    }
    auto drawBinding=[&painter,this](){
        QColor cSrc(0,255,0),cDst(255,0,255);
        QBrush nullBrush(Qt::transparent);

        QPen penBSrc(cSrc),penBDst(cDst);
        painter.setBrush(nullBrush);
        auto drawLink=[&painter,cSrc,cDst]
                (int x1,int y1,int x2,int y2){
            QLinearGradient brushBLinkG(x1,y1,x2,y2);
            brushBLinkG.setColorAt(0,cSrc);
            brushBLinkG.setColorAt(1,cDst);
            painter.setPen(QPen(QBrush(brushBLinkG),1));
            painter.drawLine(x1,y1,x2,y2);
        };

        for(u16 x=0;x<pMainWindow->map.getWidth();x++)
            for(u16 y=0;y<pMainWindow->map.getHeight();y++)
                for(KfMap::Script& s:pMainWindow->map.cellAt(x,y).scripts)
                    if(s[0]==2){
                        u16 dx,dy;
                        memcpy(&dx,s.data()+3,2);
                        memcpy(&dy,s.data()+5,2);
                        painter.setPen(penBSrc);
                        painter.drawRect(x*24+8,y*24+8,8,8);
                        painter.setPen(penBDst);
                        painter.drawRect(dx*24+8,dy*24+8,8,8);
                        drawLink(x*24+12,y*24+12,
                                 dx*24+12,dy*24+12);

                    }else if(s[0]==4){
                        KfMap::RipeItem& item=pMainWindow->map.itemAt(s[3]);
                        painter.setPen(penBSrc);
                        painter.drawRect(x*24+8,y*24+8,8,8);
                        painter.setPen(penBDst);
                        painter.drawEllipse(QPointF(item.basic.x,item.basic.y),20,20);
                        drawLink(x*24+12,y*24+12,
                                 item.basic.x,item.basic.y);
                    }

        for(u8 i=0;i<pMainWindow->map.getItemCount();i++){
            KfMap::RipeItem ii=pMainWindow->map.itemAt(i);
            for(KfMap::Script& s:ii.scripts)
                if(s[0]==2){
                    u16 dx,dy;
                    memcpy(&dx,s.data()+3,2);
                    memcpy(&dy,s.data()+5,2);
                    painter.setPen(penBSrc);
                    painter.drawEllipse(QPointF(ii.basic.x,ii.basic.y),20,20);
                    painter.setPen(penBDst);
                    painter.drawRect(dx*24+8,dy*24+8,8,8);
                    drawLink(ii.basic.x,ii.basic.y,
                             dx*24+12,dy*24+12);

                }else if(s[0]==4){
                    KfMap::RipeItem& item=pMainWindow->map.itemAt(s[3]);
                    painter.setPen(penBSrc);
                    painter.drawEllipse(QPointF(ii.basic.x,ii.basic.y),20,20);
                    painter.setPen(penBDst);
                    painter.drawEllipse(QPointF(item.basic.x,item.basic.y),20,20);
                    drawLink(ii.basic.x,ii.basic.y,
                             item.basic.x,item.basic.y);
                }
        }
    };

    if(pMainWindow->showScript){
        if(!pMainWindow->showItems){
            QBrush brushScript(QColor(255,0,255,100),Qt::SolidPattern);
            for(u32 x=0;x<pMainWindow->map.getWidth();x++)
                for(u32 y=0;y<pMainWindow->map.getHeight();y++){
                    if(!pMainWindow->map.cellAt(x,y).scripts.empty()){
                        painter.fillRect(x*24,y*24,24,24,brushScript);
                    }
                }
            drawBinding();
        }

    }
    if(pMainWindow->showItems){
        if(pMainWindow->showScript){
            drawBinding();

        }
        painter.setPen(QColor(Qt::black));

        for(u32 i=0;i<pMainWindow->map.getItemCount();i++){
            KfMap::RipeItem &item=pMainWindow->map.itemAt(i);
            u8 species=item.basic.species();
            ItemImages::ItemImage& image=res->itemImages.images[species];
            if(image.loaded){
                painter.drawPixmap(item.basic.x-image.dx,
                                   item.basic.y-image.dy,
                                   image.large);
            }else{
                u8 catagory=item.basic.catagory();
                if(catagory>=13)catagory=0;
                painter.setBrush(res->itemBackground[catagory]);
                painter.drawEllipse(item.basic.x-8,
                                    item.basic.y-8,
                                    16,16);
                painter.drawText(item.basic.x-8,
                                 item.basic.y-8,
                                 16,16,Qt::AlignCenter,QString::number(i));
            }

        }
        if(curItem!=-1){
            if(curItem>=pMainWindow->map.getItemCount()){
                curItem=-1;
            }else{
                int x,y;
                if(itemDraging){
                    x=dragX;
                    y=dragY;
                }else{
                    x=pMainWindow->map.itemAt(curItem).basic.x;
                    y=pMainWindow->map.itemAt(curItem).basic.y;
                }
                painter.setPen(QColor(255,0,0));
                painter.drawLine(x-16,y,x+16,y);
                painter.drawLine(x,y-16,x,y+16);
                painter.drawArc(x-16,y-16,32,32,0,5760);
            }

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

void MapView::reset(){
    width=pMainWindow->map.getWidth()*24;
    height=pMainWindow->map.getHeight()*24;
    setMinimumSize(width,height);
    resize(width,height);
    curX=curY=-1;
    curItem=-1;
    itemDraging=false;

}
QString MapView::generateStatusTip(u16 x,u16 y){
    assert(x!=0xFFFF);
    return QString(tr("Left button: change block. Middle button: get block."
                   " Right button: edit scripts. "
                "Cell (%1,%2)=%3, %4 script(s)"))
            .arg(x).arg(y).arg(pMainWindow->map.cellAt(x,y).blockId)
            .arg(pMainWindow->map.cellAt(x,y).scripts.size());
}


void MapView::mouseMoveEvent(QMouseEvent * event){
    if(!pMainWindow->map.isLoaded()){
        curX=curY=-1;
        curItem=-1;
        return;
    }
    if(itemDraging){
        if(!itemShaked){
            int dx=dragX-event->x();
            int dy=dragY-event->y();
            if(dx*dx+dy*dy>256)itemShaked=true;
        }
        if(itemShaked){
            dragX=event->x();
            dragY=event->y();
            dragX=std::min(std::max(0,dragX),pMainWindow->map.getWidth()*24);
            dragY=std::min(std::max(0,dragY),pMainWindow->map.getHeight()*24);
            update();
        }

    }
    else if(!pMainWindow->showItems){
        curItem=-1;
        if(event->x()>pMainWindow->map.getWidth()*24||
           event->y()>pMainWindow->map.getHeight()*24||
                event->x()<0||event->y()<0){
            curX=curY=-1;
            emit showStatusTip("");
        }
        else{
            curX=event->x()/24;
            curY=event->y()/24;
            emit showStatusTip(generateStatusTip(curX,curY));
        }

    }else{
        curX=curY=-1;
        for(int i=pMainWindow->map.getItemCount();i>=0;--i){
            KfMap::RipeItem &item=pMainWindow->map.itemAt(i);
            ItemImages::ItemImage image=res->itemImages.images[item.basic.species()];
            QRect rect;
            if(image.loaded)
                rect=QRect(item.basic.x-image.dx,item.basic.y-image.dy,
                       image.large.width(),image.large.height());
            else
                rect=QRect(item.basic.x-8,item.basic.y-8,16,16);
            if(rect.contains(event->pos())){
                curItem=i;
                emit showStatusTip(QString(tr("Item #%1, %2 at(%3(%4),%5(%6)"))
                                   .arg(i)
                                   .arg(res->itemDictionary.entries[item.basic.species()].speciesName)
                                   .arg(item.basic.x/24)
                                   .arg(item.basic.x%24)
                                   .arg(item.basic.y/24)
                                   .arg(item.basic.y%24));
                goto set_curItem;
            }
        }
        curItem=-1;
        emit showStatusTip("");
        set_curItem:
        ;
    }


    update();
}

void MapView::mousePressEvent(QMouseEvent* event){
    if(!pMainWindow->map.isLoaded()){
        return;
    }
    if(!pMainWindow->showItems){
        if(curX!=-1){
            if(event->button()==Qt::LeftButton){
                MainWindow::MoEditCell editCell(curX,curY,pMainWindow->selBlock);
                editCell.toolTip=QString(tr("Edit cell(%1,%2)")).arg(curX).arg(curY);
                pMainWindow->doOperation(&editCell);
                emit showStatusTip(generateStatusTip(curX,curY));
            }else if(event->button()==Qt::MidButton){
                pMainWindow->selBlock=pMainWindow->map.cellAt(curX,curY).blockId;
                pBlockStore->update();
            }else if(event->button()==Qt::RightButton){
                DialogScripts dlg(pMainWindow->map.cellAt(curX,curY).scripts,pMainWindow);
                int x=curX,y=curY;
                dlg.setWindowTitle(QString(tr("Scripts for cell(%1,%2)")).arg(x).arg(y));
                if(dlg.exec()==QDialog::Accepted){
                    MainWindow::MoEditCellScript mo(dlg.scripts,x,y);
                    mo.toolTip=QString(tr("Edit Scripts for cell(%1,%2)")).arg(x).arg(y);
                    pMainWindow->doOperation(&mo);
                }
            }

        }

    }else{
        if(curItem!=-1 && curItem<pMainWindow->map.getItemCount()){
            if(event->button()==Qt::LeftButton){
                emit selectItem(curItem);
                grabKeyboard();
                grabMouse();
                itemDraging=true;
                itemShaked=false;
                emit itemDragging(true);
                dragX=pMainWindow->map.itemAt(curItem).basic.x;
                dragY=pMainWindow->map.itemAt(curItem).basic.y;
            }
            else if(event->button()==Qt::RightButton){
                pMainWindow->editItemScripts(curItem);
            }
        }
    }
}
void MapView::mouseReleaseEvent(QMouseEvent *){
    if(pMainWindow->showItems){
        releaseKeyboard();
        releaseMouse();
        emit itemDragging(false);
        itemDraging=false;
        if(itemShaked){
            KfMap::Item item(pMainWindow->map.itemAt(curItem).basic);
            item.x=dragX;
            item.y=dragY;
            MainWindow::MoEditItemBasic mo(curItem,item);
            mo.toolTip=tr("Move Item#%1").arg(curItem);
            pMainWindow->doOperation(&mo);
        }
    }
}

void MapView::leaveEvent(QEvent * ){
    curX=curY=-1;
    curItem=-1;
    update();
    emit showStatusTip("");
}
