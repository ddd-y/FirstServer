#pragma once

#include<mysql.h>
#include <memory>
#include <iostream>
#include<string>
//用户的数据库操作类
#include<iostream>
#include"logger.h"
#include<nlohmann/json.hpp>
#include"to_json.h"

using json = nlohmann::json;

/*	数据库基类
*	完成连接到各个数据库 用户信息数据库 用户各个记录
*/

//每个用户还有运算记录等 使用记录等

//分页查询代码

class Database
{
public:
	Database(const std::string& addr, const std::string& user, const std::string& passwd, const std::string& database, unsigned int port)
	{
		mysql_init(&sql);
		mysql_options(&sql, MYSQL_SET_CHARSET_NAME, "gbk");
		if (mysql_real_connect(&sql, addr.c_str(), user.c_str(), passwd.c_str(), database.c_str(), port, NULL, 0) == NULL)
		{
			LOG_DEBUG("数据库连接失败");
			LOG_ERROR("数据库连接失败");
			std::cerr << "数据库连接失败" << mysql_error(&sql) << std::endl;
			connected = false;
		}
		else
		{
			LOG_DEBUG("数据库连接成功");
			connected = true;
		}
	}

	~Database()
	{
		if (connected)
			mysql_close(&sql);
	}

	Database(const Database&) = delete;
	Database& operator=(const Database&) = delete;

	bool is_connected() const;

	std::string error_mes() const
	{
		return error_message;
	}

	// 事务支持
	void start_transaction() {
		mysql_query(&sql, "START TRANSACTION");
	}

	bool commit() {
		if (mysql_query(&sql, "COMMIT")) {
			error_message = "提交事务失败: " + std::string(mysql_error(&sql));
			return false;
		}
		return true;
	}

	bool roll_back() {
		if (mysql_query(&sql, "ROLLBACK")) {
			error_message = "回滚事务失败: " + std::string(mysql_error(&sql));
			return false;
		}
		return true;
	}

	struct PageRequest
	{
		uint32_t page;
		uint32_t limit;

		uint64_t get_offset() const
		{
			return (page - 1) * limit;
		}
	};

	struct Pagination
	{
		uint32_t current_page;
		uint32_t limit;
		uint32_t total_page;
		uint64_t total_count;

		bool has_next()
		{
			return current_page < total_page;
		}

		bool has_previous()
		{
			return current_page > 1;
		}

		crow::json::wvalue to_json()
		{
			crow::json::wvalue result;
			result["current_page"] = current_page;
			result["limit"] = limit;
			result["total_page"] = total_page;
			result["total_count"] = total_count;
			return result;
		};
	};

	template<typename T>
	struct PageResponse
	{
		bool success;
		std::string message;
		T data;
		Pagination pagination;

		crow::json::wvalue to_json()
		{
			crow::json::wvalue result;
			result["success"] = success;
			result["message"] = message;

			// Serialize pagination
			crow::json::wvalue pagination_json;
			pagination_json["current_page"] = pagination.current_page;
			pagination_json["limit"] = pagination.limit;
			pagination_json["total_page"] = pagination.total_page;
			pagination_json["total_count"] = pagination.total_count;
			result["pagination"] = std::move(pagination_json);

			// Serialize data
			result["data"] = convert_to_crow_json(std::move(data));

			return result;
		};
	};

	// 获取总记录数
	int64_t get_total_count(const std::string& table_name, const std::string& where_clause = "") {
		std::string count_query = "SELECT COUNT(*) FROM " + table_name;
		if (!where_clause.empty()) {
			count_query += " WHERE " + where_clause;
		}

		if (mysql_query(&sql, count_query.c_str())) {
			error_message = "计数查询失败: " + std::string(mysql_error(&sql));
			return -1;
		}

		MYSQL_RES* result = mysql_store_result(&sql);
		if (!result) {
			error_message = "获取结果失败: " + std::string(mysql_error(&sql));
			return -1;
		}

		MYSQL_ROW row = mysql_fetch_row(result);
		int64_t total_count = std::stoll(row[0]);

		mysql_free_result(result);
		return total_count;
	}

protected:
	MYSQL sql;
	bool connected = false;
	std::string error_message;
};