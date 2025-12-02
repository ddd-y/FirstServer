#include "users_database.h"
#include<sstream>
#include "eigen_matrix_serializer.h" // 假设有这个序列化器

UsersDatabase::~UsersDatabase()
{

    start_transaction();
    mysql_query(&sql, "UPDATE info SET status = 'inactive'");
    if (!commit()) roll_back();

    if (connected)
        mysql_close(&sql);
}

bool UsersDatabase::register_user(const std::string& username, const std::string& password, const std::string& email)
{
    char query[256];
    start_transaction();
    snprintf(query, sizeof(query), "INSERT INTO info(username,password,email,entry_date,status) values('%s','%s','%s',current_date(),'inactive')", username.c_str(), password.c_str(), email.c_str());


    int unsuccess = mysql_query(&sql, query);
    if (unsuccess)
    {
        error_message = ": " + std::string(mysql_error(&sql));
        roll_back();
        return false;
    }

    if (!commit()) {
        roll_back();
        return false;
    }

    return true;
}

bool UsersDatabase::login_username(const std::string& username, const std::string& password)
{

    start_transaction();
    char query[256];
    snprintf(query, sizeof(query), "SELECT password,status FROM info WHERE username = '%s'", username.c_str());
    if (mysql_query(&sql, query))
    {
        error_message = ": " + std::string(mysql_error(&sql));
        roll_back();
        return false;
    }

    MYSQL_RES* res;
    MYSQL_ROW row;


    res = mysql_store_result(&sql);
    if (mysql_num_rows(res))
    {
        row = mysql_fetch_row(res);
    }
    else
    {
        error_message = "";
        roll_back();
        return false;
    }

    //殊霞畜鷹頁倦屎鳩
    if (password != row[0])
    {
        error_message = "";
        roll_back();
        return false;
    }


    if (row[1] != nullptr && std::string(row[1]) == "active")
    {

        roll_back();
        return false;
    }


    snprintf(query, sizeof(query), "UPDATE info SET status = 'active' WHERE username = '%s'", username.c_str());
    mysql_query(&sql, query);
    if (!commit()) {
        roll_back(); return false;
    }

    mysql_free_result(res);


    return true;

}

bool UsersDatabase::logout_username(const std::string& username)
{
    start_transaction();
    char query[256];
    snprintf(query, sizeof(query), "UPDATE info SET status = 'inactive' WHERE username = '%s'", username.c_str());
    if (mysql_query(&sql, query))
    {
        error_message = ": " + std::string(mysql_error(&sql));
        roll_back();
        return false;
    }
    if (!commit()) {
        roll_back(); return false;
    }

    return true;
}

bool UsersDatabase::login_email(const std::string& email, const std::string& password)
{
    //殊霞喘薩頁倦贋壓
    start_transaction();
    char query[256];
    snprintf(query, sizeof(query), "SELECT password,status FROM info WHERE email = '%s'", email.c_str());
    if (mysql_query(&sql, query))
    {
        error_message = ": " + std::string(mysql_error(&sql));
        roll_back();
        return false;
    }

    MYSQL_RES* res;
    MYSQL_ROW row;

    res = mysql_store_result(&sql);
    if (mysql_num_rows(res))
    {
        row = mysql_fetch_row(res);
    }
    else
    {
        error_message = "";
        roll_back();
        return false;
    }


    if (password != row[0])
    {
        error_message = "";
        roll_back();
        return false;
    }


    if (row[1] != nullptr && std::string(row[1]) == "active")
    {
        error_message = "";
        roll_back();
        return false;
    }

    //繍喘薩彜蓑譜葎active
    snprintf(query, sizeof(query), "UPDATE info SET status = 'active' WHERE email = '%s'", email.c_str());
    mysql_query(&sql, query);
    if (!commit()) {
        roll_back(); return false;
    }

    mysql_free_result(res);

    //窟綜兎公人薩極
    return true;

}

int64_t UsersDatabase::get_user_id(const std::string& username)
{
    start_transaction();
    char query[256];
    snprintf(query, sizeof(query), "SELECT id FROM info WHERE username = '%s'", username.c_str());
    if (mysql_query(&sql, query))
    {
        error_message = ": " + std::string(mysql_error(&sql));
        roll_back();
        return -1;
    }

    MYSQL_RES* res;
    MYSQL_ROW row;

    res = mysql_store_result(&sql);
    if (mysql_num_rows(res))
    {
        row = mysql_fetch_row(res);
    }
    else
    {
        error_message = "";
        roll_back();
        return -1;
    }

    int64_t user_id = std::stoll(row[0]);

    if (!commit()) {
        roll_back(); return -1;
    }

    mysql_free_result(res);

    return user_id;
}

