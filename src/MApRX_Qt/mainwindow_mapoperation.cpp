/*************************************************************************
    mainwindow_mapoperation
    :implement for MainWindow::MapOperation classes and Undo/Redo

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
#include <mainwindow.h>
#include "ui_mainwindow.h"
#include <cassert>
KfMap* MainWindow::MapOperation::pMap=0;
MainWindow* MainWindow::MapOperation::pMainWindow=0;
MainWindow::MapOperation::~MapOperation(){

}
MainWindow::MoEditCell::MoEditCell(u16 _x,u16 _y,u16 toBe)
    :x(_x),y(_y),blockIdToBe(toBe){

}

void MainWindow::MoEditCell::doOperation(){
    pMap->at(x,y).blockId=blockIdToBe;
}

MainWindow::MapOperation* MainWindow::MoEditCell::generateReversal(){
    return new MoEditCell(x,y,pMap->at(x,y).blockId);
}

MainWindow::MoEditItemBasic::MoEditItemBasic(u8 _itemId, const KfMap::Item &toBe)
    :itemId(_itemId),itemBasicToBe(toBe){

}
void MainWindow::MoEditItemBasic::doOperation(){
    pMap->Items(itemId).basic=itemBasicToBe;
    pMainWindow->itemTableModal.itemChanged(itemId);
}
MainWindow::MapOperation* MainWindow::MoEditItemBasic::generateReversal(){
    return new MoEditItemBasic(itemId,pMap->Items(itemId).basic);
}
MainWindow::MoSwapItem::MoSwapItem(u8 _firstItemId):
    firstItemId(_firstItemId){

}

void MainWindow::MoSwapItem::doOperation(){
    pMap->swapItem(firstItemId);
    pMainWindow->itemTableModal.itemChanged(firstItemId);
    pMainWindow->itemTableModal.itemChanged(firstItemId+1);
}

MainWindow::MapOperation* MainWindow::MoSwapItem::generateReversal(){
    return new MoSwapItem(firstItemId);
}


MainWindow::MoDeleteItem::MoDeleteItem(u8 _itemId)
    :itemId(_itemId){

}

void MainWindow::MoDeleteItem::doOperation(){
    pMap->deleteItem(itemId);
    emit pMainWindow->itemTableModal.layoutChanged();
}

MainWindow::MapOperation* MainWindow::MoDeleteItem::generateReversal(){
    return new MoNewItem(itemId,pMap->Items(itemId));
}


MainWindow::MoNewItem::MoNewItem(u8 _itemId,const KfMap::RipeItem& item)
    :itemId(_itemId),itemToInsert(item){

}

void MainWindow::MoNewItem::doOperation(){
    pMap->newItem(itemId,itemToInsert);
    emit pMainWindow->itemTableModal.layoutChanged();
}

MainWindow::MapOperation* MainWindow::MoNewItem::generateReversal(){
    return new MoDeleteItem(itemId);
}

MainWindow::MoEditMetaData::MoEditMetaData(const KfMap::MetaData_Struct &metaData)
    :metaDataToBe(metaData){

}
void MainWindow::MoEditMetaData::doOperation(){
    pMap->metaData=metaDataToBe;
}
MainWindow::MapOperation* MainWindow::MoEditMetaData::generateReversal(){
    return new MoEditMetaData(pMap->metaData);
}



MainWindow::MoResizeMap::MoResizeMap(u8 width,u8 height,KfMap::Align hA,KfMap::Align vA)
    :widthToBe(width),heightToBe(height),hAlign(hA),vAlign(vA){

}

void MainWindow::MoResizeMap::doOperation(){
    pMap->resizeMap(widthToBe,heightToBe,hAlign,vAlign);
    emit pMainWindow->itemTableModal.layoutChanged();
    pMainWindow->resetMap();
}

MainWindow::MapOperation* MainWindow::MoResizeMap::generateReversal(){
    return new MoPasteMap(*pMap);
}


MainWindow::MoPasteMap::MoPasteMap(const KfMap& map)
    :mapToBe(map){

}

void MainWindow::MoPasteMap::doOperation(){
    *pMap=mapToBe;
    emit pMainWindow->itemTableModal.layoutChanged();
    pMainWindow->resetMap();
}

MainWindow::MapOperation* MainWindow::MoPasteMap::generateReversal(){
    return new MoPasteMap(*pMap);
}


MainWindow::MoEditCellScript::MoEditCellScript(const std::vector<KfMap::Script>& scripts,u16 x,u16 y)
    :scriptsToBe(scripts),x(x),y(y){

}

void MainWindow::MoEditCellScript::doOperation(){
    pMap->at(x,y).scripts=scriptsToBe;
}

MainWindow::MapOperation* MainWindow::MoEditCellScript::generateReversal(){
    return new MoEditCellScript(pMap->at(x,y).scripts,x,y);
}


MainWindow::MoEditItemScript::MoEditItemScript(const std::vector<KfMap::Script>& scripts,u8 itemId)
    :scriptsToBe(scripts),itemId(itemId){

}

void MainWindow::MoEditItemScript::doOperation(){
    pMap->Items(itemId).scripts=scriptsToBe;
}

MainWindow::MapOperation* MainWindow::MoEditItemScript::generateReversal(){
    return new MoEditItemScript(pMap->Items(itemId).scripts,itemId);
}


//////////////////////////////////////////////////////////////////////

void MainWindow::clearOperationStack(){
    while(!undoStack.empty())undoStack.pop();
    while(!redoStack.empty())redoStack.pop();
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
}
void MainWindow::doOperation(MapOperation *op){
    MapOperation *opRev=op->generateReversal();
    opRev->toolTip=op->toolTip;
    undoStack.emplace(opRev);
    op->doOperation();
    ui->actionUndo->setEnabled(true);
    while(!redoStack.empty())redoStack.pop();
    ui->actionRedo->setEnabled(false);
    ui->actionUndo->setToolTip(tr("Undo")+" "+op->toolTip);
    ui->actionRedo->setToolTip(tr("Redo"));
    ui->mapView->update();
}
void MainWindow::undo(){
    assert(!undoStack.empty());
    MapOperation *opRev=undoStack.top().get()->generateReversal();
    opRev->toolTip=undoStack.top().get()->toolTip;
    redoStack.emplace(opRev);
    undoStack.top().get()->doOperation();
    undoStack.pop();
    ui->actionRedo->setEnabled(true);
    ui->actionUndo->setEnabled(!undoStack.empty());
    if(undoStack.empty()){
        ui->actionUndo->setToolTip(tr("Undo"));
    }else{
        ui->actionUndo->setToolTip(tr("Undo")+" "+undoStack.top().get()->toolTip);
    }
    ui->actionRedo->setToolTip(tr("Redo")+" "+redoStack.top().get()->toolTip);
    ui->mapView->update();
}
void MainWindow::redo(){
    assert(!redoStack.empty());
    MapOperation *opRev=redoStack.top().get()->generateReversal();
    opRev->toolTip=redoStack.top().get()->toolTip;
    undoStack.emplace(opRev);
    redoStack.top().get()->doOperation();
    redoStack.pop();
    ui->actionUndo->setEnabled(true);
    ui->actionRedo->setEnabled(!redoStack.empty());
    if(redoStack.empty()){
        ui->actionRedo->setToolTip(tr("Redo"));
    }else{
        ui->actionRedo->setToolTip(tr("Redo")+" "+redoStack.top().get()->toolTip);
    }
    ui->actionUndo->setToolTip(tr("Undo")+" "+undoStack.top().get()->toolTip);
    ui->mapView->update();

}

