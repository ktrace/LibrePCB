/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
 * https://librepcb.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "kicadlibraryimportwizardpage_start.h"

#include "kicadlibraryimportwizardcontext.h"
#include "ui_kicadlibraryimportwizardpage_start.h"

#include <QtCore>
#include <QtWidgets>

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {
namespace editor {

/*******************************************************************************
 *  Constructors / Destructor
 ******************************************************************************/

KiCadLibraryImportWizardPage_Start::KiCadLibraryImportWizardPage_Start(
    std::shared_ptr<KiCadLibraryImportWizardContext> context,
    QWidget* parent) noexcept
  : QWizardPage(parent),
    mUi(new Ui::KiCadLibraryImportWizardPage_Start),
    mContext(context) {
  mUi->setupUi(this);
}

KiCadLibraryImportWizardPage_Start::
    ~KiCadLibraryImportWizardPage_Start() noexcept {
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace editor
}  // namespace librepcb
