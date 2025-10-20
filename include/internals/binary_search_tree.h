#ifndef BINARY_SEARCH_TREE_H
#define BINARY_SEARCH_TREE_H

#include "concepts_utils.h"
#include "tree.h"
#include <initializer_list>
#include <algorithm>
#include <cassert>
#include <iostream>

/// TODO: calculate diameter tree  

namespace mstl {

	/// ---------------------------------------------------------------
	/// BST
	/// ---------------------------------------------------------------

	template<
		typename T, 
		typename compare = std::less<T>, 
		typename A = std::allocator<T>,
		template<class> class node_t = node
	>
	class bst : public tree_base<T, compare, A, node_t> {

		using base_type      = tree_base<T, compare, A, node_t>;
		using node_type      = typename base_type::node_type;
		using base_node_type = node_base;
		using node_alloc     = typename base_type::node_alloc;
		using node_traits    = typename base_type::node_traits;

	public:

		using key_type        = typename node_type::value_type;
		using value_type      = typename node_type::value_type;
		using size_type       = typename base_type::size_type;
		using difference_type = std::ptrdiff_t;
		using key_compare     = compare;
		using value_compare   = compare;
		using alloc_type      = A;

		using iterator       = tree_iterator<node_type, false>;
		using const_iterator = tree_iterator<node_type, true>;

		// === Constructors ===
		// C++20: inherites constructors
		using base_type::base_type;

	    //bst() = default;

		explicit bst(const alloc_type& a = alloc_type{}, const compare& c = compare{})
			: base_type(a,c){
		}

		template<typename It = std::input_iterator>
		bst(It first, It last, const alloc_type& a = alloc_type{}, const compare& c = compare{})
			: base_type(a,c)
		{
			for (; first != last; ++first)
				insert(*first);
		}

		bst(std::initializer_list<T> il, const alloc_type& a = alloc_type{}, const compare& c = compare{})
			: base_type(a, c)
		{
			for (const auto& v : il)
				insert(v);
		}

		// === Copy semantics ===

		bst(const bst& other)
			: base_type(other.m_ValueAlloc, other.m_Comp) {

			for (const auto& v : other) insert(v);
		}

		bst& operator=(const bst& other) {

			if (this == &other) return *this;

			bst tmp(other);
			swap(tmp);
			return *this;
		}

		// === Move semantics ===

		bst(const bst&& other) noexcept {

			swap(other);
		}

		bst& operator=(bst&& other) noexcept {

			if (this != &other) swap(other);
			return *this;
		}

		/// === Iterators ===

		iterator begin() noexcept { return iterator{ min_node() }; }
		const_iterator begin() const noexcept { return const_iterator{ max_node() }; }
		//const_iterator cbegin() const noexcept { return const_iterator{ leftmost() }; }

		iterator end() noexcept { return iterator{ nullptr }; }
		const_iterator end() const noexcept { return const_iterator{ nullptr }; }
		//const_iterator cend() const noexcept { return const_iterator{ nullptr }; }

		/// === Capacity ===

		size_type size() const noexcept { return this->m_Size; }

		bool empty() const noexcept { return this->m_Size == 0; }

		/// ================= Access =================

		// lookup

		iterator find(const key_type& v) noexcept { return iterator{ find_node(v) }; }
		const_iterator find(const key_type& v) const noexcept { return const_iterator{ find_node(v) }; }

		iterator lower_bound(const key_type& v) noexcept { return iterator{ lower_bound_node(v) }; }
		const_iterator lower_bound(const key_type& v) const noexcept { return const_iterator{ lower_bound_node(v) }; }

		iterator upper_bound(const key_type& v) noexcept { return iterator{ upper_bound_node(v) }; }
		const_iterator upper_bound(const key_type& v) const noexcept { return const_iterator{ upper_bound_node(v) }; }

		bool contains(const key_type& key) const noexcept {
			return find_node(key) != nullptr;
		}

		std::pair<iterator, iterator> equal_range(const key_type& key) noexcept {
			return { lower_bound(key), upper_bound(key) };
		}

		std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const noexcept {
			return { lower_bound(key), upper_bound(key) };
		}

		/// ================= Modifiers =================

		void clear() noexcept { this->DoClear(); }

		/// Using U and not T for perfect forwading.
		/// U is called universal reference or forwading reference.
		/// U&& isn't always an rvalue!
		/// The problem of using T/value_type is that rigidly choose T
		/// as param.
		/// You may want an lvalue, rvalue or any type compatible with T, so
		/// that you can convert or construct in T
		
		template<typename U>
		std::pair<iterator, bool> insert(U&& v) {

			auto [n, ok] = insert_impl(std::forward<U>(v));

			return { iterator{ n }, ok };
		}

