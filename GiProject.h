//#define GIPROJECT_RENDER_GLSL
#define GIPROJECT_RENDER_TRISZ	16

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

	// Do
	/*
	bool DoPaintLayers2(GlslMain &glsl){
		int count = 0, size = 0;

		// Count
		GiLayer *el = 0;
		while(el = lays.Next(el)){
			count += el->cls.Size();	
		}

		if(paint_cir.size() < count * sizeof(GlslCircleEl))
			paint_cir.Reserve(count * sizeof(GlslCircleEl));

		// Collect
		GlslCircleEl *cel = (GlslCircleEl*)paint_cir.data;

		while(el = lays.Next(el)){
			size += el->SetCirData(cel + size, count - size);
		}

		paint_cir_count = size;

#ifdef GIPROJECT_RENDER_GLSL
		glsl.UpdateCircleBuffer(paint_cir, paint_cir_count);
#endif

		is_paint = 0;		

		return 1;
	}*/

	bool DoPaintLayers(GlslMain &glsl){
		int cir_count = 0, path_count = 0, path_size = 0, size = 0;

		// Count layers
		GiLayer *el = 0;
		while(el = lays.Next(el)){
			if (!el->GetActive())
				continue;

			// Circles
			cir_count += el->cls.Size();
			
			// Paths
			GiLayerPath *pel = 0;
			while(pel = el->paths.Next(pel)){
				path_count++;
				path_size += pel->path.Size();				
			}
		}

		// Count paths
		GiPath* pel = 0;
		while (pel = paths.Next(pel)) {
			if (!pel->GetActive())
				continue;

			path_count ++;
			path_size += pel->PaintGetCount();
		}

		// Make buffer
		GlslObjectsBuffer.Reserve(cir_count + path_count, cir_count * GIPROJECT_RENDER_TRISZ + path_size);

		GlslObjectsHead *head = GlslObjectsBuffer.GetHead();
		GlslObjectsData *data = GlslObjectsBuffer.GetData();
		GlslObjectsColor *color = GlslObjectsBuffer.GetColors();

		// Write data
		while(el = lays.Next(el)){
			GiLayerCircle *cel = 0;

			if (!el->GetActive())
				continue;

			// Circles
			while(cel = el->cls.Next(cel)){
				head->pos = size;
				head->type = GL_POLYGON;
				head->size = GIPROJECT_RENDER_TRISZ;
				KiVec4 col = el->GetColor();
				if (el->AppGetSelected(cel->app_id)) {
					float c = col.x;
					col.x = col.z;
					col.z = c;
				}

				for(int i = 0; i < GIPROJECT_RENDER_TRISZ; i++){
					float angle = i * 2 * PI / GIPROJECT_RENDER_TRISZ;
					data->x = cel->x + (cos(angle) * cel->dia / 2);
					data->y = cel->y + (sin(angle) * cel->dia / 2);
					data->z = 0;
					data ++;

					color->SetColor(col);
					color ++;
				}

				head ++;
				size += GIPROJECT_RENDER_TRISZ;
			}

			// Paths
			GiLayerPath *pel = 0;
			GiLayerPathEl *pel2 = 0;

			while(pel = el->paths.Next(pel)){
				head->pos = size;
				head->size = pel->path.Size();
				head->type = GL_LINES;
				head ++;
				
				while(pel2 = pel->path.Next(pel2)){
					data->x = pel2->x;
					data->y = pel2->y;

					data ++;
					size ++;

					color->SetColor(el->GetColor());
					color ++;
				}		
			}
		}

		while (pel = paths.Next(pel)) {
			if (!pel->GetActive())
				continue;

			head->pos = size;
			head->size = pel->PaintGetCount();
			head->type = GL_LINE_STRIP;
			head++;

			int s = pel->PaintGetData(data, color);
			data += s;
			color += s;

			//path_count++;
			//path_size += pel->PaintGetCount();
		}

		//paint_cir_count = size;

#ifdef GIPROJECT_RENDER_GLSL
		glsl.UpdateCircleBuffer(paint_cir, paint_cir_count);
#endif

		// Test
		if (head > GlslObjectsBuffer.GetHead() + cir_count + path_count) {
			print("Stack overflow! DoPaintLayers();\r\n");
		}

		is_paint = 0;		

		return 1;
	}

	/*
	void RenderLayers(KiVec2 move, KiVec2 scale, float zoom){
		GlslCircleEl *cel = (GlslCircleEl*)paint_cir.data, *tel = cel + paint_cir_count;

		if(!cel)
			return ;

		while(cel < tel){
			OpenGLDrawCircle((cel->px - move.x) / scale.x, (cel->py - move.y) / scale.y, cel->dia / zoom * .001, 111);
			cel ++;
		}
	}*/

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
	int NewLayer(GiBaseFile* file) {
		GiLayer* el = lays.NewE();
		el->Clean();

		el->SetId(++layer_id);
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
			rc = el->GetLayerRect();

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
		ImGui::Begin("Layers", nullptr, ImGuiWindowFlags_NoMove);
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
		if (prog_type == GiProjectProgDrill) {

			ImGui::Begin("Drill", nullptr, 0);
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

			if (ImGui::Button("Close")) {
				prog_type = GiProjectProgUnknown;
			}

			ImGui::End();
		}
	}

//	void Render(KiVec2 move, KiVec2 scale, float zoom){
//#ifndef GIPROJECT_RENDER_GLSL
//		//RenderLayers(move, scale, zoom);
//#endif
//	}

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
		GiLayerCircle* el = 0;

		// Sort
		GiSortVec2 sort;
		int count = 0;

		// -> Count
		while (el = lel->cls.Next(el)) {
			if (!lel->AppGetEnable(el->app_id))
				continue;
			count++;
		}

		sort.Init(count);

		// -> Add
		while (el = lel->cls.Next(el)) {
			if (!lel->AppGetEnable(el->app_id))
				continue;
			sort.Add(KiVec2(el->x, el->y), el);
		}

		// ->Sort
		sort.SortMinPath();

		// Sort result
		GiSortVec2El* res = sort.GetData();

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

bool GiProjectLayerAddCircle(int layer_id, int app_id, double x, double y, double dia){
	GiLayer *el = GiProject.GetLayerById(layer_id);
	if(!el)
		return 0;

	el->AddCircle(app_id, x, y, dia);
	return 1;
}

bool GiProjectLayerAddPath(int layer_id){
	GiLayer *el = GiProject.GetLayerById(layer_id);
	if(!el)
		return 0;

	el->AddPath();
	return 1;
}

bool GiProjectLayerAddPPoi(int layer_id, double x, double y){
	GiLayer *el = GiProject.GetLayerById(layer_id);
	if(!el)
		return 0;

	el->AddPoi(x, y, 0);
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