#include <mainwindow.h>
#include "ui_mainwindow.h"
#include <assert.h>
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
    ui->actionUndo->setToolTip("Undo "+op->toolTip);
    ui->actionRedo->setToolTip("Redo");
    ui->mapPlane0->update();
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
        ui->actionUndo->setToolTip("Undo");
    }else{
        ui->actionUndo->setToolTip("Undo "+undoStack.top().get()->toolTip);
    }
    ui->actionRedo->setToolTip("Redo "+redoStack.top().get()->toolTip);
    ui->mapPlane0->update();
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
        ui->actionRedo->setToolTip("Redo");
    }else{
        ui->actionRedo->setToolTip("Redo "+redoStack.top().get()->toolTip);
    }
    ui->actionUndo->setToolTip("Undo "+undoStack.top().get()->toolTip);
    ui->mapPlane0->update();

}

