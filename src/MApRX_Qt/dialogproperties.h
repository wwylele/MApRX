/*************************************************************************
    dialogproperties.h
    :"Map Properties" dialog

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

#ifndef DIALOGPROPERTIES_H
#define DIALOGPROPERTIES_H

#include <QDialog>
#include "../KssuFile.h"

namespace Ui {
class DialogProperties;
}

class DialogProperties : public QDialog
{
    Q_OBJECT

public:
    explicit DialogProperties(const KfMap::MetaData_Struct &metaData,QWidget *parent = 0);
    ~DialogProperties();

    KfMap::MetaData_Struct metaData;
private slots:
    void on_buttonBox_accepted();

private:
    Ui::DialogProperties *ui;
};

#endif // DIALOGPROPERTIES_H
