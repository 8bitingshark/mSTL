#include "test/list_test.h"
#include "mlist.h"
#include <iostream>

void mstl::list_test()
{
	std::cout << "\n=============================\n";
	std::cout << "     TEST MSTL LIST\n";
	std::cout << "=============================\n";

	mstl::list<int> lst;
	mstl::print_list(lst, "empty list");
	mstl::visualize(lst);

	// Push back
	lst.push_back(1);
	lst.push_back(2);
	lst.push_back(3);
	mstl::print_list(lst, "after push_back(1,2,3)");
	mstl::visualize(lst);

	// Push front
	lst.push_front(0);
	mstl::print_list(lst, "after push_front(0)");
	mstl::visualize(lst);

	// Erase middle
	mstl::list<int>::iterator it = ++lst.begin(); // points to element 1
	lst.erase(it);
	mstl::print_list(lst, "after erase(1)");
	mstl::visualize(lst);

	// Insert at second position
	it = ++lst.begin(); // points to element 2
	lst.insert(it, 42);
	mstl::print_list(lst, "after insert(42 before 2)");
	mstl::visualize(lst);

	// Pop front/back
	lst.pop_front();
	lst.pop_back();
	mstl::print_list(lst, "after pop_front() + pop_back()");
	mstl::visualize(lst);

	// Iterate backwards
	std::cout << "Backward traversal: [ ";
	mstl::list<int>::iterator itb = --lst.end();
	
	for (; itb != lst.end(); --itb)
		std::cout << *itb << " ";

	std::cout << "]\n\n";

	std::cout << "front = " << lst.front() << ", back = " << lst.back() << "\n";

	std::cout << "All tests complete.\n";
}
