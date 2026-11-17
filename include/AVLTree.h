#pragma once

#include <memory> // unique_ptr
#include <algorithm> // std::max
#include <stdexcept>
#include <functional>
#include <optional>
#include <concepts>
#include <utility>
#include "StandartTraversal.h"


template <class K, class V>
requires requires(const K& a, const K& b)
{
    {a < b} -> std::convertible_to<bool>;
}
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

    std::unique_ptr<Node> root_;
    unsigned long size_;

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
        if (slot->data.key < key)
        {
            ok = insert_rec(key, value, slot->right_child);
        }
        else if (key < slot->data.key)
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
        if (slot->data.key < key)
        {
            remove_rec(key, slot->right_child);
            update_height(slot.get());
            balance(slot);
        }
        else if (key < slot->data.key)
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

    void for_range_rec(const Node* node, const K& low, const K& high, const std::function<void(const K& key, const V& value)>& visit) const
    {
        if (node == nullptr) return;

        if (low < node->data.key) // слева еще могут значения из диапазона
        {
            for_range_rec(node->left_child.get(), low, high, visit);
        }
        if (!(node->data.key < low) && !(high < node->data.key)) // сама нода в диапазоне
        {
            visit(node->data.key, node->data.value);
        }
        if (node->data.key < high) // справа еще могут быть значения из диапазона
        {
            for_range_rec(node->right_child.get(), low, high, visit);
        }
    }

public:
    class NodeView
    {
    private:
        const Node* node_;

        friend class AVLTree;

        explicit NodeView(const Node* node) : node_(node) {}

    public:
        NodeView() : node_(nullptr) {}

        const KeyValue& data() const
        {
            if (node_ == nullptr) throw std::logic_error("Node is empty");
            return node_->data;
        }

        NodeView left() const
        {
            if (node_ == nullptr) return NodeView();
            return NodeView(node_->left_child.get());
        }

        NodeView right() const
        {
            if (node_ == nullptr) return NodeView();
            return NodeView(node_->right_child.get());
        }

        explicit operator bool() const
        {
            return node_ != nullptr;
        }

        bool operator==(const NodeView& other) const
        {
            return node_ == other.node_;
        }
    };

    class ITraversal
    {
    public:
        virtual ~ITraversal() = default;
        virtual void start(NodeView root) = 0;
        virtual NodeView next() = 0;
        virtual std::unique_ptr<ITraversal> clone() const = 0;
    };

    using StandartTraversal = ::StandartTraversal<AVLTree>;

    class Iterator
    {
    private:
        std::unique_ptr<ITraversal> traversal_;
        NodeView current_;
        const AVLTree* owner_;

        friend class AVLTree;

        Iterator(const AVLTree* owner, NodeView root, const ITraversal& traversal) : traversal_(traversal.clone()), current_(), owner_(owner)
        {
            traversal_->start(root);
            current_ = traversal_->next();
        }

        Iterator(const AVLTree* owner) : traversal_(), current_(), owner_(owner) {}

    public:
        Iterator() : traversal_(), current_(), owner_(nullptr) {}

        Iterator(const Iterator& other) : traversal_(), current_(other.current_), owner_(other.owner_)
        {
            if (other.traversal_) traversal_ = other.traversal_->clone();
        }

        Iterator& operator=(const Iterator& other)
        {
            if (this == &other) return *this;

            std::unique_ptr<ITraversal> copy;
            if (other.traversal_) copy = other.traversal_->clone();
            traversal_ = std::move(copy);
            current_ = other.current_;
            owner_ = other.owner_;
            return *this;
        }

        Iterator(Iterator&& other) noexcept = default;
        Iterator& operator=(Iterator&& other) noexcept = default;

        Iterator& operator++()
        {
            if (traversal_) current_ = traversal_->next();
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator old = *this;
            ++(*this);
            return old;
        }

        const KeyValue& operator*() const
        {
            if (!current_)
            {
                throw std::logic_error("Iterator is not on an element");
            }
            return current_.data();
        }

        const KeyValue* operator->() const
        {
            return &operator*();
        }

        bool operator==(const Iterator& other) const
        {
            if (owner_ != other.owner_) return false;
            return current_ == other.current_;
        }

        bool operator!=(const Iterator& other) const
        {
            return !(*this == other);
        }
    };

    AVLTree() : root_(), size_() {}

    AVLTree(const AVLTree& other) : size_(other.size_)
    {
        root_ = copy_rec(other.root_.get());
    }

    AVLTree(AVLTree&& other) noexcept : root_(std::move(other.root_)), size_(other.size_)
    {
        other.size_ = 0;    
    }

    AVLTree& operator=(const AVLTree& other)
    {
        root_ = copy_rec(other.root_.get());
        size_ = other.size_;
        return *this;
    }

    AVLTree& operator=(AVLTree&& other) noexcept
    {
        if (this == &other) return *this;

        root_ = std::move(other.root_);
        size_ = other.size_;
        other.size_ = 0;

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
            if (current->data.key < key)
            {
                current = current->right_child.get();
            }
            else if (key < current->data.key)
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

    const V* find_ptr(const K& key) const
    {
        const Node* current = root_.get();
        while (current != nullptr)
        {
            if (current->data.key < key)
            {
                current = current->right_child.get();
            }
            else if (key < current->data.key)
            {
                current = current->left_child.get();
            }
            else
            {
                return &(current->data.value);
            }
        }
        return nullptr;
    }

    bool insert(const K& key, const V& value)
    {
        return insert_rec(key, value, root_);
    }

    void for_range(const K& low, const K& high, const std::function<void(const K& key, const V& value)>& visit) const
    {
        if (high < low)
        {
            throw std::invalid_argument("high is lower low");
        }  
        for_range_rec(root_.get(), low, high, visit);
    }

    void remove(const K& key)
    {
        remove_rec(key, root_);
    }

    template <class U>
    AVLTree<K, U> map(const std::function<U(const V&)>& f) const
    {
        AVLTree<K, U> result;
        for (Iterator it = begin(); it != end(); ++it)
        {
            result.insert(it->key, f(it->value));
        }
        return result;
    }

    AVLTree where(const std::function<bool(const V&)>& pred) const
    {
        AVLTree result;
        for (Iterator it = begin(); it != end(); ++it)
        {
            if (pred(it->value))
            {
                result.insert(it->key, it->value);
            }
        }
        return result;
    }

    V reduce(const std::function<V(const V&, const V&)>& f, const V& start, Order order = Order::LKP) const
    {
        V acc = start;
        StandartTraversal traversal(order);
        for (Iterator it = begin(traversal); it != end(); ++it)
        {
            acc = f(it->value, acc);
        }
        return acc;
    }

    
    Iterator begin(const ITraversal& traversal = StandartTraversal{}) const
    {
        return Iterator(this, NodeView(root_.get()), traversal);
    }

    Iterator end() const
    {
        return Iterator(this);
    }

    std::optional<AVLTree> get_subtree(const K& key) const 
    {
        const Node* current = root_.get();
        while(current != nullptr)
        {
            if (key < current->data.key)
            {
                current = current->left_child.get();
            }
            else if (current->data.key < key)
            {
                current = current->right_child.get();
            }
            else break;
        }

        if (current == nullptr) 
        {
            return std::nullopt;
        }

        AVLTree result;
        result.root_ = copy_rec(current);
        result.size_ = size_rec(current);
        
        return result;
    }
};


