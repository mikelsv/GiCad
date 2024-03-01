#define GIPROJECT_RENDER_GLSL
#define GIPROJECT_RENDER_TRISZ	12


class GiProject{
	// Project
	MString name, path;

	// Layers & files
	OList<GrblFile> grbs;
	OList<DrillFile> drls;
	OList<GiLayer> lays;
	//OList<GiPath> paths;

	// Opt
	int layer_id;
	bool is_paint;

	// Data
	MString paint_cir;
	int paint_cir_count;

public:
	// New
	GiProject(){
		layer_id = 0;
		is_paint = 0;
		paint_cir_count = 0;
	}

	bool New(){
		Clean();

		name = "Undefined.gic";
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
	}

	bool DoPaintLayers(GlslMain &glsl){
		int count = 0, size = 0;

		// Count
		GiLayer *el = 0;
		while(el = lays.Next(el)){
			count += el->cls.Size();
		}

	//	if(paint_cir.size() < count * sizeof(GlslCircleEl))
//			paint_cir.Reserve(count * sizeof(GlslCircleEl));

		// Collect
		//GlslCircleEl *cel = (GlslCircleEl*)paint_cir.data;

		GlslObjectsBuffer.Reserve(count, count * GIPROJECT_RENDER_TRISZ);

		GlslObjectsHead *head = GlslObjectsBuffer.GetHead();
		GlslObjectsData *data = GlslObjectsBuffer.GetData();

		// Make circles
		while(el = lays.Next(el)){
			GiLayerCircle *cel = 0;

			while(cel = el->cls.Next(cel)){
				head->pos = size;
				head->size = GIPROJECT_RENDER_TRISZ;

				for(int i = 0; i < GIPROJECT_RENDER_TRISZ; i++){
					float angle = i * 2 * PI / GIPROJECT_RENDER_TRISZ;
					data->x = cel->x + (cos(angle) * cel->dia / 2);
					data->y = cel->y + (sin(angle) * cel->dia / 2);
					data->z = 0;
					data ++;
				}

				head ++;
				size += GIPROJECT_RENDER_TRISZ;
			}
		}

		paint_cir_count = size;

#ifdef GIPROJECT_RENDER_GLSL
		glsl.UpdateCircleBuffer(paint_cir, paint_cir_count);
#endif

		is_paint = 0;		

		return 1;
	}

	// Layer
	int NewLayer(){
		GiLayer *el = lays.NewE();
		el->Clean();

		el->SetId(++layer_id);		

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

	void RenderLayers(KiInt2 move, KiVec2 scale, float zoom){
		GlslCircleEl *cel = (GlslCircleEl*)paint_cir.data, *tel = cel + paint_cir_count;

		if(!cel)
			return ;

		while(cel < tel){
			OpenGLDrawCircle((cel->px - move.x) / scale.x, (cel->py - move.y) / scale.y, cel->dia / zoom * .001, 111);
			cel ++;
		}
	}

	// Gerber
	bool AddGbrFile(VString file){
		GrblFile *el = GetGrbByPath(file);
		if(!el){
			el = grbs.NewE();
			el->SetLayerId(NewLayer());
		}

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
			el->SetLayerId(NewLayer());
		}

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
	void Render(KiInt2 move, KiVec2 scale, float zoom){
#ifndef GIPROJECT_RENDER_GLSL
		RenderLayers(move, scale, zoom);
#endif
	}


	// ~
	void Clean(){
		is_paint = 1;
		layer_id = 0;

		name.Clean();
		path.Clean();
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

bool GiProjectLayerAddPath(int layer_id, double x, double y, double dia){
	GiLayer *el = GiProject.GetLayerById(layer_id);
	if(!el)
		return 0;

	el->AddPath(x, y, dia);
	return 1;
}

bool GiProjectLayerClean(int layer_id){
	GiLayer *el = GiProject.GetLayerById(layer_id);
	if(!el)
		return 0;

	el->Clean();
	return 1;
}