.PHONY : all

DIST := ../dist/shaders
CP := cp
MKDIR_P := mkdir -p

all : \
	$(DIST) \
	$(DIST)/frag_vertex_colored.glsl \
	$(DIST)/vert_vertex_colored.glsl \
	$(DIST)/vert_text.glsl \
	$(DIST)/frag_text.glsl \
	$(DIST)/vert_texture.glsl \
	$(DIST)/frag_texture.glsl \
	$(DIST)/vert_depth.glsl \
	$(DIST)/frag_depth.glsl

$(DIST):
	$(MKDIR_P) $(DIST)

$(DIST)/%.glsl : %.glsl
	$(CP) -f '$<' '$@'
