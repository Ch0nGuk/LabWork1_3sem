#include "UI.h"

#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "include/AVLTree.h"
#include "include/DynamicArray.h"
#include "include/Student.h"
#include "tests.h"

namespace
{
    typedef AVLTree<int, int> Tree;
    typedef AVLTree<unsigned long, Student> StudentTree;

    const int demo_depth = 10;
    const int demo_size = (1 << demo_depth) - 1;

    Student MakeDemoStudent(int index)
    {
        const char* surnames[] = {
            "Иванов", "Петров", "Сидоров", "Смирнов", "Кузнецов", "Попов", "Васильев", "Соколов",
            "Михайлов", "Новиков", "Фёдоров", "Морозов", "Волков", "Алексеев", "Лебедев", "Семёнов",
            "Егоров", "Павлов", "Козлов", "Степанов", "Николаев", "Орлов", "Андреев", "Макаров",
            "Захаров", "Белов", "Тарасов", "Борисов", "Киселёв", "Григорьев", "Данилов", "Титов"
        };
        const char* names[] = {
            "Александр", "Дмитрий", "Иван", "Сергей", "Андрей", "Михаил", "Алексей", "Николай",
            "Павел", "Владимир", "Евгений", "Артём", "Максим", "Кирилл", "Денис", "Илья",
            "Роман", "Виктор", "Олег", "Юрий", "Антон", "Борис", "Григорий", "Георгий",
            "Тимофей", "Валентин", "Константин", "Леонид", "Матвей", "Данил", "Фёдор", "Пётр"
        };

        const int name_count = sizeof(names) / sizeof(names[0]);
        const int surname_count = sizeof(surnames) / sizeof(surnames[0]);
        std::string digits = std::to_string(1000000 + index);

        return Student{
            surnames[(index / name_count) % surname_count],
            names[index % name_count],
            "Б23-" + std::to_string(501 + index / 30),
            "+7-916-" + digits.substr(0, 3) + "-" + digits.substr(3, 2) + "-" + digits.substr(5, 2)
        };
    }

    void FillDemoLevel(Tree& numbers, StudentTree& students, int first, int last, int level)
    {
        if (first > last) return;
        int middle = first + (last - first) / 2;

        if (level == 0)
        {
            numbers.insert(middle, middle * 10);
            students.insert(100000UL + static_cast<unsigned long>(middle), MakeDemoStudent(middle - 1));
            return;
        }

        FillDemoLevel(numbers, students, first, middle - 1, level - 1);
        FillDemoLevel(numbers, students, middle + 1, last, level - 1);
    }

    struct InputClosed {};

    struct UiState
    {
        DynamicArray<std::shared_ptr<Tree>> trees;
        int active;
        StudentTree students;

        UiState() : trees(1, std::make_shared<Tree>()), active(0), students()
        {
            for (int level = 0; level < demo_depth; level++)
            {
                FillDemoLevel(Current(), students, 1, demo_size, level);
            }
        }

        Tree& Current()
        {
            return *trees[active];
        }

        const Tree& Current() const
        {
            return *trees[active];
        }
    };

    int ReadInt(const std::string& prompt)
    {
        while (true)
        {
            std::cout << prompt;
            std::string line;
            if (!std::getline(std::cin, line)) throw InputClosed();

            std::istringstream input(line);
            int value = 0;
            char extra = 0;
            if ((input >> value) && !(input >> extra)) return value;

            std::cout << "Введите целое число.\n";
        }
    }

    unsigned long ReadId()
    {
        while (true)
        {
            std::cout << "ID студента: ";
            std::string line;
            if (!std::getline(std::cin, line)) throw InputClosed();

            std::istringstream input(line);
            unsigned long id = 0;
            char extra = 0;
            if (line.find('-') == std::string::npos && (input >> id) && !(input >> extra)) return id;

            std::cout << "Введите неотрицательное целое число в диапазоне unsigned long.\n";
        }
    }

    std::string ReadText(const std::string& prompt)
    {
        while (true)
        {
            std::cout << prompt;
            std::string line;
            if (!std::getline(std::cin, line)) throw InputClosed();
            if (line.find_first_not_of(" \t") != std::string::npos) return line;
            std::cout << "Поле не должно быть пустым.\n";
        }
    }

    Tree::Order ReadOrder()
    {
        Tree::Order orders[6] = {
            Tree::Order::LKP, Tree::Order::LPK, Tree::Order::KLP,
            Tree::Order::KPL, Tree::Order::PKL, Tree::Order::PLK
        };

        while (true)
        {
            std::cout << "Порядок обхода: 1. ЛКП  2. ЛПК  3. КЛП  4. КПЛ  5. ПКЛ  6. ПЛК\n";
            int choice = ReadInt("Выбор: ");
            if (choice >= 1 && choice <= 6) return orders[choice - 1];
            std::cout << "Нет такого порядка.\n";
        }
    }

