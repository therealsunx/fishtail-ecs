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

    using comp_id_t = uint64_t;
    using archetype_id_t = uint64_t;

    using comprow_t = std::vector<std::any>;
    using comptable_t = std::unordered_map<comp_id_t, comprow_t>;

    struct archetype_t;
    using archetype_edge_t = std::unordered_map<comp_id_t, archetype_t*>;
    using emptyslots_t = std::unordered_set<size_t>;
    using entry_t = std::unordered_map<comp_id_t, std::any>;

    struct archetype_t {
        archetype_id_t id = 0;
        comptable_t table; 

        archetype_edge_t plus;
        archetype_edge_t minus;

        private:
        emptyslots_t _emptySlots;

        public:
        inline comprow_t& operator[](comp_id_t id){
            Assert(table.find(id) != table.end(), "archetype doesnot have component");
            return table[id];
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
            Assert(_emptySlots.count(index) == 0, "Attempt to access empty slot");
            _emptySlots.insert(index);
            bool clearmark = table.begin() == table.end();
            entry_t ent;
            for(auto& [c_id, vec]:table){
                Assert(index < vec.size(), "Invalid index to access from");
                ent[c_id] = std::move(vec[index]);
                vec[index] = {};
                if(vec.size() == _emptySlots.size()){
                    vec.clear();
                    clearmark = true;
                }
            }
            if(clearmark) _emptySlots.clear();
            return ent;
        }

        inline size_t add_entry(entry_t& entry){
            if(entry.begin() == entry.end()) return 0; //special case

            bool _has_empty = _emptySlots.begin() != nullptr;
            int index = -1;
            if(_has_empty){
                index = *_emptySlots.begin();
                _emptySlots.erase(index);
            }
            for(auto& [c_id, comp]: entry){
                if(_has_empty){
                    table[c_id][index] = comp;
                }else{
                    table[c_id].push_back(comp);
                }
            }
            if(!_has_empty) index = table.begin()->second.size()-1;
            return index;
        }
    };
}
