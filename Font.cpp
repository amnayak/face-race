#include "Font.hpp"
#include "Load.hpp"
#include "compile_program.hpp"
#include "load_save_png.hpp"
#include "data_path.hpp"

#include "GL.hpp"
#include "gl_errors.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <iterator>
#include <cstddef>
#include <string>
#include <stdexcept>
#include <map>
#include <iostream>
#include <sstream>

static GLuint uniform_texture;
static GLuint uniform_smoothing;
static GLuint uniform_projTrans;
static GLuint font_program = 0;

Font::Font(std::string src_path, glm::vec2 screen_dim) : screen_dim(screen_dim) {
	// for some reason my completion plugin for sublime text thinks
	// that ifstream is typedefed to int (??) so I'm writing out the
	// full type here
	std::cout << "Loading font " << src_path << std::endl;
	std::basic_ifstream<char> infile(src_path);

	if(!infile.is_open())
		throw std::runtime_error("Font " + src_path + " not found!");

	bool chars_start = false;
	std::string line;
	while(std::getline(infile, line)) {
		std::basic_stringstream<char> line_ss; line_ss << line;
		std::string tag; 
		std::getline(line_ss, tag, ' ');

		if(tag == "")
			continue;

		std::map<std::string,std::string> params;
		std::string param;
		while(std::getline(line_ss, param, ' ')) {
			size_t epos = param.find('=');
			if(epos == std::string::npos)
				continue;

			std::string lhs = param.substr(0, epos);
			std::string rhs = param.substr(epos + 1, std::string::npos);

			if(rhs[0] == '\"')
				rhs = rhs.substr(1, rhs.length() - 2);

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

			std::string file = data_path(dir + '/' + params["file"]);

			// Load the texture & send to gpu
			glm::uvec2 size;
			std::vector< glm::u8vec4 > data;
			load_png(file, &size, &data, LowerLeftOrigin);

			std::cout << "--> Page " << id << " [" << size.x << "x" << size.y << "]" << " @ " << file << std::endl;

			GLuint tex = 0;
			glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
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
			all_chars.resize(std::stoi(params["count"]));
		} else if(tag == "char") {
			if(params["id"] == "" || params["x"] == "" || params["y"] == "" || 
				params["width"] == "" || params["height"] == "" || 
				params["xoffset"] == "" || params["yoffset"] == "" || 
				params["xadvance"] == "" || params["page"] == "") {
				continue;
			}
			
			if(!chars_start) {
				chars_start = true;
				std::cout << "--> Characters: ";
			}	

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

			if(cur->id < 256)
				std::cout << (char)cur->id;

			if(cur->id >= all_chars.size())
				all_chars.resize(cur->id + 1);
			all_chars[cur->id] = cur;

			if(cur->id < 256)
				ascii_chars[cur->id] = cur;
		} else if (tag == "kerning") {
			if(params["first"] == "" || params["second"] == "" || params["amount"] == "") {
				continue;
			}

			unsigned short a, b;
			signed short amnt;

			a = std::stoi(params["first"]);
			b = std::stoi(params["second"]);
			amnt = std::stoi(params["amount"]);

			kerning[{a, b}] = amnt;
		}
	}

	std::cout << std::endl;

	if(!font_program) {
		font_program = compile_program_file("shaders/vert_text.glsl", "shaders/frag_text.glsl");
		uniform_texture = glGetUniformLocation(font_program, "u_texture");
		uniform_smoothing = glGetUniformLocation(font_program, "u_smoothing");
		uniform_projTrans = glGetUniformLocation(font_program, "u_projTrans");
	}

	// Init OpenGL objects
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	GLint lpos = glGetAttribLocation(font_program, "position");
	GLint lcol = glGetAttribLocation(font_program, "color");
	GLint ltex = glGetAttribLocation(font_program, "texcoord0");
	glVertexAttribPointer(lpos, 3, GL_FLOAT, GL_FALSE, sizeof(font_vertex), (GLbyte *) 0 + offsetof(font_vertex, position));
	glVertexAttribPointer(lcol, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(font_vertex), (GLbyte *) 0 + offsetof(font_vertex, color));
	glVertexAttribPointer(ltex, 2, GL_FLOAT, GL_FALSE, sizeof(font_vertex), (GLbyte *) 0 + offsetof(font_vertex, texcoord));
	glEnableVertexAttribArray(lpos);
	glEnableVertexAttribArray(lcol);
	glEnableVertexAttribArray(ltex);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Font::~Font() {
	for(char_data *cd : all_chars)
		delete cd;
}

static bool debugging_print = true;

float Font::draw_ascii_char(unsigned short c, glm::vec2 pos, float size) {
	char_data *cur = all_chars[c];

	if(!cur) {
		// Treat this character as a space
		// Using 0.25em for space size
		return (.25f * (size == 0 ? this->size : size)) / screen_dim.x;
	}

	size = size == 0 ? this->size : size;
	float scale = size / this->size;
	glm::vec2 dim((float)cur->width * scale / screen_dim.x, (float)(cur->height * scale) / screen_dim.y);
	glm::vec2 offset((float)cur->xoffset / screen_dim.x, (render_info.base - cur->height - cur->yoffset) / screen_dim.y);
	offset *= scale;
	pos += offset;

	verts_buf[0].position = glm::vec3(pos.x, pos.y, 0);
	verts_buf[1].position = glm::vec3(pos.x + dim.x, pos.y, 0);
	verts_buf[2].position = glm::vec3(pos.x + dim.x, pos.y + dim.y, 0);
	verts_buf[4].position = glm::vec3(pos.x, pos.y + dim.y, 0);

	glm::vec2 uv(cur->x, cur->y);
	uv.x /= render_info.tex_width;
	uv.y /= render_info.tex_height;
	glm::vec2 uv_dim(cur->width, cur->height);
	uv_dim.x /= render_info.tex_width;
	uv_dim.y /= render_info.tex_height;

	uv.y = 1.f - uv.y;
	uv.y = uv.y - uv_dim.y;

	if(debugging_print) {
		std::cout << ((char)c) << ": "  << "uvs: " << uv.x << ", " << uv.y << " : " << uv_dim.x << ", " << uv_dim.y << std::endl
		  		                     << "   pos: " << pos.x << ", " << pos.y << " : " << dim.x << ", " << dim.y << " : " << cur->xoffset << ", " << cur->yoffset << std::endl;
	}

	verts_buf[0].texcoord = glm::vec2(uv.x, uv.y);
	verts_buf[1].texcoord = glm::vec2(uv.x + uv_dim.x, uv.y);
	verts_buf[2].texcoord = glm::vec2(uv.x + uv_dim.x, uv.y + uv_dim.y);
	verts_buf[4].texcoord = glm::vec2(uv.x, uv.y + uv_dim.y);

	verts_buf[3] = verts_buf[2];
	verts_buf[5] = verts_buf[0];

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(
		GL_ARRAY_BUFFER, 
		6 * sizeof(font_vertex), 
		verts_buf.data(), 
		GL_STREAM_DRAW);

	static glm::mat4 proj = glm::ortho(0, 1, 0, 1);

	glUseProgram(font_program);
	glUniform1f(uniform_smoothing, (float)smooth);
	glUniformMatrix4fv(uniform_projTrans, 1, GL_FALSE, glm::value_ptr(proj));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, page_files[cur->page].gl_texture);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return (float)cur->xadvance * scale / screen_dim.x;
}

