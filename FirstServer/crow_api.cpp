#include"crow_api.h"
#include"logger.h"
#include"users_database.h"
#include<regex>
#include"jwt_service.h"
#include"ProcessPool.h"


std::vector<std::string> messages = std::vector<std::string>();

struct CORSMiddleware {
	struct context {};

	void before_handle(crow::request& req, crow::response& res, context& ctx) {
		// 空实现 - 在请求处理前不需要做任何事情
	}

	void after_handle(crow::request& req, crow::response& res, context& ctx) {
		res.add_header("Access-Control-Allow-Origin", "*");
		res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
		res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");

		std::string content_type = res.get_header_value("Content-Type");
		if (content_type.empty()) {
			// 如果没有设置Content-Type，默认为JSON并添加UTF-8编码
			res.add_header("Content-Type", "application/json; charset=utf-8");
		}
		else if (content_type.find("application/json") != std::string::npos &&
			content_type.find("charset") == std::string::npos) {
			// 如果是JSON但没有指定编码，添加UTF-8编码
			res.set_header("Content-Type", "application/json; charset=utf-8");
		}
		else if (content_type.find("text/") != std::string::npos &&
			content_type.find("charset") == std::string::npos) {
			// 如果是文本类型但没有指定编码，添加UTF-8编码
		}
		res.set_header("Content-Type", content_type + "; charset=utf-8");
	}
};


