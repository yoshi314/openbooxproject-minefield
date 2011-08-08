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

#ifndef FILE_CLIPBOARD_H_
#define FILE_CLIPBOARD_H_

class QString;

namespace obx
{

class FileClipboard
{
public:
    FileClipboard();
    ~FileClipboard();

public:
    void cut(QString absoluteFilePath);
    void copy(QString absoluteFilePath);
    bool paste(QString absolutePath);
    void clear();
    bool isEmpty();
    bool holdsDir();
    QString fileName();

private:
    static bool (*actionFunction_)( const QString &, const QString &);
    static QString absolutePath_;
    static QString fileName_;
    static bool holdsDir_;
};

}

#endif // FILE_CLIPBOARD_H_
