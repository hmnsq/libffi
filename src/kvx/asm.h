/* args are passed on registers from r0 up to r11 => 12*8 bytes */
#define REG_ARGS_আকার (12*8)
#define KVX_REGISTER_আকার (8)
#define KVX_ABI_SLOT_আকার (KVX_REGISTER_আকার)
#define KVX_ABI_MAX_AGGREGATE_IN_REG_আকার (4*KVX_ABI_SLOT_আকার)
