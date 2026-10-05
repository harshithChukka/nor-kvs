#include <string.h>

#include "unity.h"
#include "flash_driver.h"

/* 64 KiB simulated flash: keep it off the stack. */
static flash_driver_t fd;

#define LAST_ADDR (KVS_FLASH_SIZE - 1)

void setUp(void)
{
  /* Fill with garbage so tests prove flash_init really erases. */
  memset(fd.flash, 0xA5, KVS_FLASH_SIZE);
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_init(&fd));
}

void tearDown(void)
{
}

/* ---------- flash_init ---------- */

void test_init_null_driver_fails(void)
{
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_init(NULL));
}

void test_init_erases_entire_flash(void)
{
  TEST_ASSERT_EACH_EQUAL_HEX8(0xFF, fd.flash, KVS_FLASH_SIZE);
}

/* ---------- flash_write ---------- */

void test_write_null_driver_fails(void)
{
  uint8_t data = 0x00;
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_write(NULL, 0, &data, 1));
}

void test_write_null_data_fails(void)
{
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_write(&fd, 0, NULL, 1));
}

void test_write_zero_size_fails(void)
{
  uint8_t data = 0x00;
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_write(&fd, 0, &data, 0));
}

void test_write_addr_past_end_fails(void)
{
  uint8_t data = 0x00;
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_write(&fd, KVS_FLASH_SIZE, &data, 1));
}

void test_write_range_crossing_end_fails_and_leaves_flash_untouched(void)
{
  uint8_t data[2] = {0x00, 0x00};
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_write(&fd, LAST_ADDR, data, sizeof(data)));
  TEST_ASSERT_EQUAL_HEX8(0xFF, fd.flash[LAST_ADDR]);
}

void test_write_failure_does_not_partially_modify_flash(void)
{
  uint8_t initial[] = {0xFF, 0xFF, 0x00, 0xFF};
  uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};

  memcpy(fd.flash + 100, initial, sizeof(initial));

  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_write(&fd, 100, data, sizeof(data)));
  TEST_ASSERT_EQUAL_HEX8(0xFF, fd.flash[100]);
  TEST_ASSERT_EQUAL_HEX8(0xFF, fd.flash[101]);
  TEST_ASSERT_EQUAL_HEX8(0x00, fd.flash[102]);
  TEST_ASSERT_EQUAL_HEX8(0xFF, fd.flash[103]);
}

void test_write_huge_size_does_not_overflow_bounds_check(void)
{
  uint8_t data = 0x00;
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_write(&fd, 1, &data, UINT32_MAX));
}

void test_write_exactly_to_end_of_flash_succeeds(void)
{
  uint8_t data[1] = {0xAA};
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, KVS_FLASH_SIZE - 1, data, 1));
  TEST_ASSERT_EQUAL_HEX8(0xAA, fd.flash[KVS_FLASH_SIZE - 1]);
}

void test_write_then_read_back(void)
{
  const uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
  uint8_t out[sizeof(data)] = {0};

  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, 100, data, sizeof(data)));
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_read(&fd, 100, out, sizeof(out)));
  TEST_ASSERT_EQUAL_HEX8_ARRAY(data, out, sizeof(data));
}

void test_write_does_not_touch_neighbouring_bytes(void)
{
  const uint8_t data[] = {0x11, 0x22};

  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, 10, data, sizeof(data)));
  TEST_ASSERT_EQUAL_HEX8(0xFF, fd.flash[9]);
  TEST_ASSERT_EQUAL_HEX8(0xFF, fd.flash[12]);
}

void test_write_first_and_last_byte(void)
{
  uint8_t data = 0x5A;

  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, 0, &data, 1));
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, LAST_ADDR, &data, 1));
  TEST_ASSERT_EQUAL_HEX8(0x5A, fd.flash[0]);
  TEST_ASSERT_EQUAL_HEX8(0x5A, fd.flash[LAST_ADDR]);
}

void test_write_entire_flash(void)
{
  static uint8_t data[KVS_FLASH_SIZE];
  memset(data, 0x00, sizeof(data));

  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, 0, data, KVS_FLASH_SIZE));
  TEST_ASSERT_EACH_EQUAL_HEX8(0x00, fd.flash, KVS_FLASH_SIZE);
}

void test_write_clearing_more_bits_on_written_byte_succeeds(void)
{
  /* NOR semantics: a program can only flip bits 1 -> 0. */
  uint8_t first = 0xF0;
  uint8_t second = 0x30;

  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, 0, &first, 1));
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, 0, &second, 1));
  TEST_ASSERT_EQUAL_HEX8(0x30, fd.flash[0]);
}

void test_write_same_value_twice_succeeds(void)
{
  uint8_t data = 0x42;

  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, 0, &data, 1));
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, 0, &data, 1));
  TEST_ASSERT_EQUAL_HEX8(0x42, fd.flash[0]);
}

void test_write_setting_bit_back_to_one_fails_and_keeps_old_value(void)
{
  uint8_t first = 0x00;
  uint8_t second = 0x01;

  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, 0, &first, 1));
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_write(&fd, 0, &second, 1));
  TEST_ASSERT_EQUAL_HEX8(0x00, fd.flash[0]);
}

/* ---------- flash_read ---------- */

void test_read_null_driver_fails(void)
{
  uint8_t out = 0;
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_read(NULL, 0, &out, 1));
}

void test_read_null_data_fails(void)
{
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_read(&fd, 0, NULL, 1));
}

void test_read_zero_size_fails(void)
{
  uint8_t out = 0;
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_read(&fd, 0, &out, 0));
}

