#include <iostream>
#include "mlist.h"

int main() {

	std::cout << "=== TEST BASE ===\n";

	// not doing this at home
	using namespace mstl;

	list<int> lst;
	print_list(lst, "empty list");
	visualize(lst);

	// Push back
	lst.push_back(1);
	lst.push_back(2);
	lst.push_back(3);
	print_list(lst, "after push_back(1,2,3)");
	visualize(lst);

	// Push front
	lst.push_front(0);
	print_list(lst, "after push_front(0)");
	visualize(lst);

	// Erase middle
	list<int>::iterator it = ++lst.begin(); // points to element 1
	lst.erase(it);
	print_list(lst, "after erase(1)");
	visualize(lst);

	// Insert at second position
	it = ++lst.begin(); // points to element 2
	lst.insert(it, 42);
	print_list(lst, "after insert(42 before 2)");
	visualize(lst);

	// Pop front/back
	lst.pop_front();
	lst.pop_back();
	print_list(lst, "after pop_front() + pop_back()");
	visualize(lst);

	// Iterate backwards
	std::cout << "Backward traversal: [ ";
	list<int>::iterator itb = --lst.end();
	for (; itb != lst.end(); --itb)
		std::cout << *itb << " ";
	std::cout << "]\n\n";

	std::cout << "front = " << lst.front() << ", back = " << lst.back() << "\n";

	std::cout << "All tests complete.\n";

	return 0;
}