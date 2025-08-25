define BUILD_BINARY_O
	$(OBJCOPY) \
		-I binary $(OBJARCH) \
		--rename-section .data=.data.$(basename $@) \
		$< $@
endef

makefile_path := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))
makefile_relative = $(shell realpath --relative-to $(makefile_path) $(1))
as_obj_binary = _binary_$(subst -,_,$(subst .,_,$(subst /,_,$(subst .h,,$(call makefile_relative,$(1))))))

define BUILD_BINARY_H
	@echo gen $(call makefile_relative,$@)
	@echo '#pragma once' > $@
	@echo '' >> $@
	@echo '#include <stdint.h>' >> $@
	@echo '' >> $@
	@echo '#ifdef __cplusplus' >> $@
	@echo 'extern "C" {' >> $@
	@echo '#endif' >> $@
	@echo '' >> $@
	@echo 'extern uint32_t $(call as_obj_binary,$@)_start __asm("$(call as_obj_binary,$@)_start");' >> $@
	@echo 'extern uint32_t $(call as_obj_binary,$@)_end __asm("$(call as_obj_binary,$@)_end");' >> $@
	@echo 'extern uint32_t $(call as_obj_binary,$@)_size __asm("$(call as_obj_binary,$@)_size");' >> $@
	@echo '' >> $@
	@echo '#ifdef __cplusplus' >> $@
	@echo '}' >> $@
	@echo '#endif' >> $@
endef
