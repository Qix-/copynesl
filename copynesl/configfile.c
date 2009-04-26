
/* Enumeration for states in the configuration parsing. */

enum configuration_state {
  NEWLINE = 0x00,
  COMMENT = 0x01,
  SETTING = 0x10,
  VALUE = 0x12,
  SINGLE_QUOTE = 0x20,
  DOUBLE_QUOTE = 0x22
};

