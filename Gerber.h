enum GiLayerAppType {
	GiLayerAppTypeUnk,
	GiLayerAppTypeCircle
};


bool GiProjectLayerAddAppCircle(int layer_id, int id, float dia);
bool GiProjectLayerAddCircle(int layer_id, double x, double y, double dia);
bool GiProjectLayerAddPath(int layer_id);
bool GiProjectLayerAddPPoi(int layer_id, double x, double y);
bool GiProjectLayerSetColor(int layer_id, KiVec4 color);
bool GiProjectLayerClean(int layer_id);



//template<int size>
//auto gui_check_f(const char(&input)[size]) {
//	return ImGuiChar{ input };
//}

class GiBaseFile{
private :
	// File
	MString path, file;
	bool is_open, is_active;

	// State
	sstat64 state;

	// Layer
	int layer_id;
	KiVec4 color;

	// ImGui
	ImGuiCharId<11> gui_check;
	ImGuiCharId<8> gui_color;

	//inline static ImGuiChar gui_check2 = gui_check_f("##Checkbox");

public:
	GiBaseFile() : gui_check("##Checkbox"), gui_color("##Color") {}

	bool Open(VString name){		
		MString data = LoadFile(name);
		path = name;

		// Get file name
		ILink link;
		link.Link(path, 1);
		file = link.file;

		state = GetFileInfo(path);

		Clean();

		if(!data){
			Error(LString() + "File is empty: " + name);
			return 0;
		}

		GiProjectLayerClean(layer_id);

		return is_open = is_active = Read(data);
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
	int GetLayerId() {
		return layer_id;
	}

	VString GetPath(){
		return path;
	}

	VString GetFile() {
		return file;
	}

	bool GetActive() {
		return is_active;
	}

	KiVec4 GetColor() {
		return color;
	}

	void SetColor(const KiVec4 &c) {
		color = c;
	}

	void SetActive(bool active) {
		is_active = active;
	}

	void SetLayerId(int id){
		layer_id = id;
	}

	// Gui
	void UpdateGui() {
		gui_check.SetId(layer_id);
		gui_color.SetId(layer_id);
	}

	char* GuiNameCheck() {
		return gui_check;
	}

	char* GuiNameColor() {
		return gui_check;
	}

	// Error
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

template<int size>
struct ImGuiChar {
	char str[size];

public:
	constexpr ImGuiChar(const char(&data)[size]) {
		memcpy(str, data, size);
		str[size - 1] = '\0';
	}

	operator char* () {
		return str;
	}
};

template <int size>
ImGuiChar(const char(&data)[size]) -> ImGuiChar<size>;


/*
using namespace std;
#include <iostream>
#include <array>
#include <string>
#include <string_view>
#include <ranges>


template <std::size_t Size>
struct ConstexprString {
	std::array<char, Size> data;
	inline constexpr auto begin() -> char* {
		return this->data.begin();
	};
	inline constexpr auto begin() const -> const char* {
		return this->data.begin();
	};
	inline constexpr auto end() -> char* {
		return this->data.end();
	};
	inline constexpr auto end() const -> const char* {
		return this->data.end();
	};
	static constexpr auto kSize = Size == 0 ? 0 : Size - 1;
	inline constexpr ConstexprString() = default;
	inline constexpr ConstexprString(const char(&data)[Size]) : data{} {
		std::ranges::copy_n(data, Size, this->data.begin());
	};
	inline constexpr ConstexprString(std::string data) : data{} {
		std::ranges::copy_n(data.begin(), Size, this->data.begin());
	};
	inline constexpr ConstexprString(std::array<char, Size> data) : data(std::move(data)) {};
	inline constexpr auto size() const {
		return Size == 0 ? 0 : Size - 1;
	};
	//inline constexpr operator std::string_view() const& {
	//	return { this->begin() };
	//};
//	inline constexpr bool operator==(std::string_view other) const {
//		return static_cast<std::string_view>(*this) == other;
//	};
	template <std::size_t SSize>
	inline constexpr auto operator+(const ConstexprString<SSize>& other) -> ConstexprString<Size + SSize - 1> {
		ConstexprString<Size + SSize - 1> response;
		std::ranges::copy_n(this->begin(), Size - 1, response.begin());
		std::ranges::copy_n(other.begin(), SSize, response.begin() + Size - 1);
		return response;
	};
	inline constexpr ConstexprString(const ConstexprString&) = default;
	inline constexpr ConstexprString(ConstexprString&&) = default;
};


template <std::size_t Count>
inline constexpr auto CreateStringWith(char c) {
	ConstexprString<Count + 1> str = {};
	for (std::size_t i = 0; i < Count; i++) {
		str.data[i] = c;
	};
	str.data[Count] = '\0';
	return str;
};


template <std::size_t Size>
ConstexprString(const char(&data)[Size]) -> ConstexprString<Size>;
*/




class GrblFile : public GiBaseFile {
private:
	// Data
	GrblFileMetric metric;
	GrblFilePolarity polarity;
	float fsla_x, fsla_y;

	// Commands
	int cmd_x, cmd_y, cmd_d, cmd_g, cmd_i, cmd_j, cmd_m;
	int cmd_x2, cmd_y2, cmd_g75;

	// Apertures
	OList<GrblFileAperture> aps;

	// ImGui
	//ImGuiCharId<11> gui_check;
	//ImGuiCharId<11> gui_check;

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

			if(line.incompareu("%TO.")){
				GiProjectLayerAddPath(layer_id);
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
					int ok = 0;

					if(resi == 3)
						if(res[1] == "C"){
							AppAddCircle(res[0].toi(), res[2].tod());
							ok = 1;
						}
					
					if(!ok)
						resi = PartLines(line, "%ADD$d$c,$fX$f*%", res);

					if(resi == 4 && !ok)
						if(res[1] == "O" || res[1] == "R"){
							AppAddCircle(res[0].toi(), res[2].tod());
							ok = 1;
						}

					if(!ok)
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
				if(line_code == 'X'){
					if(cmd_g == 1)
						GiProjectLayerAddPPoi(layer_id, cmd_x / 1000000., cmd_y / 1000000.);
					else if(cmd_g == 02 || cmd_g == 03){
						//cmd_i += 1000000.;
						//cmd_j += 1000000.;

						// G02 to G01
						double sx = cmd_x2 / 1000000., sy = cmd_y2 / 1000000.;
						double ex = cmd_x / 1000000., ey = cmd_y / 1000000.;
						double cx = sx + cmd_i / 1000000., cy = sy + cmd_j / 1000000.;
						double rad = pow(pow(cx - sx, 2) + pow(cy - sy, 2), 0.5);

						// Start / end angles
						double sangle = atan2(sy - cy, sx - cx);
						double eangle = atan2(ey - cy, ex - cx);
						bool stop = 0;
						
						if(cmd_g == 02 && 0)
							if(sangle > eangle){
								double t = sangle;
								sangle = eangle;
								eangle = t;
								stop = 1;
							}
							//if(sangle > eangle)
							//	sangle -= PI;
						//if(cmd_g == 03)
						//	if(eangle < sangle)
						//		eangle += PI;

						if(cmd_g75){
							cmd_g75 = 0;
							sangle += PI;
						}

						int points = int(ceil(abs(eangle - sangle) / (PI / 180)));

						if(stop)
							points = -1;

						for(int i = 0; i < points + 1; i ++){
							double angle = sangle + (eangle - sangle) * i / points;

							double x = cx + rad * cos(angle);
							double y = cy + rad * sin(angle);

							GiProjectLayerAddPPoi(layer_id, x, y);
							GiProjectLayerAddPPoi(layer_id, x, y);
						}

						GiProjectLayerAddPPoi(layer_id, cmd_x / 1000000., cmd_y / 1000000.);
					}

					cmd_x2 = cmd_x;
					cmd_y2 = cmd_y;
				}
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
				// Comment
				if(val.toi() == 4)
					return 1;

				cmd_g = val.toi();

				// Lines
				if (cmd_g == 01){
					//GiProjectLayerAddPath(layer_id);
				}

				if(cmd_g == 75)
					cmd_g75 = 1;

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


class DrillFile : public GiBaseFile {
	// State
	DrillFileState state;
	GrblFileMetric metric;
	
	// Apertures
	OList<DrillFileAperture> aps;

	// Cmd
	int cmd_t;
	double cmd_t_dia;

public:
	DrillFile() {}

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
					GiProjectLayerAddAppCircle(layer_id, res[0].toi(), res[1].tod());

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

		GiBaseFile::Clean();
	}

};

