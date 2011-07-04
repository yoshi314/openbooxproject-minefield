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

#include <QtSql/QSqlQuery>

#include "explorer_view.h"
#include "boox_action.h"
#include "about_dialog.h"
#include "obx_explorer.h"

#include "onyx/sys/sys.h"
#include "onyx/sys/sys_utils.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/menu.h"
#include "onyx/wireless/wifi_dialog.h"

using namespace ui;

namespace obx
{

enum
{
    FILE_DELETE = 0,
};
enum
{
    ORG_ADD2APPS = 0,
    ORG_ADD2GAMES,
    ORG_REMOVE
};
enum
{
    SET_WIFI = 0,
    SET_ABOUT
};

ExplorerView::ExplorerView(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , vbox_(this)
    , model_(0)
    , treeview_(0, 0)
    , status_bar_(this, MENU | PROGRESS | MESSAGE | BATTERY)
    , home_row_(0)
{
    QSize size = qApp->desktop()->screenGeometry().size();

//    qDebug() << "Rotation: " << SysStatus::instance().screenTransformation();
//    qDebug() << "Dimensions: " << size.width() << "x" << size.height();

    resize(size);

    connect(&treeview_, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(onItemActivated(const QModelIndex &)));
    connect(&treeview_, SIGNAL(positionChanged(int, int)),
            this, SLOT(onPositionChanged(int, int)));

    connect(&status_bar_, SIGNAL(menuClicked()), this, SLOT(popupMenu()));
    SysStatus &sys_status = SysStatus::instance();
    connect(&sys_status, SIGNAL(aboutToSuspend()), this, SLOT(onAboutToSuspend()));
    connect(&sys_status, SIGNAL(aboutToShutdown()), this, SLOT(onAboutToShutDown()));

    treeview_.showHeader(true);
    treeview_.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    vbox_.setSpacing(0);
    vbox_.setContentsMargins(0, 0, 0, 0);
    vbox_.addWidget(&treeview_);
    vbox_.addWidget(&status_bar_);

    showHome();

    status_bar_.setProgress(treeview_.currentPage(), treeview_.pages());
}

ExplorerView::~ExplorerView()
{
}

void ExplorerView::isMainUI(bool mainUI)
{
    mainUI_ = mainUI;
}

void ExplorerView::showHome()
{
    int i = 0;
    QFont itemFont;
    itemFont.setPointSize(22);

    curr_category_ = CAT_HOME;

    model_.clear();
    model_.setColumnCount(1);

    QSqlQuery query("SELECT name, icon, id FROM categories WHERE name <> '' ORDER BY id");

    while (query.next())
    {
        QStandardItem *item = new QStandardItem();
        item->setText(query.value(0).toString());
        item->setIcon(QIcon(query.value(1).toString()));
        item->setData(query.value(2).toInt());
        item->setEditable(false);
        item->setFont(itemFont);
        item->setTextAlignment(Qt::AlignLeft);
        model_.setItem(i++, 0, item);
    }
    model_.setHeaderData(0, Qt::Horizontal, "/", Qt::DisplayRole);

    if (model_.rowCount())
    {
        treeview_.hide();
    }
    treeview_.update();
    treeview_.setModel(&model_);
    treeview_.show();

    QModelIndex index = model_.index(home_row_, 0);
    if (index.isValid())
    {
        treeview_.select(index);
    }
}

void ExplorerView::showFiles(CategoryType category, QString path)
{
    QFont itemFont;
    itemFont.setPointSize(20);

    curr_category_ = category;

    model_.clear();
    model_.setColumnCount(1);

    QDir dir(path);
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name | QDir::DirsFirst | QDir::IgnoreCase);

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QStandardItem *item = new QStandardItem();
        item->setText(fileInfo.fileName());
        if (fileInfo.isDir())
        {
            item->setIcon(QIcon("/usr/share/explorer/images/middle/directory.png"));
        }
        else
        {
            QString extIcon = QString("/usr/share/explorer/images/middle/%1.png").arg(fileInfo.suffix());

            if (QFile::exists(extIcon))
            {
                item->setIcon(QIcon(extIcon));
            }
            else if (fileInfo.isExecutable())
            {
                item->setIcon(QIcon("/usr/share/explorer/images/middle/runnable_file.png"));
            }
            else
            {
                item->setIcon(QIcon("/usr/share/explorer/images/middle/unknown_document.png"));
            }
        }
        QBitArray properties(2);
        properties[0] = fileInfo.isDir();
        properties[1] = fileInfo.isExecutable();
        item->setData(properties);
        item->setEditable(false);
        item->setFont(itemFont);
        item->setTextAlignment(Qt::AlignLeft);
        model_.setItem(i, 0, item);
    }
    model_.setHeaderData(0, Qt::Horizontal, path, Qt::DisplayRole);

