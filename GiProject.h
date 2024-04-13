//#define GIPROJECT_RENDER_GLSL


void GiWindowsUpdateTitle();
void GiWindowsResetView100p();
void GiWindowsInsertPopUp(VString text);

// Project type
enum GiProjectType {
	GiProjectTypeUnknown,
	GiProjectTypeGerber,
	GiProjectTypeDrill,
	GiProjectTypePath
};

// Project Prog
enum GiProjectProg {
	GiProjectProgUnknown,
	GiProjectProgDrill,
	GiProjectProgLaser
};

class GiProject{
	// Project
	MString name, path;

	// Layers & files
	OList<GrblFile> grbs;
	OList<DrillFile> drls;
	OList<GiLayer> lays;
	OList<GiPath> paths;

	// Show
	bool show_grbs, show_drls, show_lays, show_prog;
	int tree_click, tree_selected, tree_type;

	// Opt
	KiVec2 zero_pos;
	int layer_id;
	bool is_paint, is_first;

	// Prog
	GiProjectProg prog_type;
	int prog_drill_file_id;
	float prog_drill_depth;
	bool prog_drill_check;
	char* prog_drill_file_name;

	// Path Show Options
	int opt_path_show;

	// Drill
	GiToolsEl tool;

public:
	// New
	GiProject(){
		NewProject();

		layer_id = 0;
		is_paint = 0;
		is_first = 1;

		tree_click = 0;
		tree_selected = 0;

		prog_type = GiProjectProgUnknown;
		prog_drill_file_name = 0;
		prog_drill_depth = 1;

		opt_path_show = 0;
	}

	VString GetProjectName(){
		return name;
	}

	void NewProject(){
		// If exist
		if(path || lays.Size()){
			int res = tinyfd_messageBox("New Project", "Project not be saved. Really create new?", "yesno", "question", 0);
			if(!res)
				return ;
		}

		Clean();
		name = "Untitled.gic";

		GiWindowsUpdateTitle();
	}

	void OpenProject(){
		char const * lFilterPatterns[] = { "*.gic" };

		// If exist
		if(path || lays.Size()){
			int res = tinyfd_messageBox("New Project", "Project not be saved. Really create new?", "yesno", "question", 0);
			if(!res)
				return ;
		}

		char const * selection = tinyfd_openFileDialog( // there is also a wchar_t version
			"Open project", // title
			0, // optional initial directory
			1, // number of filter patterns
			lFilterPatterns, // char const * lFilterPatterns[2] = { "*.txt", "*.jpg" };
			NULL, // optional filter description
			0 // forbid multiple selections
        );

		if(!selection)
			return ;

		Clean();

		LoadProject(selection);
	}

	void LoadProject(VString file){
		ILink link(file);

		path = file;
		name = link.file;
		GiWindowsUpdateTitle();

		// Load
		MString data = LoadFile(file);
		XDataCont json(data);

		XDataPEl files = json("files.drill")->a();
		while(files){
			AddDrlFileOnly(files["file"], files["id"].toi(), files["mtime"].toi(), files["size"].toi());
			files = files.n();
		}
	}

	bool SavePtoject() {
		JsonEncode json;
		DrillFile *drl = 0;

		if (!path) {
			const char *ext[] = { "*.gic" };
			char const *file = tinyfd_saveFileDialog("Sace project", 0, 1, ext, 0);
			if (!file)
				return 0;

			ILink link(file);

			path = file;
			name = link.file;
			GiWindowsUpdateTitle();
		}

		// Project
		json.Up("project");
		json("name", name);
		json("path", path);
		json("mtime", itos(time()));
		json.Down();

		// Files
		json.Up("files");
		json.Up("drill");
		while (drl = drls.Next(drl)) {
			json("file", drl->GetPath());
			json("id", itos(drl->GetLayerId()));
			json("mtime", itos(drl->GetState().st_mtime));
			json("size", itos(drl->GetState().st_size));
		}

		json.Down();
		json.Down();

		json.Save(path);
	}

	// Is
	bool IsPaintLayers(){
		return is_paint;
	}

	// Get
	KiVec2 GetZeroPoint() {
		return zero_pos;
	}

	// Set
	void SetPaint(){
		is_paint = 1;
	}

	void SetProg(GiProjectProg val) {
		prog_type = val;
	}

	void SetZeroPoint(KiVec2 z) {
		zero_pos = z;
	}

	// On
	void OnDrop() {
		if (is_first && (grbs.Size() || drls.Size())) {
			is_first = 0;
			GiWindowsResetView100p();
		}
	}

