/*************************************************************************
    dialogaboutme.h
    :"About MApRX" dialog

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

#ifndef DIALOGABOUTME_H
#define DIALOGABOUTME_H

#include <QDialog>

namespace Ui {
class DialogAboutMe;
}

class DialogAboutMe : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAboutMe(QWidget *parent = 0);
    ~DialogAboutMe();

private:
    Ui::DialogAboutMe *ui;
};

#endif // DIALOGABOUTME_H
