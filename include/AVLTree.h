#pragma once

#include <memory> // unique_ptr
#include <algorithm> // std::max
#include <stdexcept>
#include <functional>


template <class T> // функтор компаратор по умолчанию. Функтор -- объект, который можно вызывать как функцию. 
struct Less        // Т.е. это класс или структура, в которых определен operator() 
{
    bool operator()(const T& a, const T& b) const
    {
        return (a < b);
    }
};


template <class T, class Comparator = Less<T>>
class AVLTree
{
private:
    struct Node
    {
        T data;
        // Y key; Добавить key и компаратор сравнивает по нему
        int height;
        std::unique_ptr<Node> left_child;
        std::unique_ptr<Node> right_child;

        Node(const T& data_) : data(data_), height(1), left_child(), right_child() {} 
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

    bool insert_rec(const T& value, std::unique_ptr<Node>& slot)
    {
        bool ok = false;

        if (slot == nullptr)
        {
            slot = std::make_unique<Node>(value);
            size_++;
            return true;
        }
        if (comp_(slot->data, value))
        {
            ok = insert_rec(value, slot->right_child);
        }
        else if (comp_(value, slot->data))
        {
            ok = insert_rec(value, slot->left_child);
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

    void remove_rec(const T& value, std::unique_ptr<Node>& slot)
    {
        if (slot.get() == nullptr) // элемент не найден
        {
            throw std::logic_error("Removable value isn't in tree");
        }
        if (comp_(slot->data, value))
        {
            remove_rec(value, slot->right_child);
            update_height(slot.get());
            balance(slot);
        }
        else if (comp_(value, slot->data))
        {
            remove_rec(value, slot->left_child);
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

    void map_rec(const Node* node, const std::function<T(const T&)>& f, AVLTree& result) const
    { // U->T
        if (node == nullptr) return;

        map_rec(node->left_child.get(), f, result);
        result.insert(f(node->data));
        map_rec(node->right_child.get(), f, result);
    }
    void where_rec(const Node* node, const std::function<bool(const T&)>& pred, AVLTree& result) const
    {
        if (node == nullptr) return;

        where_rec(node->left_child.get(), pred, result);
        if (pred(node->data))
        {
            result.insert(node->data);
        }
        where_rec(node->right_child.get(), pred, result);
    }
    void reduce_rec(const Node* node, const std::function<T(const T&, const T&)>& f, T& acc) const
    {
        if (node == nullptr) return;

        reduce_rec(node->left_child.get(), f, acc);
        acc = f(node->data, acc);
        reduce_rec(node->right_child.get(), f, acc);
    }

    std::unique_ptr<Node> copy_rec(const Node* node) const // копирует поддерево по ноде
    {
        if (node == nullptr) return std::unique_ptr<Node>();

        std::unique_ptr<Node> result = std::make_unique<Node>(node->data);
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

    bool contains(const T& value) const
    {
        const Node* current = root_.get();
        while (current != nullptr)
        {
            if (comp_(current->data, value))
            {
                current = current->right_child.get();
            }
            else if (comp_(value, current->data))
            {
                current = current->left_child.get();
            }
            else
            {
                return true;
            }
        }
        return false;
    }

    bool insert(const T& value)
    {
        return insert_rec(value, root_);
    }

    void remove(const T& value)
    {
        remove_rec(value, root_);
    }

    AVLTree map(const std::function<T(const T&)>& f) const
    {
        AVLTree result(comp_);
        map_rec(root_.get(), f, result);

        return result;
    }

    AVLTree where(const std::function<bool(const T&)>& pred) const
    {
        AVLTree result(comp_);
        where_rec(root_.get(), pred, result);

        return result;
    }

    T reduce(const std::function<T(const T&, const T&)>& f, const T& start) const
    {
        T acc = start;
        reduce_rec(root_.get(), f, acc);

        return acc;
    }
    
    AVLTree get_subtree(const T& value) const // std::optional добавить
    {
        const Node* current = root_.get();
        while(current != nullptr)
        {
            if (comp_(value, current->data))
            {
                current = current->left_child.get();
            }
            else if (comp_(current->data, value))
            {
                current = current->right_child.get();
            }
            else break;
        }

        if (current == nullptr) 
        {
            throw std::logic_error("Subtree not found");
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