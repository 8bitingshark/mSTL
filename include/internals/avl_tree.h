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

		int m_Height{}; // usually enough

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
	
	template<
		typename T,
		typename compare = std::less<T>,
		typename A = std::allocator<T>,
		template<class> class node_t = node
	>
	class avl_tree : public tree_base<T, compare, A, node_t> {

		using base_type = tree_base<T, compare, A, node_t>;
		using node_type = typename base_type::node_type;
		using base_node_type = node_base;
		using node_alloc = typename base_type::node_alloc;
		using node_traits = typename base_type::node_traits;

	public:
		
		using key_type = typename node_type::value_type;
		using value_type = typename node_type::value_type;
		using size_type = typename base_type::size_type;
		using difference_type = std::ptrdiff_t;
		using key_compare = compare;
		using value_compare = compare;
		using alloc_type = A;

		using iterator = tree_iterator<node_type, false>;
		using const_iterator = tree_iterator<node_type, true>;
	
		// === Constructors ===
		// C++20: inherites constructors
		using base_type::base_type;

		explicit avl_tree(const alloc_type& a = alloc_type{}, const compare& c = compare{})
			: base_type(a,c) {
		}

		template<typename It = std::input_iterator>
		avl_tree(It first, It last, const alloc_type& a = alloc_type{}, const compare& c = compare{})
			: base_type(a, c)
		{
			for (; first != last; ++first)
				insert(*first);
		}

		avl_tree(std::initializer_list<T> il, const alloc_type& a = alloc_type{}, const compare& c = compare{})
			: base_type(a, c)
		{
			for (const auto& v : il)
				insert(v);
		}
	
		// === Copy semantics ===

		avl_tree(const avl_tree& other)
			: base_type(other.m_ValueAlloc, other.m_Comp) {

			for (const auto& v : other) insert(v);
		}

		avl_tree& operator=(const avl_tree& other) {

			if (this == &other) return *this;

			avl_tree tmp(other);
			swap(tmp);
			return *this;
		}

		// === Move semantics ===

		avl_tree(const avl_tree&& other) noexcept {

			swap(other);
		}

		avl_tree& operator=(avl_tree&& other) noexcept {

			if (this != &other) swap(other);
			return *this;
		}

		/// === Iterators ===
	};
}

#endif // !MSTL_AVL_TREE_H
