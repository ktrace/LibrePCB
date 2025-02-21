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

#ifndef LIBREPCB_CORE_COMPONENTPINSIGNALMAP_H
#define LIBREPCB_CORE_COMPONENTPINSIGNALMAP_H

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "../../serialization/serializableobjectlist.h"
#include "../../types/uuid.h"
#include "cmpsigpindisplaytype.h"

#include <QtCore>

/*******************************************************************************
 *  Namespace / Forward Declarations
 ******************************************************************************/
namespace librepcb {

/*******************************************************************************
 *  Class ComponentPinSignalMapItem
 ******************************************************************************/

/**
 * @brief The ComponentPinSignalMapItem class maps a symbol pin to a component
 * signal
 *
 * Following information is considered as the "interface" of a
 * pin-signal-mapping and must therefore never be changed:
 *  - Pin UUID
 *  - Signal UUID
 */
class ComponentPinSignalMapItem final {
  Q_DECLARE_TR_FUNCTIONS(ComponentPinSignalMapItem)

public:
  // Signals
  enum class Event {
    PinUuidChanged,
    SignalUuidChanged,
    DisplayTypeChanged,
  };
  Signal<ComponentPinSignalMapItem, Event> onEdited;
  typedef Slot<ComponentPinSignalMapItem, Event> OnEditedSlot;

  // Constructors / Destructor
  ComponentPinSignalMapItem() = delete;
  ComponentPinSignalMapItem(const ComponentPinSignalMapItem& other) noexcept;
  ComponentPinSignalMapItem(const Uuid& pin, const std::optional<Uuid>& signal,
                            const CmpSigPinDisplayType& displayType) noexcept;
  explicit ComponentPinSignalMapItem(const SExpression& node);
  ~ComponentPinSignalMapItem() noexcept;

  // Getters
  const Uuid& getUuid() const noexcept {
    return mPinUuid;
  }  // used for UuidObjectMap
  const Uuid& getPinUuid() const noexcept { return mPinUuid; }
  const std::optional<Uuid>& getSignalUuid() const noexcept {
    return mSignalUuid;
  }
  const CmpSigPinDisplayType& getDisplayType() const noexcept {
    return mDisplayType;
  }

  // Setters
  bool setSignalUuid(const std::optional<Uuid>& uuid) noexcept;
  bool setDisplayType(const CmpSigPinDisplayType& type) noexcept;

  // General Methods

  /**
   * @brief Serialize into ::librepcb::SExpression node
   *
   * @param root    Root node to serialize into.
   */
  void serialize(SExpression& root) const;

  // Operator Overloadings
  bool operator==(const ComponentPinSignalMapItem& rhs) const noexcept;
  bool operator!=(const ComponentPinSignalMapItem& rhs) const noexcept {
    return !(*this == rhs);
  }
  ComponentPinSignalMapItem& operator=(
      const ComponentPinSignalMapItem& rhs) noexcept;

private:  // Data
  Uuid mPinUuid;  ///< must be valid
  std::optional<Uuid>
      mSignalUuid;  ///< std::nullopt if not connected to a signal
  CmpSigPinDisplayType mDisplayType;
};

/*******************************************************************************
 *  Class PinSignalMap
 ******************************************************************************/

struct ComponentPinSignalMapNameProvider {
  static constexpr const char* tagname = "pin";
};
using ComponentPinSignalMap =
    SerializableObjectList<ComponentPinSignalMapItem,
                           ComponentPinSignalMapNameProvider,
                           ComponentPinSignalMapItem::Event>;

/*******************************************************************************
 *  Class ComponentPinSignalMapHelpers
 ******************************************************************************/

class ComponentPinSignalMapHelpers {
public:
  ComponentPinSignalMapHelpers() = delete;  // disable instantiation

  static ComponentPinSignalMap create(
      const QSet<Uuid> pins,
      const CmpSigPinDisplayType& display =
          CmpSigPinDisplayType::componentSignal()) noexcept {
    ComponentPinSignalMap map;
    foreach (const Uuid& pin, pins) {
      map.append(std::make_shared<ComponentPinSignalMapItem>(pin, std::nullopt,
                                                             display));
    }
    return map;
  }
};

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace librepcb

#endif
