/*
jwt-token的认证和获取
*/

#pragma once
#include <jwt-cpp/jwt.h>
#include <string>
#include <chrono>


class JWTService {
private:
    std::string secret_key_; //TODO : 轮换机制
    std::chrono::seconds token_expiry_;

public:
    JWTService(const std::string& secret_key, std::chrono::seconds expiry = std::chrono::hours(24))
        : secret_key_(secret_key), token_expiry_(expiry) {}

    // 生成JWT token
    std::string generateToken(int64_t user_id, const std::string& username);


    // 验证JWT token
    bool verifyToken(const std::string& token, int64_t& user_id, std::string& username); 

    // 从HTTP请求中提取token
    std::string extractTokenFromHeader(const std::string& auth_header);
};

