// Font
class GlslFontTexture{
	GLuint texture;

public:
	void Init(){
		MString data = LoadFile("glsl/font.bin");

		if(!data)
			return ;

		GLuint t = 0;
		//glEnable(GL_TEXTURE_2D);
		glGenTextures( 1, &t );
		glBindTexture(GL_TEXTURE_2D, t);

		// Set parameters to determine how the texture is resized
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		// Set parameters to determine how the texture wraps at edges
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T, GL_REPEAT );
		// Read the texture data from file and upload it to the GPU
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data.data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		texture = t;
	}

	GLuint GetTexture(){
		return texture;
	}


} GlslFontTexture;

class GlslMain : public MglSimpleGlsl{
	GLuint vbo, vao, vertex_id, color_id;

	GLint uni_iTime, uni_iRes, uni_iMove, uni_iZoom, uni_iZero, uni_iMouse, uni_iFps, unu_iChan0, uni_GiCircles, uni_GiCirclesCount;
	GLuint font_id;

public:
	bool Init(KiVec2 res){
		if(!Load(LoadFile("glsl/gicad.vs"), LoadFile("glsl/gicad.fs")))
			return 0;

		InitVbo();

		// Uniforms
		uni_iTime = GetUniformLocation("iTime");
		uni_iRes = GetUniformLocation("iResolution");
		uni_iMove = GetUniformLocation("iMove");
		uni_iZoom = GetUniformLocation("iZoom");
		uni_iZero = GetUniformLocation("iZero");
		uni_iMouse = GetUniformLocation("iMouse");
		uni_iFps = GetUniformLocation("iFps");

		unu_iChan0 = GetUniformLocation("iChannel0");

		uni_GiCircles = GetUniformLocation("GiCircles");
		//uni_GiCircles = glGetUniformBlockIndex(prog_id, "GiCircles");
		uni_GiCirclesCount = GetUniformLocation("GiCirclesCount");

		//MakeCircleBuffer();

		UpdateRes(res);
		UpdateZoom(1);

		return 1;
	}

	int InitVbo(){
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		return 1;
	}

	GLuint ssbo_cir;

	void UpdateRes(KiVec2 res){
		UseProgram();
		glUniform2f(uni_iRes, res.x, res.y);
		glUseProgram(0);
	}

	void SetFont(GLuint f){
		font_id = f;
	}

	void UpdateMove(KiVec2 p){
		UseProgram();
		glUniform2f(uni_iMove, p.x, p.y);
		glUseProgram(0);
	}

	void UpdateZoom(float zoom){
		UseProgram();
		glUniform1f(uni_iZoom, zoom);
		glUseProgram(0);
	}

	void UpdateZero(KiVec2 zero){
		UseProgram();
		glUniform2f(uni_iZero, zero.x, zero.y);
		glUseProgram(0);
	}

	void UpdateMouseSelect(KiVec2 cur, KiVec2 hold, bool down){
		UseProgram();
		
		if(down)
			glUniform4f(uni_iMouse, cur.x, cur.y, hold.x, hold.y);
		else
			glUniform4f(uni_iMouse, cur.x, cur.y, 0, 0);
		
		glUseProgram(0);
	}

	void UpdateFps(int fps){
		UseProgram();
		glUniform1i(uni_iFps, fps);
		glUseProgram(0);
	}

	void Render(float iTime){
		//GLint tex_id = 0;

	   // activate corresponding render state	
		UseProgram();
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(vao);

		glUniform1fv(uni_iTime, 1, &iTime);

		float xpos = -1, ypos = -1;
		float w = 2, h = 2;
		
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, font_id);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
		//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}
};

class GlslObjectsHead{
public:
	int pos, size;
	GLuint type; // GL_LINES or GL_POLYGON
	MRGB color;
};

class GlslObjectsData{
public:
	float x, y, z;	
};

class GlslObjectsColor{
public:
	KiVec4 color;
	
	void SetColor(KiVec4 col){
		color = col;
	}

};

class GlslObjectsBuffer{
	MString head, data, color;
	int head_count, data_count;
	int head_size, data_size;
	GlslObjectsHead* last_head;
	int update;

public:
	void AddHead(const GlslObjectsHead& h) {

		// Reserve
		if (head_count >= head_size)
			ReserveHead(head_size + S1K);

		// Pointers
		GlslObjectsHead* el = GetHead() + head_count;
		
		//Set
		*el = h;

		// Init pos & size
		el->pos = data_count;
		el->size = 0;

		// Last head
		last_head = el;

		head_count++;
	}

	void AddData(const GlslObjectsData& d, const GlslObjectsColor& c) {
		// Reserve
		if (data_count >= data_size)
			ReserveData(data_size + S1K);

		// Pointers
		GlslObjectsData* el = GetData() + data_count;
		GlslObjectsColor* cel = GetColors() + data_count;

		// Set
		*el = d;
		*cel = c;

		// Update head size
		last_head->size++;

		data_count++;
	}

	// Reserve
	void ReserveHead(int size) {
		head.Reserve(sizeof(GlslObjectsHead) * size);
		head_size = size;
	}

	void ReserveData(int size) {
		data.Reserve(sizeof(GlslObjectsData) * size);
		color.Reserve(sizeof(GlslObjectsColor) * size);
		data_size = size;
	}

	GlslObjectsHead* GetHead(){
		return (GlslObjectsHead*) head.data;
	}

	GlslObjectsData* GetData(){
		return (GlslObjectsData*) data.data;
	}

	GlslObjectsColor* GetColors(){
		return (GlslObjectsColor*) color.data;
	}

	int GetHeadCount(){
		return head_count;
	}

