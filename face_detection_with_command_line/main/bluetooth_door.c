#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
// #include "bta_api.h"

#include "servo.h"
#include "blink.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_main.h"

enum {
  DCS_IDX_SVC,      // Door Controller Service index
  DCS_IDX_CHAR,     // Door Controller Characteristic index
  DCS_IDX_VAL,      // Door Controller Characteristic Value index
  DCS_IDX_NTF_CFG,  // Door Controller Notifications Configuration index
  DCS_IDX_NB,       // Number of table elements
};

#define GATTS_TABLE_TAG "GATTS_TABLE"

#define DOOR_PROFILE_NUM 	   1
#define DOOR_PROFILE_APP_IDX   0
#define DOOR_CONTROLLER_APP_ID 0x60
#define DEVICE_NAME			   "ESP_DOOR"

#define GATTS_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE        1024
#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

#define ADV_CONFIG_FLAG             (1 << 0)
#define SCAN_RSP_CONFIG_FLAG        (1 << 1)

// static uint8_t adv_config_done       = 0;

static uint16_t connection_id = 0;
static bool connected = false;
static uint16_t door_controller_attribute_handle = 0;

static uint8_t door_controller_service_uuid[16] = {
	0x20, 0xB1, 0x66, 0x12, 0xAD, 0x37, 0x48, 0xE0, 0x8C, 0x3D, 0xBB, 0x9D, 0x50, 0xF6, 0x7B, 0xFE,
};

static uint8_t door_controller_char_uuid[16] = {
	0x30, 0xB1, 0x66, 0x12, 0xAD, 0x37, 0x48, 0xE0, 0x8C, 0x3D, 0xBB, 0x9D, 0x50, 0xF6, 0x7B, 0xFE,
};

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

uint16_t door_controller_handle_table[DCS_IDX_NB];

//GLOBALS for comms
static uint16_t conn_id_global;
static uint16_t handle_global;
static esp_gatt_if_t gatts_if_global;

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
					esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