	/*
	bool PaintCmdRot(GiLayer *el, GiLayerCmdEl *cel_pos, GiLayerCmdEl *cel_cir, GiLayerCmdEl *cel) {
		if (!cel_pos || !cel_cir)
			return 0;

		GlslObjectsData data;
		GlslObjectsColor color;

		color.SetColor(el->GetColor());

		bool g02 = !(cel->opt & GI_LAYER_CMD_G03), g75 = (cel->opt & GI_LAYER_CMD_G75);

		// G02 or G03 to G01
		double sx = cel_pos->pos.x, sy = cel_pos->pos.y;
		double ex = cel->pos.x, ey = cel->pos.y;
		double cx = sx + cel_cir->pos.x, cy = sy + cel_cir->pos.y;
		double rad = pow(pow(cx - sx, 2) + pow(cy - sy, 2), 0.5);

		// Start / end angles
		double sangle = atan2(sy - cy, sx - cx);
		double eangle = atan2(ey - cy, ex - cx);		

		if (g02)
			if (sangle < eangle)
				eangle -= PI * 2;

		if (!g02)
			if (sangle > eangle)
				eangle += PI * 2;

		if (g75) {
			if (eangle == sangle)
				eangle += g02 ? -PI * 2 : PI * 2;
		}

		int points = int(ceil(abs(eangle - sangle) / (PI / 180)));
		//int points = int(ceil(abs(eangle - sangle)) * rad * rad);

		for (int i = 0; i < points + 1; i++) {
			double angle = sangle + (eangle - sangle) * i / points;

			data.x = cx + rad * cos(angle);
			data.y = cy + rad * sin(angle);

			GlslObjectsBuffer.AddData(data, color);
			GlslObjectsBuffer.AddData(data, color);
		}

		GlslObjectsBuffer.AddData(data, color);
		return 1;
	}*/

	bool PaintLayers(GlslMain &glsl){
		GiLayer *el = 0;
		GiPath *pel = 0;

		// Objects
		GlslObjectsHead head;
		GlslObjectsHeadExt hext;
		GlslObjectsData data;
		GlslObjectsColor color;

		GlslObjectsBuffer.Clean();

		// Write layers data
		while (el = lays.Next(el)) {

			GiLayerCmd *cel = 0, *cel_pos = 0, *cel_cir = 0;

			if (!el->GetActive())
				continue;

			// Get Base file
			GiBaseFile *base = GetGrbl(el->GetId());
			if (!base)
				base = GetDrill(el->GetId());

			// Unselect
			bool unselect = 0;
			if (base) {
				unselect = base->GetUnselect();
				base->SetUnselect(0);
			}

			// Paint
			el->cmds.Paint(el->GetColor(), unselect);

			/*/ Circles
			while(cel = el->cmds.Next(cel)){
				// Object
				if (cel->type == GiLayerCmdObject) {
					head.type = GL_LINES;
					GlslObjectsBuffer.AddHead(head);
				}

				// Move
				if (cel->type == GiLayerCmdMove) {
					data.x = cel->pos.x;
					data.y = cel->pos.y;					
					color.SetColor(el->GetColor());
					cel_pos = cel;

					GlslObjectsBuffer.AddData(data, color);
				}

				// Circle
				if (cel->type == GiLayerCmdCir) {
					cel_cir = cel;
				}

				// Rotate
				if (cel->type == GiLayerCmdRot) {
					PaintCmdRot(el, cel_pos, cel_cir, cel);
				}

				// Hole
				if (cel->type == GiLayerCmdHole) {
					head.type = GL_POLYGON;
					GlslObjectsBuffer.AddHead(head);

					KiVec4 col = el->GetColor();
					if (cel->app && cel->app->selected) {
						float c = col.x;
						col.x = col.z;
						col.z = c;
					}

					for (int i = 0; i < GIPROJECT_RENDER_TRISZ; i++) {
						float angle = i * 2 * PI / GIPROJECT_RENDER_TRISZ;
						data.x = cel->pos.x + (cos(angle) * cel->app->dia / 2);
						data.y = cel->pos.y + (sin(angle) * cel->app->dia / 2);
						data.z = 0;
						//data ++;

						color.SetColor(col);
						GlslObjectsBuffer.AddData(data, color);
					}
				}
			}*/
		}

		while (pel = paths.Next(pel)) {
			if (!pel->GetActive())
				continue;

			head.type = GI_GL_PATH;
			GlslObjectsBuffer.AddHead(head, hext);

			pel->PaintGetData();
		}

		is_paint = 0;
		GlslObjectsBuffer.SetUpdate(1);

		return 1;
	}

	// Gerber
	bool AddGbrFile(VString file){
		GrblFile *el = GetGrblByPath(file);
		if(!el){
			el = grbs.NewE();
			el->SetLayerId(NewLayer(el));
			el->UpdateGui();
		}

		show_grbs = 1;

		SetPaint();
		return el->Open(file);
	}

