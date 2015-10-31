/*************************************************************************
    main.h
    :Qt frontend utility

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
#ifndef MAIN_H
#define MAIN_H

#include <cstdio>
#include <QApplication>
#include <QTranslator>
#include "itemdictionary.h"
#include "itemimages.h"
std::FILE *fopenQ(const QString& name,const char* mode);

extern QTranslator translator;
extern QApplication* pApp;
extern QString commandLineFile;

class CommonResources{
public:
    ItemDictionary itemDictionary;
    ItemImages itemImages;
    QPixmap essenceSheet;
    QBrush itemBackground[13]{
                        QColor(255,255,255),
                        QColor(128,255,128),
                        QColor(255,255,128),
                        QColor(255,128,128),
                        QColor(255,128,255),
                        QColor(128,128,255),
                        QColor(128,255,255),
                        Qt::transparent,
                        Qt::transparent,
                        Qt::transparent,
                        QColor(128,128,128),
                        QColor(192,192,192),
                        QColor(255,192,128)
    };
    CommonResources();
    int version[4];
};
extern CommonResources* res;

#endif // MAIN_H

