#pragma once

#include <memory> // unique_ptr
#include <algorithm> // std::max
#include <stdexcept>
#include <functional>
#include <optional>

#include "IEnumerator.h"
#include "DynamicArray.h"


template <class T> // функтор компаратор по умолчанию. Функтор -- объект, который можно вызывать как функцию. 
struct Less        // Т.е. это класс или структура, в которых определен operator() 
{
    bool operator()(const T& a, const T& b) const
    {
        return (a < b);
    }
};


template <class K, class V, class Comparator = Less<K>>
class AVLTree
{
public:
    struct KeyValue
    {
        K key;
        V value;
        KeyValue(const K& key_, const V& value_) : key(key_), value(value_) {}
    };
    enum class Order // К - корень, Л - левое поддерево, П - правое поддерево
    {
        LKP,
        LPK,
        KLP,
        KPL,
        PKL,
        PLK
    };
private:
    struct Node
    {
        KeyValue data;
        int height;
        std::unique_ptr<Node> left_child;
        std::unique_ptr<Node> right_child;

        Node(const K& key_, const V& value_) : data(key_, value_), height(1), left_child(), right_child() {} 
    };

    class TreeEnumerator : public IEnumerator<KeyValue> 
    {
    private:
        DynamicArray<const Node*> stack_;
        int count_; // число элементов в стеке
        const Node* current_;
        Order order_;

        void push(const Node* node)
        {
            stack_.Set(count_, node);
            count_++;
        }

        const Node* pop()
        {
            count_--;
            return stack_.Get(count_);
        }

        const Node* top() const
        {
            return stack_.Get(count_ - 1);
        }

        bool is_mirrored() const // П раньше Л
        {
            return order_ == Order::KPL || order_ == Order::PKL || order_ == Order::PLK;
        }

        const Node* first(const Node* node) const // поддерево, которое обходится первым
        {
            if (is_mirrored()) return node->right_child.get();
            return node->left_child.get();
        }

        const Node* second(const Node* node) const // поддерево, которое обходится вторым
        {
            if (is_mirrored()) return node->left_child.get();
            return node->right_child.get();
        }

        void push_first(const Node* node) // для ЛКП/ПКЛ: спуск по первым детям
        {
            while (node != nullptr)
            {
                push(node);
                node = first(node);
            }
        }

        void push_to_leaf(const Node* node) // для ЛПК/ПЛК: спуск до листа, первый ребенок в приоритете
        {
            while (node != nullptr)
            {
                push(node);
                if (first(node) != nullptr)
                {
                    node = first(node);
                }
                else
                {
                    node = second(node);
                }
            }
        }

        void move_next_root_first() // КЛП/КПЛ
        {
            current_ = pop();
            if (second(current_) != nullptr) push(second(current_)); // второй кладем раньше, чтобы достать позже
            if (first(current_) != nullptr) push(first(current_));
        }

        void move_next_root_middle() // ЛКП/ПКЛ
        {
            current_ = pop();
            push_first(second(current_));
        }

        void move_next_root_last() // ЛПК/ПЛК
        {
            current_ = pop();
            if (count_ > 0)
            {
                const Node* parent = top();
                if (first(parent) == current_) // вернулись из первого поддерева, теперь обходим второе
                {
                    push_to_leaf(second(parent));
                }
            }
        }

    public:
        TreeEnumerator(const Node* root, int height, Order order) : stack_(height + 1, nullptr), count_(0), current_(nullptr), order_(order)
        {
            if (order_ == Order::KLP || order_ == Order::KPL)
            {
                if (root != nullptr) push(root);
            }
            else if (order_ == Order::LKP || order_ == Order::PKL)
            {
                push_first(root);
            }
            else
            {
                push_to_leaf(root);
            }
        }

        bool MoveNext() override
        {
            if (count_ == 0)
            {
                current_ = nullptr;
                return false;
            }

            if (order_ == Order::KLP || order_ == Order::KPL)
            {
                move_next_root_first();
            }
            else if (order_ == Order::LKP || order_ == Order::PKL)
            {
                move_next_root_middle();
            }
            else
            {
                move_next_root_last();
            }
            return true;
        }

        const KeyValue& Current() const override
        {
            if (current_ == nullptr)
            {
                throw std::logic_error("Enumerator is not on an element");
            }
            return current_->data;
        }
    };

