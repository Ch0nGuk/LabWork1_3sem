#include "tests.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <optional>
#include <memory>
#include <chrono>
#include <iomanip>
#include "include/AVLTree.h"

namespace
{
    using Clock = std::chrono::steady_clock;

    void PrintTime(const char* operation, Clock::time_point start, Clock::time_point end)
    {
        double milliseconds = std::chrono::duration<double, std::milli>(end - start).count();
        std::cout << "  " << operation << ": " << std::fixed << std::setprecision(3) << milliseconds << " ms\n";
    }

    bool RunStressTest(int count)
    {
        AVLTree<int, int> tree;
        bool ok = true;

        std::cout << "\nStress test: " << count << " elements\n";

        auto start = Clock::now();
        for (int i = 0; i < count; i++) tree.insert(i, i % 2);
        auto end = Clock::now();
        PrintTime("insert", start, end);
        if (tree.size() != static_cast<unsigned long>(count)) ok = false;

        start = Clock::now();
        for (int i = 0; i < count; i++)
        {
            if (tree.find(i) != i % 2) ok = false;
        }
        end = Clock::now();
        PrintTime("find", start, end);

        int visited = 0;
        start = Clock::now();
        for (const auto& item : tree)
        {
            if (item.key != visited) ok = false;
            visited++;
        }
        end = Clock::now();
        PrintTime("iterate", start, end);
        if (visited != count) ok = false;

        {
            start = Clock::now();
            AVLTree<int, int> mapped = tree.map<int>([](const int& value) { return value + 1; });
            end = Clock::now();
            PrintTime("map", start, end);
            if (mapped.size() != static_cast<unsigned long>(count)) ok = false;
            if (mapped.find(0) != 1) ok = false;
            if (mapped.find(count - 1) != (count - 1) % 2 + 1) ok = false;
        }

        {
            start = Clock::now();
            AVLTree<int, int> filtered = tree.where([](const int& value) { return value == 0; });
            end = Clock::now();
            PrintTime("where", start, end);
            if (filtered.size() != static_cast<unsigned long>(count / 2)) ok = false;
            if (filtered.find(0) != 0) ok = false;
            if (filtered.find(1)) ok = false;
        }

        start = Clock::now();
        int sum = tree.reduce([](const int& value, const int& result) { return value + result; }, 0);
        end = Clock::now();
        PrintTime("reduce", start, end);
        if (sum != count / 2) ok = false;

        start = Clock::now();
        for (int i = 0; i < count; i++) tree.remove(i);
        end = Clock::now();
        PrintTime("remove", start, end);
        if (!tree.is_empty()) ok = false;

        if (ok) std::cout << "[OK]   stress " << count << "\n";
        else std::cout << "[FAIL] stress " << count << "\n";
        return ok;
    }
}

