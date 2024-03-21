#define GIGUI_GLOBAL_SCALE 2.9

enum GiCadZeroPoint {
	GiCadZeroPointNull,
	GiCadZeroPointLeftUp,
	GiCadZeroPointLeftDown,
	GiCadZeroPointRightUp,
	GiCadZeroPointRightDown
};

// ImGui CharId
template<int size>
class ImGuiCharId {
	char str[size + 4];

public:
	ImGuiCharId(const char(&input)[size]) {
		memcpy(str, input, size);
		str[size] = '\0';
	}

	void SetId(int id) {
		str[size - 1] = 48 + (id / 1000) % 10;
		str[size + 0] = 48 + (id / 100) % 10;
		str[size + 1] = 48 + (id / 10) % 10;
		str[size + 2] = 48 + (id / 1) % 10;
		str[size + 3] = 0;
	}

	void SetId2(int id) {
		str[size - 1] = 48 + (id / 10) % 10;
		str[size + 0] = 48 + (id / 1) % 10;
		str[size + 1] = 0;
	}

	operator char* () {
		return str;
	}
};

template <int size>
ImGuiCharId(const char(&data)[size]) -> ImGuiCharId<size>;