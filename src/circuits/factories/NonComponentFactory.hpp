#pragma once

#include "ComponentFactory.hpp"
#include <filesystem>
#include <unordered_map>

using std::filesystem::path;

class NonComponentFactory : public ComponentFactory {
protected:
  static std::unordered_map<path, void *> textureMap;
  path texturePath;
  vector<pair<int, int>> pins;
public:
  ~NonComponentFactory();
  NonComponentFactory(path texturePath, const vector<pair<int, int>> pins);
  ComponentModel *fromJson(const json &data, const int *pins,
                           const size_t &nPins) override;
  string getComponentType() const override;
  json getDefaultJson() const override;
  void *getTexture() const override;
  pair<int, int> getSize() const override;
  const vector<pair<int, int>> &getPins() const override;
};