	bool AddGbrFileOnly(VString file, int layer_id, int mtime, int size) {
		if (!file || !layer_id)
			return 0;

		GrblFile *el = GetGrblByPath(file);
		if (!el) {
			el = grbs.NewE();
			el->SetLayerId(NewLayer(el, layer_id));
			el->UpdateGui();
		}

		el->SetFile(file, mtime, size);

		show_grbs = 1;
		SetPaint();
		return 1;
	}

	GrblFile* GetGrbl(int id) {
		GrblFile* el = 0;
		while (el = grbs.Next(el)) {
			if (el->GetLayerId() == id)
				return el;
		}

		return 0;
	}

	GrblFile* GetGrblByPath(VString file){
		GrblFile *el = 0;
		while(el = grbs.Next(el)){
			if(file == el->GetPath())
				return el;
		}

		return 0;
	}

	void DelGrbl(int id) {
		GrblFile* el = GetGrbl(id);
		grbs.Free(el);
		DelLayer(id);
	}

	// Drill
	bool AddDrlFile(VString file){
		DrillFile *el = GetDrillByPath(file);
		if(!el){
			el = drls.NewE();
			el->SetLayerId(NewLayer(el));
			el->UpdateGui();
		}

		show_drls = 1;

		SetPaint();
		return el->Open(file);
	}

	bool AddDrlFileOnly(VString file, int layer_id, int mtime, int size) {
		if (!file || !layer_id)
			return 0;

		DrillFile *el = GetDrillByPath(file);
		if (!el) {
			el = drls.NewE();
			el->SetLayerId(NewLayer(el, layer_id));
			el->UpdateGui();
		}

		el->SetFile(file, mtime, size);

		show_drls = 1;
		SetPaint();
		return 1;
	}

	DrillFile* GetDrill(int id) {
		DrillFile* el = 0;
		while (el = drls.Next(el)) {
			if (el->GetLayerId() == id)
				return el;
		}

		return 0;
	}

	DrillFile* GetDrillByPath(VString file){
		DrillFile *el = 0;
		while(el = drls.Next(el)){
			if(file == el->GetPath())
				return el;
		}

		return 0;
	}

	void DelDrill(int id) {
		DrillFile* el = GetDrill(id);
		drls.Free(el);
		DelLayer(id);
	}

	// Layer
	int NewLayer(GiBaseFile* file, int id = 0) {
		GiLayer* el = lays.NewE();
		el->Clean();

		if(!id)
			el->SetId(++layer_id);
		else {
			el->SetId(id);
			layer_id = max(layer_id, id);
		}

		el->SetFile(file);

		return layer_id;
	}

	GiLayer* GetLayerById(int id) {
		GiLayer* el = 0;
		while (el = lays.Next(el)) {
			if (el->GetId() == id)
				return el;
		}

		return 0;
	}

	void DelLayer(int id) {
		GiLayer* el = GetLayerById(id);
		lays.Free(el);
		SetPaint();
	}

	KiVec4 GetLayersRect() {
		KiVec4 rect, rc;

		GiLayer* el = 0;
		while (el = lays.Next(el)) {
			rc = el->cmds.GetRect();

			if (!rc.IsNull())
				if (!rect.IsNull())
					rect.UnionRect(rc);
				else
					rect = rc;
		}

		return rect;
	}

	// Paths
	GiPath* GetPath(int id) {
		GiPath* el = 0;
		while (el = paths.Next(el)) {
			if (el->GetLayerId() == id)
				return el;
		}

		return 0;
	}

	void SavePath(int id, VString path) {
		GiPath* el = GetPath(id);
		if (!el)
			return;

		MString data = el->GetSaveData(zero_pos);
		int res = SaveFile(path, data);

		if(res)
			GiWindowsInsertPopUp(LString() + "Saved success: " + path);
		else
			GiWindowsInsertPopUp(LString() + "[WARNING!] Save fail: " + path);
	}

	void DelPath(int id) {
		GiPath* el = GetPath(id);
		paths.Free(el);
	}

	// Render
	void GuiRender() {
		GuiTreeRender();
		GuiOptionRender();
		GuiProg();
	}