void run_crow_server(int port)
{

	ProcessPool* process_pool = ProcessPool::getInstance();
	std::shared_ptr<UsersDatabase> db = std::make_shared<UsersDatabase>("192.168.10.1", "cpp_user", "Wyz20050817.", "users", 3306);
	LOG_DEBUG("数据库连接初始化完成");
	crow::App<CORSMiddleware> app;
	JWTService jwt_service("your_secret_key_here"); //使用更为安全的密钥

	CROW_ROUTE(app, "/api/health")
		.methods("GET"_method)
		([]() {
		crow::json::wvalue response;
		response["status"] = "healthy";
		return OK(response);
			});

	// 获取所有消息
	CROW_ROUTE(app, "/api/messages")
		.methods("GET"_method)
		([]() {
		crow::json::wvalue response;
		response["messages"] = messages;
		return OK(response);
			});

	//注册 
	/*
		字段：
		username,
		password,
		email //验证码机制

	*/

	/*
		返回的请求体
		success
		error_message

		jwt_token
	*/
	CROW_ROUTE(app, "/api/auth/register")
		.methods("POST"_method)
		([&](const crow::request& req) {
		LOG_DEBUG("收到register请求");


		//解析请求体
		try {
			auto body = crow::json::load(req.body);
			if (!body) {
				return  ERROR_REQUEST("{\"error\": \"Invalid JSON\"}");
			}

			if (!body.has("name")) {
				return  ERROR_REQUEST("{\"error\": \"Missing username field\"}");
			}

			std::string username = body["name"].s();

			//TODO email 格式验证
			if (!body.has("email")) {
				return  ERROR_REQUEST("{\"error\": \"Missing email field\"}");
			}

			std::string email = body["email"].s();

			{
				std::regex email_pattern(R"(\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Z|a-z]{2,}\b)");

				if (std::regex_match(email, email_pattern))
				{
					LOG_DEBUG("邮箱格式正确: {}", email);
				}
				else
				{
					LOG_DEBUG("邮箱格式错误: {}", email);
					return  ERROR_REQUEST("{\"error\": \"Invalid email format\"}");

				}
			}

			if (!body.has("password")) {
				return  ERROR_REQUEST("{\"error\": \"Missing password field\"}");
			}
			std::string password = body["password"].s();

			if (db->register_user(username, password, email))
			{
				//LOG_DEBUG("用户 {} 注册成功", username);
			}
			else
			{
				//LOG_DEBUG("用户 {} 注册失败: {}", username, db->error_mes());
				crow::json::wvalue error_response;
				error_response["success"] = false;
				error_response["message"] = db->error_mes();
				return  OK(error_response);
			}


			crow::json::wvalue response;
			response["success"] = true;

			return OK(response);
		}
		catch (const std::exception& e) {
			return SERVER_ERROR;
		}
			});

	/*
		登录
		请求体：
		email,
		password
	*/

	CROW_ROUTE(app, "/api/auth/login")
		.methods("POST"_method)
		([&](const crow::request& req) {
		//LOG_DEBUG("收到login请求");


		//解析请求体
		try {
			auto body = crow::json::load(req.body);
			if (!body) {
				LOG_DEBUG("解析JSON失败");
				return  ERROR_REQUEST("{\"error\": \"Invalid JSON\"}");
			}

			/*if (!body.has("name")) {
				return  ERROR_REQUEST("{\"error\": \"Missing username field\"}");
			}*/



			/*if (!body.has("email")) {
				return  ERROR_REQUEST("{\"error\": \"Missing email field\"}");
			}*/
			if (!body.has("username")) {
				LOG_DEBUG("缺少用户名字段");
				return  ERROR_REQUEST("{\"error\": \"Missing username field\"}");
			}
			/*else
			{
				std::regex email_pattern(R"(\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-])");
				std::string email = body["email"].s();
				if (std::regex_match(email, email_pattern))
				{
					LOG_DEBUG("邮箱格式正确: {}", email);
				}
				else
				{
					LOG_DEBUG("邮箱格式错误: {}", email);
					return  ERROR_REQUEST("{\"error\": \"Invalid email format\"}");

				}
			}*/

			if (!body.has("password")) {
				LOG_DEBUG("缺少密码字段");
				return  ERROR_REQUEST("{\"error\": \"Missing password field\"}");
			}

			bool success = db->login_username(body["username"].s(), body["password"].s());
			std::string username = body["username"].s();
			std::string password = body["password"].s();
			if (success)
			{
				LOG_DEBUG("用户 {} 登录成功", username);
			}
			else
			{
				LOG_DEBUG("用户 {} 登录失败: {}", username, db->error_mes());
				crow::json::wvalue error_response;
				error_response["success"] = false;
				error_response["message"] = db->error_mes();
				return  OK(error_response);
			}

			int64_t user_id = db->get_user_id(body["username"].s());

			std::string ip = process_pool->GetProcessIP();
			crow::json::wvalue response;
			response["success"] = true;
			response["username"] = body["username"].s();
			response["token"] = jwt_service.generateToken(user_id, body["username"].s());
			response["ip"] = "http://" + ip + ":8081";

			return OK(response);
		}
		catch (const std::exception& e) {
			return SERVER_ERROR;
		}
			});

	CROW_ROUTE(app, "/api/auth/add_records")
		.methods("POST"_method)
		([&](const crow::request& req) {
		LOG_DEBUG("收到add Records请求");


		//解析请求体
		try {

			std::string auth_header = req.get_header_value("Authorization");

			if (auth_header.empty() || auth_header.find("Bearer ") == std::string::npos)
			{
				crow::json::wvalue response;
				response["success"] = false;
				response["message"] = "缺少认证token";
				return UNAUTHORIZED(response);

			}
			std::string token = auth_header.substr(7);

			/*if (!body.has("name")) {
				return  ERROR_REQUEST("{\"error\": \"Missing username field\"}");
			}*/

			std::string username;
			int64_t user_id;

			if (jwt_service.verifyToken(token, user_id, username))
			{
				LOG_DEBUG("Token 验证成功，用户: {} 请求获取历史记录", username);
			}

			else
			{
				LOG_DEBUG("Token 验证失败");
				return UNAUTHORIZED("{\"error\": \"Invalid or expired token\"}");
			}





			crow::json::wvalue response;
			response["success"] = true;
			return OK(response);
		}
		catch (const std::exception& e) {
			return SERVER_ERROR;
		}
			});

	//登录之后用token解析
	CROW_ROUTE(app, "/api/auth/get_records")
		.methods("GET"_method)
		([&](const crow::request& req) {
		LOG_DEBUG("收到get Records请求");


		//解析请求体
		try {

			std::string auth_header = req.get_header_value("Authorization");

			if (auth_header.empty() || auth_header.find("Bearer ") == std::string::npos)
			{
				crow::json::wvalue response;
				response["success"] = false;
				response["message"] = "缺少认证token";
				return UNAUTHORIZED(response);

			}
			std::string token = auth_header.substr(7);

			/*if (!body.has("name")) {
				return  ERROR_REQUEST("{\"error\": \"Missing username field\"}");
			}*/

			std::string username;
			int64_t user_id;

			if (jwt_service.verifyToken(token, user_id, username))
			{
				LOG_DEBUG("Token 验证成功，用户: {} 请求获取历史记录", username);
			}

			else
			{
				LOG_DEBUG("Token 验证失败");
				return UNAUTHORIZED("{\"error\": \"Invalid or expired token\"}");
			}

			auto& param = req.url_params;

			Database::PageRequest page_request;
			page_request.page = param.get("page") == nullptr ? 1 : std::stoi(param.get("page"));
			page_request.limit = param.get("limit") == nullptr ? 10 : std::stoi(param.get("limit"));
			LOG_DEBUG("页面请求信息,{},{}", page_request.page, page_request.limit);

			auto page_response = db->get_calculation_records(user_id, page_request);
			auto response = page_response.to_json();
			if (page_response.success)
			{
				LOG_DEBUG("获取历史记录成功，用户: {} ", username);
			}
			else
			{
				LOG_DEBUG("获取历史记录失败，用户: {} ,错误信息: {}", username, db->error_mes());
			}

			return OK(response);
		}
		catch (const std::exception& e) {
			return SERVER_ERROR;
		}
			});

	CROW_ROUTE(app, "/api/get_history")
		.methods("GET"_method)
		([&](const crow::request& req) {
		LOG_DEBUG("收到获取历史记录列表请求");

		try {
			// 验证JWT token
			std::string auth_header = req.get_header_value("Authorization");
			if (auth_header.empty() || auth_header.find("Bearer ") == std::string::npos) {
				crow::json::wvalue response;
				response["success"] = false;
				response["message"] = "缺少认证token";
				return UNAUTHORIZED(response);
			}

			std::string token = auth_header.substr(7);
			int64_t user_id;
			std::string username;

			if (!jwt_service.verifyToken(token, user_id, username)) {
				LOG_DEBUG("Token 验证失败");
				return UNAUTHORIZED("{\"error\": \"Invalid or expired token\"}");
			}

			LOG_DEBUG("Token 验证成功，用户: {} (ID: {}) 请求获取历史记录", username, user_id);

			// 解析查询参数
			auto& params = req.url_params;

			// 不再需要session_id，使用user_id
			int page = params.get("page") ? std::stoi(params.get("page")) : 1;
			int page_size = params.get("page_size") ? std::stoi(params.get("page_size")) : 20;
			std::string record_type = params.get("record_type") ? params.get("record_type") : "";
			std::string operation = params.get("operation") ? params.get("operation") : "";
			std::string date_from = params.get("date_from") ? params.get("date_from") : "";
			std::string date_to = params.get("date_to") ? params.get("date_to") : "";


					LOG_DEBUG("查询参数 - user_id: {}, page: {}, page_size: {}, record_type: {}",
						user_id, page, page_size, record_type);

					// 获取分页结果
					auto result = db->getHistoryRecordsPaginated(user_id, page, page_size, record_type);

					LOG_DEBUG("查询结果 - 总记录数: {}, 总页数: {}, 当前页记录数: {}",
						result.total_count, result.total_pages, result.records.size());

					// 检查是否有记录
					if (result.records.empty() && result.total_count > 0) {
						LOG_WARN("有 {} 条总记录，但当前页返回了 0 条记录", result.total_count);
						LOG_WARN("可能原因: 1) 查询条件错误 2) 分页偏移量错误 3) 数据转换失败");
					}

					// 构建响应
					crow::json::wvalue response;
					response["success"] = true;
					response["records"] = crow::json::wvalue(std::move(result.records));
					response["pagination"] = {
						{"current_page", page},
						{"page_size", page_size},
						{"total_count", result.total_count},
						{"total_pages", result.total_pages}
					};

					LOG_DEBUG("成功获取历史记录，共 {} 条记录", result.total_count);
					return OK(response);
				}
				catch (const std::exception& e) {
					LOG_ERROR("获取历史记录列表失败: {}", e.what());
					return SERVER_ERROR;
				}
			});

	// 获取单个历史记录的完整数据
	CROW_ROUTE(app, "/api/get_history/<int>")
		.methods("GET"_method)
		([&](const crow::request& req, int record_id) {
		LOG_DEBUG("收到获取单个历史记录请求，记录ID: {}", record_id);

		try {
			// 验证JWT token
			std::string auth_header = req.get_header_value("Authorization");
			if (auth_header.empty() || auth_header.find("Bearer ") == std::string::npos) {
				crow::json::wvalue response;
				response["success"] = false;
				response["message"] = "缺少认证token";
				return UNAUTHORIZED(response);
			}

			std::string token = auth_header.substr(7);
			int64_t user_id;
			std::string username;

			if (!jwt_service.verifyToken(token, user_id, username)) {
				LOG_DEBUG("Token 验证失败");
				return UNAUTHORIZED("{\"error\": \"Invalid or expired token\"}");
			}

			LOG_DEBUG("Token 验证成功，用户: {} 请求获取记录ID: {}", username, record_id);

			// 获取记录详情
			auto record = db->getHistoryRecordWithData(record_id);


			crow::json::wvalue response;
			response["success"] = true;
			response["record"] = std::move(record);

			LOG_DEBUG("成功获取记录ID: {} 的详细信息", record_id);
			return OK(response);

		}
		catch (const std::exception& e) {
			LOG_ERROR("获取历史记录详情失败: {}", e.what());
			return SERVER_ERROR;
		}
			});

	// 下载历史记录中的矩阵数据
	CROW_ROUTE(app, "/api/download_history/<int>/matrix/<string>")
		.methods("GET"_method)
		([&](const crow::request& req, int record_id, const std::string& data_type) {
		LOG_DEBUG("收到下载矩阵数据请求，记录ID: {}, 数据类型: {}", record_id, data_type);

		try {
			// 验证JWT token
			std::string auth_header = req.get_header_value("Authorization");
			if (auth_header.empty() || auth_header.find("Bearer ") == std::string::npos) {
				crow::json::wvalue response;
				response["success"] = false;
				response["message"] = "缺少认证token";
				return UNAUTHORIZED(response);
			}

			std::string token = auth_header.substr(7);
			int64_t user_id;
			std::string username;

			if (!jwt_service.verifyToken(token, user_id, username)) {
				LOG_DEBUG("Token 验证失败");
				return UNAUTHORIZED("{\"error\": \"Invalid or expired token\"}");
			}

			LOG_DEBUG("Token 验证成功，用户: {} 请求下载记录ID: {} 的矩阵数据 {}",
				username, record_id, data_type);

			// 验证数据类型
			if (data_type != "input" && data_type != "output") {
				LOG_DEBUG("无效的数据类型: {}", data_type);
				crow::json::wvalue response;
				response["success"] = false;
				response["message"] = "Invalid data type, use 'input' or 'output'";
				return ERROR_REQUEST(response);
			}

			// 加载矩阵数据
			auto matrix_data = db->loadMatrixData(record_id, data_type);
			if (!matrix_data.isMatrix()) {
				LOG_DEBUG("记录ID: {} 的矩阵数据未找到", record_id);
				crow::json::wvalue response;
				response["success"] = false;
				response["message"] = "Matrix data not found";
				return NOT_FOUND(response);
			}

			// 序列化为二进制
			auto buffer = EigenMatrixSerializer::serialize(matrix_data, false);
			if (buffer.empty()) {
				LOG_ERROR("序列化矩阵数据失败");
				crow::json::wvalue response;
				response["success"] = false;
				response["message"] = "Failed to serialize matrix data";
				return SERVER_ERROR;
			}

			// 创建文件下载响应
			auto response = crow::response(std::string(buffer.begin(), buffer.end()));
			response.set_header("Content-Type", "application/octet-stream");
			response.set_header("Content-Disposition",
				"attachment; filename=\"matrix_" +
				std::to_string(record_id) + "_" + data_type + ".bin\"");
			response.set_header("Cache-Control", "no-cache");

			LOG_DEBUG("成功下载记录ID: {} 的矩阵数据 {}", record_id, data_type);
			return response;

		}
		catch (const std::exception& e) {
			LOG_ERROR("下载矩阵数据失败: {}", e.what());
			return SERVER_ERROR;
		}
			});

	CROW_ROUTE(app, "/api/auth/add_history")
		.methods("POST"_method)
		([&](const crow::request& req) {
		LOG_DEBUG("收到创建历史记录请求");

		try {
			// 验证JWT token
			std::string auth_header = req.get_header_value("Authorization");
			if (auth_header.empty() || auth_header.find("Bearer ") == std::string::npos) {
				crow::json::wvalue response;
				response["success"] = false;
				response["message"] = "缺少认证token";
				return UNAUTHORIZED(response);
			}

			std::string token = auth_header.substr(7);
			int64_t user_id;
			std::string username;

			if (!jwt_service.verifyToken(token, user_id, username)) {
				LOG_DEBUG("Token 验证失败");
				return UNAUTHORIZED("{\"error\": \"Invalid or expired token\"}");
			}

			LOG_DEBUG("Token 验证成功，用户: {} (ID: {}) 请求创建历史记录", username, user_id);

			// 解析请求体
			auto json_body = crow::json::load(req.body);
			if (!json_body) {
				LOG_DEBUG("解析JSON失败");
				crow::json::wvalue response;
				response["success"] = false;
				response["message"] = "Invalid JSON";
				return ERROR_REQUEST(response);
			}

			// 解析输入输出数据
			UnifiedData input_data("");
			UnifiedData output_data("");

			try {
				input_data = parseInputData(json_body);
				output_data = parseOutputData(json_body);
			}
			catch (const std::exception& e) {
				LOG_ERROR("解析输入输出数据失败: {}", e.what());
				crow::json::wvalue response;
				response["success"] = false;
				response["message"] = "Failed to parse input/output data: " + std::string(e.what());
				return ERROR_REQUEST(response);
			}

			// 从请求中获取操作类型
			std::string operation = "unknown";
			if (json_body.has("operation")) {
				operation = json_body["operation"].s();
			}
			else {
				// 如果没有operation字段，根据输入数据类型推断
				if (input_data.isMatrix() && output_data.isMatrix()) {
					operation = "matrix_operation";
				}
				else if (input_data.isText() || output_data.isText()) {
					operation = "text_operation";
				}
				else if (input_data.isJson() || output_data.isJson()) {
					operation = "json_operation";
				}
			}

			LOG_DEBUG("创建历史记录 - 用户: {}, 操作: {}, 输入类型: {}, 输出类型: {}",
				username, operation,
				input_data.isMatrix() ? "matrix" : (input_data.isJson() ? "json" : "text"),
				output_data.isMatrix() ? "matrix" : (output_data.isJson() ? "json" : "text"));

			// 插入历史记录 - 只传递操作类型
			int64_t record_id = db->insertHistoryRecord(user_id, input_data, output_data, operation);

			if (record_id == -1) {
				LOG_ERROR("插入历史记录失败: {}", db->error_mes());
				crow::json::wvalue response;
				response["success"] = false;
				response["message"] = db->error_mes();
				return SERVER_ERROR;
			}

			crow::json::wvalue response;
			response["success"] = true;
			response["record_id"] = record_id;
			response["message"] = "Record created successfully";

			LOG_DEBUG("成功创建历史记录，记录ID: {}", record_id);
			return CREATED(response);

		}
		catch (const std::exception& e) {
			LOG_ERROR("创建历史记录失败: {}", e.what());
			crow::json::wvalue response;
			response["success"] = false;
			response["message"] = std::string("Error: ") + e.what();
			return SERVER_ERROR;
		}
			});


			CROW_ROUTE(app, "/api/auth/logout")
				.methods("POST"_method)
				([&](const crow::request& req) {
				LOG_DEBUG("收到logout请求");

				std::string auth_header = req.get_header_value("Authorization");

				if (auth_header.empty() || auth_header.find("Bearer ") == std::string::npos)
				{
					crow::json::wvalue response;
					response["success"] = false;
					response["message"] = "缺少认证token";
					return UNAUTHORIZED(response);
				}


				std::string token = auth_header.substr(7);
				crow::json::wvalue response;

				int64_t user_id;
				std::string user_name;

				if (jwt_service.verifyToken(token, user_id, user_name))
				{
					LOG_DEBUG("Token 验证成功，用户: {} 登出", user_name);
					db->logout_username(user_name);

					response["success"] = true;
					return OK(response);
				}
				else
				{
					LOG_DEBUG("Token 验证失败");
					return UNAUTHORIZED("{\"error\": \"Invalid or expired token\"}");
				}

					});

	std::cout << "==========================================" << std::endl;
	std::cout << "🚀 CORS 服务器已启动" << std::endl;
	std::cout << "📍 地址: http://localhost:8080" << std::endl;
	std::cout << "🔧 已启用 CORS 中间件支持" << std::endl;
	std::cout << "==========================================" << std::endl;

	app.port(port).multithreaded().run();
}


