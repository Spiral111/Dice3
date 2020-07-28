/*
机器人控制
注释：陈末
*/

#include "dice_bot_module.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "cqsdk/cqsdk.h"
#include "dice_db.h"
#include "dice_msg.h"
#include "dice_utils.h"
#include "dice_msg_queue.h"

namespace cq::event 
{
    struct MessageEvent;
}

namespace dice 
{
    //匹配正则
    bool bot_module::match(const cq::event::MessageEvent &e, const std::wstring &ws) 
    {
        std::wregex re(L"[\\s]*[\\.。．][\\s]*bot.*", std::regex_constants::ECMAScript | std::regex_constants::icase);
        if (std::regex_match(ws, re)) return true;
        return search_db(e);
    }

    bool bot_module::search_db(const cq::event::MessageEvent &e) 
    {
        //如果在讨论组里则查群信息表
        if (e.message_type == cq::message::DISCUSS) 
        {
            SQLite::Statement statement(*db::db,
                                        "SELECT bot_on, is_ban FROM group_info WHERE group_id = ? AND type = ?");
            statement.bind(1, *e.target.discuss_id);
            statement.bind(2, 1);
            if (statement.executeStep()) 
            {
                return !statement.getColumn(0).getInt() || statement.getColumn(1).getInt();
            }
            return false;
        }

        //如果在群里则查群信息表
        if (e.message_type == cq::message::GROUP) 
        {
            SQLite::Statement statement(*db::db,
                                        "SELECT bot_on, is_ban FROM group_info WHERE group_id = ? AND type = ?");
            statement.bind(1, *e.target.group_id);
            statement.bind(2, 0);
            if (statement.executeStep()) 
            {
                return !statement.getColumn(0).getInt() || statement.getColumn(1).getInt();
            }
            return false;
        }
        
        //查用户信息表
        SQLite::Statement statement(*db::db, "SELECT bot_on, is_ban FROM qq_info WHERE qq_id = ?");
        statement.bind(1, *e.target.user_id);
        if (statement.executeStep()) 
        {
            return !statement.getColumn(0).getInt() || statement.getColumn(1).getInt();
        }
        return false;
    }

    //自动更新数据库
    void bot_module::update_db(const cq::event::MessageEvent &e, bool bot_on) 
    {
        //讨论组
        if (e.message_type == cq::message::DISCUSS) 
        {
            SQLite::Statement statement(*db::db,
                                        "REPLACE INTO group_info(group_id, type, bot_on) VALUES(?, ?, ?)");
            statement.bind(1, *e.target.discuss_id);
            statement.bind(2, 1);
            statement.bind(3, bot_on);
            statement.exec();
        } 
        //群
        else if (e.message_type == cq::message::GROUP) 
        {
            SQLite::Statement statement(*db::db,
                                        "REPLACE INTO group_info(group_id, type, bot_on) VALUES(?, ?, ?)");
            statement.bind(1, *e.target.group_id);
            statement.bind(2, 0);
            statement.bind(3, bot_on);
            statement.exec();
        } 
        //私聊
        else
        {
            SQLite::Statement statement(*db::db,
                                        "REPLACE INTO qq_info(qq_id, bot_on) VALUES(?, ?)");
            statement.bind(1, *e.target.user_id);
            statement.bind(2, bot_on);
            statement.exec();
        }
    }

    //机器人开关
    void bot_module::process(const cq::event::MessageEvent &e, const std::wstring &ws) 
    {
        std::wregex re(L"[\\s]*[\\.。．][\\s]*bot[\\s]*(on|off)?[\\s]*(.*)",
                       std::regex_constants::ECMAScript | std::regex_constants::icase);
        std::wsmatch m;
        if (std::regex_match(ws, m, re)) 
        {
            std::wstring command = m[1];
            std::wstring target = m[2];
            std::wstring self_id = std::to_wstring(cq::api::get_login_user_id());
            //响应空/qq号/qq号后四位/qq昵称
            if (target.empty() || target == self_id || target == self_id.substr(self_id.length() - 4)
                || target == cq::utils::s2ws(cq::api::get_login_nickname())) 
            {
                //开
                if (command == L"on") 
                {
                    if (e.message_type == cq::message::GROUP && !utils::is_admin_or_owner(e.target)) 
                    {
                        dice::msg_queue::MsgQueue.add(e.target, msg::GetGlobalMsg("strPermissionDeniedError"));
                        return;
                    }
                    update_db(e, true);
                    dice::msg_queue::MsgQueue.add(e.target, msg::GetGlobalMsg("strEnabled"));
                } 
                //关
                else if (command == L"off") 
                {
                    if (e.message_type == cq::message::GROUP && !utils::is_admin_or_owner(e.target)) 
                    {
                        dice::msg_queue::MsgQueue.add(e.target, msg::GetGlobalMsg("strPermissionDeniedError"));
                        return;
                    }
                    update_db(e, false);
                    dice::msg_queue::MsgQueue.add(e.target, msg::GetGlobalMsg("strDisabled"));
                } 
                //如果无开关指令则回复机器人信息
                else 
                {
                    dice::msg_queue::MsgQueue.add(e.target, msg::dice_full_info);
                }
            }
        }
    }
} // namespace dice
