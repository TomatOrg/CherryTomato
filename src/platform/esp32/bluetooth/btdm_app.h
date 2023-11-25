#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* Bluetooth system and controller config */
#define BTDM_CFG_BT_DATA_RELEASE            (1<<0)
#define BTDM_CFG_HCI_UART                   (1<<1)
#define BTDM_CFG_CONTROLLER_RUN_APP_CPU     (1<<2)
#define BTDM_CFG_SCAN_DUPLICATE_OPTIONS     (1<<3)
#define BTDM_CFG_SEND_ADV_RESERVED_SIZE     (1<<4)
#define BTDM_CFG_BLE_FULL_SCAN_SUPPORTED    (1<<5)

typedef void (*xt_handler)(void *);

typedef void (* coex_func_cb_t)(uint32_t event, int sched_cnt);

#define OSI_FUNCS_TIME_BLOCKING  0xffffffff
#define OSI_VERSION              0x00010004
#define OSI_MAGIC_VALUE          0xFADEBEAD

typedef struct esp32_osi_funcs {
    uint32_t _version;
    xt_handler (*_set_isr)(int n, xt_handler f, void *arg);
    void (*_ints_on)(unsigned int mask);
    void (*_interrupt_disable)(void);
    void (*_interrupt_restore)(void);
    void (*_task_yield)(void);
    void (*_task_yield_from_isr)(void);
    void *(*_semphr_create)(uint32_t max, uint32_t init);
    void (*_semphr_delete)(void *semphr);
    int32_t (*_semphr_take_from_isr)(void *semphr, void *hptw);
    int32_t (*_semphr_give_from_isr)(void *semphr, void *hptw);
    int32_t (*_semphr_take)(void *semphr, uint32_t block_time_ms);
    int32_t (*_semphr_give)(void *semphr);
    void *(*_mutex_create)(void);
    void (*_mutex_delete)(void *mutex);
    int32_t (*_mutex_lock)(void *mutex);
    int32_t (*_mutex_unlock)(void *mutex);
    void *(* _queue_create)(uint32_t queue_len, uint32_t item_size);
    void (* _queue_delete)(void *queue);
    int32_t (* _queue_send)(void *queue, void *item, uint32_t block_time_ms);
    int32_t (* _queue_send_from_isr)(void *queue, void *item, void *hptw);
    int32_t (* _queue_recv)(void *queue, void *item, uint32_t block_time_ms);
    int32_t (* _queue_recv_from_isr)(void *queue, void *item, void *hptw);
    int32_t (* _task_create)(void *task_func, const char *name, uint32_t stack_depth, void *param, uint32_t prio, void *task_handle, uint32_t core_id);
    void (* _task_delete)(void *task_handle);
    bool (* _is_in_isr)(void);
    int (* _cause_sw_intr_to_core)(int core_id, int intr_no);
    void *(* _malloc)(size_t size);
    void *(* _malloc_internal)(size_t size);
    void (* _free)(void *p);
    int32_t (* _read_efuse_mac)(uint8_t mac[6]);
    void (* _srand)(unsigned int seed);
    int (* _rand)(void);
    uint32_t (* _btdm_lpcycles_2_us)(uint32_t cycles);
    uint32_t (* _btdm_us_2_lpcycles)(uint32_t us);
    bool (* _btdm_sleep_check_duration)(uint32_t *slot_cnt);
    void (* _btdm_sleep_enter_phase1)(uint32_t lpcycles);  /* called when interrupt is disabled */
    void (* _btdm_sleep_enter_phase2)(void);
    void (* _btdm_sleep_exit_phase1)(void);  /* called from ISR */
    void (* _btdm_sleep_exit_phase2)(void);  /* called from ISR */
    void (* _btdm_sleep_exit_phase3)(void);  /* called from task */
    bool (* _coex_bt_wakeup_request)(void);
    void (* _coex_bt_wakeup_request_end)(void);
    int (* _coex_bt_request)(uint32_t event, uint32_t latency, uint32_t duration);
    int (* _coex_bt_release)(uint32_t event);
    int (* _coex_register_bt_cb)(coex_func_cb_t cb);
    uint32_t (* _coex_bb_reset_lock)(void);
    void (* _coex_bb_reset_unlock)(uint32_t restore);
    int (* _coex_schm_register_btdm_callback)(void *callback);
    void (* _coex_schm_status_bit_clear)(uint32_t type, uint32_t status);
    void (* _coex_schm_status_bit_set)(uint32_t type, uint32_t status);
    uint32_t (* _coex_schm_interval_get)(void);
    uint8_t (* _coex_schm_curr_period_get)(void);
    void *(* _coex_schm_curr_phase_get)(void);
    int (* _coex_wifi_channel_get)(uint8_t *primary, uint8_t *secondary);
    int (* _coex_register_wifi_channel_change_callback)(void *cb);
    xt_handler (*_set_isr_l3)(int n, xt_handler f, void *arg);
    void (*_interrupt_l3_disable)(void);
    void (*_interrupt_l3_restore)(void);
    void *(* _customer_queue_create)(uint32_t queue_len, uint32_t item_size);
    int (* _coex_version_get)(unsigned int *major, unsigned int *minor, unsigned int *patch);
    uint32_t _magic;
} esp_osi_funcs_t;