UnifiedData parseInputData(const crow::json::rvalue& json) {
	if (json.has("input_matrix_dense")) {
		// 处理稠密矩阵数据
		const auto& matrix_json = json["input_matrix_dense"];
		Eigen::MatrixXd matrix = jsonToMatrix(matrix_json);
		return UnifiedData(matrix);
	}
	else if (json.has("input_matrix_sparse")) {
		// 处理稀疏矩阵数据
		const auto& matrix_json = json["input_matrix_sparse"];
		Eigen::SparseMatrix<double> matrix = jsonToSparseMatrix(matrix_json);
		return UnifiedData(matrix);
	}
	else if (json.has("input_json")) {
		return UnifiedData(json["input_json"].s());
	}
	else if (json.has("input_text")) {
		return UnifiedData(json["input_text"].s());
	}
	else {
		throw std::runtime_error("No input data provided");
	}
}

UnifiedData parseOutputData(const crow::json::rvalue& json) {
	if (json.has("output_matrix_dense")) {
		const auto& matrix_json = json["output_matrix_dense"];
		Eigen::MatrixXd matrix = jsonToMatrix(matrix_json);
		return UnifiedData(matrix);
	}
	else if (json.has("output_matrix_sparse")) {
		const auto& matrix_json = json["output_matrix_sparse"];
		Eigen::SparseMatrix<double> matrix = jsonToSparseMatrix(matrix_json);
		return UnifiedData(matrix);
	}
	else if (json.has("output_json")) {
		return UnifiedData(json["output_json"].s());
	}
	else if (json.has("output_text")) {
		return UnifiedData(json["output_text"].s());
	}
	else {
		throw std::runtime_error("No output data provided");
	}
}

