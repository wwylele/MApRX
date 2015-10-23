#ifndef ITEMIMAGES_H
#define ITEMIMAGES_H

#include <QImage>
#include <QPixmap>
#include <QTextStream>
class ItemImages
{
public:
    ItemImages();
    void load(const QImage& srcImage,QTextStream& srcDesc);
    struct{
        QPixmap large;
        QPixmap small;
        int dx,dy;
    }images[256];
    static int smallImageSize;

signals:

public slots:
};

#endif // ITEMIMAGES_H
