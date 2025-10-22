#ifndef MSTL_RED_BLACK_TREE_H
#define MSTL_RED_BLACK_TREE_H

#include "tree.h"
#include "cassert"

namespace mstl {

	/// ---------------------------------------------------------------
	/// Red–Black enums
	/// ---------------------------------------------------------------

	enum RBColor : unsigned char  { RBRed, RBBk };
	enum RBSide  : unsigned char  { RBLeft, RBRight };

	/// ---------------------------------------------------------------
	/// rb node base
	/// ---------------------------------------------------------------

	struct rb_node_base {
		rb_node_base* mp_Left{};
		rb_node_base* mp_Right{};
		rb_node_base* mp_Parent{};
		RBColor m_Color{ RBRed };
	};

	/// ---------------------------------------------------------------
	/// rb node
	/// ---------------------------------------------------------------
	/// derives from node<T>
	/// 

	template<typename T>
	struct rb_node : rb_node_base {
		
		using value_type = T;
		using base_type = rb_node_base;
		T m_Val;

		explicit rb_node(const T& v) : rb_node_base{}, m_Val(v) {}
		explicit rb_node(T&& v) : rb_node_base{}, m_Val(std::move(v)) {}

		template<class... Args>
		explicit rb_node(std::in_place_t, Args&&... args)
			: rb_node_base{}
			, m_Val(std::forward<Args>(args)...) {
		}
	};

	/// ---------------------------------------------------------------
	/// RB Tree
	/// ---------------------------------------------------------------
	/// Invariants:
	/// 1. Every node is red or black.
	/// 2. The root is black.
	/// 3. All leaves (nullptr) are black.
	/// 4. Red nodes cannot have red children.
	/// 5. Every path from a node to descendant leaves has the same 
	///    number of black nodes.
	/// ---------------------------------------------------------------

	template<
		typename T,
		template<class> class NodeT = rb_node,
		typename KeyOfValue = identity_key<T>,
		typename compare = std::less<
		std::remove_cvref_t<decltype(std::declval<KeyOfValue>()(std::declval<const T&>()))>>,
		typename A = std::allocator<T>
		>
		class rb_tree : public tree_base<T, NodeT, KeyOfValue, compare, A> {

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
	
			explicit rb_tree(const alloc_type& a = alloc_type{}, const compare& c = compare{})
				: base_type(a, c) {
			}

			template<std::input_iterator It>
			rb_tree(It first, It last, const alloc_type& a = alloc_type{}, const compare& c = compare{})
				: base_type(a, c)
			{
				for (; first != last; ++first)
					insert(*first);
			}

			rb_tree(std::initializer_list<T> il, const alloc_type& a = alloc_type{}, const compare& c = compare{})
				: base_type(a, c)
			{
				for (const auto& v : il)
					insert(v);
			}

			// ============= Copy semantics =================

			rb_tree(const rb_tree& other)
				: base_type(other.m_ValueAlloc, other.m_Comp)
			{
				for (const auto& v : other) insert(v);
			}

			rb_tree& operator=(const rb_tree& other)
			{
				if (this == &other) return *this;
				rb_tree tmp(other);
				swap(tmp);
				return *this;
			}

			// ============= Move semantics =================

			rb_tree(rb_tree&& other) noexcept
			{
				swap(other);
			}

			rb_tree& operator=(rb_tree&& other) noexcept
			{
				if (this != &other) swap(other);
				return *this;
			}

			// ================= Capacity =================

			size_type size() const noexcept { return this->m_Size; }

			bool empty() const noexcept { return this->m_Size == 0; }

			// ================= Modifiers =================

			void clear() noexcept { this->DoClear(); }

