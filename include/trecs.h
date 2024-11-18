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
#include <functional>

#define __entity_id__(x) (x & 0x00ffffff)
#define __entity_rc__(x) (x & 0xff000000)


namespace trecs {

    using entity_t = uint32_t;

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
