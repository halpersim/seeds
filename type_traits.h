#pragma once

namespace type_traits {

	template<bool b, int i1, int i2>
	struct SelectInt {
		enum{value = i1};
	};


	template<int i1, int i2>
	struct SelectInt<false, i1, i2> {
		enum{value = i2};
	};
}