		template<class... Args>
		std::pair<iterator, bool> emplace(Args&&... args)
		{
			value_type temp(std::forward<Args>(args)...);

			auto [n, ok] = insert_impl(std::move(temp));

			return { iterator{ n }, ok };
		}

		// erase by key
		size_type erase(const key_type& v) {
			
			base_node_type* z = find_node(v); 
			if (!z) return 0;
			erase_node(z);
			return 1;
		}

		// erase by iterator -> returns successor
		iterator erase(iterator pos) {
			
			base_node_type* z = pos.curr;
			if (!z) return end();
			base_node_type* s = successor(z);
			erase_node(z);
			return iterator{ s };
		}

		void swap(bst& other) noexcept {
			using std::swap;
			swap(this->m_ValueAlloc, other.m_ValueAlloc);
			swap(this->m_NodeAlloc, other.m_NodeAlloc);
			swap(this->m_Comp, other.m_Comp);
			swap(this->m_Size, other.m_Size);
			swap(this->mp_Root, other.mp_Root);
		}

		/// ================= Observers =================

		const node_type* root() const noexcept { return this->mp_Root; }

		alloc_type get_allocator() const { return this->m_ValueAlloc; }

		/// ================= Utility =================

		void inorder_print() const noexcept {
			inorder_print_rec(this->mp_Root, nullptr, "root");
		}

	private:

		// ================= Helpers =================

		static base_node_type* min_node(base_node_type* i_node) noexcept
		{
			while (i_node && i_node->mp_Left) {
			
				i_node = i_node->mp_Left;
			}
				
			return i_node;
		}

		base_node_type* min_node() const noexcept { return min_node(this->mp_Root); }

		static base_node_type* max_node(base_node_type* i_node) noexcept
		{
			while (i_node && i_node->mp_Right)
			{
				i_node = i_node->mp_Right;
			}

			return i_node;
		}

		base_node_type* max_node() const noexcept { return max_node(this->mp_Root); }

		static base_node_type* successor(base_node_type* n) noexcept {

			if (!n) return nullptr;

			// Case 1: there is a right sub-tree: find the minimum of this right sub-tree
			if (n->mp_Right)
			{
				return min_node(n->mp_Right);
			}

			// Case 2: go up until you are left child 
			auto* p = n->mp_Parent;

			while (p && n == p->mp_Right)
			{
				n = p;
				p = p->mp_Parent;
			}

			return p;
		}

		static base_node_type* predecessor(base_node_type* n) noexcept {

			if (!n) return nullptr;

			// case 1: left child exist so find the max
			if (n->mp_Left)
			{
				return max_node(n->mp_Left);
			}

			// case 2: no left child, go up until you are a right child 
			// and return parent
			auto* p = n->mp_Parent;
			while (p && n == p->mp_Left)
			{
				n = p;
				p = p->mp_Parent;
			}

			return p;
		}

		base_node_type* find_node(const key_type& v) const noexcept {
		
			base_node_type* current = this->mp_Root;

			while (current) {
			
				const value_type& val = static_cast<const node_type*>(current)->m_Val;

				if (this->m_Comp(v, val))
				{
					current = current->mp_Left;
				}
				else if (this->m_Comp(val, v))
				{
					current = current->mp_Right;
				}
				else
				{
					return current;
				}
			}

			return nullptr;
		}

		// first element >= key
		base_node_type* lower_bound_node(const key_type& key) const noexcept {

			base_node_type* current = this->mp_Root;
			base_node_type* result  = nullptr;

			while (current)
			{
				const value_type& val = static_cast<const node_type*>(current)->m_Val;

				if (!this->m_Comp(val, key))
				{
					// val >= key
					result = current;
					current = current->mp_Left;
				}
				else
				{
					current = current->mp_Right;
				}
			}

			return result;
		}

		// first element > key
		base_node_type* upper_bound_node(const key_type& key) const {

			base_node_type* current = this->mp_Root;
			base_node_type* result  = nullptr;

			while (current) {

				const value_type& val = static_cast<const node_type*>(current)->m_Val;
				
				if (this->m_Comp(key, val)) 
				{
					// val > key
					result = current;
					current = current->mp_Left;
				}
				else 
				{
					current = current->mp_Right;
				}
			}

			return result;
		}

		template<typename U>
		std::pair<base_node_type*, bool> insert_impl(U&& v)
		{
			base_node_type* parent  = nullptr;
			base_node_type* current = this->mp_Root;

			while (current)
			{
				parent = current;

				const value_type& val = static_cast<const node_type*>(current)->m_Val;

				// using comp, find a place
				// where to insert the new node
				if (this->m_Comp(v, val))
				{
					current = current->mp_Left;
				}
				else if (this->m_Comp(val, v))
				{
					current = current->mp_Right;
				}
				else
				{
					// it's a duplicate
					// return 
					return { current, false };
				}
			}

			node_type* n = create_node(std::forward<U>(v), parent);

			if (!parent)
			{
				this->mp_Root = n;
			}
			else if (this->m_Comp(n->m_Val, static_cast<node_type*>(parent)->m_Val))
			{
				parent->mp_Left = n;
			}
			else 
			{
				parent->mp_Right = n;
			}

			++this->m_Size; 
			return { n, true };
		}

