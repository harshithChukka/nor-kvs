#ifndef FLASH_DRIVER_H
#define FLASH_DRIVER_H

#include <stdint.h>

#include "config.h"

typedef enum
{
  FLASH_SUCCESS,
  FLASH_FAIL
} FLASH_STATUS;

typedef struct
{
  uint8_t flash[KVS_FLASH_SIZE];
} flash_driver_t;

FLASH_STATUS flash_init(flash_driver_t *fd);

FLASH_STATUS flash_write(flash_driver_t *fd, const uint32_t addr, const void *data, const uint32_t size);

FLASH_STATUS flash_read(flash_driver_t *fd, const uint32_t addr, void *data, const uint32_t size);

FLASH_STATUS flash_erase(flash_driver_t *fd, const uint8_t pageNum);

#endif