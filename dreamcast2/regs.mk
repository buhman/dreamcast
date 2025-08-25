%.csv: %.ods
	libreoffice --headless --convert-to csv:"Text - txt - csv (StarCalc)":44,34,76,,,,true --outdir $(dir $@) $<


# HOLLY

holly/holly.hpp: regs/holly/holly.csv regs/render_block_regs.py
	python regs/render_block_regs.py $< holly > $@

holly/holly_bits.hpp: regs/holly/holly_bits.csv regs/render_bits.py
	python regs/render_bits.py $< holly > $@

holly/region_array_bits.hpp: regs/holly/region_array_bits.csv regs/render_bits.py
	python regs/render_bits.py $< holly region_array > $@

holly/object_list_bits.hpp:regs/holly/object_list_bits.csv regs/render_bits.py
	python regs/render_bits.py $< holly object_list > $@

# SH7091

sh7091/sh7091.hpp: regs/sh7091/sh7091.csv regs/sh7091.py
	python regs/sh7091.py $< sh7091 > $@

sh7091/sh7091_bits.hpp: regs/sh7091/sh7091_bits.csv regs/render_bits.py
	python regs/render_bits.py $< sh7091 > $@

# SYSTEMBUS

systembus/systembus.hpp: regs/systembus/systembus.csv regs/render_block_regs.py
	python regs/render_block_regs.py $< systembus > $@

systembus/systembus_bits.hpp: regs/systembus/systembus_bits.csv regs/render_bits.py
	python regs/render_bits.py $< systembus > $@