void test_read_addr_past_end_fails(void)
{
  uint8_t out = 0;
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_read(&fd, KVS_FLASH_SIZE, &out, 1));
}

void test_read_range_crossing_end_fails_and_leaves_buffer_untouched(void)
{
  uint8_t out[2] = {0x12, 0x34};

  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_read(&fd, LAST_ADDR, out, sizeof(out)));
  TEST_ASSERT_EQUAL_HEX8(0x12, out[0]);
  TEST_ASSERT_EQUAL_HEX8(0x34, out[1]);
}

void test_read_erased_flash_returns_ff(void)
{
  uint8_t out[16];
  memset(out, 0, sizeof(out));

  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_read(&fd, 0, out, sizeof(out)));
  TEST_ASSERT_EACH_EQUAL_HEX8(0xFF, out, sizeof(out));
}

void test_read_across_page_boundary(void)
{
  const uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
  const uint32_t addr = KVS_FLASH_PAGE_SIZE - 2;
  uint8_t out[sizeof(data)] = {0};

  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, addr, data, sizeof(data)));
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_read(&fd, addr, out, sizeof(out)));
  TEST_ASSERT_EQUAL_HEX8_ARRAY(data, out, sizeof(data));
}

/* ---------- flash_erase ---------- */

void test_erase_null_driver_fails(void)
{
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_erase(NULL, 0));
}

void test_erase_page_out_of_range_fails(void)
{
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_erase(&fd, KVS_NUM_PAGES));
}

void test_erase_resets_page_to_ff(void)
{
  static uint8_t zeros[KVS_FLASH_PAGE_SIZE];
  const uint32_t page = 3;

  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, page * KVS_FLASH_PAGE_SIZE, zeros, sizeof(zeros)));
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_erase(&fd, page));
  TEST_ASSERT_EACH_EQUAL_HEX8(0xFF, &fd.flash[page * KVS_FLASH_PAGE_SIZE], KVS_FLASH_PAGE_SIZE);
}

void test_erase_does_not_touch_other_pages(void)
{
  static uint8_t zeros[KVS_FLASH_SIZE];
  const uint32_t page = 5;

  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, 0, zeros, sizeof(zeros)));
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_erase(&fd, page));

  TEST_ASSERT_EACH_EQUAL_HEX8(0x00, fd.flash, page * KVS_FLASH_PAGE_SIZE);
  TEST_ASSERT_EACH_EQUAL_HEX8(0x00, &fd.flash[(page + 1) * KVS_FLASH_PAGE_SIZE], KVS_FLASH_SIZE - (page + 1) * KVS_FLASH_PAGE_SIZE);
}

void test_erase_first_and_last_page(void)
{
  uint8_t data = 0x00;

  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, 0, &data, 1));
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, LAST_ADDR, &data, 1));
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_erase(&fd, 0));
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_erase(&fd, KVS_NUM_PAGES - 1));
  TEST_ASSERT_EQUAL_HEX8(0xFF, fd.flash[0]);
  TEST_ASSERT_EQUAL_HEX8(0xFF, fd.flash[LAST_ADDR]);
}

void test_erase_allows_rewrite_of_previously_written_bytes(void)
{
  uint8_t first = 0x00;
  uint8_t second = 0xAB;

  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, 0, &first, 1));
  TEST_ASSERT_EQUAL(FLASH_FAIL, flash_write(&fd, 0, &second, 1));
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_erase(&fd, 0));
  TEST_ASSERT_EQUAL(FLASH_SUCCESS, flash_write(&fd, 0, &second, 1));
  TEST_ASSERT_EQUAL_HEX8(0xAB, fd.flash[0]);
}

int main(void)
{
  UNITY_BEGIN();

  RUN_TEST(test_init_null_driver_fails);
  RUN_TEST(test_init_erases_entire_flash);

  RUN_TEST(test_write_null_driver_fails);
  RUN_TEST(test_write_null_data_fails);
  RUN_TEST(test_write_zero_size_fails);
  RUN_TEST(test_write_addr_past_end_fails);
  RUN_TEST(test_write_range_crossing_end_fails_and_leaves_flash_untouched);
  RUN_TEST(test_write_failure_does_not_partially_modify_flash);
  RUN_TEST(test_write_huge_size_does_not_overflow_bounds_check);
  RUN_TEST(test_write_exactly_to_end_of_flash_succeeds);
  RUN_TEST(test_write_then_read_back);
  RUN_TEST(test_write_does_not_touch_neighbouring_bytes);
  RUN_TEST(test_write_first_and_last_byte);
  RUN_TEST(test_write_entire_flash);
  RUN_TEST(test_write_clearing_more_bits_on_written_byte_succeeds);
  RUN_TEST(test_write_same_value_twice_succeeds);
  RUN_TEST(test_write_setting_bit_back_to_one_fails_and_keeps_old_value);

  RUN_TEST(test_read_null_driver_fails);
  RUN_TEST(test_read_null_data_fails);
  RUN_TEST(test_read_zero_size_fails);
  RUN_TEST(test_read_addr_past_end_fails);
  RUN_TEST(test_read_range_crossing_end_fails_and_leaves_buffer_untouched);
  RUN_TEST(test_read_erased_flash_returns_ff);
  RUN_TEST(test_read_across_page_boundary);

  RUN_TEST(test_erase_null_driver_fails);
  RUN_TEST(test_erase_page_out_of_range_fails);
  RUN_TEST(test_erase_resets_page_to_ff);
  RUN_TEST(test_erase_does_not_touch_other_pages);
  RUN_TEST(test_erase_first_and_last_page);
  RUN_TEST(test_erase_allows_rewrite_of_previously_written_bytes);

  return UNITY_END();
}