    if (model_.rowCount())
    {
        treeview_.hide();
    }
    treeview_.update();
    treeview_.setModel(&model_);
    treeview_.show();

    QModelIndex index = model_.index(0, 0);
    if (index.isValid())
    {
        treeview_.select(index);
    }
}

void ExplorerView::showBooks(CategoryType category, QString name)
{
    int i = 0;
    QFont itemFont;
    itemFont.setPointSize(20);

    curr_category_ = category;

    QSqlDatabase db = QSqlDatabase::database("ONYX");
    QSqlQuery query(db);
    query.exec("SELECT location, name, authors FROM content WHERE name <> '' ORDER BY upper(name), name");

    model_.clear();
    model_.setColumnCount(1);

    while (query.next())
    {
        QString fullFileName = query.value(0).toString() + "/" + query.value(1).toString();
        if (QFile::exists(fullFileName))
        {
            QFileInfo name(fullFileName);
            QStandardItem *item = new QStandardItem();
            item->setIcon(QIcon(QString("/usr/share/explorer/images/middle/%1.png").arg(name.suffix())));
            item->setText(name.baseName());
            item->setData(fullFileName);
            if (!query.value(2).toString().isEmpty())
            {
                item->setToolTip("By: " + query.value(2).toString());
            }
            item->setEditable(false);
            item->setFont(itemFont);
            item->setTextAlignment(Qt::AlignLeft);
            model_.setItem(i, 0, item);
            i++;
        }
    }
    model_.setHeaderData(0, Qt::Horizontal, "/" + name, Qt::DisplayRole);

    if (model_.rowCount())
    {
        treeview_.hide();
    }
    treeview_.update();
    treeview_.setModel(&model_);
    treeview_.show();

    QModelIndex index = model_.index(0, 0);
    if (index.isValid())
    {
        treeview_.select(index);
    }
}

void ExplorerView::showApps(CategoryType category, QString name)
{
    int i = 0;
    QFont itemFont;
    itemFont.setPointSize(22);

    curr_category_ = category;

    QSqlQuery query(QString("SELECT name, icon, executable, category_id FROM applications "
                            "WHERE category_id = '%1' ORDER BY upper(name), name").arg(category));

    model_.clear();
    model_.setColumnCount(1);

    while (query.next())
    {
        if (QFile::exists(query.value(2).toString()))
        {
            QStandardItem *item = new QStandardItem();
            item->setText(query.value(0).toString());
            if (QFile::exists(query.value(1).toString()))
            {
                item->setIcon(QIcon(query.value(1).toString()));
            }
            else
            {
                if (category == 3)
                {
                    item->setIcon(QIcon("/usr/share/explorer/images/middle/games.png"));
                }
                else
                {
                    item->setIcon(QIcon("/usr/share/explorer/images/middle/runnable_file.png"));
                }
            }
            item->setData(query.value(2).toString());
            item->setEditable(false);
            item->setFont(itemFont);
            item->setTextAlignment(Qt::AlignLeft);
            model_.setItem(i, 0, item);
            i++;
        }
    }
    model_.setHeaderData(0, Qt::Horizontal, "/" + name, Qt::DisplayRole);

    if (model_.rowCount())
    {
        treeview_.hide();
    }
    treeview_.update();
    treeview_.setModel(&model_);
    treeview_.show();

    QModelIndex index = model_.index(0, 0);
    if (index.isValid())
    {
        treeview_.select(index);
    }
}

void ExplorerView::showWebsites(CategoryType category, QString name)
{
    int i = 0;
    QFont itemFont;
    itemFont.setPointSize(22);

    curr_category_ = category;

    QSqlQuery query("SELECT name, icon, url FROM websites WHERE name <> '' ORDER BY upper(name), name");

    model_.clear();
    model_.setColumnCount(1);

    while (query.next())
    {
        QStandardItem *item = new QStandardItem();
        item->setText(query.value(0).toString());
        if (QFile::exists(query.value(1).toString()))
        {
            item->setIcon(QIcon(query.value(1).toString()));
        }
        else
        {
            item->setIcon(QIcon("/usr/share/explorer/images/middle/websites.png"));
        }
        item->setData(query.value(2).toStringList());
        item->setEditable(false);
        item->setFont(itemFont);
        item->setTextAlignment(Qt::AlignLeft);
        model_.setItem(i, 0, item);
        i++;
    }
    model_.setHeaderData(0, Qt::Horizontal, "/" + name, Qt::DisplayRole);

    if (model_.rowCount())
    {
        treeview_.hide();
    }
    treeview_.update();
    treeview_.setModel(&model_);
    treeview_.show();

    QModelIndex index = model_.index(0, 0);
    if (index.isValid())
    {
        treeview_.select(index);
    }
}