    std::unique_ptr<Node> root_;
    unsigned long size_;
    Comparator comp_;

    int height(const Node* node) const
    {
        if (node == nullptr) return 0;
        return node->height;
    }

    void update_height(Node* node) 
    {
        node->height = 1 + std::max(height(node->left_child.get()), height(node->right_child.get()));
    }

    int balance_factor(const Node* node) const
    {
        return height(node->left_child.get()) - height(node->right_child.get());
    }

    std::unique_ptr<Node> cut_min(std::unique_ptr<Node>& slot)
    {
        if (slot->left_child.get() == nullptr) // нашли минимум
        {
            std::unique_ptr<Node> min = std::move(slot); // вырезаем минимум
            slot = std::move(min->right_child); // на место минимума ставим его правого ребенка
            return min;
        }
        std::unique_ptr<Node> min = cut_min(slot->left_child);
        update_height(slot.get());
        balance(slot);
        return min;

    }

    bool insert_rec(const K& key, const V& value, std::unique_ptr<Node>& slot)
    {
        bool ok = false;

        if (slot == nullptr)
        {
            slot = std::make_unique<Node>(key, value);
            size_++;
            return true;
        }
        if (comp_(slot->data.key, key))
        {
            ok = insert_rec(key, value, slot->right_child);
        }
        else if (comp_(key, slot->data.key))
        {
            ok = insert_rec(key, value, slot->left_child);
        }
        else
        {
            return false;
        }

        if (ok) // если был найден дубликат и дерево не менялось, то балансировать не нужно (нечего)
        {
            update_height(slot.get());
            balance(slot);
        }
        
        return ok;
    }

    void remove_rec(const K& key, std::unique_ptr<Node>& slot)
    {
        if (slot.get() == nullptr) // элемент не найден
        {
            throw std::logic_error("Removable value isn't in tree");
        }
        if (comp_(slot->data.key, key))
        {
            remove_rec(key, slot->right_child);
            update_height(slot.get());
            balance(slot);
        }
        else if (comp_(key, slot->data.key))
        {
            remove_rec(key, slot->left_child);
            update_height(slot.get());
            balance(slot);
        }
        else // нашли совпадение
        {
            if (slot->right_child.get() != nullptr) // есть правый ребенок, ищем самого маленького из них
            {
                std::unique_ptr<Node> min = cut_min(slot->right_child);
                min->left_child = std::move(slot->left_child);
                min->right_child = std::move(slot->right_child);
                slot = std::move(min);
            }
            else // нет правого ребенка
            {
                slot = std::move(slot->left_child);
            }
            size_--;

            if (slot.get() != nullptr)
            {
                update_height(slot.get());
                balance(slot);
            }
        }
    }

    void balance(std::unique_ptr<Node>& slot)
    {
        int bf = balance_factor(slot.get());
        if (bf <= -2) // правый ребенок тяжелый
        {
            int bf_child = balance_factor(slot->right_child.get());
            if (bf_child > 0) // у ребенка лево тяжелее
            {
                rotate_rl(slot);
            }
            else 
            {
                rotate_l(slot);
            }
        }
        else if (bf >= 2) // левый ребенок тяжелый
        {
            int bf_child = balance_factor(slot->left_child.get());
            if (bf_child < 0) // у ребенка право тяжлее
            {
                rotate_lr(slot);
            }
            else
            {
                rotate_r(slot);
            }
        }
    }

    void rotate_r(std::unique_ptr<Node>& slot)
    {
        std::unique_ptr<Node> tmp = std::move(slot->left_child);
        slot->left_child = std::move(tmp->right_child);
        update_height(slot.get());
        tmp->right_child = std::move(slot);
        update_height(tmp.get());
        slot = std::move(tmp);
    }

    void rotate_l(std::unique_ptr<Node>& slot)
    {
        std::unique_ptr<Node> tmp = std::move(slot->right_child);
        slot->right_child = std::move(tmp->left_child);
        update_height(slot.get());
        tmp->left_child = std::move(slot);
        update_height(tmp.get());
        slot = std::move(tmp);
    }

    void rotate_rl(std::unique_ptr<Node>& slot)
    {
        rotate_r(slot->right_child);
        rotate_l(slot);
    }

    void rotate_lr(std::unique_ptr<Node>& slot)
    {
        rotate_l(slot->left_child);
        rotate_r(slot);
    }

