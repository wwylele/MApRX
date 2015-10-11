/*************************************************************************
    dialogmakerom.cpp
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

#include "dialogmakerom.h"
#include "ui_dialogmakerom.h"
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QProcess>
#include "../Nitro.h"
#include <cassert>
#include <memory>
#include "main.h"

DialogMakeRom::DialogMakeRom(QString mapdataFileName,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMakeRom)
{
    ui->setupUi(this);
    ui->editSrcFile->setText(mapdataFileName);

    QSettings settings("maprx.ini",QSettings::IniFormat);
    ui->editRom->setText(settings.value("MAKEROM/ROM_PATH").toString());
    ui->editEmulator->setText(settings.value("MAKEROM/EMU_PATH").toString());
}

DialogMakeRom::~DialogMakeRom()
{
    delete ui;
}

void DialogMakeRom::on_buttonOpenRom_clicked()
{
    QSettings settings("maprx.ini",QSettings::IniFormat);
    QString fileName=QFileDialog::getOpenFileName(this, tr("Open ROM..."),
        "",
        "DS ROM(*.nds)");
    if(fileName==QString::null)return;
    ui->editRom->setText(fileName);
    settings.setValue("MAKEROM/ROM_PATH",fileName);
}

void DialogMakeRom::on_buttonEmulator_clicked()
{
    QSettings settings("maprx.ini",QSettings::IniFormat);
    QString fileName=QFileDialog::getOpenFileName(this, tr("Select Emulator..."),
        "",
        tr("Executable file(*.exe)"));
    if(fileName==QString::null)return;
    ui->editEmulator->setText(fileName);
    settings.setValue("MAKEROM/EMU_PATH",fileName);
}

void DialogMakeRom::on_buttonRun_clicked()
{
    if(!QProcess::startDetached(ui->editEmulator->text().replace("/","\\"),
                                QStringList()<<ui->editRom->text().replace("/","\\"))){
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to start the emulator."));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
    }
}

void DialogMakeRom::on_buttonMake_clicked()
{
    std::FILE *rom,*mapdata;
    rom=fopenQ(ui->editRom->text(),"rb+");
    if(rom==0){
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to open the ROM."));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    mapdata=fopenQ(ui->editSrcFile->text(),"rb");
    if(mapdata==0){
        QMessageBox msgBox;
        msgBox.setText(tr("Failed to open the mapdata file."));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.exec();
        return;
    }
    u32 len;
    std::fseek(mapdata,0,SEEK_END);
    len=std::ftell(mapdata);
    std::fseek(mapdata,0,SEEK_SET);
    std::unique_ptr<u8[]> buf(new u8[len]);
    std::fread(buf.get(),len,1,mapdata);
    std::fclose(mapdata);

    u16 fileId=nitroGetSubFileId(rom,"rom/map01/mapdata");
    assert(fileId!=0xFFFF);
    u32 dp,dl;
    dp=nitroGetSubFileOffset(rom,fileId,&dl);
    if(dl>=len){
        std::fseek(rom,dp,SEEK_SET);
        std::fwrite(buf.get(),len,1,rom);
    }else{
        ROM_HEADER rom_header;
        std::fseek(rom,0,SEEK_SET);
        std::fread(&rom_header,sizeof(rom_header),1,rom);
        dp=rom_header.ROMSize;
        rom_header.ROMSize+=len;
        rom_header.CRC16=nitroCrc16(&rom_header,offsetof(ROM_HEADER,headerCRC16));
        std::fseek(rom,0,SEEK_SET);
        std::fwrite(&rom_header,sizeof(rom_header),1,rom);
        nitroSetSubFileOffset(rom,fileId,dp,len);
        std::fseek(rom,dp,SEEK_SET);
        std::fwrite(buf.get(),len,1,rom);
    }



    std::fclose(rom);

    QMessageBox msgBox;
    msgBox.setText(tr("Success!"));
    msgBox.exec();
    return;
}
