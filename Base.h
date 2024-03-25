#define GIGUI_GLOBAL_SCALE 2.8

enum GiCadZeroPoint {
	GiCadZeroPointNull,
	GiCadZeroPointLeftUp,
	GiCadZeroPointLeftDown,
	GiCadZeroPointRightUp,
	GiCadZeroPointRightDown
};

// ImGui CharId
template<int size>
class ImGuiCharId {
	char str[size + 4];

public:
	ImGuiCharId(const char(&input)[size]) {
		memcpy(str, input, size);
		str[size] = '\0';
	}

	void SetId(int id) {
		str[size - 1] = 48 + (id / 1000) % 10;
		str[size + 0] = 48 + (id / 100) % 10;
		str[size + 1] = 48 + (id / 10) % 10;
		str[size + 2] = 48 + (id / 1) % 10;
		str[size + 3] = 0;
	}

	void SetId2(int id) {
		str[size - 1] = 48 + (id / 10) % 10;
		str[size + 0] = 48 + (id / 1) % 10;
		str[size + 1] = 0;
	}

	operator char* () {
		return str;
	}
};

template <int size>
ImGuiCharId(const char(&data)[size]) -> ImGuiCharId<size>;

// ImGui CharId Ext
template<int maxsize>
class ImGuiCharIdExt {
	char data[maxsize + 1]; // +1 for safe
	int size;

public:
	// Init
	ImGuiCharIdExt() {
		size = 0;
		data[0] = 0;
	}

	ImGuiCharIdExt(const char* input) {
		if (!input) {
			size = 0;
			return;
		}

		size = strlen(input);

		memcpy(data, input, size);
		data[size] = '\0';
	}

	// Get 
	int GetSize() {
		return size;
	}

	int GetMaxSize() {
		return maxsize;
	}

	// Set Int
	void SetInt(int val, int len = 0) {
		if (len == 0)
			return SetIntL0(val);
	}

	void SetIntL0(int val) { // val > 0
		int sz = GetIntLen(val);
		int pow = Pow10(sz - 1);
		int count = 0;

		while (count < sz && size + count < maxsize && pow != 0) {
			data[size + count] = 48 + (val / pow) % 10;
			pow /= 10;
			count++;
		}

		data[size + count] = 0;
	}

	int GetIntLen(int val) {
		int count = 0;

		while (val > 0) {
			val /= 10;
			count++;
		}

		return count;
	}

	int Pow10(int val) {
		int res = 1;

		while (val > 0) {
			res *= 10;
			val--;			
		}

		return res;
	}

	// Set Str
	void SetStr(VString str) {
		int msize = min(maxsize, str.size());
		memcpy(data, str, msize);
		size = msize;
		data[size] = 0;
	}

	// Data
	VString GetStr() {
		return VString(data, size);
	}

	operator char* () {
		return data;
	}
};

template <int size>
ImGuiCharIdExt(const char(&data)[size]) -> ImGuiCharIdExt<size>;


// Gi Tools
class GiToolsEl {
public:
	// Base
	int id;
	MString name;
	float diameter;
	int speed;

	// Gui
	ImGuiCharIdExt<11> gui_id;
	ImGuiCharIdExt<255> gui_name;

	// Opt
	bool mod_name;
	int selected;

	// Init
	GiToolsEl() {
		id = 0;
		mod_name = 0;
		selected = 0;
	}

	// Get
	MString GetToolFile() {
		return LString() + "tools/user/" + id + ".tool";
	}

	// Gui
	void UpdateGui() {
		gui_id.SetInt(id);
		gui_name.SetStr(name);
	}
};

void GiProjectSelectedTool(GiToolsEl* tool);

class GiTools {
	OList <GiToolsEl> tools;
	GiToolsEl tool_tmp;
	GiToolsEl* tool;

	// Opt
	int max_id;
	bool tool_new, tool_select;

	// Gui
	bool show_tools, show_tools_edit;

public:
	GiTools() {
		max_id = 0;
		show_tools = 0;
		show_tools_edit = 0;
		tool_select = 0;
		tool = 0;
	}

	// Init
	void Init() {
		LoadTools();
	}

	// Load
	void LoadTools() {
		Readdir rd;
		rd.ReadDir("tools/user/");

		for (int i = 0; i < rd.size(); i++) {
			VSi& el = rd[i];

			LoadTool(el);
		}
	}

	// Show
	void ShowTools(bool v) {
		show_tools = v;
	}

	void ShowToolsEdit(bool v) {
		show_tools_edit = v;
	}

	// Tools
	void NewTool() {
		// New
		//tool = tools.NewE();
		tool = &tool_tmp;
		tool->id = ++max_id;

		// Gui
		SString s;
		tool->name = s.Format("Tool %d", tool->id);
		tool->UpdateGui();

		// Tool
		tool->diameter = 1;
		tool->speed = 1000;

		tool_new = 1;
		show_tools_edit = 1;
	}

	GiToolsEl* GetToolById(int id){
		GiToolsEl* el = 0;
		while (el = tools.Next(el)) {
			if (el->id == id)
				return el;
		}

		return 0;
	}

	void EditTool() {
		tool = GetSelectedTool();

		if (!tool)
			return;

		tool_tmp = *tool;
		tool = &tool_tmp;

		tool_new = 0;
		show_tools_edit = 1;
	}

