#ifndef CondFormats_MTDObjects_MTDContainer_h
#define CondFormats_MTDObjects_MTDContainer_h

#include "CondFormats/Serialization/interface/Serializable.h"

#include <cstddef>
#include <cstdint>
#include <map>

/**
 * Generic container for MTD conditions indexed by an identifier Id.
 *
 * Id must provide uint32_t rawId() const (e.g. BTLDetId or
 * BTLElectronicsId). Values are stored using the packed raw ID as key.
 *
 */
template <typename Id, typename T>
class MTDContainer {
public:
  using Item = T;
  using value_type = Item;
  using Items = std::map<uint32_t, Item>;
  using iterator = typename Items::iterator;
  using const_iterator = typename Items::const_iterator;

  MTDContainer() = default;
  ~MTDContainer() = default;

  // Insert or replace the calibration associated with id.
  void setValue(const Id& id, const Item& value) { items_[id.rawId()] = value; }

  // Insert a new calibration. Returns false if id is already present.
  bool insert(const Id& id, const Item& value) { return items_.emplace(id.rawId(), value).second; }

  // Non-const operator[] inserts a default-constructed Item when missing.
  Item& operator[](const Id& id) { return items_[id.rawId()]; }

  // Const access never inserts a new entry; it throws if id is absent.
  const Item& operator[](const Id& id) const { return at(id); }

  Item& at(const Id& id) { return items_.at(id.rawId()); }
  const Item& at(const Id& id) const { return items_.at(id.rawId()); }

  // Return nullptr when id is not present.
  Item* find(const Id& id) {
    auto it = items_.find(id.rawId());
    return it == items_.end() ? nullptr : &it->second;
  }
  const Item* find(const Id& id) const {
    auto it = items_.find(id.rawId());
    return it == items_.end() ? nullptr : &it->second;
  }

  bool contains(const Id& id) const { return items_.find(id.rawId()) != items_.end(); }
  std::size_t erase(const Id& id) { return items_.erase(id.rawId()); }

  std::size_t size() const { return items_.size(); }
  bool empty() const { return items_.empty(); }
  void clear() { items_.clear(); }

  // Iteration exposes pairs (rawId, Item).
  iterator begin() { return items_.begin(); }
  iterator end() { return items_.end(); }
  const_iterator begin() const { return items_.begin(); }
  const_iterator end() const { return items_.end(); }

  const Items& items() const { return items_; }

private:
  Items items_;

  COND_SERIALIZABLE;
};

#endif  // CondFormats_MTDObjects_MTDContainer_h
