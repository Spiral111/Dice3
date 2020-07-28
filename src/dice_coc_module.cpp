/*
coc人物投掷
注释：陈末
*/

#include "dice_coc_module.h"
#include "cqsdk/cqsdk.h"
#include "dice_calculator.h"
#include "dice_exception.h"
#include "dice_utils.h"
#include "dice_msg_queue.h"

namespace cq::event {
    struct MessageEvent;
}

namespace dice {
    bool coc_module::match(const cq::event::MessageEvent &e, const std::wstring &ws) {
        std::wregex re(L"[\\s]*[\\.。．][\\s]*coc.*", std::regex_constants::ECMAScript | std::regex_constants::icase);
        return std::regex_match(ws, re);
    }

    void coc_module::process(const cq::event::MessageEvent &e, const std::wstring &ws) {
        std::wregex re(L"[\\s]*[\\.。．][\\s]*coc(6|7)?(d)?[\\s]*([0-9]*).*",
                       std::regex_constants::ECMAScript | std::regex_constants::icase);
        std::wsmatch m;
        if (std::regex_match(ws, m, re)) {
            bool UseCOC6 = false;
            if (m[1].first != m[1].second && std::wstring(m[1].first, m[1].second) == L"6") {
                UseCOC6 = true;
            }
            std::string CharacterCards;
            int GenerateCount = 1;
            if (m[2].first == m[2].second) {
                if (m[3].first != m[3].second) {
                    auto GenerateCountStr = std::wstring(m[3].first, m[3].second);
                    if (GenerateCountStr.length() > 2) {
                        throw exception::exception(msg::GetGlobalMsg("strGenerateCountError"));
                    }
                    GenerateCount = std::stoi(std::wstring(m[3].first, m[3].second));
                    if (GenerateCount > 10 || GenerateCount <= 0) {
                        throw exception::exception(msg::GetGlobalMsg("strGenerateCountError"));
                    }
                }
            }
            if (UseCOC6) {
                const std::string strProperty[] = {"力量", "体质", "体型", "敏捷", "外貌", "智力", "意志", "教育", "资产"};
                const std::wstring strRoll[] = {L"3D6", L"3D6", L"2D6+6", L"3D6", L"3D6", L"2D6+6", L"3D6", L"3D6+3", L"1D10"};
                const bool AddSpace = GenerateCount != 1;
                int AllTotal = 0;
                while (GenerateCount--) {
                    for (int i = 0; i != 9; i++) {
                        CharacterCards += strProperty[i];
                        CharacterCards += ":";
                        int RollRes = static_cast<int>(dice_calculator(strRoll[i]).result);
                        AllTotal += RollRes;
                        CharacterCards += std::to_string(RollRes);
                        CharacterCards += " ";
                        if (AddSpace && RollRes < 10) {
                            CharacterCards += " ";
                        }
                    }
                    CharacterCards += "共计:";
                    CharacterCards += std::to_string(AllTotal);
                    if (GenerateCount) CharacterCards += "\n";
                    AllTotal = 0;
                }

            } else {
                const std::string strProperty[] = {
                    "力量", "体质", "体型", "敏捷", "外貌", "智力", "意志", "教育", "幸运"};
                const std::wstring strRoll[] = {L"3D6*5",
                                                L"3D6*5",
                                                L"(2D6+6)*5",
                                                L"3D6*5",
                                                L"3D6*5",
                                                L"(2D6+6)*5",
                                                L"3D6*5",
                                                L"(2D6+6)*5",
                                                L"(3D6)*5"};
                int AllTotalWithoutLuck = 0;
                int AllTotal = 0;
                while (GenerateCount--) {
                    for (int i = 0; i != 9; i++) {
                        CharacterCards += strProperty[i];
                        CharacterCards += ":";
                        int RollRes = static_cast<int>(dice_calculator(strRoll[i]).result);
                        AllTotal += RollRes;
                        if (i != 8) AllTotalWithoutLuck += RollRes;
                        CharacterCards += std::to_string(RollRes);
                        CharacterCards += " ";
                    }
                    CharacterCards += "共计:";
                    CharacterCards += std::to_string(AllTotalWithoutLuck);
                    CharacterCards += "/";
                    CharacterCards += std::to_string(AllTotal);
                    if (GenerateCount) CharacterCards += "\n";
                    AllTotal = 0;
                    AllTotalWithoutLuck = 0;
                }
            }
            if (m[2].first != m[2].second) {
                CharacterCards += "\n性别: {%性别}\n年龄: {?7d6+8}\n职业: {%调查员职业}\n{%调查员背景}";
            }
            dice::msg_queue::MsgQueue.add(
                e.target,
                utils::format_string(msg::GetGlobalMsg("strCharacterCard"),
                                     std::map<std::string, std::string>{{"nick", utils::get_nickname(e.target)},
                                                                        {"version", UseCOC6 ? "COC6" : "COC7"},
                                                                        {"character_cards", CharacterCards}}));
           
        }
    }
} // namespace dice
