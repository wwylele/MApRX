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



void MainWindow::clearOperationStack(){
    while(!undoStack.empty())undoStack.pop();
    while(!redoStack.empty())redoStack.pop();
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
}
void MainWindow::doOperation(MapOperation *op){
    undoStack.emplace(op->generateReversal());
    op->doOperation();
    ui->actionUndo->setEnabled(true);
    while(!redoStack.empty())redoStack.pop();
    ui->actionRedo->setEnabled(false);
    ui->mapPlane0->update();
}
void MainWindow::undo(){
    assert(!undoStack.empty());
    redoStack.emplace(undoStack.top().get()->generateReversal());
    undoStack.top().get()->doOperation();
    undoStack.pop();
    ui->actionRedo->setEnabled(true);
    ui->actionUndo->setEnabled(!undoStack.empty());
    ui->mapPlane0->update();
}
void MainWindow::redo(){
    assert(!redoStack.empty());
    undoStack.emplace(redoStack.top().get()->generateReversal());
    redoStack.top().get()->doOperation();
    redoStack.pop();
    ui->actionUndo->setEnabled(true);
    ui->actionRedo->setEnabled(!redoStack.empty());
    ui->mapPlane0->update();

}

