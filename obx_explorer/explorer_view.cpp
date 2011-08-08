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
#include "about_dialog.h"
#include "obx_explorer.h"
#include "file_system_utils.h"
#include "database_utils.h"

#include "onyx/sys/sys.h"
#include "onyx/sys/sys_utils.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/menu.h"
#include "onyx/ui/onyx_keyboard_dialog.h"
#include "onyx/ui/time_zone_dialog.h"
#include "onyx/ui/power_management_dialog.h"
#include "onyx/wireless/wifi_dialog.h"

using namespace ui;

namespace obx
{

enum
{
    CAT_APPS = 4,   // TODO This enum has to become obsolete
    CAT_GAMES = 5
};

enum
{
    FILE_EDIT = 0,
    FILE_RENAME,
    FILE_DELETE,
    FILE_CUT,
    FILE_COPY,
    FILE_PASTE
};

enum
{
    ORG_CATEGORIES = 0,
    ORG_ADD2APPS,
    ORG_ADD2GAMES,
    ORG_ADDWEBSITE,
    ORG_ADDWEBSITEICON,
    ORG_REMOVE
};

enum
{
    SET_TZ = 0,
    SET_PWR_MGMT,
    SET_CALIBRATE,
    SET_WIFI,
    SET_DEFAULTS,
    SET_ABOUT
};

ExplorerView::ExplorerView(bool mainUI, QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , waveform_(onyx::screen::ScreenProxy::GC)
    , vbox_(this)
    , model_(0)
    , treeview_(0, 0)
    , status_bar_(this, MENU | PROGRESS | BATTERY)
    , handler_type_(HDLR_HOME)
    , category_id_(0)
    , selected_row_(0)
    , organize_mode_(0)
    , fileClipboard_()
{
    mainUI_ = mainUI;

    QSize size = qApp->desktop()->screenGeometry().size();

//    qDebug() << "Rotation: " << SysStatus::instance().screenTransformation();
//    qDebug() << "Dimensions: " << size.width() << "x" << size.height();

    resize(size);

    connect(&treeview_, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(onItemActivated(const QModelIndex &)));
    connect(&treeview_, SIGNAL(positionChanged(int, int)),
            this, SLOT(onPositionChanged(int, int)));

    connect(&status_bar_, SIGNAL(menuClicked()), this, SLOT(popupMenu()));

    if (mainUI)
    {
        SysStatus &sys_status = SysStatus::instance();
        connect(&sys_status, SIGNAL(aboutToSuspend()), this, SLOT(onAboutToSuspend()));
        connect(&sys_status, SIGNAL(aboutToShutdown()), this, SLOT(onAboutToShutDown()));
    }

    QSqlQuery query(QString("SELECT extension FROM associations WHERE handler_id = %1")
                            .arg(HDLR_BOOKS));
    while (query.next())
    {
        book_extensions_ << query.value(0).toString();
    }
    query.finish();

    icon_extensions_ << "png" << "jpg";

    treeview_.showHeader(true);
    treeview_.setHovering(true);

    vbox_.setSpacing(0);
    vbox_.setContentsMargins(0, 0, 0, 0);
    vbox_.addWidget(&treeview_);
    vbox_.addWidget(&status_bar_);

    showHome();

//    status_bar_.setProgress(treeview_.currentPage(), treeview_.pages());
    int itemsPerPage = (size.height() == 800 ? 7 : 5);
    status_bar_.setProgress(1, model_.rowCount() / itemsPerPage + (model_.rowCount() % itemsPerPage ? 1 : 0));
}

ExplorerView::~ExplorerView()
{
}

void ExplorerView::showHome()
{
    int i = 0;
    QFont itemFont;
    itemFont.setPointSize(22);

    category_id_ = 0;

    QSqlQuery query("SELECT name, icon, id, handler_id, handler_data FROM categories "
                    "WHERE name <> '' AND visible = 1 ORDER BY position");

    model_.clear();
    model_.setColumnCount(1);

    while (query.next())
    {
        QStandardItem *item = new QStandardItem();
        item->setText(query.value(0).toString());
        item->setIcon(QIcon(query.value(1).toString()));
        item->setData(QStringList() << query.value(2).toString() << query.value(3).toString() << query.value(4).toString());
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

    QModelIndex index = model_.index(selected_row_, 0);
    if (index.isValid())
    {
        treeview_.select(index);
    }
}

void ExplorerView::showFiles(int category, QString path)
{
    QFont itemFont;
    itemFont.setPointSize(20);

    category_id_ = category;

    qDebug() << "current directory:" << path;

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
        item->setIcon(QIcon(getIconByExtension(fileInfo)));
        QBitArray properties(2);
        properties[0] = fileInfo.isDir();
        properties[1] = FileSystemUtils::isRunnable(fileInfo);
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

void ExplorerView::showBooks(int category, QString name)
{
    int i = 0;
    QFont itemFont;
    itemFont.setPointSize(20);

    category_id_ = category;

    QSqlDatabase db = QSqlDatabase::database("ONYX");
    QSqlQuery query(db);
    query.exec("SELECT DISTINCT location, name, authors FROM content WHERE name <> '' ORDER BY upper(name), name");

    model_.clear();
    model_.setColumnCount(1);

    while (query.next())
    {
        QString fullFileName = query.value(0).toString() + "/" + query.value(1).toString();
        QFileInfo fileInfo(fullFileName);
        if (fileInfo.exists() && book_extensions_.contains(fileInfo.suffix()))
        {
            QStandardItem *item = new QStandardItem();
            item->setText(fileInfo.baseName());
            item->setIcon(QIcon(getIconByExtension(fileInfo)));
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

void ExplorerView::showApps(int category, QString name)
{
    int i = 0;
    QFont itemFont;
    itemFont.setPointSize(22);

    category_id_ = category;

    QSqlQuery query(QString("SELECT DISTINCT name, icon, executable, category_id FROM applications "
                            "WHERE category_id = %1 ORDER BY upper(name), name").arg(category));

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

void ExplorerView::showWebsites(int category, QString name)
{
    int i = 0;
    QFont itemFont;
    itemFont.setPointSize(22);

    category_id_ = category;

    QSqlQuery query("SELECT DISTINCT name, icon, url FROM websites WHERE name <> '' ORDER BY upper(name), name");

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

void ExplorerView::organizeCategories(int row)
{
    int i = 0;
    QFont itemFont;
    itemFont.setPointSize(22);

    organize_mode_ = true;

    QSqlQuery query("SELECT name, icon, id, position, visible FROM categories "
                    "WHERE name <> '' ORDER BY position");

    model_.clear();
    model_.setColumnCount(1);

    while (query.next())
    {
        QStandardItem *item = new QStandardItem();
        item->setText(query.value(0).toString());
        item->setIcon(QIcon(query.value(1).toString()));
        item->setData(QStringList() << query.value(2).toString() << query.value(3).toString());
        item->setEditable(false);
        item->setSelectable(query.value(4).toBool());
        item->setFont(itemFont);
        item->setTextAlignment(Qt::AlignLeft);
        model_.setItem(i++, 0, item);
    }
    model_.setHeaderData(0, Qt::Horizontal, "Organize Categories", Qt::DisplayRole);

    if (model_.rowCount())
    {
        treeview_.hide();
    }
    treeview_.update();
    treeview_.setModel(&model_);
    repaint();
    treeview_.show();

    QModelIndex index = model_.index(row, 0);
    if (index.isValid())
    {
        treeview_.select(index);
    }
}

void ExplorerView::onItemActivated(const QModelIndex & index)
{
    QStandardItem *item = model_.itemFromIndex(index);
    if (!organize_mode_ && item != 0)
    {
        switch (handler_type_)
        {
        case HDLR_HOME:
        {
            selected_row_ = treeview_.selected();
            int categoryId = item->data().toStringList()[0].toInt();
            handler_type_ = HandlerType(item->data().toStringList()[1].toInt());
            QString handlerData = item->data().toStringList()[2];

            switch (handler_type_)
            {
            case HDLR_FILES:
                root_path_ = handlerData;
                current_path_ = handlerData;
                showFiles(categoryId, current_path_);
                break;
            case HDLR_BOOKS:
                showBooks(categoryId, item->text());
                break;
            case HDLR_APPS:
                showApps(categoryId, item->text());
                break;
            case HDLR_WEBSITES:
                showWebsites(categoryId, item->text());
                break;
            default:
                break;
            }
            break;
        }
        case HDLR_FILES:
        {
            QBitArray properties = item->data().toBitArray();
            if (properties[0] == true)
            {
                current_path_ += "/" + item->text();
                showFiles(category_id_, current_path_);
            }
            else
            {
                QString fullFileName = current_path_ + "/" + item->text();
                QString viewer = getByExtension("viewer", QFileInfo(fullFileName).suffix());

                if (!viewer.isNull())
                {
                    qDebug() << "view:" << fullFileName;
                    run(viewer, QStringList() << fullFileName);
                }
                else if (properties[1] == true)
                {
                    qDebug() << "app:" << fullFileName;
                    run(fullFileName, QStringList());
                }
            }
            break;
        }
        case HDLR_BOOKS:
        {
            QStringList doc(item->data().toString());
            QString viewer = getByExtension("viewer", QFileInfo(doc.at(0)).suffix());
            if (!viewer.isNull())
            {
                qDebug() << "view:" << doc.at(0);
                run(viewer, doc);
            }
            break;
        }
        case HDLR_APPS:
        {
            QString app = item->data().toString();
            if (!app.isEmpty())
            {
                qDebug() << "app:" << app;
                run(app, QStringList());
            }
            break;
        }
        case HDLR_WEBSITES:
        {
            QStringList url = item->data().toStringList();
            if (!url.at(0).isEmpty())
            {
                qDebug() << "website:" << url.at(0);
                run("/opt/onyx/arm/bin/web_browser", url);
            }
            break;
        }
        default:
            break;
        }
    }
}

QString ExplorerView::getByExtension(const QString &field, const QString &extension)
{
    QString result;
    QSqlQuery query(QString("SELECT %1 FROM associations WHERE extension = '%2'")
                            .arg(field)
                            .arg(extension));
    if (query.first())
    {
        result = query.value(0).toString();
    }
    query.finish();

    if (QFile::exists(result))
    {
        return result;
    }

    return QString();
}

QString ExplorerView::getIconByExtension(const QFileInfo &fileInfo)
{
    QString extIcon;

    if (fileInfo.isDir())
    {
        extIcon = "/usr/share/explorer/images/middle/directory.png";
    }
    else
    {
        // Check file system for extension icon
        extIcon = getMatchingIcon(fileInfo);
        if (extIcon.isNull())
        {
            // Check database for extension icon
            extIcon = getByExtension("icon", fileInfo.suffix());
            if (extIcon.isNull())
            {
                // Use one of the default icons
                if (FileSystemUtils::isRunnable(fileInfo))
                {
                    extIcon = "/usr/share/explorer/images/middle/runnable_file.png";
                }
                else
                {
                    extIcon = "/usr/share/explorer/images/middle/unknown_document.png";
                }
            }
        }
    }

    return extIcon;
}

QString ExplorerView::getDisplayName(const QFileInfo &fileInfo)
{
    QString displayName;
    bool ok;

    fileInfo.suffix().toInt(&ok);

    if (ok)
    {
        displayName = fileInfo.fileName();
    }
    else
    {
        displayName = fileInfo.completeBaseName();
    }

    displayName[0] = displayName[0].toUpper();

    return displayName;
}

QString ExplorerView::getMatchingIcon(const QFileInfo &fileInfo)
{
    bool ok;

    QString extension = fileInfo.suffix();
    if (icon_extensions_.contains(extension))
    {
        return fileInfo.absoluteFilePath();
    }

    extension.toInt(&ok);

    for (int i = 0; i < icon_extensions_.size(); i++)
    {
        QString iconFileName = fileInfo.absoluteFilePath();

        if (ok)
        {
            iconFileName.append(QString(".%1").arg(icon_extensions_[i]));
        }
        else
        {
            iconFileName.replace(fileInfo.suffix(), icon_extensions_[i]);
        }

        if (QFile::exists(iconFileName))
        {
            return iconFileName;
        }
    }

    return QString();
}

void ExplorerView::addApplication(int category, QString fullFileName)
{
    // Add to database
    QFileInfo fileInfo(fullFileName);
    QString displayName = getDisplayName(fileInfo);
    QString iconFileName = getMatchingIcon(fileInfo);

    QSqlQuery query(QString("INSERT INTO applications (name, executable, icon, category_id) "
                            "VALUES ('%1', '%2', '%3', %4)")
                            .arg(displayName)
                            .arg(fullFileName)
                            .arg(iconFileName)
                            .arg(category));

    if (query.numRowsAffected() > 0)
    {
        qDebug() << "added to applications/games:" << fullFileName;
    }
}

int ExplorerView::run(const QString &command, const QStringList & parameters)
{
    int exitCode;

    sys::SysStatus::instance().setSystemBusy(true);
    exitCode = sys::runScript(command, parameters);

    if (FileSystemUtils::isScript(command) || exitCode != 0)
    {
        sys::SysStatus::instance().setSystemBusy(false);
    }

    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);

    return exitCode;
}

void ExplorerView::onPositionChanged(int currentPage, int pages)
{
    status_bar_.setProgress(currentPage, pages);
    onyx::screen::instance().flush(this, waveform_);
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
    QString organizeString = "Organize ";

    if (category_id_)
    {
        QSqlQuery query(QString("SELECT name FROM categories WHERE id = %1").arg(category_id_));
        if (query.first())
        {
            organizeString += query.value(0).toString();
        }
        query.finish();
    }

    QModelIndex index = model_.index(treeview_.selected(), 0);
    QStandardItem *item = 0;

    if (index.isValid())
    {
        item = model_.itemFromIndex(index);
    }

    if (onyx::screen::instance().defaultWaveform() == onyx::screen::ScreenProxy::DW)
    {
        // Stop fastest update mode to get better image quality.
        onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GC);
    }

    PopupMenu menu(this);
    QString editor;
    QStringList doc;

    // system actions
    std::vector<int> sys_actions;
    sys_actions.push_back(ROTATE_SCREEN);
    if (mainUI_)
    {
        if (FileSystemUtils::isSDMounted())
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
    systemActions_.generateActions(sys_actions);
    menu.setSystemAction(&systemActions_);

    switch (handler_type_)
    {
    case HDLR_HOME:
        if (item != 0)
        {
            organizeActions_.initializeActions(QIcon(":/images/organize.png"), organizeString);
            organizeActions_.addAction(QIcon(":/images/category_organization.png"), tr("Categories"), ORG_CATEGORIES);
            menu.addGroup(&organizeActions_);
        }
        break;
    case HDLR_FILES:
        if (item != 0)
        {
            doc << (current_path_ + "/" + item->text());
            QString extension = QFileInfo(doc[0]).suffix();
            editor = getByExtension("editor", extension);

            // organize actions
            QBitArray properties = item->data().toBitArray();
            if (properties[0] == false && properties[1] == true)
            {
                organizeActions_.initializeActions(QIcon(":/images/organize.png"), organizeString);
                organizeActions_.addAction(QIcon(":/images/applications.png"), tr("Add to Applications"), ORG_ADD2APPS);
                organizeActions_.addAction(QIcon(":/images/games.png"), tr("Add to Games"), ORG_ADD2GAMES);
                menu.addGroup(&organizeActions_);
            }
            else if (icon_extensions_.contains(extension))
            {
                organizeActions_.initializeActions(QIcon(":/images/organize.png"), organizeString);
                organizeActions_.addAction(QIcon(":/images/website_icon.png"), tr("Set as website icon"), ORG_ADDWEBSITEICON);
                menu.addGroup(&organizeActions_);
            }

            // file actions
            fileActions_.initializeActions(QIcon(":/images/edit.png"), tr("File"));
            if (!editor.isNull())
            {
                fileActions_.addAction(QIcon(":/images/file_edit.png"), tr("Edit"), FILE_EDIT);
            }
            fileActions_.addAction(QIcon(":/images/rename.png"), tr("Rename"), FILE_RENAME);
            fileActions_.addAction(QIcon(":/images/delete.png"), tr("Delete"), FILE_DELETE);
            fileActions_.addSeparator();
            fileActions_.addAction(QIcon(":/images/cut.png"), tr("Cut"), FILE_CUT);
            fileActions_.addAction(QIcon(":/images/copy.png"), tr("Copy"), FILE_COPY);
            if (!fileClipboard_.isEmpty())
            {
                fileActions_.addAction(QIcon(":/images/paste.png"), tr("Paste"), FILE_PASTE);
            }
            menu.addGroup(&fileActions_);
        }
        break;
    case HDLR_BOOKS:
        if (item != 0)
        {
            doc << item->data().toString();
            editor = getByExtension("editor", QFileInfo(doc[0]).suffix());

            if (!editor.isNull())
            {
                // file actions
                fileActions_.initializeActions(QIcon(":/images/edit.png"), tr("File"));
                fileActions_.addAction(QIcon(":/images/file_edit.png"), tr("Edit"), FILE_EDIT);
                menu.addGroup(&fileActions_);
            }
        }
        break;
    case HDLR_APPS:
        if (item != 0)
        {
            // organize actions
            organizeActions_.initializeActions(QIcon(":/images/organize.png"), organizeString);
            organizeActions_.addAction(QIcon(":/images/delete.png"), tr("Remove"), ORG_REMOVE);
            menu.addGroup(&organizeActions_);
        }
        break;
    case HDLR_WEBSITES:
        // organize actions
        organizeActions_.initializeActions(QIcon(":/images/organize.png"), organizeString);
        organizeActions_.addAction(QIcon(":/images/add.png"), tr("Add"), ORG_ADDWEBSITE);
        if (item != 0)
        {
            organizeActions_.addAction(QIcon(":/images/delete.png"), tr("Remove"), ORG_REMOVE);
        }
        menu.addGroup(&organizeActions_);
        break;
    default:
        break;
    }

    // settings actions
    settingsActions_.initializeActions(QIcon(":/images/settings.png"), tr("Settings"));
    if (mainUI_)
    {
        settingsActions_.addAction(QIcon(":/images/time_zone.png"), tr("Time Zone"), SET_TZ);
        settingsActions_.addAction(QIcon(":/images/power_management.png"), tr("Power Management"), SET_PWR_MGMT);
        settingsActions_.addAction(QIcon(":/images/screen_calibration.png"), tr("Screen Calibration"), SET_CALIBRATE);
        settingsActions_.addAction(QIcon(":/images/wifi.png"), tr("Wireless LAN"), SET_WIFI);
    }
    settingsActions_.addAction(QIcon(":/images/restore.png"), tr("Default Categories"), SET_DEFAULTS);
    settingsActions_.addAction(QIcon(":/images/about.png"), tr("About"), SET_ABOUT);
    menu.addGroup(&settingsActions_);

    if (menu.popup() != QDialog::Accepted)
    {
        QApplication::processEvents();
        return;
    }

    onyx::screen::instance().enableUpdate(false);
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);

    QAction *group = menu.selectedCategory();
    if (group == systemActions_.category())
    {
        switch (systemActions_.selected())
        {
        case RETURN_TO_LIBRARY:
            qApp->exit();
            return;
        case ROTATE_SCREEN:
            SysStatus::instance().rotateScreen();
            return;
        case REMOVE_SD:
            if (SysStatus::instance().umountSD() == false)
            {
                qDebug("SD removal failed.");
            }
            break;
        case STANDBY:
            onAboutToSuspend();
            return;
        case SHUTDOWN:
            onAboutToShutDown();
            return;
        default:
            return;
        }
    }
    else if (group == fileActions_.category())
    {
        if (handler_type_ == HDLR_FILES || handler_type_ == HDLR_BOOKS)
        {
            switch (fileActions_.selected())
            {
            case FILE_EDIT:
                qDebug() << "edit:" << doc[0];
                run(editor, doc);
                return;
            case FILE_RENAME:
            {
                treeview_.setHovering(false);
                onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU);

                OnyxKeyboardDialog dialog(this, tr("Enter new file name"));
                QString newFileName = dialog.popup(item->text());
                treeview_.setHovering(true);

                if (!newFileName.isEmpty())
                {
                    newFileName.prepend(current_path_ + "/");
                    if (QFile::rename(current_path_ + "/" + item->text(), newFileName))
                    {
                        qDebug() << "renamed:" << current_path_ + "/" + item->text() << "to" << newFileName;
                        showFiles(category_id_, model_.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString());
                        return;
                    }
                    else
                    {
                        QString message;
                        if (QFile::exists(newFileName))
                        {
                            if (QFileInfo(newFileName).isDir())
                            {
                                message = tr("Directory name already exists");
                            }
                            else
                            {
                                message = tr("File name already exists");
                            }
                        }
                        else
                        {
                            message = tr("Rename failed");
                        }

                        MessageDialog error(QMessageBox::Icon(QMessageBox::Warning) , tr("Rename failed"),
                                            message, QMessageBox::Ok);
                        error.exec();
                    }
                }
                break;
            }
            case FILE_DELETE:
            {
                // Confirmation dialog
                MessageDialog del(QMessageBox::Icon(QMessageBox::Warning) , tr("Delete"),
                                  tr("Do you want to delete %1?").arg(item->text()),
                                  QMessageBox::Yes | QMessageBox::No);

                if (del.exec() == QMessageBox::Yes)
                {
                    bool removed = false;
                    QString fullName = current_path_ + "/" + item->text();
                    QFileInfo fileInfo(fullName);
                    if (fileInfo.isDir())
                    {
                        removed = FileSystemUtils::removeDir(fullName);
                    }
                    else if (fileInfo.isFile())
                    {
                        removed = QFile::remove(fullName);
                    }

                    if (removed == true)
                    {
                        qDebug() << "deleted:" << fullName;
                        showFiles(category_id_, model_.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString());
                        return;
                    }
                    else
                    {
                        qDebug() << "deleting failed!";
                    }
                }
                break;
            }
            case FILE_CUT:
                fileClipboard_.cut(current_path_ + "/" + item->text());
                break;
            case FILE_COPY:
                fileClipboard_.copy(current_path_ + "/" + item->text());
                break;
            case FILE_PASTE:
                if (fileClipboard_.paste(current_path_))
                {
                    qDebug() << "pasted:" << (current_path_ + "/" + fileClipboard_.fileName());
                    showFiles(category_id_, model_.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString());
                    return;
                }
                else
                {
                    QString message;
                    if (QFile::exists(current_path_ + "/" + fileClipboard_.fileName()))
                    {
                        if (fileClipboard_.holdsDir())
                        {
                            message = tr("Directory already exists");
                        }
                        else
                        {
                            message = tr("File already exists");
                        }
                    }
                    else
                    {
                        message = tr("Paste failed");
                    }

                    MessageDialog error(QMessageBox::Icon(QMessageBox::Warning) , tr("Paste failed"),
                                        message, QMessageBox::Ok);
                    error.exec();
                }
                break;
            default:
                return;
            }
        }
    }
    else if (group == organizeActions_.category())
    {
        switch(handler_type_)
        {
        case HDLR_HOME:
            if (organizeActions_.selected() == ORG_CATEGORIES)
            {
                organizeCategories(0);
            }
            return;
        case HDLR_FILES:
        {
            QString fullFileName = current_path_ + "/" + item->text();

            switch (organizeActions_.selected())
            {
            case ORG_ADD2APPS:
                addApplication(CAT_APPS, fullFileName);     // TODO derive category id from database
                break;
            case ORG_ADD2GAMES:
                addApplication(CAT_GAMES, fullFileName);
                break;
            case ORG_ADDWEBSITEICON:
            {
                QSqlQuery query(QString("UPDATE websites SET icon = '%1' WHERE url LIKE '%%2%'")
                                       .arg(fullFileName)
                                       .arg(QFileInfo(fullFileName).baseName()));

                if (query.numRowsAffected() > 0)
                {
                    qDebug() << "Set as website icon:" << fullFileName;
                }
                break;
            }
            default:
                return;
            }
            break;
        }
        case HDLR_APPS:
            if (organizeActions_.selected() == ORG_REMOVE)
            {
                // Confirmation dialog
                MessageDialog remove(QMessageBox::Icon(QMessageBox::Warning) , tr("Remove"),
                                     tr("Do you want to remove %1?").arg(item->text()),
                                     QMessageBox::Yes | QMessageBox::No);

                if (remove.exec() == QMessageBox::Yes)
                {
                    QSqlQuery query(QString("DELETE FROM applications WHERE executable = '%1' "
                                            "AND category_id = '%2'").arg(item->data().toString())
                                            .arg(category_id_));

                    if (query.numRowsAffected() > 0)
                    {
                        qDebug() << "removed:" << item->data().toString();
                        showApps(category_id_, model_.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString().remove(0, 1));
                        return;
                    }
                }
            }
            break;
        case HDLR_WEBSITES:
            if (organizeActions_.selected() == ORG_ADDWEBSITE)
            {
                treeview_.setHovering(false);
                onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GU);

                OnyxKeyboardDialog dialog(this, tr("Enter web address"));
                QString url = dialog.popup("");
                treeview_.setHovering(true);

                if (!url.isEmpty())
                {
                    QString name = url;
                    if (name.startsWith("http://"))
                        name.remove("http://");
                    if (name.startsWith("www."))
                        name.remove("www.");
                    name.truncate(name.lastIndexOf('.'));
                    name[0] = name[0].toUpper();

                    if (!url.startsWith("http://"))
                        url.prepend("http://");

                    QString icon = "/usr/share/explorer/images/middle/websites.png";

                    QSqlQuery query(QString("INSERT INTO websites (name, url, icon) "
                                            "VALUES ('%1', '%2', '%3')").arg(name).arg(url).arg(icon));

                    if (query.numRowsAffected() > 0)
                    {
                        qDebug() << "added to websites:" << url;
                        showWebsites(category_id_, model_.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString().remove(0, 1));
                        return;
                    }
                }
            }
            else if (organizeActions_.selected() == ORG_REMOVE)
            {
                // Confirmation dialog
                MessageDialog remove(QMessageBox::Icon(QMessageBox::Warning) , tr("Remove"),
                                     tr("Do you want to remove %1?").arg(item->text()),
                                     QMessageBox::Yes | QMessageBox::No);

                if (remove.exec() == QMessageBox::Yes)
                {
                    QSqlQuery query(QString("DELETE FROM websites WHERE url = '%1'")
                                            .arg(item->data().toString()));

                    if (query.numRowsAffected() > 0)
                    {
                        qDebug() << "removed:" << item->data().toString();
                        showWebsites(category_id_, model_.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString().remove(0, 1));
                        return;
                    }
                }
            }
            break;
        default:
            return;
        }
    }
    else if (group == settingsActions_.category())
    {
        switch (settingsActions_.selected())
        {
        case SET_TZ:
        {
            TimeZoneDialog dialog(this);
            if (dialog.popup("Select Time Zone") == QDialog::Accepted)
            {
                if (SystemConfig::setTimezone(dialog.selectedTimeZone()))
                {
                    qDebug() << "set time zone:" << dialog.selectedTimeZone();
                }
            }
            break;
        }
        case SET_PWR_MGMT:
        {
            PowerManagementDialog dialog(this, SysStatus::instance());
            dialog.exec();
            break;
        }
        case SET_CALIBRATE:
            run("/opt/onyx/arm/bin/mouse_calibration", QStringList());
            return;
        case SET_WIFI:
        {
            WifiDialog dialog(this, SysStatus::instance());
            dialog.popup();
            return;
        }
        case SET_DEFAULTS:
        {
            // Confirmation dialog
            MessageDialog defaults(QMessageBox::Icon(QMessageBox::Warning) , tr("Default Categories"),
                                   tr("Do you want to restore the default categories and their content?"),
                                   QMessageBox::Yes | QMessageBox::No);

            if (defaults.exec() == QMessageBox::Yes)
            {
                DatabaseUtils::clearDatabase();
                obx::initializeDatabase();
                qDebug() << "default database restored";
                showHome();
                return;
            }
            break;
        }
        case SET_ABOUT:
        {
            AboutDialog aboutDialog(mainUI_, this);
            aboutDialog.exec();
            break;
        }
        default:
            return;
        }
    }

    onyx::screen::instance().flush(this, onyx::screen::ScreenProxy::GC);
}

void ExplorerView::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void ExplorerView::keyReleaseEvent(QKeyEvent *ke)
{
    if (organize_mode_)
    {
        QModelIndex index = model_.index(treeview_.selected(), 0);

        if (index.isValid())
        {
            if (QStandardItem *item = model_.itemFromIndex(index))
            {
                switch(ke->key())
                {
                case Qt::Key_Left:
                case Qt::Key_Right:
                {
                    bool visible = (ke->key() == Qt::Key_Left ? 1 : 0);

                    if (visible != item->isSelectable())
                    {
                        QSqlQuery query(QString("UPDATE categories SET visible = %1 WHERE id = %2")
                                               .arg(visible)
                                               .arg(item->data().toStringList()[0].toInt()));

                        if (query.numRowsAffected() > 0)
                        {
                            waveform_ = onyx::screen::ScreenProxy::GU;
                            organizeCategories(treeview_.selected());
                        }
                    }
                    break;
                }
                case Qt::Key_PageUp:
                case Qt::Key_PageDown:
                case Qt::Key_Up:
                case Qt::Key_Down:
                    treeview_.keyReleaseEvent(ke);
                    break;
                case Qt::Key_Return:
                    if (treeview_.selected())
                    {
                        int id;
                        int position = item->data().toStringList()[1].toInt();

                        QSqlQuery query;
                        query.exec(QString("SELECT id FROM categories WHERE position = %1").arg(position - 1));
                        if (query.first())
                        {
                            id = query.value(0).toInt();
                            query.exec(QString("UPDATE categories SET position = %1 WHERE id = %2")
                                               .arg(position - 1)
                                               .arg(item->data().toStringList()[0].toInt()));
                            query.exec(QString("UPDATE categories SET position = %1 WHERE id = %2")
                                               .arg(position)
                                               .arg(id));
                        }

                        if (query.numRowsAffected() > 0)
                        {
                            waveform_ = onyx::screen::ScreenProxy::GU;
                            organizeCategories(treeview_.selected() - 1);
                        }
                    }
                    break;
                case Qt::Key_Escape:
                    organize_mode_ = false;
                    waveform_ = onyx::screen::ScreenProxy::GC;
                    showHome();
                    break;
                case Qt::Key_Menu:
                default:
                    break;
                }
            }
        }
    }
    else
    {
        switch(ke->key())
        {
        case Qt::Key_Right:
        {
            QKeyEvent keyEvent(QEvent::KeyRelease,Qt::Key_Return, Qt::NoModifier);
            QApplication::sendEvent(&treeview_, &keyEvent);
            break;
        }
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Return:
            treeview_.keyReleaseEvent(ke);
            break;
        case Qt::Key_Left:
            if (handler_type_ == HDLR_FILES && current_path_ != root_path_)
            {
                current_path_.chop(current_path_.size() - current_path_.lastIndexOf('/'));
                showFiles(category_id_, current_path_);
                break;
            }
            // fall through
        case Qt::Key_Escape:
            if (handler_type_ == HDLR_HOME)
            {
                if (ke->key() == Qt::Key_Escape && !mainUI_)
                {
                    qApp->exit();
                }
                break;
            }

            handler_type_ = HDLR_HOME;
            showHome();
            break;
        case Qt::Key_Menu:
            popupMenu();
            break;
        default:
            break;
        }
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
