%.csv: %.ods
	libreoffice --headless --convert-to csv:"Text - txt - csv (StarCalc)":44,34,76,,,,true --outdir $(dir $@) $<


# HOLLY

holly/holly.hpp: regs/holly/holly.csv regs/render_block_regs.py
	python regs/render_block_regs.py $< holly > $@

holly/holly_bits.hpp: regs/holly/holly_bits.csv regs/render_bits.py
	python regs/render_bits.py $< holly > $@

holly/core/region_array_bits.hpp: regs/holly/core/region_array_bits.csv regs/render_bits.py
	python regs/render_bits.py $< holly::core::region_array > $@

holly/core/object_list_bits.hpp: regs/holly/core/object_list_bits.csv regs/render_bits.py
	python regs/render_bits.py $< holly::core::object_list > $@

holly/core/parameter_bits.hpp: regs/holly/core/parameter_bits.csv regs/render_bits.py
	python regs/render_bits.py $< holly::core::parameter > $@

holly/ta/parameter_bits.hpp: regs/holly/ta/parameter_bits.csv regs/render_bits.py
	python regs/render_bits.py $< holly::ta::parameter > $@

holly/ta/global_parameter.hpp: regs/holly/ta/global_parameter.csv regs/render_ta_parameter_struct.py
	python regs/render_ta_parameter_struct.py $< holly::ta::global_parameter > $@

holly/ta/vertex_parameter.hpp: regs/holly/ta/vertex_parameter.csv regs/render_ta_parameter_struct.py
	python regs/render_ta_parameter_struct.py $< holly::ta::vertex_parameter > $@

# SH7091

sh7091/sh7091.hpp: regs/sh7091/sh7091.csv regs/render_sh7091.py
	python regs/render_sh7091.py $< sh7091 > $@

sh7091/sh7091_bits.hpp: regs/sh7091/sh7091_bits.csv regs/render_bits.py
	python regs/render_bits.py $< sh7091 > $@

# SYSTEMBUS

systembus/systembus.hpp: regs/systembus/systembus.csv regs/render_block_regs.py
	python regs/render_block_regs.py $< systembus > $@

systembus/systembus_bits.hpp: regs/systembus/systembus_bits.csv regs/render_bits.py
	python regs/render_bits.py $< systembus > $@

# MAPLE

maple/maple_bits.hpp: regs/maple/maple_bits.csv regs/render_bits.py
	python regs/render_bits.py $< maple > $@

maple/maple_bus_bits.hpp: regs/maple/maple_bus_bits.csv regs/render_bits.py
	python regs/render_bits.py $< maple > $@

maple/maple_bus_commands.hpp: regs/maple/maple_bus_commands.csv regs/render_maple_bus_commands.py
	python regs/render_maple_bus_commands.py $< > $@

maple/maple_bus_ft0.hpp: regs/maple/maple_bus_ft0.csv regs/render_maple_data_format.py
	python regs/render_maple_data_format.py $< > $@

maple/maple_bus_ft6.hpp: regs/maple/maple_bus_ft6.csv regs/render_maple_data_format.py
	python regs/render_maple_data_format.py $< > $@

maple/maple_bus_ft6_scan_code.hpp: regs/maple/maple_bus_ft6_scan_code.csv regs/render_maple_scan_code.py
	python regs/render_maple_scan_code.py $< > $@
