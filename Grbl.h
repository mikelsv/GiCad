bool GiProjectLayerAddCircle(int layer_id, double x, double y, double dia);
bool GiProjectLayerClean(int layer_id);


class BaseFile{
private :
	// File
	MString path;
	bool is_open;

	// State
	sstat64 state;

	// Layer
	int layer_id;

public:
	bool Open(VString name){		
		MString data = LoadFile(name);
		path = name;

		state = GetFileInfo(path);

		Clean();

		if(!data){
			Error(LString() + "File is empty: " + name);
			return 0;
		}

		return is_open = Read(data);
	}

	virtual bool Read(VString data){ 
		return 0;
	}

	bool AddLayerCircle(double x, double y, double dia){
		return GiProjectLayerAddCircle(layer_id, x, y, dia);
	}

	bool LayerClean(){
		GiProjectLayerClean(layer_id);
	}

	// Get / Set
	VString GetPath(){
		return path;
	}

	void SetLayerId(int id){
		layer_id = id;
	}

	bool Error(VString err){
		print(err, "\r\n");
		return 0;
	}

	// ~
	virtual void Clean(){
		//layer_id = 0;
	}

	friend class GrblFile;
	friend class DrillFile;
};


enum GrblFileMetric{
	GrblFileMetricUnk = 0,
	GrblFileMetricMM = 1
};

enum GrblFilePolarity{
	GrblFilePolarityUnk = 0,
	GrblFilePolarityDark = 1
};

enum GrblFileApertureType{
	GrblFileApertureUnk = 0,
	GrblFileApertureCircle = 1
};

class GrblFileAperture{
	int id;
	GrblFileApertureType type;

	// Circle
	float rad;

public:
	void SetCircle(int _id, float _rad){
		id = _id;
		type = GrblFileApertureCircle;
		rad = _rad;
	}

	int Id(){
		return id;
	}

};

class GrblFile : public BaseFile{
private:
	// Data
	GrblFileMetric metric;
	GrblFilePolarity polarity;
	float fsla_x, fsla_y;

	// Commands
	int cmd_x, cmd_y, cmd_d, cmd_g, cmd_i, cmd_j, cmd_m;

	// Apertures
	OList<GrblFileAperture> aps;

public:

	bool Read(VString data){
		while(data){
			// read line
			VString line = PartLine(data, data, "\n");

			// Clean \r
			if(line && line.endo() == '\r')
				line.sz --;

			// TF
			if(line.incompareu("%TF.")){
				// Additional data
			}

			// %
			else if(line.incompareu("%")){
				if(line.incompareu("%MOMM*%")){
					metric = GrblFileMetricMM;
				}

				if(line.incompareu("%LPD*%")){
					polarity = GrblFilePolarityDark;
				}

				if(line.incompareu("%FSLA")){ // %FSLAX46Y46*%
					VString res[8];
					int resi = PartLines(line, "%FSLAX$dY$d*%", res);
					if(resi != 2)
						return Error(LString() + "Command bad: " + line);

					fsla_x = res[0].toi();
					fsla_y = res[1].toi();

					//line = PartLine(line, line, "%FSLAX");
					//fsla_x = PartLine(line, line, "Y").tod();
					//line = PartLine(line, line, "Y");
					//fsla_y = PartLine(line, line, "*%").tod();
				}

				if(line.incompareu("%ADD")){ // %ADD10C,0.120000*%
					VString res[8];
					int resi = PartLines(line, "%ADD$d$c,$f*%", res);
					
					if(resi != 3)
						return Error(LString() + "Command bad: " + line);

					if(res[1] == "C")
						AppAddCircle(res[0].toi(), res[2].tod());
					else
						return Error(LString() + "Command bad: " + line);
				}
			}

			// Any
			else{
				if(!ReadLine(line)){
					//Error(LString() + "Code not found: " + line);
					return 0;
				}
			}


		}

		return 1;
	}

	bool ReadLine(VString line){
		VString val;
		unsigned char code, line_code = line[0];
		unsigned char *f, *l = line, *t = line.endu();

		while(l < t){
			code = *l;
			f = ++l;

			while(l < t){
				if(*l >= '0' && *l <= '9' || *l == '.' || *l =='-')
					l ++;
				else 
					break;
			}
			
			val.setu(f, l - f);

			// Code
			switch(code){
			case '*':
				if(line_code == 'X')
					GiProjectLayerAddCircle(layer_id, cmd_x / 1000000., cmd_y / 1000000., 10);
				return 1;

			case 'X':
				cmd_x = val.toi();
				break;

			case 'Y':
				cmd_y = val.toi();
				break;

			case 'D':
				cmd_d = val.toi();
				break;

			case 'I':
				cmd_i = val.toi();
				break;

			case 'J':
				cmd_j = val.toi();
				break;

			case 'M':
				cmd_m = val.toi();
				break;

			case 'G':
				cmd_g = val.toi();

				// Comment
				if(cmd_g == 4)
					return 1;

				break;

			default:
				Error(LString() + "Code not found: " + line);
				return 0;
			}
		}

		return 1;
	}

