#include "include/trecs.h"

int main(){
    trecs::registry_t registry;

    trecs::entity_t e1 = registry.create();

    registry.addComponent<int>(e1, 12);
    registry.addComponent<float>(e1, 24.12);

    trecs::entity_t e2 = registry.create();

    registry.addComponent<float>(e2, 34.12);

    std::cout <<std::endl << registry.getComponent<int>(e1);
    std::cout <<std::endl << registry.getComponent<float>(e1);
    std::cout <<std::endl << registry.getComponent<float>(e2);

    trecs::entity_t e3 = registry.create();
    registry.addComponent<char>(e3, 's');
    registry.addComponent<int>(e3, 54.f);

    int& s = registry.getComponentRef<int>(e3);
    std::cout <<std::endl << s;
    std::cout <<std::endl << registry.getComponent<int>(e3);
    s++;
    std::cout <<std::endl << s;
    std::cout <<std::endl << registry.getComponent<int>(e3);
    std::cout <<std::endl << registry.getComponent<char>(e3);

    if(registry.hasComponent<unsigned int>(e3)) registry.getComponent<unsigned int>(e3); 
    else std::cout << std::endl << "Nooo";

    return 0;
}
