#include "../libs/tree.hpp"

template <typename T, typename Priority, typename Cmp>
struct priority_search_tree
{
    using priority_t = Priority;
    using key_type = T;
    using value_type = std::pair<key_type,priority_t>;

    struct node
    {
        std::array<node*,2> _children;
        key_type _split;
        key_type _key;
        priority_t _priority;
        bool color;
        Cmp cmp;

        void insert(const key_type &key, const priority_t &priority)
        {
            if (cmp(priority, _priority))
            {
                auto cur_key = std::move(_key);
                auto cur_priority = std::move(_priority);
                _key = key;
                _priority = priority;
                insert(cur_key, cur_priority);
            }
            else
            {
                auto dir = key > _split;
                if(_children[dir])
                    _children[dir]->insert(key,priority);
                else {
                    _children[dir] = new node{key,priority,key};
                }
                tree::insert_fix_up(this,dir);
            }
        }
    };
    node* _root;
    void insert(const key_type &key, const priority_t &priority){
        tree::insert(_root,key);
    }
};
