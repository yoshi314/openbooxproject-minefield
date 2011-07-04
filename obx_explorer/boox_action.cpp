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

#include <QtGui>

#include "boox_action.h"

namespace obx
{

CBooxActions::CBooxActions() : ui::BaseActions()
{
}

void CBooxActions::AddAction(QIcon icon, QString title, int itemKey)
{
    QAction* newAction=new QAction(icon, title, this);
    newAction->setCheckable(true);
    newAction->setProperty("key", QVariant(itemKey));
    actions_.push_back(shared_ptr<QAction>(newAction));
}

void CBooxActions::InitializeActions(QIcon icon, QString title)
{
    clear();
    category_action_.setIcon(icon);
    category_action_.setIconText(title);
}

int CBooxActions::selected()
{
    for (unsigned int i = 0; i < actions_.size(); i++)
    {
        if (actions_[i]->isChecked())
            return actions_[i]->property("key").toInt();
    }

    return -1;
}

}