	void GuiTreeRender(){
		bool isChecked = false;
		KiVec4 color = { 1, 0., 0, 1 };

		ImGui::SetNextWindowPos(ImVec2(0, 45));
		ImGui::Begin("Layers", nullptr, 0);
		ImGui::SetWindowFontScale(GIGUI_GLOBAL_SCALE);

		// Gerber
		if (ImGui::TreeNodeEx("Gerber", ImGuiTreeNodeFlags_DefaultOpen)) {
			GrblFile* el = 0;
			while (el = grbs.Next(el)) {
				VString path = el->GetFile();
				color = el->GetColor();

				// CheckBox
				isChecked = el->GetActive();
				if (ImGui::Checkbox(el->GuiNameCheck(), &isChecked)) {
					el->SetActive(isChecked);
					if(isChecked)
						el->SetUnselect(1);
					SetPaint();
				}

				// Color
				ImGui::SameLine();
				if (ImGui::ColorEdit4(el->GuiNameColor(), &color.x, ImGuiColorEditFlags_NoInputs)) {
					el->SetColor(color);
					GiProjectLayerSetColor(el->GetLayerId(), color);
				}

				// Text
				ImGui::SameLine();
				ImGui::TextUnformatted(path, path.end());

				// Right click
				if (ImGui::IsItemClicked(1)) {
					tree_selected = el->GetLayerId();
					tree_type = GiProjectTypeGerber;
					tree_click = 1;
				}
			}

			ImGui::TreePop();
		}

		// Drill
		if (ImGui::TreeNodeEx("Drill", ImGuiTreeNodeFlags_DefaultOpen)) {
			DrillFile* el = 0;
			while (el = drls.Next(el)) {
				VString path = el->GetFile();
				color = el->GetColor();

				// CheckBox
				isChecked = el->GetActive();
				if (ImGui::Checkbox(el->GuiNameCheck(), &isChecked)) {
					el->SetActive(isChecked);
					if (isChecked)
						el->SetUnselect(1);
					SetPaint();
				}

				// Color
				ImGui::SameLine();
				if (ImGui::ColorEdit4(el->GuiNameColor(), &color.x, ImGuiColorEditFlags_NoInputs)) {
					el->SetColor(color);
					GiProjectLayerSetColor(el->GetLayerId(), color);
				}

				// Text
				ImGui::SameLine();
				ImGui::TextUnformatted(path, path.end());

				// Right click
				if (ImGui::IsItemClicked(1)) {
					tree_selected = el->GetLayerId();
					tree_type = GiProjectTypeDrill;
					tree_click = 1;
				}
			}

			ImGui::TreePop();
		}		

		// Layers
		if (ImGui::TreeNodeEx("Layers", ImGuiTreeNodeFlags_DefaultOpen)) {
			GiLayer* el = 0;
			while (el = lays.Next(el)) {
				//VString path = el->GetPath();
				//ImGui::TextUnformatted(path, path.end());
			}

			ImGui::TreePop();
		}		

		// Path
		if (ImGui::TreeNodeEx("Paths", ImGuiTreeNodeFlags_DefaultOpen)) {
			GiPath* el = 0;
			while (el = paths.Next(el)) {
				VString name = el->GetName();
				color = el->GetColor();

				// CheckBox
				isChecked = el->GetActive();
				if (ImGui::Checkbox(el->GuiNameCheck(), &isChecked)) {
					el->SetActive(isChecked);
					SetPaint();
				}

				// Color
				ImGui::SameLine();
				if (ImGui::ColorEdit4(el->GuiNameColor(), &color.x, ImGuiColorEditFlags_NoInputs)) {
					el->SetColor(color);
					GiProjectLayerSetColor(el->GetLayerId(), color);
				}

				// Text
				ImGui::SameLine();
				ImGui::TextUnformatted(name, name.end());

				// Right click
				if (ImGui::IsItemClicked(1)) {
					tree_selected = el->GetLayerId();
					tree_type = GiProjectTypePath;
					tree_click = 1;
				}				
			}

			ImGui::TreePop();
		}

		// Tree menu
		if (tree_click) {
			ImGui::OpenPopup("LayersTreeMenu");
			tree_click = 0;
		}
		
		GuiShowTreeMenu();

		ImGui::End();
	}

	void GuiShowTreeMenu() {
		//if (!show_tree_menu)
		//	return;

		//ImGui::OpenPopup("TreeMenu");
		if (ImGui::BeginPopupContextItem("LayersTreeMenu")) {
			// Save
			if (tree_type == GiProjectTypePath) {
				if (ImGui::MenuItem("Save path")) {
					const char* ext[] = { "*.tap", "*.bin" };

					char* file = tinyfd_saveFileDialog("Save path", "", 1, ext, "Toolpath file");
					if (file)
						SavePath(tree_selected, file);
				}
				ImGui::Separator();

				// Control
				if (ImGui::MenuItem("Show options")) {
					opt_path_show = tree_selected;
				}
			}

			// Close / Delete
			if (ImGui::MenuItem(tree_type != GiProjectTypePath ? "Close" : "Delete")) {
				DelGrbl(tree_selected);
				DelDrill(tree_selected);
				DelPath(tree_selected);
			}

			//ImGui::Separator();
			//ImGui::Text("12345!");

			ImGui::EndPopup();
		}
	}

	void GuiOptionRender() {
		if (!opt_path_show)
			return;

		GiPath* el = GetPath(opt_path_show);
		if (!el)
			return;

		ImGui::Begin("Path show options", nullptr, 0);
		ImGui::SetWindowFontScale(GIGUI_GLOBAL_SCALE);

		ImGui::Text("Show: ");
		ImGui::SameLine();

		int val = el->GetShowPerc();
		if (ImGui::SliderInt("%", &val, 0, 100)) {
			el->SetShowPerc(val);
			SetPaint();
		}
		ImGui::SameLine();

		// Close
		if (ImGui::Button("Close")) {
			opt_path_show = 0;
		}

		ImGui::End();
	}

