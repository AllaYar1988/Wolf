/*
 * inv_fault_recorder.c
 *
 *  Created on: Oct 13, 2025
 *      Author: Allayar Moazami
 */
#include "inv_fault_recorder.h"
#include "httpFrame.h"
#include <stdio.h>
#include <string.h>
#include "server.h"

/* Constants */
#define EVENT_BUFFER_SIZE 500
#define TEMP_STRING_SIZE 40
#define SNAPSHOT_THRESHOLD 98

/* Static variables */
static StuSnapShotType gStuSnapshot;
static u8 gLocalReport = 0;
static u8 iEventBuff[EVENT_BUFFER_SIZE];

/* Static function prototypes */
static void inv_fault_recorder_start_json(char *str, size_t str_size, DateTime tm);
static void inv_fault_recorder_append_field_u16(char *str, size_t str_size,
                                                  const char *field_name,
                                                  const char *label,
                                                  u16 value,
                                                  const char *unit,
                                                  u8 is_last);
static void inv_fault_recorder_append_field_s16(char *str, size_t str_size,
                                                  const char *field_name,
                                                  const char *label,
                                                  int16_t value,
                                                  const char *unit,
                                                  u8 is_last);
static void inv_fault_recorder_build_json(char *str, size_t str_size, u16 idx);
/*
 * @brief Set data at specified index in the current snapshot
 * @param index Data field index
 * @param invData Data value to store
 */
void inv_fault_recorder_set_data(u16 index, u16 invData)
{
    u16 headIdx = gStuSnapshot.head;

    if (headIdx < SNAP_SHOT_BUFFER_SIZE) {
        gStuSnapshot.node[headIdx].data[index].U = invData;
    }
}

/*
 * @brief Increment snapshot buffer index
 * @return 1 if successful, 0 if buffer is full
 */
u8 inv_fault_recorder_inc_idx(void)
{
    if (gStuSnapshot.head < SNAP_SHOT_BUFFER_SIZE) {
        gStuSnapshot.head++;
        return 1;
    }
    return 0;
}

/*
 * @brief Get fault recorder status
 * @return 0=idle, 1=remote reporting pending, 2=local reporting ready
 */
u8 inv_fault_recorder_status(void)
{
    if (gLocalReport == 0) {
        /* Remote reporting mode */
        if (gStuSnapshot.head != 0) {
            return 1;
        }
    } else {
        /* Local reporting mode */
        if (gStuSnapshot.head >= SNAPSHOT_THRESHOLD) {
            return 2;
        }
    }
    return 0;
}

/*
 * @brief Append unsigned 16-bit field to JSON string
 */
static void inv_fault_recorder_append_field_u16(char *str, size_t str_size,
                                                  const char *field_name,
                                                  const char *label,
                                                  u16 value,
                                                  const char *unit,
                                                  u8 is_last)
{
    char tempStr[TEMP_STRING_SIZE];

    if (is_last) {
        snprintf(tempStr, sizeof(tempStr), "\"%s\": \"%s*%u*%s\"\r}\r}",
                 field_name, label, value, unit);
    } else {
        snprintf(tempStr, sizeof(tempStr), "\"%s\": \"%s*%u*%s\",\r",
                 field_name, label, value, unit);
    }

    strncat(str, tempStr, str_size - strlen(str) - 1);
}

/*
 * @brief Append signed 16-bit field to JSON string
 */
static void inv_fault_recorder_append_field_s16(char *str, size_t str_size,
                                                  const char *field_name,
                                                  const char *label,
                                                  int16_t value,
                                                  const char *unit,
                                                  u8 is_last)
{
    char tempStr[TEMP_STRING_SIZE];

    if (is_last) {
        snprintf(tempStr, sizeof(tempStr), "\"%s\": \"%s*%d*%s\"\r}\r}",
                 field_name, label, value, unit);
    } else {
        snprintf(tempStr, sizeof(tempStr), "\"%s\": \"%s*%d*%s\",\r",
                 field_name, label, value, unit);
    }

    strncat(str, tempStr, str_size - strlen(str) - 1);
}

/*
 * @brief Build JSON header with metadata
 */
static void inv_fault_recorder_start_json(char *str, size_t str_size, DateTime tm)
{
    snprintf(str, str_size,
             "{\r\"serial_number\":\"%s\",\r"
             "\"date\":\"%04d-%02d-%02d\",\r"
             "\"time\":\"%02d:%02d:%02d\",\r"
             "\"fault\":\"%d\",\r"
             "\"index\":\"%d\",\r"
             "\"data\":{\r",
             _serialN, tm.year, tm.month, tm.day,
             tm.hour, tm.minute, tm.second,
             gStuSnapshot.id + 1, gStuSnapshot.tail + 1);
}

/*
 * @brief Build complete JSON snapshot data
 * @param str Output buffer for JSON string
 * @param str_size Size of output buffer
 * @param idx Snapshot buffer index
 */
