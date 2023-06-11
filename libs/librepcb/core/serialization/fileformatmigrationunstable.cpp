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
#include "fileformatmigrationunstable.h"

#include "../application.h"
#include "../fileio/transactionaldirectory.h"

#include <QtCore>

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {

/*******************************************************************************
 *  Constructors / Destructor
 ******************************************************************************/

FileFormatMigrationUnstable::FileFormatMigrationUnstable(
    QObject* parent) noexcept
  : FileFormatMigrationV01(parent) {
  mFromVersion = mToVersion;  // Clearly distinguish from the base class.
}

FileFormatMigrationUnstable::~FileFormatMigrationUnstable() noexcept {
}

/*******************************************************************************
 *  General Methods
 ******************************************************************************/

void FileFormatMigrationUnstable::upgradeComponentCategory(
    TransactionalDirectory& dir) {
  Q_UNUSED(dir);
}

void FileFormatMigrationUnstable::upgradePackageCategory(
    TransactionalDirectory& dir) {
  Q_UNUSED(dir);
}

void FileFormatMigrationUnstable::upgradeSymbol(TransactionalDirectory& dir) {
  Q_UNUSED(dir);
}

void FileFormatMigrationUnstable::upgradePackage(TransactionalDirectory& dir) {
  Q_UNUSED(dir);
  const QString fp = "package.lp";
  SExpression root = SExpression::parse(dir.read(fp), dir.getAbsPath(fp));
  upgradeLayers(root);
  dir.write(fp, root.toByteArray());
}

void FileFormatMigrationUnstable::upgradeComponent(
    TransactionalDirectory& dir) {
  Q_UNUSED(dir);
}

void FileFormatMigrationUnstable::upgradeDevice(TransactionalDirectory& dir) {
  Q_UNUSED(dir);
}

void FileFormatMigrationUnstable::upgradeLibrary(TransactionalDirectory& dir) {
  Q_UNUSED(dir);
}

void FileFormatMigrationUnstable::upgradeWorkspaceData(
    TransactionalDirectory& dir) {
  Q_UNUSED(dir);
  const QString settingsFp = "settings.lp";
  if (dir.fileExists(settingsFp)) {
    SExpression root =
        SExpression::parse(dir.read(settingsFp), dir.getAbsPath(settingsFp));
    root.replaceRecursive(SExpression::createToken("board_placement_top"),
                          SExpression::createToken("board_legend_top"));
    root.replaceRecursive(SExpression::createToken("board_placement_bottom"),
                          SExpression::createToken("board_legend_bottom"));
    dir.write(settingsFp, root.toByteArray());
  }
}

/*******************************************************************************
 *  Private Methods
 ******************************************************************************/

void FileFormatMigrationUnstable::upgradeSettings(SExpression& root) {
  Q_UNUSED(root);
}

void FileFormatMigrationUnstable::upgradeCircuit(SExpression& root) {
  Q_UNUSED(root);
}

void FileFormatMigrationUnstable::upgradeErc(SExpression& root,
                                             ProjectContext& context) {
  Q_UNUSED(root);
  Q_UNUSED(context);
}

void FileFormatMigrationUnstable::upgradeSchematic(SExpression& root,
                                                   ProjectContext& context) {
  Q_UNUSED(root);
  Q_UNUSED(context);
}

void FileFormatMigrationUnstable::upgradeBoard(SExpression& root,
                                               ProjectContext& context) {
  Q_UNUSED(root);
  Q_UNUSED(context);
  upgradeLayers(root);
}

void FileFormatMigrationUnstable::upgradeBoardUserSettings(SExpression& root) {
  Q_UNUSED(root);
  upgradeLayers(root);
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace librepcb