	void SelectedTool(GiToolsEl* t) {
		if (!t)
			return;

		tool = *t;
	}

	void GuiProg() {
		if (prog_type == GiProjectProgDrill)
			GuiProgDrill();

		if (prog_type == GiProjectProgLaser)
			GuiProgLaser();
	}

	void GuiProgDrill() {
		bool show_window;

		ImGui::Begin("Drill", &show_window, 0);
		ImGui::SetWindowFontScale(GIGUI_GLOBAL_SCALE);
		ImGui::SameLine();

		ImGui::Text("Depth:");
		ImGui::SameLine();

		float floatValue = 1.0f;
		if (ImGui::InputFloat("##Depth", &prog_drill_depth)) {}

		// Combo
		ImGui::Text("File:");
		ImGui::SameLine();

		DrillFile* el = 0;
		int count = 0;

		if (ImGui::BeginCombo("##Combo", prog_drill_file_name)) {
			while (el = drls.Next(el)) {
				//for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
				bool isSelected = (prog_drill_file_id == el->GetLayerId());
				if (ImGui::Selectable(el->GetFile(), isSelected)) {
					prog_drill_file_id = el->GetLayerId();
					prog_drill_file_name = el->GetFile();
					prog_drill_check = GetLayerById(prog_drill_file_id)->AppGetCheckAll();
				}
				//if (isSelected) {
				//	ImGui::SetItemDefaultFocus();
				//}
			//}					
			}

			ImGui::EndCombo();
		}

		// Tool
		if (ImGui::Button(tool.id ? tool.gui_name.GetStr() : VString("Select tool"))) {
			GiTools.SelectTool();
		}

		// Table
		if (ImGui::BeginTable("Table", 2)) {
			// Head
			ImGui::TableNextColumn();
			if (ImGui::Checkbox("##Checkbox", &prog_drill_check)) {
				GetLayerById(prog_drill_file_id)->AppSetCheckAll(prog_drill_check);
			}

			ImGui::TableNextColumn();
			ImGui::Text("Size");

			//ImGui::TableNextColumn();
			//ImGui::Text("-");

			// Data
			auto el = GetLayerById(prog_drill_file_id);
			GiLayerAppEl* app = 0;

			if (el)
				while (app = el->NextApp(app)) {
					ImGui::TableNextColumn();
					if (ImGui::Checkbox(app->c_check, &app->checked)) {
						prog_drill_check = GetLayerById(prog_drill_file_id)->AppGetCheckAll();
					}
					ImGui::SameLine();

					if (ImGui::Selectable(app->c_type, app->selected)) {
						el->AppSetSelectAll(0);
						app->selected = 1;
						SetPaint();
					}

					ImGui::TableNextColumn();
					if (ImGui::Selectable(app->c_dia, app->selected)) {
						el->AppSetSelectAll(0);
						app->selected = 1;
						SetPaint();
					}

					//ImGui::TableNextColumn();
					//ImGui::Text("Tool");
				}

			ImGui::EndTable();
		}

		// Buttons
		if (ImGui::Button("Create") && tool.id != 0) {
			CreateDrillPath();
		}
		ImGui::SameLine();

		if (ImGui::Button("Close") || !show_window) {
			prog_type = GiProjectProgUnknown;
		}

		ImGui::End();
	}

