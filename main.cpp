#include <iostream>
#include <stdexcept>
#include "include/AVLTree.h"

int main()
{
    int passed = 0;
    int total = 0;

    // insert + size
    {
        AVLTree<int> tree;
        tree.insert(5);
        tree.insert(3);
        tree.insert(8);
        total++;
        if (tree.size() == 3) { passed++; std::cout << "[OK]   insert\n"; }
        else std::cout << "[FAIL] insert\n";
    }

    // insert duplicate
    {
        AVLTree<int> tree;
        tree.insert(5);
        bool second = tree.insert(5);
        total++;
        if (!second && tree.size() == 1) { passed++; std::cout << "[OK]   insert duplicate\n"; }
        else std::cout << "[FAIL] insert duplicate\n";
    }

    // contains
    {
        AVLTree<int> tree;
        tree.insert(1);
        tree.insert(2);
        total++;
        if (tree.contains(2) && !tree.contains(7)) { passed++; std::cout << "[OK]   contains\n"; }
        else std::cout << "[FAIL] contains\n";
    }

    // is_empty
    {
        AVLTree<int> tree;
        bool empty_before = tree.is_empty();
        tree.insert(1);
        total++;
        if (empty_before && !tree.is_empty()) { passed++; std::cout << "[OK]   is_empty\n"; }
        else std::cout << "[FAIL] is_empty\n";
    }

    // remove
    {
        AVLTree<int> tree;
        tree.insert(1);
        tree.insert(2);
        tree.insert(3);
        tree.remove(2);
        total++;
        if (!tree.contains(2) && tree.size() == 2) { passed++; std::cout << "[OK]   remove\n"; }
        else std::cout << "[FAIL] remove\n";
    }

    // remove missing value
    {
        AVLTree<int> tree;
        tree.insert(1);
        bool thrown = false;
        try { tree.remove(42); }
        catch (const std::logic_error&) { thrown = true; }
        total++;
        if (thrown && tree.size() == 1) { passed++; std::cout << "[OK]   remove missing\n"; }
        else std::cout << "[FAIL] remove missing\n";
    }

    // balance: 1000 sorted inserts
    {
        AVLTree<int> tree;
        for (int i = 1; i <= 1000; i++) tree.insert(i);
        total++;
        if (tree.size() == 1000 && tree.contains(1) && tree.contains(1000)) { passed++; std::cout << "[OK]   balance (1000 sorted inserts)\n"; }
        else std::cout << "[FAIL] balance (1000 sorted inserts)\n";
    }

    // map
    {
        AVLTree<int> tree;
        tree.insert(1);
        tree.insert(2);
        tree.insert(3);
        AVLTree<int> mapped = tree.map([](const int& x) { return x * 10; });
        total++;
        if (mapped.contains(10) && mapped.contains(30) && !mapped.contains(1)) { passed++; std::cout << "[OK]   map\n"; }
        else std::cout << "[FAIL] map\n";
    }

    // where
    {
        AVLTree<int> tree;
        for (int i = 1; i <= 6; i++) tree.insert(i);
        AVLTree<int> even = tree.where([](const int& x) { return x % 2 == 0; });
        total++;
        if (even.size() == 3 && even.contains(4) && !even.contains(3)) { passed++; std::cout << "[OK]   where\n"; }
        else std::cout << "[FAIL] where\n";
    }

    // reduce: example from the task, [1,2,3], f = 2x + 3y, c = 4 -> 144
    {
        AVLTree<int> tree;
        tree.insert(1);
        tree.insert(2);
        tree.insert(3);
        int result = tree.reduce([](const int& x, const int& y) { return 2 * x + 3 * y; }, 4);
        total++;
        if (result == 144) { passed++; std::cout << "[OK]   reduce\n"; }
        else std::cout << "[FAIL] reduce\n";
    }

    // copy
    {
        AVLTree<int> tree;
        tree.insert(1);
        tree.insert(2);
        AVLTree<int> copy(tree);
        copy.insert(3);
        total++;
        if (copy.size() == 3 && tree.size() == 2 && !tree.contains(3)) { passed++; std::cout << "[OK]   copy\n"; }
        else std::cout << "[FAIL] copy\n";
    }

    // move
    {
        AVLTree<int> tree;
        tree.insert(1);
        tree.insert(2);
        AVLTree<int> moved(std::move(tree));
        total++;
        if (moved.size() == 2 && moved.contains(1) && tree.size() == 0) { passed++; std::cout << "[OK]   move\n"; }
        else std::cout << "[FAIL] move\n";
    }

    // get_subtree
    {
        AVLTree<int> tree;
        for (int i = 1; i <= 7; i++) tree.insert(i);
        AVLTree<int> sub = tree.get_subtree(2);
        total++;
        if (sub.size() == 3 && sub.contains(1) && sub.contains(3) && !sub.contains(4)) { passed++; std::cout << "[OK]   get_subtree\n"; }
        else std::cout << "[FAIL] get_subtree\n";
    }

    std::cout << "\nPassed " << passed << " / " << total << "\n";
    return (passed == total) ? 0 : 1;
}
