#pragma once
#include <result/verify.h>

#define D2D_INVOKE_ALL(invocable_collection, fn_name, ...) \
for(sl::index_t i = 0; i < invocable_collection[timeline::callback_event::fn_name].size(); ++i) \
	RESULT_VERIFY(std::invoke(invocable_collection[timeline::callback_event::fn_name][i], __VA_ARGS__));