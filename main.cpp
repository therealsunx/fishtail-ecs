#include <stdio.h>
#include "include/trecs.h"

int main(){
    trecs::registry_t registry;

    trecs::entity_t e1 = registry.create();
    trecs::entity_t e2 = registry.create();

    registry.addComponent<int>(e1, 12);
    registry.addComponent<float>(e2, 24.12f);
    registry.addComponent<uint32_t>(e1, 1224);
    registry.addComponent<bool>(e1, true);


#if 0
    printf("%lu %lu %lu %lu",
            registry.getComponentID<int>(),
            registry.getComponentID<float>(),
            registry.getComponentID<bool>(),
            registry.getComponentID<uint32_t>()
        );
#endif
    
    printf("%d %f %u %d",
            registry.getComponent<int>(e1),
            registry.getComponent<float>(e2),
            registry.getComponent<bool>(e1),
            registry.getComponent<uint32_t>(e1)
          );

    return 0;
}
