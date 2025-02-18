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
	//OList<GiPathEl> paths;
public:
	GiLayerCmd cmds;

protected:
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
	void AddObject() {
		cmds.AddObject(0);
	}

	void AddHole(KiVec2 pos, GiLayerAppEl *app) {
		cmds.AddCmdHole(app, pos.x, pos.y);
	}

	void AddHoleToCircle(KiVec2 pos, float dia) {
		cmds.AddHoleToCircle(pos, dia);
	}

	void AddMove(KiVec2 pos) {
		cmds.AddCmdMove(0, pos);
	}

	void AddCir(KiVec2 pos) {
		cmds.AddCmdCir(0, pos);
	}

	void AddRot(KiVec2 pos, int opt) {
		cmds.AddCmdRotOnly(0, pos, opt);
	}

	void AddCircle(GiLayerAppEl *app, KiVec2 pos) {
		cmds.AddCmdCircle(app, pos);
	}

	void AddDrill(KiVec2 pos, float depth) {
		cmds.AddCmdDrill(pos, depth);
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
		GiLayerCmdEl *el = 0;
		LString ls;

		// Safe
		if (tool.depth < 0.1)
			tool.depth = 0.1;

		SString fspeed, pspeed;
		float v = tool.feed * 60;
		fspeed.Format("%f", v);

		v = tool.plunge * 60;
		pspeed.Format("%f", v);

		// Head
		ls + "; Gicad " + PROJECTVER->ver + "\r\n";
		ls + "G21 G17 G90" + "\r\n";
		ls + "G0 Z5" + "\r\n";
		ls + "S" + tool.speed + "M3\r\n";

		while (el = cmds.cmds.Next(el)) {
			if (el->type == GiLayerCmdMove) {
				ls + "G0Z10" + "\r\n";
				ls + "G0X" + (el->pos.x - zero.x) + "Y" + (el->pos.y - zero.y) + "\r\n";
				ls + "G0Z0" + "F" + pspeed + "\r\n";
			}

			if (el->type == GiLayerCmdDrill) {
				float depth = 0;
				while (depth < el->depth) {
					float d = min(el->depth, depth + tool.depth);
					
					ls + "G1Z" + (float(depth - 2) * -1) + "F" + pspeed + "\r\n";
					ls + "G1Z" + (d * -1) + "F" + pspeed + "\r\n";

					depth = d;
				}				
			}
		}

		ls + "G0 Z5" + "\r\n";
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
	//int PaintGetCount() {
	//	return paths.Size();
	//}

	int PaintGetData() {
		GlslObjectsData data;
		GlslObjectsColor color;

		GiLayerCmdEl *el = 0;
		int count = 0;
		int prount = cmds.cmds.Size() * show_perc / 100;

		// Write
		while (el = cmds.cmds.Next(el)){
			data.x = el->pos.x;
			data.y = el->pos.y;

			color.color = count <= prount ? this->color : this->color - .5;
			//count++;

			GlslObjectsBuffer.AddData(data, color);
		}		

		return count;
	}

	// Clean
	void Clean() {
		cmds.Clean();
	}

};
