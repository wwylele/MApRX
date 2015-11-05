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
#include <cassert>
#include <cstring>
#include <QFontMetrics>
#include <QImage>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>

QString ScriptDelegate::scriptText[7]={
    tr(""),
    tr("Change block to "),
    tr("Bind with cell"),//+(%1,%2)
    tr("Transport to room#"),//+%1, cell(%2,%3)
    tr("Bind with item#"),//+%1
    tr("Misc:"),//+ %1, %2
    tr("Generate Meta Knights:")
};

ScriptDelegate::ScriptDelegate(MainWindow* _pMainWindow,QWidget *parent) :
    QStyledItemDelegate(parent),
    pMainWindow(_pMainWindow)
{

}
QString ScriptDelegate::scriptToString(const KfMap::Script& script){
    switch(script[0]){
    case 1:
        return scriptText[1];
    case 2:{
        u16 x,y;
        std::memcpy(&x,script.data()+3,2);
        std::memcpy(&y,script.data()+5,2);
        return scriptText[2]+QString("(%1,%2)").arg(x).arg(y);
    }
    case 3:{
        u16 r,x,y;
        std::memcpy(&r,script.data()+3,2);
        std::memcpy(&x,script.data()+5,2);
        std::memcpy(&y,script.data()+7,2);
        return scriptText[3]+QString("%1, cell(%2,%3)").arg(r).arg(x).arg(y);
    }
    case 4:
        return scriptText[4]+QString::number(script[3]);
    case 5:{

        QString arg1,arg2;
        s16 time;
        std::memcpy(&time,script.data()+3,2);
        if(time<0){
            arg1=QString::number(-time);
            arg2="VOID";
        }else{
            arg1=QString::number(time);
            arg2=QString::number(script[6]);
        }
        return scriptText[5]+QString(" %1, %2").arg(arg1,arg2);
    }
    case 6:{
        QString t=scriptText[6];
        for(int i=0;i<script[3];i++){
            t+=" #"+QString::number(script[4+2*i]);
        }
        return t;
    }
    }
    assert(0);
    return QString("");
}

