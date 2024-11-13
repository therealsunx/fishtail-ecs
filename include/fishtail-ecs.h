/*
 * ============== Fishtail ECS ==================
 * Author: Sunil Sapkota
 * Description: Fishtail-ECS is a fast and reliable Entity Component System,
 * written for its usage in the game engine - Everest.
 */

#pragma once

# include <iostream>
#include <inttypes.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum componentID_t {
    Transform   = 1<<0,
    Collider    = 1<<1,
    Rigidbody   = 1<<2,
    Sprite      = 1<<3,
};

// e.g. Transform | Collider ....., up to 64 different components supported, which
// is enough for any game
using archeTypeID_t = uint64_t;

struct componentColumn_t {
    void *elements;
    size_t cellSize;
    size_t count;

    void* operator[](size_t index){
        if(index<count && index>0){
            return &((uint8_t*)elements)[cellSize*index];
        }
        return nullptr;
    }
};

using archTypeTable_t = std::unordered_map<componentID_t, componentColumn_t>;

struct archeType_t {
    archeTypeID_t id;
    archTypeTable_t table;
};

using entityID_t = uint32_t;

struct record_t {
    archeType_t& archType;
    size_t row;
};
using entityIndex_t     = std::unordered_map<entityID_t, record_t>;

using archeTypeSet_t    = std::unordered_set<archeTypeID_t&>;
using compArchTsetMap_t = std::unordered_map<componentID_t, archeTypeSet_t>;


class ECSregistry {
    public:
        void* getComponent(entityID_t entity, componentID_t component);

    private:
        entityIndex_t _registry;
        compArchTsetMap_t _compArchMap;
};
