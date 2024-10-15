#include <concepts>
#include <memory>
#include <array>

namespace tree
{

// support raw pointer only
template <typename T>
concept Tree = requires(T t) {
    t->children;
    t->key;
};

template <typename T>
concept RBTree = Tree<T> && requires(T t) {
    { t->color } -> std::convertible_to<bool>;
};

template <typename T>
concept BPST = RBTree<T> && requires(T t) {
    { t->priority };
    { t->split};
};

template <Tree T>
void dispose(T root); // not implement yet

template <Tree T, typename U>
auto contain(const T root, const U &key) -> bool
{
    if (!root)
        return false;
    if (key < root->key)
        return contain(root->children[0], key);
    else if (key == root->key)
        return true;
    else
        return contain(root->children[1], key);
}

template <RBTree T>
void color_flip(const T root)
{
    root->color = !root->color;
    for (auto &child : root->children)
        child->color = !child->color;
}

template <RBTree T>
auto rotate(T root, bool dir) -> T
{
    auto temp = root->children[!dir];
    root->children[!dir] = std::move(temp->children[dir]);
    temp->color = root->color;
    root->color = true;
    temp->children[dir] = std::move(root);
    return temp;
}

template <RBTree T>
auto double_rotate(T root, bool dir) -> T
{
    root->children[!dir] = rotate(root->children[!dir], !dir);
    return rotate(root, dir);
}

template <RBTree T, typename U>
void insert(T &root, const U &key)
{
    root = insert_impl(root, key);
    root->color = false;
}

template <RBTree T, typename U>
T insert_impl(T root, const U &key)
{
    if (!root)
    {
        auto cur = new std::pointer_traits<T>::element_type();
        cur->color = true;
        cur->key = key;
        return cur;
    }
    bool dir = key > root->key;
    root->children[dir] = std::move(insert_impl(root->children[dir], key));
    return insert_fix_up(root, dir);
}

template <RBTree T>
bool is_red(T root)
{
    return root && root->color;
}

template <RBTree T>
T insert_fix_up(T root, bool dir)
{
    if (is_red(root->children[dir]))
    {
        if (is_red(root->children[!dir]))
        {
            if (is_red(root->children[dir]->children[dir]) || is_red(root->children[dir]->children[!dir]))
            {
                color_flip(root);
            }
        }
        else
        {
            if (is_red(root->children[dir]->children[dir]))
            {
                root = rotate(root, !dir);
            }
            else if (is_red(root->children[dir]->children[!dir]))
            {
                root = double_rotate(root, !dir);
            }
        }
    }
    return root;
}

template <typename TKey, typename TPriority>
struct BPST_node{
    using key_type = TKey;
    using priority_type = TPriority;
    std::array<BPST_node*,2> children;
    key_type key;
    priority_type priority;
    key_type split;
    bool color;
};

template <typename TKey, typename TPriority>
void insert(BPST_node<TKey,TPriority>* root, const TKey &key, const TPriority &priority){
    root = insert_impl(root,key,priority);
    root->color = false;
}
template <typename TKey, typename TPriority>
auto insert(BPST_node<TKey,TPriority>* root, const TKey &key, const TPriority &priority)->BPST_node<TKey,TPriority>*
{
    if (!root)
    {
        auto cur = new BPST_node<TKey,TPriority>;
        cur->color = true;
        cur->key = key;
        cur->priority = priority;
        cur->split = key;
        return cur;
    }
    if (priority < root->priority)
    {
        std::swap(root->key, key);
        std::swap(root->priority, priority);
        return insert_impl(root, key, priority);
    }
    else
    {
        bool dir = key > root->split;
        root->children[dir] = std::move(insert_impl(root->children[dir], key, priority));
        return insert_fix_up(root, dir);
    }
}

template <typename TKey, typename TPriority>
auto insert_fix_up(BPST_node<TKey,TPriority>* root, bool dir)->BPST_node<TKey,TPriority>*
{
    if (is_red(root->children[dir]))
    {
        if (is_red(root->children[!dir]))
        {
            if (is_red(root->children[dir]->children[dir]) || is_red(root->children[dir]->children[!dir]))
            {
                color_flip(root);
            }
        }
        else
        {
            if (is_red(root->children[dir]->children[dir]))
            {
                root = rotate(root, !dir);
            }
            else if (is_red(root->children[dir]->children[!dir]))
            {
                root = double_rotate(root, !dir);
            }
        }
    }
    return root;
}

}; // namespace tree