	VString PartLineFloat(VString line, VString &res){
		unsigned char *f = line, *l = line, *to = line.endu();

		while(l < to){
			if(*l >= '0' && *l <= '9' || *l == '.')
				{}
			else
				break;
		}

		res.setu(l, to - l);
		
		return VString(f, l - f);
	}

	bool AppAddCircle(int id, float rad){
		auto el = AppGetEl(id);
		if(!el)
			el = aps.NewE();
		el->SetCircle(id, rad);

		return 1;
	}

	GrblFileAperture* AppGetEl(int id){
		GrblFileAperture *el = 0;
		while(el = aps.Next(el))
			if(el->Id() == id)
				return el;
		return 0;
	}

	bool Error(VString error){
		
		return 0;
	}

	bool IsModificated(){
		if(is_open){
			sstat64 ss = GetFileInfo(path);
			if(ss.st_mtime != state.st_mtime || ss.st_size != state.st_size)
				return 1;
		}
		return 0;
	}


	void Clean(){
		path.Clean();
		is_open = 0;

		metric = GrblFileMetricUnk;
		polarity = GrblFilePolarityUnk;
		fsla_x = fsla_y = 0;
		cmd_x = cmd_y = 0;
		cmd_d = cmd_g = 0;
		cmd_i = cmd_j = 0;
		cmd_m = 0;

		aps.Clean();
	}

	~GrblFile(){
		Clean();
	}

};

enum DrillFileState{
	DrillFileHead,
	DrillFileData
};

class DrillFileAperture{
	int id;

	// Circle
	float dia;

public:
	void Set(int _id, float _dia){
		id = _id;
		dia = _dia;
	}

	int Id(){
		return id;
	}

	double Dia(){
		return dia;
	}

};


class DrillFile : public BaseFile{
	// State
	DrillFileState state;
	GrblFileMetric metric;
	
	// Apertures
	OList<DrillFileAperture> aps;

	// Cmd
	int cmd_t;
	double cmd_t_dia;

private:
	bool Read(VString data){
		state = DrillFileHead;

		while(data){
			// read line
			VString line = PartLine(data, data, "\n");

			// Clean \r
			if(line && line.endo() == '\r')
				line.sz --;

			if(!ReadLine(line))
				return 0;
		}

		return 1;
	}

	bool ReadLine(VString line){	
			VString key, val;
			unsigned char *f = line, *l = line, *t = line.endu(), k;

			// Ignore comment
			if(line[0] == ';')
				return 1;

			if(line == "%"){
				state = DrillFileData;
				return 1;
			}				

			while(l < t){
				// Read key
				while(l < t && *l >= 'A' && *l <= 'Z')
					l ++;

				key.setu(f, l - f);

				if(key == "METRIC"){
					metric = GrblFileMetricMM;
				} else if(key == "FMAT"){
					return 1;
				} else if(key == "M"){
					return 1;
				} else if(key == "G"){
					return 1;
				} else if(key == "T" && state == DrillFileHead){
					VString res[8];
					int resi = PartLines(line, "T$dC$f", res);
					if(resi != 2)
						return Error(LString() + "Bad code: " + line);
					SetAperture(res[0].toi(), res[1].tod());

					return 1;
				} else if(key == "T" && state == DrillFileData){
					VString res[8];
					int resi = PartLines(line, "T$d", res);
					if(resi != 1)
						return Error(LString() + "Bad code: " + line);

					cmd_t = res[0].toi();
					cmd_t_dia = AppGetDia(cmd_t);

					return 1;
				} else if(key == "X" && state == DrillFileData){
					VString res[8];
					int resi = PartLines(line, "X$fY$f", res);
					if(resi != 2)
						return Error(LString() + "Bad code: " + line);
					SetDrill(cmd_t, res[0].tod(), res[1].tod());

					return 1;
				} else
					return Error(LString() + "Bad code: " + line);
			}

		return 1;
	}

	bool SetAperture(int id, float dia){
		DrillFileAperture *el = AppGetEl(id);
		if(!el)
			el = aps.NewE();
		el->Set(id, dia);

		return 1;
	}

	DrillFileAperture* AppGetEl(int id){
		DrillFileAperture *el = 0;
		while(el = aps.Next(el))
			if(el->Id() == id)
				return el;
		return 0;
	}

	double AppGetDia(int id){
		DrillFileAperture *el = AppGetEl(id);
		if(!el)
			return 0;
		return el->Dia();
	}

	bool SetDrill(int t, double x, double y){
		return AddLayerCircle(x, y, cmd_t_dia);
	}

	void Clean(){		
		aps.Clean();

		BaseFile::Clean();
	}

};

