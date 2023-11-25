#include "bt.h"
#include "util/except.h"
#include "btdm_app.h"

static xt_handler esp32_set_isr(int n, xt_handler f, void *arg) { LOG_ERROR("btdm: _set_isr"); for (;;); __builtin_unreachable(); }
static void esp32_ints_on(unsigned int mask) { LOG_ERROR("btdm: _ints_on"); for (;;); __builtin_unreachable(); }
static void esp32_interrupt_disable(void) { LOG_ERROR("btdm: _interrupt_disable"); for (;;); __builtin_unreachable(); }
static void esp32_interrupt_restore(void) { LOG_ERROR("btdm: _interrupt_restore"); for (;;); __builtin_unreachable(); }
static void esp32_task_yield(void) { LOG_ERROR("btdm: _task_yield"); for (;;); __builtin_unreachable(); }
static void esp32_task_yield_from_isr(void) { LOG_ERROR("btdm: _task_yield_from_isr"); for (;;); __builtin_unreachable(); }
static void* esp32_semphr_create(uint32_t max, uint32_t init) { LOG_ERROR("btdm: _semphr_create"); for (;;); __builtin_unreachable(); }
static void esp32_semphr_delete(void *semphr) { LOG_ERROR("btdm: _semphr_delete"); for (;;); __builtin_unreachable(); }
static int32_t esp32_semphr_take_from_isr(void *semphr, void *hptw) { LOG_ERROR("btdm: _semphr_take_from_isr"); for (;;); __builtin_unreachable(); }
static int32_t esp32_semphr_give_from_isr(void *semphr, void *hptw) { LOG_ERROR("btdm: _semphr_give_from_isr"); for (;;); __builtin_unreachable(); }
static int32_t esp32_semphr_take(void *semphr, uint32_t block_time_ms) { LOG_ERROR("btdm: _semphr_take"); for (;;); __builtin_unreachable(); }
static int32_t esp32_semphr_give(void *semphr) { LOG_ERROR("btdm: _semphr_give"); for (;;); __builtin_unreachable(); }
static void* esp32_mutex_create(void) { LOG_ERROR("btdm: _mutex_create"); for (;;); __builtin_unreachable(); }
static void esp32_mutex_delete(void *mutex) { LOG_ERROR("btdm: _mutex_delete"); for (;;); __builtin_unreachable(); }
static int32_t esp32_mutex_lock(void *mutex) { LOG_ERROR("btdm: _mutex_lock"); for (;;); __builtin_unreachable(); }
static int32_t esp32_mutex_unlock(void *mutex) { LOG_ERROR("btdm: _mutex_unlock"); for (;;); __builtin_unreachable(); }
static void* esp32_queue_create(uint32_t queue_len, uint32_t item_size) { LOG_ERROR("btdm: _queue_create"); for (;;); __builtin_unreachable(); }
static void esp32_queue_delete(void *queue) { LOG_ERROR("btdm: _queue_delete"); for (;;); __builtin_unreachable(); }
static int32_t esp32_queue_send(void *queue, void *item, uint32_t block_time_ms) { LOG_ERROR("btdm: _queue_send"); for (;;); __builtin_unreachable(); }
static int32_t esp32_queue_send_from_isr(void *queue, void *item, void *hptw) { LOG_ERROR("btdm: _queue_send_from_isr"); for (;;); __builtin_unreachable(); }
static int32_t esp32_queue_recv(void *queue, void *item, uint32_t block_time_ms) { LOG_ERROR("btdm: _queue_recv"); for (;;); __builtin_unreachable(); }
static int32_t esp32_queue_recv_from_isr(void *queue, void *item, void *hptw) { LOG_ERROR("btdm: _queue_recv_from_isr"); for (;;); __builtin_unreachable(); }
static int32_t esp32_task_create(void *task_func, const char *name, uint32_t stack_depth, void *param, uint32_t prio, void *task_handle, uint32_t core_id) { LOG_ERROR("btdm: _task_create"); for (;;); __builtin_unreachable(); }
static void esp32_task_delete(void *task_handle) { LOG_ERROR("btdm: _task_delete"); for (;;); __builtin_unreachable(); }
static bool esp32_is_in_isr(void) { LOG_ERROR("btdm: _is_in_isr"); for (;;); __builtin_unreachable(); }
static int esp32_cause_sw_intr_to_core(int core_id, int intr_no) { LOG_ERROR("btdm: _cause_sw_intr_to_core"); for (;;); __builtin_unreachable(); }
static void* esp32_malloc(size_t size) { LOG_ERROR("btdm: _malloc"); for (;;); __builtin_unreachable(); }
static void* esp32_malloc_internal(size_t size) { LOG_ERROR("btdm: _malloc_internal"); for (;;); __builtin_unreachable(); }
static void esp32_free(void *p) { LOG_ERROR("btdm: _free"); for (;;); __builtin_unreachable(); }
static int32_t esp32_read_efuse_mac(uint8_t mac[6]) { LOG_ERROR("btdm: _read_efuse_mac"); for (;;); __builtin_unreachable(); }
static void esp32_srand(unsigned int seed) { LOG_ERROR("btdm: _srand"); for (;;); __builtin_unreachable(); }
static int esp32_rand(void) { LOG_ERROR("btdm: _rand"); for (;;); __builtin_unreachable(); }
static uint32_t esp32_btdm_lpcycles_2_us(uint32_t cycles) { LOG_ERROR("btdm: _btdm_lpcycles_2_us"); for (;;); __builtin_unreachable(); }
static uint32_t esp32_btdm_us_2_lpcycles(uint32_t us) { LOG_ERROR("btdm: _btdm_us_2_lpcycles"); for (;;); __builtin_unreachable(); }
static bool esp32_btdm_sleep_check_duration(uint32_t *slot_cnt) { LOG_ERROR("btdm: _btdm_sleep_check_duration"); for (;;); __builtin_unreachable(); }
static void esp32_btdm_sleep_enter_phase1(uint32_t lpcycles) { LOG_ERROR("btdm: _btdm_sleep_enter_phase1"); for (;;); __builtin_unreachable(); }
static void esp32_btdm_sleep_enter_phase2(void) { LOG_ERROR("btdm: _btdm_sleep_enter_phase2"); for (;;); __builtin_unreachable(); }
static void esp32_btdm_sleep_exit_phase1(void) { LOG_ERROR("btdm: _btdm_sleep_exit_phase1"); for (;;); __builtin_unreachable(); }
static void esp32_btdm_sleep_exit_phase2(void) { LOG_ERROR("btdm: _btdm_sleep_exit_phase2"); for (;;); __builtin_unreachable(); }
static void esp32_btdm_sleep_exit_phase3(void) { LOG_ERROR("btdm: _btdm_sleep_exit_phase3"); for (;;); __builtin_unreachable(); }
static bool esp32_coex_bt_wakeup_request(void) { LOG_ERROR("btdm: _coex_bt_wakeup_request"); for (;;); __builtin_unreachable(); }
static void esp32_coex_bt_wakeup_request_end(void) { LOG_ERROR("btdm: _coex_bt_wakeup_request_end"); for (;;); __builtin_unreachable(); }
static int esp32_coex_bt_request(uint32_t event, uint32_t latency, uint32_t duration) { LOG_ERROR("btdm: _coex_bt_request"); for (;;); __builtin_unreachable(); }
static int esp32_coex_bt_release(uint32_t event) { LOG_ERROR("btdm: _coex_bt_release"); for (;;); __builtin_unreachable(); }
static int esp32_coex_register_bt_cb(coex_func_cb_t cb) { LOG_ERROR("btdm: _coex_register_bt_cb"); for (;;); __builtin_unreachable(); }
static uint32_t esp32_coex_bb_reset_lock(void) { LOG_ERROR("btdm: _coex_bb_reset_lock"); for (;;); __builtin_unreachable(); }
static void esp32_coex_bb_reset_unlock(uint32_t restore) { LOG_ERROR("btdm: _coex_bb_reset_unlock"); for (;;); __builtin_unreachable(); }
static int esp32_coex_schm_register_btdm_callback(void *callback) { LOG_ERROR("btdm: _coex_schm_register_btdm_callback"); for (;;); __builtin_unreachable(); }
static void esp32_coex_schm_status_bit_clear(uint32_t type, uint32_t status) { LOG_ERROR("btdm: _coex_schm_status_bit_clear"); for (;;); __builtin_unreachable(); }
static void esp32_coex_schm_status_bit_set(uint32_t type, uint32_t status) { LOG_ERROR("btdm: _coex_schm_status_bit_set"); for (;;); __builtin_unreachable(); }
static uint32_t esp32_coex_schm_interval_get(void) { LOG_ERROR("btdm: _coex_schm_interval_get"); for (;;); __builtin_unreachable(); }
static uint8_t esp32_coex_schm_curr_period_get(void) { LOG_ERROR("btdm: _coex_schm_curr_period_get"); for (;;); __builtin_unreachable(); }
static void* esp32_coex_schm_curr_phase_get(void) { LOG_ERROR("btdm: _coex_schm_curr_phase_get"); for (;;); __builtin_unreachable(); }
static int esp32_coex_wifi_channel_get(uint8_t *primary, uint8_t *secondary) { LOG_ERROR("btdm: _coex_wifi_channel_get"); for (;;); __builtin_unreachable(); }
static int esp32_coex_register_wifi_channel_change_callback(void *cb) { LOG_ERROR("btdm: _coex_register_wifi_channel_change_callback"); for (;;); __builtin_unreachable(); }
static xt_handler esp32_set_isr_l3(int n, xt_handler f, void *arg) { LOG_ERROR("btdm: _set_isr_l3"); for (;;); __builtin_unreachable(); }
static void esp32_interrupt_l3_disable(void) { LOG_ERROR("btdm: _interrupt_l3_disable"); for (;;); __builtin_unreachable(); }
static void esp32_interrupt_l3_restore(void) { LOG_ERROR("btdm: _interrupt_l3_restore"); for (;;); __builtin_unreachable(); }
static void* esp32_customer_queue_create(uint32_t queue_len, uint32_t item_size) { LOG_ERROR("btdm: _customer_queue_create"); for (;;); __builtin_unreachable(); }
static int esp32_coex_version_get(unsigned int *major, unsigned int *minor, unsigned int *patch) { LOG_ERROR("btdm: _coex_version_get"); for (;;); __builtin_unreachable(); }

