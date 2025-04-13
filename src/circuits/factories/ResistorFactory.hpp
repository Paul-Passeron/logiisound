#pragma once

#include "ComponentFactory.hpp"

class ResistorFactory : public ComponentFactory {
protected:
  static void *texture;

public:
  ~ResistorFactory() = default;
  ResistorFactory();
  ComponentModel *fromJson(const json &data, const int *const pins,
                           const size_t &nPins) override;
  string getComponentType() const override;
  json getDefaultJson() const override;
  void *getTexture() const override;
  pair<int, int> getSize() const override;
  vector<pair<int, int>> getPins() const override;
};
