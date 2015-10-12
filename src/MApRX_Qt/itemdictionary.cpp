#include "itemdictionary.h"



void ItemDictionary::Load(QTextStream& stream){
    for(ItemEntry& entry:entries){
        entry.speciesName="";
        entry.behaviorName.clear();
    }
    int currentSpecies;
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
