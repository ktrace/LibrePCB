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
#include "cmdreplacedevice.h"

#include "../../project/cmd/cmdboardnetsegmentaddelements.h"
#include "../../project/cmd/cmddeviceinstanceremove.h"
#include "cmdadddevicetoboard.h"
#include "cmdcomponentinstanceedit.h"
#include "cmdremoveboarditems.h"

#include <librepcb/core/library/dev/device.h>
#include <librepcb/core/project/board/board.h>
#include <librepcb/core/project/board/items/bi_device.h>
#include <librepcb/core/project/board/items/bi_footprintpad.h>
#include <librepcb/core/project/board/items/bi_netpoint.h>
#include <librepcb/core/project/board/items/bi_netsegment.h>
#include <librepcb/core/project/circuit/componentinstance.h>
#include <librepcb/core/utils/scopeguard.h>

#include <QtCore>

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {
namespace editor {

/*******************************************************************************
 *  Constructors / Destructor
 ******************************************************************************/

CmdReplaceDevice::CmdReplaceDevice(
    Workspace& workspace, Board& board, BI_Device& device,
    const Uuid& newDeviceUuid,
    const std::optional<Uuid>& newFootprintUuid) noexcept
  : UndoCommandGroup(tr("Change Device")),
    mWorkspace(workspace),
    mBoard(board),
    mDeviceInstance(device),
    mNewDeviceUuid(newDeviceUuid),
    mNewFootprintUuid(newFootprintUuid) {
}

CmdReplaceDevice::~CmdReplaceDevice() noexcept {
}

/*******************************************************************************
 *  Inherited from UndoCommand
 ******************************************************************************/

bool CmdReplaceDevice::performExecute() {
  // if an error occurs, undo all already executed child commands
  auto undoScopeGuard = scopeGuard([&]() { performUndo(); });

  // remove all connected netlines
  foreach (BI_FootprintPad* pad, mDeviceInstance.getPads()) {
    BI_NetSegment* netsegment = pad->getNetSegmentOfLines();
    if (netsegment) {
      std::unique_ptr<CmdBoardNetSegmentAddElements> cmdAdd(
          new CmdBoardNetSegmentAddElements(*netsegment));
      QMap<const Layer*, BI_NetPoint*> newNetPoints = {};
      QSet<BI_NetLine*> connectedNetLines = pad->getNetLines();
      if (connectedNetLines.count() > 1) {
        foreach (BI_NetLine* netline, connectedNetLines) {
          auto it = newNetPoints.find(&netline->getLayer());
          if (it == newNetPoints.end()) {
            it = newNetPoints.insert(&netline->getLayer(),
                                     cmdAdd->addNetPoint(pad->getPosition()));
          }
          cmdAdd->addNetLine(**it, *netline->getOtherPoint(*pad),
                             netline->getLayer(), netline->getWidth());
        }
      }
      execNewChildCmd(cmdAdd.release());
      std::unique_ptr<CmdRemoveBoardItems> cmdRemove(
          new CmdRemoveBoardItems(netsegment->getBoard()));
      cmdRemove->removeNetLines(pad->getNetLines());
      execNewChildCmd(cmdRemove.release());  // can throw
    }
  }

  // Remove the device instance-
  execNewChildCmd(new CmdDeviceInstanceRemove(mDeviceInstance));  // can throw

  // If the assembly option is now unused, remove it.
  if (!mDeviceInstance.getComponentInstance().getLockAssembly()) {
    const Uuid oldDevice = mDeviceInstance.getLibDevice().getUuid();
    bool deviceUsedInOtherBoards = false;
    foreach (const BI_Device* device,
             mDeviceInstance.getComponentInstance().getDevices()) {
      if (device->getLibDevice().getUuid() == oldDevice) {
        deviceUsedInOtherBoards = true;
        break;
      }
    }
    if (!deviceUsedInOtherBoards) {
      ComponentAssemblyOptionList options =
          mDeviceInstance.getComponentInstance().getAssemblyOptions();
      for (int i = options.count() - 1; i >= 0; --i) {
        auto option = options.at(i);
        if ((option->getDevice() == oldDevice) &&
            (option->getParts().isEmpty())) {
          options.remove(i);
        }
      }
      std::unique_ptr<CmdComponentInstanceEdit> cmd(
          new CmdComponentInstanceEdit(mDeviceInstance.getCircuit(),
                                       mDeviceInstance.getComponentInstance()));
      cmd->setAssemblyOptions(options);
      execNewChildCmd(cmd.release());
    }
  }

  // Add the new device.
  CmdAddDeviceToBoard* cmd = new CmdAddDeviceToBoard(
      mWorkspace, mBoard, mDeviceInstance.getComponentInstance(),
      mNewDeviceUuid, mNewFootprintUuid, mDeviceInstance.getLibModelUuid(),
      mDeviceInstance.getPosition(), mDeviceInstance.getRotation(),
      mDeviceInstance.getMirrored());
  execNewChildCmd(cmd);  // can throw
  BI_Device* newDevice = cmd->getDeviceInstance();
  Q_ASSERT(newDevice);

  // TODO: reconnect all netpoints/netlines

  undoScopeGuard.dismiss();  // no undo required
  return (getChildCount() > 0);
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace editor
}  // namespace librepcb
