#pragma once

typedef enum tpl {
    TPL_APPLICATION = 4,
    TPL_CALLBACK = 8,
    TPL_NOTIFY = 16,
    TPL_HIGH_LEVEL = 31
} tpl_t;

tpl_t raise_tpl(tpl_t new_tpl);

void restore_tpl(tpl_t new_tpl);
