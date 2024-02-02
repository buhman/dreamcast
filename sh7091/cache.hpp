#pragma once

namespace cache {

void init() __attribute__ ((section (".text.startup.cache_init")));

}
