#pragma once
#include <crow.h>


// 前向声明
template<typename U>
crow::json::wvalue convert_to_crow_json(const U& value);


template<typename U>
auto convert_to_crow_json(const U& value) -> decltype(crow::json::wvalue(value)) {
	return crow::json::wvalue(value);
}


template<typename U>
auto convert_to_crow_json(const U& obj) -> decltype(obj.to_json(), crow::json::wvalue()) {
	return obj.to_json();
}

template<typename U>
auto convert_to_crow_json(U&& obj) -> decltype(obj.to_json(), crow::json::wvalue()) {
	return obj.to_json();
}

// 处理 std::vector
template<typename U>
crow::json::wvalue convert_to_crow_json(std::vector<U>&& vec) {
    crow::json::wvalue::list result;
    for (auto& item : vec) {
        result.push_back(convert_to_crow_json(std::move(item)));
    }
    return crow::json::wvalue(std::move(result));
}

// 保留原有的 const 引用版本，但内部使用移动
template<typename U>
crow::json::wvalue convert_to_crow_json(const std::vector<U>& vec) {
    // 创建副本以便移动
    std::vector<U> temp = vec;
    return convert_to_crow_json(std::move(temp));
}


template<typename U>
crow::json::wvalue convert_to_crow_json(const std::map<std::string, U>& map) {
	crow::json::wvalue result;
	for (const auto& pair : map) {
		result[pair.first] = convert_to_crow_json(pair.second);
	}
	return result;
}
