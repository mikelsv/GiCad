//#define GIPROJECT_RENDER_GLSL
#define GIPROJECT_RENDER_TRISZ	12

void GiWindowsUpdateTitle();
void GiWindowsResetView100p();

enum GiProjectProg {
	GiProjectUnknown, 
	GiProjectProgDrill
};


class GiProject{
	// Project
	MString name, path;

	// Layers & files
	OList<GrblFile> grbs;
	OList<DrillFile> drls;
	OList<GiLayer> lays;
	//OList<GiPath> paths;

	// Show
	bool show_grbs, show_drls, show_lays, show_prog;

	// Opt
	KiVec2 zero_pos;
	int layer_id;
	bool is_paint, is_first;

	// Prog
	GiProjectProg prog_type;
	int prog_drill_file_id;
	bool prog_drill_check;
	char* prog_drill_file_name;

public:
	// New
	GiProject(){
		NewProject();

		layer_id = 0;
		is_paint = 0;
		is_first = 1;
		prog_type = GiProjectUnknown;
		prog_drill_file_name = 0;
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

		// Count
		GiLayer *el = 0;
		while(el = lays.Next(el)){
			if (!el->GetActive())
				continue;

			cir_count += el->cls.Size();
			
			GiLayerPath *pel = 0;
			while(pel = el->paths.Next(pel)){
				path_count++;
				path_size += pel->path.Size();				
			}
		}		

		GlslObjectsBuffer.Reserve(cir_count + path_count, cir_count * GIPROJECT_RENDER_TRISZ + path_size);

		GlslObjectsHead *head = GlslObjectsBuffer.GetHead();
		GlslObjectsData *data = GlslObjectsBuffer.GetData();
		GlslObjectsColor *color = GlslObjectsBuffer.GetColors();

		// Make circles
		while(el = lays.Next(el)){
			GiLayerCircle *cel = 0;

			if (!el->GetActive())
				continue;

			// Circles
			while(cel = el->cls.Next(cel)){
				head->pos = size;
				head->type = GL_POLYGON;
				head->size = GIPROJECT_RENDER_TRISZ;

				for(int i = 0; i < GIPROJECT_RENDER_TRISZ; i++){
					float angle = i * 2 * PI / GIPROJECT_RENDER_TRISZ;
					data->x = cel->x + (cos(angle) * cel->dia / 2);
					data->y = cel->y + (sin(angle) * cel->dia / 2);
					data->z = 0;
					data ++;

					color->SetColor(el->GetColor());
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

		//paint_cir_count = size;

#ifdef GIPROJECT_RENDER_GLSL
		glsl.UpdateCircleBuffer(paint_cir, paint_cir_count);
#endif

		//is_paint = 0;		

		return 1;
	}

	// Layer
	int NewLayer(GiBaseFile *file){
		GiLayer *el = lays.NewE();
		el->Clean();

		el->SetId(++layer_id);		
		el->SetFile(file);		

		return layer_id;
	}

	GiLayer* GetLayerById(int id){
		GiLayer *el = 0;
		while(el = lays.Next(el)){
			if(el->GetId() == id)
				return el;
		}

		return 0;
	}

	KiVec4 GetLayersRect(){
		KiVec4 rect, rc;

		GiLayer *el = 0;
		while(el = lays.Next(el)){
			rc = el->GetLayerRect();

			if(!rc.IsNull())
				if(!rect.IsNull())
					rect.UnionRect(rc);
				else
					rect = rc;
		}

		return rect;
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
		GrblFile *el = GetGrbByPath(file);
		if(!el){
			el = grbs.NewE();
			el->SetLayerId(NewLayer(el));
			el->UpdateGui();
		}

		show_grbs = 1;

		SetPaint();
		return el->Open(file);
	}

	GrblFile* GetGrbByPath(VString file){
		GrblFile *el = 0;
		while(el = grbs.Next(el)){
			if(file == el->GetPath())
				return el;
		}

		return 0;
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

	DrillFile* GetDrillByPath(VString file){
		DrillFile *el = 0;
		while(el = drls.Next(el)){
			if(file == el->GetPath())
				return el;
		}

		return 0;
	}

	// Render
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
			}

			ImGui::TreePop();
		}

		// Drill
		if (ImGui::TreeNodeEx("Drill", ImGuiTreeNodeFlags_DefaultOpen)) {
			DrillFile* el = 0;
			while (el = drls.Next(el)) {
				VString path = el->GetFile();

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
			}

			ImGui::TreePop();
		}		

		// Layers
		if (ImGui::TreeNode("Layers")) {
			GiLayer* el = 0;
			while (el = lays.Next(el)) {
				//VString path = el->GetPath();
				//ImGui::TextUnformatted(path, path.end());
			}

			ImGui::TreePop();
		}		

		// Program
		if (ImGui::TreeNode("Program")) {
			DrillFile* el = 0;
			while (el = drls.Next(el)) {
				//VString path = el->GetPath();
				//ImGui::TextUnformatted(path, path.end());
			}

			ImGui::TreePop();
		}

		ImGui::End();
	}

	void GuiProg() {
		if (prog_type == GiProjectProgDrill) {

			ImGui::Begin("Drill", nullptr, 0);
			ImGui::SetWindowFontScale(GIGUI_GLOBAL_SCALE);
			ImGui::SameLine();

			ImGui::Text("Depth:");
			ImGui::SameLine();

			float floatValue = 1.0f;
			if (ImGui::InputFloat("##Float Value", &floatValue)) {
				// Действия при изменении значения
			}

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

			// Table
			if (ImGui::BeginTable("Table", 3)) {
				// Head
				ImGui::TableNextColumn();
				if (ImGui::Checkbox("##Checkbox", &prog_drill_check)) {
						GetLayerById(prog_drill_file_id)->AppSetCheckAll(prog_drill_check);
				}

				ImGui::TableNextColumn();
				ImGui::Text("Size");

				ImGui::TableNextColumn();
				ImGui::Text("Tool");

				// Data
				auto el = GetLayerById(prog_drill_file_id);
				GiLayerAppEl *app = 0;

				if(el)
				while (app = el->NextApp(app)) {
					ImGui::TableNextColumn();
					if (ImGui::Checkbox(app->c_check, &app->checked)) {
						prog_drill_check = GetLayerById(prog_drill_file_id)->AppGetCheckAll();
					}
					ImGui::SameLine();
					ImGui::Text(app->c_type);

					ImGui::TableNextColumn();
					ImGui::Text(dtos(app->dia));

					ImGui::TableNextColumn();
					ImGui::Text("Tool");
				}

				ImGui::EndTable();
			}

			ImGui::End();
		}
	}

//	void Render(KiVec2 move, KiVec2 scale, float zoom){
//#ifndef GIPROJECT_RENDER_GLSL
//		//RenderLayers(move, scale, zoom);
//#endif
//	}


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

bool GiProjectLayerAddCircle(int layer_id, double x, double y, double dia){
	GiLayer *el = GiProject.GetLayerById(layer_id);
	if(!el)
		return 0;

	el->AddCircle(x, y, dia);
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