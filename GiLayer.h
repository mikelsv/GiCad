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
	MRGB color;

	// Data
	OList<GiLayerCircle> cls;
	OList<GiLayerPath> paths;

	GiLayerPath *last_path;

public:
	GiLayer(){
		last_path = 0;
		color.set(rand(), rand(), rand());
	}

	// Get
	int GetId(){
		return layer_id;
	}

	MRGB GetColor(){
		return color;
	}

	// Set
	void SetId(int id){
		layer_id = id;
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

	// ~
	void Clean(){
		last_path = 0;

		cls.Clean();
	}

	friend class GiProject;

};