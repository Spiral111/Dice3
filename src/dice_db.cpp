/*
数据库核心（待改造）
值得注意的是，Dice3的master分支移除了帮助信息的数据库，这也导致了我不再合并master分支。
注释：陈末
*/

#include "dice_db.h"
#include <memory>
#include "SQLiteCpp/SQLiteCpp.h"
#include "cqsdk/cqsdk.h"
#include "dice_exception.h"
#include "dice_msg.h"

namespace dice::db 
{
    std::unique_ptr<SQLite::Database> db;

    //初始化数据库，如果表不存在则创建新表
    void InitialiseDB() {
        SQLite::Transaction tran(*db);

        //QQ信息
        db->exec(
            "CREATE TABLE IF NOT EXISTS qq_info (qq_id INTEGER PRIMARY KEY NOT NULL, is_ban INTEGER DEFAULT 0, "
            "ban_reason "
            "TEXT, ban_time INTEGER, "
            "is_admin INTEGER DEFAULT 0, admin_time INTEGER, is_white INTEGER DEFAULT 0, white_time INTEGER, "
            "jrrp_value "
            "INTEGER, jrrp_date "
            "TEXT, bot_on INTEGER DEFAULT 1, card_chosen TEXT DEFAULT \"default\", nick_name TEXT, default_dice "
            "INTEGER NOT NULL DEFAULT 100, success_rule INTEGER NOT NULL DEFAULT 0 CHECK (success_rule >= 0 AND success_rule <= 5))");
        
        //角色卡
        db->exec(
            "CREATE TABLE IF NOT EXISTS character_cards (qq_id INTEGER NOT NULL, card_name TEXT NOT NULL, property "
            "TEXT NOT NULL, value INTEGER NOT NULL, "
            "PRIMARY KEY (qq_id, card_name, property))");
        
        //群信息
        db->exec(
            "CREATE TABLE IF NOT EXISTS group_info (group_id INTEGER NOT NULL, type INTEGER NOT NULL, bot_on INTEGER "
            "DEFAULT 1, help_on INTEGER DEFAULT 1, jrrp_on INTEGER DEFAULT 1, is_ban INTEGER DEFAULT 0, ban_reason "
            "TEXT, ban_time INTEGER, "
            "is_white "
            "INTEGER DEFAULT 0, white_time INTEGER, default_dice INTEGER NOT NULL DEFAULT 100, success_rule INTEGER NOT NULL DEFAULT 0 CHECK (success_rule >= 0 AND success_rule <= 5), PRIMARY KEY(group_id, type))");
        
        //群员信息
        db->exec(
            "CREATE TABLE IF NOT EXISTS group_user_info (qq_id INTEGER NOT NULL, group_id INTEGER NOT NULL, type "
            "INTEGER "
            "NOT NULL, nick_name TEXT, card_chosen TEXT DEFAULT \"default\", PRIMARY KEY(qq_id, group_id, type))");
        
        //全局消息
        db->exec("CREATE TABLE IF NOT EXISTS global_msg (title TEXT PRIMARY KEY NOT NULL, val TEXT NOT NULL)");
        
        //帮助信息
        db->exec("CREATE TABLE IF NOT EXISTS help_msg (title TEXT PRIMARY KEY NOT NULL, val TEXT NOT NULL)");

        //牌堆
        if (db->execAndGet("SELECT count(*) FROM sqlite_master WHERE type = \"table\" AND name = \"deck\"").getInt() ==0) 
            {
            db->exec(
                "CREATE TABLE deck(name TEXT NOT NULL, content TEXT NOT NULL, origin TEXT NOT NULL "
                "DEFAULT "
                "\"public\")");
            db->exec("CREATE INDEX idx_name ON deck(name)");
            db->exec("CREATE INDEX idx_ori ON deck(origin)");

            for (const auto &msg : msg::default_deck) 
            {
                for (const auto &item : msg.second) 
                {
                    SQLite::Statement st(*db, "INSERT INTO deck(name, content) VALUES(?, ?)");
                    st.bind(1, msg.first);
                    st.bind(2, item);
                    st.exec();
                }
            }
        }

        //全局信息
        for (const auto &msg : msg::global_msg) 
        {
            SQLite::Statement st(*db, "INSERT OR IGNORE INTO global_msg(title, val) VALUES(?, ?)");
            st.bind(1, msg.first);
            st.bind(2, msg.second);
            st.exec();
        }

        //帮助信息
        for (const auto &msg : msg::help_msg) 
        {
            SQLite::Statement st(*db, "INSERT OR IGNORE INTO help_msg(title, val) VALUES(?, ?)");
            st.bind(1, msg.first);
            st.bind(2, msg.second);
            st.exec();
        }
        tran.commit();
    }

    //半重置数据库
    void SemiReplaceDB() 
    {
        SQLite::Transaction tran(*db);
        for (const auto &msg : msg::global_msg) 
        {
            SQLite::Statement st(*db, "REPLACE INTO global_msg(title, val) VALUES(?, ?)");
            st.bind(1, msg.first);
            st.bind(2, msg.second);
            st.exec();
        }
        for (const auto &msg : msg::help_msg) 
        {
            SQLite::Statement st(*db, "REPLACE INTO help_msg(title, val) VALUES(?, ?)");
            st.bind(1, msg.first);
            st.bind(2, msg.second);
            st.exec();
        }
        db->exec("DROP TABLE deck");
        db->exec(
            "CREATE TABLE deck(name TEXT NOT NULL, content TEXT NOT NULL, origin TEXT NOT NULL "
            "DEFAULT "
            "\"public\")");
        db->exec("CREATE INDEX idx_name ON deck(name)");
        db->exec("CREATE INDEX idx_ori ON deck(origin)");

        for (const auto &msg : msg::default_deck) 
        {
            for (const auto &item : msg.second) 
            {
                SQLite::Statement st(*db, "INSERT OR IGNORE INTO deck(name, content) VALUES(?, ?)");
                st.bind(1, msg.first);
                st.bind(2, item);
                st.exec();
            }
        }
        tran.commit();
    }

    //重置数据库
    void ReplaceDB() 
    {
        db = nullptr;
        if (!std::filesystem::remove(cq::utils::s2ws(cq::api::get_app_directory() + "DiceConfig_"
                                                     + std::to_string(cq::api::get_login_user_id()) + ".db"))) 
        {
            cq::logging::debug("Dice! V3", "Unable to locate database");
        }
        db = std::make_unique<SQLite::Database>(
            cq::api::get_app_directory() + "DiceConfig_" + std::to_string(cq::api::get_login_user_id()) + ".db",
            SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE,
            3000);
        InitialiseDB();
    }

} // namespace dice::db
