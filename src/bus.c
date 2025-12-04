#include "bus.h"

/*
 * bus_load
 * 作用：从总线加载数据。
 * 行为：
 *   - 调用 DRAM 加载函数，从 DRAM 中读取数据；
 *   - 返回读取到的数据。
 * 示例：
 *   bus_load(bus, 0x00001000, 32) => 0x10001111000050ee000060ff000011f1
 */
uint64_t bus_load(BUS* bus, uint64_t addr, uint64_t size) {
    return dram_load(&(bus->dram), addr, size);
}

/*
 * bus_store
 * 作用：向总线存储数据。
 * 行为：
 *   - 调用 DRAM 存储函数，将数据写入 DRAM；
 *   - 无返回值。
 * 示例：
 *   bus_store(bus, 0x00001000, 32, 0x10001111000050ee000060ff000011f1) => 无返回值
 */
void bus_store(BUS* bus, uint64_t addr, uint64_t size, uint64_t value) {
    dram_store(&(bus->dram), addr, size, value);
}
