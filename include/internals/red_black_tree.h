#ifndef MSTL_RED_BLACK_TREE_H
#define MSTL_RED_BLACK_TREE_H

#include "tree.h"

namespace mstl {

	/// Both EASTL and MSVC don't use 
	/// enum class to define color and side
	/// MSVC has an enum class for Strategy: copy or move
	/// MSVC doesn't have side

	enum RBColor : unsigned char  { RBRed, RBBk };
	enum RBSide  : unsigned char  { RBLeft, RBRight };

	/// ---------------------------------------------------------------
	/// rb node base
	/// ---------------------------------------------------------------

	struct rb_node_base {
		rb_node_base* mp_Left{};
		rb_node_base* mp_Right{};
		rb_node_base* mp_Parent{};
		unsigned char m_Color{ RBRed };
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


}

#endif // !MSTL_RED_BLACK_TREE_H