	void GuiProgLaser() {
		bool show_window;

		ImGui::Begin("Laser", &show_window, 0);
		ImGui::SetWindowFontScale(GIGUI_GLOBAL_SCALE);
		ImGui::SameLine();

		ImGui::Text("Depth:");
		ImGui::SameLine();

		float floatValue = 1.0f;
		if (ImGui::InputFloat("##Depth", &prog_drill_depth)) {}

		// Combo
		ImGui::Text("File:");
		ImGui::SameLine();

		DrillFile *el = 0;
		int count = 0;

		if (ImGui::BeginCombo("##Combo", prog_drill_file_name)) {
			while (el = drls.Next(el)) {
				//for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
				bool isSelected = (prog_drill_file_id == el->GetLayerId());
				if (ImGui::Selectable(el->GetFile(), isSelected)) {
					prog_drill_file_id = el->GetLayerId();
					prog_drill_file_name = el->GetFile();
					prog_drill_check = GetLayerById(prog_drill_file_id)->AppGetCheckAll();
				}
				//if (isSelected) {
				//	ImGui::SetItemDefaultFocus();
				//}
			//}					
			}

			ImGui::EndCombo();
		}

		// Tool
		if (ImGui::Button(tool.id ? tool.gui_name.GetStr() : VString("Select tool"))) {
			GiTools.SelectTool();
		}

		// Table
		if (ImGui::BeginTable("Table", 2)) {
			// Head
			ImGui::TableNextColumn();
			if (ImGui::Checkbox("##Checkbox", &prog_drill_check)) {
				GetLayerById(prog_drill_file_id)->AppSetCheckAll(prog_drill_check);
			}

			ImGui::TableNextColumn();
			ImGui::Text("Size");

			//ImGui::TableNextColumn();
			//ImGui::Text("-");

			// Data
			auto el = GetLayerById(prog_drill_file_id);
			GiLayerAppEl *app = 0;

			if (el)
				while (app = el->NextApp(app)) {
					ImGui::TableNextColumn();
					if (ImGui::Checkbox(app->c_check, &app->checked)) {
						prog_drill_check = GetLayerById(prog_drill_file_id)->AppGetCheckAll();
					}
					ImGui::SameLine();

					if (ImGui::Selectable(app->c_type, app->selected)) {
						el->AppSetSelectAll(0);
						app->selected = 1;
						SetPaint();
					}

					ImGui::TableNextColumn();
					if (ImGui::Selectable(app->c_dia, app->selected)) {
						el->AppSetSelectAll(0);
						app->selected = 1;
						SetPaint();
					}

					//ImGui::TableNextColumn();
					//ImGui::Text("Tool");
				}

			ImGui::EndTable();
		}

		// Buttons
		if (ImGui::Button("Create") && tool.id != 0) {
			CreateDrillPath();
		}
		ImGui::SameLine();

		if (ImGui::Button("Close") || !show_window) {
			prog_type = GiProjectProgUnknown;
		}

		ImGui::End();
	}

	// Mouse
	void OnMouseMove(KiVec2 mouse, KiVec2 mouse2, bool area, float zoom, bool ctrl_on) {
		GiLayer *el;
		
		// Test
		GlslObjectsHead *head = GlslObjectsBuffer.GetHead(), *head_end = head + GlslObjectsBuffer.GetHeadCount();
		GlslObjectsData *data = GlslObjectsBuffer.GetData();
		GlslObjectsColor *color = GlslObjectsBuffer.GetColors();
		KiVec4 mpos(mouse.x, mouse.y, mouse2.x, mouse2.y);
		float line = .5;

		while (head < head_end) {
			int cross = 0, pos = head->pos, to = pos + head->size;
			//bool inside = false;
			bool select = 0, aselect = 0;

			// Polygon
			if (head->type == GL_POLYGON) {
				bool inside = false;

				// Mouse once
				for (int i = 0, j = head->size - 1; i < head->size; j = i++){
					if ((data[pos + i].y > mouse.y) != (data[pos + j].y > mouse.y) &&
						mouse.x < (data[pos + j].x - data[pos + i].x) * (mouse.y - data[pos + i].y) / (data[pos + j].y - data[pos + i].y) + data[pos + i].x){
						inside = !inside;
					}
				} 

				// Mouse area
				if(area)
					for (int i = 0; i < head->size; i++) {
						if (mpos.InRect(KiVec2(data[pos + i].x, data[pos + i].y))) {
							aselect = 1;
							break;
						}
					}

				select = inside;
			}
			else if (head->type == GL_LINES) {
				if ((head->size & 2) != 0 && !_error_gerber_reader_align) {
					_error_gerber_reader_align = 1;
					GiWindowsInsertPopUp("Error gerber align!");
				}

				if ((head->size % 2) != 0)
					to--;

				double epsilon = 0.1;
				for (int i = pos; i < to; i+= 2) {
					KiVec2 &point = mouse;
					KiVec2 p1 = KiVec2(data[i].x, data[i].y), p2 = KiVec2(data[i + 1].x, data[i + 1].y);

					float dx1 = p2.x - p1.x;
					float dy1 = p2.y - p1.y;

					float dx = point.x - p1.x;
					float dy = point.y - p1.y;

					float S = dx1 * dy - dx * dy1;
					float ab = sqrt(dx1 * dx1 + dy1 * dy1);
					float h = S / ab;

					float d = 1;
					
					if (abs(h) < d / 2) {
						//select = 1;
						//break;
					}

					// Проверяем, что точка лежит на прямой, заданной двумя точками p1 и p2
					if (fabs((p2.y - p1.y) * point.x - (p2.x - p1.x) * point.y + p2.x * p1.y - p2.y * p1.x) < epsilon) {
						// Проверяем, что точка находится внутри отрезка между точками p1 и p2
						if ((point.x + line >= min(p1.x, p2.x) && point.x - line <= max(p1.x, p2.x)) &&
							(point.y + line >= min(p1.y, p2.y) && point.y - line <= max(p1.y, p2.y))) {
							select = 1;
							break;
						}
					}
				}

				// Area
				if(area)
					for (int i = pos; i < to; i += 2) {
						KiVec2 &point = mouse;
						KiVec2 p1 = KiVec2(data[i].x, data[i].y), p2 = KiVec2(data[i + 1].x, data[i + 1].y);

						if (mpos.InRect(p1) || mpos.InRect(p2)) {
							aselect = 1;
							break;
						}
					}
			}
			else if (head->type == GI_GL_MOUSE_AREA) {}
			else
			for (int i = pos; i < to; i++) {
				if (KiVec2(data[i].x - mouse.x, data[i].y - mouse.y).Length() < 2 / zoom)
					select = 1;
			}

			// On area
			if (area) {
				GlslObjectsHeadExt *hext = GlslObjectsBuffer.GetExtByHead(head);
				if (hext->cmd && hext->cmd->aselect != aselect)
					OnMouseMoveActiveArea(head, hext, aselect);
			}

			// Update color
			if (select) {
				OnMouseMoveActive(head, area);

				if(!area)
					return;
			}

			head++;			
		}

		OnMouseMoveActive(0, area);

		return;
	}

