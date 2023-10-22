#pragma once

#define SAVE_CONTEXT(sp)
#define RESTORE_CONTEXT(sp)

#define SWAP_CONTEXT(saved_sp, restored_sp) \
  SAVE_CONTEXT(saved_sp)                    \
  RESTORE_CONTEXT(restored_sp)