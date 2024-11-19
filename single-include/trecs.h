/*
 * ============== TR-ECS ==================
 * Author: Sunil Sapkota
 * Description: This is a single include header file for the trecs. Just copy this file into your
 * project and use it.
 *
 * TR-ECS is a fast and reliable Entity Component System,
 * written for its usage in the game engine - Everest. It has efficient implementation
 * for adding, removing, quering or getting the components from entities. It uses
 * archetype graph method for organizing data neatly, so that there is no memory wastage
 * or leak. Entities recycling is also supported by the way. As of now it supports 64
 * different component types, which should be enough for any game engine.
 */

#pragma once


#include <iostream>
#include <inttypes.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <any>
#include <bitset>

#include <functional>

#define TR_ASSERT

#ifdef TR_ASSERT
#   include <assert.h>
#   define Assert(exp, msg) if(!(exp)){printf("\x1b[31m%s\x1b[m\n", msg); assert(0);}
#else
#   define Assert(exp, msg)
#endif

#define __entity_id__(x) (x & 0x00ffffff)
#define __entity_rc__(x) (x & 0xff000000)

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


    struct record_t {
        archetype_t* archeType;
        size_t index = 0;

    };

    using view_id_t = archetype_id_t;
    using archetype_map_t = std::unordered_map<archetype_id_t, archetype_t>;
    using entity_records_t = std::vector<record_t>;
    using recycleReg_t = std::vector<entity_t>;

    template<typename... T>
    struct view_t {
        view_id_t id = 0;

        inline void forEach(const std::function<void(T&...)>& callback){
            size_t i = 0;
            archetype_map_t::iterator _cur_arch = _archmap.begin();
            archetype_map_t::iterator _end_arch = _archmap.end();
            while(1){
                while(i >= _cur_arch->second.size() || (_cur_arch->first & id) != id){
                    if(++_cur_arch == _end_arch) return;
                    i = 0;
                }
                callback(std::any_cast<T&>(_cur_arch->second[__ctype__].at(i)) ...);
                i++;
            }
        }

        inline void forEach(const std::function<void(T&..., entity_t)>& callback){
            size_t i = 0;
            archetype_map_t::iterator _cur_arch = _archmap.begin();
            archetype_map_t::iterator _end_arch = _archmap.end();
            while(1){
                while(i >= _cur_arch->second.size() || (_cur_arch->first & id) != id){
                    if(++_cur_arch == _end_arch) return;
                    i = 0;
                }
                callback(std::any_cast<T&>(_cur_arch->second[__ctype__].at(i)) ...,
                        _cur_arch->second.entityAt(i));
                i++;
            }
        }

        private:
        archetype_map_t& _archmap;
        friend class registry_t;

        view_t(view_id_t id_, archetype_map_t& arch_):id(id_), _archmap(arch_){}
    };

    class registry_t {
        public:
            registry_t(){
                _archetypeStore[0] = {/*root*/};
                _records.push_back({}); // leave first slot empty, entity 0 never exists
            }

            /*Entity Ops*/

            /*creates an entity*/
            inline entity_t create(){
                while(!_recycleReg.empty()){
                    int en = _recycleReg.back();
                    _recycleReg.pop_back();
                    int rc = __entity_rc__(en);
                    if(rc < 0xff){
                        return ((rc+1)<<24) | __entity_id__(en);
                    }
                }

                _records.push_back(record_t{&_archetypeStore[0], 0});
                return ++__entity_generator;
            }

            inline void destroy(entity_t entity){
                Assert(__entity_id__(entity) <= _records.size(), "Invalid entity");
                const entity_t ind = __entity_id__(entity);
                record_t& rec = _records[ind];
                auto en = rec.archeType->remove_entry(rec.index);
                if(en.updatedEntity) _records[__entity_id__(en.updatedEntity)].index = rec.index;
                rec.archeType = &_archetypeStore[0];
                _recycleReg.push_back(entity);
            }

            /*Component Ops*/
            /* returns the component-type's id*/

            /*tries to add component if not already added*/
            template<typename T>
            inline bool tryAdd(const entity_t entity, T data){
                if(has<T>(entity)) return false;
                add<T>(entity, data);
                return true;
            }

            template<typename T>
            inline void addOrUpdate(const entity_t entity, T data){
                if(has<T>(entity)) update<T>(entity, data);
                else add<T>(entity, data);
            }

            template<typename T>
            inline void update(const entity_t entity, T data){
                const entity_t ind = __entity_id__(entity);
                Assert(ind < _records.size(), "Invalid entity");
                record_t& rec = _records[ind];
                Assert(rec.archeType->id & __ctype__, "Entity does not have the component to update");
                (*rec.archeType)[__ctype__][rec.index] = data;
            }

            template<typename T>
            void add(const entity_t entity, T data){
                const entity_t ind = __entity_id__(entity);
                Assert(ind < _records.size(), "Invalid entity");
                comp_id_t c_id = __ctype__;
                record_t& rec = _records[ind];
                archetype_t *p_arch = rec.archeType;

                Assert(!(p_arch->id & c_id), "Component already exists on the entity");
                
                archetype_t* n_arch = p_arch->has_plus(c_id)
                    ? p_arch->get_plus(c_id)
                    : p_arch->add_plus(c_id, _getNewArchetype(c_id | p_arch->id));

                entry_t en = p_arch->remove_entry(rec.index);
                en.entry[c_id] = data;
                if(en.updatedEntity) _records[__entity_id__(en.updatedEntity)].index = rec.index;
                en.updatedEntity = entity;
                rec.index = n_arch->add_entry(en);
                rec.archeType = n_arch;
            }

            template<typename... T>
            inline void tryRemove(entity_t entity){
                (_tryRemove<T>(entity), ...);
            }
            
            template<typename... T>
            inline void remove(entity_t entity){
                (_remove<T>(entity), ...);
            }
            
            template<typename... T>
            inline bool has(const entity_t entity){
                Assert(__entity_id__(entity) < _records.size(), "Invalid entity");
                return (_has_findex<T>(__entity_id__(entity)) && ...);
            }

            template<typename T>
            inline bool tryGet(const entity_t entity, T& output){
                if(!has<T>(entity)) return false;
                output = get<T>(entity);
                return true;
            }

            template<typename... T>
            inline std::tuple<T...> gett(const entity_t entity){
                const entity_t ind = __entity_id__(entity);
                Assert(ind <= _records.size(), "Invalid entity");
                return _records[ind].archeType->get<T...>(_records[ind].index);
            }

            template<typename T>
            inline T& get(const entity_t entity){
                const entity_t ind = __entity_id__(entity);
                Assert(ind <= _records.size(), "Invalid entity");
                Assert(_records[ind].archeType->id & __ctype__, "Entity does not have the component");
                return std::any_cast<T&>((*_records[ind].archeType)[__ctype__]
                        .at(_records[ind].index));
            }

            /*View Ops*/
            /*Returns the view to components*/
            template<typename... T>
            inline view_t<T...> view(){
                view_t<T...> view((_get_comp_type_id<T>() | ...), _archetypeStore);
                return view;
            }

        private:
            template<typename T>
            inline bool _has_findex(const size_t ind){
                return __ctype__ & _records[ind].archeType->id;
            }

            template<typename T>
            inline void _remove(entity_t entity){
                const entity_t ind = __entity_id__(entity);
                Assert(ind < _records.size(), "Invalid entity");
                comp_id_t c_id = __ctype__;

                record_t& rec = _records[ind];
                archetype_t* p_arch = rec.archeType;
                Assert(p_arch->id & c_id, "Attempt to remove non-existent component");

                archetype_t* n_arch = p_arch->has_minus(c_id)
                    ? p_arch->get_minus(c_id)
                    : p_arch->add_minus(c_id, _getNewArchetype(p_arch->id & (~c_id)));

                entry_t en = p_arch->remove_entry(rec.index);
                en.entry.erase(c_id);
                if(en.updatedEntity) _records[__entity_id__(en.updatedEntity)].index = rec.index;
                en.updatedEntity = entity;
                rec.index = n_arch->add_entry(en);
                rec.archeType = n_arch;
            }

            template<typename T>
            inline void _tryRemove(const entity_t entity){
                if(has<T>(entity)) _remove<T>(entity);
            }

        private:
            entity_records_t _records;
            archetype_map_t _archetypeStore;
            recycleReg_t _recycleReg;
            entity_t __entity_generator = 0;

            inline archetype_t* _getNewArchetype(archetype_id_t id){
                if(_archetypeStore.find(id) != _archetypeStore.end()) return &_archetypeStore[id];
                auto& a = _archetypeStore.try_emplace(id, archetype_t{}).first->second;
                a.id = id;
                return &a;
            }
    };
}

