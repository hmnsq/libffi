/* args are passed on registers from r0 up to r11 => 12*8 bytes */
#تعریف_کن REG_آرگومانها_اندازه (12*8)
#تعریف_کن KVX_ثبات_اندازه (8)
#تعریف_کن KVX_ABI_جایگاه_اندازه (KVX_ثبات_اندازه)
#تعریف_کن KVX_ABI_MAX_AGGREGATE_درون_REG_اندازه (4*KVX_ABI_جایگاه_اندازه)
