#include "resources/resources.h"


class Engine {
    std::shared_ptr<ResourceManager> resourceManager;

    Engine() : resourceManager{std::make_shared<ResourceManager>()} {}


};