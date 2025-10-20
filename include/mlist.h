#ifndef MSTL_LIST_H
#define MSTL_LIST_H

#include <memory>
#include <vector>
#include <iterator>
#include <string>
#include <iostream>
#include <initializer_list>
#include "concepts_utils.h"

namespace mstl {

	// linkbase: node of a list

	struct linkbase {

		linkbase* prev;
		linkbase* succ;
	};

	// link<T> for the list

	template<typename T>
		requires Element<T>
	struct link : public linkbase {

		T m_Val;

		template<class... Args>
		explicit link(Args&&... args)
			: linkbase{}, m_Val(std::forward<Args>(args)...) {   // perfect forwarding
		}
	};

	/// list_rep handles node allocation, size and
	/// linkMaster with RAII principle. 
	/// In its destructor it destroy also
	/// the links still alive. 

	template<typename T, typename A = std::allocator<T>>
		requires Element<T>
	class list_rep {

	public:
		using value_type = T;
		using alloc_type = A;
		using link_type = link<T>;
		using alloc_traits = std::allocator_traits<A>;
		using size_type = typename alloc_traits::size_type;
		using difference_type = typename alloc_traits::difference_type;
		using link_alloc = typename alloc_traits::template rebind_alloc<link_type>;
		using link_traits = std::allocator_traits<link_alloc>;

		// === Constructors ===

		list_rep()
			: m_Alloc{ alloc_type{} }
			, m_LinkAlloc{ m_Alloc }
			, m_Size{ 0 }
			, m_LinkMaster{}
		{
			InitMaster();
		}

		explicit list_rep(const alloc_type& a)
			: m_Alloc{ a }
			, m_LinkAlloc{ a }
			, m_LinkMaster{}
			, m_Size{ 0 }
		{
			InitMaster();
		}

		// === Destructor ===

		~list_rep() {
			DoClear();
		}

	protected:

		[[no_unique_address]] alloc_type m_Alloc{};
		[[no_unique_address]] link_alloc m_LinkAlloc{ m_Alloc };
		linkbase m_LinkMaster{};
		size_type m_Size{};

		// Initialize LinkMaster
		void InitMaster() noexcept {
			m_LinkMaster.succ = &m_LinkMaster;
			m_LinkMaster.prev = &m_LinkMaster;
		}

		// For safety destroy nodes and then release memory
		void DoClear() noexcept {

			linkbase* p = m_LinkMaster.succ;

			while (p != &m_LinkMaster) {

				link_type* const pTemp = static_cast<link_type*>(p);
				p = p->succ;
				link_traits::destroy(m_LinkAlloc, pTemp);  // pTemp->~link_type();
				DoDeallocateNode(pTemp);
			}

			InitMaster();
			m_Size = 0;
		}

		link_type* DoAllocateNode() {
			return link_traits::allocate(m_LinkAlloc, 1);
		}

		void DoDeallocateNode(link_type* p) noexcept {
			link_traits::deallocate(m_LinkAlloc, p, 1);
		}
	};


	/// Lists are sequence containers that allow constant 
	/// time insert and erase operations anywhere
	///	within the sequence, and iteration in both directions.
	/// 
	/// List containers are implemented as doubly linked lists; 
	/// Doubly linked lists can store each of the elements they 
	/// contain in different and unrelated storage locations.
	/// The ordering is kept internally by the association to each 
	/// element of a link to the element preceding it and 
	/// a link to the element following it.
	/// 
	/// ADVANTAGES:
	/// Compared to other base standard sequence containers 
	/// (array, vector and deque ), lists perform 
	/// generally better in 
	///		- inserting, 
	///		- extracting
	///		- moving elements 
	/// in ANY position within the container for 
	/// which an iterator has already been obtained.
	/// 
	/// benefits also for algorithms that make intensive use
	/// of these, like sorting algorithms.
	/// 
	/// DRAWBACKS:
	/// 
	/// that they lack direct access to the elements by
	/// their position; For example, to access the sixth element. O(N)
	/// 
	/// They also consume some extra memory to keep
	/// the linking information associated to each element
	/// (which may be an important factor for large lists of
	/// small sized elements).

	template<typename T, typename A = std::allocator<T>>
		requires Element<T>
	class list : public list_rep<T, A> {

		using base_type = list_rep<T, A>;
		using link_type = typename base_type::link_type;
		using link_base = typename linkbase;
		using size_type = typename base_type::size_type;
		using link_traits = typename base_type::link_traits;

	public:

		using value_type = T;
		using alloc_type = A;
		using reference = value_type&;
		using const_reference = const value_type&;

		// Iterator

		class iterator {

		private:

			link_base* curr{};

		public:

			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using pointer = T*;
			using reference = T&;

