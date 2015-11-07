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
#include <QSettings>
#include <QComboBox>
#include <ctime>
#include "dialogaboutme.h"
#include "dialogmakerom.h"
#include "dialogproperties.h"
#include "dialogscripts.h"
#include "dialogresizemap.h"
#include "main.h"
#include <cassert>




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
    if(!pMap->isLoaded())return 0;
    return pMap->metaData.itemCount;
}
QVariant ItemTableModal::data(const QModelIndex &index, int role) const{

    if(!pMap->isLoaded())return QVariant();
    u8 itemId=index.row();
    if(itemId>=pMap->metaData.itemCount)return QVariant();
    if(role==Qt::DecorationRole && index.column()==0){
        return res->itemImages.images[
                pMap->itemAt(itemId).basic.species()]
                .small;
    }
    else if(role==Qt::DisplayRole||role==Qt::EditRole){
        switch(index.column()){
        case 0:
            return QString::number(pMap->itemAt(itemId).basic.species())
                    +(role==Qt::DisplayRole?QString("(%1)")
                    .arg(res->itemDictionary.entries
                         [pMap->itemAt(itemId).basic.species()].speciesName):"");
        case 1:
            return QString::number(pMap->itemAt(itemId).basic.behavior())
                    +(role==Qt::DisplayRole?QString("(%1)")
                    .arg(res->itemDictionary.entries
                         [pMap->itemAt(itemId).basic.species()]
                         .behaviorName[pMap->itemAt(itemId).basic.behavior()]):"");

        case 4:
            if(role==Qt::DisplayRole)return QString("%1:%2").arg(
                        res->itemDictionary.entries
                        [pMap->itemAt(itemId).basic.species()].paramName)
                        .arg(pMap->itemAt(itemId).basic.param());
            else return QString::number(pMap->itemAt(itemId).basic.param());
        case 5:
            return QString::number(pMap->itemAt(itemId).scripts.size());
        case 6:{
            int x,y;
            x=pMap->itemAt(itemId).basic.x;
            y=pMap->itemAt(itemId).basic.y;
            QString str;
            str.sprintf("%d(%02d),%d(%02d)",x/24,x%24,y/24,y%24);
            return str;
        }
        }
    }else if(role==Qt::CheckStateRole){
        switch(index.column()){
        case 2:
            return pMap->itemAt(itemId).basic.flagA()?Qt::Checked:Qt::Unchecked;
        case 3:
            return pMap->itemAt(itemId).basic.flagB()?Qt::Checked:Qt::Unchecked;
        }
    }else if(role==Qt::BackgroundRole){
        if(pMap->itemAt(itemId).basic.catagory()>=13)return res->itemBackground[0];
        return res->itemBackground[pMap->itemAt(itemId).basic.catagory()];
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
            case 0:return tr("Species");
            case 1:return tr("Behavior");
            case 2:return "A";
            case 3:return "B";
            case 4:return tr("Parameter");
            case 5:return tr("Script");
            case 6:return tr("Position");
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
    KfMap::Item itemBasic=pMap->itemAt(index.row()).basic;
    if(role==Qt::EditRole){
        bool toIntOk;
        int toIntBuf;
        if(index.column()==0){
            toIntBuf=value.toInt(&toIntOk);
            if(!toIntOk || toIntBuf<0 || toIntBuf>255)return false;
            itemBasic.setSpecies(toIntBuf);
            itemBasic.setCatagory(KfMap::itemCatagory[toIntBuf]);
            itemBasic.setBehavior(0);
        }
        else if(index.column()==1){
            toIntBuf=value.toInt(&toIntOk);
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
    op.toolTip=QString(tr("Edit Item#%1")).arg(index.row());
    pMainWindow->doOperation(&op);
    return true;
}

ItemTableDelegate::ItemTableDelegate(MainWindow *pMainWindow,QWidget* parent)
    :QStyledItemDelegate(parent),
     pMainWindow(pMainWindow){

}

QWidget *ItemTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const{
    if(index.column()==0){
        QComboBox* combo=new QComboBox(parent);
        int j=0;
        for(int i=0;i<256;i++){
            QString name(res->itemDictionary.entries[i].speciesName);
            if(name.length()){
                combo->addItem(QString("%1(%2)").
                               arg(i).
                               arg(name),
                               QVariant(i));
                combo->setItemData(j,res->itemBackground[KfMap::itemCatagory[i]],Qt::BackgroundRole);
                combo->setItemData(j,res->itemImages.images[i].small,Qt::DecorationRole);
                j++;
            }

        }
        //issue#22, bad imp, TODO
        connect(combo,&QComboBox::currentTextChanged,[=](){
            emit commitData(combo);
            emit closeEditor(combo);
        });
        return combo;
    }else if(index.column()==1){
        QComboBox* combo=new QComboBox(parent);
        int species=pMainWindow->map.itemAt(index.row()).basic.species();
        for(int p:res->itemDictionary.entries[species].behaviorName.keys()){
            combo->addItem(QString("%1(%2)").
                           arg(p).
                           arg(res->itemDictionary.entries[species].behaviorName[p]),
                  QVariant(p));
        }
        //issue#22, bad imp, TODO
        connect(combo,&QComboBox::currentTextChanged,[=](){
            emit commitData(combo);
            emit closeEditor(combo);
        });
        return combo;
    }
    else return QStyledItemDelegate::createEditor(parent,option,index);
}

void ItemTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const{
    if(index.column()==0){
        QComboBox* combo=qobject_cast<QComboBox*>(editor);
        for(int i=0;;i++){
            if(combo->itemData(i).toInt()==
                    pMainWindow->map.itemAt(index.row()).basic.species()){
                combo->setCurrentIndex(i);
                break;
            }
        }
    }else if(index.column()==1){
        QComboBox* combo=qobject_cast<QComboBox*>(editor);
        for(int i=0;;i++){
            if(combo->itemData(i).toInt()==
                    pMainWindow->map.itemAt(index.row()).basic.behavior()){
                combo->setCurrentIndex(i);
                break;
            }
        }
    }
    else return QStyledItemDelegate::setEditorData(editor,index);
}
void ItemTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index)const{
    if(index.column()==0 || index.column()==1){
        QComboBox* combo=qobject_cast<QComboBox*>(editor);
        model->setData(index,combo->currentData());
    }
    else return QStyledItemDelegate::setModelData(editor,model,index);
}

void MainWindow::loadRoomList(){
    ui->listRoom->clear();
    QString roomName[MAP_COUNT];
    QFile roomNameResFile(":/text/RoomName.txt");
    roomNameResFile.open(QIODevice::ReadOnly);
    QTextStream roomNameRes(&roomNameResFile);

    QChar command0;
    int command1;
    QStringList command12;
    auto readNextCommand=[&](){
        if(roomNameRes.atEnd()){
            command1=-1;
            return;
        }
        QString in=roomNameRes.readLine();
        command0=in[0];
        command12=in.right(in.size()-1).split('.');
        command1=command12[0].toInt();
    };

    int currentRoom=0;
    QTreeWidgetItem *currentParent=ui->listRoom->invisibleRootItem();
    readNextCommand();
    while(currentRoom<MAP_COUNT){
        if(currentRoom==command1){
            if(command0==QChar('}')){
                currentParent=currentParent->parent();
                if(!currentParent)currentParent=ui->listRoom->invisibleRootItem();
            }
            else if(command0==QChar('{')){
                currentParent=new QTreeWidgetItem(currentParent);
                currentParent->setText(0,command12[1]);
                currentParent->setData(0,Qt::UserRole,-1);
            }else if(command0==QChar('-')){
                QTreeWidgetItem* newRoom=new QTreeWidgetItem(currentParent);
                newRoom->setText(0,QString("Room#%1 %2").arg(currentRoom).arg(command12[1]));
                newRoom->setData(0,Qt::UserRole,currentRoom);
                currentRoom++;
            }
            readNextCommand();
        }else{
            QTreeWidgetItem* newRoom=new QTreeWidgetItem(currentParent);
            newRoom->setText(0,QString("Room#%1").arg(currentRoom));
            newRoom->setData(0,Qt::UserRole,currentRoom);
            currentRoom++;
        }
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    itemTableModal(this),
    ui(new Ui::MainWindow),
    mapUpdateTimer(this)
{
    ui->setupUi(this);
    ui->actionShowAnimation->setChecked(true);

    ui->mapView->pMainWindow=this;
    ui->mapView->pBlockStore=ui->blockStore;
    ui->blockStore->pMainWindow=this;

    ui->itemTable->setModel(&itemTableModal);
    ui->itemTable->setItemDelegate(new ItemTableDelegate(this));
    ui->itemTable->resizeRowsToContents();
    ui->itemTable->resizeColumnsToContents();

    ui->splitterMain->setStretchFactor(1,1);
    ui->splitterRight->setStretchFactor(0,1);

    connect(ui->mapView,SIGNAL(showStatusTip(const QString&)),
            ui->statusBar,SLOT(showMessage(const QString&)));
    connect(ui->blockStore,SIGNAL(showStatusTip(const QString&)),
            ui->statusBar,SLOT(showMessage(const QString&)));
    connect(ui->mapView,SIGNAL(selectItem(int)),
            this,SLOT(onSelectItem(int)));

    QToolButton* scrollAreaCornerResize=new QToolButton(this);
    scrollAreaCornerResize->setDefaultAction(ui->actionResizeMap);
    ui->mapViewScrollArea->setCornerWidget(scrollAreaCornerResize);

    MapOperation::pMap=&map;
    MapOperation::pMainWindow=this;

    connect(&mapUpdateTimer, SIGNAL(timeout()), this, SLOT(on_updateMap()));
    connect(ui->actionUndo, SIGNAL(triggered()), this, SLOT(undo()));
    connect(ui->actionRedo, SIGNAL(triggered()), this, SLOT(redo()));


    loadRoomList();

    clearOperationStack();


    //Open a file
    QString fileName=commandLineFile;
    std::FILE* file=fopenQ(fileName,"rb");
    if(!file)return;
    mapdata.fromFile(file);
    std::fclose(file);
    currentFileName=fileName;
    ui->actionSave->setEnabled(true);
    ui->actionSaveAs->setEnabled(true);
    ui->actionMakeRom->setEnabled(true);




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
        msgBox.setText(tr("Failed to load block set data."
                          "\nDefault block set data will be used."));
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

    ui->menuMap->setEnabled(true);

    mapUpdateTimer.start(5);
}

void MainWindow::on_listRoom_itemDoubleClicked(QTreeWidgetItem *item)
{
    if(currentFileName==QString::null)return;

    int roomId=item->data(0,Qt::UserRole).toInt();
    if(roomId==-1)return;
    if(map.isLoaded()){
        saveCurrentRoom();
    }
    loadRoom(roomId);


}

void MainWindow::on_updateMap(){
    static long lTime=0,timeA=0;
    long cTime;
    if(!showAnimation)return;
    cTime=std::clock();
    timeA+=cTime-lTime;
    if(timeA>1000)timeA=0;
    while(timeA>=16){
        timeA-=16;

        plt.tick();
        tiles.tick();
        if(bckScr.isLoaded()){
            bckPlt.tick();
            bckTiles.tick();
        }
        ui->mapView->update();
        ui->blockStore->update();

    }

    lTime=cTime;

}

void MainWindow::on_actionAboutMe_triggered()
{
    DialogAboutMe dlg;
    dlg.exec();
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName=QFileDialog::getOpenFileName(this, tr("Open File"),
        "",
        tr("mapdata File(*.bin)"));
    if(fileName==QString::null)return;
    openMapdata(fileName);
}

void MainWindow::openMapdata(QString fileName){
    std::FILE* file=fopenQ(fileName,"rb");
    if(file==nullptr){
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to open the file."));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    mapdata.fromFile(file);
    std::fclose(file);
    currentFileName=fileName;

    mapUpdateTimer.stop();
    map.unload();
    ui->menuMap->setEnabled(true);

    ui->actionSave->setEnabled(true);
    ui->actionSaveAs->setEnabled(true);
    ui->actionMakeRom->setEnabled(true);
    clearOperationStack();
}

void MainWindow::on_actionSave_triggered()
{
    if(currentFileName==QString::null)return;
    std::FILE* file=fopenQ(currentFileName,"wb");
    if(file==nullptr){
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to open the file."));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    if(map.isLoaded())saveCurrentRoom();
    mapdata.toFile(file);
    std::fclose(file);
}
void MainWindow::on_actionSaveAs_triggered(){
    QString fileName=QFileDialog::getSaveFileName(this, tr("Save As ..."),
        "",
        tr("mapdata File(*.bin)"));
    if(fileName==QString::null)return;
    std::FILE* file=fopenQ(fileName,"wb");
    if(file==nullptr){
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to open the file."));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    if(map.isLoaded())saveCurrentRoom();
    mapdata.toFile(file);
    std::fclose(file);
    currentFileName=fileName;
}


void MainWindow::on_actionShowEssence_triggered(bool checked)
{
    showEssence=checked;
    ui->mapView->update();
    ui->blockStore->update();
}
void MainWindow::on_actionShowScript_triggered(bool checked){
    showScript=checked;
    ui->mapView->update();
    ui->blockStore->update();
}

void MainWindow::on_actionShowAnimation_triggered(bool checked)
{
    showAnimation=checked;
}
void MainWindow::on_actionShowItem_triggered(bool checked)
{

    showItems=checked;
    ui->mapView->update();
}
void MainWindow::on_actionShowBackground_triggered(bool checked)
{

    showBackground=checked;
    ui->mapView->update();
}


void MainWindow::on_actionMakeRom_triggered()
{
    on_actionSave_triggered();
    DialogMakeRom dlg(currentFileName);
    dlg.exec();
}

void MainWindow::on_actionMapProperties_triggered()
{
    if(!map.isLoaded()){
        return;
    }
    DialogProperties dlg(map.metaData);
    if(QDialog::Accepted==dlg.exec()){
        MoEditMetaData mo(dlg.metaData);
        mo.toolTip=tr("Change Properties");
        doOperation(&mo);
    }
}


void MainWindow::on_actionEnglish_triggered(){
    QSettings settings("maprx.ini",QSettings::IniFormat);
    pApp->removeTranslator(&translator);
    ui->retranslateUi(this);
    settings.setValue("UI/LANG","en");
}

void MainWindow::on_actionChinese_triggered()
{
    QSettings settings("maprx.ini",QSettings::IniFormat);
    pApp->installTranslator(&translator);
    ui->retranslateUi(this);
    settings.setValue("UI/LANG","ch");
}
void MainWindow::on_actionExtract_triggered(){
    QString fileName=QFileDialog::getOpenFileName(this, tr("Select ROM"),
        "",
        tr("ROM File(*.nds *.bin);;Any files(*.*)"));
    if(fileName==QString::null)return;
    std::FILE* rom=fopenQ(fileName,"rb");
    if(rom==nullptr){
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to open the ROM."));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }

    fileName=QFileDialog::getSaveFileName(this, tr("Save mapdata to..."),
        "",
        tr("mapdata File(*.bin)"));
    if(fileName==QString::null)return;
    std::FILE* mapdataFile=fopenQ(fileName,"wb");
    if(mapdataFile==nullptr){
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to open mapdata file."));
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
    msgBox.setText(tr("Succeeded to extract mapdata from ROM."));
    msgBox.exec();

    openMapdata(fileName);
}

void MainWindow::on_buttonItemUp_clicked()
{
    if(!map.isLoaded())return;
    QModelIndex selection=ui->itemTable->currentIndex();
    if(!selection.isValid())return;
    u8 selItem=selection.row();
    if(selItem==0)return;
    if(selItem>=map.metaData.itemCount)return;
    MoSwapItem mo(selItem-1);
    mo.toolTip=QString(tr("Move up Item#%1")).arg(selItem);
    doOperation(&mo);

    ui->itemTable->setCurrentIndex(itemTableModal.getIndex(selItem-1,selection.column()));
}

void MainWindow::on_buttonItemDown_clicked()
{
    if(!map.isLoaded())return;
    QModelIndex selection=ui->itemTable->currentIndex();
    if(!selection.isValid())return;
    u8 selItem=selection.row();
    if(selItem==map.metaData.itemCount-1)return;
    if(selItem>=map.metaData.itemCount)return;
    MoSwapItem mo(selItem);
    mo.toolTip=QString(tr("Move down Item#%1")).arg(selItem);
    doOperation(&mo);

    ui->itemTable->setCurrentIndex(itemTableModal.getIndex(selItem+1,selection.column()));
}

void MainWindow::on_buttonItemDelete_clicked()
{
    if(!map.isLoaded())return;
    QModelIndex selection=ui->itemTable->currentIndex();
    if(!selection.isValid())return;
    u8 selItem=selection.row();
    if(selItem>=map.metaData.itemCount)return;
    MoDeleteItem mo(selItem);
    mo.toolTip=QString(tr("Remove Item#%1")).arg(selItem);
    doOperation(&mo);
}

void MainWindow::on_buttonItemNew_clicked()
{
    if(!map.isLoaded())return;
    KfMap::RipeItem item;
    QSize size=ui->mapViewScrollArea->size();
    int tx=(ui->mapViewScrollArea->horizontalScrollBar()->value()+size.width()/2);
    item.basic.x=(tx>=0?tx:0);
    int ty=(ui->mapViewScrollArea->verticalScrollBar()->value()+size.height()/2);
    item.basic.y=(ty>=0?ty:0);
    if(item.basic.x>map.metaData.width*24)item.basic.x=map.metaData.width*24;
    if(item.basic.y>map.metaData.height*24)item.basic.y=map.metaData.height*24;
    MoNewItem mo(map.metaData.itemCount,item);
    mo.toolTip=tr("Add Item");
    doOperation(&mo);
    ui->itemTable->setCurrentIndex(itemTableModal.getIndex(
                                       map.metaData.itemCount-1,0));
}

void MainWindow::on_itemTable_clicked(const QModelIndex &index)
{
    if(!map.isLoaded())return;
    u8 itemId=index.row();
    KfMap::Item& item=map.itemAt(itemId).basic;
    QSize size=ui->mapViewScrollArea->size();
    ui->mapViewScrollArea->horizontalScrollBar()->setValue(
                item.x-size.width()/2);
    ui->mapViewScrollArea->verticalScrollBar()->setValue(
                item.y-size.height()/2);

    if(index.column()==5){
        DialogScripts dlg(map.itemAt(itemId).scripts,this);
        dlg.setWindowTitle(QString(tr("Scripts for item#%1")).arg(itemId));
        if(dlg.exec()==QDialog::Accepted){
            MoEditItemScript mo(dlg.scripts,itemId);
            mo.toolTip=QString(tr("Edit Scripts for item#%1")).arg(itemId);
            doOperation(&mo);
        }
    }

}
void MainWindow::on_actionDiscardChanges_triggered(){
    if(!map.isLoaded())return;
    QMessageBox msgBox;
    msgBox.setText(tr("Do you really want to discard your changes on this room?"));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    if(msgBox.exec()!=QMessageBox::Yes)return;
    loadRoom(curRoomId);
}
void MainWindow::on_actionResizeMap_triggered(){
    if(!map.isLoaded())return;
    DialogResizeMap dlg(map.metaData.width,map.metaData.height);
    if(dlg.exec()!=QDialog::Accepted)return;
    MoResizeMap mo(dlg.mapWidth,dlg.mapHeight,dlg.hAlign,dlg.vAlign);
    mo.toolTip=tr("Resize Map");
    doOperation(&mo);
}
void MainWindow::on_actionSaveToImage_triggered(){
    if(!map.isLoaded())return;
    QString fileName=QFileDialog::getSaveFileName(this, tr("Save image to..."),
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
    ui->mapView->reset();
    ui->mapViewScrollArea->horizontalScrollBar()->setValue(0);
    ui->mapViewScrollArea->verticalScrollBar()->setValue(0);
}


void MainWindow::onSelectItem(int itemId){
    ui->itemTable->setCurrentIndex(itemTableModal.getIndex(itemId,0));
}
const char* exportMapMagic="KSSU";
void MainWindow::on_actionExportMap_triggered()
{
    if(!map.isLoaded())return;
    QString fileName=QFileDialog::getSaveFileName(this, tr("Export map to..."),
        "",
        tr("Binary file(*.bin)"));
    if(fileName==QString::null)return;
    u32 len;
    std::unique_ptr<u8[]> buf(map.generateFile(&len));
    QFile file(fileName);
    if(!file.open(QIODevice::ReadWrite)){
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to create the file."));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    file.resize(len+8);
    uchar* dst=file.map(0,len+8);
    std::memcpy(dst,exportMapMagic,4);
    std::memcpy(dst+4,&curRoomId,4);
    std::memcpy(dst+8,buf.get(),len);
}

void MainWindow::on_actionImportMap_triggered()
{
    if(!map.isLoaded())return;
    QString fileName=QFileDialog::getOpenFileName(this, tr("Import map from..."),
        "",
        tr("Binary file(*.bin)"));
    if(fileName==QString::null)return;
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to open the file."));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    const uchar* src=file.map(0,file.size());
    if(std::memcmp(src,exportMapMagic,4)!=0){
        QMessageBox msgBox;
        msgBox.setText(tr("The file is not a map."));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    int importRoomId;
    memcpy(&importRoomId,src+4,4);
    if(importRoomId!=curRoomId){
        QMessageBox msgBox;
        msgBox.setText(tr("The index of the map being imported is not equal to current map's, "
                       "and they probably use different block sets.\n"
                       "Do you still want to import this map?"));
        msgBox.setIcon(QMessageBox::Icon::Warning);
        msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if(msgBox.exec()!=QMessageBox::Yes)return;
    }
    KfMap importMap;
    importMap.readFile(src+8);
    MoPasteMap pasteMap(importMap);
    pasteMap.toolTip=tr("Import Map");
    doOperation(&pasteMap);
}
