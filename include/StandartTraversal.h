#pragma once

#include <memory>
#include "DynamicArray.h"

template <class Tree>
class StandartTraversal : public Tree::ITraversal
{
private:
    using NodeView = typename Tree::NodeView;
    using Order = typename Tree::Order;

    DynamicArray<NodeView> stack_;
    int count_;
    Order order_;

    void push(NodeView node)
    {
        if (count_ == stack_.GetSize()) stack_.Resize(count_ + 1, NodeView());
        stack_.Set(count_, node);
        count_++;
    }

    NodeView pop()
    {
        count_--;
        return stack_.Get(count_);
    }

    NodeView top() const
    {
        return stack_.Get(count_ - 1);
    }

    bool is_mirrored() const // П раньше Л
    {
        return order_ == Order::KPL || order_ == Order::PKL || order_ == Order::PLK;
    }

    NodeView first(NodeView node) const // поддерево, которое обходится первым
    {
        if (is_mirrored()) return node.right();
        return node.left();
    }

    NodeView second(NodeView node) const // поддерево, которое обходится вторым
    {
        if (is_mirrored()) return node.left();
        return node.right();
    }

    void push_first(NodeView node) // для ЛКП/ПКЛ: спуск по первым детям
    {
        while (node)
        {
            push(node);
            node = first(node);
        }
    }

    void push_to_leaf(NodeView node) // для ЛПК/ПЛК: спуск до листа, первый ребенок в приоритете
    {
        while (node)
        {
            push(node);
            if (first(node))
            {
                node = first(node);
            }
            else
            {
                node = second(node);
            }
        }
    }

public:
    StandartTraversal(Order order = Order::LKP) : stack_(), count_(0), order_(order) {}

    void start(NodeView root) override
    {
        count_ = 0;
        if (order_ == Order::KLP || order_ == Order::KPL)
        {
            if (root) push(root);
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

    NodeView next() override
    {
        if (count_ == 0) return NodeView();

        NodeView current = pop();
        if (order_ == Order::KLP || order_ == Order::KPL)
        {
            if (second(current)) push(second(current)); // второй кладем раньше, чтобы достать позже
            if (first(current)) push(first(current));
        }
        else if (order_ == Order::LKP || order_ == Order::PKL)
        {
            push_first(second(current));
        }
        else if (count_ > 0 && first(top()) == current) // вернулись из первого поддерева
        {
            push_to_leaf(second(top()));
        }

        return current;
    }

    std::unique_ptr<typename Tree::ITraversal> clone() const override
    {
        return std::make_unique<StandartTraversal>(*this);
    }
};
