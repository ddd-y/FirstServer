#include"jwt_service.h"
#include"logger.h"

std::string JWTService::generateToken(int64_t user_id, const std::string& username)
{
    auto now = std::chrono::system_clock::now();
    auto expire_time = now + token_expiry_;

    auto token = jwt::create()
        .set_issuer("linear-algebra-platform")
        .set_type("JWS")
        .set_issued_at(now)
        .set_expires_at(expire_time)
        .set_payload_claim("user_id", jwt::claim(std::to_string(user_id)))
        .set_payload_claim("username", jwt::claim(username))
        .sign(jwt::algorithm::hs256{ secret_key_ });

    LOG_DEBUG("Generated JWT token for user_id: {}, username: {}", user_id, username);
    return token;
}

bool JWTService::verifyToken(const std::string& token, int64_t& user_id, std::string& username)
{
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{ secret_key_ })
            .with_issuer("linear-algebra-platform");

        verifier.verify(decoded);

        user_id = std::stoll(decoded.get_payload_claim("user_id").as_string());
        username = decoded.get_payload_claim("username").as_string();


        LOG_DEBUG("Verified JWT token for user_id: {}, username: {}", user_id, username);
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("JWT verification failed: {}", e.what());
        LOG_DEBUG("JWT verification failed: {}", e.what());
        return false;
    }
}

std::string JWTService::extractTokenFromHeader(const std::string& auth_header)
{
    // Bearer token∏Ò Ω: "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
    if (auth_header.find("Bearer ") == 0) {
        return auth_header.substr(7);
    }
    return "";
}

