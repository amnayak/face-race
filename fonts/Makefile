.PHONY : all

DIST := ../dist/fonts
CP := cp
MKDIR_P := mkdir -p

all : \
	$(DIST) \
	$(DIST)/arial.fnt \
	$(DIST)/arial1.png \
	$(DIST)/arial2.png \
	$(DIST)/times.fnt \
	$(DIST)/times1.png \
	$(DIST)/times2.png \
	$(DIST)/noto_italic.fnt \
	$(DIST)/noto_italic1.png \
	$(DIST)/noto_italic2.png \
	$(DIST)/noto_italic3.png

$(DIST):
	$(MKDIR_P) $(DIST)

$(DIST)/%.fnt : %.fnt
	$(CP) -f '$<' '$@'

$(DIST)/%.png : %.png
	$(CP) -f '$<' '$@'
