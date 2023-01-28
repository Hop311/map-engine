#pragma once

#include <source_location>
#include <iostream>

const char *get_filename(const char *filepath);

template <typename... Ts>
struct logger {
	logger(Ts&&... ts, const std::source_location &location = std::source_location::current()) {
		std::cout
			<< get_filename(location.file_name()) << "("
			<< location.line() << ":"
			<< location.column() << ") `"
			<< location.function_name() << "`: ";
		((std::cout << std::forward<Ts>(ts)), ...);
		std::cout << std::endl;
	}
};

template <typename... Ts>
logger(Ts&&...) -> logger<Ts...>;
