#ifndef MSTL_AVL_TREE_H
#define MSTL_AVL_TREE_H

#include "tree.h"

namespace mstl {

	/// ---------------------------------------------------------------
	/// avl node
	/// ---------------------------------------------------------------
	/// derives from node<T>
	/// it adds height as param
	/// 
	/// [!] no need of virtual dtor because I don't use new/delete with
	///     base class.
	
	template<typename T>
	struct avl_node : public node<T> {

		size_t m_Height{};

		explicit avl_node(const T& v)
			: node<T>(v) {
		}

		explicit avl_node(T&& v)
			: node<T>(std::move(v)) {
		}

		template<class... Args>
		explicit avl_node(std::in_place_t, Args&&... args)
			: node<T>(std::in_place, std::forward<Args>(args)...) {
		}
	};

	/// ---------------------------------------------------------------
	/// AVL Tree
	/// ---------------------------------------------------------------
	/// Balance Property: | left_height - right_height | <= 1
	
	template<typename T, typename compare = std::less<T>, typename A = std::allocator<T>>
	class avl : public tree_base<T, compare, A, avl_node> {

		using base_type = tree_base<T, compare, A>;
		using node_type = avl_node<T>;
		using base_node_type = node_base;

	public:
		using typename base_type::iterator;
		using typename base_type::const_iterator;

		// if in the future you want to modify public visibility
		// of the base class you can export them
		// using typename base_type::key_type;
		// using typename base_type::value_type;
	
		using base_type::base_type;
		explicit avl(const compare& c, const A& a = A{}) : base_type(a, c) {}
	
	};
}

#endif // !MSTL_AVL_TREE_H
