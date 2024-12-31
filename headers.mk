%.csv: %.ods
	libreoffice --headless --convert-to csv:"Text - txt - csv (StarCalc)":44,34,76,,,,true --outdir $(dir $@) $<

# SH7091

sh7091/sh7091.hpp: regs/sh7091.csv regs/gen/sh7091.py
	python regs/gen/sh7091.py $< > $@

sh7091/sh7091_bits.hpp: regs/sh7091_bits.csv regs/gen/core_bits.py
	python regs/gen/core_bits.py $< > $@

# SYSTEMBUS

systembus_bits.hpp: regs/systembus_bits.csv regs/gen/core_bits.py
	python regs/gen/core_bits.py $< > $@

# DVE

dve.hpp: regs/dve.csv regs/gen/core_bits.py
	python regs/gen/core_bits.py $< > $@

# HOLLY

holly/core_bits.hpp: regs/core_bits.csv regs/gen/core_bits.py
	python regs/gen/core_bits.py $< > $@

holly/holly.hpp: regs/holly.csv regs/gen/holly.py
	python regs/gen/holly.py $< > $@

holly/ta_global_parameter.hpp: regs/global_parameter_format.csv regs/gen/ta_parameter_format.py regs/gen/generic_sparse_struct.py
	python regs/gen/ta_parameter_format.py $< ta_global_parameter > $@

holly/ta_vertex_parameter.hpp: regs/vertex_parameter_format.csv regs/gen/ta_parameter_format.py regs/gen/generic_sparse_struct.py
	python regs/gen/ta_parameter_format.py $< ta_vertex_parameter > $@

holly/object_list_data.hpp: regs/object_list.csv regs/gen/core_bits.py
	python regs/gen/core_bits.py $< object_list_data > $@

holly/video_output_mode.inc: regs/video_output.csv regs/gen/video_output.py
	python regs/gen/video_output.py $< > $@

# MAPLE

maple/maple_bus_commands.hpp: regs/maple_bus_commands.csv regs/gen/maple_bus_commands.py
	python regs/gen/maple_bus_commands.py $< > $@

maple/maple_bus_bits.hpp: regs/maple_bus_bits.csv regs/gen/core_bits.py
	python regs/gen/core_bits.py $< > $@

maple/maple_bus_ft0.hpp: regs/maple_bus_ft0.csv regs/gen/maple_data_format.py
	python regs/gen/maple_data_format.py $< > $@

maple/maple_bus_ft1.hpp: regs/maple_bus_ft1.csv regs/gen/maple_data_format.py
	python regs/gen/maple_data_format.py $< > $@

maple/maple_bus_ft6.hpp: regs/maple_bus_ft6.csv regs/gen/maple_data_format.py
	python regs/gen/maple_data_format.py $< > $@

maple/maple_bus_ft6_key_scan_codes.hpp: regs/maple_bus_ft6_key_scan_codes.csv regs/gen/maple_key_scan_codes.py
	python regs/gen/maple_key_scan_codes.py $< > $@

maple/maple_bus_ft8.hpp: regs/maple_bus_ft8.csv regs/gen/maple_data_format.py
	python regs/gen/maple_data_format.py $< > $@

maple/maple_bus_ft9.hpp: regs/maple_bus_ft9.csv regs/gen/maple_data_format.py
	python regs/gen/maple_data_format.py $< > $@

# AICA

aica/aica_channel.hpp: regs/aica_channel_data.csv regs/gen/aica.py
	python regs/gen/aica.py $< aica_channel 0x80 > $@

aica/aica_common.hpp: regs/aica_common_data.csv regs/gen/aica.py
	python regs/gen/aica.py $< aica_common > $@

aica/aica_rtc.hpp: regs/aica_rtc_data.csv regs/gen/aica.py
	python regs/gen/aica.py $< aica_rtc > $@

aica/aica_dsp_out.hpp: regs/aica_dsp_out_data.csv regs/gen/aica.py
	python regs/gen/aica.py $< aica_dsp_out > $@

# GDROM

gdrom/gdrom.hpp: regs/gdrom.csv regs/gen/gdrom.py
	python regs/gen/gdrom.py $< gdrom > $@

gdrom/gdrom_bits.hpp: regs/gdrom_bits.csv regs/gen/core_bits.py
	python regs/gen/core_bits.py $< gdrom > $@

gdrom/command_packet_format.hpp: regs/gdrom_command_packet_format.csv regs/gen/gdrom_command_packet_format.py regs/gen/generic_sparse_struct.py
	python regs/gen/gdrom_command_packet_format.py $< gdrom_command_packet_format > $@

# ISO9660

iso9660/%.hpp: iso9660/%.csv iso9660/byte_position_cpp.py
	python iso9660/byte_position_cpp.py $< > $@
