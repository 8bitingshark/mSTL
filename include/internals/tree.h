#ifndef MSTL_TREE_H
#define MSTL_TREE_H

#include <type_traits>    // std::remove_cvref_t, std::declval
#include <functional> 
#include <memory>
#include <utility>
#include <iterator>
#include <cstddef> 

namespace mstl {

	/// ---------------------------------------------------------------
	/// base node
	/// ---------------------------------------------------------------
	/// it allows non-templated operations
	/// not waste of space for T value

	struct node_base {

		node_base* mp_Left{};
		node_base* mp_Right{};
		node_base* mp_Parent{};
	};

	/// ---------------------------------------------------------------
	/// templated node
	/// ---------------------------------------------------------------

	template<typename T>
	struct node : public node_base {

		using value_type = T;
		T m_Val{};

		explicit node(const T& v) : node_base{}, m_Val(v) {}
		explicit node(T&& v) : node_base{}, m_Val(std::move(v)) {}

		template<class... Args>
		explicit node(std::in_place_t, Args&&... args)
			: node_base{}
			, m_Val(std::forward<Args>(args)...) {
		}
	};

	/// ---------------------------------------------------------------
	/// Key extractors
	/// ---------------------------------------------------------------

	// used for set
	template<typename T>
	struct identity_key {
		constexpr const T& operator()(const T& v) const noexcept { return v; }
	};

	// used for map
	template<typename Pair>
	struct first_key {
		constexpr const auto& operator()(const Pair& p) const noexcept { return p.first; }
	};

	/// ---------------------------------------------------------------
	/// Tree global functions
	/// ---------------------------------------------------------------

	inline node_base* TreeMin(node_base* node) noexcept {
		
		while (node && node->mp_Left)
		{
			node = node->mp_Left;
		}
		return node;
	}

	inline node_base* TreeMax(node_base* node) noexcept {

		while (node && node->mp_Right)
		{
			node = node->mp_Right;
		}
		return node;
	}

