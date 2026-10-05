#ifndef CONFIG_H
#define CONFIG_H

#define KVS_FLASH_SIZE              (64 * 1024)

#define KVS_FLASH_PAGE_SIZE         (4 * 1024)

#define KVS_NUM_PAGES               (KVS_FLASH_SIZE / KVS_FLASH_PAGE_SIZE)

#define KVS_FLASH_ERASE_CYCLES      10000

#define KVS_MAX_RECORD_SIZE         KVS_FLASH_PAGE_SIZE

#define KVS_FLASH_LOG_LEVEL         3

#endif
