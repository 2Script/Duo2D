#pragma once
#include <result/verify.h>

#define D2D_INVOKE_ALL(invocable_collection, fn_name, ...) \
for(sl::index_t i = 0; i < invocable_collection.size(); ++i) \
	if(invocable_collection[i].fn_name) \
		RESULT_VERIFY(std::invoke(invocable_collection[i].fn_name, __VA_ARGS__));