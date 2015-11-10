/*************************************************************************
    mapview.h
    :A widget present the map

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

#ifndef MAPPLANE0_H
#define MAPPLANE0_H

#include <QWidget>
#include <QImage>
#include "MainWindow.h"
#include "blockstore.h"
#include "../KssuFile.h"

class MapView : public QWidget
{
    Q_OBJECT
public:
    explicit MapView(QWidget *parent = 0);
    void reset();
    MainWindow* pMainWindow;
    BlockStore* pBlockStore;
protected:
    int curX,curY;
    int curItem;
    int width;
    int height;
    bool itemDraging,itemShaked;
    int dragX,dragY;
#define TRAN_PAT_GRID_SIZE 6
    QImage transparentPattern;
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void leaveEvent(QEvent * event);
    QString generateStatusTip(u16 x,u16 y);
signals:
    void showStatusTip(const QString& message);
    void selectItem(int itemId);
    void itemDragging(bool isDragging);

public slots:
};

#endif // MAPPLANE0_H
