#include <stdbool.h>
#include <string.h>

#include "flash_driver.h"
#include "logger.h"

static logger_t flash_logger;

#define MODULE "FLASH"
#define FLASH_INFO(...) logger_log(&flash_logger, LOG_INFO, __VA_ARGS__)
#define FLASH_ERROR(...) logger_log(&flash_logger, LOG_ERROR, __VA_ARGS__)
#define FLASH_WARN(...) logger_log(&flash_logger, LOG_WARN, __VA_ARGS__)
#define FLASH_DEBUG(...) logger_log(&flash_logger, LOG_DEBUG, __VA_ARGS__)

static bool is_addr_valid(uint32_t addr, uint32_t size)
{
  if (size == 0)
    return false;

  if (addr >= KVS_FLASH_SIZE)
    return false;

  if (size > KVS_FLASH_SIZE - addr)
    return false;

  return true;
}

FLASH_STATUS flash_init(flash_driver_t *fd)
{
  if (fd == NULL)
    return FLASH_FAIL;

  logger_init(&flash_logger, KVS_FLASH_LOG_LEVEL, MODULE);
  memset(fd->flash, 0xFF, KVS_FLASH_SIZE);

  FLASH_INFO("Flash initialized, size=%u", KVS_FLASH_SIZE);
  return FLASH_SUCCESS;
}

FLASH_STATUS flash_write(flash_driver_t *fd, const uint32_t addr, const void *data, const uint32_t size)
{
  if (fd == NULL || data == NULL || !is_addr_valid(addr, size))
    return FLASH_FAIL;

  uint8_t *src = (uint8_t *)data;
  for (int i = 0; i < size; i++)
  {
    if ((fd->flash[addr + i] & src[i]) != src[i])
    {
      FLASH_ERROR("Cannot program address %u: 0 -> 1 transition", addr + i);
      return FLASH_FAIL;
    }
  }

  for (int i = 0; i < size; i++)
  {
    fd->flash[addr + i] &= src[i];
  }
  return FLASH_SUCCESS;
}

FLASH_STATUS flash_read(flash_driver_t *fd, const uint32_t addr, void *data, const uint32_t size)
{
  if (fd == NULL || data == NULL || !is_addr_valid(addr, size))
    return FLASH_FAIL;

  memcpy(data, fd->flash + addr, size);
  return FLASH_SUCCESS;
}

FLASH_STATUS flash_erase(flash_driver_t *fd, const uint8_t pageNum)
{
  if (fd == NULL || pageNum >= KVS_NUM_PAGES)
    return FLASH_FAIL;

  uint32_t addr = pageNum * KVS_FLASH_PAGE_SIZE;
  memset(fd->flash + addr, 0xFF, KVS_FLASH_PAGE_SIZE);
  return FLASH_SUCCESS;
}