static esp_osi_funcs_t m_esp32_osi_funcs = {
    ._version = OSI_VERSION,
    ._set_isr = esp32_set_isr,
    ._ints_on = esp32_ints_on,
    ._interrupt_disable = esp32_interrupt_disable,
    ._interrupt_restore = esp32_interrupt_restore,
    ._task_yield = esp32_task_yield,
    ._task_yield_from_isr = esp32_task_yield_from_isr,
    ._semphr_create = esp32_semphr_create,
    ._semphr_delete = esp32_semphr_delete,
    ._semphr_take_from_isr = esp32_semphr_take_from_isr,
    ._semphr_give_from_isr = esp32_semphr_give_from_isr,
    ._semphr_take = esp32_semphr_take,
    ._semphr_give = esp32_semphr_give,
    ._mutex_create = esp32_mutex_create,
    ._mutex_delete = esp32_mutex_delete,
    ._mutex_lock = esp32_mutex_lock,
    ._mutex_unlock = esp32_mutex_unlock,
    ._queue_create = esp32_queue_create,
    ._queue_delete = esp32_queue_delete,
    ._queue_send = esp32_queue_send,
    ._queue_send_from_isr = esp32_queue_send_from_isr,
    ._queue_recv = esp32_queue_recv,
    ._queue_recv_from_isr = esp32_queue_recv_from_isr,
    ._task_create = esp32_task_create,
    ._task_delete = esp32_task_delete,
    ._is_in_isr = esp32_is_in_isr,
    ._cause_sw_intr_to_core = esp32_cause_sw_intr_to_core,
    ._malloc = esp32_malloc,
    ._malloc_internal = esp32_malloc_internal,
    ._free = esp32_free,
    ._read_efuse_mac = esp32_read_efuse_mac,
    ._srand = esp32_srand,
    ._rand = esp32_rand,
    ._btdm_lpcycles_2_us = esp32_btdm_lpcycles_2_us,
    ._btdm_us_2_lpcycles = esp32_btdm_us_2_lpcycles,
    ._btdm_sleep_check_duration = esp32_btdm_sleep_check_duration,
    ._btdm_sleep_enter_phase1 = esp32_btdm_sleep_enter_phase1,
    ._btdm_sleep_enter_phase2 = esp32_btdm_sleep_enter_phase2,
    ._btdm_sleep_exit_phase1 = esp32_btdm_sleep_exit_phase1,
    ._btdm_sleep_exit_phase2 = esp32_btdm_sleep_exit_phase2,
    ._btdm_sleep_exit_phase3 = esp32_btdm_sleep_exit_phase3,
    ._coex_bt_wakeup_request = esp32_coex_bt_wakeup_request,
    ._coex_bt_wakeup_request_end = esp32_coex_bt_wakeup_request_end,
    ._coex_bt_request = esp32_coex_bt_request,
    ._coex_bt_release = esp32_coex_bt_release,
    ._coex_register_bt_cb = esp32_coex_register_bt_cb,
    ._coex_bb_reset_lock = esp32_coex_bb_reset_lock,
    ._coex_bb_reset_unlock = esp32_coex_bb_reset_unlock,
    ._coex_schm_register_btdm_callback = esp32_coex_schm_register_btdm_callback,
    ._coex_schm_status_bit_clear = esp32_coex_schm_status_bit_clear,
    ._coex_schm_status_bit_set = esp32_coex_schm_status_bit_set,
    ._coex_schm_interval_get = esp32_coex_schm_interval_get,
    ._coex_schm_curr_period_get = esp32_coex_schm_curr_period_get,
    ._coex_schm_curr_phase_get = esp32_coex_schm_curr_phase_get,
    ._coex_wifi_channel_get = esp32_coex_wifi_channel_get,
    ._coex_register_wifi_channel_change_callback = esp32_coex_register_wifi_channel_change_callback,
    ._set_isr_l3 = esp32_set_isr_l3,
    ._interrupt_l3_disable = esp32_interrupt_l3_disable,
    ._interrupt_l3_restore = esp32_interrupt_l3_restore,
    ._customer_queue_create = esp32_customer_queue_create,
    ._coex_version_get = esp32_coex_version_get,
    ._magic = OSI_MAGIC_VALUE,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "hardware/rtc_cntl.h"

static esp_bt_controller_config_t m_bt_config = {
    .controller_task_prio = 23,
    .controller_task_stack_size = ESP_TASK_BT_CONTROLLER_STACK,
    .mode = ESP_BT_MODE_BLE,
    .ble_max_conn = 1,
    .magic = ESP_BT_CONTROLLER_CONFIG_MAGIC_VAL
};

err_t esp32_init_bluetooth() {
    err_t err = NO_ERROR;

    // register the osi functions
    btdm_osi_funcs_register(&m_esp32_osi_funcs);

    // init the controller
    uint32_t config_mask = BTDM_CFG_SCAN_DUPLICATE_OPTIONS | BTDM_CFG_SEND_ADV_RESERVED_SIZE;
    CHECK(btdm_controller_init(config_mask, &m_bt_config) == 0);

cleanup:
    return err;
}
