drawings = \
	overview.png log_overview.png modules.png log_modules.png \
	nns_simple4.png nns_wifi_only.png nns_wifi_detail.png 1_4.png


.SUFFIXES: .svg .pdf
.SUFFIXES: .svg .png
.SUFFIXES: .m   .svg

.m.svg:
	octave -q $<

.svg.pdf:
	inkscape --without-gui -D --export-pdf=$@ $<

.svg.png:
	inkscape --without-gui -D --export-png=$@ $<

all: $(drawings)

clean:
	rm -f *.pdf *.png

