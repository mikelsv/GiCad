//#define GIPROJECT_RENDER_GLSL
#define GIPROJECT_RENDER_TRISZ	12

void GiWindowsUpdateTitle();

class GiProject{
	// Project
	MString name, path;

	// Layers & files
	OList<GrblFile> grbs;
	OList<DrillFile> drls;
	OList<GiLayer> lays;
	//OList<GiPath> paths;

	bool show_grbs, show_drls, show_lays, show_prog;

	// Opt
	int layer_id;
	bool is_paint;

public:
	// New
	GiProject(){
		NewProject();

		layer_id = 0;
		is_paint = 0;
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
			cir_count += el->cls.Size();
			
			GiLayerPath *pel = 0;
			while(pel = el->paths.Next(pel)){
				if (!el->GetActive())
					continue;

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
		ImGui::SetWindowFontScale(3.0f);

		// Gerber
		if (ImGui::TreeNodeEx("Gerber", ImGuiTreeNodeFlags_DefaultOpen)) {
			GrblFile* el = 0;
			while (el = grbs.Next(el)) {
				VString path = el->GetFile();
				color = el->GetColor();

				// CheckBox
				isChecked = el->GetActive();
				if (ImGui::Checkbox("##Checkbox", &isChecked)){
					el->SetActive(isChecked);
					SetPaint();
				}

				ImGui::SameLine();
				if (ImGui::ColorEdit4("##Color", &color.x, ImGuiColorEditFlags_NoInputs)) {
					el->SetColor(color);
					GiProjectLayerSetColor(el->GetLayerId(), color);
				}
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
				if (ImGui::Checkbox("##Checkbox", &isChecked)) {
					el->SetActive(isChecked);
					SetPaint();
				}

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