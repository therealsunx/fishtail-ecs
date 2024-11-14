/*
 * ============== Fishtail ECS ==================
 * Author: Sunil Sapkota
 * Description: Fishtail-ECS is a fast and reliable Entity Component System,
 * written for its usage in the game engine - Everest.
 */

#pragma once

#include <assert.h>
#include <iostream>
#include <inttypes.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <any>

#define Assert(exp, msg) if(!(exp)){printf("\x1b[31m%s\x1b[m\n", msg); assert(0);}

namespace trecs {
    using entity_t = uint32_t;
    using comp_id_t = uint64_t;
    using archetype_id_t = uint64_t;

    using comprow_t = std::vector<std::any>;
    using comptable_t = std::unordered_map<comp_id_t, comprow_t>;

    struct archetype_t;
    using archetype_edge_t = std::unordered_map<comp_id_t, archetype_t&>;

    struct archetype_t {
        archetype_id_t id = 0;
        comptable_t table; 

        archetype_edge_t plus;
        archetype_edge_t minus;

        inline comptable_t::iterator begin(){
            return table.begin();
        }
        inline comptable_t::iterator end(){
            return table.end();
        }

        inline comptable_t::const_iterator cbegin(){
            return table.cbegin();
        }
        inline comptable_t::const_iterator cend(){
            return table.cend();
        }

        inline bool has_plus(comp_id_t comp){
            return plus.find(comp) != plus.end();
        }
        inline bool has_minus(comp_id_t comp){
            return minus.find(comp) != minus.end();
        }

        inline archetype_t& get_plus(comp_id_t comp){
            return plus.find(comp)->second;
        }
        inline archetype_t& get_minus(comp_id_t comp){
            return minus.find(comp)->second;
        }

        inline archetype_t& add_plus(comp_id_t comp, archetype_t& archetype){
            archetype.minus.try_emplace(comp, *this);
            plus.try_emplace(comp, archetype);
            return archetype;
        }
        inline archetype_t& add_minus(comp_id_t comp, archetype_t& archetype){
            archetype.plus.try_emplace(comp, *this);
            minus.try_emplace(comp, archetype);
            return archetype;
        }
    };

    struct record_t {
        archetype_t& archeType;
        size_t index = 0;
    };

    using archetype_map_t = std::unordered_map<archetype_id_t, archetype_t>;
    using entity_records_t = std::vector<record_t>;

    class registry_t {
        public:
            registry_t(){
                _archetypeStore[0] = {/*root*/};
            }

            entity_t create(){
                _records.push_back(record_t{_archetypeStore[0], 0});
                return __entity_generator++;
            }

            template<typename T>
            void addComponent(entity_t entity, T data){
                comp_id_t c_id = _get_comp_type_id<T>();

                record_t& rec = _records[entity];
                archetype_id_t id = c_id | rec.archeType.id;

                Assert(!(rec.archeType.id & c_id), "component already exists on the entity. TODO: override");
                
                archetype_t& n_arch = rec.archeType.has_plus(c_id)
                    ? rec.archeType.get_plus(c_id)
                    : rec.archeType.add_plus(c_id, _getNewArchetype(id));
                for(auto it:rec.archeType){
                    n_arch.table[it.first].push_back(it.second[rec.index]); 
                }
                //TODO: invalidate the rec.index of rec.archetype to make it usable
                //by other entities
                n_arch.table[c_id].push_back(data);
                rec.index = n_arch.table[c_id].size()-1;
                rec.archeType = n_arch;
            }

            template<typename T>
            inline bool hasComponent(entity_t entity){
                return _get_comp_type_id<T>() & _records[entity].archeType.id;
            }

            template<typename T>
            inline T getComponent(entity_t entity){
                record_t& rec = _records[entity];
                return std::any_cast<T>(rec.archeType.table[_get_comp_type_id<T>()]
                        .at(rec.index));
            }

            template<typename T>
            inline comp_id_t getComponentID(){
                return _get_comp_type_id<T>();
            }

        private:
            entity_records_t _records;
            archetype_map_t _archetypeStore;
            entity_t __entity_generator = 0;

            inline archetype_t& _getNewArchetype(archetype_id_t id){
                if(_archetypeStore.find(id) != _archetypeStore.end()) return _archetypeStore[id];
                auto& a = _archetypeStore.try_emplace(id, archetype_t{}).first->second;
                a.id = id;
                return a;
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