    void PrintTree(const Tree& tree, Tree::Order order)
    {
        std::cout << "Размер: " << tree.size() << ". Элементы: ";
        if (tree.is_empty())
        {
            std::cout << "(пусто)\n";
            return;
        }

        const std::size_t limit = 50;
        std::size_t printed = 0;
        for (Tree::Iterator it = tree.begin(order); it != tree.end() && printed < limit; ++it)
        {
            if (printed > 0) std::cout << ", ";
            std::cout << it->key << ":" << it->value;
            printed++;
        }
        if (tree.size() > limit) std::cout << ", ... (показаны первые " << limit << " из " << tree.size() << ")";
        std::cout << "\n";
    }

    void PrintTrees(const UiState& state)
    {
        for (int index = 0; index < state.trees.GetSize(); index++)
        {
            std::cout << index + 1 << ". Дерево, размер " << state.trees[index]->size();
            if (index == state.active) std::cout << " (активное)";
            std::cout << "\n";
        }
    }

    void AddTree(UiState& state, std::shared_ptr<Tree> tree)
    {
        int index = state.trees.GetSize();
        state.trees.Resize(index + 1, std::shared_ptr<Tree>());
        state.trees[index] = std::move(tree);
        state.active = index;
    }

    void SaveTree(UiState& state, Tree&& tree)
    {
        AddTree(state, std::make_shared<Tree>(std::move(tree)));
        std::cout << "Результат сохранён как дерево №" << state.active + 1 << ".\n";
        PrintTree(state.Current(), Tree::Order::LKP);
    }

    int CheckedInt(long long value)
    {
        if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
        {
            throw std::overflow_error("Результат не помещается в int");
        }
        return static_cast<int>(value);
    }

    void CreateTree(UiState& state)
    {
        AddTree(state, std::make_shared<Tree>());
        std::cout << "Создано дерево №" << state.active + 1 << ".\n";
    }

    void SelectTree(UiState& state)
    {
        PrintTrees(state);
        int choice = ReadInt("Номер дерева: ");
        if (choice < 1 || choice > state.trees.GetSize())
        {
            std::cout << "Нет дерева с таким номером.\n";
            return;
        }
        state.active = choice - 1;
        std::cout << "Активно дерево №" << choice << ".\n";
    }

    void Insert(UiState& state)
    {
        int key = ReadInt("Ключ: ");
        int value = ReadInt("Значение: ");
        if (state.Current().insert(key, value)) std::cout << "Элемент добавлен.\n";
        else std::cout << "Такой ключ уже есть; значение не изменено.\n";
    }

    void Find(const UiState& state)
    {
        int key = ReadInt("Ключ: ");
        std::optional<int> value = state.Current().find(key);
        if (value) std::cout << "Найдено: " << key << ":" << *value << "\n";
        else std::cout << "Ключ не найден.\n";
    }

    void Remove(UiState& state)
    {
        int key = ReadInt("Ключ: ");
        if (!state.Current().find(key))
        {
            std::cout << "Ключ не найден.\n";
            return;
        }
        state.Current().remove(key);
        std::cout << "Элемент удалён.\n";
    }

    void Map(UiState& state)
    {
        std::cout << "1. Прибавить число к значениям\n2. Умножить значения на число\n";
        int choice = ReadInt("Выбор: ");
        Tree result;

        if (choice == 1)
        {
            int delta = ReadInt("Сколько прибавить: ");
            result = state.Current().map<int>([delta](const int& value) {
                return CheckedInt(static_cast<long long>(value) + delta);
            });
        }
        else if (choice == 2)
        {
            int factor = ReadInt("Множитель: ");
            result = state.Current().map<int>([factor](const int& value) {
                return CheckedInt(static_cast<long long>(value) * factor);
            });
        }
        else
        {
            std::cout << "Нет такого преобразования.\n";
            return;
        }

        SaveTree(state, std::move(result));
    }

    void Where(UiState& state)
    {
        std::cout << "1. Чётные значения\n2. Положительные значения\n3. Значения не меньше порога\n";
        int choice = ReadInt("Выбор: ");
        Tree result;

        if (choice == 1)
        {
            result = state.Current().where([](const int& value) { return value % 2 == 0; });
        }
        else if (choice == 2)
        {
            result = state.Current().where([](const int& value) { return value > 0; });
        }
        else if (choice == 3)
        {
            int threshold = ReadInt("Порог: ");
            result = state.Current().where([threshold](const int& value) { return value >= threshold; });
        }
        else
        {
            std::cout << "Нет такого условия.\n";
            return;
        }

        SaveTree(state, std::move(result));
    }

    void Reduce(UiState& state)
    {
        std::cout << "1. Сумма\n2. Значение минус накопленный результат\n";
        int choice = ReadInt("Выбор: ");
        if (choice != 1 && choice != 2)
        {
            std::cout << "Нет такой операции.\n";
            return;
        }

        int start = ReadInt("Начальное значение: ");
        Tree::Order order = ReadOrder();
        int result = 0;

        if (choice == 1)
        {
            result = state.Current().reduce([](const int& value, const int& acc) {
                return CheckedInt(static_cast<long long>(acc) + value);
            }, start, order);
        }
        else
        {
            result = state.Current().reduce([](const int& value, const int& acc) {
                return CheckedInt(static_cast<long long>(value) - acc);
            }, start, order);
        }

        std::cout << "Результат: " << result << "\n";
    }

