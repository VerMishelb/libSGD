#include "libSGDNamespace.h"

namespace libSGD {
	std::string read_string(std::ifstream& a) {
		std::string result;
		uint8_t t = a.get();

		//Protection from yeeting into space
		for (int length = 0; t != 0 && length < 0xff; length++) {
			if (t == '\0') {
				return result;
			}
			result += t;
			t = a.get();
		}
		return result;
	}
}