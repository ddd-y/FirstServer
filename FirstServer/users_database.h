#pragma once

#include"database.h"
#include"unified_data.h"
#include<cstring>
#include<nlohmann/json.hpp>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <random>

using json = nlohmann::json;


class UsersDatabase : public Database
{
public:
	struct CalculationRecords
	{
		std::string operation_type;
		crow::json::wvalue input_data;
		crow::json::wvalue output_data;
		int matrix_size_rows;
		int matrix_size_cols;
		std::string status;

		CalculationRecords() = default;
		CalculationRecords(CalculationRecords&& other) noexcept
			: operation_type(std::move(other.operation_type)),
			input_data(std::move(other.input_data)),
			output_data(std::move(other.output_data)),
			matrix_size_rows(other.matrix_size_rows),
			matrix_size_cols(other.matrix_size_cols),
			status(std::move(other.status)) {}


		CalculationRecords(const CalculationRecords&) = delete;


		CalculationRecords& operator=(const CalculationRecords&) = delete;

	
		CalculationRecords& operator=(CalculationRecords&& other) noexcept {
			if (this != &other) {
				operation_type = std::move(other.operation_type);
				input_data = std::move(other.input_data);
				output_data = std::move(other.output_data);
				matrix_size_rows = other.matrix_size_rows;
				matrix_size_cols = other.matrix_size_cols;
				status = std::move(other.status);
			}
			return *this;
		}

		crow::json::wvalue to_json()
		{
			crow::json::wvalue j;
			j["operation_type"] = operation_type;
			j["input_data"] = std::move(input_data);
			j["output_data"] = std::move(output_data);
			j["matrix_size_rows"] = matrix_size_rows;
			j["matrix_size_cols"] = matrix_size_cols;
			j["status"] = status;
			return j;
		}
	};


	struct HistoryPageResult {
		std::vector<crow::json::wvalue> records;
		int total_count;
		int total_pages;

		crow::json::wvalue to_json() {
			crow::json::wvalue result;
			result["records"] = crow::json::wvalue(std::move(records));
			result["total_count"] = total_count;
			result["total_pages"] = total_pages;
			return result;
		}
	};

	UsersDatabase(const std::string& addr, const std::string& user, const std::string& passwd, const std::string& database, unsigned int port) : Database(addr, user, passwd, database, port) {
	};
	~UsersDatabase();


	bool register_user(const std::string& username, const std::string& password, const std::string& email);
	bool login_username(const std::string& username, const std::string& password);
	bool logout_username(const std::string& username);
	bool login_email(const std::string& username, const std::string& password);
	int64_t get_user_id(const std::string& username);


	PageResponse<std::vector<CalculationRecords>> get_calculation_records(int64_t user_id, const PageRequest& page_request);

	// users_database.h
// 修改函数声明，移除metadata参数
	int64_t insertHistoryRecord(int64_t user_id,
		const UnifiedData& input_data,
		const UnifiedData& output_data,
		const std::string& operation_type);  // 只传递操作类型

	HistoryPageResult getHistoryRecordsPaginated(int64_t user_id,  // 浣跨敤user_id
		int page = 1,
		int page_size = 20,
		const std::string& record_type = "");

	crow::json::wvalue getHistoryRecordWithData(int64_t record_id);

	UnifiedData loadMatrixData(int64_t record_id, const std::string& data_type);

private:

	bool isLargeMatrix(const UnifiedData& data);
	std::string determineRecordType(const UnifiedData& input, const UnifiedData& output);
	std::string dataTypeToString(UnifiedData::DataType type);
	std::string getMatrixTypeString(const UnifiedData& data);
	crow::json::wvalue buildInputMetadata(const UnifiedData& data);
	crow::json::wvalue buildOutputMetadata(const UnifiedData& data);
	crow::json::wvalue historyRowToJson(MYSQL_ROW row, MYSQL_FIELD* fields, int num_fields);
	std::string serializeForStorage(const UnifiedData& data);
	void storeBinaryData(int64_t record_id, int64_t user_id, const std::string& data_type, const UnifiedData& data);
	void loadBinaryData(int64_t record_id, crow::json::wvalue& record);
	std::string escapeString(const std::string& str);

	bool insertMatrixRecord(int64_t record_id, int64_t user_id,
		const UnifiedData& input_data,
		const UnifiedData& output_data);
	bool insertTextRecord(int64_t record_id, int64_t user_id,
		const UnifiedData& input_data,
		const UnifiedData& output_data);

	static std::string base64_encode(const uint8_t* data, size_t length);
	std::vector<uint8_t> base64_decode(const std::string& encoded_string);
};