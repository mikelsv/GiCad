class GiLayerCircle{
public:
	double x, y, dia;
};

class GiLayerPathEl{
public:
	double x, y;

};

class GiLayerPath{
public:
	OList<GiLayerPathEl> path;

	void AddPoint(double x, double y){
		GiLayerPathEl *el = path.NewE();
		el->x = x;
		el->y = y;
	}
};

class GiLayer{
	// Head
	int layer_id;
	MString name;
	KiVec4 color;

	// Data
	OList<GiLayerCircle> cls;
	OList<GiLayerPath> paths;

	// Links
	GiBaseFile* file;
	GiLayerPath *last_path;

public:
	GiLayer(){
		file = 0;
		last_path = 0;
		color.RandomOne();
	}

	// Get
	int GetId(){
		return layer_id;
	}

	KiVec4 GetColor(){
		return color;
	}

	bool GetActive() {
		return file && file->GetActive();
	}

	// Set
	void SetId(int id) {
		layer_id = id;
	}

	void SetColor(KiVec4 col) {
		color = col;
	}

	void SetFile(GiBaseFile* f) {
		file = f;
		file->SetColor(color);
	}

	// Add
	void AddCircle(double x, double y, double dia){
		GiLayerCircle *el = cls.NewE();
		el->x = x;
		el->y = y;
		el->dia = dia;
	}

	void AddPath(){
		//if(!last_path)
		last_path = paths.NewE();
		//last_path->AddPoint(x, y);
	}

	void AddPoi(double x, double y, double dia){
		if(!last_path)
			last_path = paths.NewE();

		last_path->AddPoint(x, y);
	}

	int SetCirData(GlslCircleEl *cel, int count){
		GiLayerCircle *el = 0;
		int size = 0;

		while(el = cls.Next(el)){
			if(count <= 0)
				break;

			cel->px = el->x;
			cel->py = el->y;
			cel->dia = el->dia;

			cel ++;
			size ++;
		}

		return size;
	}

	KiVec4 GetLayerRect(){
		KiVec4 rect;
		bool first = 1;

		// Circles
		GiLayerCircle *el = 0;
		while(el = cls.Next(el)){
			if(first){
				rect = KiVec4(el->x - el->dia / 2, el->y - el->dia / 2, el->x + el->dia / 2, el->y + el->dia / 2);
				first = 0;
			}

			rect.x = min(rect.x, el->x - el->dia / 2);
			rect.y = min(rect.y, el->y - el->dia / 2);
			rect.z = max(rect.z, el->x + el->dia / 2);
			rect.w = max(rect.w, el->y + el->dia / 2);
		}

		// Paths
		GiLayerPath *pel = 0;
		GiLayerPathEl *pel2 = 0;
		while(pel = paths.Next(pel)){
			while(pel2 = pel->path.Next(pel2)){
				if(first){
					rect = KiVec4(pel2->x, pel2->y, pel2->x, pel2->y);
					first = 0;
				}

				rect.x = min(rect.x, pel2->x);
				rect.y = min(rect.y, pel2->y);
				rect.z = max(rect.z, pel2->x);
				rect.w = max(rect.w, pel2->y);
			}
		}

		return rect;
	}

	// ~
	void Clean(){
		last_path = 0;

		cls.Clean();
	}

	friend class GiProject;

};