	inline node_base* TreeSuccessor(node_base* n) noexcept {

		if (!n) return nullptr;

		// Case 1: there is a right sub-tree: find the minimum of this right sub-tree
		if (n->mp_Right)
		{
			return TreeMin(n->mp_Right);
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

	inline node_base* TreePredecessor(node_base* n) noexcept {

		if (!n) return nullptr;

		if (n->mp_Left)
		{
			return TreeMax(n->mp_Left);
		}

		auto* p = n->mp_Parent;

		while (p && n == p->mp_Left) 
		{
			n = p;
			p = p->mp_Parent;
		}

		return p;
	}

	/// Compare passed by-value because
	/// usually stateless, avoid lifetime issues
	/// good for inlining and compiler optimizations

	template <typename NodeT, typename KeyOfValue, typename Compare, typename Key>
	inline node_base* TreeFind(node_base* root, const Key& i_key, KeyOfValue key_of_value, Compare comp) noexcept
	{
		while (root) 
		{
			const auto& val = static_cast<const NodeT*>(root)->m_Val; // could be a pair
			const auto& ext_key = key_of_value(val);

			if (comp(i_key, ext_key))
			{
				root = root->mp_Left;
			}	
			else if (comp(ext_key, i_key))
			{
				root = root->mp_Right;
			}	
			else
			{
				return root;
			}
				
		}

		return nullptr;
	}

	template <typename NodeT, typename KeyOfValue, typename Compare, typename Key>
	inline node_base* TreeLowerBound(node_base* root, const Key& i_key, KeyOfValue key_of_value, Compare comp) noexcept
	{
		node_base* res = nullptr;

		while (root) 
		{
			const auto& val = static_cast<const NodeT*>(root)->m_Val;
			const auto& ext_key = key_of_value(val);

			if (!comp(ext_key, i_key))
			{
				res = root;
				root = root->mp_Left;
			}
			else 
			{
				root = root->mp_Right;
			}
		}
		return res;
	}

	template <typename NodeT, typename KeyOfValue, typename Compare, typename Key>
	inline node_base* TreeUpperBound(node_base* root, const Key& i_key, KeyOfValue key_of_value, Compare comp) noexcept
	{
		node_base* res = nullptr;

		while (root) 
		{
			const auto& val = static_cast<const NodeT*>(root)->m_Val;
			const auto& ext_key = key_of_value(val);

			if (comp(i_key, ext_key))
			{
				res = root;
				root = root->mp_Left;
			}
			else 
			{
				root = root->mp_Right;
			}
		}
		return res;
	}
	
	/// ---------------------------------------------------------------
	/// Tree Iterator
	/// ---------------------------------------------------------------
	/// In-order bidirectional iterator over base nodes

	template<typename node_t, bool IsConst>
	class tree_iterator {

		using base_node_type = node_base;
		using node_type = node_t;

		base_node_type* curr{};

	public:

		using iterator_category = std::bidirectional_iterator_tag;
		using value_type        = typename node_t::value_type;
		using difference_type   = std::ptrdiff_t;
		using reference         = std::conditional_t<IsConst, const value_type&, value_type&>;
		using pointer           = std::conditional_t<IsConst, const value_type*, value_type*>;

		// required for some algorithms
		tree_iterator() = default;

		explicit tree_iterator(base_node_type* n) : curr(n) {}

		/// Constructor for const conversion
		/// 
		/// std::enable_if_t<C> is defined only when C is true
		/// if C is false the compiler removes this overload (SFINAE)
		/// because enable_if is ill-formed, so constructor is ignored
		/// 
		/// This constructor exists only for const iterators

		template<bool C = IsConst, typename = std::enable_if_t<C>>
		tree_iterator(const tree_iterator<node_t, false>& other)
			: curr{ other.curr } {
		}

		reference operator*()  const { return static_cast<node_type*>(curr)->m_Val; }
		pointer   operator->() const { return std::addressof(static_cast<node_type*>(curr)->m_Val); }

		friend bool operator==(const tree_iterator& a, const tree_iterator& b) { return a.curr == b.curr; }
		friend bool operator!=(const tree_iterator& a, const tree_iterator& b) { return !(a == b); }

		tree_iterator& operator++() noexcept {
			curr = mstl::TreeSuccessor(curr);
			return *this;
		}

		tree_iterator& operator++(int) noexcept {

			tree_iterator tmp = *this;
			++(*this);
			return tmp;
		}

		tree_iterator& operator--() noexcept {
			curr = mstl::TreePredecessor(curr);
			return *this;
		}

		tree_iterator operator--(int) noexcept {
			tree_iterator tmp = *this;
			--(*this);
			return tmp;
		}

	private:

		// friend class
		// to access curr node from tree
		template<typename T, template<class> class NodeT, typename KeyOfValue, typename Compare, typename A>
		friend class tree_base;

		// all specializations 
		// can access each other curr
		template<typename, bool>
		friend class tree_iterator;
	};

	/// ---------------------------------------------------------------
	/// Tree base
	/// ---------------------------------------------------------------
	/// Handles allocation, comparison, node destruction.
	/// It is designed to be inherited by specialized trees
	/// such as bst_tree, avl_tree, rb_tree, map, multimap, etc.
	/// ---------------------------------------------------------------

	template<
		typename T,
		template<class> class NodeT = node,
		typename KeyOfValue = identity_key<T>,
		typename compare = std::less<
			std::remove_cvref_t<decltype(std::declval<KeyOfValue>()(std::declval<const T&>()))>>,
		typename A = std::allocator<T>
		>
	class tree_base {

	public:

		using value_type      = typename NodeT<T>::value_type; // supports pair<const K,V>
		using key_type        = std::remove_cvref_t<decltype(std::declval<KeyOfValue>()(std::declval<const value_type&>()))>;
		using key_compare     = compare;
		using value_compare   = key_compare;
		using alloc_type      = A;
		using alloc_traits    = std::allocator_traits<A>;
		using size_type       = typename alloc_traits::size_type;
		using difference_type = std::ptrdiff_t;

		using node_type   = NodeT<value_type>;
		using node_alloc  = typename alloc_traits::template rebind_alloc<node_type>;
		using node_traits = std::allocator_traits<node_alloc>;

		using iterator       = tree_iterator<node_type, false>;
		using const_iterator = tree_iterator<node_type, true>;


		// ============== Ctors =================

		tree_base()
			: m_ValueAlloc{ alloc_type{} }
			, m_NodeAlloc{ m_ValueAlloc }
			, m_Comp{ key_compare{} }
			, m_Size{ 0 }
			, mp_Root{ nullptr } {
		}

		explicit tree_base(const alloc_type& i_alloc)
			: m_ValueAlloc{ i_alloc }
			, m_NodeAlloc{ m_ValueAlloc }
			, m_Comp{ key_compare{} }
			, m_Size{ 0 }
			, mp_Root{ nullptr } {
		}

		explicit tree_base(const alloc_type& i_alloc, const key_compare& i_comp)
			: m_ValueAlloc{ i_alloc }
			, m_NodeAlloc{ m_ValueAlloc }
			, m_Comp{ i_comp }
			, m_Size{ 0 }
			, mp_Root{ nullptr } {
		}

		// === Destructor ===

		~tree_base() { DoClear(); }

		// ============== Iterators =================

		iterator begin() noexcept { return iterator{ mstl::TreeMin(mp_Root) }; }
		const_iterator begin() const noexcept { return const_iterator{ mstl::TreeMin(mp_Root) }; }
		const_iterator cbegin() const noexcept { return const_iterator{ mstl::TreeMin(mp_Root) }; }

		iterator end() noexcept { return iterator{ nullptr }; }
		const_iterator end() const noexcept { return const_iterator{ nullptr }; }
		const_iterator cend() const noexcept { return const_iterator{ nullptr }; }

		// ============== Lookups =================

		iterator find(const key_type& key) noexcept {
			return iterator{ mstl::TreeFind<node_type>(mp_Root, key, m_KeyExtractor, m_Comp) };
		}

		const_iterator find(const key_type& key) const noexcept {
			return const_iterator{ mstl::TreeFind<node_type>(mp_Root, key, m_KeyExtractor, m_Comp) };
		}

		iterator lower_bound(const key_type& key) noexcept {
			return iterator{ mstl::TreeLowerBound<node_type>(mp_Root, key, m_KeyExtractor, m_Comp) };
		}

		const_iterator lower_bound(const key_type& key) const noexcept {
			return const_iterator{ mstl::TreeLowerBound<node_type>(mp_Root, key, m_KeyExtractor, m_Comp) };
		}

		iterator upper_bound(const key_type& key) noexcept {
			return iterator{ mstl::TreeUpperBound<node_type>(mp_Root, key, m_KeyExtractor, m_Comp) };
		}

		const_iterator upper_bound(const key_type& key) const noexcept {
			return const_iterator{ mstl::TreeUpperBound<node_type>(mp_Root, key, m_KeyExtractor, m_Comp) };
		}

		bool contains(const key_type& key) const noexcept {
			return mstl::TreeFind<node_type>(mp_Root, key, m_KeyExtractor, m_Comp) != nullptr;
		}

		std::pair<iterator, iterator> equal_range(const key_type& key) noexcept {
			return { lower_bound(key), upper_bound(key) };
		}

		std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const noexcept {
			return { lower_bound(key), upper_bound(key) };
		}

	protected:

		using base_node_type = node_base;

		[[no_unique_address]] alloc_type  m_ValueAlloc{};
		[[no_unique_address]] node_alloc  m_NodeAlloc{ m_ValueAlloc };
		[[no_unique_address]] key_compare m_Comp{};
		[[no_unique_address]] KeyOfValue  m_KeyExtractor{};

		size_type  m_Size{};
		node_type* mp_Root{};

		// ================= Alloc/Dealloc =================

		node_type* DoAllocateNode() {
			return node_traits::allocate(m_NodeAlloc, 1);
		}

		void DoDeallocateNode(node_type* p) noexcept {
			node_traits::deallocate(m_NodeAlloc, p, 1);
		}

		// ================ Cleanup =================

		void DoDestroyNode(node_type* p) noexcept {
			node_traits::destroy(m_NodeAlloc, p);
			DoDeallocateNode(p);
		}

		// Recursively clear tree nodes
		void DoClear() noexcept {
			ClearRec(mp_Root);
			mp_Root = nullptr;
			m_Size = 0;
		}

	private:

		void ClearRec(node_base* n) noexcept {
			if (!n) return;

			ClearRec(n->mp_Left);
			ClearRec(n->mp_Right);

			this->DoDestroyNode(static_cast<node_type*>(n));
		}

		template<typename U, template<class> class N, typename K, typename C, typename A>
		friend void swap(tree_base<U, N, K, C, A>&, tree_base<U, N, K, C, A>&) noexcept;
	};

	// swap for tree_base

	template<typename U, template<class> class N, typename K, typename C, typename A>
	void swap(tree_base<U, N, K, C, A>& a, tree_base<U, N, K, C, A>& b) noexcept {
		
		using std::swap;
		swap(a.m_ValueAlloc, b.m_ValueAlloc);
		swap(a.m_NodeAlloc, b.m_NodeAlloc);
		swap(a.m_Comp, b.m_Comp);
		swap(a.m_KeyExtractor, b.m_KeyExtractor);
		swap(a.m_Size, b.m_Size);
		swap(a.mp_Root, b.mp_Root);
	}
}

#endif // !MSTL_TREE_H