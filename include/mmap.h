#ifndef MSTL_MAP_H
#define MSTL_MAP_H

#include "internals/red_black_tree.h"

namespace mstl {

	/// ---------------------------------------------------------------
	/// Map
	/// ---------------------------------------------------------------

	template<
		typename Key,
		typename T,
		typename Compare = std::less<Key>,
		typename Alloc = std::allocator<std::pair<const Key, T>>
	>
	class map {

	public:
		using key_type = Key;
		using mapped_type = T;
		using value_type = std::pair<const Key, T>;
		using key_compare = Compare;
		using allocator_type = Alloc;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

	private:

		using tree_type = rb_tree<
			value_type,
			rb_node,
			first_key<value_type>,
			key_compare,
			allocator_type
		>;

		tree_type m_Tree;

	public:

		using iterator       = typename tree_type::iterator;
		using const_iterator = typename tree_type::const_iterator;

		// ================= Constructors =================
		map() = default;

		explicit map(const key_compare& comp,
			const allocator_type& alloc = allocator_type{})
			: m_Tree(alloc, comp) {
		}

		template<class InputIt>
		map(InputIt first, InputIt last,
			const key_compare& comp = key_compare{},
			const allocator_type& alloc = allocator_type{})
			: m_Tree(first, last, alloc, comp) {
		}

		map(std::initializer_list<value_type> il,
			const key_compare& comp = key_compare{},
			const allocator_type& alloc = allocator_type{})
			: m_Tree(il, alloc, comp) {
		}

		// ================= Iterators =================

		iterator begin() noexcept { return m_Tree.begin(); }
		const_iterator begin() const noexcept { return m_Tree.begin(); }
		const_iterator cbegin() const noexcept { return m_Tree.begin(); }

		iterator end() noexcept { return m_Tree.end(); }
		const_iterator end() const noexcept { return m_Tree.end(); }
		const_iterator cend() const noexcept { return m_Tree.end(); }

		// ================= Capacity =================

		bool empty() const noexcept { return m_Tree.empty(); }
		size_type size() const noexcept { return m_Tree.size(); }

		// ================= Modifiers =================

		void clear() noexcept { m_Tree.clear(); }

		std::pair<iterator, bool> insert(const value_type& val)
		{
			return m_Tree.insert(val);
		}

		std::pair<iterator, bool> insert(value_type&& val)
		{
			return m_Tree.insert(std::move(val));
		}

		template<class... Args>
		std::pair<iterator, bool> emplace(Args&&... args)
		{
			return m_Tree.emplace(std::forward<Args>(args)...);
		}

		void erase(iterator pos) { m_Tree.erase(pos); }
		size_type erase(const key_type& key) { return m_Tree.erase(key); }

		void swap(map& other) noexcept { m_Tree.swap(other.m_Tree); }

		// ================= Element access =================

		T& operator[](const Key& key)
		{
			auto [it, inserted] = m_Tree.insert(std::make_pair(key, T{}));
			return (*it).second;
		}

		T& operator[](Key&& key)
		{
			auto [it, inserted] = m_Tree.insert(std::make_pair(std::move(key), T{}));
			return (*it).second;
		}

		T& at(const Key& key)
		{
			auto it = find(key);
			if (it == end()) throw std::out_of_range("mstl::map::at: key not found");
			return (*it).second;
		}

		const T& at(const Key& key) const
		{
			auto it = find(key);
			if (it == end()) throw std::out_of_range("mstl::map::at: key not found");
			return (*it).second;
		}

		// ================= Lookup =================

		iterator find(const Key& key) {
			return m_Tree.find(key);
		}

		const_iterator find(const Key& key) const {
			return m_Tree.find(key);
		}

		size_type count(const Key& key) const
		{
			return find(key) == end() ? 0 : 1;
		}

		// ================= Observers =================

		key_compare key_comp() const { return m_Tree.m_Comp; }

		allocator_type get_allocator() const { return m_Tree.get_allocator(); }

		// ================= Debug =================

		void inorder_print() const noexcept { m_Tree.inorder_print(); }
		bool verify() const noexcept { return m_Tree.IsRBTree(); }
	};
}

#endif // ! MSTL_MAP_H