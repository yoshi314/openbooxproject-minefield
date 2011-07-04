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

#include "onyx/sys/sys.h"
#include "onyx/screen/screen_proxy.h"

namespace obx
{

ObxExplorer::ObxExplorer(int &argc, char **argv)
    : QApplication(argc, argv)
    , view_()
{
    bool mainUI = false;

    connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onScreenSizeChanged()), Qt::QueuedConnection);

    if (argc > 1 && QString::fromLocal8Bit(argv[1]) == "--mainui")
    {
        mainUI = true;
    }

    view_.isMainUI(mainUI);
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

bool isSDMounted()
{
    QFile mtab("/etc/mtab");
    mtab.open(QIODevice::ReadOnly);
    QString content(mtab.readAll());
    return content.contains("/media/sd");
}

}