    void ExtractSubtree(UiState& state)
    {
        int key = ReadInt("Ключ корня поддерева: ");
        std::optional<Tree> subtree = state.Current().get_subtree(key);
        if (!subtree)
        {
            std::cout << "Ключ не найден.\n";
            return;
        }
        SaveTree(state, std::move(*subtree));
    }

    void PrintStudent(unsigned long id, const Student& student)
    {
        std::cout << "ID " << id << ": " << student.surname << " " << student.name
                  << ", группа " << student.group_number
                  << ", телефон " << student.phone_number << "\n";
    }

    void AddStudent(UiState& state)
    {
        unsigned long id = ReadId();
        if (state.students.find(id))
        {
            std::cout << "Студент с таким ID уже есть.\n";
            return;
        }

        Student student{
            ReadText("Фамилия: "),
            ReadText("Имя: "),
            ReadText("Номер группы: "),
            ReadText("Номер телефона: ")
        };

        state.students.insert(id, student);
        std::cout << "Студент добавлен.\n";
    }

    void FindStudent(const UiState& state)
    {
        unsigned long id = ReadId();
        std::optional<Student> student = state.students.find(id);
        if (student) PrintStudent(id, *student);
        else std::cout << "Студент не найден.\n";
    }

    void PrintStudents(const UiState& state)
    {
        if (state.students.is_empty())
        {
            std::cout << "Студентов пока нет.\n";
            return;
        }

        std::cout << "Студенты (" << state.students.size() << "):\n";
        const std::size_t limit = 50;
        std::size_t printed = 0;
        for (StudentTree::Iterator it = state.students.begin(); it != state.students.end() && printed < limit; ++it)
        {
            PrintStudent(it->key, it->value);
            printed++;
        }
        if (state.students.size() > limit)
        {
            std::cout << "Показаны первые " << limit << " из " << state.students.size() << ".\n";
        }
    }

    void RemoveStudent(UiState& state)
    {
        unsigned long id = ReadId();
        if (!state.students.find(id))
        {
            std::cout << "Студент не найден.\n";
            return;
        }
        state.students.remove(id);
        std::cout << "Студент удалён.\n";
    }

    void StudentsMenu(UiState& state)
    {
        while (true)
        {
            std::cout << "\n--- Студенты: " << state.students.size() << " ---\n";
            std::cout << "1. Добавить\n2. Найти по ID\n3. Показать первые 50\n4. Удалить по ID\n0. Назад\n";
            int choice = ReadInt("Выбор: ");

            switch (choice)
            {
            case 1: AddStudent(state); break;
            case 2: FindStudent(state); break;
            case 3: PrintStudents(state); break;
            case 4: RemoveStudent(state); break;
            case 0: return;
            default: std::cout << "Нет такого пункта меню.\n"; break;
            }
        }
    }

    void PrintMenu(const UiState& state)
    {
        std::cout << "\n--- AVL дерево №" << state.active + 1
                  << " (размер " << state.Current().size() << "), студентов "
                  << state.students.size() << " ---\n";
        std::cout << "1. Создать пустое дерево\n";
        std::cout << "2. Показать и выбрать дерево\n";
        std::cout << "3. Показать элементы в выбранном порядке\n";
        std::cout << "4. Вставить элемент\n";
        std::cout << "5. Найти элемент\n";
        std::cout << "6. Удалить элемент\n";
        std::cout << "7. Map\n";
        std::cout << "8. Where\n";
        std::cout << "9. Reduce\n";
        std::cout << "10. Извлечь поддерево\n";
        std::cout << "11. Запустить тесты\n";
        std::cout << "12. Студенты\n";
        std::cout << "0. Выход\n";
    }
}

void RunUI()
{
    UiState state;
    std::cout << "Созданы два демонстрационных дерева: " << state.Current().size()
              << " чисел и " << state.students.size() << " студентов.\n";

    try
    {
        while (true)
        {
            PrintMenu(state);
            int choice = ReadInt("Выбор: ");

            try
            {
                switch (choice)
                {
                case 1: CreateTree(state); break;
                case 2: SelectTree(state); break;
                case 3: PrintTree(state.Current(), ReadOrder()); break;
                case 4: Insert(state); break;
                case 5: Find(state); break;
                case 6: Remove(state); break;
                case 7: Map(state); break;
                case 8: Where(state); break;
                case 9: Reduce(state); break;
                case 10: ExtractSubtree(state); break;
                case 11: RunTests(); break;
                case 12: StudentsMenu(state); break;
                case 0: return;
                default: std::cout << "Нет такого пункта меню.\n"; break;
                }
            }
            catch (const std::exception& error)
            {
                std::cout << "Ошибка: " << error.what() << "\n";
            }
        }
    }
    catch (const InputClosed&)
    {
        std::cout << "\nВвод завершён.\n";
    }
}
