#pragma once

#include <filesystem>
#include <sstream>

#include <WindowLayer.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <pugixml.hpp>

#define IMGUI_LEFT_LABEL(func, label, ...) (ImGui::TextUnformatted(label), ImGui::SameLine(), func("##" label, __VA_ARGS__))

class MainTabLayer : public WindowLayer
{
public:
	struct XMLDocument
	{
		pugi::xml_document* document;
		std::string path;
	};

	WindowManager* m_Manager = nullptr;
	std::vector<XMLDocument> m_XMLDocuments;

	char buffer[128];

	std::vector<const char*> listboxItems;
	int listbox_item_current = 0;
	int listbox_item_last = -1;

	ImGuiID current_tab = 0;
	pugi::xml_document* current_document = nullptr;

	char search[64] = { 0 };
	bool search_meta = false;
	bool search_list = false;

	void RenderRecursiveTest(pugi::xml_node& node, int level = 0)
	{
		if (strlen(node.name()))
			//printf("%s%s - %d\n", std::string(level, '\t').c_str(), node.name(), level);
			ImGui::Text("A %s - %d", node.name(), level);
		
		for (auto& n : node.children())
		{
			if (!n.children().empty() && n.first_child().type() != pugi::xml_node_type::node_pcdata)
			{
				//sprintf_s(buffer, sizeof(buffer), "%s##%d", n.name(), n.hash_value());
				//if (ImGui::BeginChild(buffer, ImVec2(0, 0)))
				{
					sprintf_s(buffer, sizeof(buffer), "%s##%d", node.name(), level);
					printf("Buffer: %s\n", buffer);

					if (ImGui::BeginTable(buffer, 3))
					{
						RenderRecursiveTest(n, level + 1);
						ImGui::EndTable();
					}

					//ImGui::EndChild();
				}
			}
			else
			{
				if (strlen(n.name()))
					ImGui::Text("B %s - %d", n.name(), level);
			}
		}
	}

	int columnSize = 2;
	int columnCount = 0;

	char inputBuffer[512];
	float inputFloat[3];

	void PaddingText(ImVec2 padding, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		//ImGui::TextV(fmt, args);

		ImGuiContext& g = *GImGui;
		const char* value_text_begin = &g.TempBuffer[0];
		const char* value_text_end = value_text_begin + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
		const ImVec2 value_size = ImGui::CalcTextSize(value_text_begin, value_text_end, false);

		//ImVec2 sz = ImGui::CalcTextSize(str.c_str());
		ImVec2 cursor = ImGui::GetCursorPos();
		ImGui::InvisibleButton("##padded-text", value_size + ImVec2(padding.x * 2, padding.y * 2));    // ImVec2 operators require imgui_internal.h include and -DIMGUI_DEFINE_MATH_OPERATORS=1
		ImVec2 final_cursor_pos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(cursor + ImVec2(padding.x, padding.y));
		ImGui::TextV(fmt, args);
		ImGui::SetCursorPos(final_cursor_pos);

		va_end(args);
	}

	size_t findCaseInsensitive(std::string data, std::string toSearch, size_t pos = 0)
	{
		// Convert complete given String to lower case
		std::transform(data.begin(), data.end(), data.begin(), ::tolower);
		// Convert complete given Sub String to lower case
		std::transform(toSearch.begin(), toSearch.end(), toSearch.begin(), ::tolower);
		// Find sub string in given string
		return data.find(toSearch, pos);
	}

