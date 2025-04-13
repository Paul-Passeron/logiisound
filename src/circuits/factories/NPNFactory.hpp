#pragma once

#include "ComponentFactory.hpp"

class NPNFactory : public ComponentFactory {
protected:
  static void *texture;

public:
  ~NPNFactory() = default;
  NPNFactory();
  ComponentModel *fromJson(const json &data, const int *pins,
                           const size_t &nPins) override;
  string getComponentType() const override;
  json getDefaultJson() const override;
  void *getTexture() const override;
  pair<int, int> getSize() const override;
  vector<pair<int, int>> getPins() const override;
};
