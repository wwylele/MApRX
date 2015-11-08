/*************************************************************************
    blockstore.h
    :A widget contain blocks

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

#ifndef BLOCKSTORE_H
#define BLOCKSTORE_H

#include <QWidget>
#include <QMouseEvent>
#include "../KssuFile.h"
#include "MainWindow.h"

class BlockStore : public QWidget
{
    Q_OBJECT
public:
    explicit BlockStore(QWidget *parent = 0);
    void reset();
    MainWindow* pMainWindow;

signals:
    void showStatusTip(const QString& message);
protected:
    int curBlock;
    int width;
    int height;
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void leaveEvent(QEvent * event);
public slots:
};

#endif // BLOCKSTORE_H
