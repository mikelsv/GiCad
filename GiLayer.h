class GiLayerCircle{
public:
	double x, y, dia;
};

class GiLayerPathEl{
public:
	double x, y;

};

class GiLayerPath{
	OList<GiLayerPathEl> path;
};


class GiLayer{
	// Head
	int layer_id;
	MString name;
//	MGRB color;

	// Data
	OList<GiLayerCircle> cls;

public:

	// Get
	int GetId(){
		return layer_id;
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

	void AddPath(double x, double y, double dia){

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
		cls.Clean();
	}

	friend class GiProject;

};