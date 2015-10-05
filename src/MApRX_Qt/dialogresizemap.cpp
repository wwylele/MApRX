/*************************************************************************
    dialogresizemap.cpp
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
#include "dialogresizemap.h"
#include "ui_dialogresizemap.h"
#include <QMessageBox>
#include <map>

DialogResizeMap::DialogResizeMap(u16 mapWidth, u16 mapHeight, QWidget *parent) :
    QDialog(parent),
    mapWidth(mapWidth),
    mapHeight(mapHeight),
    ui(new Ui::DialogResizeMap)
{
    ui->setupUi(this);
    ui->editWidth->setText(QString::number(mapWidth));
    ui->editHeight->setText(QString::number(mapHeight));
    for(auto p:{ui->editWidth,ui->editHeight}){
        p->setValidator(new QIntValidator(0,32767));
    }
    ui->buttonAlign00->setChecked(true);
}

DialogResizeMap::~DialogResizeMap()
{
    delete ui;
}

void DialogResizeMap::on_buttonBox_accepted()
{
    mapWidth=ui->editWidth->text().toInt();
    mapHeight=ui->editHeight->text().toInt();
    if(mapWidth*mapHeight>32767){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("Cells count must be less than 32768!");
        msgBox.exec();
        return;
    }
    std::map<QAbstractButton*,std::pair<KfMap::Align,KfMap::Align>> alignMap;
    int j=0;
    for(QAbstractButton* button:{
        ui->buttonAlign00,
        ui->buttonAlign01,
        ui->buttonAlign02,
        ui->buttonAlign10,
        ui->buttonAlign11,
        ui->buttonAlign12,
        ui->buttonAlign20,
        ui->buttonAlign21,
        ui->buttonAlign22}){
        alignMap[button]=std::make_pair((KfMap::Align)(j%3-1),(KfMap::Align)(j/3-1));
        ++j;
    }
    std::pair<KfMap::Align,KfMap::Align>& align=alignMap[ui->buttonGroupAlign->checkedButton()];
    hAlign=align.first;
    vAlign=align.second;

    accept();
}
