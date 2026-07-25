#include "resources.h"



uint64_t ResourceHandle::getID() const {
    return id;
}

bool ResourceHandle::operator==(const ResourceHandle& rhs) const {
    return this->getID() == rhs.getID();
}

void Resource::registerDependency(uint64_t id){
    dependents.emplace_back(id);
}


bool Resource::init(){
    if(!isInitialized){
        isInitialized = init_impl();
    }

    return isInitialized;
}

void Resource::destroy(){
    if(isInitialized) cleanup_impl();
}


Resource::~Resource() {};



ResourceHandle ResourceManager::add(std::unique_ptr<Resource> res) {
    if(res) res->init();
    
    auto id = next_id;

    resources.emplace(id, std::move(res));

    return ResourceHandle(id);
}

ResourceManager::~ResourceManager(){
    for (auto& [_, res] : resources) {
        if(res) res->destroy();
    }
}
