#ifndef CondFormats_MTDObjects_BTLChannelCondObjectContainer_h
#define CondFormats_MTDObjects_BTLChannelCondObjectContainer_h

#include "CondFormats/Serialization/interface/Serializable.h"

#include "CondFormats/MTDObjects/interface/BTLElectronicsId.h"
#include "CondFormats/MTDObjects/interface/MTDContainer.h"
#include "EventFilter/MTDRawToDigi/interface/BTLElectronicsSpecs.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>

/**
 * Container for BTL conditions addressed by Channel ID (BTLElectronicsId).
 */
template <typename T>
class BTLChannelCondObjectContainer {
public:
  typedef T Item;
  typedef Item value_type;
  typedef BTLChannelCondObjectContainer<T> self;
  typedef typename MTDContainer<BTLElectronicsId, Item>::Items Items;
  typedef typename Items::const_iterator const_iterator;
  typedef typename Items::iterator iterator;

  BTLChannelCondObjectContainer() = default;
  ~BTLChannelCondObjectContainer() = default;

  void clear() { btlchs_.clear(); }

  // ---- return full lists of items ----
  inline const Items& btlItems() const { return btlchs_.items(); }

  // ---- Insert the calibration associated with id ----
  inline void insert(std::pair<uint32_t, Item> const& a) { btlchs_.insert(validatedId(a.first), a.second); }

  inline const_iterator find(uint32_t rawId) const {
    const auto id = validatedId(rawId);
    return btlchs_.items().find(id.rawId());
  }

  inline const_iterator end() const { return btlchs_.end(); }

  inline void setValue(const uint32_t id, const Item& item) { (*this)[id] = item; }

  inline const self& getMap() const { return *this; }

  inline size_t size() const { return btlchs_.size(); }

  inline Item& operator[](uint32_t rawId) { return btlchs_[validatedId(rawId)]; }

  inline Item operator[](uint32_t rawId) const { return btlchs_[validatedId(rawId)]; }

private:
  static BTLElectronicsId validatedId(uint32_t rawId) {
    if ((rawId >> 30) != 0) {
      throw std::invalid_argument("BTLChannelCondObjectContainer: the two most significant bits must be zero");
    }
    const BTLElectronicsId id(rawId);
    if (static_cast<uint32_t>(id.channelId()) >= BTLDetId::kCrystalsPerModuleV2 * 2) {
      throw std::invalid_argument("BTLChannelCondObjectContainer: channelId is out of range");
    }
    if (static_cast<uint32_t>(id.eLinkId()) >= BTLDetId::kDModulesPerRU * BTLDetId::kSModulesPerDM) {
      throw std::invalid_argument("BTLChannelCondObjectContainer: eLinkId is out of range");
    }
    if (static_cast<uint32_t>(id.hsLinkId()) < BTLElectronicsSpecs::kHSLinksOffset || 
        static_cast<uint32_t>(id.hsLinkId()) >= BTLElectronicsSpecs::kHSLinksOffset + BTLElectronicsSpecs::kHSLinksNum) {
      throw std::invalid_argument("BTLChannelCondObjectContainer: hsLinkId is out of range");
    }
    return id;
  }

  MTDContainer<BTLElectronicsId, Item> btlchs_;

  COND_SERIALIZABLE;
};

#endif  // CondFormats_MTDObjects_BTLChannelCondObjectContainer_h
