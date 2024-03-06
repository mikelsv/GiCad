struct GlslCircleEl{
	float px, py, dia;

	float _none;
};

//struct GlslPath


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

	GLint uni_iTime, uni_iRes, uni_iMove, uni_iZoom, uni_iMouse, uni_iFps, unu_iChan0, uni_GiCircles, uni_GiCirclesCount;
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
		uni_iMouse = GetUniformLocation("iMouse");
		uni_iFps = GetUniformLocation("iFps");

		unu_iChan0 = GetUniformLocation("iChannel0");

		uni_GiCircles = GetUniformLocation("GiCircles");
		//uni_GiCircles = glGetUniformBlockIndex(prog_id, "GiCircles");
		uni_GiCirclesCount = GetUniformLocation("GiCirclesCount");

		MakeCircleBuffer();

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

	//GLuint ssbo;
    
	void MakeCircleBuffer(){
		GlslCircleEl f[2] = {0., 0., 10., 100., 100., 10.};

		glGenBuffers(1, &ssbo_cir);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_cir);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(f), f, GL_STATIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_cir);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
		//return ;

		UseProgram();
		glUniform1ui(uni_GiCirclesCount, 2);
		glUseProgram(0);


		//glGenBuffers(1, &ssbo);
		//glBindBuffer(GL_UNIFORM_BUFFER, ssbo);		

		//glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 2 * 2, f, GL_STATIC_DRAW);
		//glBindBufferBase(GL_UNIFORM_BUFFER, uni_GiCircles, ssbo);
		////glBindBuffer(GL_ARRAY_BUFFER, uni_GiCircles);
		//glBindBuffer(GL_UNIFORM_BUFFER, NULL);
	}

	void UpdateCircleBuffer(VString data, int size){
		UseProgram();

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_cir);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GlslCircleEl) * size, data, GL_STATIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
		
		glUniform1ui(uni_GiCirclesCount, size);
		glUseProgram(0);		
	}

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

	void UpdateMouseSelect(KiInt2 cur, KiInt2 hold, bool down){
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
	KiVec3 color;
	
	void SetColor(MRGB rgb){
		color.x = rgb.red / 255.;
		color.y = rgb.green / 255.;
		color.z = rgb.blue / 255.;
	}

};

class GlslObjectsBuffer{
	MString head, data, color;
	int head_count, data_count;
	int update;

public:
	void Reserve(int hcount, int dcount){
		if(head.size() < sizeof(GlslObjectsHead) * hcount)
			head.Reserve(sizeof(GlslObjectsHead) * hcount);

		if(data.size() < sizeof(GlslObjectsData) * dcount)
			data.Reserve(sizeof(GlslObjectsData) * dcount);

		if(color.size() < sizeof(GlslObjectsColor) * dcount)
			color.Reserve(sizeof(GlslObjectsColor) * dcount);

		head_count = hcount;
		data_count = dcount;
		update = 1;

		return ;
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

	bool GetUpdate(){
		return update;
	}

	void Updated(){
		update = 0;
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

	void Render(GLint tex_id){
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
			   3,                  // size
			   GL_FLOAT,           // type
			   GL_FALSE,           // normalized?
			   0,                  // stride
			   (void*)0            // array buffer offset
			);
			glEnableVertexAttribArray(1);

			// Ok
			GlslObjectsBuffer.Updated();

			// Unbind
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		
		
		// Render
		auto head = GlslObjectsBuffer.GetHead(), head_end = head + GlslObjectsBuffer.GetHeadCount();
		while(head < head_end){
			glDrawArrays(head->type, head->pos, head->size);
			head ++;
		}

		// Draw all
		//glDrawArrays(GL_LINE_LOOP, 0, GlslObjectsBuffer.GetDataCount());
				
		glBindVertexArray(0);		
		glUseProgram(0);
	}	
};

class GlslMenu : public MglSimpleGlsl{


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

