#pragma once
#include <iostream>

#include <fstream>
#include <sstream>

#include <string>
#include <filesystem>


#include <map>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class ResourceManager;

class ResourceHandle {
    protected:    
    uint64_t id;
    std::weak_ptr<ResourceManager> rmRef;

    public:

    explicit ResourceHandle(uint64_t _id) : id{_id} {}

    ResourceHandle(const ResourceHandle&) = default;
    ResourceHandle(ResourceHandle&&) = default;

    ResourceHandle& operator=(const ResourceHandle&) = default;
    ResourceHandle& operator=(ResourceHandle&&) = default;

    uint64_t getID() const;
    bool operator==(const ResourceHandle& rhs) const;
};


class Resource {
    std::vector<ResourceHandle> dependents;
    
    bool isInitialized;

    protected:

    virtual bool init_impl() = 0;
    virtual void cleanup_impl() = 0;
    
    void registerDependency(uint64_t id);

    public:

    Resource() : isInitialized{false} {}

    bool init();
    void destroy();

    virtual ~Resource();
};

template <typename T>
class TypedHandle : ResourceHandle {
    public:

    explicit TypedHandle(ResourceHandle &&handle) : ResourceHandle(handle) {}

    T *get() const;
    T *operator->() const;
};

class ResourceManager {
    std::map<uint64_t, std::unique_ptr<Resource>> resources;

    uint64_t next_id = 1;

    public:

    template<typename T, typename... Args>
    TypedHandle<T> create(Args &&... args) {
        auto resource = std::make_unique<T>(
            std::forward<Args>(args)...
        );

        return TypedHandle<T>(add(std::move(resource)));
    }

    ResourceHandle add(std::unique_ptr<Resource> res);

    Resource *get(ResourceHandle handle) {
        auto it = resources.find(handle.getID());

        if(it != resources.end()) return it->second.get();
    }

    ~ResourceManager();
};

template <typename T>
T *TypedHandle<T>::get() const {
    std::shared_ptr<ResourceManager> rm = rmRef.lock();
    if(!rm) return nullptr;

    Resource * res = rm->get(*this);
    return static_cast<T*>(res);
}

template <typename T>
T *TypedHandle<T>::operator->() const {
    return get();
}