	void SelectTool() {
		tool_select = 1;
		ShowTools(1);
	}

	void ApplyTool() {
		if (tool->mod_name)
			tool->name = tool->gui_name.GetStr();

		if (tool_new) {
			tool = tools.NewE();			
		}
		else
			tool = GetToolById(tool->id);

		if (!tool)
			return;

		*tool = tool_tmp;

		SaveTool();
	}

	void LoadTool(VSi& el) {
		if (el.key.str(-5) != ".tool")
			return;

		// Load
		MString data = LoadFile(LString() + "tools/user/" + el.key);
		XDataCont json(data);

		if (!data)
			return;

		// New
		tool = tools.NewE();

		// Set
		tool->id = json["tool.id"].toi();
		tool->name = json["tool.name"];
		tool->diameter = json["tool.dia"].tod();
		tool->speed = json["tool.speed"].toi();

		// Gui
		tool->UpdateGui();

		// Max Id
		max_id = max(max_id, tool->id);
	}

	void SaveTool() {
		if (tool->id == 1) {
			MkDir("tools");
			MkDir("tools/user");
		}

		JsonEncode json;
		json.Up("tool");
		json("id", itos(tool->id));
		json("name", tool->name);
		json("dia", itos(tool->diameter));
		json("speed", itos(tool->speed));

		json.Save(tool->GetToolFile());
	}

	void CancelTool(){
		if (tool_new) {
			max_id--;
		}
	}

	void DeleteTool(int req = 0) {
		if (!tool)
			return;

		// Request
		if (req) {
			SString ss;
			ss.Format("Tool id %d will be deleted. Yes/No?", tool->id);

			int res = tinyfd_messageBox("Delete Tool", ss, "yesno", "question", 0);
			if (!res)
				return;
		}

		DeleteFile(tool->GetToolFile());

		tools.Free(tool);
		tool = 0;
	}

	// Select
	GiToolsEl* GetSelectedTool() {
		GiToolsEl* el = 0;
		while (el = tools.Next(el)) {
			if (el->selected)
				return el;
		}

		return 0;
	}

	void UnSelectAllTools() {
		GiToolsEl* el = 0;
		while (el = tools.Next(el)) {
			el->selected = 0;
		}
	}

	// Render
	void Render() {
		if(show_tools)
			RenderTools();
	}

	void RenderTools() {
		ImGui::Begin("Tools");
		ImGui::SetWindowFontScale(GIGUI_GLOBAL_SCALE);

		if(tool_select)
			if (ImGui::Button("Select Tool")) {
				GiProjectSelectedTool(GetSelectedTool());
				ShowTools(0);
				tool_select = 0;
			}

		// Table
		if (ImGui::BeginTable("Table", 2)) {
			// Head
			ImGui::TableNextColumn();
			ImGui::Text("Id");

			ImGui::TableNextColumn();
			ImGui::Text("Tool");

			// Body
			GiToolsEl* el = 0;
			while (el = tools.Next(el)) {

				ImGui::TableNextColumn();
				ImGui::Text(el->gui_id);

				ImGui::TableNextColumn();
				//ImGui::Text(el->name);
				if (ImGui::Selectable(el->name, el->selected)) {
					UnSelectAllTools();
					el->selected = 1;
				}

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && el->selected) {
					tool = GetSelectedTool();
					EditTool();
				}
			}

			ImGui::EndTable();
		}

		// New tool
		if (ImGui::Button("New Tool")) {
			NewTool();
		}
		ImGui::SameLine();

		// Edit tool
		if (ImGui::Button("Edit")) {
			EditTool();
		}
		ImGui::SameLine();

		// Delete tool
		if (ImGui::Button("Delete")) {
			show_tools_edit = 0;
			tool = GetSelectedTool();
			DeleteTool(1);
		}
		ImGui::SameLine();

		// Close
		if (ImGui::Button("Close"))
			ShowTools(0);

		ImGui::End();

		if (show_tools_edit)
			RenderToolsEdit();
	}

	void RenderToolsEdit() {
		if (!tool)
			return;

		// Window
		ImGui::Begin("Tool edit");
		ImGui::SetWindowFontScale(GIGUI_GLOBAL_SCALE);

		// Id
		ImGui::Text("Id: ");
		ImGui::SameLine();
		ImGui::Text(tool->gui_id);

		// Name
		ImGui::Separator();	
		ImGui::Text("Name: ");
		ImGui::SameLine();
		if (ImGui::InputText("##Name", tool->gui_name, tool->gui_name.GetMaxSize()))  {
			tool->mod_name = 1;
		}

		// Diameter
		ImGui::Text("Diameter: ");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Diameter", &tool->diameter)) {}

		// Speed
		ImGui::Text("Spindle Speed: ");
		ImGui::SameLine();
		if (ImGui::InputInt("##Speed", &tool->speed)) {}

		// Buttons //
		ImGui::Separator();

		// Apply
		if (ImGui::Button("Apply")) {
			ApplyTool();
			ShowToolsEdit(0);
		}
		ImGui::SameLine();

		// Cancel
		if (ImGui::Button("Cancel")) {
			CancelTool();
			ShowToolsEdit(0);
		}

		ImGui::End();

	}


} GiTools;