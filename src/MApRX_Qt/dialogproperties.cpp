/*************************************************************************
    dialogproperties.cpp
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

#include "dialogproperties.h"
#include "ui_dialogproperties.h"
#include <QMessageBox>
DialogProperties::DialogProperties(const KfMap::MetaData_Struct &c,QWidget *parent) :
    QDialog(parent),
    metaData(c),
    ui(new Ui::DialogProperties)
{
    ui->setupUi(this);
    ui->editClipLeft->setText(QString::number(metaData.clipLeft));
    ui->editClipTop->setText(QString::number(metaData.clipTop));
    ui->editClipBottom->setText(QString::number(metaData.clipBottom));
    ui->editClipRight->setText(QString::number(metaData.clipRight));
    for(auto p:{ui->editClipLeft,
        ui->editClipTop,
        ui->editClipBottom,
        ui->editClipRight})p->setValidator(new QIntValidator(0,65535));

    ui->editOffsetHori->setText(QString::number(metaData.bckOffsetHori));
    ui->editOffsetVert->setText(QString::number(metaData.bckOffsetVert));
    ui->editSpeedHori->setText(QString::number(metaData.bckSpeedHori));
    ui->editSpeedVert->setText(QString::number(metaData.bckSpeedVert));
    for(auto p:{ui->editOffsetHori,
        ui->editOffsetVert,
        ui->editSpeedHori,
        ui->editSpeedVert})p->setValidator(new QIntValidator(-32768,32767));

    ui->editPal0->setText(QString::number(metaData.itemPlts[0]));
    ui->editPal1->setText(QString::number(metaData.itemPlts[1]));
    ui->editPal2->setText(QString::number(metaData.itemPlts[2]));
    ui->editPal3->setText(QString::number(metaData.itemPlts[3]));
    ui->editPal4->setText(QString::number(metaData.itemPlts[4]));
    for(auto p:{ui->editPal0,
        ui->editPal1,
        ui->editPal2,
        ui->editPal3,
        ui->editPal4})p->setValidator(new QIntValidator(0,255));

    ui->checkEffect8->setChecked(metaData.globalEffect&0x80);
    ui->comboWind->addItem("Up",0);
    ui->comboWind->addItem("Right",1);
    ui->comboWind->addItem("Down",2);
    ui->comboWind->addItem("Left",3);
    ui->comboWind->setCurrentIndex((metaData.globalEffect&0x0C)>>2);
    ui->spinWind->setValue(metaData.globalEffect&3);

    for(int i=0;i<256;i++){
        QString str;
        str.sprintf("%d:",i);
        ui->comboBgm->addItem(str,i);
    }
    ui->comboBgm->setCurrentIndex(metaData.bgm);
}

DialogProperties::~DialogProperties()
{
    delete ui;
}


void DialogProperties::on_buttonBox_accepted()
{
    metaData.clipLeft=ui->editClipLeft->text().toInt();
    metaData.clipRight=ui->editClipRight->text().toInt();
    metaData.clipTop=ui->editClipTop->text().toInt();
    metaData.clipBottom=ui->editClipBottom->text().toInt();

    metaData.bckOffsetHori=ui->editOffsetHori->text().toInt();
    metaData.bckOffsetVert=ui->editOffsetVert->text().toInt();
    metaData.bckSpeedHori=ui->editSpeedHori->text().toInt();
    metaData.bckSpeedVert=ui->editSpeedVert->text().toInt();

    metaData.itemPlts[0]=ui->editPal0->text().toInt();
    metaData.itemPlts[1]=ui->editPal1->text().toInt();
    metaData.itemPlts[2]=ui->editPal2->text().toInt();
    metaData.itemPlts[3]=ui->editPal3->text().toInt();
    metaData.itemPlts[4]=ui->editPal4->text().toInt();

    metaData.bgm=ui->comboBgm->currentIndex();
    metaData.globalEffect=(ui->comboWind->currentIndex()<<2)
            |ui->spinWind->value()
            |(ui->checkEffect8->isChecked()?0x80:0);

    accept();
}
