#include "models/ComponentModel.hpp"
#include <filesystem>
#include <functional>
#include <string>

using std::filesystem::path;

struct ComponentInfo {
  std::string name;
  SDL_Texture *previewTexture;
  std::function<ComponentModel *()> createFunction;
  void renderPreview(SDL_Renderer *renderer, const SDL_Rect &rect);
  int xSize = 2.0;
  int ySize = 2.0;
};

class ComponentRegistry {
public:
  static ComponentRegistry &instance();
  void registerComponent(const std::string &type, const std::string &name,
                         const path texturePath,
                         std::function<ComponentModel *()> createFunction,
                         int xSize = 2.0, int ySize = 2.0);

  const std::unordered_map<std::string, ComponentInfo> &getRegistry() const;
  static ComponentInfo getComponent(std::string id);
  ComponentModel *createComponent(const std::string &type);

private:
  std::unordered_map<std::string, ComponentInfo> registry;
  ComponentRegistry() = default;
};

void registerComponents();
