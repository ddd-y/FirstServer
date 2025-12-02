#pragma once

#include<crow.h>
#include<string>
#include"to_json.h"
#include"unified_data.h"
#include"eigen_matrix_serializer.h"


// 成功状态码
#define OK(response_) crow::response(200, response_);                    // 成功
#define CREATED(response_) crow::response(201, response_);               // 创建成功

 // 客户端错误状态码
#define ERROR_REQUEST(response_) crow::response(400, response_);           // 错误请求
#define UNAUTHORIZED(response_) crow::response(401, response_);          // 未授权
#define FORBIDDEN(response_) crow::response(403, response_);             // 禁止访问
#define NOT_FOUND(response_) crow::response(404, response_);             // 未找到
#define CONFLICT(response_) crow::response(409, response_);              // 冲突

 // 服务器错误状态码
#define SERVER_ERROR crow::response(500, "Internal Server Error"); // 服务器内部错误


// 跨域资源共享（CORS）中间件 跨域连接

struct CORSMiddleware;
extern std::vector<std::string> messages;

UnifiedData parseInputData(const crow::json::rvalue& json);
UnifiedData parseOutputData(const crow::json::rvalue& json);
Eigen::MatrixXd jsonToMatrix(const crow::json::rvalue& json);
Eigen::SparseMatrix<double> jsonToSparseMatrix(const crow::json::rvalue& json);
crow::json::wvalue matrixToJson(const Eigen::MatrixXd& matrix);
crow::json::wvalue sparseMatrixToJson(const Eigen::SparseMatrix<double>& matrix);




void run_crow_server(int port);
