#ifndef MSTL_TREE_H
#define MSTL_TREE_H

#include "concepts_utils.h"
#include <memory>

namespace mstl {

	/// ---------------------------------------------------------------
	/// base node
	/// ---------------------------------------------------------------
	/// it allows non-templated operations
	/// not waste of space for T value

	struct node_base {

		node_base* mp_Left{ nullptr };
		node_base* mp_Right{ nullptr };
		node_base* mp_Parent{ nullptr };
	};

	/// ---------------------------------------------------------------
	/// templated node
	/// ---------------------------------------------------------------
	/// [?]: why T&& constructor

	template<typename T>
		requires Element<T>
	struct node : public node_base {

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
	/// Tree base
	/// ---------------------------------------------------------------
	/// memory management, compare and node clear when destructed

	template<typename T, typename compare = std::less<T>, typename A = std::allocator<T>>
		requires Element<T>
	class tree_base {

	public:

		using value_type = T;
		using alloc_type = A;
		using alloc_traits = std::allocator_traits<A>;
		using size_type = typename alloc_traits::size_type;

		using node_type = node<value_type>;
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

		template<typename U, typename C, typename A>
		friend void swap(tree_base<U, C, A>&, tree_base<U, C, A>&) noexcept;
	};

	// swap for tree_base

	template<typename T, typename C, typename A>
	void swap(tree_base<T, C, A>& a, tree_base<T, C, A>& b) noexcept {
		using std::swap;
		swap(a.m_ValueAlloc, b.m_ValueAlloc);
		swap(a.m_NodeAlloc, b.m_NodeAlloc);
		swap(a.m_Comp, b.m_Comp);
		swap(a.m_Size, b.m_Size);
		swap(a.mp_Root, b.mp_Root);
	}

}

#endif // !MSTL_TREE_H