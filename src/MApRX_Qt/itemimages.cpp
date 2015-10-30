#include "itemimages.h"
#include <QBitmap>
ItemImages::ItemImages()
{

}
int ItemImages::smallImageSize=24;
void ItemImages::load(const QImage &srcImage, QTextStream &srcDesc){
    for(ItemImage& i:images)i.loaded=false;
    QImage mask=srcImage.createMaskFromColor(srcImage.pixel(0,0));
    QPixmap pixmap=QPixmap::fromImage(srcImage);
    pixmap.setMask(QBitmap::fromImage(mask));

    while(!srcDesc.atEnd()){
        QString in;
        QStringList inl;
        in=srcDesc.readLine();
        inl=in.split('.');
        int currentSpec=inl[0].toInt();
        int x,y,w,h,dx,dy;
        x=inl[1].toInt();
        y=inl[2].toInt();
        w=inl[3].toInt();
        h=inl[4].toInt();
        if(inl.size()>5){
            dx=inl[5].toInt();
            dy=inl[6].toInt();
        }else{
            dx=w/2;
            dy=h/2;
        }
        images[currentSpec].dx=dx;
        images[currentSpec].dy=dy;
        images[currentSpec].large=pixmap.copy(x,y,w,h);
        images[currentSpec].small=pixmap.copy(
                    x+(w-smallImageSize)/2,
                    y+(h-smallImageSize)/2,
                    smallImageSize,
                    smallImageSize
                    );
        images[currentSpec].loaded=true;
    }
}