QWidget *ScriptDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
                      const QModelIndex &index) const{
    KfMap::Script script=qvariant_cast<KfMap::Script>(index.data());
    QWidget* editor=new QWidget(parent);
    editor->setBackgroundRole(QPalette::Highlight);
    editor->setAutoFillBackground(true);
    switch(script[0]){
    case 1:case 2:case 4:case 5:case 6:{
        QHBoxLayout *layout=new QHBoxLayout();
        layout->addWidget(new QLabel(scriptText[script[0]],editor));
        layout->addWidget(new QLineEdit(editor));
        editor->setLayout(layout);
        break;
    }
    case 3:{
        QHBoxLayout *layout=new QHBoxLayout();
        layout->addWidget(new QLabel(scriptText[3],editor));
        layout->addWidget(new QLineEdit(editor));
        layout->addWidget(new QLabel(tr(",cell"),editor));
        layout->addWidget(new QLineEdit(editor));
        editor->setLayout(layout);
        break;
    }
    default:
        assert(0);
    }
    return editor;
}
void ScriptDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const{
    KfMap::Script script=qvariant_cast<KfMap::Script>(index.data());
    switch(script[0]){
    case 1:case 6:{
        u8 count=script[3];
        QString str;
        for(int i=0;i<count;i++){
            u16 id;
            memcpy(&id,script.data()+4+i*2,2);
            if(i!=0)str+=",";
            str+=QString::number(id);
        }
        (qobject_cast<QLineEdit*>(
             editor->layout()->itemAt(1)->widget()))
             ->setText(str);
        break;
    }
    case 2:{
        u16 x,y;
        std::memcpy(&x,script.data()+3,2);
        std::memcpy(&y,script.data()+5,2);
        (qobject_cast<QLineEdit*>(
             editor->layout()->itemAt(1)->widget()))
             ->setText(QString("%1,%2").arg(x).arg(y));
        break;
    }
    case 3:{
        u16 r,x,y;
        std::memcpy(&r,script.data()+3,2);
        std::memcpy(&x,script.data()+5,2);
        std::memcpy(&y,script.data()+7,2);
        (qobject_cast<QLineEdit*>(
             editor->layout()->itemAt(1)->widget()))
             ->setText(QString::number(r));
        (qobject_cast<QLineEdit*>(
             editor->layout()->itemAt(3)->widget()))
             ->setText(QString("%1,%2").arg(x).arg(y));
        break;
    }
    case 4:{
        (qobject_cast<QLineEdit*>(
             editor->layout()->itemAt(1)->widget()))
             ->setText(QString::number(script[3]));
        break;
    }
    case 5:{
        QString str;
        s16 time;
        std::memcpy(&time,script.data()+3,2);
        if(time<0){
            str=QString::number(-time);
        }else{
            str=QString("%1,%2").arg(time).arg(script[6]);
        }
        (qobject_cast<QLineEdit*>(
             editor->layout()->itemAt(1)->widget()))
             ->setText(str);
        break;
    }
    default:break;
    }
}
void ScriptDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index) const{
    KfMap::Script script=qvariant_cast<KfMap::Script>(index.data());
    switch(script[0]){
    case 1:case 6:{
        QString str=(qobject_cast<QLineEdit*>(
                editor->layout()->itemAt(1)->widget()))
                ->text();
        QStringList strL=str.split(',');
        if(strL.size()==0||strL.size()>16)return;
        script.resize(4+strL.size()*2);
        script[3]=strL.size();
        for(int i=0;i<strL.size();i++){
            u16 id;
            bool ok;
            id=strL[i].toUShort(&ok);
            if(!ok)return;
            memcpy(script.data()+4+i*2,&id,2);
        }
        break;
    }
    case 2:{
        u32 x,y;
        QString str=(qobject_cast<QLineEdit*>(
                editor->layout()->itemAt(1)->widget()))
                ->text();
        if(std::swscanf(str.toStdWString().c_str(),L"%u,%u",
                   &x,&y)!=2)
            return;
        if(x>0xFFFF || y>0xFFFF)return;
        std::memcpy(script.data()+3,&x,2);
        std::memcpy(script.data()+5,&y,2);
        break;
    }
    case 3:{
        u32 r,x,y;
        QString str=(qobject_cast<QLineEdit*>(
                editor->layout()->itemAt(3)->widget()))
                ->text();
        if(std::swscanf(str.toStdWString().c_str(),L"%u,%u",
                   &x,&y)!=2)
            return;
        bool toUIntOk;
        r=(qobject_cast<QLineEdit*>(
               editor->layout()->itemAt(1)->widget()))
               ->text().toULong(&toUIntOk);
        if(!toUIntOk)return;
        if(r>0xFFFF||x>0xFFFF || y>0xFFFF)return;
        std::memcpy(script.data()+3,&r,2);
        std::memcpy(script.data()+5,&x,2);
        std::memcpy(script.data()+7,&y,2);
        break;
    }
    case 4:{
        u32 itemId;
        bool ok;
        itemId=(qobject_cast<QLineEdit*>(
             editor->layout()->itemAt(1)->widget()))
             ->text().toULong(&ok);
        if(!ok || itemId>0xFF)return;
        script[3]=itemId;
        break;

    }
    case 5:{
        QString str=(qobject_cast<QLineEdit*>(
                editor->layout()->itemAt(1)->widget()))
                ->text();
        QStringList strL=str.split(',');
        bool ok;
        if(strL.size()>1){
            u16 t=strL[0].toUShort(&ok);
            if(!ok||t>0x7FFF)return;
            script.resize(7);
            memcpy(script.data()+3,&t,2);
            script[5]=0;
            t=strL[1].toUShort(&ok);
            if(!ok||t>0xFF)return;
            script[6]=t;
        }else{
            u16 t=strL[0].toUShort(&ok);
            s16 time;
            if(!ok||t>0x7FFF || t==0)return;
            time=-t;
            script.resize(5);
            memcpy(script.data()+3,&time,2);
        }
        break;
    }

    default:break;
    }
    model->setData(index,QVariant::fromValue(script));
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
            image.fill(Qt::transparent);
            u16 blockId;
            std::memcpy(&blockId,script.data()+4+i*2,2);
            pMainWindow->blocks[blockId].draw([this,&image](int x,int y,const Color15& c15){
                u32 c=c15.toARGB32();
                image.setPixel(x,y,c);
            }, pMainWindow->plt,0,0,pMainWindow->tiles);
            painter->drawPixmap(dx+width+i*30+3,dy+3,24,24,QPixmap::fromImage(image));
        }
        break;}
    case 2:case 3:case 4:case 5:case 6:{
        QString t=scriptToString(script);
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
    case 2:case 3:case 4:case 5:case 6:
        width=option.fontMetrics.width(scriptToString(script));
        break;
    }

    return QSize(width,30);
}
DialogScripts::DialogScripts(const std::vector<KfMap::Script> _scripts, MainWindow *_pMainWindow, QWidget *parent) :
    QDialog(parent),
    scripts(_scripts),
    ui(new Ui::DialogScripts),
    pMainWindow(_pMainWindow)
{
    ui->setupUi(this);


    ui->scriptListWidget->setItemDelegate(
                new ScriptDelegate(pMainWindow));
    for(u32 i=0;i<scripts.size();i++){
        QListWidgetItem *pItem=new QListWidgetItem();
        pItem->setData(Qt::DisplayRole,QVariant::fromValue
                       (scripts[i]));
        pItem->setFlags(Qt::ItemIsEditable|pItem->flags());
        ui->scriptListWidget->addItem(pItem);
    }

    ui->buttonAdd->setMenu(&menu);
    menu.addAction(ui->actionAddScript1);
    menu.addAction(ui->actionAddScript2);
    menu.addAction(ui->actionAddScript3);
    menu.addAction(ui->actionAddScript4);
    menu.addAction(ui->actionAddScript5);
    menu.addAction(ui->actionAddScript6);

}

