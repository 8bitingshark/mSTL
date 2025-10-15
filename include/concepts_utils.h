#ifndef CONCEPT_UTILS_H
#define CONCEPT_UTILS_H

#include <algorithm>

namespace mstl {

	template<typename E>
	concept Element = std::semiregular<E>;

	template<typename N>
	concept Number = std::integral<N> || std::floating_point<N>;

	template<typename T>
	concept Boolean = std::convertible_to<T, bool>;
}

#endif //!CONCEPT_UTILS_H