			template<typename U>
			std::pair<iterator, bool> insert(U&& v)
			{
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
			size_type erase(const key_type& key)
			{
				base_node_type* z = mstl::TreeFind<node_type, base_node_type>(this->mp_Root, key, this->m_KeyExtractor, this->m_Comp);
				if (!z) return 0;
				erase_node(z);
				return 1;
			}

			// erase by iterator -> returns successor
			iterator erase(iterator pos)
			{
				base_node_type* z = pos.curr;
				if (!z) return this->end();
				base_node_type* s = mstl::TreeSuccessor<base_node_type>(z);
				erase_node(z);
				return iterator{ s };
			}

			void swap(rb_tree& other) noexcept
			{
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

			bool IsRBTree() const noexcept
			{
				return verify_rb_properties();
			}

		private:

			// ================= Helpers =================

			static RBColor color_of(const base_node_type* n) noexcept
			{
				if (!n) return RBBk; // leaves are always black
				return static_cast<const node_type*>(n)->m_Color;
			}

			static void set_color(base_node_type* n, RBColor c) noexcept
			{
				if (n) static_cast<node_type*>(n)->m_Color = c;
			}

			template<typename U>
			std::pair<base_node_type*, bool> insert_impl(U&& v)
			{
				base_node_type* parent = nullptr;
				base_node_type* current = this->mp_Root;

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

				// fixup color
				insert_fixup(new_node);

				return { new_node, true };
			}

			// Could be better to initialize the color to black, directly?
			template<class U>
			node_type* create_node(U&& v, base_node_type* parent)
			{
				node_type* n = this->DoAllocateNode();

				try {

					node_traits::construct(this->m_NodeAlloc, n, std::forward<U>(v));
					n->mp_Parent = parent;
					n->mp_Left = n->mp_Right = nullptr;

					// default color
					n->m_Color = RBRed;
				}
				catch (...)
				{
					this->DoDeallocateNode(n);
					throw;
				}

				return n;
			}

			void insert_fixup(base_node_type* x) noexcept
			{
				base_node_type* px = x->mp_Parent;
				
				// until I return to root or I have red child and red parent
				while (px && color_of(px) == RBRed)
				{
					base_node_type* gx = px->mp_Parent;
					
					// if gx not valid px is root
					if (!gx) break;

					bool IsXLeftChild = x == px->mp_Left ? true : false;

					bool IsUncleLeftChild = false;
					base_node_type* ux = nullptr;

					if (px == gx->mp_Left)
					{
						// x parent is left child
						// so retrieve x uncle, that is a right child
						ux = gx->mp_Right;
					}
					else
					{
						// px is right child
						// so uncle is left child
						ux = gx->mp_Left;
						IsUncleLeftChild = true;
					}

					// quite-fortunate case: uncle is black and same side of x
					if (color_of(ux) == RBBk && (IsXLeftChild == IsUncleLeftChild))
					{
						if (!IsXLeftChild)
						{
							mstl::TreeRotateLeft<base_node_type>(px);
						}
						else
						{
							mstl::TreeRotateRight<base_node_type>(px);
						}

						base_node_type* tmp = x;
						x = px;
						px = tmp;
						IsXLeftChild = x == px->mp_Left ? true : false;
					}

					// fortunate case: uncle is black and opposite side of x
					if (color_of(ux) == RBBk) // && (IsXLeftChild != IsUncleLeftChild)
					{
						base_node_type* new_root = nullptr;

						if (IsXLeftChild)
						{
							// right rotation
							new_root = mstl::TreeRotateRight<base_node_type>(gx);
						}
						else
						{
							// left rotation
							new_root = mstl::TreeRotateLeft<base_node_type>(gx);
						}

						// recolor
						set_color(gx, RBRed);
						set_color(px, RBBk);

						if (!new_root->mp_Parent)
							this->mp_Root = static_cast<node_type*>(new_root);

						break;
					}

					// unlucky: uncle is red
					assert(color_of(ux) == RBRed);

					set_color(px, RBBk);
					set_color(ux, RBBk);
					set_color(gx, RBRed);

					px = gx->mp_Parent;
					x = gx;
				}

				if (this->mp_Root)
				{
					set_color(this->mp_Root, RBBk);
				}
			}

			// For an optimization I can choose a successor or predecessor based on
			// its color?
			void erase_node(base_node_type* x)
			{
				if (!x) return;

				node_type* node_to_delete = static_cast<node_type*>(x);

				base_node_type* removed_node = x;
				RBColor rn_original_color = x->m_Color;
				base_node_type* rn_substitute = nullptr;

				if (!node_to_delete->mp_Left)
				{
					rn_substitute = node_to_delete->mp_Right;
					mstl::TreeTransplant<node_type, base_node_type>(this->mp_Root, node_to_delete, node_to_delete->mp_Right);
				}
				else if (!node_to_delete->mp_Right)
				{
					rn_substitute = node_to_delete->mp_Left;
					mstl::TreeTransplant<node_type, base_node_type>(this->mp_Root, node_to_delete, node_to_delete->mp_Left);
				}
				else
				{
					// Use the successor
					base_node_type* s = mstl::TreeSuccessor<base_node_type>(x);
					
					removed_node = s;
					rn_original_color = s->m_Color;

					// successor has at the most one child
					// and it has no left child, otherwise it wouldn't be
					// the successor

					rn_substitute = s->mp_Right;

					if (removed_node->mp_Parent == node_to_delete)
					{
						if (rn_substitute) rn_substitute->mp_Parent = removed_node;
					}

					// it isn't a direct child
					else
					{
						mstl::TreeTransplant<node_type, base_node_type>(this->mp_Root, s, s->mp_Right);

						s->mp_Right = node_to_delete->mp_Right;

						assert(s->mp_Right && "[bst_erase_node]: case 2 children, right subtree of erased must be valid since successor isn't direct child");

						s->mp_Right->mp_Parent = s;
					}

					mstl::TreeTransplant<node_type, base_node_type>(this->mp_Root, node_to_delete, s);

					s->mp_Left = node_to_delete->mp_Left;
					if (s->mp_Left) s->mp_Left->mp_Parent = s;

					// Keep the original color
					set_color(removed_node, node_to_delete->m_Color);
				}

				this->DoDestroyNode(node_to_delete);
				--this->m_Size;

				base_node_type leafSentinel{};
				leafSentinel.m_Color = RBBk;
				base_node_type* ptr_leafSentinel = &leafSentinel;

				// check for fix up
				if (rn_original_color == RBBk)
				{
					if (!rn_substitute)
					{
						leafSentinel.mp_Parent = removed_node;
						erase_fixup(ptr_leafSentinel);
					}
					else
					{
						erase_fixup(rn_substitute);
					}	
				}		
			}

			void erase_fixup(base_node_type* double_black)
			{
				while (double_black && double_black->m_Color == RBBk)
				{
					// parent of double black
					base_node_type* px = double_black->mp_Parent;

					// if px is not valid double_black is the root
					if (!px) break;

					bool IsDBLeftChild = double_black == px->mp_Left ? true : false;

					// brother of double black
					base_node_type* bx = IsDBLeftChild ? px->mp_Right : px->mp_Left;

					// children of brother
					base_node_type* sameX{};
					base_node_type* oppoX{};

					if (bx)
					{
						sameX = IsDBLeftChild ? bx->mp_Left : bx->mp_Right;
						oppoX = IsDBLeftChild ? bx->mp_Right : bx->mp_Left;
					}

					// worstCase: brother of double_black is red, but the siblings are black
					if (bx && bx->m_Color == RBRed)
					{
						// if px is the root, now it became bx
						if (!px->mp_Parent) this->mp_Root = static_cast<node_type*>(bx);

						// rotate 
						if (IsDBLeftChild)
						{
							mstl::TreeRotateLeft<base_node_type>(px);
						}
						else
						{
							mstl::TreeRotateRight<base_node_type>(px);
						}

						// recolor
						set_color(px, RBRed);
						set_color(bx, RBBk);

						// update brother
						bx = px->mp_Right;
					}

					// bad case: brother and siblings are black
					if ((!sameX || (sameX && sameX->m_Color == RBBk))
						&& (!oppoX || (oppoX && oppoX->m_Color == RBBk)))
					{
						set_color(bx, RBRed);
						double_black = px;

						if (double_black->m_Color == RBRed)
						{
							set_color(double_black, RBBk);
							break;
						}
					}
					else
					{

						// semi fortunate case: nephew of the same side is red, but the other is black

						if ((sameX && sameX->m_Color == RBRed) && (!oppoX || (oppoX && oppoX->m_Color == RBBk)))
						{
							if (IsDBLeftChild)
							{
								mstl::TreeRotateRight<base_node_type>(bx);
							}
							else
							{
								mstl::TreeRotateLeft<base_node_type>(bx);
							}

							set_color(sameX, RBBk);
							set_color(bx, RBRed);

							//base_node_type* tmp = sameX;
							bx = sameX;
						}

						// technically no need of tests ?
						// if (oppoX && oppoX->m_Color == RBRed)

						// fortunate case: double_black (uncle) has a RED nephew at the opposite side
						if (IsDBLeftChild)
						{
							mstl::TreeRotateLeft<base_node_type>(px);
						}
						else
						{
							mstl::TreeRotateRight<base_node_type>(px);
						}

						// recolor
						set_color(oppoX, RBBk);
						set_color(bx, px->m_Color);
						set_color(px, RBBk);
						set_color(double_black, RBBk);
						
						if (!bx->mp_Parent)
						{
							this->mp_Root = static_cast<node_type*>(bx);
						}

						break;
					}
				}

				if (double_black) set_color(double_black, RBBk);
			}

			// RB_Tree Verification

			bool verify_rb_properties() const noexcept {
				bool ok = true;

				// 1. Root is black
				if (this->mp_Root && color_of(this->mp_Root) != RBBk) {
					std::cerr << "[RB VERIFY] Root is not black!\n";
					ok = false;
				}

				int black_height = -1;
				if (!verify_node_rec(this->mp_Root, 0, black_height)) {
					ok = false;
				}

				return ok;
			}

			bool verify_node_rec(const base_node_type* node, int black_count, int& expected_black_height) const noexcept {
				if (!node) {
					// Leaf (nullptr) — treat as black
					if (expected_black_height == -1)
						expected_black_height = black_count + 1;
					else if (black_count + 1 != expected_black_height) {
						std::cerr << "[RB VERIFY] Black height mismatch at leaf\n";
						return false;
					}
					return true;
				}

				// Check parent linkage consistency
				if (node->mp_Left && node->mp_Left->mp_Parent != node) {
					std::cerr << "[RB VERIFY] Left child parent pointer mismatch at node\n";
					return false;
				}
				if (node->mp_Right && node->mp_Right->mp_Parent != node) {
					std::cerr << "[RB VERIFY] Right child parent pointer mismatch at node\n";
					return false;
				}

				// Count black nodes along this path
				if (color_of(node) == RBBk)
					++black_count;

				// Rule 4: Red node cannot have red children
				if (color_of(node) == RBRed) {
					if ((node->mp_Left && color_of(node->mp_Left) == RBRed) ||
						(node->mp_Right && color_of(node->mp_Right) == RBRed)) {
						std::cerr << "[RB VERIFY] Red node with red child detected\n";
						return false;
					}
				}

				// Recurse left and right
				bool left_ok = verify_node_rec(node->mp_Left, black_count, expected_black_height);
				bool right_ok = verify_node_rec(node->mp_Right, black_count, expected_black_height);

				return left_ok && right_ok;
			}

			// Print Utility

			void inorder_print_rec(base_node_type* node, base_node_type* parent, const char* relation) const noexcept {

				if (!node) return;

				inorder_print_rec(node->mp_Left, node, "left");

				const node_type* n = static_cast<const node_type*>(node);
				const node_type* p = static_cast<const node_type*>(parent);
				const node_type* l = static_cast<const node_type*>(node->mp_Left);
				const node_type* r = static_cast<const node_type*>(node->mp_Right);

				std::cout << "Node: " << n->m_Val
					<< " (" << relation << ")\n";

				std::cout << "Color: " << (n->m_Color == RBBk ? "Black" : "Red")
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

	template<
		typename T,
		template<class> class NodeT,
		typename KeyOfValue,
		typename Compare,
		typename Alloc
	>
	bool operator==(const rb_tree<T, NodeT, KeyOfValue, Compare, Alloc>& a,
		const rb_tree<T, NodeT, KeyOfValue, Compare, Alloc>& b)
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
	bool operator!=(const rb_tree<T, NodeT, KeyOfValue, Compare, Alloc>& a,
		const rb_tree<T, NodeT, KeyOfValue, Compare, Alloc>& b)
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
	void swap(rb_tree<T, NodeT, KeyOfValue, Compare, Alloc>& a,
		rb_tree<T, NodeT, KeyOfValue, Compare, Alloc>& b) noexcept
	{
		a.swap(b);
	}
}

#endif // !MSTL_RED_BLACK_TREE_H