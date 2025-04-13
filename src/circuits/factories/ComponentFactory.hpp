#pragma once

#include "../models/ComponentModel.hpp"
#include <string>
#include <utility>

using std::pair;
using std::string;

class ComponentFactory {
public:
  virtual ~ComponentFactory() = default;
  virtual ComponentModel *fromJson(const json &data, const int *pins,
                                   const size_t &nPins) = 0;
  virtual string getComponentType() const = 0;
  virtual json getDefaultJson() const = 0;
  virtual void *getTexture() const = 0;
  virtual pair<int, int> getSize() const = 0;
  virtual vector<pair<int, int>> getPins() const = 0;
};
