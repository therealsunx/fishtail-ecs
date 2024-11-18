#pragma once


#include "utils.h"


#include <iostream>
#include <inttypes.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <any>
#include <bitset>


namespace trecs {

    using entity_t = uint32_t;
    using comp_id_t = uint64_t;
    using archetype_id_t = uint64_t;

    using comprow_t = std::vector<std::any>;
    using comptable_t = std::unordered_map<comp_id_t, comprow_t>;

    struct archetype_t;
    using archetype_edge_t = std::unordered_map<comp_id_t, archetype_t*>;

    using entry_data_t = std::unordered_map<comp_id_t, std::any>;
    struct entry_t {
        entry_data_t entry;
        entity_t updatedEntity = 0;
    };


    static uint32_t __comp_type_ctr__ = 0;
    template<typename t>
    inline comp_id_t _get_comp_type_id(){
        static comp_id_t id = 1ull << __comp_type_ctr__++;
        return id;
    }
// this macro is usable only inside templated methods, to make things easier... 
#define __ctype__ _get_comp_type_id<T>()


    struct archetype_t {
        private:
        std::vector<entity_t> _entities;
        
        public:
        archetype_id_t id = 0;
        comptable_t table;

        archetype_edge_t plus;
        archetype_edge_t minus;

        inline comprow_t& operator[](comp_id_t id){
            Assert(table.find(id) != table.end(), "archetype doesnot have component");
            return table[id];
        }

        inline size_t size(){
            return _entities.size();
        }

        inline entity_t entityAt(size_t index){
             return _entities.at(index);
        }

        template<typename... T>
        inline std::tuple<T...> get(size_t index){
            return std::make_tuple(std::any_cast<T>(table[__ctype__].at(index))...);
        }

        inline comptable_t::iterator begin(){
            return table.begin();
        }
        inline comptable_t::iterator end(){
            return table.end();
        }

        inline comptable_t::const_iterator cbegin() const {
            return table.cbegin();
        }
        inline comptable_t::const_iterator cend() const {
            return table.cend();
        }

        inline bool has_plus(comp_id_t comp) const {
            return plus.find(comp) != plus.end();
        }
        inline bool has_minus(comp_id_t comp) const {
            return minus.find(comp) != minus.end();
        }

        inline archetype_t* get_plus(comp_id_t comp) const {
            Assert(has_plus(comp), "Tried to get plus-neighbour archetype for invalid component");
            return plus.find(comp)->second;
        }
        inline archetype_t* get_minus(comp_id_t comp) const {
            Assert(has_minus(comp), "Tried to get minus-neighbour archetype for invalid component");
            return minus.find(comp)->second;
        }

        inline archetype_t* add_plus(comp_id_t comp, archetype_t* archetype){
            Assert(archetype, "Cannot pass nullptr while adding neighbouring archetype");
            plus.try_emplace(comp, archetype);
            archetype->minus.try_emplace(comp, this);
            return archetype;
        }
        inline archetype_t* add_minus(comp_id_t comp, archetype_t* archetype){
            Assert(archetype, "Cannot pass nullptr while adding neighbouring archetype");
            minus.try_emplace(comp, archetype);
            archetype->plus.try_emplace(comp, this);
            return archetype;
        }

        inline entry_t remove_entry(size_t index){
            entry_t entr;
            if(!_entities.size()) return entr;
            for(auto& [c_id, vec]:table){
                entr.entry[c_id] = std::move(vec[index]);
                if(index != vec.size()-1){
                    vec[index] = std::move(vec.back());
                    if(!entr.updatedEntity){
                        _entities[index] = _entities.back();
                        entr.updatedEntity = _entities[index];
                    }
                }
                vec.pop_back(); 
            }
            _entities.pop_back();
            return entr;
        }

        inline size_t add_entry(entry_t& entry){
            if(entry.entry.begin() == entry.entry.end()) return 0; //special case
            for(auto& [c_id, comp]: entry.entry){
                table[c_id].push_back(comp);
            }
            _entities.push_back(entry.updatedEntity);
            return _entities.size()-1;
        }
    };
}