// static const uint8_t char_prop_read                =  ESP_GATT_CHAR_PROP_BIT_READ;
// static const uint8_t char_prop_write               = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_read_write_notify   = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t door_controller_ccc[2]      = {0x00, 0x00};

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst door_profile_tab[DOOR_PROFILE_NUM] = {
    [DOOR_PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

static esp_ble_adv_data_t door_controller_adv_config = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006,
    .max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(door_controller_service_uuid),
    .p_service_uuid = door_controller_service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t door_controller_adv_params = {
    .adv_int_min         = 0x20,
    .adv_int_max         = 0x40,
    .adv_type            = ADV_TYPE_IND,
    .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
    .channel_map         = ADV_CHNL_ALL,
    .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static const esp_gatts_attr_db_t door_controller_gatt_db[DCS_IDX_NB] =
{
    // Service Declaration
    [DCS_IDX_SVC]                       =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      128, sizeof(door_controller_service_uuid), (uint8_t *)&door_controller_service_uuid}},

    // Characteristic Declaration
    [DCS_IDX_CHAR]            =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

    // Characteristic Value
    [DCS_IDX_VAL]               =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_128, (uint8_t *)&door_controller_char_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_CHAR_VAL_LEN_MAX,0, NULL}},

    // Characteristic - Client Characteristic Configuration Descriptor
    [DCS_IDX_NTF_CFG]           =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
      sizeof(uint16_t),sizeof(door_controller_ccc), (uint8_t *)door_controller_ccc}},
};

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGE(GATTS_TABLE_TAG, "event = %x\n",event);
    switch (event) {
        case ESP_GATTS_REG_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "%s %d\n", __func__, __LINE__);
            esp_ble_gap_set_device_name(DEVICE_NAME);
            ESP_LOGI(GATTS_TABLE_TAG, "%s %d\n", __func__, __LINE__);
            esp_ble_gap_config_adv_data(&door_controller_adv_config);
            ESP_LOGI(GATTS_TABLE_TAG, "%s %d\n", __func__, __LINE__);
			esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(door_controller_gatt_db, gatts_if, DCS_IDX_NB, DOOR_CONTROLLER_APP_ID);
            if (create_attr_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
            }
			break;
		case ESP_GATTS_CREAT_ATTR_TAB_EVT:
        	ESP_LOGI(GATTS_TABLE_TAG, "The number handle =%x\n",param->add_attr_tab.num_handle);
        	if (param->add_attr_tab.status != ESP_GATT_OK){
            	ESP_LOGE(GATTS_TABLE_TAG, "Create attribute table failed, error code=0x%x", param->add_attr_tab.status);
        	}
        	else if (param->add_attr_tab.num_handle != DCS_IDX_NB){
            	ESP_LOGE(GATTS_TABLE_TAG, "Create attribute table abnormally, num_handle (%d) \
                    	doesn't equal to HRS_IDX_NB(%d)", param->add_attr_tab.num_handle, DCS_IDX_NB);
        	}
        	else {
            	memcpy(door_controller_handle_table, param->add_attr_tab.handles, sizeof(door_controller_handle_table));
            	esp_ble_gatts_start_service(door_controller_handle_table[DCS_IDX_SVC]);
        	}
        	break;
		case ESP_GATTS_READ_EVT:
			ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_READ_EVT");
			// TODO: insert code
			break;
		// TODO: connect event, disconnect event, write event
		case ESP_GATTS_DISCONNECT_EVT:
			connected = false;
			ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x", param->disconnect.reason);
      esp_ble_gap_start_advertising(&door_controller_adv_params);
			break;
		case ESP_GATTS_CONNECT_EVT:
			connected = true;
			connection_id = param->connect.conn_id;
			ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONNECT_EVT, conn_id = %d", param->connect.conn_id);
			esp_log_buffer_hex(GATTS_TABLE_TAG, param->connect.remote_bda, 6);
			esp_ble_conn_update_params_t conn_params = {0};
			memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
			/* For the iOS system, please refer to Apple official documents about the BLE connection parameters restrictions. */
			conn_params.latency = 0;
			conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
			conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
			conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
			//start sent the update connection parameters to the peer device.
			esp_ble_gap_update_conn_params(&conn_params);
      esp_ble_gap_stop_advertising();
			break;
		case ESP_GATTS_WRITE_EVT:
      ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_WRITE_EVT reached %d\n", *(param->write.value));

			// TODO: react to the camera telling us to open the door
      if((*param->write.value) == 5)
        app_servo_main();
      else if((*param->write.value) == 6)
      {
        //Flash red led for failure
        app_led_main();
        ESP_LOGI(GATTS_TABLE_TAG, "No mask found. Val: %d ", *(param->write.value));
      }
      else if((*param->write.value) == 7)
      {
        //Flash red led for failure
        app_led_main();
        ESP_LOGI(GATTS_TABLE_TAG, "No face found. Val: %d ", *(param->write.value));
      }
			// if(connection_id == param->write.conn_id && param->write.handle == door_controller_attribute_handle){
      // uint8_t temp_val = 5;
			// esp_ble_gatts_set_attr_value(door_controller_attribute_handle, 1, &temp_val);
      // esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, param->write.handle, 1, &temp_val, false);
      gatts_if_global = gatts_if;
      conn_id_global = param->write.conn_id;
      handle_global = param->write.handle;
			// }
			break;
		case ESP_GATTS_ADD_CHAR_EVT:
			ESP_LOGI(GATTS_TABLE_TAG, "ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d\n",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
			bool uuid_match = true;
			for(int i = 0; i < 16; i++){
				if(uuid_match){
					uuid_match = param->add_char.char_uuid.uuid.uuid128[i] == door_controller_char_uuid[i];
				}
			}
			if(uuid_match){
				door_controller_attribute_handle = param->add_char.attr_handle;
			}
			break;
		case ESP_GATTS_STOP_EVT:
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    case ESP_GATTS_UNREG_EVT:
    case ESP_GATTS_DELETE_EVT:
    default:
        break;

	}
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    ESP_LOGE(GATTS_TABLE_TAG, "GAP_EVT, event %d\n", event);

    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&door_controller_adv_params);
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        //advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TABLE_TAG, "Advertising start failed\n");
        }
        break;
    default:
        break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGI(GATTS_TABLE_TAG, "EVT %d, gatts if %d\n", event, gatts_if);

    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            door_profile_tab[DOOR_PROFILE_APP_IDX].gatts_if = gatts_if;
        } else {
            ESP_LOGI(GATTS_TABLE_TAG, "Reg app failed, app_id %04x, status %d\n",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    do {
        int idx;
        for (idx = 0; idx < DOOR_PROFILE_NUM; idx++) {
            if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
            gatts_if == door_profile_tab[idx].gatts_if) {
                if (door_profile_tab[idx].gatts_cb) {
                    door_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

void send_message(uint8_t val)
{
  ESP_LOGI(GATTS_TABLE_TAG, "sending %d to camera", val);
  esp_ble_gatts_send_indicate(gatts_if_global, conn_id_global, handle_global, 1, &val, false);
}

void app_door_bt_main(){
	/* in order to update the characteristic value, which is the data that both devices read, call
	 esp_ble_gatts_set_attr_value(door_controller_attribute_handle, uint16_t length, const uint8_t *value)*/

    esp_err_t ret;

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed\n", __func__);
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed\n", __func__);
        return;
    }

    ESP_LOGI(GATTS_TABLE_TAG, "%s init bluetooth\n", __func__);
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s init bluetooth failed\n", __func__);
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed\n", __func__);
        return;
    }

    esp_ble_gatts_register_callback(gatts_event_handler);
    esp_ble_gap_register_callback(gap_event_handler);
    esp_ble_gatts_app_register(DOOR_CONTROLLER_APP_ID);

    // const TickType_t xDelay = 30000 / portTICK_PERIOD_MS;
    // vTaskDelay( xDelay );
    // uint8_t temp_val = 9;
    // ESP_LOGI(GATTS_TABLE_TAG, "DELAY DONE");
    // esp_ble_gatts_send_indicate(gatts_if_global, conn_id_global, handle_global, 1, &temp_val, false);

    return;

}
