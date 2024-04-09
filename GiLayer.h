//class GiLayerCircle{
//public:
//	int app_id;
//	double x, y, dia; 
//};

/*
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
*/

// AppType
// Circle: pos(d.x, d.y), dia(d.z);
// Rectabgle: pos1(d.x, d.y), pos2(d.z, d.w)

class GiLayerAppEl {
public:
	GiLayerAppType type;
	int id;
	float dia;	

	KiVec4 d;

	bool checked, selected;

	// Gui
	ImGuiCharId<2> c_type; // T01
	ImGuiCharId<11> c_check; // ##Checkbox
	ImGuiCharIdExt<11> c_dia; //

public:
	GiLayerAppEl() : c_type("T"), c_check("##Checkbox") {
		checked = 1;
		selected = 0;
	}

	void OnUpdate() {
		c_type.SetId2(id);
		c_check.SetId(id);
		c_dia.SetFloat(dia);
	}

	bool GetEnable() {
		return checked;
	}

};

// Gi Layer Commands
enum GiLayerCmdEn {
	GiLayerCmdUnknown = 0,
	// Types
	GiLayerCmdObject = 1,
	GiLayerCmdHole = 2,	
	// Actions
	GiLayerCmdMove = 10, // Line Move
	GiLayerCmdCir = 11, // Cirlce center
	GiLayerCmdRot = 12, // Rotation Move (G2, G3)
	GiLayerCmdDrill = 13 // Z Move	
};

class GiLayerCmdEl {
public:
	GiLayerCmdEn type;
	GiLayerAppEl* app;
	KiVec2 pos;
	int opt;

	KiVec4 GetRect() {
		if (type == GiLayerCmdHole)
			return KiVec4(pos.x - app->dia / 2, pos.y - app->dia / 2, pos.x + app->dia / 2, pos.y + app->dia / 2);
		else if (type == GiLayerCmdMove || type == GiLayerCmdRot)
			return KiVec4(pos.x, pos.y, pos.x, pos.y);
		return KiVec4();
	}
};

class GiLayerCmd {
public:
	OList<GiLayerCmdEl> cmds;

public:
	GiLayerCmd(){}

	// Add
	void AddObject(GiLayerAppEl *app) {
		auto el = cmds.NewE();
		el->type = GiLayerCmdObject;
		el->app = app;
	}

	void AddCmdHole(GiLayerAppEl *app, double x, double y) {
		auto el = cmds.NewE();
		el->type = GiLayerCmdHole;
		el->app = app;
		el->pos.x = x;
		el->pos.y = y;
	}

	void AddCmdMove(GiLayerAppEl *app, double x, double y) {
		auto el = cmds.NewE();
		el->type = GiLayerCmdMove;
		el->app = app;
		el->pos.x = x;
		el->pos.y = y;
	}

	void AddCmdCir(GiLayerAppEl *app, double x, double y) {
		auto el = cmds.NewE();
		el->type = GiLayerCmdCir;
		el->app = app;
		el->pos.x = x;
		el->pos.y = y;
	}

	void AddCmdRot(GiLayerAppEl *app, double x, double y, double cx, double cy, int opt) {
		AddCmdCir(app, cx, cy);

		auto el = cmds.NewE();
		el->type = GiLayerCmdRot;
		el->app = app;
		el->pos.x = x;
		el->pos.y = y;
		el->opt = opt;
	}

	// Get
	KiVec4 GetRect() {
		KiVec4 rect;
		bool first = 1;

		// Cmds
		GiLayerCmdEl *cmd = 0;
		while (cmd = cmds.Next(cmd)) {
			if (cmd->type == GiLayerCmdMove || cmd->type == GiLayerCmdRot || cmd->type == GiLayerCmdHole) {
				KiVec4 rc = cmd->GetRect();

				if (first) {
					rect = rc;
					first = 0;
				}

				rect = GetUnion(rect, rc);
			}
		}

		return rect;
	}

	KiVec4 GetUnion(KiVec4 one, KiVec4 two) {
		return KiVec4(
			min(one.x, two.x),
			min(one.y, two.y),
			max(one.z, two.z),
			max(one.w, two.w)
		);
	}

