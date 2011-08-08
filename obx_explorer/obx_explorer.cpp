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

#include "obx_explorer.h"
#include "explorer_view.h"
#include "database_utils.h"

#include "onyx/sys/sys.h"
#include "onyx/screen/screen_proxy.h"

namespace obx
{

ObxExplorer::ObxExplorer(int &argc, char **argv)
    : QApplication(argc, argv)
    , view_(argc > 1 && QString::fromLocal8Bit(argv[1]) == "--mainui")
{
    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged()), Qt::QueuedConnection);

    view_.show();
    sys::SysStatus::instance().setSystemBusy(false);
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().flush(&view_, onyx::screen::ScreenProxy::GC);
}

ObxExplorer::~ObxExplorer()
{
}

void ObxExplorer::onScreenSizeChanged()
{
    QSize size = qApp->desktop()->screenGeometry().size();

//    qDebug() << "Rotation:" << SysStatus::instance().screenTransformation();
//    qDebug() << "Dimensions:" << size.width() << "x" << size.height();

    view_.resize(size);
    if (view_.isActiveWindow())
    {
        onyx::screen::instance().flush(&view_, onyx::screen::ScreenProxy::GC);
    }
}

bool initializeDatabase()
{
    const QStringList queries = QStringList()
        << "CREATE TABLE revision (id INTEGER PRIMARY KEY, major NUMERIC, minor NUMERIC)"
        << "INSERT INTO revision VALUES(1,0,0)"
        << "CREATE TABLE categories (id INTEGER PRIMARY KEY, position NUMERIC, visible NUMERIC, name TEXT, handler_id NUMERIC, handler_data TEXT, icon TEXT)"
        << "INSERT INTO categories VALUES(1,1,1,'Internal Flash',1,'/media/flash','/usr/share/explorer/images/middle/directory.png')"
        << "INSERT INTO categories VALUES(2,2,1,'SD Card',1,'/media/sd','/usr/share/explorer/images/middle/sd.png')"
        << "INSERT INTO categories VALUES(3,3,1,'Books',2,NULL,'/usr/share/explorer/images/middle/library.png')"
        << "INSERT INTO categories VALUES(4,4,1,'Applications',3,NULL,'/usr/share/explorer/images/middle/applications.png')"
        << "INSERT INTO categories VALUES(5,5,1,'Games',3,NULL,'/usr/share/explorer/images/middle/games.png')"
        << "INSERT INTO categories VALUES(6,6,1,'Websites',4,NULL,'/usr/share/explorer/images/middle/websites.png')"
        << "INSERT INTO categories VALUES(7,7,1,'Scribbles',1,'/root/notes','/usr/share/explorer/images/middle/notes.png')"
        << "CREATE TABLE associations (id INTEGER PRIMARY KEY, extension TEXT, viewer TEXT, editor TEXT, icon TEXT, handler_id NUMERIC)"
        << "INSERT INTO associations VALUES(1,'chm','/opt/onyx/arm/bin/html_reader',NULL,'/usr/share/explorer/images/middle/chm.png',2)"
        << "INSERT INTO associations VALUES(2,'djvu','/opt/onyx/arm/bin/djvu_reader',NULL,'/usr/share/explorer/images/middle/djvu.png',2)"
        << "INSERT INTO associations VALUES(3,'epub','/opt/onyx/arm/bin/naboo_reader',NULL,'/usr/share/explorer/images/middle/epub.png',2)"
        << "INSERT INTO associations VALUES(4,'mobi','/opt/onyx/arm/bin/onyx_reader',NULL,'/usr/share/explorer/images/middle/mobi.png',2)"
        << "INSERT INTO associations VALUES(5,'pdf','/opt/onyx/arm/bin/naboo_reader',NULL,'/usr/share/explorer/images/middle/pdf.png',2)"
        << "INSERT INTO associations VALUES(6,'fb2','/opt/onyx/arm/bin/onyx_reader',NULL,'/usr/share/explorer/images/middle/fb2.png',2)"
        << "INSERT INTO associations VALUES(7,'html','/opt/onyx/arm/bin/web_browser',NULL,'/usr/share/explorer/images/middle/html.png',2)"
        << "INSERT INTO associations VALUES(8,'txt','/opt/onyx/arm/bin/onyx_reader','/opt/onyx/arm/bin/text_editor','/usr/share/explorer/images/middle/txt.png',2)"
        << "INSERT INTO associations VALUES(9,'sketch','/opt/onyx/arm/bin/scribbler',NULL,'/usr/share/explorer/images/middle/note.png',NULL)"
        << "INSERT INTO associations VALUES(10,'png','/opt/onyx/arm/bin/image_reader',NULL,'/usr/share/explorer/images/middle/png.png',NULL)"
        << "INSERT INTO associations VALUES(11,'jpg','/opt/onyx/arm/bin/image_reader',NULL,'/usr/share/explorer/images/middle/jpg.png',NULL)"
        << "CREATE TABLE applications (id INTEGER PRIMARY KEY, name TEXT, executable TEXT, options TEXT, icon TEXT, category_id NUMERIC)"
        << "INSERT INTO applications VALUES(1,'Sudoku','/opt/onyx/arm/bin/sudoku',NULL,'/usr/share/explorer/images/middle/suduko.png',5)"
        << "INSERT INTO applications VALUES(2,'Dictionary','/opt/onyx/arm/bin/dict_tool',NULL,'/usr/share/explorer/images/middle/dictionary.png',4)"
        << "INSERT INTO applications VALUES(3,'Text Editor','/opt/onyx/arm/bin/text_editor',NULL,'/usr/share/explorer/images/middle/write_pad.png',4)"
        << "INSERT INTO applications VALUES(4,'Scribbler','/opt/onyx/arm/bin/scribbler',NULL,'/usr/share/explorer/images/middle/note.png',4)"
        << "CREATE TABLE websites (id INTEGER PRIMARY KEY, name TEXT, url TEXT, icon TEXT)"
        << "INSERT INTO websites VALUES(1,'Google','http://www.google.com','/usr/share/explorer/images/middle/google.png')"
        << "INSERT INTO websites VALUES(2,'Wikipedia','http://en.wikipedia.org','/usr/share/explorer/images/middle/wiki.png')";

    return (DatabaseUtils::execQueries(queries) == queries.size());
}

}
