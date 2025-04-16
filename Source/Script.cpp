#include "Script.h"

using json = nlohmann::json;

namespace Script
{
    void parseActScript(const std::string& filename, Act& act)
    {
        act.maps.clear();

        std::ifstream file(filename);
        if (!file.is_open()) 
        {
            std::cerr << "Could not open file: " << filename << std::endl;
            throw std::logic_error("Could not open file: " + filename);
        }
        json jsonData;
        file >> jsonData;

        act.act_id = jsonData["act_id"];
        act.name = stringToWstring(jsonData["name"]);
        act.description = stringToWstring(jsonData["description"]);

        try
        {
            //解析场景
            for (const auto& mapData : jsonData["maps"])
            {
                Map map;
                map.map_id = mapData["map_id"];
                map.name = mapData["name"];
                if (mapData.contains("description"))
                    map.description = stringToWstring(mapData["description"]);
                else
                    map.description = L"";
                if (mapData.contains("background"))
                    map.background = mapData["background"];
                else
                    map.background = "";
                if (mapData.contains("bgm"))
                    map.bgm = mapData["bgm"];
                else
                    map.bgm = DEFAULT_BGM;

                if (mapData.contains("time_limit"))
                {
                    map.time_limit.minute = mapData["time_limit"]["minute"];
                    map.time_limit.second = mapData["time_limit"]["second"];
                    map.time_limit.message = stringToWstring(mapData["time_limit"]["message"]);
                }
                else
                {
                    map.time_limit.minute = -1;
                    map.time_limit.second = -1;
                }

                if (mapData.contains("quake_zone"))
                {
                    sf::FloatRect zone;
                    zone.left = mapData["quake_zone"]["left"];
                    zone.top = mapData["quake_zone"]["top"];
                    zone.width = static_cast<float>(mapData["quake_zone"]["right"]) - zone.left;
                    zone.height = static_cast<float>(mapData["quake_zone"]["bottom"]) - zone.top;
                    map.quake_zone.push_back(zone);
                }

                if (mapData.contains("dialogues"))
                {
                    for (const auto& dialogueData : mapData["dialogues"])
                    {
                        Dialogue dialogue;
                        dialogue.dialogue_id = dialogueData["dialogue_id"];
                        if (dialogueData.contains("speaker"))
                            dialogue.speaker = stringToWstring(dialogueData["speaker"]);
                        dialogue.text = stringToWstring(dialogueData["text"]);
                        dialogue.next_dialogue = dialogueData.value("next_dialogue", -1);

                        //解析选项
                        if (dialogueData.contains("choice"))
                        {
                            for (const auto& choiceData : dialogueData["choice"])
                            {
                                Choice choice;
                                choice.choice_id = choiceData["choice_id"];
                                choice.text = stringToWstring(choiceData["text"]);
                                choice.next_dialogue = choiceData["next_dialogue"];

                                if (choiceData.contains("effects"))
                                {
                                    for (const auto& i_effect : choiceData["effects"])
                                    {
                                        Expression effect;
                                        effect.variable = i_effect["variable"];
                                        effect.i_operator = i_effect["operator"];
                                        effect.value = i_effect["value"];

                                        choice.effect.push_back(effect);
                                    }
                                }

                                dialogue.choices.push_back(choice);
                            }
                        }

                        if (dialogueData.contains("effects"))
                        {
                            for (const auto& i_effect : dialogueData["effects"])
                            {
                                Expression effect;
                                effect.variable = i_effect["variable"];
                                effect.i_operator = i_effect["operator"];
                                effect.value = i_effect["value"];

                                dialogue.effect.push_back(effect);
                            }
                        }

                        map.dialogues.push_back(dialogue);
                    }
                }

                if (mapData.contains("dialogue_trigger"))
                {
                    for (const auto& triggerData : mapData["dialogue_trigger"])
                    {
                        DialogueTrigger trigger;
                        trigger.trigger_zone.left = triggerData["trigger_zone"]["left"];
                        trigger.trigger_zone.right = triggerData["trigger_zone"]["right"];
                        trigger.trigger_zone.top = triggerData["trigger_zone"]["top"];
                        trigger.trigger_zone.bottom = triggerData["trigger_zone"]["bottom"];
                        trigger.trigger_dialogue_id = triggerData["trigger_dialouge_id"];
                        trigger.trigger_id = triggerData["trigger_id"];

                        map.dialogue_triggers.push_back(trigger);
                    }
                }

                map.next_map = mapData.value("next_map", -1);

                if (mapData.contains("be_able_to_rewind_time"))
                    map.be_able_to_rewind_time = mapData["be_able_to_rewind_time"];
                else
                    map.be_able_to_rewind_time = true;

                if (mapData.contains("global_mode"))
                    map.global_mode = mapData["global_mode"];
                else
                    map.global_mode = false;

                if (map.global_mode == false)
                {
                    if (mapData.contains("move_camera"))
                        map.move_camera = mapData["move_camera"];
                    else
                        map.move_camera = false;
                }
                else
                    map.move_camera = false;

                if (mapData.contains("recorded_timeline_filename"))
                    map.recorded_timeline_filename = mapData["recorded_timeline_filename"];

                if (mapData.contains("forward_time_limit"))
                {
                    map.forward_time_limit.second = mapData["forward_time_limit"]["second"];
                    map.forward_time_limit.message = stringToWstring(mapData["forward_time_limit"]["message"]);
                }
                else
                    map.forward_time_limit.second = -1;

                act.maps.push_back(std::move(map));
            }
        }
        catch (const nlohmann::json::out_of_range& e) 
        {
            std::cerr << "JSON out of range error: " << e.what() << std::endl;
            throw;
        }
        catch (const std::exception& e) 
        {
            std::cerr << "standard exception occurred: " << e.what() << std::endl;
            throw;
        }
        catch (...) {
            std::cerr << "an unknown error occurred." << std::endl;
            throw;
        }

        file.close();
    }
    
