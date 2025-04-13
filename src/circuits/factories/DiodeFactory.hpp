#pragma once

#include "ComponentFactory.hpp"

class DiodeFactory : public ComponentFactory {
protected:
  static void *texture;
  ~DiodeFactory() = default;
  DiodeFactory();
  ComponentModel *fromJson(const json &data, const int *pins,
                           const size_t &nPins) override;
  string getComponentType() const override;
  json getDefaultJson() const override;
  void *getTexture() const override;
  pair<int, int> getSize() const override;
  vector<pair<int, int>> getPins() const override;
};
