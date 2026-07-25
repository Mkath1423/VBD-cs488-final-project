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


class ResourceHandle {
    uint64_t id;
    

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


class ResourceManager {
    std::map<uint64_t, std::unique_ptr<Resource>> resources;

    uint64_t next_id = 1;

    public:

    ResourceHandle add(std::unique_ptr<Resource> res);

    template <typename T>
    T *get(ResourceHandle handle) {
        auto it = resources.find(handle.getID());

        if(it != resources.end()) return static_cast<T*>(it->second.get());
    }

    ~ResourceManager();
};