    void parseScriptTree(const std::string& filename, ScriptTree& script_tree)
    {
        script_tree.acts.clear();

        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Could not open file: " << filename << std::endl;
            throw std::runtime_error("Could not open file: " + filename);
        }
        json jsonData;
        file >> jsonData;

        try
        {
            for (const auto& i_act : jsonData["script_tree"])
            {
                ActInTree act_in_tree;
                act_in_tree.act_id = i_act["act_id"];
                act_in_tree.act_file_name = i_act["act_file_name"];

                for (const auto& i_next_act : i_act["next_act"])
                {
                    NextAct next_act;
                    next_act.next_act_id = i_next_act["next_act_id"];

                    for (const auto& i_condition : i_next_act["enter_condition"])
                    {
                        Expression condition;
                        condition.variable = i_condition["variable"];
                        condition.i_operator = i_condition["operator"];
                        condition.value = i_condition["value"];

                        next_act.enter_condition.push_back(condition);
                    }

                    act_in_tree.next_acts.push_back(next_act);
                }

                if (i_act.contains("end_name"))
                    act_in_tree.end_name = stringToWstring(i_act["end_name"]);

                script_tree.acts.push_back(act_in_tree);
            }
        }
        catch (const nlohmann::json::out_of_range& e) 
        {
            std::cerr << "JSON out of range error: " << e.what() << std::endl;
            throw;
        }
        catch (const std::exception& e) 
        {
            std::cerr << "standard exception occurred: " << e.what() << std::endl;
            throw;
        }
        catch (...) 
        {
            std::cerr << "an unknown error occurred." << std::endl;
            throw;
        }

        file.close();
    }

    Dialogue findDialogue(Map& i_map, short id)
    {
        for (auto& dialogue : i_map.dialogues)
        {
            if (dialogue.dialogue_id == id)
            {
                Dialogue dialogue_copy = dialogue;
                return dialogue_copy;
            }
        }

        Dialogue blank_dialogue =
        {
            -1, L"", L""
        };
        return blank_dialogue;
    }

    ActInTree findAct(ScriptTree& i_script_tree, int id)
    {
        for (const auto& act_script : i_script_tree.acts)
        {
            if (act_script.act_id == id)
                return act_script;
        }

        ActInTree blank_act =
        {
            -1, ""
        };
        return blank_act;
    }

    std::wstring stringToWstring(const std::string& str) 
    {
        try 
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            return converter.from_bytes(str);
        }
        catch (const std::range_error& e)
        {
            // 处理异常，返回一个空的wstring
            std::wcerr << L"转换错误: " << e.what() << std::endl;
            return L"";
        }
    }

    const bool checkEnterCondition(const NextAct& next_act)
    {
        for (const auto& condition : next_act.enter_condition)
        {
            if (!parseEffect(condition))
                return false;
        }

        return true;
    }

    const bool parseEffect(const Expression& expression)
    {
        if (expression.variable == "reset_game")
        {
            MessageQueue::send(MessageQueue::interface_header << 26, MessageQueue::scene_code, "reset game");
            return true;
        }
        else
        {
            unsigned short* variable = nullptr;
            if (expression.variable == "ending")
                variable = &ending;
            else if (expression.variable == "switch_to_next_scene")
                variable = &switch_to_next_scene;
            else if (expression.variable == "ending_has_achieved")
                variable = &ending_has_achieved;
            return operate(variable, expression);
        }
    }

    const bool operate(unsigned short* value, const Expression& expression)
    {
        if (value == nullptr)
            return false;

        if (expression.i_operator == "+")
        {
            *value += expression.value;
            return true;
        }
        else if (expression.i_operator == "-")
        {
            *value -= expression.value;
            return true;
        }
        else if (expression.i_operator == "=")
        {
            *value = expression.value;
            return true;
        }
        else if (expression.i_operator == "==")
            return *value == expression.value;
        else if (expression.i_operator == ">")
            return *value > expression.value;
        else if (expression.i_operator == "<")
            return *value < expression.value;
        else if (expression.i_operator == ">=")
            return *value >= expression.value;
        else if (expression.i_operator == "<=")
            return *value <= expression.value;
        else if (expression.i_operator == "<<&")
            return *value & (1 << expression.value);
        else if (expression.i_operator == "<<!&")
            return !(*value & (1 << expression.value));
        else if (expression.i_operator == "+<<")
        {
            if (*value & (1 << expression.value))
                return false;
            *value += (1 << expression.value);
            return true;
        }
        else if (expression.i_operator == "-<<")
        {
            if (!(*value & (1 << expression.value)))
                return false;
            *value -= (1 << expression.value);
            return true;
        }
        return false;
    }
}