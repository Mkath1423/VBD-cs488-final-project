#pragma once

#include "resources/resources.h"

class ResourceHandle {
    protected:    
    uint64_t id;
    std::shared_ptr<ResourceManager> rm;

    public:

    ResourceHandle(uint64_t _id, std::shared_ptr<ResourceManager> _rm) : 
        id{_id}, rm{_rm} {}

    uint64_t getID() const;
    bool operator==(const ResourceHandle& rhs) const;
};


template <typename T>
class Resource : ResourceHandle {
    
    public:
    Resource(uint64_t _id, std::shared_ptr<ResourceManager> _rm) : 
        ResourceHandle(_id, _rm) {}

    T *get() const;
    T *operator->() const;
};

class Engine {
    std::shared_ptr<ResourceManager> resourceManager;

    private:
        void init(); //  create the window
        void cleanup(); // take down gl and such

    public:
        Engine() : resourceManager{std::make_shared<ResourceManager>()} {}

        template<typename T, typename... Args>
        Resource<T> create(Args &&... args) {

            if(!resourceManager) return Resource<T>(0, nullptr);

            auto resource = std::make_unique<T>(
                std::forward<Args>(args)...
            );

            auto id = resourceManager->add(std::move(resource));

            return Resource<T>(id, resourceManager);
        }

        ~Engine() {
            
        }
};

template <typename T>
T *Resource<T>::get() const {
    if(!rm) return nullptr;

    ManagedResource * res = rm->get(id);
    return static_cast<T*>(res);
}

template <typename T>
T *Resource<T>::operator->() const {
    return get();
}