// JSON到Eigen矩阵的转换
Eigen::MatrixXd jsonToMatrix(const crow::json::rvalue& json) {

	int rows = json.size();
	if (rows == 0) return Eigen::MatrixXd(0, 0);

	int cols = json[0].size();
	Eigen::MatrixXd matrix(rows, cols);

	for (int i = 0; i < rows; i++) {
		if (json[i].size() != cols) {
			throw std::runtime_error("Inconsistent matrix dimensions");
		}

		for (int j = 0; j < cols; j++) {
			matrix(i, j) = json[i][j].d();
		}
	}

	return matrix;
}

// JSON到Eigen稀疏矩阵的转换
Eigen::SparseMatrix<double> jsonToSparseMatrix(const crow::json::rvalue& json) {
	if (!json.has("rows") || !json.has("cols") || !json.has("data")) {
		throw std::runtime_error("Sparse matrix must have rows, cols, and data fields");
	}

	int rows = json["rows"].i();
	int cols = json["cols"].i();
	const auto& data_json = json["data"];

	std::vector<Eigen::Triplet<double>> triplets;

	for (size_t i = 0; i < data_json.size(); i++) {
		const auto& item = data_json[i];
		if (!item.has("row") || !item.has("col") || !item.has("value")) {
			throw std::runtime_error("Sparse matrix data item must have row, col, and value");
		}

		int row = item["row"].i();
		int col = item["col"].i();
		double value = item["value"].d();

		triplets.emplace_back(row, col, value);
	}

	Eigen::SparseMatrix<double> matrix(rows, cols);
	matrix.setFromTriplets(triplets.begin(), triplets.end());
	return matrix;
}