DialogScripts::~DialogScripts()
{
    delete ui;
}

void DialogScripts::on_buttonBox_accepted()
{
    scripts.clear();
    for(int i=0;;i++){
        QListWidgetItem* pItem=ui->scriptListWidget->item(i);
        if(pItem==0)break;
        scripts.push_back(qvariant_cast<KfMap::Script>
                          (pItem->data(Qt::DisplayRole)));
    }
    accept();
}

void DialogScripts::on_buttonRemove_clicked()
{

    delete ui->scriptListWidget->takeItem(
                ui->scriptListWidget->currentRow());
}


void DialogScripts::addScript(const KfMap::Script& script){
    QListWidgetItem *pItem=new QListWidgetItem();
    pItem->setData(Qt::DisplayRole,QVariant::fromValue
                   (script));
    pItem->setFlags(Qt::ItemIsEditable|pItem->flags());
    ui->scriptListWidget->addItem(pItem);
}

void DialogScripts::on_actionAddScript1_triggered()
{
    addScript(KfMap::Script{1,0xCC,0xCC,1,0,0});
}

void DialogScripts::on_actionAddScript2_triggered()
{
    addScript(KfMap::Script{2,0xCC,0xCC,0,0,0,0});
}

void DialogScripts::on_actionAddScript3_triggered()
{
    addScript(KfMap::Script{3,0xCC,0xCC,0,0,0,0,0,0});
}

void DialogScripts::on_actionAddScript4_triggered()
{
    addScript(KfMap::Script{4,0xCC,0xCC,0});
}

void DialogScripts::on_actionAddScript5_triggered()
{
    addScript(KfMap::Script{5,0xCC,0xCC,0xFF,0xFF});
}

void DialogScripts::on_actionAddScript6_triggered()
{
    addScript(KfMap::Script{6,0xCC,0xCC,1,0,0});
}
