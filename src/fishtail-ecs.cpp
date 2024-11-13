#include "../include/fishtail-ecs.h"
#include "../include/utils.h"


void* ECSregistry::getComponent(entityID_t entity, componentID_t component){
    auto it = _registry.find(entity);
    Assert(it != _registry.end(), "Invalid entityID");

    archeType_t& archType = it->second.archType;
    if(!(archType.id & component)) return nullptr;

    // For future debugger
    // if any errors in next line, then archType.id is not matching the map of
    // componentID -> column. Most probably add component did not work as expected
    // or some code manually modifies thee archType table.
    return archType.table[component][it->second.row];
}
