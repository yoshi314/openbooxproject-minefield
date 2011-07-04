/*  Copyright (C) 2011  OpenBOOX
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef EXPLORER_VIEW_H_
#define EXPLORER_VIEW_H_

#include <QtGui/QWidget>

#include "tree_view.h"

#include "onyx/ui/system_actions.h"
#include "onyx/ui/status_bar.h"

using namespace ui;

namespace obx
{

typedef enum
{
    CAT_HOME = 0,
    CAT_INT_FLASH,
    CAT_SD_CARD,
    CAT_BOOKS,
    CAT_APPS,
    CAT_GAMES,
    CAT_WEBSITES
} CategoryType;

class ExplorerView : public QWidget
{
    Q_OBJECT

public:
    ExplorerView(QWidget *parent = 0);
    ~ExplorerView();

public:
    void isMainUI(bool mainUI);

private Q_SLOTS:
    void onItemActivated(const QModelIndex &);
    void onPositionChanged(int currentPage, int pages);
    void onAboutToSuspend();
    void onAboutToShutDown();
    void popupMenu();

private:
    void showHome();
    void showFiles(CategoryType, QString);
    void showBooks(CategoryType, QString);
    void showApps(CategoryType, QString);
    void showWebsites(CategoryType, QString);
    bool openDataFile(QString);
    int  run(const QString &command, const QStringList & parameters);
    void keyPressEvent(QKeyEvent *ke);
    void keyReleaseEvent(QKeyEvent *ke);

private:
    QVBoxLayout        vbox_;
    QStandardItemModel model_;
    ObxTreeView        treeview_;
    StatusBar          status_bar_;

    bool               mainUI_;
    CategoryType       curr_category_;
    int                home_row_;
    QString            current_path_;
};

class ExplorerSplash : public QWidget
{
public:
    ExplorerSplash(QPixmap pixmap, QWidget *parent = 0);
    ~ExplorerSplash();

private:
    void paintEvent(QPaintEvent *);

private:
    QPixmap pixmap_;
};

}

#endif // EXPLORER_VIEW_H_
