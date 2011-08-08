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
#include "database_utils.h"

using namespace obx;

static bool createConnections();

int main(int argc, char *argv[])
{
    createConnections();

    ObxExplorer app(argc, argv);

    Q_INIT_RESOURCE(onyx_ui_images);

    return app.exec();
}

static bool createConnections()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/root/obx_explorer.db");
    if (!db.open())
    {
        qDebug() << "Database Error";
        return false;
    }

    if (db.record("revision").isEmpty())    // TODO also check database revision
    {
        if (db.tables().size())
        {
            qDebug() << "clearing database...";
            if (!DatabaseUtils::clearDatabase())
            {
                qDebug() << "...clearing failed!";
                return false;
            }
        }

        qDebug() << "initializing database...";
        if (!obx::initializeDatabase())
        {
            qDebug() << "...initialization failed!";
            return false;
        }
    }

    db = QSqlDatabase::addDatabase("QSQLITE", "ONYX");
    db.setDatabaseName("/root/content.db");
    if (!db.open())
    {
        qDebug() << "Database Error";
        return false;
    }

    return true;
}