	// Paint
	void Paint(KiVec4 col) {
		GiLayerCmdEl *cel = 0, *cel_pos = 0, *cel_cir = 0;

		// Objects
		GlslObjectsHead head;
		GlslObjectsData data;
		GlslObjectsColor color;

		// Circles
		while (cel = cmds.Next(cel)) {
			// Object
			if (cel->type == GiLayerCmdObject) {
				head.type = GL_LINES;
				GlslObjectsBuffer.AddHead(head);
			}

			// Move
			if (cel->type == GiLayerCmdMove) {
				data.x = cel->pos.x;
				data.y = cel->pos.y;
				color.SetColor(col);
				cel_pos = cel;

				GlslObjectsBuffer.AddData(data, color);
			}

			// Circle
			if (cel->type == GiLayerCmdCir) {
				cel_cir = cel;
			}

			// Rotate
			if (cel->type == GiLayerCmdRot) {
				PaintRot(col, cel_pos, cel_cir, cel);
			}

			// Hole
			if (cel->type == GiLayerCmdHole) {
				head.type = GL_POLYGON;
				GlslObjectsBuffer.AddHead(head);

				//KiVec4 col = el->GetColor();
				if (cel->app && cel->app->selected) {
					float c = col.x;
					col.x = col.z;
					col.z = c;
				}

				for (int i = 0; i < GI_LAYER_PAINT_TRISZ; i++) {
					float angle = i * 2 * PI / GI_LAYER_PAINT_TRISZ;
					data.x = cel->pos.x + (cos(angle) * cel->app->dia / 2);
					data.y = cel->pos.y + (sin(angle) * cel->app->dia / 2);
					data.z = 0;

					color.SetColor(col);
					GlslObjectsBuffer.AddData(data, color);
				}
			}
		}
	}

	bool PaintRot(KiVec4 col, GiLayerCmdEl *cel_pos, GiLayerCmdEl *cel_cir, GiLayerCmdEl *cel) {
		if (!cel_pos || !cel_cir)
			return 0;

		GlslObjectsData data;
		GlslObjectsColor color;

		color.SetColor(col);

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
	}

	

	void Clean() {
		cmds.Clean();
	}

};

class GiLayer{
	// Head
	int layer_id;
	MString name;
	KiVec4 color;

	// Data
	OList<GiLayerAppEl> apps;
	GiLayerCmd cmds;
	//OList<GiLayerCmd> cmds;
	// 
//	OList<GiLayerCircle> cls;
	//OList<GiLayerPath> paths;