	void OnMouseUpdateColor(GlslObjectsHead *head, GlslObjectsHeadExt *hext, bool over) {
		//GlslObjectsHeadExt *hext = GlslObjectsBuffer.GetMouseOverExt();

		int mix = (hext->cmd->selected || hext->cmd->aselect) * 50 + over * 50;

		//SString ss;
		//ss.Format("SetColor %d %d %d \r\n", mix, hext->cmd->selected, hext->cmd->aselect);
		//print(ss);

		GlslObjectsBuffer.SetColor(head, GiColorMix(hext->color, hext->scolor, mix));
	}

	void OnMouseMoveActive(GlslObjectsHead *head, bool area) {
		GlslObjectsHeadExt *hext;

		// No action
		if (GlslObjectsBuffer.GetMouseOver() == head)
			return;
		
		// Un Over
		if (GlslObjectsBuffer.GetMouseOver()) {
			hext = GlslObjectsBuffer.GetMouseOverExt();
			//GlslObjectsBuffer.SetColor(GlslObjectsBuffer.GetMouseOver(), ColorMix(hext->color, hext->scolor, hext->cmd->selected * 50));
			OnMouseUpdateColor(GlslObjectsBuffer.GetMouseOver(), hext, 0);
		}

		//
		if (!head)
			return GlslObjectsBuffer.SetMouseOver(head);

		// Over
		hext = GlslObjectsBuffer.GetExtByHead(head);
		GlslObjectsBuffer.SetMouseOver(head);
		//GlslObjectsBuffer.SetColor(head, hext->cmd->selected == 0 ? hext->acolor : hext->scolor);
		//GlslObjectsBuffer.SetColor(head, ColorMix(hext->color, hext->scolor, hext->cmd->selected * 50 + 50));
		OnMouseUpdateColor(head, hext, GlslObjectsBuffer.GetMouseOver() == head);
	}

	void OnMouseMoveActiveArea(GlslObjectsHead *head, GlslObjectsHeadExt *hext, bool state) {
		hext->cmd->aselect = state;
		//GlslObjectsBuffer.SetColor(head, ColorMix(hext->color, hext->scolor, state * 50));
		OnMouseUpdateColor(head, hext, GlslObjectsBuffer.GetMouseOver() == head);
	}

	void OnMouseClick(bool ctrl_on) {
		GlslObjectsHeadExt *hext;

		if (!ctrl_on)
			SelectAll(0);

		if (hext = GlslObjectsBuffer.GetMouseOverExt()) {
			hext->cmd->selected = !hext->cmd->selected;
			OnMouseUpdateColor(GlslObjectsBuffer.GetMouseOver(), hext, 1);
			//GlslObjectsBuffer.SetColor(GlslObjectsBuffer.GetMouseOver(), hext->cmd->selected ? hext->scolor : hext->acolor);
			//GlslObjectsBuffer.SetColor(GlslObjectsBuffer.GetMouseOver(), ColorMix(hext->color, hext->scolor, hext->cmd->selected * 50 + 50));
		}

		return;
	}

	void OnMouseAreaClick(bool state, bool ctrl_on) {
		GiLayer *el;

		// Objects
		GlslObjectsHead *head;// = GlslObjectsBuffer.GetHead(), *head_end = head + GlslObjectsBuffer.GetHeadCount();
		GlslObjectsHeadExt *hext = GlslObjectsBuffer.GetHeadExt(), *hext_end = hext + GlslObjectsBuffer.GetHeadCount();

		for (hext; hext < hext_end; hext++) {
			head = GlslObjectsBuffer.GetHeadByExt(hext);

			if (hext->cmd) {
				if (state) {
					// Control 
					if(!ctrl_on)
						hext->cmd->selected = hext->cmd->aselect;
					else
						hext->cmd->selected |= hext->cmd->aselect;
					hext->cmd->aselect = 0;
					OnMouseUpdateColor(head, hext, GlslObjectsBuffer.GetMouseOver() == head);
				}
				else {
					//hext->cmd->selected = hext->cmd->aselect;
					hext->cmd->aselect = 0;
					//GlslObjectsBuffer.SetColor(head, ColorMix(hext->color, hext->scolor, hext->cmd->selected * 50));
					OnMouseUpdateColor(head, hext, GlslObjectsBuffer.GetMouseOver() == head);
				}
			}
		}
	}

