/*************************************************************************
    dialogscripts.h
    :"Scripts" dialog

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
#ifndef DIALOGSCRIPTS_H
#define DIALOGSCRIPTS_H

#include <QDialog>
#include "../KssuFile.h"
#include <QStyledItemDelegate>
#include <QPainter>
#include <QMenu>
#include "mainwindow.h"
namespace Ui {
class DialogScripts;
}
class ScriptDelegate:public QStyledItemDelegate{
    Q_OBJECT
private:
    MainWindow* pMainWindow;

    static QString scriptText[7];
    static QString scriptToString(const KfMap::Script& script);
public:
    ScriptDelegate(MainWindow* pMainWindow,QWidget *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;
};
class DialogScripts : public QDialog
{
    Q_OBJECT

public:
    std::vector<KfMap::Script> scripts;
    explicit DialogScripts(const std::vector<KfMap::Script> _scripts,
                           MainWindow* pMainWindow, QWidget *parent = 0);
    ~DialogScripts();

private slots:
    void on_buttonBox_accepted();

    void on_buttonRemove_clicked();

    void on_actionAddScript1_triggered();

    void on_actionAddScript2_triggered();

    void on_actionAddScript3_triggered();

    void on_actionAddScript4_triggered();

    void on_actionAddScript5_triggered();

    void on_actionAddScript6_triggered();

private:
    Ui::DialogScripts *ui;
    MainWindow* pMainWindow;
    QMenu menu;
    void addScript(const KfMap::Script& script);
};

#endif // DIALOGSCRIPTS_H
