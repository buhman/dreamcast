import square
import triangle

object_list_size = 16

for i in range((640 // 32) * (480 // 32)):
    block_start = (i + 0) * object_list_size
    block_end   = (i + 1) * object_list_size
    objects = square.object_list[block_start:block_end]

    stride = (640 // 32)

    tile_x = (i % stride)
    tile_y = (i // stride)

    for j in range(16):
        if objects[j] & (1 << 28) != 0:
            break
        print(f"tile {tile_x: 4} {tile_y: 4}: {hex(objects[j])}")
