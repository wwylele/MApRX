/*************************************************************************
    dialogmakerom.h
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

#ifndef DIALOGMAKEROM_H
#define DIALOGMAKEROM_H

#include <QDialog>

namespace Ui {
class DialogMakeRom;
}

class DialogMakeRom : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMakeRom(QString, QWidget *parent = 0);
    ~DialogMakeRom();

private slots:
    void on_buttonOpenRom_clicked();

    void on_buttonEmulator_clicked();

    void on_buttonRun_clicked();

    void on_buttonMake_clicked();

private:
    Ui::DialogMakeRom *ui;
};

#endif // DIALOGMAKEROM_H