void Font::draw_ascii_string(const char *text, glm::vec2 og_pos, float size, float width) {
	glm::vec2 pos = og_pos;
	float pos_in_line = 0;
	char prev = 0;

	size = size == 0 ? this->size : size;
	float scale = size / this->size;

	static bool printed = true;//false;
	debugging_print = false;

	// TODO: Overflow text based on word, not by char
	for(; *text != 0; ++text) {
		char_data *cur = all_chars[*text];
		float adv = 0;
		if(cur) {
			adv = cur->xadvance * scale / screen_dim.x;
		}
		if(*text == '\n' || (width && pos_in_line + adv > width)) {
			pos.x = og_pos.x;
			pos.y -= (float)render_info.line_height * scale / screen_dim.y;
			pos_in_line = 0;
		}
		pos_in_line += adv;

		float real_adv = draw_ascii_char(*text, pos, size);
		pos.x += real_adv;
		if(!cur) {
			pos_in_line += real_adv;
		}

		if(prev)
			pos.x += (float)kerning[{(unsigned short)prev, (unsigned short)(*text)}] * scale / screen_dim.x;
		if(!printed && *text != ' ' && prev)
			std::cout << "   kerning w/ " << prev << ": " << kerning[id_pair(prev, *text)] << std::endl;
		prev = *text;
	}

	printed = true;

	debugging_print = false;
}