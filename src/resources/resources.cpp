#include "resources.h"





// void ManagedResource::registerDependency(uint64_t id){
//     dependents.emplace_back(id);
// }


bool ManagedResource::init(){
    if(!isInitialized){
        isInitialized = init_impl();
    }

    return isInitialized;
}

void ManagedResource::destroy(){
    if(isInitialized) cleanup_impl();
}


ManagedResource::~ManagedResource() {};


uint64_t ResourceManager::add(std::unique_ptr<ManagedResource> res) {
    if(res) res->init();
    
    auto id = next_id++;

    resources.emplace(id, std::move(res));

    return id;
}

ManagedResource *ResourceManager::get(uint64_t id) {
    auto it = resources.find(id);
    if(it != resources.end()) return it->second.get();
    return nullptr;
}

ResourceManager::~ResourceManager(){
    for (auto& [_, res] : resources) {
        if(res) res->destroy();
    }
}