int calculate_total_pages(int64_t total_count, int page_size) {
    if (total_count <= 0 || page_size <= 0) {
        return 0;
    }
    return (total_count + page_size - 1) / page_size;
}

UsersDatabase::PageResponse<std::vector<UsersDatabase::CalculationRecords>> UsersDatabase::get_calculation_records(int64_t user_id, const PageRequest& page_request)
{
    start_transaction();
    std::stringstream ss;
    ss << "SELECT operation_type,input_data,output_matrix,matrix_size_rows,matrix_size_cols,status FROM calculation_records WHERE user_id = " << user_id
        << " LIMIT " << page_request.limit << " OFFSET " << page_request.get_offset();

    PageResponse<std::vector<UsersDatabase::CalculationRecords>> response;
    response.pagination.current_page = page_request.page;
    response.pagination.limit = page_request.limit;


    std::vector<UsersDatabase::CalculationRecords> records;


    if (mysql_query(&sql, ss.str().c_str()))
    {
        error_message = ": " + std::string(mysql_error(&sql));
        roll_back();

        response.success = false;
        return response;
    }

    MYSQL_RES* res;
    MYSQL_ROW row;

    res = mysql_store_result(&sql);
    while (row = mysql_fetch_row(res))
    {
        records.emplace_back();
        auto& record = records.back();
        record.operation_type = row[0] ? row[0] : "";

        if (row[1]) {
            auto input_json = crow::json::load(row[1]);
            if (input_json) {
                record.input_data = std::move(input_json);
            }
        }
        else {
            record.input_data = crow::json::wvalue();
        }

        if (row[2]) {
            auto input_json = crow::json::load(row[2]);
            if (input_json) {
                record.output_data = std::move(input_json);
            }
        }
        else {
            record.output_data = crow::json::wvalue();
        }


        record.matrix_size_rows = row[3] ? std::stoi(row[3]) : 0;
        record.matrix_size_cols = row[4] ? std::stoi(row[4]) : 0;
        record.status = row[5] ? row[5] : "";
    }

    std::string where_clause = "user_id = " + std::to_string(user_id);
    response.pagination.total_count = get_total_count("calculation_records", where_clause);
    response.pagination.total_page = calculate_total_pages(response.pagination.total_count, page_request.limit);

    response.data = std::move(records);
    response.success = true;

    mysql_free_result(res);
    if (!commit()) {
        roll_back();
        response.success = false;
    }

    return response;
}

// ==================== 历史记录方法实现 ====================

std::string UsersDatabase::escapeString(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        if (c == '\'') escaped += "\\'";
        else if (c == '\\') escaped += "\\\\";
        else escaped += c;
    }
    return escaped;
}

