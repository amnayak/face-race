#include "UIElement.hpp"
#include "Load.hpp"
#include "data_path.hpp"
#include "compile_program.hpp"
#include "gl_errors.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <functional>

UIElement *UIElement::ui_element_focused = nullptr;

Load < UIProgram > ui_program(LoadTagDefault, []() {
    UIProgram *ret = new UIProgram;
    ret->program = compile_program_file("shaders/vert_uielement.glsl", "shaders/frag_uielement.glsl");
    
    ret->u_projTrans = glGetUniformLocation(ret->program, "u_projTrans");
    ret->u_color     = glGetUniformLocation(ret->program, "u_color");

    ret->in_position = glGetAttribLocation(ret->program, "position");
    ret->in_texcoord0 = glGetAttribLocation(ret->program, "texcoord0");
    
    return ret;
});

Load < GLuint > white_texture(LoadTagDefault, []() {
    GLuint *tex = new GLuint;
    glGenTextures(1, tex);

    static glm::u8vec4 white(255, 255, 255, 170);

    glBindTexture(GL_TEXTURE_2D, *tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, glm::value_ptr(white));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    GL_ERRORS();

    return tex;
});


UIBox::UIBox(glm::vec2 pos, glm::vec2 size, glm::vec4 color) : UIElement(pos, size), color(color) {
    tex = *white_texture;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GL_ERRORS();
    
    glVertexAttribPointer(ui_program->in_position , 2, GL_FLOAT, GL_FALSE, sizeof(button_vertex), (GLbyte *) 0 + offsetof(button_vertex, position));
    glVertexAttribPointer(ui_program->in_texcoord0, 2, GL_FLOAT, GL_FALSE, sizeof(button_vertex), (GLbyte *) 0 + offsetof(button_vertex, texcoord));
    glEnableVertexAttribArray(ui_program->in_position );
    glEnableVertexAttribArray(ui_program->in_texcoord0);

    GL_ERRORS();

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Only needs to be set once
    verts_buf[0].texcoord = glm::vec2(0.f,0.f);
    verts_buf[1].texcoord = glm::vec2(1.f,0.f);
    verts_buf[2].texcoord = glm::vec2(1.f,1.f);
    verts_buf[4].texcoord = glm::vec2(0.f,1.f);

    verts_buf[3] = verts_buf[2];
    verts_buf[5] = verts_buf[0];
}

void UIBox::draw(glm::uvec2 const& window_size, glm::mat4 const& view) {
    glm::vec2 dim((float) size.x , (float)size.y);
    if(dim.x == 0) {
        pos.x = window_size.x / 2;
        dim.x = window_size.x;
    }
    if(dim.y == 0) {
        pos.y = window_size.y / 2;
        dim.y = window_size.y;
    }

    GL_ERRORS();

    verts_buf[0].position = glm::vec2(pos.x - dim.x/2, pos.y - dim.y/2);
    verts_buf[1].position = glm::vec2(pos.x + dim.x/2, pos.y - dim.y/2);
    verts_buf[2].position = glm::vec2(pos.x + dim.x/2, pos.y + dim.y/2);
    verts_buf[4].position = glm::vec2(pos.x - dim.x/2, pos.y + dim.y/2);

    verts_buf[3].position = verts_buf[2].position;
    verts_buf[5].position = verts_buf[0].position;

    GL_ERRORS();

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER, 
        6 * sizeof(button_vertex), 
        verts_buf.data(), 
        GL_STREAM_DRAW);

    GL_ERRORS();

    glm::mat4 proj = glm::ortho(0.f, (float)window_size.x, 0.f, (float)window_size.y) * view;
    GL_ERRORS();

    glUseProgram(ui_program->program);
    glUniform4f(ui_program->u_color, color.x, color.y, color.z, color.w);
    glUniformMatrix4fv(ui_program->u_projTrans, 1, GL_FALSE, glm::value_ptr(proj));
    
    GL_ERRORS();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    GL_ERRORS();

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    GL_ERRORS();
}

