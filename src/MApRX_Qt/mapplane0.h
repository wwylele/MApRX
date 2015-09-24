/*************************************************************************
    mapplane0.h
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
#include "MainWindow.h"
#include "../KssuFile.h"

class MapPlane0 : public QWidget
{
    Q_OBJECT
public:
    explicit MapPlane0(QWidget *parent = 0);
    void reset();
    MainWindow* pMainWindow;

protected:
    int curX,curY;
    int width;
    int height;
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void leaveEvent(QEvent * event);
signals:

public slots:
};

#endif // MAPPLANE0_H
