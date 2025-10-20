#ifndef MSTL_TREE_H
#define MSTL_TREE_H

#include <memory>

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
		using value_type = typename node_t::value_type;
		using difference_type = std::ptrdiff_t;
		using reference = std::conditional_t<IsConst, const value_type&, value_type&>;
		using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;

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
		tree_iterator(const tree_iterator<value_type, false>& other)
			: curr{ other.curr } {
		}

		reference operator*()  const { return static_cast<node_type*>(curr)->m_Val; }
		pointer   operator->() const { return std::addressof(static_cast<node_type*>(curr)->m_Val); }

		friend bool operator==(const tree_iterator& a, const tree_iterator& b) { return a.curr == b.curr; }
		friend bool operator!=(const tree_iterator& a, const tree_iterator& b) { return !(a == b); }

		tree_iterator& operator++() noexcept {
			curr = successor(curr);
			return *this;
		}

		tree_iterator& operator++(int) noexcept {

			tree_iterator tmp = *this;
			++(*this);
			return tmp;
		}

		tree_iterator& operator--() noexcept {
			curr = predecessor(curr);
			return *this;
		}

		tree_iterator operator--(int) noexcept {
			tree_iterator tmp = *this;
			--(*this);
			return tmp;
		}

	private:

		// === Helpers ===
		// they are static because they are indipendent from an object
		// if you don't mark static the compiler WILL generate
		// this pointer and it is a waste

		static base_node_type* min_node(base_node_type* n) noexcept
		{
			// find the leaf
			while (n && n->mp_Left)
			{
				n = n->mp_Left;
			}

			return n;
		}

		static base_node_type* max_node(base_node_type* n) noexcept
		{
			// find the leaf
			while (n && n->mp_Right)
			{
				n = n->mp_Right;
			}

			return n;
		}

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

		// friend class
		// to access curr node from tree
		template<typename T, typename Compare, typename Alloc, template<class> class node_t>
		friend class tree_base;
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
		typename compare = std::less<T>, 
		typename A = std::allocator<T>, 
		template<class> class node_t = node
	>
	class tree_base {

	public:

		using value_type = typename node_t<T>::value_type; // supports pair<const K,V>
		using alloc_type = A;
		using alloc_traits = std::allocator_traits<A>;
		using size_type = typename alloc_traits::size_type;

		using node_type = node_t<value_type>;
		using node_alloc = typename alloc_traits::template rebind_alloc<node_type>;
		using node_traits = std::allocator_traits<node_alloc>;

		// === Constructors ===

		tree_base()
			: m_ValueAlloc{ alloc_type{} }
			, m_NodeAlloc{ m_ValueAlloc }
			, m_Comp{ compare{} }
			, m_Size{ 0 }
			, mp_Root{ nullptr } {
		}

		explicit tree_base(const alloc_type& i_alloc)
			: m_ValueAlloc{ i_alloc }
			, m_NodeAlloc{ m_ValueAlloc }
			, m_Comp{ compare{} }
			, m_Size{ 0 }
			, mp_Root{ nullptr } {
		}

		explicit tree_base(const alloc_type& i_alloc, const compare& i_comp)
			: m_ValueAlloc{ i_alloc }
			, m_NodeAlloc{ m_ValueAlloc }
			, m_Comp{ i_comp }
			, m_Size{ 0 }
			, mp_Root{ nullptr } {
		}

		// === Destructor ===

		~tree_base() { DoClear(); }

	protected:

		[[no_unique_address]] alloc_type m_ValueAlloc{};
		[[no_unique_address]] node_alloc m_NodeAlloc{ m_ValueAlloc };
		[[no_unique_address]] compare m_Comp{};

		size_type  m_Size{};
		node_type* mp_Root{ nullptr };

		// helpers

		node_type* DoAllocateNode() {
			return node_traits::allocate(m_NodeAlloc, 1);
		}

		void DoDeallocateNode(node_type* p) noexcept {
			node_traits::deallocate(m_NodeAlloc, p, 1);
		}

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

		template<typename U, typename C, typename A, template<class> class N>

		friend void swap(tree_base<U, C, A, N>&, tree_base<U, C, A, N>&) noexcept;
	};

	// swap for tree_base

	template<typename U, typename C, typename A, template<class> class N>
	void swap(tree_base<U, C, A, N>& a , tree_base<U, C, A, N>& b) noexcept {
		using std::swap;
		swap(a.m_ValueAlloc, b.m_ValueAlloc);
		swap(a.m_NodeAlloc, b.m_NodeAlloc);
		swap(a.m_Comp, b.m_Comp);
		swap(a.m_Size, b.m_Size);
		swap(a.mp_Root, b.mp_Root);
	}

}

#endif // !MSTL_TREE_H