    std::unique_ptr<Node> copy_rec(const Node* node) const // копирует поддерево по ноде
    {
        if (node == nullptr) return std::unique_ptr<Node>();

        std::unique_ptr<Node> result = std::make_unique<Node>(node->data.key, node->data.value);
        result->height = node->height;
        result->left_child = copy_rec(node->left_child.get());
        result->right_child = copy_rec(node->right_child.get());

        return result;
    }

    unsigned long size_rec(const Node* node) const
    {
        if (node == nullptr) return 0;
        return 1 + size_rec(node->left_child.get()) + size_rec(node->right_child.get());
    }

public:
    explicit AVLTree(const Comparator& comp = Comparator()) : root_(), size_(), comp_(comp) {} // 

    AVLTree(const AVLTree& other) : size_(other.size_), comp_(other.comp_)
    {
        root_ = copy_rec(other.root_.get());
    }

    AVLTree(AVLTree&& other) noexcept : root_(std::move(other.root_)), size_(other.size_), comp_(std::move(other.comp_))
    {
        other.size_ = 0;    
    }

    AVLTree& operator=(const AVLTree& other)
    {
        root_ = copy_rec(other.root_.get());
        size_ = other.size_;
        comp_ = other.comp_;
        return *this;
    }

    AVLTree& operator=(AVLTree&& other) noexcept
    {
        if (this == &other) return *this;

        root_ = std::move(other.root_);
        size_ = other.size_;
        other.size_ = 0;
        comp_ = std::move(other.comp_);

        return *this;
    }

    unsigned long size() const
    {
        return size_;
    }

    bool is_empty() const
    {
        return size_ == 0;
    }
    
    std::optional<V> find(const K& key) const
    {
        const Node* current = root_.get();
        while (current != nullptr)
        {
            if (comp_(current->data.key, key))
            {
                current = current->right_child.get();
            }
            else if (comp_(key, current->data.key))
            {
                current = current->left_child.get();
            }
            else
            {
                return current->data.value;
            }
        }
        return std::nullopt;
    }

    bool insert(const K& key, const V& value)
    {
        return insert_rec(key, value, root_);
    }

    void remove(const K& key)
    {
        remove_rec(key, root_);
    }

    template <class U>
    AVLTree<K, U, Comparator> map(const std::function<U(const V&)>& f) const
    {
        AVLTree<K, U, Comparator> result(comp_);
        std::unique_ptr<IEnumerator<KeyValue>> e = GetEnumerator();
        while (e->MoveNext())
        {
            result.insert(e->Current().key, f(e->Current().value));
        }
        return result;
    }

    AVLTree where(const std::function<bool(const V&)>& pred) const
    {
        AVLTree result(comp_);
        std::unique_ptr<IEnumerator<KeyValue>> e = GetEnumerator();
        while (e->MoveNext())
        {
            if (pred(e->Current().value))
            {
                result.insert(e->Current().key, e->Current().value);
            }
        }
        return result;
    }

    V reduce(const std::function<V(const V&, const V&)>& f, const V& start, Order order = Order::LKP) const
    {
        V acc = start;
        std::unique_ptr<IEnumerator<KeyValue>> e = GetEnumerator(order);
        while (e->MoveNext())
        {
            acc = f(e->Current().value, acc);
        }
        return acc;
    }

    std::unique_ptr<IEnumerator<KeyValue>> GetEnumerator(Order order = Order::LKP) const
    {
        return std::make_unique<TreeEnumerator>(root_.get(), height(root_.get()), order);
    }

    std::optional<AVLTree> get_subtree(const K& key) const 
    {
        const Node* current = root_.get();
        while(current != nullptr)
        {
            if (comp_(key, current->data.key))
            {
                current = current->left_child.get();
            }
            else if (comp_(current->data.key, key))
            {
                current = current->right_child.get();
            }
            else break;
        }

        if (current == nullptr) 
        {
            return std::nullopt;
        }

        AVLTree result(comp_);
        result.root_ = copy_rec(current);
        result.size_ = size_rec(current);
        
        return result;
    }

};




/*
1. Сделать итератор (добавить выбор порядка обхода)
2. map-where-reduce должны работать через итератор (через интерфейс или концепт)
3. из 2 лабы мб добавить интерфейс idictionary
4. добавить key 
*/