		template<class U> 
		node_type* create_node(U&& v, base_node_type* parent) 
		{ 
			node_type* n = this->DoAllocateNode();

			try { 

				node_traits::construct(this->m_NodeAlloc, n, std::forward<U>(v));
				n->mp_Parent = parent; 
				n->mp_Left = n->mp_Right = nullptr; 
			} 
			catch (...) 
			{ 
				this->DoDeallocateNode(n); 
				throw; 
			} 
			
			return n; 
		}

		/// replace node a with b 

		void transplant(base_node_type* a, base_node_type* b)
		{
			//assert(a && b && "[bst_transplant]: nodes must be valid");

			if (!a->mp_Parent)
			{
				// a was the root
				this->mp_Root = static_cast<node_type*>(b);
			}
			else if (a == a->mp_Parent->mp_Left)
			{
				// a was a left child
				a->mp_Parent->mp_Left = b;
			}
			else
			{
				// a was a right child
				a->mp_Parent->mp_Right = b;
			}

			// update parent

			if(b) b->mp_Parent = a->mp_Parent;
		}

		/// We have 3 cases on deletion:
		/// 1. no children -> delete it
		/// 2. has 1 child -> replace parent with child
		/// 3. has 2 children -> replace with the successor

		void erase_node(base_node_type* z)
		{
			// case 1,2
			if (!z->mp_Left)
			{
				transplant(z, z->mp_Right); // if nullptr ok
			}
			else if (!z->mp_Right)
			{
				transplant(z, z->mp_Left);
			}
			else
			{
				// case 3

				// successor is a right child if is a 
				// child of z, otherwise it is a left
				// child

				base_node_type* s = successor(z);

				if (s->mp_Parent != z)
				{
					// if s isn't a direct child
					// replace s with its right child
					// successor can't have left child
					// otherwise it wouldn't be the successor
					transplant(s, s->mp_Right);

					// transfer z right subtree to s right subtree
					// s doesn't have left child for now 
					// so make s right child, z right child
					s->mp_Right = z->mp_Right;

					assert(s->mp_Right && "[bst_erase_node]: case 2 children, right subtree of erased must be valid since successor isn't direct child");
					
					s->mp_Right->mp_Parent = s;
				}

				// now substitute z with successor
				// and attach z left subtree to s
				transplant(z, s);

				s->mp_Left = z->mp_Left;
				if (s->mp_Left)
					s->mp_Left->mp_Parent = s;
			}

			this->DoDestroyNode(static_cast<node_type*>(z));
			--this->m_Size;
		}

		// Utility

		void inorder_print_rec(base_node_type* node, base_node_type* parent, const char* relation) const noexcept {
			
			if (!node) return;

			inorder_print_rec(node->mp_Left, node, "left");

			const node_type* n = static_cast<const node_type*>(node);
			const node_type* p = static_cast<const node_type*>(parent);
			const node_type* l = static_cast<const node_type*>(node->mp_Left);
			const node_type* r = static_cast<const node_type*>(node->mp_Right);

			std::cout << "Node: " << n->m_Val
				<< " (" << relation << ")\n";

			if (p)
				std::cout << "  Parent: " << p->m_Val << "\n";
			else
				std::cout << "  Parent: nullptr\n";

			if (l)
				std::cout << "  Left: " << l->m_Val << "\n";
			else
				std::cout << "  Left: nullptr\n";

			if (r)
				std::cout << "  Right: " << r->m_Val << "\n";
			else
				std::cout << "  Right: nullptr\n";

			std::cout << "----------------------------\n";

			inorder_print_rec(node->mp_Right, node, "right");
		}
	};

	// binary operators

	template<typename T, typename compare, typename A>
	bool operator==(const bst<T, compare, A>& a, const bst<T, compare, A>& b) {
		if (a.size() != b.size()) return false;
		return std::equal(a.begin(), a.end(), b.begin(), b.end());
	}

	template<typename T, typename compare, typename A>
	bool operator!=(const bst<T, compare, A>& a, const bst<T, compare, A>& b) {
		return !(a == b);
	}

	// utility
	template<typename T, typename compare, typename A>
	void swap(bst<T, compare, A>& a, bst<T, compare, A>& b) noexcept {
		a.swap(b);
	}

}

#endif // !BINARY_SEARCH_TREE_H