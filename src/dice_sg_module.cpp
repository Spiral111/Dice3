#include "dice_sg_module.h"
#include "cqsdk/cqsdk.h"
#include "dice_calculator.h"
#include "dice_exception.h"
#include "dice_utils.h"
#include "dice_msg_queue.h"

namespace cq::event {
    struct MessageEvent;
}

namespace dice {
    bool sg_module::match(const cq::event::MessageEvent &e, const std::wstring &ws) {
        std::wregex re(L"[\\s]*[\\.。．][\\s]*sg.*", std::regex_constants::ECMAScript | std::regex_constants::icase);
        return std::regex_match(ws, re);
    }

    void sg_module::process(const cq::event::MessageEvent &e, const std::wstring &ws) {
        std::wregex re(L"[\\s]*[\\.。．][\\s]*sg[\\s]*([0-9]*)[\\s]*([^]*)",
                       std::regex_constants::ECMAScript | std::regex_constants::icase);
        std::wsmatch m;
        if (std::regex_match(ws, m, re)) {
            std::string CharacterCards;
            int GenerateCount = 1;
            if (m[1].first != m[1].second) {
                auto GenerateCountStr = std::wstring(m[1].first, m[1].second);
                if (GenerateCountStr.length() > 2) {
                    throw exception::exception(msg::GetGlobalMsg("strGenerateCountError"));
                }
                GenerateCount = std::stoi(std::wstring(m[1].first, m[1].second));
                if (GenerateCount > 10 || GenerateCount <= 0) {
                    throw exception::exception(msg::GetGlobalMsg("strGenerateCountError"));
                }
            }
            const std::string strProperty[] = {"壮硕值", "爆发力", "协调性", "精神力", "反应力", "幸运值"};
            const std::wstring strRoll[] = {L"2D3",L"2D3",L"2D3",L"2D3",L"2D3",L"2D3"};
            int AllTotal = 0;
            while (GenerateCount--) {
                for (int i = 0; i != 6; i++) {
                    CharacterCards += strProperty[i];
                    CharacterCards += ":";
                    int RollRes = static_cast<int>(dice_calculator(strRoll[i]).result);
                    AllTotal += RollRes;
                    CharacterCards += std::to_string(RollRes);
                    CharacterCards += " ";
                }
                CharacterCards += "共计:";
                CharacterCards += std::to_string(AllTotal);
                
                std::wstring reason(m[2]);
                char test_flag[]="测试";
                if(cq::utils::ws2s(reason)[0]==test_flag[0] && cq::utils::ws2s(reason)[1]==test_flag[1] || cq::utils::ws2s(reason)[0]=='t' )
                {
                    CharacterCards += "\n本次仅为测试，无法填入人物卡";
                } 
                else
                    CharacterCards += "\n不要忘了还有10点自由属性点";

                if (GenerateCount) CharacterCards += "\n";
                AllTotal = 0;
            }
            dice::msg_queue::MsgQueue.add(
                e.target,
                utils::format_string(msg::GetGlobalMsg("strCharacterCard"),
                                     std::map<std::string, std::string>{{"nick", utils::get_nickname(e.target)},
                                                                        {"version", "轮回游戏"},
                                                                        {"character_cards", CharacterCards}}));

        }
    }
} // namespace dice
