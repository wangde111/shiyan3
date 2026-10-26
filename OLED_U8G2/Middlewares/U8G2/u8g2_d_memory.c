/* u8g2_d_memory.c */
/* 裁剪版: 只保留 SSD1306 128x64 全屏缓冲所需的 1 KB 显存。 */

#include "u8g2.h"

uint8_t *u8g2_m_16_8_f(uint8_t *page_cnt)
{
#ifdef U8G2_USE_DYNAMIC_ALLOC
  *page_cnt = 8;
  return 0;
#else
  static uint8_t buf[1024];
  *page_cnt = 8;
  return buf;
#endif
}