// 修改 insertHistoryRecord 函数
// 修改 insertHistoryRecord 函数 - 根据实际的数据库表结构
int64_t UsersDatabase::insertHistoryRecord(int64_t user_id,
    const UnifiedData& input_data,
    const UnifiedData& output_data,
    const std::string& operation_type) {
    start_transaction();

    try {
        LOG_DEBUG("开始插入历史记录，用户ID: {}, 操作类型: {}", user_id, operation_type);

        // 确定存储策略
        bool input_is_large = input_data.isMatrix() && isLargeMatrix(input_data);
        bool output_is_large = output_data.isMatrix() && isLargeMatrix(output_data);

        // 准备插入数据
        std::string record_type = determineRecordType(input_data, output_data);
        std::string input_type = dataTypeToString(input_data.getType());
        std::string output_type = dataTypeToString(output_data.getType());

        // 构建插入查询 - 根据实际的history_records表结构
        std::stringstream query;
        query << "INSERT INTO history_records (user_id, record_type, operation_type, "
            << "input_data_type, output_data_type, storage_method, "
            << "input_data, output_data) "
            << "VALUES ("
            << user_id << ", '"
            << record_type << "', '"
            << operation_type << "', '"
            << input_type << "', '"
            << output_type << "', '";

        std::string storage_method = (input_is_large || output_is_large) ? "external" : "inline";
        query << storage_method << "', ";

        LOG_DEBUG("存储方法: {}, 输入类型: {}, 输出类型: {}",
            storage_method, input_type, output_type);

        // 处理内联存储的数据
        if (!input_is_large) {
            std::string input_serialized = serializeForStorage(input_data);
            std::string escaped_input = escapeString(input_serialized);
            query << "'" << escaped_input << "', ";
        }
        else {
            query << "NULL, ";
        }

        if (!output_is_large) {
            std::string output_serialized = serializeForStorage(output_data);
            std::string escaped_output = escapeString(output_serialized);
            query << "'" << escaped_output << "')";
        }
        else {
            query << "NULL)";
        }

        LOG_DEBUG("执行SQL: {}", query.str());

        // 执行插入
        if (mysql_query(&sql, query.str().c_str())) {
            error_message = "插入记录失败: " + std::string(mysql_error(&sql));
            LOG_ERROR("SQL错误: {}", error_message);
            roll_back();
            return -1;
        }

        // 获取插入的ID
        int64_t record_id = mysql_insert_id(&sql);
        LOG_DEBUG("插入成功，记录ID: {}", record_id);

        // 处理大型矩阵的外部存储
        if (input_is_large) {
            storeBinaryData(record_id, user_id, "input", input_data);

            // 更新外部存储引用
            std::stringstream update_query;
            update_query << "UPDATE history_records SET input_external_ref = 'ref_"
                << record_id << "_input' WHERE id = " << record_id;
            if (mysql_query(&sql, update_query.str().c_str())) {
                LOG_WARN("更新输入外部引用失败: {}", mysql_error(&sql));
            }
        }

        if (output_is_large) {
            storeBinaryData(record_id, user_id, "output", output_data);

            // 更新外部存储引用
            std::stringstream update_query;
            update_query << "UPDATE history_records SET output_external_ref = 'ref_"
                << record_id << "_output' WHERE id = " << record_id;
            if (mysql_query(&sql, update_query.str().c_str())) {
                LOG_WARN("更新输出外部引用失败: {}", mysql_error(&sql));
            }
        }

        // 插入矩阵记录（如果适用）
        if (record_type == "matrix") {
            std::stringstream matrix_query;
            matrix_query << "INSERT INTO matrix_records (record_id, user_id";

            // 添加输入矩阵信息
            if (input_data.isMatrix()) {
                matrix_query << ", input_rows, input_cols, input_matrix_type, input_format";
            }

            // 添加输出矩阵信息
            if (output_data.isMatrix()) {
                if (input_data.isMatrix()) {
                    matrix_query << ", ";
                }
                matrix_query << "output_rows, output_cols, output_matrix_type, output_format";
            }

            matrix_query << ") VALUES (" << record_id << ", " << user_id;

            if (input_data.isMatrix()) {
                int rows = input_data.getMatrixRows();
                int cols = input_data.getMatrixCols();
                std::string format = input_data.isMatrixDense() ? "dense" : "sparse";
                matrix_query << ", " << rows << ", " << cols << ", 'double', '" << format << "'";
            }

            if (output_data.isMatrix()) {
                int rows = output_data.getMatrixRows();
                int cols = output_data.getMatrixCols();
                std::string format = output_data.isMatrixDense() ? "dense" : "sparse";
                matrix_query << ", " << rows << ", " << cols << ", 'double', '" << format << "'";
            }

            matrix_query << ")";

            LOG_DEBUG("插入矩阵记录: {}", matrix_query.str());

            if (mysql_query(&sql, matrix_query.str().c_str())) {
                error_message = "插入矩阵记录失败: " + std::string(mysql_error(&sql));
                LOG_ERROR("矩阵记录插入失败: {}", error_message);
                roll_back();
                return -1;
            }
        }
        // 如果是文本/JSON记录，插入text_records表
        else if (record_type == "text" || record_type == "json") {
            std::stringstream text_query;
            text_query << "INSERT INTO text_records (record_id, user_id, input_length, output_length, text_format) VALUES ("
                << record_id << ", " << user_id << ", ";

            if (input_data.isText() || input_data.isJson()) {
                std::string input_str = input_data.isText() ? input_data.getText() : input_data.getJson();
                text_query << input_str.length() << ", ";
            }
            else {
                text_query << "0, ";
            }

            if (output_data.isText() || output_data.isJson()) {
                std::string output_str = output_data.isText() ? output_data.getText() : output_data.getJson();
                text_query << output_str.length() << ", ";
            }
            else {
                text_query << "0, ";
            }

            if (input_data.isJson() || output_data.isJson()) {
                text_query << "'json')";
            }
            else {
                text_query << "'plain')";
            }

            LOG_DEBUG("插入文本记录: {}", text_query.str());

            if (mysql_query(&sql, text_query.str().c_str())) {
                error_message = "插入文本记录失败: " + std::string(mysql_error(&sql));
                LOG_ERROR("文本记录插入失败: {}", error_message);
                roll_back();
                return -1;
            }
        }

        if (!commit()) {
            LOG_ERROR("提交事务失败");
            roll_back();
            return -1;
        }

        LOG_DEBUG("历史记录插入完成，记录ID: {}", record_id);
        return record_id;

    }
    catch (const std::exception& e) {
        roll_back();
        error_message = "插入记录异常: " + std::string(e.what());
        LOG_ERROR("插入异常: {}", error_message);
        return -1;
    }
}
bool UsersDatabase::insertMatrixRecord(int64_t record_id, int64_t user_id,
    const UnifiedData& input_data,
    const UnifiedData& output_data) {
    std::stringstream query;
    query << "INSERT INTO matrix_records (record_id, user_id, ";

    if (input_data.isMatrix()) {
        if (input_data.isMatrixDense()) {
            const auto& matrix = input_data.getMatrixDense();
            query << "input_rows, input_cols, input_matrix_type, input_format, "
                << "input_min_value, input_max_value, input_mean_value, ";
        }
    }

    if (output_data.isMatrix()) {
        if (output_data.isMatrixDense()) {
            const auto& matrix = output_data.getMatrixDense();
            query << "output_rows, output_cols, output_matrix_type, output_format, "
                << "output_min_value, output_max_value, output_mean_value) VALUES ("
                << record_id << ", " << user_id << ", "
                << matrix.rows() << ", " << matrix.cols() << ", 'double', 'dense', "
                << matrix.minCoeff() << ", " << matrix.maxCoeff() << ", " << matrix.mean() << ")";
        }
    }

    if (mysql_query(&sql, query.str().c_str())) {
        error_message = "插入矩阵记录失败: " + std::string(mysql_error(&sql));
        return false;
    }

    return true;
}

