#include "test/tree_test.h"
#include "internals/binary_search_tree.h"
#include "internals/avl_tree.h"
#include <vector>

void mstl::bst_test()
{
    std::cout << "\n=============================\n";
    std::cout << "     TEST BINARY SEARCH TREE\n";
    std::cout << "=============================\n";

    // Base insertion
    bst_tree<int> t;
    std::vector<int> values = { 8, 3, 10, 1, 6, 14, 4, 7, 13 };

    for (int v : values) {
        t.insert(v);
    }

    std::cout << "\nAfter insertion: " << "\n";
    t.inorder_print();

    // Find, contains
    std::cout << "\nFind(6): " << (t.find(6) != t.end() ? "Found" : "Not found") << "\n";
    std::cout << "Contains(11): " << (t.contains(11) ? "Yes" : "No") << "\n";

    // Lower/upper bound
    std::cout << "\nLower_bound(5): ";
    auto itL = t.lower_bound(5);
    if (itL != t.end()) std::cout << *itL << "\n"; else std::cout << "end\n";

    std::cout << "Upper_bound(7): ";
    auto itU = t.upper_bound(7);
    if (itU != t.end()) std::cout << *itU << "\n"; else std::cout << "end\n";

    // Erasing
    t.erase(7);
    std::cout << "\nAfter erase(7)" << "\n";
    t.inorder_print();

    t.erase(14);
    std::cout << "\nAfter erase(14)" << "\n";
    t.inorder_print();

    t.erase(3);
    std::cout << "\nAfter erase(3)" << "\n";
    t.inorder_print();

    t.erase(8);
    std::cout << "\nAfter erase(8)" << "\n";
    t.inorder_print();
    
    std::cout << "\nTutti i test completati.\n";
}

void mstl::avl_test()
{
    std::cout << "\n=============================\n";
    std::cout << "     TEST AVL TREE\n";
    std::cout << "=============================\n";

    // Base insertion
    avl_tree<int> t;
    std::vector<int> values = { 8, 4, 10 };

    for (int v : values) {
        t.insert(v);
    }

    /*std::cout << "\nAfter insertion: " << "\n";
    t.inorder_print();*/

    t.insert(6);
    t.insert(1);
    t.insert(5);
    t.erase(6);
    t.inorder_print();
}
