.PHONY : all

DIST := ../dist/shaders
CP := cp
MKDIR_P := mkdir -p

all : \
	$(DIST) \
	$(DIST)/frag_vertex_colored.glsl \
	$(DIST)/vert_vertex_colored.glsl

$(DIST):
	$(MKDIR_P) $(DIST)

$(DIST)/%.glsl : %.glsl
	$(CP) -f '$<' '$@'