bool UsersDatabase::insertTextRecord(int64_t record_id, int64_t user_id,
    const UnifiedData& input_data,
    const UnifiedData& output_data) {
    std::stringstream query;
    query << "INSERT INTO text_records (record_id, user_id, input_length, output_length, text_format) VALUES ("
        << record_id << ", " << user_id << ", ";

    if (input_data.isText() || input_data.isJson()) {
        std::string input_str = input_data.isText() ? input_data.getText() : input_data.getJson();
        query << input_str.length() << ", ";
    }
    else {
        query << "0, ";
    }

    if (output_data.isText() || output_data.isJson()) {
        std::string output_str = output_data.isText() ? output_data.getText() : output_data.getJson();
        query << output_str.length() << ", ";
    }
    else {
        query << "0, ";
    }

    if (input_data.isJson() || output_data.isJson()) {
        query << "'json')";
    }
    else {
        query << "'plain')";
    }

    if (mysql_query(&sql, query.str().c_str())) {
        error_message = "插入文本记录失败: " + std::string(mysql_error(&sql));
        return false;
    }

    return true;
}

UsersDatabase::HistoryPageResult UsersDatabase::getHistoryRecordsPaginated(int64_t user_id,
    int page,
    int page_size,
    const std::string& record_type) {
    start_transaction();
    HistoryPageResult result;

    // 计算偏移量
    int offset = (page - 1) * page_size;

    // 构建查询条件
    std::string where_clause = "user_id = " + std::to_string(user_id);
    if (!record_type.empty()) {
        where_clause += " AND record_type = '" + record_type + "'";
    }

    // 获取总记录数
    result.total_count = get_total_count("history_records", where_clause);
    if (result.total_count < 0) {
        roll_back();
        return result;
    }

    result.total_pages = (result.total_count + page_size - 1) / page_size;

    // 获取分页记录
    std::stringstream query;
    query << "SELECT h.id, h.user_id, h.record_type, h.input_data_type, h.output_data_type, "
        << "h.storage_method, h.input_data, h.output_data, h.input_external_ref, h.output_external_ref, "
        << "h.created_at, "
        // 联接matrix_records表获取矩阵信息
        << "m.input_rows, m.input_cols, m.input_format, m.output_rows, m.output_cols, m.output_format "
        << "FROM history_records h "
        << "LEFT JOIN matrix_records m ON h.id = m.record_id "
        << "WHERE h." << where_clause
        << " ORDER BY h.created_at DESC LIMIT " << page_size << " OFFSET " << offset;

    if (mysql_query(&sql, query.str().c_str())) {
        LOG_DEBUG("查询失败,{}", mysql_error(&sql));
        roll_back();
        return result;
    }

    MYSQL_RES* res = mysql_store_result(&sql);
    if (!res) {
        error_message = "获取查询结果失败: " + std::string(mysql_error(&sql));
        roll_back();
        LOG_DEBUG("获取查询结果失败");
        return result;
    }

    int num_fields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(res))) {
        LOG_DEBUG("result.records.push_back(historyRowToJson(row, fields, num_fields));");
        result.records.push_back(historyRowToJson(row, fields, num_fields));
    }

    mysql_free_result(res);

    if (!commit()) {
        roll_back();
        result.records.clear();
    }

    return result;
}