void ExplorerView::onItemActivated(const QModelIndex & index)
{
    switch (curr_category_)
    {
    case CAT_HOME:
    {
        home_row_ = index.row();
        QStandardItem *item = model_.itemFromIndex(index);
        if (item != 0)
        {
            CategoryType category = CategoryType(item->data().toInt());
            switch (category)
            {
            case CAT_INT_FLASH:
                current_path_ = "/media/flash";
                showFiles(category, current_path_);
                break;
            case CAT_SD_CARD:
                current_path_ = "/media/sd";
                showFiles(category, current_path_);
                break;
            case CAT_BOOKS:
                showBooks(category, item->text());
                break;
            case CAT_APPS:
            case CAT_GAMES:
                showApps(category, item->text());
                break;
            case CAT_WEBSITES:
                showWebsites(category, item->text());
                break;
            default:
                break;
            }
        }
        break;
    }
    case CAT_INT_FLASH:
    case CAT_SD_CARD:
    {
        QStandardItem *item = model_.itemFromIndex(index);
        if (item != 0)
        {
            QBitArray properties = item->data().toBitArray();
            if (properties[0] == true)
            {
                current_path_ += "/" + item->text();
                showFiles(curr_category_, current_path_);
            }
            else
            {
                QString fullFileName = current_path_ + "/" + item->text();

                if (!openDataFile(fullFileName))
                {
                    if (properties[1] == true)
                    {
                        qDebug() << "app:" << fullFileName;
                        run(fullFileName, QStringList());
                    }
                }
            }
        }
        break;
    }
    case CAT_BOOKS:
    {
        QStandardItem *item = model_.itemFromIndex(index);
        if (item != 0)
        {
            openDataFile(item->data().toString());
        }
        break;
    }
    case CAT_APPS:
    case CAT_GAMES:
    {
        QStandardItem *item = model_.itemFromIndex(index);
        if (item != 0)
        {
            QString app = item->data().toString();
            if (!app.isEmpty())
            {
                qDebug() << "app:" << app;
                run(app, QStringList());
            }
        }
        break;
    }
    case CAT_WEBSITES:
    {
        QStandardItem *item = model_.itemFromIndex(index);
        if (item != 0)
        {
            QStringList url = item->data().toStringList();
            if (!url.at(0).isEmpty())
            {
                qDebug() << "website:" << url.at(0);
                run("/opt/onyx/arm/bin/web_browser", url);
            }
        }
        break;
    }
    default:
        break;
    }
}

bool ExplorerView::openDataFile(QString fullFileName)
{
    QString app;
    QStringList doc(fullFileName);
    QString ext = QFileInfo(fullFileName).suffix();

    // Use database for extension/application associations
    QSqlQuery query(QString("SELECT application FROM associations WHERE extension = '%1'").arg(ext));
    if (query.first())
    {
        app = query.value(0).toString();
    }
    query.finish();

    if (QFile::exists(app))
    {
        qDebug() << "document:" << doc.at(0);

        run(app, doc);
        return true;
    }

    return false;
}

int ExplorerView::run(const QString &command, const QStringList & parameters)
{
    bool isScript = false;
    int exitCode;

    QFile script(command);
    script.open(QIODevice::ReadOnly);
    QString content(script.readLine());
    if (content.startsWith("#!/"))
    {
        content.remove(0, 2);
        if (QFile::exists(content.simplified()))
        {
            isScript = true;
        }
    }

    sys::SysStatus::instance().setSystemBusy(true);
    exitCode = sys::runScript(command, parameters);
    if (isScript == true || exitCode != 0)
    {
        sys::SysStatus::instance().setSystemBusy(false);
    }

    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);

    return exitCode;
}