// Eigen矩阵到JSON的转换
crow::json::wvalue matrixToJson(const Eigen::MatrixXd& matrix) {
	crow::json::wvalue json;

	// 添加维度信息
	json["rows"] = matrix.rows();
	json["cols"] = matrix.cols();
	json["type"] = "dense";

	// 添加数据
	auto& data_json = json["data"];
	for (int i = 0; i < matrix.rows(); i++) {
		auto row_json = crow::json::wvalue::list();
		for (int j = 0; j < matrix.cols(); j++) {
			row_json[j] = matrix(i, j);
		}
		data_json[i] = std::move(row_json);
	}

	return json;
}

// Eigen稀疏矩阵到JSON的转换
crow::json::wvalue sparseMatrixToJson(const Eigen::SparseMatrix<double>& matrix) {
	crow::json::wvalue json;

	// 添加维度信息
	json["rows"] = matrix.rows();
	json["cols"] = matrix.cols();
	json["non_zeros"] = matrix.nonZeros();
	json["type"] = "sparse";

	// 添加数据（COO格式）
	auto& data_json = json["data"];
	int index = 0;

	for (int k = 0; k < matrix.outerSize(); ++k) {
		for (Eigen::SparseMatrix<double>::InnerIterator it(matrix, k); it; ++it) {
			crow::json::wvalue item;
			item["row"] = it.row();
			item["col"] = it.col();
			item["value"] = it.value();
			data_json[index++] = std::move(item);
		}
	}

	return json;
}