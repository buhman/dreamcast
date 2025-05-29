%.ppm: %.png
	magick -depth 8 $< $@

%.ppm: %.jpg
	magick -depth 8 $< $@

%.vq: %.ppm
	../dreamcast/gen/k_means/k_means_vq $< $@
