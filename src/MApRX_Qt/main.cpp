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

QTranslator translator;
QApplication* pApp;
int main(int argc, char *argv[])
{
    /*
     * Are you ready?
     * ~~~~~~~~~~~~~~
     */

    QApplication a(argc, argv);
    pApp=&a;
    translator.load("maprx_zh",":/");
    //a.installTranslator(&translator);
    MainWindow w;
    w.show();

    return a.exec();
}


std::FILE *fopenQ(const QString& name, const char *mode){
    QByteArray nameLocale=name.toLocal8Bit();
    nameLocale.push_back('\0');
    return std::fopen(nameLocale.data(),mode);
}
