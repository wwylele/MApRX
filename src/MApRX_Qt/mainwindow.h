/*************************************************************************
    mainwindow.h
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../KssuFile.h"
#include <QMainWindow>
#include <QListWidget>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <stack>
#include <memory>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    class MapOperation{
    public:
        static KfMap* pMap;
        virtual void doOperation()=0;
        virtual MapOperation* generateReversal()=0;//Call before doOperation
        virtual ~MapOperation();
    };
    class MoEditCell:public MapOperation{
    private:
        u16 x,y;
        u16 blockIdToBe;
    public:
        MoEditCell(u16 _x,u16 _y,u16 toBe);
        void doOperation();
        MapOperation* generateReversal();
    };

    std::stack<std::unique_ptr<MapOperation>> undoStack;//store the reversal of history operation
    std::stack<std::unique_ptr<MapOperation>> redoStack;//store the reversal of operation pop from undoStack

    void clearOperationStack();
    void doOperation(MapOperation *op);
public slots:
    void undo();
    void redo();


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    KfPlt plt,bckPlt;
    KfTileSet tiles,bckTiles;
    KfBlockSet blocks;
    KfBckScr bckScr;
    KfMap map;
    int curRoomId;

    bool showEssence=false;
    bool showScript=false;
    bool showAnimation=true;
    bool showItems=false;
    QPixmap essenceSheet;

    int selBlock;
protected:
    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent * event);
private slots:
    void on_listRoom_itemDoubleClicked(QListWidgetItem * item);
    void on_updateMap();

    void on_actionAbout_MApRX_triggered();

    void on_action_Open_triggered();

    void on_action_Save_triggered();

    void on_actionShow_Essence_triggered(bool checked);
    void on_actionShow_Script_triggered(bool checked);

    void on_actionShow_Animation_triggered(bool checked);

    void on_actionShow_Items_triggered(bool checked);

    void on_actionSave_As_triggered();

    void on_actionMake_Rom_triggered();

    void on_actionMap_Properties_triggered();

    void on_actionEnglish_triggered();

    void on_actionChinese_triggered();

    void on_actionExtract_triggered();

    void openMapdata(QString fileName);

private:
    Ui::MainWindow *ui;
    QTimer mapUpdateTimer;

    QString currentFileName;
    Kf_mapdata mapdata;
    void updateItemList();
    void saveCurrentRoom();


};

#endif // MAINWINDOW_H