crow::json::wvalue UsersDatabase::getHistoryRecordWithData(int64_t record_id) {
    start_transaction();

    std::stringstream query;
    query << "SELECT h.*, m.*, t.* FROM history_records h "
        << "LEFT JOIN matrix_records m ON h.id = m.record_id "
        << "LEFT JOIN text_records t ON h.id = t.record_id "
        << "WHERE h.id = " << record_id;

    if (mysql_query(&sql, query.str().c_str())) {
        error_message = "查询记录失败: " + std::string(mysql_error(&sql));
        roll_back();
        return crow::json::wvalue();
    }

    MYSQL_RES* res = mysql_store_result(&sql);
    if (!res || mysql_num_rows(res) == 0) {
        if (res) mysql_free_result(res);
        roll_back();
        return crow::json::wvalue();
    }

    int num_fields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);
    MYSQL_ROW row = mysql_fetch_row(res);

    auto record = historyRowToJson(row, fields, num_fields);

    // 获取用户信息
    if (row[1]) { // user_id字段
        int64_t user_id = std::stoll(row[1]);
        // 查询用户信息
        char user_query[256];
        snprintf(user_query, sizeof(user_query), "SELECT username FROM info WHERE id = %ld", user_id);
        if (mysql_query(&sql, user_query) == 0) {
            MYSQL_RES* user_res = mysql_store_result(&sql);
            if (user_res && mysql_num_rows(user_res) > 0) {
                MYSQL_ROW user_row = mysql_fetch_row(user_res);
                record["username"] = user_row[0] ? user_row[0] : "";
            }
            mysql_free_result(user_res);
        }
    }

    // 如果是外部存储，加载二进制数据
    std::string storage_method = row[5] ? row[5] : ""; // storage_method字段位置
    if (storage_method == "external") {
        loadBinaryData(record_id, record);
    }

    mysql_free_result(res);

    if (!commit()) {
        roll_back();
        return crow::json::wvalue();
    }

    return record;
}

UnifiedData UsersDatabase::loadMatrixData(int64_t record_id, const std::string& data_type) {
    start_transaction();

    try {
        // 首先获取记录基本信息
        std::stringstream query;
        query << "SELECT input_data_type, output_data_type, storage_method, "
            << (data_type == "input" ? "input_data" : "output_data")
            << " FROM history_records WHERE id = " << record_id;

        if (mysql_query(&sql, query.str().c_str())) {
            error_message = "查询记录失败: " + std::string(mysql_error(&sql));
            roll_back();
            return UnifiedData("");
        }

        MYSQL_RES* res = mysql_store_result(&sql);
        if (!res || mysql_num_rows(res) == 0) {
            if (res) mysql_free_result(res);
            roll_back();
            return UnifiedData("");
        }

        MYSQL_ROW row = mysql_fetch_row(res);
        std::string input_type = row[0] ? row[0] : "";
        std::string output_type = row[1] ? row[1] : "";
        std::string storage_method = row[2] ? row[2] : "";

        std::string matrix_type = (data_type == "input") ? input_type : output_type;

        UnifiedData result("");

        // 根据存储方式加载数据
        if (storage_method == "inline") {
            // 从内联存储加载
            if (row[3] != nullptr) {
                // 获取BLOB数据长度
                unsigned long* lengths = mysql_fetch_lengths(res);
                if (lengths != nullptr && lengths[3] > 0) {
                    std::vector<uint8_t> buffer(lengths[3]);
                    memcpy(buffer.data(), row[3], lengths[3]);

                    if (matrix_type == "matrix_dense") {
                        Eigen::MatrixXd matrix = EigenMatrixSerializer::deserializeDense(buffer, false);
                        result = UnifiedData(matrix);
                    }
                    else if (matrix_type == "matrix_sparse") {
                        Eigen::SparseMatrix<double> matrix = EigenMatrixSerializer::deserializeSparse(buffer, false);
                        result = UnifiedData(matrix);
                    }
                }
            }
        }
        else {
            // 从外部存储加载
            mysql_free_result(res);

            std::stringstream binary_query;
            binary_query << "SELECT data, compressed FROM binary_storage WHERE record_id = "
                << record_id << " AND data_type = '" << data_type << "'";

            if (mysql_query(&sql, binary_query.str().c_str())) {
                roll_back();
                return UnifiedData("");
            }

            MYSQL_RES* binary_res = mysql_store_result(&sql);
            if (!binary_res || mysql_num_rows(binary_res) == 0) {
                if (binary_res) mysql_free_result(binary_res);
                roll_back();
                return UnifiedData("");
            }

            MYSQL_ROW binary_row = mysql_fetch_row(binary_res);
            if (binary_row[0] != nullptr) {
                unsigned long* lengths = mysql_fetch_lengths(binary_res);
                if (lengths != nullptr && lengths[0] > 0) {
                    std::vector<uint8_t> buffer(lengths[0]);
                    memcpy(buffer.data(), binary_row[0], lengths[0]);

                    bool compressed = binary_row[1] && std::string(binary_row[1]) == "1";

                    if (matrix_type == "matrix_dense") {
                        Eigen::MatrixXd matrix = EigenMatrixSerializer::deserializeDense(buffer, compressed);
                        result = UnifiedData(matrix);
                    }
                    else if (matrix_type == "matrix_sparse") {
                        Eigen::SparseMatrix<double> matrix = EigenMatrixSerializer::deserializeSparse(buffer, compressed);
                        result = UnifiedData(matrix);
                    }
                }
            }

            mysql_free_result(binary_res);
        }

        mysql_free_result(res);

        if (!commit()) {
            roll_back();
            return UnifiedData("");
        }

        return result;

    }
    catch (const std::exception& e) {
        roll_back();
        error_message = "加载矩阵数据异常: " + std::string(e.what());
        return UnifiedData("");
    }
}

