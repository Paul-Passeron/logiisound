#include "models/ComponentModel.hpp"
#include <functional>
#include <string>

struct ComponentInfo {
  std::string name;
  std::function<ComponentModel *()> createFunction;
};

class ComponentRegistry {
public:
  static ComponentRegistry &instance();
  void registerComponent(const std::string &type, const std::string &name,
                         std::function<ComponentModel *()> createFunction);

  const std::unordered_map<std::string, ComponentInfo> &getRegistry() const;

  ComponentModel *createComponent(const std::string &type);

private:
  std::unordered_map<std::string, ComponentInfo> registry;
  ComponentRegistry() = default;
};

void registerComponents();
