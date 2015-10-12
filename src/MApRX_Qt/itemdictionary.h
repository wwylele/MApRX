#ifndef ITEMDICTIONARY_H
#define ITEMDICTIONARY_H

#include <QString>
#include <QTextStream>
#include <QMap>


class ItemDictionary
{
public:
    class ItemEntry{
    public:
        QString speciesName;
        QMap<int,QString> behaviorName;
    }entries[256];
    void Load(QTextStream& stream);
};

#endif // ITEMDICTIONARY_H
