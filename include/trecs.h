/*
 * ============== Fishtail ECS ==================
 * Author: Sunil Sapkota
 * Description: Fishtail-ECS is a fast and reliable Entity Component System,
 * written for its usage in the game engine - Everest.
 *
 * As of now it supports 64 different component types, which should be enough for
 * any game engine.
 */

#pragma once

#include <iostream>
#include <inttypes.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <any>

#include "../include/utils.h"

#define BIT(x) (1<<x)

namespace trecs {
    using entity_t = uint32_t;
    using archetypeid_t = uint64_t;

    using comptypeid_t = uint64_t;
    
    using compvector_t = std::vector<std::any>;

    using compvectormap_t = std::unordered_map<comptypeid_t, compvector_t>;

    struct archetype_t;
    using comp_arch_map_t = std::unordered_map<comptypeid_t, archetype_t&>;
    struct arch_graph_edge_t {
        comp_arch_map_t add;
        comp_arch_map_t remove;
    };

    struct archetype_t{
        compvectormap_t compvector;
        archetypeid_t id;
        arch_graph_edge_t edges;

        std::vector<std::any>& operator[](comptypeid_t id){
            return compvector[id];
        }

        compvectormap_t::iterator begin(){
            return compvector.begin();
        }
        compvectormap_t::iterator end(){
            return compvector.end();
        }
    };
    using archetyperec_t = std::unordered_map<archetypeid_t, archetype_t>;

    struct record_t {
        archetype_t archetype;
        size_t index;
    };

    using entityrecord_t = std::unordered_map<entity_t, record_t>; 

    class registry_t {
        public:
            registry_t():_entity_gen(0){}
            ~registry_t(){}

            inline entity_t create(){
                entity_t ent = ++_entity_gen;
                _records[ent] = {.archetype = {.id = 0}};
                return ent;
            }

            template<typename T>
            void addComponent(entity_t entity, T component_data){
                comptypeid_t comp_id = _get_comp_type_id<T>();

                auto it = _records.find(entity);
                Assert(it != _records.end(), "Entity not found in the registry");
                record_t& rec = it->second;

                archetype_t& p_arch = rec.archetype;
                archetype_t& n_arch = _get_neighbour(comp_id, p_arch);

                // move form p_arch to n_arch and update record index
                if(p_arch.id) {
                    for(auto it:p_arch){
                        n_arch[it.first].push_back(it.second[rec.index]);
                    }
                    // mark old slot as empty for filling up later
                    _empty_slots[p_arch.id].push_back(rec.index);
                }
                n_arch[comp_id].push_back((std::any)component_data);
                rec.index = n_arch[comp_id].size()-1;
            }

            template<typename T>
            T getComponent(entity_t entity){
                comptypeid_t comp_id = _get_comp_type_id<T>();

                auto it = _records.find(entity);
                Assert(it != _records.end(), "Entity not found in the registry");
                record_t& rec = it->second;

                return std::any_cast<T>(rec.archetype[comp_id][rec.index]);
            }

            
        private:
            entityrecord_t _records;
            entity_t _entity_gen;
            archetyperec_t _arche_records;

            using empty_slotmap_t = std::unordered_map<archetypeid_t, std::vector<size_t>>;
            empty_slotmap_t _empty_slots;

            archetype_t& _get_neighbour(comptypeid_t id, archetype_t& arch){
                auto it = arch.edges.add.find(id);
                if (it != arch.edges.add.end()) return it->second;

                // create a new archetype, add it to neighbour and archetype records and
                // and also evaluate the graph to make sure everything is connected
                archetypeid_t n_aid = arch.id | id;
                _arche_records[n_aid] = archetype_t{};
                _arche_records[n_aid].id = n_aid;

                arch.edges.add[id] = _arche_records[n_aid];
                archetype_t& a = arch.edges.add[id];
                a.edges.remove[id] = arch;
                return a;
            }
        private:
            static uint32_t _comp_t_counter;
            template<typename t>
            static comptypeid_t _get_comp_type_id(){
                static comptypeid_t id = 1ull << _comp_t_counter++;
                return id;
            }
    };

}

