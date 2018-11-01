#include "UIElement.hpp"
#include "Load.hpp"
#include "data_path.hpp"
#include "compile_program.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

static GLuint uniform_texture;
static GLuint uniform_smoothing;
static GLuint uniform_projTrans;
static GLuint font_program = 0;


UIBox::UIBox(glm::vec2 p, glm::vec2 s, glm::u8vec4 c){
    pos = p;
    size = s;
    color = c;

    if(!font_program) {
        font_program = compile_program_file("shaders/vert_text.glsl", "shaders/frag_text.glsl");
        uniform_texture = glGetUniformLocation(font_program, "u_texture");
        uniform_smoothing = glGetUniformLocation(font_program, "u_smoothing");
        uniform_projTrans = glGetUniformLocation(font_program, "u_projTrans");
    }
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLint lpos = glGetAttribLocation(font_program, "position");
    GLint lcol = glGetAttribLocation(font_program, "color");
    GLint ltex = glGetAttribLocation(font_program, "texcoord0");
    glVertexAttribPointer(lpos, 3, GL_FLOAT, GL_FALSE, sizeof(button_vertex), (GLbyte *) 0 + offsetof(button_vertex, position));
    glVertexAttribPointer(lcol, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(button_vertex), (GLbyte *) 0 + offsetof(button_vertex, color));
    glVertexAttribPointer(ltex, 2, GL_FLOAT, GL_FALSE, sizeof(button_vertex), (GLbyte *) 0 + offsetof(button_vertex, texcoord));
    glEnableVertexAttribArray(lpos);
    glEnableVertexAttribArray(lcol);
    glEnableVertexAttribArray(ltex);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void UIBox::draw(glm::uvec2 const& drawable_size) {
    glm::vec2 dim((float) size.x , (float)size.y);

    verts_buf[0].position = glm::vec3(pos.x, pos.y, 0);
    verts_buf[1].position = glm::vec3(pos.x + dim.x, pos.y, 0);
    verts_buf[2].position = glm::vec3(pos.x + dim.x, pos.y + dim.y, 0);
    verts_buf[4].position = glm::vec3(pos.x, pos.y + dim.y, 0);

    verts_buf[3] = verts_buf[2];
    verts_buf[5] = verts_buf[0];

    glm::vec2 uv(1.0f, 1.0f);

    verts_buf[0].texcoord = uv;
    verts_buf[1].texcoord = uv;
    verts_buf[2].texcoord = uv;
    verts_buf[4].texcoord = uv;
    verts_buf[3] = verts_buf[2];
    verts_buf[5] = verts_buf[0];
    if (!active) {
        verts_buf[0].color = color;
        verts_buf[1].color = color;
        verts_buf[2].color = color;
        verts_buf[3].color = color;
        verts_buf[4].color = color;
        verts_buf[5].color = color;
    }
    else {
        glm::u8vec4 white(color.r, color.g, color.b, color.a);
        verts_buf[0].color = white;
        verts_buf[1].color = white;
        verts_buf[2].color = white;
        verts_buf[3].color = white;
        verts_buf[4].color = white;
        verts_buf[5].color = white;
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER, 
        6 * sizeof(button_vertex), 
        verts_buf.data(), 
        GL_STREAM_DRAW);

    static glm::mat4 proj = glm::ortho(0, 1, 0, 1);

    glUseProgram(font_program);
    glUniform1f(uniform_smoothing, (float)smooth);
    glUniformMatrix4fv(uniform_projTrans, 1, GL_FALSE, glm::value_ptr(proj));
    glActiveTexture(GL_TEXTURE0);
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glm::u8vec4 white(color.r, color.g, color.b, color.a);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, glm::value_ptr(white));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);


    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void UIBox::on_mouse_move(glm::vec2 mouse) {
    float new_x, new_y;
    new_x = std::max(0.0f, mouse.x - size.x/2.0f);
    pos.x = std::min(1.0f, new_x);

    new_y = std::max(0.0f, mouse.y - size.y/2.0f);
    pos.y = std::min(1.0f, new_y);

}