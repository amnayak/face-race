#include "Font.hpp"
#include "Load.hpp"
#include "compile_program.hpp"
#include "load_save_png.hpp"

#include <fstream>
#include <iterator>
#include <cstddef>
#include <string>
#include <stdexcept>
#include <map>
#include <sstream>

Load< GLuint > font_program(LoadTagDefault, []() {
	return compile_program_file("shaders/vert_text.glsl", "shaders/frag_text.glsl");
});

Font::Font(std::string src_path, glm::vec2 screen_width) : screen_width(screen_width) {
	// for some reason my completion plugin for sublime text thinks
	// that ifstream is typedefed to int (??) so I'm writing out the
	// full type here
	std::basic_ifstream<char> infile(src_path);

	if(!infile.is_open())
		throw std::runtime_error("Font " + src_path + " not found!");

	std::string line;
	while(std::getline(infile, line)) {
		std::basic_stringstream<char> line_ss; line_ss << line;
		std::string tag; 
		std::getline(line_ss, tag, ' ');

		std::map<std::string,std::string> params;
		std::string param;
		while(std::getline(line_ss, param, ' ')) {
			if(tag == "")
				continue;

			size_t epos = param.find('=');
			if(epos == std::string::npos)
				continue;

			std::string lhs = param.substr(epos);
			std::string rhs = param.substr(epos + 1, std::string::npos);

			params[lhs] = rhs;
		}

		if(tag == "info") {
			face = params["face"];
			if(params["size"] != "")
				size = std::stoi(params["size"]);
			bold = params["bold"] == "1";
			italic = params["italic"] == "1";
			charset = params["charset"];
			unicode = params["unicode"] == "1";
			if(params["stretchH"] != "")
				stretchH = std::stof(params["stretchH"]);
			smooth = params["smooth"] == "1";
			if(params["aa"] != "")
				aa = std::stoi(params["aa"]);
			if(params["padding"] != "") {
				std::basic_stringstream<char> padding_ss;
				padding_ss << params["padding"];
				char comma = ' ';
				padding_ss >> padding_up;
				padding_ss >> comma;
				padding_ss >> padding_right;
				padding_ss >> comma;
				padding_ss >> padding_down;
				padding_ss >> comma;
				padding_ss >> padding_left;
			}
			if(params["spacing"] != "") {
				std::basic_stringstream<char> spacing_ss;
				spacing_ss << params["spacing"];
				char comma = ' ';
				spacing_ss >> spacing_hoz;
				spacing_ss >> comma;
				spacing_ss >> spacing_vrt;
			}
			if(params["outline"] != "")
				outline_thickness = std::stoi(params["outline"]);
		} else if(tag == "common") {
			if(params["lineHeight"] != "")
				render_info.line_height = std::stoi(params["lineHeight"]);
			if(params["base"] != "")
				render_info.base = std::stoi(params["base"]);
			if(params["scaleW"] != "")
				render_info.tex_width = std::stoi(params["scaleW"]);
			if(params["scaleH"] != "")
				render_info.tex_height = std::stoi(params["scaleH"]);
			if(params["pages"] != "") {
				render_info.pages = std::stoi(params["pages"]);
				page_files.resize(render_info.pages);
			}
		} else if(tag == "page") {
			if(params["id"] == "" || params["file"] == "")
				continue;
			unsigned short id = std::stoi(params["id"]);

			// hacky way of extracting the current folder
			std::string dir = src_path.substr(0, src_path.find_last_of('/'));

			std::string file = dir + '/' + params["file"];

			// Load the texture & send to gpu
			glm::uvec2 size;
			std::vector< glm::u8vec4 > data;
			load_png(file, &size, &data, LowerLeftOrigin);

			GLuint tex = 0;
			glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
			GL_ERRORS();

			page_files[id] = page_data(file, tex);
		} else if(tag == "chars") {
			if(params["count"] == "")
				continue;
			all_Chars.resize(std::stoi(params["count"]));
		} else if(tag == "char") {
			if(params["id"] == "" || params["x"] == "" || params["y"] == "" || 
				params["width"] == "" || params["height"] == "" || 
				params["xoffset"] == "" || params["yoffset"] == "" || 
				params["xadvance"] == "" || params["page"] == "")
				continue;

			char_data *cur = new char_data;
			cur->id = std::stoi(params["id"]);
			cur->x = std::stoi(params["x"]);
			cur->y = std::stoi(params["y"]);
			cur->width = std::stoi(params["width"]);
			cur->height = std::stoi(params["height"]);
			cur->xoffset = std::stoi(params["xoffset"]);
			cur->yoffset = std::stoi(params["yoffset"]);
			cur->xadvance = std::stoi(params["xadvance"]);
			cur->page = std::stoi(params["page"]);

			all_chars.resize(cur->id);
			all_chars[cur->id] = cur;

			if(cur->id < 256)
				ascii_chars[cur->id] = cur;
		}
	}

	// Init OpenGL objects
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLint lpos = glGetAttribLocation(*font_program, "position");
	GLint lcol = glGetAttribLocation(*font_program, "color");
	GLint ltex = glGetAttribLocation(*font_program, "texcoord0");
	glVertexAttribPointer(lpos, 3, GL_FLOAT, GL_FALSE, sizeof(font_vertex), (GLbyte *) 0 + offsetof(font_vertex, position));
	glVertexAttribPointer(lcol, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(font_vertex), (GLbyte *) 0 + offsetof(font_vertex, color));
	glVertexAttribPointer(ltex, 2, GL_FLOAT, GL_FALSE, sizeof(font_vertex), (GLbyte *) 0 + offsetof(font_vertex, texcoord));
	glEnableVertexAttribArray(lpos);
	glEnableVertexAttribArray(lcol);
	glEnableVertexAttribArray(ltex);

	glBindVertexArray(0);
}

Font::~Font() {
	for(char_data *cd : all_chars)
		delete cd;
}

float Font::draw_ascii_char(unsigned short c, glm::vec2 pos, float size) {
	char_data *cur = all_chars[c];

	float scale = size == 0 ? 1.f : size / this->size;
	glm::vec2 dim((float)cur->xadvance * scale / screen_dim.x, (float)size / screen_dim.y);
	glm::vec2 offset((float)cur->xoffset / screen_dim.x, (float)(this->size - cur->yoffset) / screen_dim.y);
	offset *= scale;
	pos += offset;

	verts_buf[0].position = glm::vec3(pos.x, pos.y, 0);
	verts_buf[1].position = glm::vec3(pos.x + dim.x, pos.y, 0);
	verts_buf[2].position = glm::vec3(pos.x + dim.x, pos.y + dim.y, 0);
	verts_buf[3].position = glm::vec3(pos.x, pos.y + dim.y, 0);

	glm::vec2 uv(cur->x, cur->y);
	uv.x /= render_info.tex_width;
	uv.y /= render_info.tex_height;
	glm::vec2 uv_dim(cur->width, cur->height);
	uv_dim.x /= render_info.tex_width;
	uv_dim.y /= render_info.tex_height;

	verts_buf[0].texcoord = glm::vec2(uv.x, uv.y);
	verts_buf[1].texcoord = glm::vec2(uv.x + uv_dim.x, uv.y);
	verts_buf[2].texcoord = glm::vec2(uv.x + uv_dim.x, uv.y + uv_dim.y);
	verts_buf[3].texcoord = glm::vec2(uv.x, uv.y + uv_dim.y);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(
		GL_ARRAY_BUFFER, 
		4 * sizeof(font_vertex), 
		verts_buf.data(), 
		GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}