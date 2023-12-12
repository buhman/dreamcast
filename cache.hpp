#pragma once

namespace cache {

void init() __attribute__ ((section (".p2ram.cache_init")));

}