void ExplorerView::onPositionChanged(int currentPage, int pages)
{
    status_bar_.setProgress(currentPage, pages);
    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void ExplorerView::onAboutToSuspend()
{
    qDebug("System is about to suspend");

    // Standby splash is provided by the system manager
    sys::SysStatus::instance().suspend();
}

void ExplorerView::onAboutToShutDown()
{
    qDebug("System is about to shutdown");

    // Shutdown splash has to be provided by the explorer (us)
    ExplorerSplash shutdownSplash(QPixmap("/usr/share/explorer/images/shutdown.png"), this);
    shutdownSplash.show();

    onyx::screen::instance().flush(&shutdownSplash, onyx::screen::ScreenProxy::GC);

    // Switch off Wifi
    if (SysStatus::instance().sdioState())
    {
        SysStatus::instance().enableSdio(false);
    }

    close();
    sys::SysStatus::instance().shutdown();
}

void ExplorerView::popupMenu()
{
    QModelIndex index = model_.index(treeview_.selected(), 0);
    QStandardItem *item;

    if ( onyx::screen::instance().defaultWaveform() == onyx::screen::ScreenProxy::DW )
    {
        // Stop fastest update mode to get better image quality.
        onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GC);
    }

    SystemActions systemActions;
    CBooxActions fileActions, organizeActions, settingsActions;
    PopupMenu menu(this);

    // system actions
    std::vector<int> sys_actions;
    sys_actions.push_back(ROTATE_SCREEN);
    if (mainUI_)
    {
        if (obx::isSDMounted())
        {
            sys_actions.push_back(REMOVE_SD);
        }
        sys_actions.push_back(STANDBY);
        sys_actions.push_back(SHUTDOWN);
    }
    else
    {
        sys_actions.push_back(RETURN_TO_LIBRARY);
    }
    systemActions.generateActions(sys_actions);
    menu.setSystemAction(&systemActions);

    if (index.isValid())
    {
        item = model_.itemFromIndex(index);
        if (item != 0)
        {
            if (curr_category_ == CAT_INT_FLASH || curr_category_ == CAT_SD_CARD)
            {
                // file actions
                fileActions.InitializeActions(QIcon(":/images/file.png"), tr("File"));
                fileActions.AddAction(QIcon(":/images/delete.png"), tr("Delete"), FILE_DELETE);
                menu.addGroup(&fileActions);

                // organize actions
                QBitArray properties = item->data().toBitArray();
                if (properties[0] == false && properties[1] == true)
                {
                    organizeActions.InitializeActions(QIcon(":/images/organize.png"), tr("Organize"));
                    organizeActions.AddAction(QIcon(":/images/runnable_file.png"), tr("Add to Applications"), ORG_ADD2APPS);
                    organizeActions.AddAction(QIcon(":/images/games.png"), tr("Add to Games"), ORG_ADD2GAMES);
                    menu.addGroup(&organizeActions);
                }
            }
            else if (curr_category_ == CAT_APPS || curr_category_ == CAT_GAMES)
            {
                // organize actions
                organizeActions.InitializeActions(QIcon(":/images/organize.png"), tr("Organize"));
                organizeActions.AddAction(QIcon(":/images/delete.png"), tr("Remove"), ORG_REMOVE);
                menu.addGroup(&organizeActions);
            }
        }
    }

    // settings actions
    settingsActions.InitializeActions(QIcon(":/images/settings.png"), tr("Settings"));
    settingsActions.AddAction(QIcon(":/images/wifi.png"), tr("Wifi"), SET_WIFI);
    settingsActions.AddAction(QIcon(":/images/about.png"), tr("About"), SET_ABOUT);
    menu.addGroup(&settingsActions);

    if (menu.popup() != QDialog::Accepted)
    {
        return;
    }

    QAction *group = menu.selectedCategory();
    if (group == systemActions.category())
    {
        switch (systemActions.selected())
        {
        case RETURN_TO_LIBRARY:
               qApp->exit();
            break;
        case ROTATE_SCREEN:
            SysStatus::instance().rotateScreen();
            break;
        case REMOVE_SD:
            if (SysStatus::instance().umountSD() == false)
            {
                qDebug("SD removal failed.");
            }
            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
            break;
        case STANDBY:
            onAboutToSuspend();
            break;
        case SHUTDOWN:
            onAboutToShutDown();
            break;
        default:
            break;
        }
    }
    else if (group == fileActions.category())
    {
        if (curr_category_ == CAT_INT_FLASH || curr_category_ == CAT_SD_CARD)
        {
            if (fileActions.selected() == FILE_DELETE)
            {
                // Confirmation dialog
                MessageDialog about(QMessageBox::Icon(QMessageBox::Warning) , tr("Delete"),
                                    tr("Do you want to delete %1?").arg(item->text()),
                                    QMessageBox::Yes | QMessageBox::No);

                if (about.exec() == QMessageBox::Yes)
                {
                    QString fullFileName = current_path_ + "/" + item->text();
                    if (QFile::remove(fullFileName) == true)
                    {
                        qDebug() << "deleted:" << fullFileName;
                    }

                    showFiles(curr_category_, model_.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString());
                }
                else
                {
                    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
                }
            }
        }
    }
    else if (group == organizeActions.category())
    {
        if (curr_category_ == CAT_INT_FLASH || curr_category_ == CAT_SD_CARD)
        {
            CategoryType category;

            switch (organizeActions.selected())
            {
            case ORG_ADD2APPS:
                category = CAT_APPS;
                break;
            case ORG_ADD2GAMES:
                category = CAT_GAMES;
                break;
            default:
                return;
            }

            // Add to database
            QString fullFileName = current_path_ + "/" + item->text();
            QFileInfo name(fullFileName);
            QString displayName = name.completeBaseName();
            displayName[0] = displayName[0].toUpper();
            QSqlQuery query(QString("INSERT INTO applications (name, executable, category_id) "
                                    "VALUES ('%1', '%2', %3)").arg(displayName).arg(fullFileName).arg(category));

            if (query.numRowsAffected() > 0)
            {
                qDebug() << "added to applications:" << fullFileName;
            }

            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
        }
        else if (curr_category_ == CAT_APPS || curr_category_ == CAT_GAMES)
        {
            if (organizeActions.selected() == ORG_REMOVE)
            {
                // Confirmation dialog
                MessageDialog about(QMessageBox::Icon(QMessageBox::Warning) , tr("Remove"),
                                    tr("Do you want to remove %1?").arg(item->text()),
                                    QMessageBox::Yes | QMessageBox::No);

                if (about.exec() == QMessageBox::Yes)
                {
                    QSqlQuery query(QString("DELETE FROM applications WHERE executable = '%1' "
                                            "AND category_id = '%2'").arg(item->data().toString())
                                            .arg(curr_category_));

                    if (query.numRowsAffected() > 0)
                    {
                        qDebug() << "removed:" << item->data().toString();
                    }

                    showApps(curr_category_, model_.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString().remove(0, 1));
                }
                else
                {
                    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
                }
            }
        }
    }
    else if (group == settingsActions.category())
    {
        switch (settingsActions.selected())
        {
        case SET_WIFI:
        {
            WifiDialog dialog(this, SysStatus::instance());
            dialog.popup();
            break;
        }
        case SET_ABOUT:
        {
            AboutDialog aboutDialog(this);
            aboutDialog.exec();

            onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
            break;
        }
        default:
            break;
        }
    }
}

