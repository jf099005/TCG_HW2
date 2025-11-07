# sources.mk
# ----------
# Edit this file!

# +-- Set to 0 for English board output --+
CHINESE = 1

SRC_DIR = MCTS/CPP
INC_DIR = MCTS/H

# +-- Add your own sources here, if any --+
ADD_SOURCES = -I. -Ilib -I$(INC_DIR) $(wildcard $(SRC_DIR)/*.cpp)
# ADD_SOURCES = 