// ==================== 私有辅助方法实现 ====================

std::string UsersDatabase::serializeForStorage(const UnifiedData& data) {
    if (data.isMatrix()) {
        // 序列化矩阵数据
        auto buffer = EigenMatrixSerializer::serialize(data, false); // 不压缩，因为这是内联存储

        // 转换为base64字符串存储
        return base64_encode(buffer.data(), buffer.size());
    }
    else if (data.isJson()) {
        return data.getJson();
    }
    else if (data.isText()) {
        return data.getText();
    }
    else {
        return "";
    }
}

void UsersDatabase::storeBinaryData(int64_t record_id, int64_t user_id, const std::string& data_type, const UnifiedData& data) {
    if (!data.isMatrix()) return;

    try {
        // 序列化矩阵数据（压缩存储）
        auto buffer = EigenMatrixSerializer::serialize(data, true);

        if (buffer.empty()) {
            return;
        }

        // 计算原始大小
        size_t original_size = 0;
        if (data.isMatrixDense()) {
            original_size = data.getMatrixDense().size() * sizeof(double);
        }
        else if (data.isMatrixSparse()) {
            original_size = data.getMatrixSparse().nonZeros() * (sizeof(double) + 2 * sizeof(int));
        }

        // 构建插入二进制数据的查询
        std::stringstream query;
        query << "INSERT INTO binary_storage (record_id, user_id, data_type, data, compressed, "
            << "original_size, compressed_size) VALUES ("
            << record_id << ", " << user_id << ", '" << data_type << "', '";

        // 转义二进制数据
        for (size_t i = 0; i < buffer.size(); ++i) {
            // 对二进制数据进行转义，确保SQL安全
            if (buffer[i] == '\'' || buffer[i] == '\\') {
                query << '\\';
            }
            query << buffer[i];
        }

        query << "', 1, " << original_size << ", " << buffer.size() << ")";

        if (mysql_query(&sql, query.str().c_str())) {
            error_message = "存储二进制数据失败: " + std::string(mysql_error(&sql));
            // 不抛出异常，因为主事务可能会回滚
        }

    }
    catch (const std::exception& e) {
        error_message = "序列化矩阵数据失败: " + std::string(e.what());
    }
}

void UsersDatabase::loadBinaryData(int64_t record_id, crow::json::wvalue& record) {
    std::stringstream query;
    query << "SELECT data_type, data, compressed FROM binary_storage WHERE record_id = " << record_id;

    if (mysql_query(&sql, query.str().c_str())) {
        return;
    }

    MYSQL_RES* res = mysql_store_result(&sql);
    if (!res) return;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        std::string data_type = row[0] ? row[0] : "";
        bool compressed = row[2] && std::string(row[2]) == "1";

        if (row[1] != nullptr) {
            unsigned long* lengths = mysql_fetch_lengths(res);
            if (lengths != nullptr && lengths[1] > 0) {
                std::vector<uint8_t> buffer(lengths[1]);
                memcpy(buffer.data(), row[1], lengths[1]);

                // 将数据转换为base64
                std::string base64_data = base64_encode(buffer.data(), buffer.size());

                if (data_type == "input") {
                    record["input_data_binary"] = base64_data;
                    record["input_compressed"] = compressed;
                }
                else {
                    record["output_data_binary"] = base64_data;
                    record["output_compressed"] = compressed;
                }
            }
        }
    }

    mysql_free_result(res);
}

