#include "engine.h"

uint64_t ResourceHandle::getID() const {
    return id;
}

bool ResourceHandle::operator==(const ResourceHandle& rhs) const {
    return this->getID() == rhs.getID();
}