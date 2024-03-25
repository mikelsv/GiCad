enum GiPathType {
	GiPathTypeUnknown,
	GiPathTypeMove
};

// Path types:
// Move to pos(x, y);

class GiPathEl {
public:
	GiPathType type;
	float x, y;
};

class GiPath {
	// Main
	MString name;
	OList<GiPathEl> paths;

	// ImGui
	ImGuiCharId<11> gui_check;
	ImGuiCharId<8> gui_color;
	KiVec4 color;

	// Opt
	bool is_active;
	int layer_id;	


public:
	GiPath() : gui_check("##Checkbox"), gui_color("##Color") {}

	void Init(int id) {		
		layer_id = id;
		is_active = 1;
		color = KiVec4(1, 0, 0, 1);

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

	// Data
	MString GetSaveData(KiVec2 zero) {
		GiPathEl* el = 0;
		LString ls;

		// Head
		ls + "; Gicad " + PROJECTVER->ver + "\r\n";
		ls + "G21 G17 G90" + "\r\n";
		ls + "M3 1000" + "\r\n";


		while (el = paths.Next(el)) {
			ls + "G0Z10" + "\r\n";
			ls + "X" + (el->x - zero.x) + "Y" + (el->y - zero.y) + "\r\n";
			ls + "Z2" + "\r\n";
			ls + "G1Z-1F60" + "\r\n";
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

	int PaintGetData(GlslObjectsData* data, GlslObjectsColor* color) {
		GiPathEl* el = 0;
		int count = 0;

		while (el = paths.Next(el)){
			data[count].x = el->x;
			data[count].y = el->y;

			color[count].color = this->color;
			count++;
		}

		return count;
	}

	// Clean
	void Clean() {
		paths.Clean();
	}

};