void UIElement::handle_event(SDL_Event const &e, glm::uvec2 const &window_size) {
    glm::vec2 br = pos - size / 2.f;

    auto inside = [&br,this](int x, int y){
        return x >= br.x && x <= br.x+size.x && y >= br.y && y <= br.y+size.y;
    };
    auto btn_int = [](int btn) {
        switch(btn) {
            case SDL_BUTTON_LEFT: return 0;
            case SDL_BUTTON_RIGHT: return 1;
            case SDL_BUTTON_MIDDLE: return 2;
            default: return 3;
        }
    };

    if(prev_window_size != window_size) {
        if(prev_window_size != glm::uvec2(0,0) && onResize) 
            onResize(prev_window_size, window_size);
        prev_window_size = window_size;
    }
    
    switch(e.type) {
        case SDL_MOUSEMOTION:
        if(onHover)
            onHover(
                glm::vec2(e.motion.x,e.motion.y), 
                inside(e.motion.x, e.motion.y));
        break;
        case SDL_MOUSEBUTTONDOWN:
        if(onMouseDown)
            onMouseDown(
                glm::vec2(e.button.x,e.button.y),
                inside(e.button.x,e.button.y),
                btn_int(e.button.button));
        break;
        case SDL_MOUSEBUTTONUP:
        if(onMouseUp)
            onMouseUp(
                glm::vec2(e.button.x,e.button.y),
                inside(e.button.x,e.button.y),
                btn_int(e.button.button));
        break;
        case SDL_WINDOWEVENT_RESIZED: 
        if(onResize) 
            onResize(prev_window_size, window_size);
        default:break;

    }
}

void UIGroupElement::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    SDL_Event e = evt;

    switch(e.type) {
        case SDL_MOUSEMOTION:
        e.motion.x -= pos.x - size.x/2;
        e.motion.y -= pos.y - size.y/2; 
        break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        e.button.x -= pos.x - size.x/2;
        e.button.y -= pos.y - size.y/2;
        break;
        default:break;
    }

    for(UIElement *cur : children)
        cur->handle_event(e, window_size);

    UIElement::handle_event(evt, window_size);
}

void UIGroupElement::draw(glm::uvec2 const& window_size, glm::mat4 const& view) {
    glm::mat4 viewnew = 
            glm::translate(glm::mat4(1), glm::vec3(pos.x - size.x/2, pos.y - size.y/2, 0))
          * view;
    for(UIElement *cur : children)
        cur->draw(window_size, viewnew);
}

void UIGroupElement::update(float elapsed) {
    UIElement::update(elapsed);

    for(UIElement *cur : children)
        cur->update(elapsed);
}

UIElement *UIElement::create_slider(
    glm::vec2 center,
    float width,
    float bar_thickness,
    float btn_thickness,
    std::function<void(float)> value_changed,
    std::function<float(void)> retrieve_value,
    float initial,
    HozAnchor hoz,
    VrtAnchor vrt) {

    std::vector<UIElement *> chld;
    UIBox *bak = new UIBox(
        glm::vec2(width/2,btn_thickness/2),
        glm::vec2(width,bar_thickness),
        glm::vec4(.2f,.2f,.2f,.7f));
    UIBox *fnt = new UIBox(
        glm::vec2(initial * width,btn_thickness/2),
        glm::vec2(btn_thickness,btn_thickness), 
        glm::vec4(1,1,1,.7f));

    fnt->onMouseDown = [fnt](glm::vec2 const& m, bool i, int b) {
        if(i) ui_element_focused = (UIElement *)fnt;
    };

    fnt->onMouseUp = [fnt](glm::vec2 const& m, bool i, int b) {
        if((UIElement *)fnt == ui_element_focused) 
            ui_element_focused = nullptr;
    };

    fnt->onHover = [fnt,value_changed,width](glm::vec2 const& m, bool i) {
        if((UIElement *)fnt == ui_element_focused) {
            fnt->pos.x = std::max(0.f,std::min(width,m.x));
            if(value_changed && width)
                value_changed(fnt->pos.x / width);
        }
    };

    fnt->onUpdate = [fnt,retrieve_value,width](float elapsed) {
        if((UIElement *)fnt != ui_element_focused) {
            if(retrieve_value && width)
                fnt->pos.x = width * retrieve_value();
        }
    };

    chld.push_back(bak);
    chld.push_back(fnt);

    UIElement *ret = (UIElement *)(new UIGroupElement(chld, center, glm::vec2(width, btn_thickness)));
    auto prev = ret->onResize;
    ret->onResize = [hoz, vrt, ret, prev](glm::vec2 const& old, glm::vec2 const& nxt) {
        if(old.x > 0.01 || old.y > 0.01 || nxt.x > 0.01 || nxt.y > 0.01) {
            switch(hoz) {
                case Left:
                break;
                case Right:
                ret->pos.x += nxt.x - old.x;
                break;
                case Center:
                ret->pos.x += (nxt.x - old.x)/2.f;
                break;
            }
            switch(vrt) {
                case Bottom:
                break;
                case Top:
                ret->pos.y += nxt.y - old.y;
                break;
                case Middle:
                ret->pos.y += (nxt.y - old.y)/2.f;
                break;
            }
        }
        
        if(prev)
            prev(old, nxt);
    };

    return ret;
}