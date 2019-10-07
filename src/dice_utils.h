#pragma once
#include <string>
#include "cqsdk/cqsdk.h"
#include "cqsdk/types.h"

namespace dice::utils {

    // 昵称获取 群
    std::string get_nickname(const int64_t group_id, const int64_t user_id);

    // 昵称获取 讨论组/私聊
    std::string get_nickname(const int64_t user_id);

    // 昵称获取 综合
    std::string get_nickname(const cq::Target& target);

    // 格式化字符串
    std::string format_string(const std::string& origin_str, const std::map<std::string, std::string>& format_para);

	// 获取权限 群
    bool is_admin_or_owner(const int64_t group_id, const int64_t user_id);

	// 获取权限 综合 (非群返回永远为真)
    bool is_admin_or_owner(const cq::Target& target);
}