	int GetDataCount(){
		return data_count;
	}

	// Update
	bool GetUpdate(){
		return update;
	}

	void SetUpdate(bool val){
		update = val;
	}

	// Clean
	void Clean() {
		head_count = 0;
		data_count = 0;
	}

} GlslObjectsBuffer;

class GlslObjects : public MglSimpleGlsl{
	// Buffers
	GLuint vertex_array, vertex_pos, vertex_color;
	
	GLuint vertex_id, color_id;

	// Uniform
	GLuint uni_iMove, uni_iRes, uni_iZoom;

	// Render texture.
	// vec4 from, vec4 to;

struct vertex{
 GLfloat x;
 GLfloat y;
};

public:
	int Init(){
		if(!Load(LoadFile("glsl/objects.vs"), LoadFile("glsl/objects.fs")))
			return 0;

		color_id = GetUniformLocation("textColor");	
		uni_iMove = GetUniformLocation("iMove");
		uni_iRes = GetUniformLocation("iResolution");
		uni_iZoom = GetUniformLocation("iZoom");

		InitVbo();

		return 1;
	}

	int InitVbo(){
		// Array
		glGenVertexArrays(1, &vertex_array);
		glBindVertexArray(vertex_array);

		// Position
		glGenBuffers(1, &vertex_pos);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_pos);
		glEnableVertexAttribArray(0);
		
		// Colors
		glGenBuffers(1, &vertex_color);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_color);		
		
		//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
		
		//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		return 1;
	}

	void UpdateMove(KiVec2 p){
		UseProgram();
		glUniform2f(uni_iMove, p.x, p.y);
		glUseProgram(0);
	}

	void UpdateRes(KiVec2 res){
		UseProgram();
		glUniform2f(uni_iRes, res.x, res.y);
		glUseProgram(0);
	}

	void UpdateZoom(float zoom){
		UseProgram();
		glUniform1f(uni_iZoom, zoom);
		glUseProgram(0);
	}

	void Render(float time){
	   // activate corresponding render state	
		UseProgram();
		//glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(vertex_array);

		float xpos = -1, ypos = -1;
		float w = 2, h = 2;
		
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		static const GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   0.0f,  1.0f, 0.0f,
   .5, .5, 0.
};

		// render glyph texture over quad
		//glBindTexture(GL_TEXTURE_2D, tex_id);
		// update content of VBO memory
		
		
		if(GlslObjectsBuffer.GetUpdate()){
			// Positions buffer
			glBindBuffer(GL_ARRAY_BUFFER, vertex_pos);
			glBufferData(GL_ARRAY_BUFFER, GlslObjectsBuffer.GetDataCount() * sizeof(GlslObjectsData), GlslObjectsBuffer.GetData(), GL_STATIC_DRAW);
			

		//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
		//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glVertexAttribPointer(
			   0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			   3,                  // size
			   GL_FLOAT,           // type
			   GL_FALSE,           // normalized?
			   0,                  // stride
			   (void*)0            // array buffer offset
			);

			// Color buffer
			glBindBuffer(GL_ARRAY_BUFFER, vertex_color);
			glBufferData(GL_ARRAY_BUFFER, GlslObjectsBuffer.GetDataCount() * sizeof(GlslObjectsColor), GlslObjectsBuffer.GetColors(), GL_STATIC_DRAW);

			glVertexAttribPointer(
			   1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			   4,                  // size
			   GL_FLOAT,           // type
			   GL_FALSE,           // normalized?
			   0,                  // stride
			   (void*)0            // array buffer offset
			);
			glEnableVertexAttribArray(1);

			// Ok
			GlslObjectsBuffer.SetUpdate(0);

			// Unbind
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		
		
		// Render
		auto head = GlslObjectsBuffer.GetHead(), head_end = head + GlslObjectsBuffer.GetHeadCount();
		while(head < head_end){
			int type;

			//if (head->type == GL_LINE_STRIP) {
			//	glEnable(GL_LINE_STIPPLE);
			//	
			//	static int pattern = 0xAAAA;
			//	const int bits = 6; // of 16
			//	//pattern = (pattern >> 1) | ((pattern & 1) << 15);
			//	//pattern = (int(time * 5.) % 2) == 0 ? 0xAAAA : 0x5555;
			//	pattern = ((1 << bits) - 1) << (int(time * 12.) % 16);
			//	pattern += pattern >> 16;
			//	
			//	glLineStipple(1, pattern);
			//}

			// Draw 
			glDrawArrays(head->type, head->pos, head->size);

			if (head->type == GL_LINE_STRIP) {
				glDisable(GL_LINE_STIPPLE);
			}

			head ++;
		}

		// Draw all
		//glDrawArrays(GL_LINE_LOOP, 0, GlslObjectsBuffer.GetDataCount());
				
		glBindVertexArray(0);		
		glUseProgram(0);
	}	
};

void OpenGLDrawCircle(GLfloat x, GLfloat y, GLfloat radius, MRGB rgb, int a=1, int lines=0){
	float angle;
	if(!lines) lines=radius+10;
	glColor3f(rgb.red/float(255), rgb.green/float(255), rgb.blue/float(255));

	glBegin(!a ? GL_LINE_LOOP : GL_POLYGON);
	for(int i = 0; i < lines; i++){
		angle = i * 2 * PI / lines;
		glVertex2f(x + (cos(angle) * radius), y + (sin(angle) * radius));
	}

	glEnd();
	return ;
}

/*
https://stackoverflow.com/questions/47297295/actually-using-gl-line-width-when-supported-by-driver
float vals[2];
glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, vals);
*/