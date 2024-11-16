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

    };

    using archetype_map_t = std::unordered_map<archetype_id_t, archetype_t>;
    using entity_records_t = std::vector<record_t>;

    class registry_t {
        public:
            registry_t(){
                _archetypeStore[0] = {/*root*/};
            }

            inline entity_t create(){
                _records.push_back(record_t{&_archetypeStore[0], 0});
                return __entity_generator++;
            }

            template<typename T>
            inline void tryAddComponent(const entity_t entity, T data){
                if(hasComponent<T>(entity)) return;
                addComponent<T>(entity, data);
            }

            template<typename T>
            inline void forceAddComponent(const entity_t entity, T data){
                if(hasComponent<T>(entity)) updateComponent<T>(entity, data);
                else addComponent<T>(entity, data);
            }

            template<typename T>
            inline void updateComponent(const entity_t entity, T data){
                comp_id_t c_id = _get_comp_type_id<T>();
                record_t& rec = _records[entity];
                Assert(rec.archeType->id & c_id, "Entity does not have the component to update");
                (*rec.archeType)[c_id][rec.index] = data;
            }

            template<typename T>
            void addComponent(const entity_t entity, T data){
                comp_id_t c_id = _get_comp_type_id<T>();
                record_t& rec = _records[entity];
                archetype_t *p_arch = rec.archeType;

                Assert(!(p_arch->id & c_id), "Component already exists on the entity. TODO: override");
                
                archetype_t* n_arch = p_arch->has_plus(c_id)
                    ? p_arch->get_plus(c_id)
                    : p_arch->add_plus(c_id, _getNewArchetype(c_id | p_arch->id));

                entry_t en = p_arch->remove_entry(rec.index);
                en[c_id] = data;
                rec.index = n_arch->add_entry(en);
                rec.archeType = n_arch;
            }

            template<typename T>
            inline void tryRemoveComponent(entity_t entity){
                if(hasComponent<T>(entity)) removeComponent<T>(entity);
            }

            template<typename T>
            inline void removeComponent(entity_t entity){
                comp_id_t c_id = _get_comp_type_id<T>();

                record_t& rec = _records[entity];
                archetype_t* p_arch = rec.archeType;
                Assert(p_arch->id & c_id, "Attempt to remove non-existent component");

                archetype_t* n_arch = p_arch->has_minus(c_id)
                    ? p_arch->get_minus(c_id)
                    : p_arch->add_minus(c_id, _getNewArchetype(p_arch->id & (~c_id)));

                entry_t en = p_arch->remove_entry(rec.index);
                en.erase(c_id);
                rec.index = n_arch->add_entry(en);
                rec.archeType = n_arch;
            }

            template<typename T>
            inline bool hasComponent(const entity_t entity){
                Assert(entity < _records.size(), "Invalid entity");
                return _get_comp_type_id<T>() & _records[entity].archeType->id;
            }

            template<typename T>
            inline bool tryGetComponent(const entity_t entity, T& output){
                if(!hasComponent<T>(entity)) return false;
                output = getComponent<T>(entity);
                return true;
            }

            template<typename T>
            inline T getComponent(const entity_t entity){
                Assert(entity < _records.size(), "Invalid entity");
                comp_id_t c_id = _get_comp_type_id<T>();
                record_t& rec = _records[entity];
                Assert(rec.archeType->id & c_id, "Entity does not have the component");
                return std::any_cast<T>(rec.archeType->table[_get_comp_type_id<T>()]
                        .at(rec.index));
            }

# if 0
            template<typename T>
                inline T& getComponentRef(const entity_t entity){
                    Assert(entity < _records.size(), "Invalid entity");
                    comp_id_t c_id = _get_comp_type_id<T>();
                    record_t& rec = _records[entity];
                    Assert(rec.archeType->id & c_id, "Entity does not have the component");
                    return std::any_cast<T&>(rec.archeType->table[c_id].at(rec.index));
                }

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