	void SelectAll(bool val) {
		GlslObjectsHead *head;// = GlslObjectsBuffer.GetHead(), *head_end = head + GlslObjectsBuffer.GetHeadCount();
		GlslObjectsHeadExt *hext = GlslObjectsBuffer.GetHeadExt(), *hext_end = hext + GlslObjectsBuffer.GetHeadCount();

		for (hext; hext < hext_end; hext++) {
			if (hext->cmd)
				if (hext->cmd->selected != val) {
					hext->cmd->selected = val;
					head = GlslObjectsBuffer.GetHeadByExt(hext);
					//GlslObjectsBuffer.SetColor(head, ColorMix(hext->color, hext->scolor, hext->cmd->selected * 50));
					OnMouseUpdateColor(head, hext, GlslObjectsBuffer.GetMouseOver() == head);
				}
		}
	}

	void CreateDrillPath() {
		// Get layer
		GiLayer* lel = GetLayerById(prog_drill_file_id);
		if (!lel)
			return;

		//DrillFile* del = GetDrill(prog_drill_file_id);
		//if (!del)
		//	return;

		// New path
		GiPath* pel = paths.NewE();
		pel->Init(++layer_id);

		// Tool
		pel->SetTool(tool, prog_drill_depth);

		// Do circles
		GiLayerCmdEl* el = 0;

		// Sort
		GiSortVec2 sort;

		// -> Add
		while (el = lel->cmds.cmds.Next(el)) {
			//if (!lel->AppGetEnable(el->app_id))
			if (!el->app->GetEnable())
				continue;
			sort.Add(el->pos, el);
		}

		// ->Sort
		sort.SortMinPath();

		// Sort result
		GiSortVec2El* res = sort.GetData();
		int count = sort.GetSize();

		// Make path
		for (int i = 0; i < count; i++) {
			pel->AddMove(res->pos);
			pel->AddDrill(res->pos, prog_drill_depth);
			res++;
		}

		SetPaint();

		return;
	}

	// ~
	void Clean(){		
		layer_id = 0;
		is_paint = 1;

		name.Clean();
		path.Clean();
		lays.Clean();

		show_grbs = show_drls = show_lays = show_prog = 0;
	}

	~GiProject(){



	}
} GiProject;


bool GiProjectLayerAddAppCircle(int layer_id, int id, float dia){
	GiLayer *el = GiProject.GetLayerById(layer_id);
	if(!el)
		return 0;

	el->AddAppCircle(id, dia);
	return 1;
}

bool GiProjectLayerCmdHole(int layer_id, int app_id, double x, double y){
	GiLayer *el = GiProject.GetLayerById(layer_id);
	if(!el)
		return 0;

	//el->AddCircle(app_id, x, y, dia);
	el->AddCmdHole(app_id, x, y);

	return 1;
}

bool GiProjectLayerCmdObject(int layer_id, int app_id){
	GiLayer *el = GiProject.GetLayerById(layer_id);
	if(!el)
		return 0;

	el->AddCmdObject(app_id);
	return 1;
}

bool GiProjectLayerCmdMove(int layer_id, int app_id, double x, double y, int cmd_d){
	GiLayer *el = GiProject.GetLayerById(layer_id);
	if(!el)
		return 0;

	el->AddCmdMove(app_id, x, y);

	return 1;
}

bool GiProjectLayerCmdRot(int layer_id, int app_id, double x, double y, double cx, double cy, int opt) {
	GiLayer* el = GiProject.GetLayerById(layer_id);
	if (!el)
		return 0;

	el->AddCmdRot(app_id, x, y, cx, cy, opt);
	return 1;
}

bool GiProjectLayerSetColor(int layer_id, KiVec4 color) {
	GiLayer* el = GiProject.GetLayerById(layer_id);
	if (!el)
		return 0;

	el->SetColor(color);
	GiProject.SetPaint();
	return 1;
}

bool GiProjectLayerClean(int layer_id){
	GiLayer *el = GiProject.GetLayerById(layer_id);
	if(!el)
		return 0;

	el->Clean();
	return 1;
}

void GiProjectSelectedTool(GiToolsEl *tool) {
	GiProject.SelectedTool(tool);
}