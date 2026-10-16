#ifndef CondFormats_MTDObjects_MTDCondObjectContainer_h
#define CondFormats_MTDObjects_MTDCondObjectContainer_h

#include "CondFormats/Serialization/interface/Serializable.h"

#include "DataFormats/DetId/interface/DetId.h"
#include "CondFormats/MTDObjects/interface/MTDContainer.h"
#include "DataFormats/ForwardDetId/interface/MTDDetId.h"
#include "DataFormats/ForwardDetId/interface/BTLDetId.h"
#include "DataFormats/ForwardDetId/interface/ETLDetId.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>


/**
 * Container for MTD conditions addressed by BTL/ETL DetId.
 *
 */
template <typename T>
class MTDCondObjectContainer {
public:
  typedef T Item;
  typedef Item value_type;
  typedef MTDCondObjectContainer<T> self;
  typedef typename MTDContainer<BTLDetId, Item>::Items Items;
  typedef typename Items::const_iterator const_iterator;
  typedef typename Items::iterator iterator;

  MTDCondObjectContainer() = default;
  ~MTDCondObjectContainer() = default;

  void clear() {
    btl_.clear();
    etl_.clear();
  }

  // ---- return full lists of items ----
  inline const Items &btlItems() const { return btl_.items(); };
  inline const Items &etlItems() const { return etl_.items(); };

 
  // ---- Insert the calibration associated with id ----
  inline void insert(std::pair<uint32_t, Item> const &a) {


    MTDDetId id(a.first);
    if (id.det() != DetId::Forward) {
      throw std::invalid_argument("MTDCondObjectContainer: rawId is not a Forward DetId");   
    } 
    if (id.subdetId() != MTDDetId::FastTime) {
      throw std::invalid_argument("MTDCondObjectContainer: rawId is not an MTD DetId");   
    }

    switch (id.mtdSubDetector()) {
      case MTDDetId::BTL: {
        btl_.insert(a);
      } break;
      case MTDDetId::ETL: {
        etl_.insert(a);
      } break;
      default:
        throw std::invalid_argument("MTDCondObjectContainer: subdetId is not BTL or ETL");   
        return;
    }
  }
 
  inline const_iterator find(uint32_t rawId) const {
    MTDDetId id(rawId);
    switch (id.mtdSubDetector()) {
        case MTDDetId::BTL: {
            const_iterator it = btl_.find(rawId);
            if (it != btl_.end()) {
                return it;
            } else {
                return etl_.end();
            }
        } break;
        case MTDDetId::ETL: {
            return etl_.find(rawId);
        } break;
        default:
            // FIXME (add throw)
            return etl_.end();
    }
    return etl_.end();
  }
 
  inline const_iterator end() const { return etl_.end(); }

  inline void setValue(const uint32_t id, const Item &item) { (*this)[id] = item; }

  inline const self &getMap() const { return *this; }

  inline size_t size() const { return btl_.size() + etl_.size(); }
  inline size_t sizeBTL() const { return btl_.size(); }
  inline size_t sizeETL() const { return etl_.size(); }

  

  inline Item &operator[](uint32_t rawId) {
    MTDDetId id(rawId);
    return (id.mtdSubDetector() == MTDDetId::BTL) ? btl_[rawId] : etl_[rawId];
  }

  inline Item operator[](uint32_t rawId) const {
    MTDDetId id(rawId);
    switch (id.mtdSubDetector()) {
      case MTDDetId::BTL: {
        return btl_[rawId];
      } break;
      case MTDDetId::ETL: {
        return etl_[rawId];
      } break;
      default:
        // FIXME (add throw)
        // sizeof(Item) <= sizeof(int64_t) for all Items.
        return Item();
    }
  }

private:
  MTDContainer<BTLDetId, Item> btl_;
  MTDContainer<ETLDetId, Item> etl_;

  COND_SERIALIZABLE;
};

#endif  // CondFormats_MTDObjects_MTDCondObjectContainer_h
