#include "dice_msg.h"
#include <map>
#include <string>

namespace dice::msg {
    std::string dice_ver = "3.0.0alpha";
    short dice_build = 1000;
    std::string dice_info = "Dice! by 溯洄 Version " + dice_ver + "(" + std::to_string(dice_build) + ")";
    std::string dice_full_info =
        dice_info + " [MSVC " + std::to_string(_MSC_FULL_VER) + " " + __DATE__ + " " + __TIME__ + "]";
    std::map<std::string, std::string> global_msg{{"strRollDice", "{nick} 投掷 {reason}: {dice_expression}"},
                                                  {"strInvalidDiceError", "错误: 不合法的掷骰表达式"},
                                                  {"strNicknameError", "内部错误: 无法获取昵称"},
                                                  {"strPermissionDeniedError", "错误: 此操作需要群主或管理员权限"},
                                                  {"strEnabled", "机器人已启用"},
                                                  {"strDisabled", "机器人已停用"},
                                                  {"strGenerateCountError", "错误: 人物生成个数超出限制(1-10)"},
                                                  {"strCharacterCard", "{nick}的{version}版人物作成:\n{character_cards}"}};
} // namespace dice::msg
