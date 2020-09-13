#
# Build specifics for Pergola hardware.
#

# This is an external board, so its identity is determined by its revision number.
# MAJOR = external board
# MINOR = 0 (Daisho)
# MINOR = 1 (Pergola)
BOARD_REVISION_MAJOR := 255
BOARD_REVISION_MINOR := 1

APP_START_ADDRESS ?= 0x6000C000
