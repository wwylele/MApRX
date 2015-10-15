/*************************************************************************
    itemdictionary.cpp
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
#include "itemdictionary.h"



void ItemDictionary::load(QTextStream& stream){
    for(ItemEntry& entry:entries){
        entry.speciesName="";
        entry.behaviorName.clear();
    }
    int currentSpecies=0;
    while(!stream.atEnd()){
        QString in;
        QStringList inl;
        in=stream.readLine();
        inl=in.split('.');
        if(inl[0][0]==QChar('+')){
            currentSpecies=inl[0]
                    .rightRef(inl[0].size()-1)
                    .toInt();
            entries[currentSpecies].speciesName
                    =inl[1];
        }else{
            entries[currentSpecies]
                    .behaviorName[inl[0].toInt()]
                    =inl[1];
        }
    }
}
