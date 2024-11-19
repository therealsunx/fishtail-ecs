#include "single-include/trecs.h"
#include <cassert>

#define __norm_cmds_test 1
 

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

    int& x__ = registry.get<int>(e1);
    x__ = 54;
    assert(registry.get<int>(e1) == 54);

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
    registry.destroy(e1);

    e1 = registry.create();
    registry.add<int>(e1, 100);
    registry.add<float>(e1, 3.122f);
    registry.add<char>(e1, 's');
    

    auto d = registry.gett<int, float, char>(e1);
    assert(std::get<int>(d) == 100 && std::get<float>(d) == 3.122f && std::get<char>(d) == 's');
    assert(registry.has<int>(e1));
    bool x = registry.has<int, float>(e1);
    assert(x);
    assert(!(registry.has<int, unsigned int>(e1)));

    registry.remove<int>(e1); 
    registry.tryRemove<float, char, unsigned int>(e1); 
    
    assert(!(registry.has<int, float, char>(e1)));

#else

    for(int i=0; i<10; i++){
        trecs::entity_t e = registry.create();
        registry.add<int>(e, i+1);
        if(i%2 == 0) registry.add<float>(e, i+1.0001f);
    }
    
    auto view = registry.view<float, int>();
    view.forEach([](float fv, int& iv){
            printf("(%f, %d) ", fv, iv);
            iv = 1224;
        });

    auto view2 = registry.view<int>();
    std::cout << std::endl;
    view2.forEach([](int iv){
            printf("(%d) ", iv);
        });

#endif

    return 0;
}