int RunTests()
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

    // map: значения string -> int (длина строки), ключи те же
    {
        AVLTree<int, std::string> tree;
        tree.insert(1, "a");
        tree.insert(2, "bb");
        tree.insert(3, "ccc");
        AVLTree<int, int> mapped = tree.map<int>([](const std::string& s) { return (int)s.size(); });
        total++;
        if (mapped.size() == 3 && mapped.find(1) == 1 && mapped.find(3) == 3) { passed++; std::cout << "[OK]   map\n"; }
        else std::cout << "[FAIL] map\n";
    }

    // where: оставить только значения "even"
    {
        AVLTree<int, std::string> tree;
        for (int i = 1; i <= 6; i++) tree.insert(i, (i % 2 == 0) ? "even" : "odd");
        AVLTree<int, std::string> even = tree.where([](const std::string& s) { return s == "even"; });
        total++;
        if (even.size() == 3 && even.find(4) && !even.find(3)) { passed++; std::cout << "[OK]   where\n"; }
        else std::cout << "[FAIL] where\n";
    }

    // reduce: пример из задания, [1,2,3], f = 2x + 3y, c = 4 -> 144
    {
        AVLTree<int, int> tree;
        tree.insert(1, 1);
        tree.insert(2, 2);
        tree.insert(3, 3);
        int result = tree.reduce([](const int& x, const int& y) { return 2 * x + 3 * y; }, 4);
        total++;
        if (result == 144) { passed++; std::cout << "[OK]   reduce\n"; }
        else std::cout << "[FAIL] reduce\n";
    }

    // reduce с порядком обхода: конкатенация в КЛП, дерево 1..3 (корень 2)
    {
        AVLTree<int, std::string> tree;
        tree.insert(1, "1");
        tree.insert(2, "2");
        tree.insert(3, "3");
        std::string result = tree.reduce([](const std::string& x, const std::string& acc) { return acc + x; }, "", AVLTree<int, std::string>::Order::KLP);
        total++;
        if (result == "213") { passed++; std::cout << "[OK]   reduce KLP\n"; }
        else std::cout << "[FAIL] reduce KLP (got " << result << ")\n";
    }

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

    // iterator: все 6 порядков на дереве 1..7 (корень 4)
    {
        typedef AVLTree<int, std::string> Tree;
        Tree tree;
        for (int i = 1; i <= 7; i++) tree.insert(i, "v");
        Tree::Order orders[6] = { Tree::Order::LKP, Tree::Order::LPK, Tree::Order::KLP, Tree::Order::KPL, Tree::Order::PKL, Tree::Order::PLK };
        std::string names[6] = { "LKP", "LPK", "KLP", "KPL", "PKL", "PLK" };
        std::string expected[6] = { "1234567", "1325764", "4213657", "4675231", "7654321", "7563124" };
        for (int i = 0; i < 6; i++)
        {
            Tree::Iterator it = tree.begin(orders[i]);
            std::string got;
            while (it != tree.end())
            {
                got += std::to_string(it->key);
                ++it;
            }
            total++;
            if (got == expected[i]) { passed++; std::cout << "[OK]   iterator " << names[i] << "\n"; }
            else std::cout << "[FAIL] iterator " << names[i] << " (got " << got << ")\n";
        }
    }

    // iterator: пустое дерево
    {
        AVLTree<int, std::string> tree;
        total++;
        if (tree.begin() == tree.end()) { passed++; std::cout << "[OK]   iterator empty\n"; }
        else std::cout << "[FAIL] iterator empty\n";
    }

    // iterator: разыменование end
    {
        AVLTree<int, std::string> tree;
        tree.insert(1, "v");
        AVLTree<int, std::string>::Iterator it = tree.end();
        total++;
        try
        {
            *it;
            std::cout << "[FAIL] iterator dereference end\n";
        }
        catch (const std::logic_error&)
        {
            passed++;
            std::cout << "[OK]   iterator dereference end\n";
        }
    }

    // iterator: копия и постфиксный инкремент
    {
        AVLTree<int, std::string> tree;
        tree.insert(1, "one");
        tree.insert(2, "two");
        tree.insert(3, "three");
        AVLTree<int, std::string>::Iterator it = tree.begin();
        AVLTree<int, std::string>::Iterator old = it++;
        total++;
        if (old->key == 1 && it->key == 2 && old->value == "one") { passed++; std::cout << "[OK]   iterator copy and postfix\n"; }
        else std::cout << "[FAIL] iterator copy and postfix\n";
    }

    // iterator: range-based for
    {
        AVLTree<int, std::string> tree;
        tree.insert(2, "two");
        tree.insert(1, "one");
        tree.insert(3, "three");
        const AVLTree<int, std::string>& const_tree = tree;
        std::string got;
        for (const auto& item : const_tree) got += std::to_string(item.key);
        total++;
        if (got == "123") { passed++; std::cout << "[OK]   iterator range-based for\n"; }
        else std::cout << "[FAIL] iterator range-based for (got " << got << ")\n";
    }

    const int stress_sizes[] = { 10000, 100000, 1000000, 2000000 };
    for (int count : stress_sizes)
    {
        total++;
        if (RunStressTest(count)) passed++;
    }

    std::cout << "\nPassed " << passed << " / " << total << "\n";
    return (passed == total) ? 0 : 1;
}
