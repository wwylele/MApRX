/*************************************************************************
    main.cpp
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

#include "mainwindow.h"
#include "main.h"
#include <QSettings>
#include <QFile>
#include <QTextStream>

QTranslator translator;
QApplication* pApp;
QString commandLineFile;
CommonResources* res=0;
int main(int argc, char *argv[])
{
    /*
     * Are you ready?
     * ~~~~~~~~~~~~~~
     */
    if(argc>1)commandLineFile=argv[1];
    QApplication a(argc, argv);
    pApp=&a;
    CommonResources theRes;
    res=&theRes;
    translator.load("maprx_zh",":/");
    QSettings settings("maprx.ini",QSettings::IniFormat);
    if(settings.value("UI/LANG","en").toString()=="ch")
        a.installTranslator(&translator);
    MainWindow w;
    w.show();

    return a.exec();
}


std::FILE *fopenQ(const QString& name, const char *mode){
    QByteArray nameLocale=name.toLocal8Bit();
    nameLocale.push_back('\0');
    return std::fopen(nameLocale.data(),mode);
}

CommonResources::CommonResources(){
    QFile itemDicResFile(":/text/itemdic.txt");
    itemDicResFile.open(QIODevice::ReadOnly);
    QTextStream itemDicRes(&itemDicResFile);
    itemDictionary.load(itemDicRes);
    itemDicResFile.close();

    QFile itemImageResFile(":/text/itemimage.txt");
    itemImageResFile.open(QIODevice::ReadOnly);
    QTextStream itemImageRes(&itemImageResFile);
    itemImages.load(QImage(":/image/itemimage.png"),itemImageRes);
    itemImageResFile.close();

    essenceSheet.load(":/image/Essence.png");

    QFile versionFile(":/text/version.txt");
    versionFile.open(QIODevice::ReadOnly);
    QTextStream versionText(&versionFile);
    QString versionString=versionText.readLine();
    QStringList sl=versionString.split(",");
    for(int i=0;i<4;i++)version[i]=sl[i].toInt();
    versionFile.close();
}
