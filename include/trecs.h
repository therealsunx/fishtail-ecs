/*
 * ============== TR-ECS ==================
 * Author: Sunil Sapkota
 * Description: TR-ECS is a fast and reliable Entity Component System,
 * written for its usage in the game engine - Everest. It has efficient implementation
 * for adding, removing, quering or getting the components from entities. It uses
 * archetype graph method for organizing data neatly, so that there is no memory wastage
 * or leak. Entities recycling is also supported by the way. As of now it supports 64
 * different component types, which should be enough for any game engine.
 */

#pragma once


#include "archetype.h"


namespace trecs {
    using entity_t = uint32_t;

    struct record_t {
        archetype_t* archeType;
        size_t index = 0;

#ifdef TR_DEBUG 
        inline void debugsymbols(){
            std::cout << "\nrecord - index(" << index << ") : archetype("
                << std::bitset<32>(archeType->id) << ")";
        }
#endif
    };

    using archetype_map_t = std::unordered_map<archetype_id_t, archetype_t>;
    using entity_records_t = std::vector<record_t>;

    class registry_t {
        public:
            registry_t(){
                _archetypeStore[0] = {/*root*/};
            }

            entity_t create(){
                _records.push_back(record_t{&_archetypeStore[0], 0});
#ifdef TR_DEBUG 
                std::cout << "\n==== records ====";
                for(auto& it:_records){
                    it.debugsymbols();
                }
                std::cout << std::endl;
#endif
                return __entity_generator++;
            }

            template<typename T>
            void addComponent(const entity_t entity, T data){
                comp_id_t c_id = _get_comp_type_id<T>();

                Assert(entity < _records.size(), "Invalid entity");
                record_t& rec = _records[entity];
                archetype_t *p_arch = rec.archeType;
                
                archetype_id_t id = c_id | p_arch->id;

                Assert(!(p_arch->id & c_id), "Component already exists on the entity. TODO: override");
                
                archetype_t* n_arch = p_arch->has_plus(c_id)
                    ? p_arch->get_plus(c_id)
                    : p_arch->add_plus(c_id, _getNewArchetype(id));

                entry_t en = p_arch->remove_entry(rec.index);
                en[c_id] = data;
                rec.index = n_arch->add_entry(en);
                rec.archeType = n_arch;
#ifdef TR_DEBUG 
                std::cout << "\n\n================= rec added : " << entity << "-" << c_id << " ===================";
                p_arch->debugsymbols();
                rec.debugsymbols();
                n_arch->debugsymbols();
#endif
            }

            template<typename T>
            inline void removeComponent(entity_t entity){
                comp_id_t c_id = _get_comp_type_id<T>();
                Assert(entity < _records.size(), "Invalid entity");
                record_t& rec = _records[entity];
                archetype_t *p_arch = rec.archeType;
                Assert(p_arch->id & c_id, "Attempt to remove non-existent component");

                archetype_id_t id = p_arch->id & (~c_id);
                archetype_t* n_arch = p_arch->has_minus(c_id)
                    ? p_arch->get_minus(c_id)
                    : p_arch->add_minus(c_id, _getNewArchetype(id));
                entry_t en = p_arch->remove_entry(rec.index);
                Assert(en.find(c_id) != en.end(), "Failed to remove component");
                en.erase(c_id);

                rec.index = n_arch->add_entry(en);
                rec.archeType = n_arch;
#ifdef TR_DEBUG 
                std::cout << "\n\n================= rec removed : " << entity << "-" << c_id << " ===================";
                p_arch->debugsymbols();
                rec.debugsymbols();
                n_arch->debugsymbols();
#endif
            }

            template<typename T>
            inline bool hasComponent(const entity_t entity){
                Assert(entity < _records.size(), "Invalid entity");
                return _get_comp_type_id<T>() & _records[entity].archeType->id;
            }

            template<typename T>
            inline T getComponent(const entity_t entity){
                Assert(entity < _records.size(), "Invalid entity");
                record_t& rec = _records[entity];
                return std::any_cast<T>(rec.archeType->table[_get_comp_type_id<T>()]
                        .at(rec.index));
            }
            template<typename T>
            inline T& getComponentRef(const entity_t entity){
                Assert(entity < _records.size(), "Invalid entity");
                record_t& rec = _records[entity];
                return std::any_cast<T&>(rec.archeType->table[_get_comp_type_id<T>()]
                        .at(rec.index));
            }
# if 0
            template<typename T>
            inline comp_id_t getComponentID(){
                return _get_comp_type_id<T>();
            }
#endif

        private:
            entity_records_t _records;
            archetype_map_t _archetypeStore;
            entity_t __entity_generator = 0;

            inline archetype_t* _getNewArchetype(archetype_id_t id){
                if(_archetypeStore.find(id) != _archetypeStore.end()) return &_archetypeStore[id];
                auto& a = _archetypeStore.try_emplace(id, archetype_t{}).first->second;
                a.id = id;
                return &a;
            }

        private:
            uint32_t _comp_t_counter = 0;
            template<typename t>
            inline comp_id_t _get_comp_type_id(){
                static comp_id_t id = 1ull << _comp_t_counter++;
                return id;
            }
    };
}
