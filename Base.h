#define GIGUI_GLOBAL_SCALE 2.5

#define GI_LAYER_CMD_G75 1
#define GI_LAYER_CMD_G03 2

#define GI_LAYER_PAINT_TRISZ	16

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

//template <int size>
//ImGuiCharId(const char(&data)[size]) -> ImGuiCharId<size>;

// ImGui CharId Ext
template<int maxsize>
class ImGuiCharIdExt {
	char data[maxsize + 1]; // +1 for safe
	int size;


public:
	// Init
	ImGuiCharIdExt() {		
		Clean();
	}

	ImGuiCharIdExt(const char* str) {
		SetStr(str);
	}

	// Get 
	int GetSize() {
		return size;
	}

	int GetMaxSize() {
		return maxsize;
	}

	// Int


	void SetInt(int val) {
		Clean();
		AddInt(val);
	}

	void AddInt(int val) {
		int len = GetIntLen(abs(val));
		int pow = Pow10(len - 1);
		int pos = size;
		int count = 0;
		
		// -
		if (val < 0 && pos < maxsize) {
			val *= -1;
			data[pos] = '-';
			pos++;
		}

		// Zero
		if (val == 0 && pos < maxsize) {
			data[pos] = '0';
			pos++;
		}

		// Int
		while (count < len && pos < maxsize && pow != 0) {
			data[pos] = 48 + (val / pow) % 10;
			pow /= 10;
			count++;
			pos++;
		}

		// Close
		size = pos;
		data[size] = 0;
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

	// Float
	void SetFloat(float val) {
		Clean();
		AddFloat2(val);
	}

	void AddFloat2(float val) {
		AddInt(val);

		if (val < 0)
			val *= -1;

		if (size < maxsize) {
			data[size] = '.';
			size++;
		}

		AddInt(int(val * 10) % 10);
		AddInt(int(val * 100) % 10);
	}

	// Str
	void SetStr(VString str) {
		Clean();
		AddStr(str);
	}

	void AddStr(VString str) {
		if (!str)		
			return;
	
		int len = str.size();
		if (len > maxsize - size)
			len = maxsize - size;

		memcpy(data + size, str, len);
		size += len;
		data[size] = '\0';
	}

	// Data
	VString GetStr() {
		return VString(data, size);
	}

	operator char* () {
		return data;
	}

	// Clean
	void Clean() {
		size = 0;
		data[size] = 0;
	}
};

//template <int size>
//ImGuiCharIdExt(const char(&data)[size]) -> ImGuiCharIdExt<size>;

// Gi Image
class GiImage {
	int width, height;
	unsigned char* data;
	GLuint texture;

public:
	GiImage() {
		width = 0;
		height = 0;
		data = 0;
		texture = 0;
	}

	bool Open(VString file) {
		data = stbi_load(file, &width, &height, NULL, 4);
		if (data == 0)
			return false;

		// Create a OpenGL texture identifier
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// Setup filtering parameters for display
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

		// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);

		return true;
	}

	operator void* () {
		return (void*)(intptr_t)texture;
	}

};

class GiImages {
public:

	GiImage new_project, open_project, save_project, save_project2;

	void Init() {
		new_project.Open("icons/new_project.png");
		open_project.Open("icons/open_project.png");
		save_project.Open("icons/save_project.png");
		save_project2.Open("icons/save_project2.png");
	}

} GiImages;

// Gi Tools //
// Tools Type
enum GiToolsType {
	GiToolsTypeDrill = 0,
	GiToolsTypeEndMill = 1,
	GiToolsTypeGraver = 2,
	GiToolsTypeLaser = 3
};

class GiToolsEl {
public:
	// Base	
	int id;
	GiToolsType type;
	MString name;
	float diameter, depth;
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
		tool->type = (GiToolsType)json["tool.type"].toi();
		tool->name = json["tool.name"];
		tool->diameter = json["tool.dia"].tod();
		tool->depth = json["tool.depth"].tod();
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
		json("type", itos(tool->type));
		json("name", tool->name);
		json("dia", dtos(tool->diameter));
		json("depth", dtos(tool->depth));
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
		if (show_tools)
			RenderTools();
	}

	void RenderTools() {
		ImGui::Begin("Tools", &show_tools);
		ImGui::SetWindowFontScale(GIGUI_GLOBAL_SCALE);

		if (tool_select)
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
			GiToolsEl *el = 0;
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

		ImGui::Separator();

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
		bool show_window;

		if (!tool)
			return;

		// Window
		ImGui::Begin("Tool edit", &show_window);
		ImGui::SetWindowFontScale(GIGUI_GLOBAL_SCALE);

		// Id
		ImGui::Text("Id: ");
		ImGui::SameLine();
		ImGui::Text(tool->gui_id);

		// Type
		const char *items[] = { "Drill", "EndMill", "Graver", "Laser" };
		int item = tool->type;

		ImGui::SameLine();
		if (ImGui::Combo("##ToolType", &item, items, IM_ARRAYSIZE(items))){
			tool->type = (GiToolsType)item;
		}

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

		// Depth
		ImGui::Text("Depth: ");
		ImGui::SameLine();
		if (ImGui::InputFloat("##Depth", &tool->depth)) {}

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
		if (ImGui::Button("Cancel") || !show_window) {
			CancelTool();
			ShowToolsEdit(0);
		}

		ImGui::End();

	}


} GiTools;

class GiSortVec2El {
public:
	KiVec2 pos;
	void* poi;
};

class GiSortVec2 {
	MString data;
	GiSortVec2El* els;
	int size, maxsize;

public:
	GiSortVec2() {
		els = 0;
		size = maxsize = 0;
	}

	void Reserve(int newsize) {
		data.Reserve(newsize * sizeof(GiSortVec2El));
		els = (GiSortVec2El*)data.data;
		maxsize = newsize;
	}

	//void Init(int asize) {
	//	data.Reserve(asize * sizeof(GiSortVec2El));
	//	els = (GiSortVec2El*) data.data;
	//	size = asize;
	//	pos = 0;
	//}

	bool Add(KiVec2 p, void *d){
		if (size == maxsize)
			Reserve(maxsize + 10000);
		//if (pos >= size)
		//	return 0;

		els[size].pos = p;
		els[size].poi = d;

		size++;
		return 1;
	}

	void SortMinPath() {
		if (!size)
			return;

		// Data
		MString d = data;
		
		// Pointers
		GiSortVec2El *el, *sel, *to = els + size, *res = (GiSortVec2El*)d.data;

		// Pos
		KiVec2 pos = els[0].pos;
		int count = 0;
		
		while (count < size) {
			float len = 99999;
			el = els;
			
			for (el; el < to; el++) {
				if (!el->poi)
					continue;

				float l = KiVec2(pos - el->pos).Length();
				if (l < len) {
					len = l;
					sel = el;
				}
			}

			// Selected
			res->poi = sel->poi;
			res->pos = sel->pos;
			sel->poi = 0;
			pos = sel->pos;
			
			count++;
			res++;
		}

		// Result
		data -= d;
		els = (GiSortVec2El*)data.data;
	}

	int GetSize() {
		return size;
	}

	GiSortVec2El* GetData() {
		return els;
	}

};