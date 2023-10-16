#pragma once

#define SAVE_CONTEXT(sp, tag)
#define RESTORE_CONTEXT(sp, tag)

#define SWAP_CONTEXT(saved_sp, restored_sp, tag) \
  SAVE_CONTEXT(saved_sp, tag)                    \
  RESTORE_CONTEXT(restored_sp, tag)