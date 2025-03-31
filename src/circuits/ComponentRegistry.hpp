#include "models/ComponentModel.hpp"
#include <filesystem>
#include <functional>
#include <string>

using std::filesystem::path;

struct ComponentInfo {
  std::string name;
  SDL_Texture *previewTexture;
  std::function<ComponentModel *()> createFunction;
  void renderPreview(SDL_Renderer *renderer,
                     const SDL_Rect &rect);

};


class ComponentRegistry {
public:
  static ComponentRegistry &instance();
  void registerComponent(const std::string &type, const std::string &name,
                         const path texturePath,
                         std::function<ComponentModel *()> createFunction);

  const std::unordered_map<std::string, ComponentInfo> &getRegistry() const;

  ComponentModel *createComponent(const std::string &type);

private:
  std::unordered_map<std::string, ComponentInfo> registry;
  ComponentRegistry() = default;
};

void registerComponents();
