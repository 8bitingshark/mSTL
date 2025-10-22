//#include "test/list_test.h"
#include <iostream>
#include "test/tree_test.h"
#include "mmap.h"


int main() {

	//mstl::list_test();
	//mstl::bst_test();
	//mstl::avl_test();
	mstl::rb_test();

	std::cout << "\n=============================\n";
	std::cout << "     TEST MAP \n";
	std::cout << "=============================\n";

	mstl::map<int, int> mm;
	std::pair<int, int> c = { 1,5 };
	std::pair<int, int> d = { 2,6 };
	std::pair<int, int> e = { 3,6 };
	mm.insert(c);
	mm.insert(d);
	mm.insert(e);
	mm.inorder_print();

	return 0;
}