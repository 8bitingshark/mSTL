#ifndef MSTL_AVL_TREE_H
#define MSTL_AVL_TREE_H

#include "tree.h"
#include "cassert"

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
		template<class> class NodeT = avl_node,
		typename KeyOfValue = identity_key<T>,
		typename compare = std::less<
		std::remove_cvref_t<decltype(std::declval<KeyOfValue>()(std::declval<const T&>()))>>,
		typename A = std::allocator<T>
		>
		class avl_tree : public tree_base<T, NodeT, KeyOfValue, compare, A> {
	
		using base_type = tree_base<T, NodeT, KeyOfValue, compare, A>;
		using node_type = typename base_type::node_type;
		using base_node_type = typename base_type::base_node_type;
		using node_alloc = typename base_type::node_alloc;
		using node_traits = typename base_type::node_traits;

		public:

			using key_type = typename base_type::key_type;
			using value_type = typename base_type::value_type;
			using key_compare = typename base_type::key_compare;
			using value_compare = typename base_type::value_compare;
			using size_type = typename base_type::size_type;
			using difference_type = typename base_type::difference_type;
			using alloc_type = typename base_type::alloc_type;

			using iterator = tree_iterator<node_type, false>;
			using const_iterator = tree_iterator<node_type, true>;

			// ================= Ctors =================

			// C++20: inherites constructors
			using base_type::base_type;

			explicit avl_tree(const alloc_type& a = alloc_type{}, const compare& c = compare{})
				: base_type(a, c) {
			}

			template<std::input_iterator It>
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

			// ============= Copy semantics =================

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

			// ============= Move semantics =================

			avl_tree(const avl_tree&& other) noexcept {

				swap(other);
			}

			avl_tree& operator=(avl_tree&& other) noexcept {

				if (this != &other) swap(other);
				return *this;
			}

			// ================= Capacity =================

			size_type size() const noexcept { return this->m_Size; }

			bool empty() const noexcept { return this->m_Size == 0; }

			// ================= Modifiers =================

			void clear() noexcept { this->DoClear(); }
	
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
			size_type erase(const key_type& key) {

				base_node_type* z = mstl::TreeFind<node_type>(this->mp_Root, key, this->m_KeyExtractor, this->m_Comp);
				if (!z) return 0;
				erase_node(z);
				return 1;
			}

			// erase by iterator -> returns successor
			iterator erase(iterator pos) {

				base_node_type* z = pos.curr;
				if (!z) return this->end();
				base_node_type* s = mstl::TreeSuccessor(z);
				erase_node(z);
				return iterator{ s };
			}

			void swap(avl_tree& other) noexcept {
				using std::swap;
				swap(this->m_ValueAlloc, other.m_ValueAlloc);
				swap(this->m_NodeAlloc, other.m_NodeAlloc);
				swap(this->m_Comp, other.m_Comp);
				swap(this->m_Size, other.m_Size);
				swap(this->mp_Root, other.mp_Root);
			}

			// ================= Observers =================

			const node_type* root() const noexcept { return this->mp_Root; }

			alloc_type get_allocator() const { return this->m_ValueAlloc; }

			// ================= Utility =================

			void inorder_print() const noexcept {
				inorder_print_rec(this->mp_Root, nullptr, "root");
			}

		private:

			// ================= Helpers =================

			template<typename U>
			std::pair<base_node_type*, bool> insert_impl(U&& v)
			{
				/*node_type* parent = nullptr;
				node_type* current = static_cast<node_type*>(this->mp_Root);*/

				base_node_type* parent = nullptr;
				base_node_type* current = this->mp_Root;

				// find insertion position

				while (current)
				{
					parent = current;
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
						return { current, false };
					}
				}

				// create node
				node_type* new_node = create_node(std::forward<U>(v), parent);

				// insert as child of parent
				if (!parent)
				{
					// update root
					this->mp_Root = new_node;
				}
				else if (this->m_Comp(v, static_cast<const node_type*>(parent)->m_Val))
				{
					parent->mp_Left = new_node;
				}
				else
				{
					parent->mp_Right = new_node;
				}

				++this->m_Size;

				// rebalance
				rebalance_upward(static_cast<node_type*>(new_node->mp_Parent));

				return { new_node, true };
			}
 
			template<class U>
			node_type* create_node(U&& v, base_node_type* parent)
			{
				node_type* n = this->DoAllocateNode();

				try {

					node_traits::construct(this->m_NodeAlloc, n, std::forward<U>(v));
					n->mp_Parent = parent;
					n->mp_Left = n->mp_Right = nullptr;

					// height
					n->m_Height = 1;
				}
				catch (...)
				{
					this->DoDeallocateNode(n);
					throw;
				}

				return n;
			}

			static int get_height(const node_type* n) noexcept {

				if (!n) return 0;
				return n->m_Height;
			}

			static int get_height(const base_node_type* n) noexcept {
				if (!n) return 0;
				return static_cast<const node_type*>(n)->m_Height;
			}

			static int get_balance_factor(const node_type* n) noexcept {

				if (!n) return 0;
				return get_height(n->mp_Left) - get_height(n->mp_Right);
			}

			static void update_height(node_type* n) noexcept {

				if (!n) return;

				int hl = get_height(n->mp_Left);
				int hr = get_height(n->mp_Right);

				n->m_Height = 1 + std::max(hl, hr);
			}

			node_type* rotate_left(node_type* n) noexcept {

				node_type* new_root = static_cast<node_type*>(mstl::TreeRotateLeft(n));
				update_height(n);
				update_height(new_root);
				return new_root;
			}

			node_type* rotate_right(node_type* n) noexcept {

				node_type* new_root = static_cast<node_type*>(mstl::TreeRotateRight(n));
				update_height(n);
				update_height(new_root);
				return new_root;
			}

			void rebalance_upward(node_type* n) noexcept
			{
				for (node_type* p = n; p; p = static_cast<node_type*>(p->mp_Parent))
				{
					update_height(p);
					int bf = get_balance_factor(p);

					// left heavy
					if (bf > 1)
					{
						node_type* left = static_cast<node_type*>(p->mp_Left);

						if (get_balance_factor(left) < 0)
						{
							rotate_left(left);
						}

						node_type* new_root = rotate_right(p);

						if (!new_root->mp_Parent)
						{
							this->mp_Root = new_root;
						}

						p = new_root;
					}
					// right heavy
					else if (bf < -1)
					{
						node_type* right = static_cast<node_type*>(p->mp_Right);

						if (get_balance_factor(right) > 0)
						{
							rotate_right(right);
						}

						node_type* new_root = rotate_left(p);

						if (!new_root->mp_Parent)
						{
							this->mp_Root = new_root;
						}

						p = new_root;
					}
				}
			}

			void erase_node(base_node_type* z)
			{
				if (!z) return;

				node_type* node_to_remove = static_cast<node_type*>(z);
				node_type* rebalance_from = nullptr;

				// case 1 child or no children
				if (!node_to_remove->mp_Left)
				{
					rebalance_from = static_cast<node_type*>(node_to_remove->mp_Parent);
					mstl::TreeTransplant(this->mp_Root, node_to_remove, node_to_remove->mp_Right);
				}
				else if (!node_to_remove->mp_Right)
				{
					rebalance_from = static_cast<node_type*>(node_to_remove->mp_Parent);
					mstl::TreeTransplant(this->mp_Root, node_to_remove, node_to_remove->mp_Left);
				}
				else
				{
					// case 2 children
					node_type* s = static_cast<node_type*>(mstl::TreeSuccessor(node_to_remove));

					// node where start rebalance
					if (s->mp_Parent == node_to_remove)
					{
						rebalance_from = s;
					}
					else
					{
						rebalance_from = static_cast<node_type*>(s->mp_Parent);
					}

					if (s->mp_Parent != node_to_remove)
					{
						// if s isn't a direct child
						// replace s with its right child
						// successor can't have left child
						// otherwise it wouldn't be the successor
						mstl::TreeTransplant(this->mp_Root, s, s->mp_Right);

						// transfer z right subtree to s right subtree
						// s doesn't have left child for now 
						// so make s right child, z right child
						s->mp_Right = node_to_remove->mp_Right;

						assert(s->mp_Right && "[bst_erase_node]: case 2 children, right subtree of erased must be valid since successor isn't direct child");

						s->mp_Right->mp_Parent = s;
					}

					mstl::TreeTransplant(this->mp_Root, node_to_remove, s);

					s->mp_Left = node_to_remove->mp_Left;
					if (s->mp_Left)
						s->mp_Left->mp_Parent = s;

					update_height(s);
				}

				this->DoDestroyNode(node_to_remove);
				--this->m_Size;

				// rebalance
				rebalance_upward(rebalance_from);
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

				std::cout << "Height: " << n->m_Height
					<< "\n";

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

	template<
		typename T,
		template<class> class NodeT,
		typename KeyOfValue,
		typename Compare,
		typename Alloc
	>
	bool operator==(const avl_tree<T, NodeT, KeyOfValue, Compare, Alloc>& a,
		            const avl_tree<T, NodeT, KeyOfValue, Compare, Alloc>& b)
	{
		if (a.size() != b.size()) return false;
		return std::equal(a.begin(), a.end(), b.begin(), b.end());
	}

	template<
		typename T,
		template<class> class NodeT,
		typename KeyOfValue,
		typename Compare,
		typename Alloc
	>
	bool operator!=(const avl_tree<T, NodeT, KeyOfValue, Compare, Alloc>& a,
		            const avl_tree<T, NodeT, KeyOfValue, Compare, Alloc>& b)
	{
		return !(a == b);
	}

	template<
		typename T,
		template<class> class NodeT,
		typename KeyOfValue,
		typename Compare,
		typename Alloc
	>
	void swap(avl_tree<T, NodeT, KeyOfValue, Compare, Alloc>& a,
		      avl_tree<T, NodeT, KeyOfValue, Compare, Alloc>& b) noexcept
	{
		a.swap(b);
	}
}

#endif // !MSTL_AVL_TREE_H