			iterator() { curr = nullptr; } // costruttore di default costruisce, inizializza il contenuto al suo default value se esiste
			explicit iterator(link_base* p) : curr{ p } {}
			link_base* base() noexcept { return curr; }
			reference operator*() const { return static_cast<link_type*>(curr)->m_Val; }
			pointer operator->() const { return std::addressof(static_cast<link_type*>(curr)->m_Val); }
			iterator& operator++() { curr = curr->succ; return *this; }
			iterator operator++(int) { auto tmp = *this; ++*this; return tmp; }
			iterator& operator--() { curr = curr->prev; return *this; }
			iterator operator--(int) { auto tmp = *this; --*this; return tmp; }
			bool operator==(const iterator& other_it) const noexcept { return this->curr == other_it.curr; }
			bool operator!=(const iterator& other_it) const noexcept { return !(*this == other_it); }
		};

		// Const Iterator

		class const_iterator {

		private:

			const link_base* curr{};

		public:

			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using pointer = const T*;
			using reference = const T&;

			const_iterator() { curr = nullptr; }
			explicit const_iterator(const link_base* p) : curr{ p } {}
			const link_base* base() const noexcept { return curr; }

			reference operator*() const { return static_cast<const link_type*>(curr)->m_Val; }
			pointer operator->() const { return std::addressof(static_cast<const link_type*>(curr)->m_Val); }

			const_iterator& operator++() { curr = curr->succ; return *this; }
			const_iterator operator++(int) { auto tmp = *this; ++*this; return tmp; }
			const_iterator& operator--() { curr = curr->prev; return *this; }
			const_iterator operator--(int) { auto tmp = *this; --*this; return tmp; }
			bool operator==(const const_iterator& other_it) const noexcept { return this->curr == other_it.curr; }
			bool operator!=(const const_iterator& other_it) const noexcept { return !(*this == other_it); }
		};

		iterator begin() noexcept { return iterator{ this->m_LinkMaster.succ }; }
		const_iterator begin() const noexcept { return const_iterator{ this->m_LinkMaster.succ }; }
		iterator end() noexcept { return iterator{ &this->m_LinkMaster }; }
		const_iterator end() const noexcept { return const_iterator{ &this->m_LinkMaster }; }

		//
		// ================= Access =================
		//

		reference front() { return static_cast<link_type*>(this->m_LinkMaster.succ)->m_Val; }
		const_reference front() const { return static_cast<const link_type*>(this->m_LinkMaster.succ)->m_Val; }
		reference back() { return static_cast<link_type*>(this->m_LinkMaster.prev)->m_Val; }
		const_reference back() const { return static_cast<const link_type*>(this->m_LinkMaster.prev)->m_Val; }

		bool empty() const noexcept { return this->m_Size == 0; }
		size_type size() const noexcept { return this->m_Size; }

		//
		// ================= Modifiers =================
		//

		void push_back(const value_type& v) {
			insert(end(), v);
		}

		void push_front(const T& v) {
			insert(begin(), v);
		}

		void pop_back() {
			erase(--end());
		}

		void pop_front() {
			erase(begin());
		}

		iterator insert(iterator pos, const value_type& v) {

			auto* node = create_node(v);

			link_before(pos.base(), node);

			++(this->m_Size);

			return iterator{ node };
		}

		iterator insert(iterator pos, value_type&& v) {

			auto* node = create_node(std::move(v));

			link_before(pos.base(), node);

			++(this->m_Size);

			return iterator{ node };
		}

		iterator erase(iterator pos)
		{
			link_base* const p = pos.base();

			if (pos.base() == &this->m_LinkMaster)
			{
				return pos;
			}

			link_base* const next = p->succ;

			unlink(p);

			destroy_node(static_cast<link_type*>(p));

			--(this->m_Size);

			return iterator{ next };
		}

	private:

		//
		// ================= Helpers =================
		//
		template<class... Args>
		link_type* create_node(Args&&... args) {

			link_type* n = this->DoAllocateNode();
			try {
				link_traits::construct(this->m_LinkAlloc, n, std::forward<Args>(args)...);
			}
			catch (...) {
				this->DoDeallocateNode(n);
				throw;
			}
			return n;
		}

		void destroy_node(link_type* n) noexcept {

			link_traits::destroy(this->m_LinkAlloc, n);
			this->DoDeallocateNode(n);
		}

		static void link_before(link_base* pos, link_base* n) noexcept {
			n->succ = pos;
			n->prev = pos->prev;
			pos->prev->succ = n;
			pos->prev = n;
		}

		static void unlink(link_base* n) noexcept {
			n->prev->succ = n->succ;
			n->succ->prev = n->prev;
		}
	};


	template <typename T>
	void print_list(const list<T>& lst, const std::string& name)
	{
		std::cout << "=== " << name << " ===\n";
		std::cout << "size = " << lst.size() << "\n";

		// Simplify visualization
		std::cout << "[ ";
		for (auto it = lst.begin(); it != lst.end(); ++it)
			std::cout << *it << " ";
		std::cout << "]";
	}


	template <typename T>
	void visualize(const list<T>& lst)
	{
		std::cout << "\n[Sentinel]";
		for (auto it = lst.begin(); it != lst.end(); ++it)
			std::cout << " <-> [" << *it << "]";
		std::cout << " <-> [Sentinel]\n";
	}


}

#endif // !MSTL_LIST_H