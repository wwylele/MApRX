/*************************************************************************
    mainwindow.cpp
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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>
#include <QTextStream>
#include <QToolButton>
#include <ctime>
#include "dialogaboutme.h"
#include "dialogmakerom.h"
#include "dialogproperties.h"
#include "dialogscripts.h"
#include "dialogresizemap.h"
#include "main.h"
#include <cassert>


QBrush itemBackground[13]{
                    QColor(255,255,255),
                    QColor(128,255,128),
                    QColor(255,255,128),
                    QColor(255,128,128),
                    QColor(255,128,255),
                    QColor(128,128,255),
                    QColor(128,255,255),
                    Qt::transparent,
                    Qt::transparent,
                    Qt::transparent,
                    QColor(128,128,128),
                    QColor(192,192,192),
                    QColor(255,192,128)
};

ItemTableModal::ItemTableModal
    (MainWindow *_pMainWindow, QObject *parent):
    QAbstractTableModel(parent),
    pMainWindow(_pMainWindow){
    pMap=&pMainWindow->map;
}
int ItemTableModal::columnCount(const QModelIndex &) const{
    return 7;
}
int ItemTableModal::rowCount(const QModelIndex &) const{
    if(!pMap->Loaded())return 0;
    return pMap->metaData.itemCount;
}
QVariant ItemTableModal::data(const QModelIndex &index, int role) const{

    if(!pMap->Loaded())return QVariant();
    u8 itemId=index.row();
    if(itemId>=pMap->metaData.itemCount)return QVariant();
    if(role==Qt::DisplayRole||role==Qt::EditRole){
        switch(index.column()){
        case 0:
            return QString::number(pMap->Items(itemId).basic.species());
        case 1:
            return QString::number(pMap->Items(itemId).basic.behavior());

        case 4:
            return QString::number(pMap->Items(itemId).basic.param());
        case 5:
            return QString::number(pMap->Items(itemId).scripts.size());
        case 6:{
            int x,y;
            x=pMap->Items(itemId).basic.x;
            y=pMap->Items(itemId).basic.y;
            QString str;
            str.sprintf("%d(%02d),%d(%02d)",x/24,x%24,y/24,y%24);
            return str;
        }
        }
    }else if(role==Qt::CheckStateRole){
        switch(index.column()){
        case 2:
            return pMap->Items(itemId).basic.flagA()?Qt::Checked:Qt::Unchecked;
        case 3:
            return pMap->Items(itemId).basic.flagB()?Qt::Checked:Qt::Unchecked;
        }
    }else if(role==Qt::BackgroundRole){
        if(pMap->Items(itemId).basic.catagory()>=13)return itemBackground[0];
        return itemBackground[pMap->Items(itemId).basic.catagory()];
    }
    return QVariant();
}
Qt::ItemFlags ItemTableModal::flags(const QModelIndex &index) const
{
    switch(index.column()){
    case 0:
    case 1:
    case 4:
    case 6:
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    case 2:
    case 3:
        return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
    default:
        return  QAbstractTableModel::flags(index);
    }


}
QVariant ItemTableModal::headerData(int section, Qt::Orientation orientation, int role) const{
    if(role==Qt::DisplayRole){
        if(orientation==Qt::Horizontal){
            switch(section){
            case 0:return "Species";
            case 1:return "Behavior";
            case 2:return "A";
            case 3:return "B";
            case 4:return "Parameter";
            case 5:return "Script";
            case 6:return "Position";
            }
        }
        else{
            return QString::number(section);
        }
    }
    return QVariant();
}
void ItemTableModal::itemChanged(u8 id){
    emit dataChanged(createIndex(id,0),createIndex(id,6));
}

QModelIndex ItemTableModal::getIndex(int row,int column){
    return createIndex(row,column);
}

bool ItemTableModal::setData(const QModelIndex & index, const QVariant & value, int role){
    KfMap::Item itemBasic=pMap->Items(index.row()).basic;
    if(role==Qt::EditRole){
        bool toIntOk;
        int toIntBuf;
        if(index.column()==0){
            toIntBuf=value.toString().toInt(&toIntOk);
            if(!toIntOk || toIntBuf<0 || toIntBuf>255)return false;
            itemBasic.setSpecies(toIntBuf);
            itemBasic.setCatagory(itemCatagory[toIntBuf]);
        }
        else if(index.column()==1){
            toIntBuf=value.toString().toInt(&toIntOk);
            if(!toIntOk || toIntBuf<0 || toIntBuf>15)return false;
            itemBasic.setBehavior(toIntBuf);
        }
        else if(index.column()==4){
            toIntBuf=value.toString().toInt(&toIntOk);
            if(!toIntOk || toIntBuf<0 || toIntBuf>255)return false;
            itemBasic.setParam(toIntBuf);
        }else if(index.column()==6){
            QString str=value.toString();
            int x1,x2,y1,y2;
            if(std::swscanf(str.toStdWString().c_str(),L"%d(%d),%d(%d)",
                       &x1,&x2,&y1,&y2)!=4)
                return false;
            itemBasic.x=x1*24+x2;
            itemBasic.y=y1*24+y2;

        }
        else return false;
    }else if(role==Qt::CheckStateRole){
        if(index.column()==2){
            itemBasic.setFlagA(value.toBool());
        }
        else if(index.column()==3){
            itemBasic.setFlagB(value.toBool());
        }
        else return false;
    }else return false;
    MainWindow::MoEditItemBasic op(index.row(),itemBasic);
    op.toolTip=QString("Edit Item#%1").arg(index.row());
    pMainWindow->doOperation(&op);
    return true;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    itemTableModal(this),
    ui(new Ui::MainWindow),
    mapUpdateTimer(this)
{
    ui->setupUi(this);
    ui->actionShow_Animation->setChecked(true);

    ui->mapPlane0->pMainWindow=this;
    ui->mapPlane0->pBlockStore=ui->blockStore;
    ui->blockStore->pMainWindow=this;

    ui->itemTable->setModel(&itemTableModal);
    ui->itemTable->resizeRowsToContents();
    ui->itemTable->resizeColumnsToContents();

    ui->splitterMain->setStretchFactor(1,1);
    ui->splitterRight->setStretchFactor(0,1);



    QToolButton* scrollAreaCornerResize=new QToolButton(this);
    scrollAreaCornerResize->setDefaultAction(ui->action_Resize_Map);
    ui->mapPlane0ScrollArea->setCornerWidget(scrollAreaCornerResize);

    MapOperation::pMap=&map;
    MapOperation::pMainWindow=this;

    essenceSheet.load(":/image/Essence.png");

    connect(&mapUpdateTimer, SIGNAL(timeout()), this, SLOT(on_updateMap()));
    connect(ui->actionUndo, SIGNAL(triggered()), this, SLOT(undo()));
    connect(ui->actionRedo, SIGNAL(triggered()), this, SLOT(redo()));

    QString roomName[MAP_COUNT];
    QFile roomNameResFile(":/text/RoomName.txt");
    roomNameResFile.open(QIODevice::ReadOnly);
    QTextStream roomNameRes(&roomNameResFile);
    while(!roomNameRes.atEnd()){
        QString in;
        QStringList inl;
        in=roomNameRes.readLine();
        inl=in.split('.');
        roomName[inl[0].toInt()]=inl[1];
    }
    roomNameResFile.close();


    for(int i=0;i<MAP_COUNT;i++){
        QString str;
        str.sprintf("Room#%d ",i);
        str+=roomName[i];
        QListWidgetItem *newItem;
        newItem=new QListWidgetItem(str);
        newItem->setData(Qt::UserRole,QVariant(i));
        ui->listRoom->addItem(newItem);

    }

    clearOperationStack();

#ifdef _DEBUG
    //Open a file
    QString fileName="D:/KSSU_MAP/mapdata";
    std::FILE* file=fopenQ(fileName,"rb");
    if(!file)return;
    mapdata.fromFile(file);
    std::fclose(file);
    currentFileName=fileName;
    ui->action_Save->setEnabled(true);
    ui->actionSave_As->setEnabled(true);
    ui->actionMake_Rom->setEnabled(true);


#endif

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::saveCurrentRoom(){
    u8* p;
    u32 len;
    p=map.generateFile(&len);
    mapdata.writeMap(curRoomId,p,len);
    delete[] p;
}
void MainWindow::loadRoom(int roomId){
    mapUpdateTimer.stop();
    RoomInfo &pMapInfo=mapdata.roomInfos[roomId];

    if(pMapInfo.subFileIdData.subFileIdSlots.rawFrtPltId==RoomInfo::invalidId){
        QMessageBox msgBox;
        msgBox.setText("Failed to load block set data.\nDefault block set data will be used.");
        msgBox.setIcon(QMessageBox::Icon::Warning);
        msgBox.exec();
        //return;
        plt.readFile(mapdata.rawFrtPlts(0));
        tiles.readFile(mapdata.rawFrtTileSets(0));
        blocks.readFile(mapdata.rawFrtBlockSets(0));
    }
    else{
        plt.readFile(mapdata.rawFrtPlts(pMapInfo.subFileIdData.subFileIdSlots.rawFrtPltId));
        tiles.readFile(mapdata.rawFrtTileSets(pMapInfo.subFileIdData.subFileIdSlots.rawFrtTileSetId));
        blocks.readFile(mapdata.rawFrtBlockSets(pMapInfo.subFileIdData.subFileIdSlots.rawFrtBlockSetId));
    }



    map.readFile(mapdata.rawMaps(pMapInfo.subFileIdData.subFileIdSlots.rawMapId));

    if(pMapInfo.subFileIdData.subFileIdSlots.rawBckScrId==RoomInfo::invalidId){
        bckScr.unload();
    }
    else{
        bckPlt.readFile(mapdata.rawBckPlts(pMapInfo.subFileIdData.subFileIdSlots.rawBckPltId));
        bckTiles.readFile(mapdata.rawBckTileSets(pMapInfo.subFileIdData.subFileIdSlots.rawBckTileSetId));
        bckScr.readFile(mapdata.rawBckScrs(pMapInfo.subFileIdData.subFileIdSlots.rawBckScrId));
    }

    resetMap();
    ui->blockStore->reset();



    curRoomId=roomId;

    clearOperationStack();

    emit itemTableModal.layoutChanged();
    ui->itemTable->clearSelection();
    ui->itemTable->resizeColumnsToContents();
    ui->itemTable->resizeRowsToContents();

    ui->menu_Map->setEnabled(true);

    mapUpdateTimer.start(5);
}

void MainWindow::on_listRoom_itemDoubleClicked(QListWidgetItem * item)
{
    if(currentFileName==QString::null)return;

    int roomId=item->data(Qt::UserRole).toInt();
    if(map.Loaded()){
        saveCurrentRoom();
    }
    loadRoom(roomId);


}

void MainWindow::on_updateMap(){
    static long lTime=0,timeA=0;
    long cTime;
    if(!showAnimation)return;
    cTime=clock();
    timeA+=cTime-lTime;
    if(timeA>1000)timeA=0;
    while(timeA>=16){
        timeA-=16;

        plt.tick();
        tiles.tick();
        if(bckScr.Loaded()){
            bckPlt.tick();
            bckTiles.tick();
        }
        ui->mapPlane0->update();
        ui->blockStore->update();

    }

    lTime=cTime;

}

void MainWindow::on_actionAbout_MApRX_triggered()
{
    DialogAboutMe dlg;
    dlg.exec();
}

void MainWindow::on_action_Open_triggered()
{
    QString fileName=QFileDialog::getOpenFileName(this, "Open File",
        "",
        "mapdata File(*.bin)");
    if(fileName==QString::null)return;
    openMapdata(fileName);
}

void MainWindow::openMapdata(QString fileName){
    std::FILE* file=fopenQ(fileName,"rb");
    if(file==nullptr){
        QMessageBox msgBox;
        msgBox.setText("Failed to open the file.");
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    mapdata.fromFile(file);
    std::fclose(file);
    currentFileName=fileName;

    mapUpdateTimer.stop();
    map.unload();
    ui->menu_Map->setEnabled(true);

    ui->action_Save->setEnabled(true);
    ui->actionSave_As->setEnabled(true);
    ui->actionMake_Rom->setEnabled(true);
    clearOperationStack();
}

void MainWindow::on_action_Save_triggered()
{
    if(currentFileName==QString::null)return;
    std::FILE* file=fopenQ(currentFileName,"wb");
    if(file==nullptr){
        QMessageBox msgBox;
        msgBox.setText("Failed to open the file.");
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    if(map.Loaded())saveCurrentRoom();
    mapdata.toFile(file);
    std::fclose(file);
}
void MainWindow::on_actionSave_As_triggered(){
    QString fileName=QFileDialog::getSaveFileName(this, "Save As ...",
        "",
        "mapdata File(*.bin)");
    if(fileName==QString::null)return;
    std::FILE* file=fopenQ(fileName,"wb");
    if(file==nullptr){
        QMessageBox msgBox;
        msgBox.setText("Failed to open the file.");
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    if(map.Loaded())saveCurrentRoom();
    mapdata.toFile(file);
    std::fclose(file);
    currentFileName=fileName;
}


void MainWindow::on_actionShow_Essence_triggered(bool checked)
{
    showEssence=checked;
    ui->mapPlane0->update();
    ui->blockStore->update();
}
void MainWindow::on_actionShow_Script_triggered(bool checked){
    showScript=checked;
    ui->mapPlane0->update();
    ui->blockStore->update();
}

void MainWindow::on_actionShow_Animation_triggered(bool checked)
{
    showAnimation=checked;
}
void MainWindow::on_actionShow_Items_triggered(bool checked)
{

    showItems=checked;
    ui->mapPlane0->update();
}
void MainWindow::on_actionShow_Background_triggered(bool checked)
{

    showBackground=checked;
    ui->mapPlane0->update();
}


void MainWindow::on_actionMake_Rom_triggered()
{
    on_action_Save_triggered();
    DialogMakeRom dlg(currentFileName);
    dlg.exec();
}

void MainWindow::on_actionMap_Properties_triggered()
{
    if(!map.Loaded()){
        return;
    }
    DialogProperties dlg(map.metaData);
    if(QDialog::Accepted==dlg.exec()){
        MoEditMetaData mo(dlg.metaData);
        mo.toolTip="Change Properties";
        doOperation(&mo);
    }
}


void MainWindow::on_actionEnglish_triggered(){
    pApp->removeTranslator(&translator);
    ui->retranslateUi(this);
}

void MainWindow::on_actionChinese_triggered()
{
    pApp->installTranslator(&translator);
    ui->retranslateUi(this);
}
void MainWindow::on_actionExtract_triggered(){
    QString fileName=QFileDialog::getOpenFileName(this, "Select ROM",
        "",
        "ROM File(*.nds *.bin);;Any files(*.*)");
    if(fileName==QString::null)return;
    std::FILE* rom=fopenQ(fileName,"rb");
    if(rom==nullptr){
        QMessageBox msgBox;
        msgBox.setText("Failed to open the ROM.");
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }

    fileName=QFileDialog::getSaveFileName(this, "Save mapdata to...",
        "",
        "mapdata File(*.bin)");
    if(fileName==QString::null)return;
    std::FILE* mapdataFile=fopenQ(fileName,"wb");
    if(mapdataFile==nullptr){
        QMessageBox msgBox;
        msgBox.setText("Failed to open mapdata file.");
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        std::fclose(rom);
        return;
    }

    u16 id=nitroGetSubFileId(rom,"rom/map01/mapdata");
    u32 off,len;
    off=nitroGetSubFileOffset(rom,id,&len);
    std::unique_ptr<u8[]> buf(new u8[len]);
    std::fseek(rom,off,SEEK_SET);
    std::fread(buf.get(),len,1,rom);
    std::fwrite(buf.get(),len,1,mapdataFile);
    std::fclose(rom);
    std::fclose(mapdataFile);

    QMessageBox msgBox;
    msgBox.setText("Succeeded to extract mapdata from ROM.");
    msgBox.exec();

    openMapdata(fileName);
}

void MainWindow::on_buttonItemUp_clicked()
{
    if(!map.Loaded())return;
    QModelIndex selection=ui->itemTable->currentIndex();
    if(!selection.isValid())return;
    u8 selItem=selection.row();
    if(selItem==0)return;
    MoSwapItem mo(selItem-1);
    mo.toolTip=QString("Move up Item#%1").arg(selItem);
    doOperation(&mo);

    ui->itemTable->setCurrentIndex(itemTableModal.getIndex(selItem-1,selection.column()));
}

void MainWindow::on_buttonItemDown_clicked()
{
    if(!map.Loaded())return;
    QModelIndex selection=ui->itemTable->currentIndex();
    if(!selection.isValid())return;
    u8 selItem=selection.row();
    if(selItem==map.metaData.itemCount-1)return;
    MoSwapItem mo(selItem);
    mo.toolTip=QString("Move down Item#%1").arg(selItem);
    doOperation(&mo);

    ui->itemTable->setCurrentIndex(itemTableModal.getIndex(selItem+1,selection.column()));
}

void MainWindow::on_buttonItemDelete_clicked()
{
    if(!map.Loaded())return;
    QModelIndex selection=ui->itemTable->currentIndex();
    if(!selection.isValid())return;
    u8 selItem=selection.row();
    MoDeleteItem mo(selItem);
    mo.toolTip=QString("Remove Item#%1").arg(selItem);
    doOperation(&mo);
}

void MainWindow::on_buttonItemNew_clicked()
{
    if(!map.Loaded())return;
    KfMap::RipeItem item;
    QSize size=ui->mapPlane0ScrollArea->size();
    int tx=(ui->mapPlane0ScrollArea->horizontalScrollBar()->value()+size.width()/2);
    item.basic.x=(tx>=0?tx:0);
    int ty=(ui->mapPlane0ScrollArea->verticalScrollBar()->value()+size.height()/2);
    item.basic.y=(ty>=0?ty:0);
    if(item.basic.x>map.metaData.width*24)item.basic.x=map.metaData.width*24;
    if(item.basic.y>map.metaData.height*24)item.basic.y=map.metaData.height*24;
    MoNewItem mo(map.metaData.itemCount,item);
    mo.toolTip="Add Item";
    doOperation(&mo);
    ui->itemTable->setCurrentIndex(itemTableModal.getIndex(
                                       map.metaData.itemCount-1,0));
}

void MainWindow::on_itemTable_clicked(const QModelIndex &index)
{
    if(!map.Loaded())return;
    u8 itemId=index.row();
    KfMap::Item& item=map.Items(itemId).basic;
    QSize size=ui->mapPlane0ScrollArea->size();
    ui->mapPlane0ScrollArea->horizontalScrollBar()->setValue(
                item.x-size.width()/2);
    ui->mapPlane0ScrollArea->verticalScrollBar()->setValue(
                item.y-size.height()/2);

    if(index.column()==5){
        DialogScripts dlg(map.Items(itemId).scripts,this);
        dlg.setWindowTitle(QString("Scripts for item#%1").arg(itemId));
        if(dlg.exec()==QDialog::Accepted){
            MoEditItemScript mo(dlg.scripts,itemId);
            mo.toolTip=QString("Edit Scripts for item#%1").arg(itemId);
            doOperation(&mo);
        }
    }

}
void MainWindow::on_actionDiscard_Changes_triggered(){
    if(!map.Loaded())return;
    QMessageBox msgBox;
    msgBox.setText("Do you really want to discard your changes on this room?");
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    if(msgBox.exec()!=QMessageBox::Yes)return;
    loadRoom(curRoomId);
}
void MainWindow::on_action_Resize_Map_triggered(){
    if(!map.Loaded())return;
    DialogResizeMap dlg(map.metaData.width,map.metaData.height);
    if(dlg.exec()!=QDialog::Accepted)return;
    MoResizeMap mo(dlg.mapWidth,dlg.mapHeight,dlg.hAlign,dlg.vAlign);
    mo.toolTip="Resize Map";
    doOperation(&mo);
}
void MainWindow::on_actionSave_to_Image_triggered(){
    if(!map.Loaded())return;
    QString fileName=QFileDialog::getSaveFileName(this, "Save image to...",
        "",
        "PNG(*.png);;BMP(*.bmp)");
    if(fileName==QString::null)return;
    QImage image(map.metaData.width*24,map.metaData.height*24,QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    map.draw([&image](int x,int y,const Color15& c){
        image.setPixel(x,y,c.toARGB32());
    },plt,0,0,blocks,tiles);
    image.save(fileName);

}

void MainWindow::resetMap(){
    ui->mapPlane0->reset();
    ui->mapPlane0ScrollArea->horizontalScrollBar()->setValue(0);
    ui->mapPlane0ScrollArea->verticalScrollBar()->setValue(0);
}
