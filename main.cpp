#include "include/trecs.h"
#include <cassert>

#define __norm_cmds_test 0
#define __query_cmd_test 1
 

// TODO: Do I want archtype graph to be updated on each add new component
// TODO: If any case, make an iterator that goes over the query
// TODO: query the components individually : <comp1>
// TODO: query the component collection/pairs like : <comp1, comp2>


struct position {
    float x=0,y=0;
};

int main(){
    trecs::registry_t registry;

#if __norm_cmds_test
    trecs::entity_t e1 = registry.create();
    trecs::entity_t e2 = registry.create();
    trecs::entity_t e3 = registry.create();

    registry.add<int>(e3, 24);
    registry.add<position>(e3, {33,44});

    assert(registry.get<int>(e3) == 24);

    position p = registry.get<position>(e3);
    assert(p.x == 33.f && p.y == 44.f);

    registry.add<int>(e1, 12);
    registry.tryAdd<int>(e1, 14);
    assert(registry.get<int>(e1) == 12);

    registry.addOrUpdate<int>(e1, 21);
    assert(registry.get<int>(e1) == 21);

    registry.update<int>(e1, 14);
    assert(registry.get<int>(e1) == 14);

    registry.add<float>(e1, 24.f);
    registry.add<int>(e2, 1024);

    int e2ival;
    if(registry.tryGet<int>(e2, e2ival)){
        assert(e2ival == 1024);
    }

    registry.add<float>(e2, 14.f);

    registry.remove<int>(e1);
    assert(!registry.has<int>(e1))  ;

    registry.remove<float>(e1);
    assert(!registry.has<float>(e1))  ;

    registry.tryRemove<int>(e1);
    
    registry.destroy(e1);
    registry.destroy(e2);

    trecs::entity_t e4 = registry.create();
    registry.add<int>(e4, 100);

    assert(registry.get<int>(e4) == 100);
#endif
#if __query_cmd_test
    trecs::entity_t e1 = registry.create();

    registry.add<int>(e1, 100);
    registry.add<float>(e1, 3.122f);
    registry.add<char>(e1, 's');
    

    auto d = registry.gets<int, float, char>(e1);
    assert(std::get<int>(d) == 100 && std::get<float>(d) == 3.122f && std::get<char>(d) == 's');
    assert(registry.has<int>(e1));
    bool x = registry.has<int, float>(e1);
    assert(x);
    assert(!(registry.has<int, unsigned int>(e1)));

    registry.remove<int>(e1); 
    registry.tryRemove<float, char, unsigned int>(e1); 
    
    assert(!(registry.has<int, float, char>(e1)));

#endif

    printf("All test successful");
    return 0;
}
