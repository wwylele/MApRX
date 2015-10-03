#ifndef DIALOGRESIZEMAP_H
#define DIALOGRESIZEMAP_H

#include <QDialog>
#include "../KssuFile.h"

namespace Ui {
class DialogResizeMap;
}

class DialogResizeMap : public QDialog
{
    Q_OBJECT

public:
    u16 mapWidth,mapHeight;
    KfMap::Align hAlign,vAlign;
    explicit DialogResizeMap(u16 width, u16 height, QWidget *parent = 0);
    ~DialogResizeMap();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::DialogResizeMap *ui;
};

#endif // DIALOGRESIZEMAP_H
