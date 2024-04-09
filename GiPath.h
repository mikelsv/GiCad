enum GiPathType {
	GiPathTypeUnknown,
	GiPathTypePos,
	GiPathTypeMove,
	GiPathTypeDrill
};

// Path types:
// Move to pos(x, y);

class GiPathEl {
public:
	GiPathType type;
	float x, y, d;
};

class GiPath {
	// Main
	MString name;
	OList<GiPathEl> paths;

	// Tool
	GiToolsEl tool;
	float depth;

	// ImGui
	ImGuiCharId<11> gui_check;
	ImGuiCharId<8> gui_color;
	KiVec4 color;

	// Opt
	bool is_active;
	int layer_id;
	int show_perc;

public:
	GiPath() : gui_check("##Checkbox"), gui_color("##Color") {}

	void Init(int id) {		
		layer_id = id;
		is_active = 1;
		color = KiVec4(1, 0, 0, 1);
		show_perc = 100;

		SetName(MString("Path ") + itos(layer_id));
		UpdateGui();
	}

	// Add
	void AddMove(KiVec2 pos) {
		auto el = paths.NewE();
		el->type = GiPathTypeMove;
		el->x = pos.x;
		el->y = pos.y;
	}

	void AddDrill(KiVec2 pos, float depth) {
		auto el = paths.NewE();
		el->type = GiPathTypeDrill;
		el->x = pos.x;
		el->y = pos.y;
		el->d = depth;
	}

	// Get
	bool GetActive() {
		return is_active;
	}

	KiVec4 GetColor() {
		return color;
	}

	int GetLayerId() {
		return layer_id;
	}

	VString GetName() {
		return name;
	}

	int GetShowPerc() {
		return show_perc;
	}

	// Set
	void SetActive(bool val) {
		is_active = val;
	}

	void SetColor(const KiVec4& c) {
		color = c;
	}

	void SetName(VString n) {
		name = n;
	}

	void SetTool(GiToolsEl &t, float d) {
		tool = t;
		depth = d;
	}

	void SetShowPerc(int val) {
		show_perc = val;
	}

	// Data
	MString GetSaveData(KiVec2 zero) {
		GiPathEl* el = 0;
		LString ls;

		// Safe
		if (tool.depth < 0.1)
			tool.depth = 0.1;

		// Head
		ls + "; Gicad " + PROJECTVER->ver + "\r\n";
		ls + "G21 G17 G90" + "\r\n";
		ls + "M3 " + tool.speed + "\r\n";

		while (el = paths.Next(el)) {
			if (el->type == GiPathTypeMove) {
				ls + "G0Z10" + "\r\n";
				ls + "X" + (el->x - zero.x) + "Y" + (el->y - zero.y) + "\r\n";
			}

			if (el->type == GiPathTypeDrill) {
				float depth = 0;
				while (depth < el->d) {
					float d = min(el->d, depth + tool.depth);
					
					ls + "G1Z" + (float(depth - 2) * -1) + "F60" + "\r\n";
					ls + "G1Z" + (d * -1) + "F60" + "\r\n";

					depth = d;
				}				
			}
		}

		ls + "M5" + "\r\n";
		ls + "M30" + "\r\n";

		return ls;
	}

	// Gui
	void UpdateGui() {
		gui_check.SetId(layer_id);
		gui_color.SetId(layer_id);
	}

	char* GuiNameCheck() {
		return gui_check;
	}

	char* GuiNameColor() {
		return gui_check;
	}

	// Paint
	int PaintGetCount() {
		return paths.Size();
	}

	int PaintGetData() {
		GlslObjectsData data;
		GlslObjectsColor color;

		GiPathEl* el = 0;
		int count = 0;
		int prount = paths.Size() * show_perc / 100;

		// Write
		while (el = paths.Next(el)){
			data.x = el->x;
			data.y = el->y;

			color.color = count <= prount ? this->color : this->color - .5;
			count++;

			GlslObjectsBuffer.AddData(data, color);
		}		

		return count;
	}

	// Clean
	void Clean() {
		paths.Clean();
	}

};
