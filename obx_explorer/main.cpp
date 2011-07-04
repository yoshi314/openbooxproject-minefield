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

#include <QtSql/QSqlDatabase>

#include "obx_explorer.h"

using namespace obx;

static bool createConnections();
static bool initializeDatabase();

int main(int argc, char *argv[])
{
    createConnections();

    ObxExplorer app(argc, argv);

    return app.exec();
}

static bool createConnections()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/root/obx_explorer.db");
    if (!db.open()) {
        qDebug() << "Database Error";
        return false;
    }

    if (db.tables().size() == 0)
    {
        qDebug() << "Initializing database...";
        if (!initializeDatabase())
        {
            qDebug() << "...Initialization failed!";
        }
        else
        {
            qDebug() << "...Done";
        }
    }

    db = QSqlDatabase::addDatabase("QSQLITE", "ONYX");
    db.setDatabaseName("/root/content.db");
    if (!db.open()) {
        qDebug() << "Database Error";
        return false;
    }

    return true;
}

static bool initializeDatabase()
{
    QSqlQuery query;
    bool active = true;
    int i = 0;

    QStringList queries;
    queries << "CREATE TABLE categories (id INTEGER PRIMARY KEY, name TEXT, icon TEXT)"
            << "INSERT INTO categories VALUES(1,'Internal Flash','/usr/share/explorer/images/middle/directory.png')"
            << "INSERT INTO categories VALUES(2,'SD Card','/usr/share/explorer/images/middle/sd.png')"
            << "INSERT INTO categories VALUES(3,'Books','/usr/share/explorer/images/middle/library.png')"
            << "INSERT INTO categories VALUES(4,'Applications','/usr/share/explorer/images/middle/applications.png')"
            << "INSERT INTO categories VALUES(5,'Games','/usr/share/explorer/images/middle/games.png')"
            << "INSERT INTO categories VALUES(6,'Websites','/usr/share/explorer/images/middle/websites.png')"
            << "CREATE TABLE associations (id INTEGER PRIMARY KEY, extension TEXT, application TEXT, icon TEXT)"
            << "INSERT INTO associations VALUES(1,'chm','/opt/onyx/arm/bin/html_reader','/usr/share/explorer/images/middle/chm.png')"
            << "INSERT INTO associations VALUES(2,'djvu','/opt/onyx/arm/bin/djvu_reader','/usr/share/explorer/images/middle/djvu.png')"
            << "INSERT INTO associations VALUES(3,'epub','/opt/onyx/arm/bin/naboo_reader','/usr/share/explorer/images/middle/epub.png')"
            << "INSERT INTO associations VALUES(4,'mobi','/opt/onyx/arm/bin/onyx_reader','/usr/share/explorer/images/middle/mobi.png')"
            << "INSERT INTO associations VALUES(5,'pdf','/opt/onyx/arm/bin/naboo_reader','/usr/share/explorer/images/middle/pdf.png')"
            << "INSERT INTO associations VALUES(6,'fb2','/opt/onyx/arm/bin/onyx_reader','/usr/share/explorer/images/middle/fb2.png')"
            << "INSERT INTO associations VALUES(7,'html','/opt/onyx/arm/bin/web_browser','/usr/share/explorer/images/middle/html.png')"
            << "INSERT INTO associations VALUES(8,'txt','/opt/onyx/arm/bin/onyx_reader','/usr/share/explorer/images/middle/txt.png')"
            << "CREATE TABLE applications (id INTEGER PRIMARY KEY, name TEXT, executable TEXT, options TEXT, icon TEXT, category_id NUMERIC)"
            << "INSERT INTO applications VALUES(1,'Sudoku','/opt/onyx/arm/bin/sudoku',NULL,'/usr/share/explorer/images/middle/suduko.png',5)"
            << "INSERT INTO applications VALUES(2,'Dictionary','/opt/onyx/arm/bin/dict_tool',NULL,'/usr/share/explorer/images/middle/dictionary.png',4)"
//            << "INSERT INTO applications VALUES(3,'Text Editor','/opt/onyx/arm/bin/text_editor',NULL,'/usr/share/explorer/images/middle/write_pad.png',4)"
//            << "INSERT INTO applications VALUES(4,'Scribbler','/opt/onyx/arm/bin/image_reader',NULL,'/usr/share/explorer/images/middle/note.png',4)"
            << "CREATE TABLE websites (id INTEGER PRIMARY KEY, name TEXT, url TEXT, icon TEXT)"
            << "INSERT INTO websites VALUES(1,'Google','http://www.google.com','/usr/share/explorer/images/middle/google.png')"
            << "INSERT INTO websites VALUES(2,'BooxUsers','http://booxusers.com','/media/sd/booxusers.png')"
            << "INSERT INTO websites VALUES(3,'Wikipedia','http://en.wikipedia.org','/usr/share/explorer/images/middle/wiki.png')";

    while (active && i < queries.size())
    {
        active = query.exec(queries.at(i));
        i++;
    }

    return active;
}
