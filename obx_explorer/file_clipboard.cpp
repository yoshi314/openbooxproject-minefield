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

#include <QFile>
#include <QFileInfo>

#include "file_system_utils.h"
#include "file_clipboard.h"

namespace obx
{

FileClipboard::FileClipboard()
{
    clear();
}

FileClipboard::~FileClipboard()
{
}

void FileClipboard::cut(QString absoluteFilePath)
{
    QFileInfo fileInfo(absoluteFilePath);

    if (fileInfo.isDir())
    {
        actionFunction_ = FileSystemUtils::moveDir;
    }
    else
    {
        actionFunction_ = QFile::rename;
    }

    absolutePath_ = fileInfo.absolutePath();
    fileName_ = fileInfo.fileName();
    holdsDir_ = fileInfo.isDir();
}

void FileClipboard::copy(QString absoluteFilePath)
{
    QFileInfo fileInfo(absoluteFilePath);

    if (fileInfo.isDir())
    {
        actionFunction_ = FileSystemUtils::copyDir;
    }
    else
    {
        actionFunction_ = QFile::copy;
    }

    absolutePath_ = fileInfo.absolutePath();
    fileName_ = fileInfo.fileName();
    holdsDir_ = fileInfo.isDir();
}

bool FileClipboard::paste(QString absolutePath)
{
    return (*actionFunction_)(absolutePath_ + "/" + fileName_, absolutePath + "/" + fileName_);
}

void FileClipboard::clear()
{
    actionFunction_ = NULL;
}

bool FileClipboard::isEmpty()
{
    return (actionFunction_ == NULL);
}

bool FileClipboard::holdsDir()
{
    return holdsDir_;
}

QString FileClipboard::fileName()
{
    return fileName_;
}

bool (*FileClipboard::actionFunction_)( const QString &, const QString &) = NULL;
QString FileClipboard::absolutePath_;
QString FileClipboard::fileName_;
bool FileClipboard::holdsDir_ = false;

}
