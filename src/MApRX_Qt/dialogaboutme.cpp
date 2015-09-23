/*************************************************************************
    dialogaboutme.cpp
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

#include "dialogaboutme.h"
#include "ui_dialogaboutme.h"

DialogAboutMe::DialogAboutMe(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAboutMe)
{
    ui->setupUi(this);
    ui->aboutMeText->setText(
                "MApRX<br>"
                "version 1.0<br>"
                "by wwylele<br>"
                "Compiled: " __DATE__" " __TIME__ "<br><br>"

                "Menu and tootbar icons by "
                      "<a href=\"http://icons8.com/\">"
                      "icons8</a><br><br>"

                "Copyright 2015 wwylele<br>"
                "This program comes with ABSOLUTELY NO WARRANTY.<br>"
                "This is free software, and you are welcome<br>"
                "to redistribute it under certain conditions."
                );
}

DialogAboutMe::~DialogAboutMe()
{
    delete ui;
}