	int edit_id = 0;
	std::string edit_str_id{""};
	void RenderRecursive(pugi::xml_node& node, int level = 0)
	{
		for (auto& child : node)
		{
			uint32_t childrenSize = std::distance(child.children().begin(), child.children().end());
			bool nextChildIsData = (child.first_child().type() == pugi::xml_node_type::node_pcdata);
			bool shouldIterateThrough = (!nextChildIsData && (childrenSize > 0));

			if (shouldIterateThrough)
			{
				//ImGui::Text("%sGroup: %s", tab.c_str(), child.name());
				RenderRecursive(child, level + 1);
			}
			else
			{
				if (search_meta && findCaseInsensitive(child.name(), search) == std::string::npos)
					continue;

				uint16_t attributeSize = std::distance(child.attributes().begin(), child.attributes().end());

				ImGui::TableNextColumn();

				//ImGui::Text("%s", child.name());
				PaddingText(ImVec2(5, 10), "%s", child.name());

				sprintf_s(buffer, sizeof(buffer), "##%s_%d", child.name(), child.hash_value());

				//ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-1);
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
				if (!edit_str_id.compare(buffer))
				{
					ImGui::PushStyleColor(ImGuiCol_FrameBg, IMGUI_COLOR(40, 40, 40, 255));
					ImGui::PushStyleColor(ImGuiCol_Border, IMGUI_COLOR(0x03, 0x63, 0x29, 0xFF));

					/*style.Colors[ImGuiCol_TabHovered] = IMGUI_COLOR(0x04, 0x78, 0x32, 0xFF);
					style.Colors[ImGuiCol_TabActive] = IMGUI_COLOR(0x03, 0x63, 0x29, 0xFF);*/
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_FrameBg, IMGUI_COLOR(30, 30, 30, 255));
					ImGui::PushStyleColor(ImGuiCol_Border, IMGUI_COLOR(0, 0, 0, 0));
				}

				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
				if (attributeSize == 1)
				{
					if (child.attribute("ref"))
					{
						sprintf_s(inputBuffer, sizeof(inputBuffer), "%s", child.attribute("ref").as_string());
						if (ImGui::InputText(buffer, inputBuffer, IM_ARRAYSIZE(inputBuffer)))
						//if (ImGui::InputTextMultiline(buffer, inputBuffer, IM_ARRAYSIZE(inputBuffer), ImVec2(-1, 30)))
						{
							printf("Input text: %s\n", inputBuffer);
						}
					}

					if (child.attribute("value"))
					{
						inputFloat[0] = child.attribute("value").as_float();
						if (ImGui::InputFloat(buffer, &inputFloat[0]))
						{
							printf("Input float: %d\n", inputFloat[0]);
						}
					}
				}
				else if (attributeSize >= 3)
				{
					inputFloat[0] = child.attribute("x").as_float();
					inputFloat[1] = child.attribute("y").as_float();
					inputFloat[2] = child.attribute("z").as_float();

					if (ImGui::InputFloat3(buffer, inputFloat))
					{
						printf("Input values: %f\n", inputFloat[0]);
					}
				}
				else
				{
					//printf("V: %d\n", child.text());
					if ((child.name()[0] != '\0') && child.text().as_string()[0])
						sprintf_s(inputBuffer, sizeof(inputBuffer), "%s", child.text().as_string());
					else
						sprintf_s(inputBuffer, sizeof(inputBuffer), "");

					if (ImGui::InputText(buffer, inputBuffer, IM_ARRAYSIZE(inputBuffer)))
					{
						printf("Input text: %s\n", inputBuffer);
					}
				}

				//printf("%d - %d - %d\n", ImGui::GetActiveID(), ImGui::GetItemID(), ImGui::GetFocusID());
				if (ImGui::IsItemActive() && ImGui::GetActiveID())
				{
					//printf("Editing: %s - %s\n", is_editing.c_str(), buffer);
					edit_id = ImGui::GetActiveID();
					edit_str_id.assign(buffer);
				}
				else if(ImGui::GetActiveID() != edit_id)
				{
					edit_id = 0;
					edit_str_id.assign("");

					//printf("Not editing\n");
				}

				ImGui::PopStyleColor(2);
				ImGui::PopStyleVar(3);

				columnCount++;
			}

			if (columnCount >= columnSize)
			{
				columnCount = 0;
				ImGui::TableNextRow();
			}
		}
	}

	void RenderWeapon()
	{
		if (current_document == nullptr)
			return;

		//Keep it that way, otherwise the table will flicker
		columnCount = 0;

		sprintf_s(buffer, sizeof(buffer), "/CWeaponInfoBlob/Infos/Item/Infos/Item[@type='CWeaponInfo' and Name='%s']", listboxItems[listbox_item_current]);
		pugi::xpath_node result_node = current_document->select_node(buffer);
		pugi::xml_node node = result_node.node();

		if (ImGui::BeginTable("Weapon Table##weapon_table", columnSize, /*ImGuiTableFlags_Borders |*/ ImGuiTableFlags_SizingStretchSame))
		{
			ImGui::TableNextRow();
			RenderRecursive(node);

			ImGui::EndTable();
		}
	}

	void Attach(WindowManager* manager) override
	{
		pugi::xml_document* test = OpenDocument("./weapons.meta");
		
		this->m_Manager = manager;

		/*manager->AddShortcut({SDL_SCANCODE_RETURN}, [&]()
		{
			if (active_node_hash != 0)
			{
				printf("Entered thing: %s\n", active_text);

				if (active_document != nullptr)
				{
					pugi::xml_node child = active_document->find_node([&](pugi::xml_node node) -> bool
						{
							printf("child: %d\n", node.hash_value());
							return node.hash_value() == active_node_hash;
						});

					std::cout << child << std::endl;

					if (child != pugi::xml_node())
					{
						if (!active_attribute.empty())
							child.attribute(active_attribute.c_str()).set_value(active_text);
						else
							child.set_value(active_text);
					}
				}

				active_node_hash = 0;
				active_text_flags = ImGuiInputTextFlags_None;
			}
		});

		manager->AddShortcut({ SDL_SCANCODE_ESCAPE }, [&]()
		{
				if (active_node_hash != 0)
				{
					active_node_hash = 0;
					active_text_flags = ImGuiInputTextFlags_None;
				}
		});*/
	}

	void PreRender(WindowManager* manager) override
	{

	}

	typedef std::function<void(const char* data)> SearchCallback;
	typedef std::function<void()> SearchDeactivatedCallback;
	struct SearchResult
	{
		SearchCallback callbackResult = nullptr;

		MainTabLayer* layer = nullptr;
	};

	void SearchBar(const char* label, bool& active, SearchCallback callback = nullptr, SearchDeactivatedCallback deactivatedCallback = nullptr, float width = 300.0f)
	{
		ImGui::SetNextItemWidth(width);
		ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 300.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, active ? 10 : 13));

		const char* searchIcon = u8"\uf002";
		bool enableSearchIcon = !strcmp(search, searchIcon);

		ImGui::PushFont(active ? m_Manager->m_InterFont : m_Manager->m_AwesomeFont);

		if (!active)
		{
			sprintf_s(search, sizeof(search), searchIcon);
		}

		//printf("Search active: %d - %s\n", active, search);

		if (!active) ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_COLOR(50, 50, 50, 255));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, IMGUI_COLOR(40, 40, 40, 255));
		ImGui::PushStyleColor(ImGuiCol_Border, IMGUI_COLOR(0x03, 0x63, 0x29, 0xFF));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.1f);
		
		SearchResult result
		{
			callback,
			this
		};

		ImGui::InputText(label, search, IM_ARRAYSIZE(search), callback != nullptr ? ImGuiInputTextFlags_CallbackEdit : NULL, [](ImGuiInputTextCallbackData* data)
			{
				SearchResult* result = static_cast<SearchResult*>(data->UserData);
				if (result->callbackResult)
					result->callbackResult(data->Buf);

				printf("S: %p\n", static_cast<SearchResult*>(data->UserData)->layer);

				return 0;
			}, &result);
		
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(2);
		if (!active) ImGui::PopStyleColor();

		if (ImGui::IsItemActivated())
		{
			ImGuiContext* g = GImGui;
			g->InputTextState.ClearText();
			sprintf_s(search, sizeof(search), "");
		}
		active = ImGui::IsItemActive();

		if (ImGui::IsItemDeactivated())
		{
			if (deactivatedCallback)
				deactivatedCallback();

			printf("Deactivated\n");
		}

		ImGui::PopFont();
		ImGui::PopStyleVar();
	}

	void ListBoxSearch(const char* input = "")
	{
		pugi::xpath_node_set result = this->current_document->select_nodes("/CWeaponInfoBlob/Infos/Item/Infos/Item[@type='CWeaponInfo']");

		this->listboxItems.clear();
		for (pugi::xpath_node_set::const_iterator it = result.begin(); it != result.end(); ++it)
		{
			if (this->search_list && this->findCaseInsensitive((*it).node().child_value("Name"), input) == std::string::npos)
				continue;

			this->listboxItems.push_back((*it).node().child_value("Name"));
		}
	}

	void Render(WindowManager* manager) override
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);

		ImGui::PushStyleColor(ImGuiCol_ChildBg, IMGUI_COLOR(255, 255, 255, 255));

		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::Begin("##weapon_tab", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

		ImGui::PushStyleColor(ImGuiCol_TabActive, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, IM_COL32(0, 0, 0, 0));

		if (ImGui::BeginTabBar("Test Bar", ImGuiTabBarFlags_NoTabListScrollingButtons))
		{
			ImGui::PopStyleColor(2);

			for (XMLDocument& documentStruct : this->m_XMLDocuments)
			{
				pugi::xml_document* document = documentStruct.document;
				std::string fileName = std::filesystem::path(documentStruct.path).filename().string();

				if (ImGui::BeginTabItem(fileName.c_str()))
				{
					ImGui::PushStyleColor(ImGuiCol_WindowBg, IMGUI_COLOR(255, 255, 255, 0));

					ImGuiID dockid = ImGui::GetID("weaponMeta");
					ImGui::PushStyleColor(ImGuiCol_Separator, IMGUI_COLOR(0, 0, 0, 0));
					ImGui::DockSpace(dockid, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
					ImGui::PopStyleColor();

					if (current_tab != ImGui::GetItemID())
					{
						current_tab = ImGui::GetItemID();
						current_document = documentStruct.document;
						
						ImGui::DockBuilderRemoveNode(dockid); // Clear out existing layout
						ImGuiID dock_main_id = ImGui::DockBuilderAddNode(dockid, ImGuiDockNodeFlags_PassthruCentralNode); // Add empty node
						ImGui::DockBuilderSetNodeSize(dockid, ImGui::GetMainViewport()->WorkSize);

						ImGuiID dock_id_left_search = ImGui::DockBuilderSplitNode(dockid, ImGuiDir_Left, 0.25f, NULL, &dockid);
						ImGuiID dock_id_left_list = ImGui::DockBuilderSplitNode(dock_id_left_search, ImGuiDir_Down, 0.90f, NULL, &dock_id_left_search);
						ImGuiID dock_id_right_search = ImGui::DockBuilderSplitNode(dockid, ImGuiDir_Right, 0.75f, NULL, &dockid);
						ImGuiID dock_id_right_weapons = ImGui::DockBuilderSplitNode(dock_id_right_search, ImGuiDir_Down, 0.90f, NULL, &dock_id_right_search);

						ImGui::DockBuilderDockWindow("###left_search", dock_id_left_search);
						ImGui::DockBuilderDockWindow("###left_list", dock_id_left_list);
						ImGui::DockBuilderDockWindow("###right_search", dock_id_right_search);
						ImGui::DockBuilderDockWindow("###right_weapons", dock_id_right_weapons);

						ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoResize;
						ImGui::DockBuilderGetNode(dock_id_left_search)->LocalFlags |= flags;
						ImGui::DockBuilderGetNode(dock_id_left_list)->LocalFlags |= flags;
						ImGui::DockBuilderGetNode(dock_id_right_search)->LocalFlags |= flags;
						ImGui::DockBuilderGetNode(dock_id_right_weapons)->LocalFlags |= flags;

						ImGui::DockBuilderGetNode(dock_id_right_search)->SplitAxis = ImGuiAxis_None;
						ImGui::DockBuilderGetNode(dock_id_right_weapons)->SplitAxis = ImGuiAxis_None;

						ImGui::DockBuilderFinish(dockid);
						printf("Finish dock builder\n");

						ListBoxSearch();
					}

					if (listbox_item_last != listbox_item_current)
					{
						listbox_item_last = listbox_item_current;

						//RenderWeapon();
					}

					if (ImGui::Begin("Weapons###left_search"))
					{
						SearchBar("Search List", search_list,
							[&](const char* input) -> int
							{
								ListBoxSearch(input);

								printf("Search bar shit: %s\n", input);

								return 0;
							});

						ImGui::End();
					}

					ImGui::Begin("Weapons###left_list");
					{
						//printf("Size1: %f - %f - %f - %f\n", ImGui::GetContentRegionMax().x, ImGui::GetMainViewport()->Size.x, ImGui::GetMainViewport()->WorkSize.x, ImGui::GetMainViewport()->GetWorkCenter().x);

						ImGui::SetNextItemWidth(-1);
						ImGui::ListBox("##weapon_list", &listbox_item_current, listboxItems.data(), listboxItems.size(), listboxItems.size());
					}
					ImGui::End();

					if (ImGui::Begin("Search###right_search"))
					{
						SearchBar("Search Weapon", search_meta);

						ImGui::End();
					}

					if (listboxItems.size() > 0 && listbox_item_current < listboxItems.size())
					{
						sprintf_s(buffer, sizeof(buffer), "%s###right_weapons", listboxItems[listbox_item_current]);
						if (ImGui::Begin(buffer))
						{
							//printf("Size2: %f - %f - %f - %f\n", ImGui::GetContentRegionMax().x, ImGui::GetMainViewport()->Size.x, ImGui::GetMainViewport()->WorkSize.x, ImGui::GetMainViewport()->GetWorkCenter().x);

							RenderWeapon();

							ImGui::End();
						}
					}

					ImGui::PopStyleColor();

					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		} else {
			ImGui::PopStyleColor(2);

			printf("Else tabbar\n");
		}

		ImGui::End();

		ImGui::PopStyleColor();
	}

	/*pugi::xml_document* active_document = nullptr;
	size_t active_node_hash = 0;
	std::string active_attribute{ "" };
	ImGuiInputTextFlags active_text_flags = ImGuiInputTextFlags_None;
	char active_text[128];

	template<typename T>
	bool isType(std::string myString)
	{
		std::istringstream iss(myString);
		T f;
		iss >> std::noskipws >> f; // noskipws considers leading whitespace invalid
		// Check the entire string was consumed and if either failbit or badbit is set
		return iss.eof() && !iss.fail();
	}

	bool isNumber(const std::string& str)
	{
		for (char const& c : str) {
			if (std::isdigit(c) == 0) return false;
		}
		return true;
	}

	void RenderTest(pugi::xml_node& node, int index = 0)
	{
		ImGui::AlignTextToFramePadding();

		int childCount = std::distance(node.children().begin(), node.children().end());
		if (childCount > 0)
		{
			for (auto& child : node.children())
			{
				if (child.name()[0] == '\0')
					continue;

				char labelName[256];
				sprintf_s(labelName, sizeof(labelName), "%s##%d", child.name(), index);

				int hasChildren = std::distance(child.children().begin(), child.children().end());
				if (hasChildren > 0 && (child.first_child().type() != pugi::xml_node_type::node_pcdata))
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					ImGui::SetNextItemOpen(true);
					ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
					if (ImGui::TreeNodeEx(labelName, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth))
					{
						if (ImGui::IsItemHovered())
						{
							ImGui::SetNextWindowSize(ImVec2(400, 0));
							ImGui::BeginTooltip();
							ImGui::PushTextWrapPos(400.0f);
							ImGui::Text("Brief description\n\nLorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.");
							ImGui::PopTextWrapPos();
							ImGui::EndTooltip();
							//ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 100);
							//ImGui::SetTooltip("Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.");
							//ImGui::PopTextWrapPos();
						}

						RenderTest(child, index + 1);

						ImGui::TreePop();
					}

					ImGui::PopStyleColor();
				}
				else
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(child.name());
				}

				if (child.first_child().type() == pugi::xml_node_type::node_pcdata)
				{
					RenderTest(child.first_child());
				}

				for (auto& attribute : child.attributes())
				{
					ImGui::TableSetColumnIndex(1);

					//static char str0[128];
					//sprintf_s(str0, sizeof(str0), "%s", attribute.value());

					if (active_node_hash == child.hash_value())
					{
						ImGui::AlignTextToFramePadding();
						ImGui::Text("%s=", attribute.name());
						ImGui::SameLine(0.0f, 0.0f);
						ImGui::AlignTextToFramePadding();
						ImGui::InputText("##active_node", active_text, IM_ARRAYSIZE(active_text), active_text_flags);
					}
					else
					{
						ImGui::AlignTextToFramePadding();
						ImGui::Text("%s=%s", attribute.name(), attribute.value());
					}

					if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						//printf("Item clicked: %p - %s - %d\n", &node, node.path().c_str(), node.hash_value());
						sprintf_s(active_text, sizeof(active_text), attribute.value());
						if (isType<float>(active_text) || isType<int>(active_text))
							active_text_flags = ImGuiInputTextFlags_CharsDecimal;
						else
							active_text_flags = ImGuiInputTextFlags_None;

						active_node_hash = child.hash_value();
						active_attribute.assign(attribute.name());
					}
				}

				index = index + 1;
			}
		}

		if ((node.name()[0] != '\0' && node.text().as_string()[0] != '\0') || (node.type() == pugi::xml_node_type::node_pcdata))
		{
			ImGui::TableSetColumnIndex(2);

			//static char labelName[128];
			//sprintf_s(labelName, sizeof(labelName), "##%d", index);

			//static char str12[128];
			//sprintf(str12, "%s", node.text().as_string());

			//ImGui::SetNextItemWidth(200.0f);
			//ImGui::InputText(labelName, str12, IM_ARRAYSIZE(str12));

			if (active_node_hash == node.hash_value())
			{
				ImGui::AlignTextToFramePadding();
				ImGui::InputText("##active_node", active_text, IM_ARRAYSIZE(active_text), active_text_flags);
			}
			else
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text("%s", node.text().as_string());
			}

			if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				sprintf_s(active_text, sizeof(active_text), node.text().as_string());

				if (isType<float>(active_text) || isType<int>(active_text))
					active_text_flags = ImGuiInputTextFlags_CharsDecimal;
				else
					active_text_flags = ImGuiInputTextFlags_None;

				active_node_hash = node.hash_value();
			}
		}

		for (int i = 0; i < 3; i++)
		{
			ImGui::TableSetColumnEnabled(i, true);
			ImGui::TableSetColumnIndex(i);

			if (ImGui::IsItemHovered())
			{
				//printf("Row shit\n");
			}
		}
	}*/

private:
	pugi::xml_document* OpenDocument(std::string path)
	{
		pugi::xml_document* doc = new pugi::xml_document;
		pugi::xml_parse_result result = doc->load_file(path.c_str());

		if (!result)
		{
			printf("Unable to load xml\n");
			return nullptr;
		}

		this->m_XMLDocuments.push_back({ doc, std::filesystem::absolute(path).string() });

		return doc;
	}
};