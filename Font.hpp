#pragma once

#include <string>
#include <array>
#include <map>
#include <vector>

#include <glm/glm.hpp>

#include "GL.hpp"

/**
 * Implementation of Distance Field font rendering, conforming to the Angel Code font format (.fnt)
 * http://www.angelcode.com/products/bmfont/doc/file_format.html
 * 
 * Much of the documentation on this page is lifted directly from the spec ^^
 */
class Font {
public:
	// Name of the font
	std::string face = "";
	// Name of the OEM charset used (when not unicode)
	std::string charset = "";
	// Size of the true type font
	unsigned short size = 32;
	// The font height stretch in percentage (0.0 - 1.0 range)
	float stretchH = 1.f;

	// If this font is bold
	bool bold = false;
	// If this font is italic
	bool italic = false;
	
	// True if this font is a unicode font
	bool unicode = false;
	
	// True if smoothing was turned on
	bool smooth = false;
	// Supersampling level used (1 => no supersampling)
	unsigned char aa = 1;

	// Padding
	unsigned short padding_up = 0;
	unsigned short padding_right = 0;
	unsigned short padding_down = 0;
	unsigned short padding_left = 0;

	// Spacing
	unsigned short spacing_hoz = 0;
	unsigned short spacing_vrt = 0;

	// Outline thickness
	unsigned short outline_thickness = 0;

	struct {
		// Distance in pixels between each line of text
		unsigned short line_height = 32;
		// The number of pixels from the absolute top of the line to the base of the characters.
		unsigned short base = 32;
		// Width of the texture
		unsigned short tex_width = 512;
		// Height of the texture
		unsigned short tex_height = 512;
		// Number of texture pages
		char pages = 1;

		// TODO: support the "packed" tag which allows you
		// to store outline data

	} render_info;

	struct page_data {
		std::string file_location;
		GLuint gl_texture;

		page_data() : file_location(""), gl_texture(0) {}
		page_data(std::string file_location, GLuint gl_texture) : file_location(file_location), gl_texture(gl_texture) {}
	};

	// Each page in the font, ordered by id
	std::vector<page_data> page_files;

	struct char_data {
		// The character id
		unsigned short id;
		// Left position of the character image in the texture
		unsigned short x;
		// Right position of the character image in the texture
		unsigned short y;
		// Width of character image in the texture
		unsigned short width;
		// Height of the character image in the texture
		unsigned short height;
		// How much the current position should be offset when copying the image from the texture to the screen.
		signed short xoffset;
		// How much the current position should be offset when copying the image from the texture to the screen.
		signed short yoffset;
		// How much the current position should be advanced after drawing the character.
		unsigned short xadvance;
		// Page to use when rendering
		char page;

		// TODO: support the chnl param (assume all channels)
	};
	// All characters in this font
	std::vector<char_data *> all_chars;
	// Shortcut to the char data, assuming ascii ordering
	std::array<char_data *, 256> ascii_chars;

	// Key: pairs of character ids <A, B>
	// Value: How much xadvance should be adjusted when B is rendered after A
	typedef std::pair<unsigned short, unsigned short> id_pair;
	std::map<id_pair, signed short> kerning;

	glm::vec2 screen_dim;

	GLuint vbo;
	GLuint vao;

	// TODO: support the kerning tag which modifies the placement
	//       of characters in certain contexts

	Font(std::string src_path, glm::vec2 screen_dim);
	~Font();

	// Renders the provided character by id, and returns how much screen
	// space (0 - 1 space) should be advanced in x direction.
	// c: id of character to render
	// pos: position of bottom left corner
	// size: font size (defaults to size of glyph)
	float draw_ascii_char(unsigned short c, glm::vec2 pos, float size = 0);
	void draw_ascii_string(const char *text, glm::vec2 pos, float size = 0);

private:
	struct font_vertex {
		glm::vec3 position;
		glm::u8vec4 color = glm::u8vec4(255, 255, 255, 255);
		glm::vec2 texcoord;
	};
	std::array<font_vertex, 6> verts_buf;

};