static void inv_fault_recorder_build_json(char *str, size_t str_size, u16 idx)
{
    _rtcFunctionRead(0);
    inv_fault_recorder_start_json(str, str_size, urtc);

    /* Add all sensor data fields */
    inv_fault_recorder_append_field_u16(str, str_size, "V1", "Fcode",
                                        gStuSnapshot.node[idx].data[ENU_SNAP_FCODE].U, "N", 0);
    inv_fault_recorder_append_field_u16(str, str_size, "V2", "State",
                                        gStuSnapshot.node[idx].data[ENU_SNAP_STATE].U, "N", 0);
    inv_fault_recorder_append_field_s16(str, str_size, "V3", "Iinv1",
                                        gStuSnapshot.node[idx].data[ENU_SNAP_CU_INV1].S, "A", 0);
    inv_fault_recorder_append_field_s16(str, str_size, "V4", "Iinv2",
                                        gStuSnapshot.node[idx].data[ENU_SNAP_CU_INV2].S, "A", 0);
    inv_fault_recorder_append_field_s16(str, str_size, "V5", "Ig1",
                                        gStuSnapshot.node[idx].data[ENU_SNAP_CU_G1].S, "A", 0);
    inv_fault_recorder_append_field_s16(str, str_size, "V6", "Ig2",
                                        gStuSnapshot.node[idx].data[ENU_SNAP_CU_G2].S, "A", 0);
    inv_fault_recorder_append_field_s16(str, str_size, "V7", "Vc1",
                                        gStuSnapshot.node[idx].data[ENU_SNAP_VC1].S, "V", 0);
    inv_fault_recorder_append_field_s16(str, str_size, "V8", "Vc2",
                                        gStuSnapshot.node[idx].data[ENU_SNAP_VC2].S, "V", 0);
    inv_fault_recorder_append_field_s16(str, str_size, "V9", "VDC_FIL",
                                        gStuSnapshot.node[idx].data[ENU_SNAP_VDC_FIL].S, "V", 0);
    inv_fault_recorder_append_field_u16(str, str_size, "V10", "T_Sec",
                                        gStuSnapshot.node[idx].data[ENU_SNAP_STIME].U, "S", 0);
    inv_fault_recorder_append_field_u16(str, str_size, "V11", "T_uSec",
                                        gStuSnapshot.node[idx].data[ENU_SNAP_UTIME].U, "uS", 1);
}

/*
 * @brief Generate web report with fault snapshot data
 * @param str Output buffer for JSON data
 * @param api Output buffer for API URL
 */
void inv_fault_recorder_web_report(u8 *str, u8 *api)
{
    char tempStr[TEMP_STRING_SIZE];
    u16 tailIdx = gStuSnapshot.tail;

    /* Build API URL */
    server_return_url((u8 *)tempStr);
    snprintf((char *)api, 100, "%s/api/send-event", tempStr);

    if (gStuSnapshot.head != gStuSnapshot.tail) {
        /* Build JSON for the current snapshot */
        inv_fault_recorder_build_json((char *)str, EVENT_BUFFER_SIZE, tailIdx);
        gStuSnapshot.tail++;
    } else {
        /* Reset buffer when empty */
        gStuSnapshot.head = 0;
        gStuSnapshot.tail = 0;
        gStuSnapshot.id++;
    }
}

/*
 * @brief Generate local report with fault snapshot data
 * Transmits snapshot data via command response interface
 */
void inv_fault_recorder_report(void)
{
    u16 tailIdx = gStuSnapshot.tail;

    if (gStuSnapshot.head != gStuSnapshot.tail) {
        /* Build JSON for the current snapshot */
        inv_fault_recorder_build_json((char *)iEventBuff, EVENT_BUFFER_SIZE, tailIdx);
        TransmitCMDResponse(iEventBuff);
        gStuSnapshot.tail++;
    } else {
        /* Notify user that buffer is empty */
        TransmitCMDResponse("\r>Event Buffer is Empty\r");
        gStuSnapshot.head = 0;
        gStuSnapshot.tail = 0;
        gStuSnapshot.id++;
    }
}

/*
 * @brief Get current snapshot buffer head index
 * @return Current head index value
 */
u16 inv_fault_recorder_head_report(void)
{
    return gStuSnapshot.head;
}




/*
 * @brief Process fault recorder debug commands
 * @param str Command string to process
 * @return 0 if command was recognized and processed, 1 otherwise
 *
 * Supported commands:
 *   - "local": Enable local event log reporting
 *   - "remote": Enable remote event log reporting to web
 */
u8 inv_fault_recorder_cmd(char *str)
{
    u8 returnValue = 1;

    if (strstr((char *)str, "local")) {
        TransmitCMDResponse("\r>Enable Sending Event Log Locally\r");
        gLocalReport = 1;
        returnValue = 0;
    } else if (strstr((char *)str, "remote")) {
        TransmitCMDResponse("\r>Enable Sending Event Log to Web\r");
        gLocalReport = 0;
        returnValue = 0;
    }

    return returnValue;
}

/*
 * @brief Display help information for fault recorder commands
 * @return Always returns 0
 *
 * Note: This function is currently a placeholder for future help text implementation
 */
u8 inv_fault_recorder_cmd_help(void)
{
    /* TODO: Implement help text display */
    return 0;
}
