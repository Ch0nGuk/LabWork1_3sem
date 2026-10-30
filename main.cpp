#include <iostream>
#include <stdexcept>
#include <string>
#include <optional>
#include "include/AVLTree.h"

int main()
{
    int passed = 0;
    int total = 0;

    // insert + size
    {
        AVLTree<int, std::string> tree;
        tree.insert(5, "v");
        tree.insert(3, "v");
        tree.insert(8, "v");
        total++;
        if (tree.size() == 3) { passed++; std::cout << "[OK]   insert\n"; }
        else std::cout << "[FAIL] insert\n";
    }

    // insert duplicate
    {
        AVLTree<int, std::string> tree;
        tree.insert(5, "v");
        bool second = tree.insert(5, "v");
        total++;
        if (!second && tree.size() == 1) { passed++; std::cout << "[OK]   insert duplicate\n"; }
        else std::cout << "[FAIL] insert duplicate\n";
    }

    // find
    {
        AVLTree<int, std::string> tree;
        tree.insert(1, "v");
        tree.insert(2, "v");
        total++;
        if (tree.find(2) && !tree.find(7)) { passed++; std::cout << "[OK]   find\n"; }
        else std::cout << "[FAIL] find\n";
    }

    // find returns value
    {
        AVLTree<int, std::string> tree;
        tree.insert(1, "one");
        tree.insert(2, "two");
        std::optional<std::string> found = tree.find(2);
        total++;
        if (found && *found == "two") { passed++; std::cout << "[OK]   find value\n"; }
        else std::cout << "[FAIL] find value\n";
    }

    // is_empty
    {
        AVLTree<int, std::string> tree;
        bool empty_before = tree.is_empty();
        tree.insert(1, "v");
        total++;
        if (empty_before && !tree.is_empty()) { passed++; std::cout << "[OK]   is_empty\n"; }
        else std::cout << "[FAIL] is_empty\n";
    }

    // remove
    {
        AVLTree<int, std::string> tree;
        tree.insert(1, "v");
        tree.insert(2, "v");
        tree.insert(3, "v");
        tree.remove(2);
        total++;
        if (!tree.find(2) && tree.size() == 2) { passed++; std::cout << "[OK]   remove\n"; }
        else std::cout << "[FAIL] remove\n";
    }

    // remove missing value
    {
        AVLTree<int, std::string> tree;
        tree.insert(1, "v");
        bool thrown = false;
        try { tree.remove(42); }
        catch (const std::logic_error&) { thrown = true; }
        total++;
        if (thrown && tree.size() == 1) { passed++; std::cout << "[OK]   remove missing\n"; }
        else std::cout << "[FAIL] remove missing\n";
    }

    // balance: 1000 sorted inserts
    {
        AVLTree<int, std::string> tree;
        for (int i = 1; i <= 1000; i++) tree.insert(i, "v");
        total++;
        if (tree.size() == 1000 && tree.find(1) && tree.find(1000)) { passed++; std::cout << "[OK]   balance (1000 sorted inserts)\n"; }
        else std::cout << "[FAIL] balance (1000 sorted inserts)\n";
    }

    // // map
    // {
    //     AVLTree<int, std::string> tree;
    //     tree.insert(1, "v");
    //     tree.insert(2, "v");
    //     tree.insert(3, "v");
    //     AVLTree<int, std::string> mapped = tree.map([](const int& x) { return x * 10; });
    //     total++;
    //     if (mapped.find(10) && mapped.find(30) && !mapped.find(1)) { passed++; std::cout << "[OK]   map\n"; }
    //     else std::cout << "[FAIL] map\n";
    // }

    // // where
    // {
    //     AVLTree<int, std::string> tree;
    //     for (int i = 1; i <= 6; i++) tree.insert(i, "v");
    //     AVLTree<int, std::string> even = tree.where([](const int& x) { return x % 2 == 0; });
    //     total++;
    //     if (even.size() == 3 && even.find(4) && !even.find(3)) { passed++; std::cout << "[OK]   where\n"; }
    //     else std::cout << "[FAIL] where\n";
    // }

    // // reduce: example from the task, [1,2,3], f = 2x + 3y, c = 4 -> 144
    // {
    //     AVLTree<int, std::string> tree;
    //     tree.insert(1, "v");
    //     tree.insert(2, "v");
    //     tree.insert(3, "v");
    //     int result = tree.reduce([](const int& x, const int& y) { return 2 * x + 3 * y; }, 4);
    //     total++;
    //     if (result == 144) { passed++; std::cout << "[OK]   reduce\n"; }
    //     else std::cout << "[FAIL] reduce\n";
    // }

    // copy
    {
        AVLTree<int, std::string> tree;
        tree.insert(1, "v");
        tree.insert(2, "v");
        AVLTree<int, std::string> copy(tree);
        copy.insert(3, "v");
        total++;
        if (copy.size() == 3 && tree.size() == 2 && !tree.find(3)) { passed++; std::cout << "[OK]   copy\n"; }
        else std::cout << "[FAIL] copy\n";
    }

    // move
    {
        AVLTree<int, std::string> tree;
        tree.insert(1, "v");
        tree.insert(2, "v");
        AVLTree<int, std::string> moved(std::move(tree));
        total++;
        if (moved.size() == 2 && moved.find(1) && tree.size() == 0) { passed++; std::cout << "[OK]   move\n"; }
        else std::cout << "[FAIL] move\n";
    }

    // get_subtree
    {
        AVLTree<int, std::string> tree;
        for (int i = 1; i <= 7; i++) tree.insert(i, "v");
        std::optional<AVLTree<int, std::string>> sub = tree.get_subtree(2);
        total++;
        if (sub && sub->size() == 3 && sub->find(1) && sub->find(3) && !sub->find(4)) { passed++; std::cout << "[OK]   get_subtree\n"; }
        else std::cout << "[FAIL] get_subtree\n";
    }

    // get_subtree missing key
    {
        AVLTree<int, std::string> tree;
        tree.insert(1, "v");
        total++;
        if (!tree.get_subtree(42)) { passed++; std::cout << "[OK]   get_subtree missing\n"; }
        else std::cout << "[FAIL] get_subtree missing\n";
    }

    std::cout << "\nPassed " << passed << " / " << total << "\n";
    return (passed == total) ? 0 : 1;
}