	// Links
	GiBaseFile* file;
	//GiLayerPath *last_path;

public:
	GiLayer(){
		file = 0;
		//last_path = 0;
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

	// Cmd
	void AddCmdObject(int app_id) {
		cmds.AddObject(GetAppById(app_id));
	}

	void AddCmdHole(int app_id, double x, double y) {
		cmds.AddCmdHole(GetAppById(app_id), x, y);
	}

	void AddCmdMove(int app_id, double x, double y) {
		cmds.AddCmdMove(GetAppById(app_id), x, y);
	}

	void AddCmdCir(int app_id, double x, double y) {
		cmds.AddCmdCir(GetAppById(app_id), x, y);
	}

	void AddCmdRot(int app_id, double x, double y, double cx, double cy, int opt) {
		//cmds.AddCmdCir(GetAppById(app_id), cx, cy);
		cmds.AddCmdRot(GetAppById(app_id), x, y, cx, cy, opt);
	}

	// App
	GiLayerAppEl* GetAppById(int id) {
		GiLayerAppEl* el = 0;

		while (el = apps.Next(el)) {
			if (el->id == id)
				return el;
		}

		return 0;
	}

	void AddAppCircle(int id, float dia) {
		auto el = apps.NewE();
		el->type = GiLayerAppTypeCircle;
		el->id = id;
		el->dia = dia;

		el->OnUpdate();
	}

	//void AddCircle(int app_id, double x, double y, double dia){
	//	GiLayerCircle *el = cls.NewE();
	//	el->app_id = app_id;
	//	el->x = x;
	//	el->y = y;
	//	el->dia = dia;
	//}

	//void AddPath(){
	//	//if(!last_path)
	//	last_path = paths.NewE();
	//	//last_path->AddPoint(x, y);
	//}

	//void AddPoi(double x, double y, double dia){
	//	if(!last_path)
	//		last_path = paths.NewE();

	//	last_path->AddPoint(x, y);
	//}

	//int SetCirData(GlslCircleEl *cel, int count){
	//	GiLayerCircle *el = 0;
	//	int size = 0;

	//	while(el = cls.Next(el)){
	//		if(count <= 0)
	//			break;

	//		cel->px = el->x;
	//		cel->py = el->y;
	//		cel->dia = el->dia;

	//		cel ++;
	//		size ++;
	//	}

	//	return size;
	//}

	// App
	bool AppGetEnable(int id) {
		GiLayerAppEl* el = 0;

		while (el = apps.Next(el)) {
			if(el->id == id)
				return el->checked;
		}

		return 0;
	}

	bool AppGetSelected(int id) {
		GiLayerAppEl* el = 0;

		while (el = apps.Next(el)) {
			if (el->id == id)
				return el->selected;
		}

		return 0;
	}

	bool AppGetCheckAll() {
		if (!this)
			return 0;

		GiLayerAppEl* el = 0;
		bool check = 0;

		while (el = apps.Next(el)) {
			check |= el->checked;
		}

		return check;
	}

	void AppSetCheckAll(bool state) {
		if (!this)
			return;

		GiLayerAppEl *el = 0;
		while (el = apps.Next(el)) {
			el->checked = state;
		}
	}

	void AppSetSelectAll(bool state) {
		if (!this)
			return;

		GiLayerAppEl *el = 0;
		while (el = apps.Next(el)) {
			el->selected = state;
		}
	}

	KiVec4 GetUnion(KiVec4 one, KiVec4 two) {
		return KiVec4(
			min(one.x, two.x),
			min(one.y, two.y),
			max(one.z, two.z),
			max(one.w, two.w)
		);
	}

	/*
	KiVec4 GetLayerRect(){
		KiVec4 rect;
		bool first = 1;

		// Cmds
		GiLayerCmd* cmd = 0;
		while (cmd = cmds.Next(cmd)) {
			if (cmd->type == GiLayerCmdMove || cmd->type == GiLayerCmdRot || cmd->type == GiLayerCmdHole) {
				KiVec4 rc = cmd->GetRect();

				if (first) {
					rect = rc;
					first = 0;
				}

				rect = GetUnion(rect, rc);
			}
		}

		// Circles
		//GiLayerCircle *el = 0;
		//while(el = cls.Next(el)){
		//	if(first){
		//		rect = KiVec4(el->x - el->dia / 2, el->y - el->dia / 2, el->x + el->dia / 2, el->y + el->dia / 2);
		//		first = 0;
		//	}

		//	rect.x = min(rect.x, el->x - el->dia / 2);
		//	rect.y = min(rect.y, el->y - el->dia / 2);
		//	rect.z = max(rect.z, el->x + el->dia / 2);
		//	rect.w = max(rect.w, el->y + el->dia / 2);
		//}

		// Paths
		//GiLayerPath *pel = 0;
		//GiLayerPathEl *pel2 = 0;
		//while(pel = paths.Next(pel)){
		//	while(pel2 = pel->path.Next(pel2)){
		//		if(first){
		//			rect = KiVec4(pel2->x, pel2->y, pel2->x, pel2->y);
		//			first = 0;
		//		}

		//		rect.x = min(rect.x, pel2->x);
		//		rect.y = min(rect.y, pel2->y);
		//		rect.z = max(rect.z, pel2->x);
		//		rect.w = max(rect.w, pel2->y);
		//	}
		//}

		return rect;
	}*/

	auto NextApp(auto el) {
		return apps.Next(el);
	}

	// ~
	void Clean(){
		apps.Clean();
		cmds.Clean();
	}

	friend class GiProject;

};