// Base64编码实现
std::string UsersDatabase::base64_encode(const uint8_t* data, size_t length) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string ret;
    int i = 0;
    int j = 0;
    uint8_t char_array_3[3];
    uint8_t char_array_4[4];

    while (length--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; j < i + 1; j++)
            ret += base64_chars[char_array_4[j]];

        while (i++ < 3)
            ret += '=';
    }

    return ret;
}

// 添加一个Base64解码的辅助函数（用于从数据库读取数据时使用）
std::vector<uint8_t> UsersDatabase::base64_decode(const std::string& encoded_string) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::vector<uint8_t> ret;
    int i = 0;
    int j = 0;
    int in_ = 0;
    uint8_t char_array_4[4], char_array_3[3];

    size_t in_len = encoded_string.size();
    while (in_len-- && (encoded_string[in_] != '=')) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; j < i - 1; j++)
            ret.push_back(char_array_3[j]);
    }

    return ret;
}



// ==================== 私有辅助函数实现 ====================

bool UsersDatabase::isLargeMatrix(const UnifiedData& data) {
    if (!data.isMatrix()) return false;

    // 定义大矩阵的阈值（例如1MB）
    const size_t LARGE_MATRIX_THRESHOLD = 1024 * 1024;

    if (data.isMatrixDense()) {
        const auto& matrix = data.getMatrixDense();
        size_t estimated_size = matrix.rows() * matrix.cols() * sizeof(double);
        return estimated_size > LARGE_MATRIX_THRESHOLD;
    }
    else if (data.isMatrixSparse()) {
        const auto& matrix = data.getMatrixSparse();
        // 稀疏矩阵：非零元素数量 * (值大小 + 索引大小)
        size_t estimated_size = matrix.nonZeros() * (sizeof(double) + 2 * sizeof(int));
        return estimated_size > LARGE_MATRIX_THRESHOLD;
    }

    return false;
}

std::string UsersDatabase::determineRecordType(const UnifiedData& input, const UnifiedData& output) {
    // 如果输入或输出是矩阵，则记录类型为 "matrix"
    if (input.isMatrix() || output.isMatrix()) {
        return "matrix";
    }

    // 如果输入或输出是JSON，则记录类型为 "json"
    if (input.isJson() || output.isJson()) {
        return "json";
    }

    // 默认返回 "text"
    return "text";
}

std::string UsersDatabase::dataTypeToString(UnifiedData::DataType type) {
    switch (type) {
    case UnifiedData::DataType::JSON:
        return "json";
    case UnifiedData::DataType::MATRIX_DENSE:
        return "matrix";  // 统一返回 "matrix"
    case UnifiedData::DataType::MATRIX_SPARSE:
        return "matrix";  // 统一返回 "matrix"
    case UnifiedData::DataType::TEXT:
        return "text";
    default:
        return "text";    // 默认返回 "text"
    }
}
std::string UsersDatabase::getMatrixTypeString(const UnifiedData& data) {
    if (data.isMatrixDense()) {
        return "dense";
    }
    else if (data.isMatrixSparse()) {
        return "sparse";
    }
    return "unknown";
}

