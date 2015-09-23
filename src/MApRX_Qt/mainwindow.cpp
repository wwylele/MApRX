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

#include <set>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mapUpdateTimer(this)
{
    ui->setupUi(this);
    ui->actionShow_Animation->setChecked(true);

    ui->mapPlane0->pMainWindow=this;
    ui->blockStore->pMainWindow=this;

    this->grabKeyboard();

    essenceSheet.load(":/image/Essence.png");

    connect(&mapUpdateTimer, SIGNAL(timeout()), this, SLOT(on_updateMap()));


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

#ifdef _DEBUG
    //Open a file
    QString fileName="D:/KSSU_MAP/mapdata";
    FILE* file=_wfopen(fileName.toStdWString().data(),L"rb");
    mapdata.FromFile(file);
    fclose(file);
    currentFileName=fileName;
    ui->action_Save->setEnabled(true);
    ui->actionSave_As->setEnabled(true);
    ui->actionMake_Rom->setEnabled(true);
    //check maps
    FILE* report=fopen("D:\\Github\\MApRX\\temp\\effectList.txt","w");
    for(u32 i=0;i<MAP_COUNT;i++){
        map.ReadFile(mapdata.rawMaps(i));
        if(map.metaData.globalEffect!=0x80){
            fprintf(report,"%d:effect=%02X\n",i,map.metaData.globalEffect);
        }
        map.Unload();
    }


    fclose(report);
#endif

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateItemList(){
    ui->itemList->clear();
    for(u32 i=0;i<map.metaData.itemCount;i++){
        QString str;
        str.sprintf("[%d]%d<%d behavior=0x%02X%c%c param=0x%02X %c",i,
                    map.Items(i).basic.species,
                    map.Items(i).basic.catagory,
                    map.Items(i).basic.behavior,
                    map.Items(i).basic.flagA?'+':'-',
                    map.Items(i).basic.flagB?'+':'-',
                    map.Items(i).basic.param,
                    map.Items(i).basic.hasScript?' ':'*');
        QListWidgetItem *newItem;
        newItem=new QListWidgetItem(str);
        newItem->setData(Qt::UserRole,QVariant((int)i));
        ui->itemList->addItem(newItem);
    }
}

void MainWindow::saveCurrentRoom(){
    u8* p;
    u32 len;
    p=map.AllocFile(&len);
    mapdata.WriteMap(curRoomId,p,len);
    delete[] p;
}

void MainWindow::on_listRoom_itemDoubleClicked(QListWidgetItem * item)
{
    if(currentFileName==QString::null)return;
    mapUpdateTimer.stop();
    int roomId=item->data(Qt::UserRole).toInt();
    MapInfo &pMapInfo=mapdata.mapInfos[roomId];

    if(pMapInfo.subFileIdSlots.rawFrtPltId==MapInfo::invalidId){
        QMessageBox msgBox;
        msgBox.setText("Failed to load block set data.\nDefault block set data will be used.");
        msgBox.setIcon(QMessageBox::Icon::Warning);
        msgBox.exec();
        //return;
        plt.ReadFile(mapdata.rawFrtPlts(0));
        tiles.ReadFile(mapdata.rawFrtTileSets(0));
        blocks.ReadFile(mapdata.rawFrtBlockSets(0));
    }
    else{
        plt.ReadFile(mapdata.rawFrtPlts(pMapInfo.subFileIdSlots.rawFrtPltId));
        tiles.ReadFile(mapdata.rawFrtTileSets(pMapInfo.subFileIdSlots.rawFrtTileSetId));
        blocks.ReadFile(mapdata.rawFrtBlockSets(pMapInfo.subFileIdSlots.rawFrtBlockSetId));
    }

    if(map.Loaded()){
        saveCurrentRoom();
    }

    map.ReadFile(mapdata.rawMaps(pMapInfo.subFileIdSlots.rawMapId));

    if(pMapInfo.subFileIdSlots.rawBckScrId==MapInfo::invalidId){
        bckScr.Unload();
    }
    else{
        bckPlt.ReadFile(mapdata.rawBckPlts(pMapInfo.subFileIdSlots.rawBckPltId));
        bckTiles.ReadFile(mapdata.rawBckTileSets(pMapInfo.subFileIdSlots.rawBckTileSetId));
        bckScr.ReadFile(mapdata.rawBckScrs(pMapInfo.subFileIdSlots.rawBckScrId));
    }

    ui->mapPlane0->reset();
    ui->blockStore->reset();

    updateItemList();

    curRoomId=roomId;

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

        plt.Tick();
        tiles.Tick();
        if(bckScr.Loaded()){
            bckPlt.Tick();
            bckTiles.Tick();
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
    FILE* file=_wfopen(fileName.toStdWString().data(),L"rb");
    if(file==nullptr){
        QMessageBox msgBox;
        msgBox.setText("Failed to open the file.");
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    mapdata.FromFile(file);
    fclose(file);
    currentFileName=fileName;

    mapUpdateTimer.stop();
    map.Unload();

    ui->action_Save->setEnabled(true);
    ui->actionSave_As->setEnabled(true);
    ui->actionMake_Rom->setEnabled(true);
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
    mapdata.ToFile(file);
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
    mapdata.ToFile(file);
    fclose(file);
    currentFileName=fileName;
}

void MainWindow::keyPressEvent(QKeyEvent * event){
    switch(event->key()){
    case Qt::Key_E:
        ui->actionShow_Essence->setChecked(true);
        on_actionShow_Essence_triggered(true);
        break;
    case Qt::Key_S:
        ui->actionShow_Script->setChecked(true);
        on_actionShow_Script_triggered(true);
        break;
    case Qt::Key_I:
        ui->actionShow_Items->setChecked(true);
        on_actionShow_Items_triggered(true);
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent * event){
    switch(event->key()){
    case Qt::Key_E:
        ui->actionShow_Essence->setChecked(false);
        on_actionShow_Essence_triggered(false);
        break;
    case Qt::Key_S:
        ui->actionShow_Script->setChecked(false);
        on_actionShow_Script_triggered(false);
        break;
    case Qt::Key_I:
        ui->actionShow_Items->setChecked(false);
        on_actionShow_Items_triggered(false);
        break;
    default:
        QWidget::keyReleaseEvent(event);
    }
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
    this->releaseKeyboard();
    DialogProperties dlg(map.metaData);
    if(QDialog::Accepted==dlg.exec())
        map.metaData=dlg.metaData;
    this->grabKeyboard();
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
