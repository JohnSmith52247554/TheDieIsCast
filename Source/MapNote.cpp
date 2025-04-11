#include "MapNote.h"

using json = nlohmann::json;

namespace MapNote
{
	void parseMapNote(const std::string& filename, Note& map_note)
	{
        map_note.movable_walls.clear();
        map_note.receptors_and_effectors.clear();
        map_note.npcs.clear();

        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Could not open file: " << filename << std::endl;
            throw std::logic_error("Could not open file: " + filename);
        }
        json jsonData;
        file >> jsonData;

       /* try
        {*/
            if (jsonData.contains("MovableWall"))
            {  
                for (const auto& i_movable_wall : jsonData["MovableWall"])
                {
                    MovableWall movable_wall;
                    movable_wall.x = i_movable_wall["x"];
                    movable_wall.y = i_movable_wall["y"];
                    movable_wall.structure_direction = static_cast<Direction>(stringToEnum(i_movable_wall["structure_direction"].get<std::string>()));
                    movable_wall.length = i_movable_wall["length"];
                    movable_wall.move_direction = static_cast<Direction>(stringToEnum(i_movable_wall["move_direction"]));
                    movable_wall.move_distance = i_movable_wall["move_distance"];
                    movable_wall.speed = i_movable_wall["speed"];
                   
                    if (i_movable_wall.contains("can_be_affected_by_time_control"))
                        movable_wall.set_can_be_affected_by_time_control = i_movable_wall["can_be_affected_by_time_control"];
                    else
                        movable_wall.set_can_be_affected_by_time_control = true;

                    map_note.movable_walls.push_back(movable_wall);
                }
            }

            if(jsonData.contains("ReceptorAndEffectors"))
            {
                for (const auto& i_receptor_and_effectors : jsonData["ReceptorAndEffectors"])
                {
                    ReceptorAndEffectors receptor_and_effectors;
                    receptor_and_effectors.receptor.type = static_cast<ReceptorType>(stringToEnum(i_receptor_and_effectors["receptor"]["type"]));
                    receptor_and_effectors.receptor.activation_message = i_receptor_and_effectors["receptor"]["activation_message"];
                    receptor_and_effectors.receptor.release_message = i_receptor_and_effectors["receptor"]["release_message"];
                    receptor_and_effectors.receptor.x = i_receptor_and_effectors["receptor"]["x"];
                    receptor_and_effectors.receptor.y = i_receptor_and_effectors["receptor"]["y"];

                    if (i_receptor_and_effectors["receptor"].contains("can_be_affected_by_time_control"))
                        receptor_and_effectors.receptor.set_can_be_affected_by_time_control = i_receptor_and_effectors["receptor"]["can_be_affected_by_time_control"];
                    else
                        receptor_and_effectors.receptor.set_can_be_affected_by_time_control = true;

                    for (const auto& i_effector_postal_code : i_receptor_and_effectors["effector_postal_codes"])
                    {
                        unsigned int code = i_effector_postal_code;
                        receptor_and_effectors.effector_postal_code.push_back(code);
                    }

                    map_note.receptors_and_effectors.push_back(receptor_and_effectors);
                }
            }

            if(jsonData.contains("NPCs"))
            {
                for (const auto& i_npc : jsonData["NPCs"])
                {
                    NPC npc;
                    npc.npc_id = i_npc["npc_id"];
                    npc.npc_skin_id = i_npc["npc_skin_id"];
                    npc.name = Script::stringToWstring(i_npc["name"]);
                    npc.x = i_npc["x"];
                    npc.y = i_npc["y"];

                    if (i_npc.contains("dialogue_id_while_rewinding_time"))
                        npc.dialogue_id_while_rewinding_time = i_npc["dialogue_id_while_rewinding_time"];
                    else
                        npc.dialogue_id_while_rewinding_time = 1;

                    for (const auto& dialogueData : i_npc["dialogues"])
                    {
                        Script::Dialogue dialogue;
                        dialogue.dialogue_id = dialogueData["dialogue_id"];
                        dialogue.speaker = Script::stringToWstring(dialogueData["speaker"]);
                        dialogue.text = Script::stringToWstring(dialogueData["text"]);
                        dialogue.next_dialogue = dialogueData.value("next_dialogue", -1);

                        //����ѡ��
                        if (dialogueData.contains("choice"))
                        {
                            for (const auto& choiceData : dialogueData["choice"])
                            {
                                Script::Choice choice;
                                choice.choice_id = choiceData["choice_id"];
                                choice.text = Script::stringToWstring(choiceData["text"]);
                                choice.next_dialogue = choiceData["next_dialogue"];

                                if (choiceData.contains("effects"))
                                {
                                    for (const auto& i_effect : choiceData["effects"])
                                    {
                                        Script::Expression effect;
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
                                Script::Expression effect;
                                effect.variable = i_effect["variable"];
                                effect.i_operator = i_effect["operator"];
                                effect.value = i_effect["value"];

                                dialogue.effect.push_back(effect);
                            }
                        }


                        npc.dialogues.push_back(dialogue);
                    }

                    map_note.npcs.push_back(npc);
                }
            }
        //}
        /*catch (const nlohmann::json::out_of_range& e) 
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
        }*/

        file.close();
	}

    int stringToEnum(const std::string& str)
    {
        if (str == "left")
            return left;
        else if (str == "right")
            return right;
        else if (str == "up")
            return up;
        else if (str == "down")
            return down;
        else if (str == "null")
            return null;

        else if (str == "drawbar")
            return drawbar;
        else if (str == "button")
            return button;
        else if (str == "pressure_plate")
            return pressure_plate;
        else if (str == "key")
            return key;

        return -1;
    }
}