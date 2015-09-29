/*************************************************************************
    dialogscripts.cpp
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
#include "dialogscripts.h"
#include "ui_dialogscripts.h"
#include <assert.h>
#include <QFontMetrics>
#include <QImage>
const QString scriptText[7]={
    "",
    " - Change block to ",
    " - Bind with cell(%1,%2)",
    " - Transport to room#%1, cell(%2,%3)",
    " - Bind with item#%1",
    " - Timer: %1, %2",
    " - Generate Meta Knights:"
};

ScriptDelegate::ScriptDelegate(MainWindow* _pMainWindow,QWidget *parent) :
    pMainWindow(_pMainWindow),
    QStyledItemDelegate(parent)
{

}
void ScriptDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index) const
{
    KfMap::Script script=qvariant_cast<KfMap::Script>(index.data());
    int width;
    int dx=option.rect.x(),dy=option.rect.y();

    if (option.state & QStyle::State_Selected){

        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }else{
        painter->setPen(option.palette.text().color());
    }
    switch(script[0]){
    case 1:{
        QImage image(24,24,QImage::Format_ARGB32);
        width=option.fontMetrics.width(scriptText[1]);
        painter->drawText(dx,dy,width,30,Qt::AlignCenter,scriptText[1]);
        for(int i=0;i<script[3];i++){
            u16 blockId;
            memcpy(&blockId,script.data()+4+i*2,2);
            pMainWindow->blocks[blockId].draw([this,&image](int x,int y,const Color15& c15){
                u32 c=c15.toARGB32();
                image.setPixel(x,y,c);
            }, pMainWindow->plt,0,0,pMainWindow->tiles);
            painter->drawPixmap(dx+width+i*30+3,dy+3,24,24,QPixmap::fromImage(image));
        }
        break;}
    case 2:{
        u16 x,y;
        memcpy(&x,script.data()+3,2);
        memcpy(&y,script.data()+5,2);
        QString t=scriptText[2].arg(x).arg(y);
        width=option.fontMetrics.width(t);
        painter->drawText(dx,dy,width,30,Qt::AlignCenter,t);
        break;}
    case 3:{
        u16 r,x,y;
        memcpy(&r,script.data()+3,2);
        memcpy(&x,script.data()+5,2);
        memcpy(&y,script.data()+7,2);
        QString t=scriptText[3].arg(r).arg(x).arg(y);
        width=option.fontMetrics.width(t);
        painter->drawText(dx,dy,width,30,Qt::AlignCenter,t);
        break;}
    case 4:{
        QString t=scriptText[4].arg(script[3]);
        width=option.fontMetrics.width(t);
        painter->drawText(dx,dy,width,30,Qt::AlignCenter,t);
        break;}
    case 5:{
        QString arg1,arg2,t;
        s16 time;
        memcpy(&time,script.data()+3,2);
        if(time<0){
            arg1=QString::number(-time);
            arg2="VOID";
        }else{
            arg1=QString::number(time);
            arg2=QString::number(script[6]);
        }
        t=scriptText[5].arg(arg1,arg2);
        width=option.fontMetrics.width(t);
        painter->drawText(dx,dy,width,30,Qt::AlignCenter,t);
        break;}
    case 6:{
        QString t=scriptText[6];
        for(int i=0;i<script[3];i++){
            t+=" #"+QString::number(script[4+2*i]);
        }
        width=option.fontMetrics.width(t);
        painter->drawText(dx,dy,width,30,Qt::AlignCenter,t);
        break;}
    }
}

QSize ScriptDelegate::sizeHint(const QStyleOptionViewItem &option,
               const QModelIndex &index) const{
    int width=60;
    KfMap::Script script=qvariant_cast<KfMap::Script>(index.data());
    switch(script[0]){
    case 1:
        width=option.fontMetrics.width(scriptText[1])+30*script[3];
        break;
    case 2:{
        u16 x,y;
        memcpy(&x,script.data()+3,2);
        memcpy(&y,script.data()+5,2);
        width=option.fontMetrics.width(scriptText[2].arg(x).arg(y));
        break;}
    case 3:{
        u16 r,x,y;
        memcpy(&r,script.data()+3,2);
        memcpy(&x,script.data()+5,2);
        memcpy(&y,script.data()+7,2);
        width=option.fontMetrics.width(scriptText[3].arg(r).arg(x).arg(y));
        break;}
    case 4:
        width=option.fontMetrics.width(scriptText[4].arg(script[3]));
        break;
    case 5:{
        QString arg1,arg2,t;
        s16 time;
        memcpy(&time,script.data()+3,2);
        if(time<0){
            arg1=QString::number(-time);
            arg2="VOID";
        }else{
            arg1=QString::number(time);
            arg2=QString::number(script[6]);
        }
        t=scriptText[5].arg(arg1,arg2);
        width=option.fontMetrics.width(t);
        break;}
    case 6:{
        QString t=scriptText[6];
        for(int i=0;i<script[3];i++){
            t+=" #"+QString::number(script[4+2*i]);
        }
        width=option.fontMetrics.width(t);
        break;}
    }

    return QSize(width,30);
}
DialogScripts::DialogScripts(const std::vector<KfMap::Script> _scripts, MainWindow *_pMainWindow, QWidget *parent) :
    scripts(_scripts),
    pMainWindow(_pMainWindow),
    QDialog(parent),
    ui(new Ui::DialogScripts)
{
    ui->setupUi(this);


    ui->scriptListWidget->setItemDelegate(
                new ScriptDelegate(pMainWindow));
    for(u32 i=0;i<scripts.size();i++){
        QListWidgetItem *pItem=new QListWidgetItem();
        pItem->setData(Qt::DisplayRole,QVariant::fromValue
                       (scripts[i]));
        ui->scriptListWidget->addItem(pItem);
    }

}

DialogScripts::~DialogScripts()
{
    delete ui;
}
