#include "Logger.hpp"

#include <iostream>

#include <string_view>

const char *get_filename(const char *filepath) {
	if (filepath == nullptr) {
		logger("filepath == nullptr");
		return nullptr;
	}
	const char *last_slash = filepath;
	while (*filepath != '\0') {
		if (*filepath == '\\' || *filepath == '/') last_slash = filepath + 1;
		filepath++;
	}
	return last_slash;
}