void ExplorerView::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void ExplorerView::keyReleaseEvent(QKeyEvent *ke)
{
    switch(ke->key())
    {
    case Qt::Key_PageUp:
        treeview_.pageUp();
        break;
    case Qt::Key_PageDown:
        treeview_.pageDown();
        break;
    case Qt::Key_Left:
    case Qt::Key_Right:
        break;
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Return:
        treeview_.keyReleaseEvent(ke);
        break;
    case Qt::Key_Escape:
        switch (curr_category_)
        {
        case CAT_HOME:
            if (!mainUI_)
            {
                qApp->exit();
            }
            break;
        case CAT_INT_FLASH:
        case CAT_SD_CARD:
            if (current_path_ != "/media/flash" && current_path_ != "/media/sd")
            {
                current_path_.chop(current_path_.size() - current_path_.lastIndexOf('/'));
                showFiles(curr_category_, current_path_);
                break;
            }
            // fallthrough
        default:
            curr_category_ = CAT_HOME;
            showHome();
            break;
        }
        break;
    case ui::Device_Menu_Key:
        popupMenu();
        break;
    default:
        break;
    }
}

ExplorerSplash::ExplorerSplash(QPixmap pixmap, QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
{
    pixmap_ = pixmap;
    resize(qApp->desktop()->screenGeometry().size());
}

ExplorerSplash::~ExplorerSplash()
{
}

void ExplorerSplash::paintEvent(QPaintEvent *)
{
    QPainter image(this);

    // Adjust to currently used rotation
    switch (SysStatus::instance().screenTransformation())
    {
    case 90:
        image.translate(800, 0);
        break;
    case 180:
        image.translate(800, 600);
        break;
    case 270:
        image.translate(0, 600);
        break;
    case 0:
    default:
        break;
    }
    image.rotate(qreal(SysStatus::instance().screenTransformation()));
    image.drawPixmap(0, 0, pixmap_);
}

}
