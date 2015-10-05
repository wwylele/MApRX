/*************************************************************************
    dialogresizemap.h
    :"Resize Map" dialog

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
#ifndef DIALOGRESIZEMAP_H
#define DIALOGRESIZEMAP_H

#include <QDialog>
#include "../KssuFile.h"

namespace Ui {
class DialogResizeMap;
}

class DialogResizeMap : public QDialog
{
    Q_OBJECT

public:
    u16 mapWidth,mapHeight;
    KfMap::Align hAlign,vAlign;
    explicit DialogResizeMap(u16 width, u16 height, QWidget *parent = 0);
    ~DialogResizeMap();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::DialogResizeMap *ui;
};

#endif // DIALOGRESIZEMAP_H