/**
 * @brief Controller config options, depend on config mask.
 *        Config mask indicate which functions enabled, this means
 *        some options or parameters of some functions enabled by config mask.
 */
typedef struct {
    /*
     * Following parameters can be configured runtime, when call esp_bt_controller_init()
     */
    uint16_t controller_task_stack_size;    /*!< Bluetooth controller task stack size */
    uint8_t controller_task_prio;           /*!< Bluetooth controller task priority */
    uint8_t hci_uart_no;                    /*!< If use UART1/2 as HCI IO interface, indicate UART number */
    uint32_t hci_uart_baudrate;             /*!< If use UART1/2 as HCI IO interface, indicate UART baudrate */
    uint8_t scan_duplicate_mode;            /*!< scan duplicate mode */
    uint8_t scan_duplicate_type;            /*!< scan duplicate type */
    uint16_t normal_adv_size;               /*!< Normal adv size for scan duplicate */
    uint16_t mesh_adv_size;                 /*!< Mesh adv size for scan duplicate */
    uint16_t send_adv_reserved_size;        /*!< Controller minimum memory value */
    uint32_t  controller_debug_flag;        /*!< Controller debug log flag */
    uint8_t mode;                           /*!< Controller mode: BR/EDR, BLE or Dual Mode */
    uint8_t ble_max_conn;                   /*!< BLE maximum connection numbers */
    uint8_t bt_max_acl_conn;                /*!< BR/EDR maximum ACL connection numbers */
    uint8_t bt_sco_datapath;                /*!< SCO data path, i.e. HCI or PCM module */
    bool auto_latency;                      /*!< BLE auto latency, used to enhance classic BT performance */
    bool bt_legacy_auth_vs_evt;             /*!< BR/EDR Legacy auth complete event required to  protect from BIAS attack */
    /*
     * Following parameters can not be configured runtime when call esp_bt_controller_init()
     * It will be overwrite with a constant value which in menuconfig or from a macro.
     * So, do not modify the value when esp_bt_controller_init()
     */
    uint8_t bt_max_sync_conn;               /*!< BR/EDR maximum ACL connection numbers. Effective in menuconfig */
    uint8_t ble_sca;                        /*!< BLE low power crystal accuracy index */
    uint8_t pcm_role;                       /*!< PCM role (master & slave)*/
    uint8_t pcm_polar;                      /*!< PCM polar trig (falling clk edge & rising clk edge) */
    bool hli;                               /*!< Using high level interrupt or not */
    uint16_t dup_list_refresh_period;       /*!< Duplicate scan list refresh period */
    uint32_t magic;                         /*!< Magic number */
} esp_bt_controller_config_t;

#define ESP_BT_CONTROLLER_CONFIG_MAGIC_VAL  0x20221207

#define ESP_TASK_BT_CONTROLLER_STACK  (4096)

/**
 * @brief Bluetooth mode for controller enable/disable
 */
typedef enum {
    ESP_BT_MODE_IDLE       = 0x00,   /*!< Bluetooth is not running */
    ESP_BT_MODE_BLE        = 0x01,   /*!< Run BLE mode */
    ESP_BT_MODE_CLASSIC_BT = 0x02,   /*!< Run Classic BT mode */
    ESP_BT_MODE_BTDM       = 0x03,   /*!< Run dual mode */
} esp_bt_mode_t;

/* OSI */
extern int btdm_osi_funcs_register(void *osi_funcs);

/* Initialise and De-initialise */
extern int btdm_controller_init(uint32_t config_mask, esp_bt_controller_config_t *config_opts);
