#include "include/trecs.h"
#include <cassert>

struct position {
    float x=0,y=0;
};

int main(){
    trecs::registry_t registry;

    trecs::entity_t e1 = registry.create();
    trecs::entity_t e2 = registry.create();
    trecs::entity_t e3 = registry.create();

    registry.addComponent<int>(e3, 24);
    registry.addComponent<position>(e3, {33,44});

    assert(registry.getComponent<int>(e3) == 24);

    position p = registry.getComponent<position>(e3);
    assert(p.x == 33.f && p.y == 44.f);

    registry.addComponent<int>(e1, 12);
    registry.tryAddComponent<int>(e1, 14);
    assert(registry.getComponent<int>(e1) == 12);

    registry.updateComponent<int>(e1, 14);
    assert(registry.getComponent<int>(e1) == 14);

    registry.addComponent<float>(e1, 24.f);
    registry.addComponent<int>(e2, 1024);

    int e2ival;
    if(registry.tryGetComponent<int>(e2, e2ival)){
        assert(e2ival == 1024);
    }

    registry.addComponent<float>(e2, 14.f);

    registry.removeComponent<int>(e1);
    assert(!registry.hasComponent<int>(e1))  ;

    registry.removeComponent<float>(e1);
    assert(!registry.hasComponent<float>(e1))  ;

    registry.tryRemoveComponent<int>(e1);

    return 0;

}