crow::json::wvalue UsersDatabase::buildInputMetadata(const UnifiedData& data) {
    crow::json::wvalue meta;

    if (data.isMatrixDense()) {
        const auto& matrix = data.getMatrixDense();
        meta["data_type"] = "matrix_dense";
        meta["rows"] = matrix.rows();
        meta["cols"] = matrix.cols();
        meta["element_count"] = matrix.size();
        meta["element_type"] = "double";
        meta["element_size"] = sizeof(double);
        meta["total_bytes"] = matrix.size() * sizeof(double);
        meta["storage_order"] = matrix.IsRowMajor ? "row_major" : "col_major";
        meta["is_large"] = isLargeMatrix(data);

        // 添加一些统计信息（可选）
        if (matrix.size() > 0) {
            double min_val = matrix.minCoeff();
            double max_val = matrix.maxCoeff();
            double sum_val = matrix.sum();
            meta["statistics"]["min"] = min_val;
            meta["statistics"]["max"] = max_val;
            meta["statistics"]["sum"] = sum_val;
            meta["statistics"]["average"] = sum_val / matrix.size();
        }

    }
    else if (data.isMatrixSparse()) {
        const auto& matrix = data.getMatrixSparse();
        meta["data_type"] = "matrix_sparse";
        meta["rows"] = matrix.rows();
        meta["cols"] = matrix.cols();
        meta["non_zeros"] = matrix.nonZeros();
        meta["sparsity"] = 1.0 - (static_cast<double>(matrix.nonZeros()) / (matrix.rows() * matrix.cols()));
        meta["element_type"] = "double";
        meta["element_size"] = sizeof(double);
        meta["estimated_bytes"] = matrix.nonZeros() * (sizeof(double) + 2 * sizeof(int));
        meta["is_large"] = isLargeMatrix(data);

        // 稀疏矩阵的统计信息
        if (matrix.nonZeros() > 0) {
            double min_val = std::numeric_limits<double>::max();
            double max_val = std::numeric_limits<double>::lowest();
            double sum_val = 0.0;

            for (int k = 0; k < matrix.outerSize(); ++k) {
                for (Eigen::SparseMatrix<double>::InnerIterator it(matrix, k); it; ++it) {
                    double val = it.value();
                    min_val = std::min(min_val, val);
                    max_val = std::max(max_val, val);
                    sum_val += val;
                }
            }

            meta["statistics"]["min"] = min_val;
            meta["statistics"]["max"] = max_val;
            meta["statistics"]["sum"] = sum_val;
            meta["statistics"]["average"] = sum_val / matrix.nonZeros();
        }

    }
    else if (data.isJson()) {
        const auto& json_str = data.getJson();
        meta["data_type"] = "json";
        meta["length"] = json_str.length();
        meta["estimated_bytes"] = json_str.length();

        try {
            auto json_obj = crow::json::load(json_str);
            if (json_obj) {
                meta["is_valid_json"] = true;
                // 可以添加更多JSON特定的元数据
            }
            else {
                meta["is_valid_json"] = false;
            }
        }
        catch (...) {
            meta["is_valid_json"] = false;
        }

    }
    else if (data.isText()) {
        const auto& text = data.getText();
        meta["data_type"] = "text";
        meta["length"] = text.length();
        meta["estimated_bytes"] = text.length();

        // 文本数据的简单分析
        size_t line_count = 0;
        size_t word_count = 0;
        std::istringstream iss(text);
        std::string line;

        while (std::getline(iss, line)) {
            line_count++;
            std::istringstream line_stream(line);
            std::string word;
            while (line_stream >> word) {
                word_count++;
            }
        }

        meta["line_count"] = line_count;
        meta["word_count"] = word_count;

    }
    else {
        meta["data_type"] = "unknown";
    }

    // 如果有自定义元数据，也包含进来
    if (!data.getMetadata().empty()) {
        try {
            meta["custom_metadata"] = crow::json::load(data.getMetadata());
        }
        catch (...) {
            meta["custom_metadata"] = data.getMetadata();
        }
    }

    return meta;
}

crow::json::wvalue UsersDatabase::buildOutputMetadata(const UnifiedData& data) {
    // 输出数据的元数据构建与输入数据类似
    auto meta = buildInputMetadata(data);

    // 可以为输出数据添加一些特定的元数据
    meta["is_output"] = true;

    return meta;
}

crow::json::wvalue UsersDatabase::historyRowToJson(MYSQL_ROW row, MYSQL_FIELD* fields, int num_fields) {
    crow::json::wvalue record;

    if (!row) {
        LOG_WARN("historyRowToJson: row为空");
        return record;
    }

    LOG_DEBUG("开始转换记录，字段数: {}", num_fields);

    for (int i = 0; i < num_fields; i++) {
        std::string field_name = fields[i].name;
        std::string field_value = row[i] ? row[i] : "NULL";

        LOG_DEBUG("字段[{}]: {} = {}", i, field_name, field_value);

        if (row[i] == nullptr) {
            record[field_name] = nullptr;
            continue;
        }

        // 根据字段名进行类型转换
        if (field_name == "id" || field_name == "user_id") {
            try {
                record[field_name] = std::stoll(row[i]);
            }
            catch (...) {
                record[field_name] = row[i];
            }
        }
        else if (field_name == "created_at") {
            record[field_name] = row[i];
        }
        else if (field_name == "record_type" ||
            field_name == "input_data_type" ||
            field_name == "output_data_type" ||
            field_name == "storage_method") {
            record[field_name] = row[i];
        }
        else {
            // 默认作为字符串处理
            record[field_name] = row[i];
        }
    }

    LOG_DEBUG("记录转换完成");
    return record;
}