#pragma once

typedef void loop_fn_t(void* arg);
void cherry_tomato_entry();
void start_event_loop(loop_fn_t* fn, void* arg);