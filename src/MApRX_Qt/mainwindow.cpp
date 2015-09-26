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
#include <QKeyEvent>
#include <QTextStream>
#include <QTranslator>
#include <time.h>
#include "dialogaboutme.h"
#include "dialogmakerom.h"
#include "dialogproperties.h"
#include <assert.h>
#include <set>
#include <QDebug>


ItemTableModal::ItemTableModal
    (MainWindow *_pMainWindow, QObject *parent):
    pMainWindow(_pMainWindow),
    QAbstractTableModel(parent){
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
    if(role==Qt::DisplayRole){
        switch(index.column()){
        case 0:
            return QString::number(pMap->Items(itemId).basic.species);
        case 1:
            return QString::number(pMap->Items(itemId).basic.behavior);

        case 4:
            return QString::number(pMap->Items(itemId).basic.param);
        case 5:
            return QString::number(pMap->Items(itemId).scripts.size());
        case 6:{
            int x,y;
            x=pMap->Items(itemId).basic.x;
            y=pMap->Items(itemId).basic.y;
            return QString("(%1.%2,%3.%4)").arg(x/24).arg(x%24).arg(y/24).arg(y%24);
        }
        }
    }else if(role==Qt::CheckStateRole){
        switch(index.column()){
        case 2:
            return pMap->Items(itemId).basic.flagA?Qt::Checked:Qt::Unchecked;
        case 3:
            return pMap->Items(itemId).basic.flagB?Qt::Checked:Qt::Unchecked;
        }
    }else if(role==Qt::BackgroundRole){
        static QBrush itemBackground[13]{
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
        if(pMap->Items(itemId).basic.catagory>=13)return itemBackground[0];
        return itemBackground[pMap->Items(itemId).basic.catagory];
    }
    return QVariant();
}
Qt::ItemFlags ItemTableModal::flags(const QModelIndex &index) const
{
    switch(index.column()){
    case 0:
    case 1:
    case 4:
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

bool ItemTableModal::setData(const QModelIndex & index, const QVariant & value, int role){
    KfMap::Item itemBasic=pMap->Items(index.row()).basic;
    if(role==Qt::EditRole){
        if(index.column()==0){
            itemBasic.species=value.toString().toInt();
            itemBasic.catagory=itemCatagory[itemBasic.species];
        }
        else if(index.column()==1){
            itemBasic.behavior=value.toString().toInt()&0xF;
        }
        else if(index.column()==4){
            itemBasic.param=value.toString().toInt();
        }
        else return false;
        MainWindow::MoEditItemBasic op(index.row(),itemBasic);
        pMainWindow->doOperation(&op);

        return true;
    }else if(role==Qt::CheckStateRole){
        if(index.column()==2){
            itemBasic.flagA=value.toBool()?1:0;
        }
        else if(index.column()==3){
            itemBasic.flagB=value.toBool()?1:0;
        }
        else return false;
        MainWindow::MoEditItemBasic op(index.row(),itemBasic);
        pMainWindow->doOperation(&op);
    }
    return false;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mapUpdateTimer(this),
    itemTableModal(this)
{
    ui->setupUi(this);
    ui->actionShow_Animation->setChecked(true);

    ui->mapPlane0->pMainWindow=this;
    ui->blockStore->pMainWindow=this;

    ui->itemTable->setModel(&itemTableModal);
    ui->itemTable->resizeRowsToContents();
    ui->itemTable->resizeColumnsToContents();

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
    FILE* file=_wfopen(fileName.toStdWString().data(),L"rb");
    mapdata.fromFile(file);
    fclose(file);
    currentFileName=fileName;
    ui->action_Save->setEnabled(true);
    ui->actionSave_As->setEnabled(true);
    ui->actionMake_Rom->setEnabled(true);

    //check maps
    u8 ctg[256];
    for(int i=0;i<256;i++)ctg[i]=0xFF;
    FILE* report=fopen("D:\\Github\\MApRX\\temp\\itemcatagory.txt","w");
    for(u32 i=0;i<MAP_COUNT;i++){
        map.readFile(mapdata.rawMaps(i));
        for(u8 j=0;j<map.metaData.itemCount;j++){
            if(ctg[map.Items(j).basic.species]==0xFF){
                ctg[map.Items(j).basic.species]=map.Items(j).basic.catagory;
            }else{
                assert(ctg[map.Items(j).basic.species]==map.Items(j).basic.catagory);
            }
        }
        map.unload();
    }
    for(int i=0;i<256;i++){
        fprintf(report,"    %d,\n",ctg[i]);
    }

    fclose(report);
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

void MainWindow::on_listRoom_itemDoubleClicked(QListWidgetItem * item)
{
    if(currentFileName==QString::null)return;
    mapUpdateTimer.stop();
    int roomId=item->data(Qt::UserRole).toInt();
    RoomInfo &pMapInfo=mapdata.roomInfos[roomId];

    if(pMapInfo.subFileIdSlots.rawFrtPltId==RoomInfo::invalidId){
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
        plt.readFile(mapdata.rawFrtPlts(pMapInfo.subFileIdSlots.rawFrtPltId));
        tiles.readFile(mapdata.rawFrtTileSets(pMapInfo.subFileIdSlots.rawFrtTileSetId));
        blocks.readFile(mapdata.rawFrtBlockSets(pMapInfo.subFileIdSlots.rawFrtBlockSetId));
    }

    if(map.Loaded()){
        saveCurrentRoom();
    }

    map.readFile(mapdata.rawMaps(pMapInfo.subFileIdSlots.rawMapId));

    if(pMapInfo.subFileIdSlots.rawBckScrId==RoomInfo::invalidId){
        bckScr.unload();
    }
    else{
        bckPlt.readFile(mapdata.rawBckPlts(pMapInfo.subFileIdSlots.rawBckPltId));
        bckTiles.readFile(mapdata.rawBckTileSets(pMapInfo.subFileIdSlots.rawBckTileSetId));
        bckScr.readFile(mapdata.rawBckScrs(pMapInfo.subFileIdSlots.rawBckScrId));
    }

    ui->mapPlane0->reset();
    ui->blockStore->reset();



    curRoomId=roomId;

    clearOperationStack();

    emit itemTableModal.layoutChanged();
    ui->itemTable->resizeRowsToContents();
    ui->itemTable->resizeColumnsToContents();

    mapUpdateTimer.start(5);

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
        "mapdata File(*.*)");
    if(fileName==QString::null)return;
    openMapdata(fileName);
}

void MainWindow::openMapdata(QString fileName){
    FILE* file=_wfopen(fileName.toStdWString().data(),L"rb");
    if(file==nullptr){
        QMessageBox msgBox;
        msgBox.setText("Failed to open the file.");
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    mapdata.fromFile(file);
    fclose(file);
    currentFileName=fileName;

    mapUpdateTimer.stop();
    map.unload();

    ui->action_Save->setEnabled(true);
    ui->actionSave_As->setEnabled(true);
    ui->actionMake_Rom->setEnabled(true);
    clearOperationStack();
}

void MainWindow::on_action_Save_triggered()
{
    if(currentFileName==QString::null)return;
    FILE* file=_wfopen(currentFileName.toStdWString().c_str(),L"wb");
    if(file==nullptr){
        QMessageBox msgBox;
        msgBox.setText("Failed to open the file.");
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    if(map.Loaded())saveCurrentRoom();
    mapdata.toFile(file);
    fclose(file);
}
void MainWindow::on_actionSave_As_triggered(){
    QString fileName=QFileDialog::getSaveFileName(this, "Save As ...",
        "",
        "mapdata File(*.*)");
    if(fileName==QString::null)return;
    FILE* file=_wfopen(fileName.toStdWString().c_str(),L"wb");
    if(file==nullptr){
        QMessageBox msgBox;
        msgBox.setText("Failed to open the file.");
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    if(map.Loaded())saveCurrentRoom();
    mapdata.toFile(file);
    fclose(file);
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
    ui->mapPlane0->update();
    showItems=checked;
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
    if(QDialog::Accepted==dlg.exec())
        map.metaData=dlg.metaData;
}

extern QTranslator translator;
extern QApplication* pApp;
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
    FILE* rom=_wfopen(fileName.toStdWString().data(),L"rb");
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
    FILE* mapdataFile=_wfopen(fileName.toStdWString().c_str(),L"wb");
    if(mapdataFile==nullptr){
        QMessageBox msgBox;
        msgBox.setText("Failed to open mapdata file.");
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        fclose(rom);
        return;
    }

    u16 id=nitroGetSubFileId(rom,"rom/map01/mapdata");
    u32 off,len;
    off=nitroGetSubFileOffset(rom,id,&len);
    std::unique_ptr<u8[]> buf(new u8[len]);
    fseek(rom,off,SEEK_SET);
    fread(buf.get(),len,1,rom);
    fwrite(buf.get(),len,1,mapdataFile);
    fclose(rom);
    fclose(mapdataFile);

    QMessageBox msgBox;
    msgBox.setText("Succeeded to extract mapdata from ROM.");
    msgBox.exec();

    openMapdata(fileName);
}
