/**
 * @file inv_fault_recorder.c
 * @brief Inverter fault recorder implementation
 *
 * This module implements fault snapshot recording functionality for inverter systems.
 * It captures fault data including currents, voltages, and timing information, and
 * provides both local serial and remote web-based reporting capabilities.
 *
 * The module uses a circular buffer to store up to 100 fault snapshots. Each snapshot
 * contains 11 data fields including fault codes, system state, current measurements,
 * voltage measurements, and timestamps.
 *
 * @date Created on: Oct 13, 2025
 * @author Allayar Moazami
 */
#include "inv_fault_recorder.h"
#include "httpFrame.h"
#include <stdio.h>
#include <string.h>
#include "server.h"

/* ========================================================================
 * Constants
 * ======================================================================== */

/** @brief Size of event buffer for JSON formatted snapshots */
#define EVENT_BUFFER_SIZE 500

/** @brief Size of temporary string buffer for formatting */
#define TEMP_STRING_SIZE 40

/** @brief Snapshot count threshold for local reporting mode */
#define SNAPSHOT_THRESHOLD 98

/* ========================================================================
 * Static Variables
 * ======================================================================== */

/** @brief Global snapshot buffer and state management structure */
static StuSnapShotType gStuSnapshot;

/** @brief Local reporting mode flag (0=remote/web, 1=local/serial) */
static u8 gLocalReport = 0;

/** @brief Internal buffer for formatting event data for transmission */
static u8 iEventBuff[EVENT_BUFFER_SIZE];

/* ========================================================================
 * Static Function Prototypes
 * ======================================================================== */

/**
 * @brief Build JSON header with metadata
 * @param[out] str Output buffer for JSON string
 * @param[in] str_size Size of output buffer
 * @param[in] tm DateTime structure with timestamp information
 */
static void inv_fault_recorder_start_json(char *str, size_t str_size, DateTime tm);

/**
 * @brief Append unsigned 16-bit field to JSON string
 * @param[in,out] str JSON string buffer to append to
 * @param[in] str_size Size of the string buffer
 * @param[in] field_name JSON field name (e.g., "V1", "V2")
 * @param[in] label Human-readable label (e.g., "Fcode", "State")
 * @param[in] value Unsigned 16-bit value to append
 * @param[in] unit Unit of measurement (e.g., "A", "V", "N")
 * @param[in] is_last Flag indicating if this is the last field (1) or not (0)
 */
static void inv_fault_recorder_append_field_u16(char *str, size_t str_size,
                                                  const char *field_name,
                                                  const char *label,
                                                  u16 value,
                                                  const char *unit,
                                                  u8 is_last);

/**
 * @brief Append signed 16-bit field to JSON string
 * @param[in,out] str JSON string buffer to append to
 * @param[in] str_size Size of the string buffer
 * @param[in] field_name JSON field name (e.g., "V3", "V4")
 * @param[in] label Human-readable label (e.g., "Iinv1", "Vc1")
 * @param[in] value Signed 16-bit value to append
 * @param[in] unit Unit of measurement (e.g., "A", "V")
 * @param[in] is_last Flag indicating if this is the last field (1) or not (0)
 */
static void inv_fault_recorder_append_field_s16(char *str, size_t str_size,
                                                  const char *field_name,
                                                  const char *label,
                                                  int16_t value,
                                                  const char *unit,
                                                  u8 is_last);

/**
 * @brief Build complete JSON snapshot data
 * @param[out] str Output buffer for JSON string
 * @param[in] str_size Size of output buffer
 * @param[in] idx Snapshot buffer index to format
 */
static void inv_fault_recorder_build_json(char *str, size_t str_size, u16 idx);

/* ========================================================================
 * Public Function Implementations
 * ======================================================================== */

/**
 * @brief Set data at specified index in the current snapshot
 *
 * Stores a 16-bit value at the specified field index in the current snapshot being recorded.
 * The data is stored at the position indicated by the head index.
 *
 * @param[in] index Data field index (should be one of EnuSnapshot values)
 * @param[in] invData Data value to store (16-bit unsigned)
 *
 * @note This function only stores data if the head index is within buffer bounds
 */
void inv_fault_recorder_set_data(u16 index, u16 invData)
{
    u16 headIdx = gStuSnapshot.head;

    if (headIdx < SNAP_SHOT_BUFFER_SIZE) {
        gStuSnapshot.node[headIdx].data[index].U = invData;
    }
}

/**
 * @brief Increment snapshot buffer head index
 *
 * Advances the head pointer to prepare for the next snapshot. This should be called
 * after all data fields for the current snapshot have been set.
 *
 * @return 1 if increment was successful, 0 if buffer is full
 */
u8 inv_fault_recorder_inc_idx(void)
{
    if (gStuSnapshot.head < SNAP_SHOT_BUFFER_SIZE) {
        gStuSnapshot.head++;
        return 1;
    }
    return 0;
}

/**
 * @brief Get fault recorder status
 *
 * Returns the current operational status of the fault recorder based on the
 * reporting mode (local vs remote) and buffer state.
 *
 * @return Status code:
 *         - 0: Idle (no data to report)
 *         - 1: Remote reporting pending (data available for web transmission)
 *         - 2: Local reporting ready (threshold reached for local output)
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

/* ========================================================================
 * Static Helper Function Implementations
 * ======================================================================== */

/**
 * @brief Append unsigned 16-bit field to JSON string
 *
 * Formats and appends a JSON field with an unsigned 16-bit value to the output string.
 * The field is formatted as: "field_name": "label*value*unit"
 * If this is the last field, closes both the data object and the root JSON object.
 *
 * @param[in,out] str JSON string buffer to append to
 * @param[in] str_size Size of the string buffer
 * @param[in] field_name JSON field name (e.g., "V1", "V2")
 * @param[in] label Human-readable label (e.g., "Fcode", "State")
 * @param[in] value Unsigned 16-bit value to append
 * @param[in] unit Unit of measurement (e.g., "A", "V", "N")
 * @param[in] is_last Flag indicating if this is the last field (1) or not (0)
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

/**
 * @brief Append signed 16-bit field to JSON string
 *
 * Formats and appends a JSON field with a signed 16-bit value to the output string.
 * The field is formatted as: "field_name": "label*value*unit"
 * If this is the last field, closes both the data object and the root JSON object.
 *
 * @param[in,out] str JSON string buffer to append to
 * @param[in] str_size Size of the string buffer
 * @param[in] field_name JSON field name (e.g., "V3", "V4")
 * @param[in] label Human-readable label (e.g., "Iinv1", "Vc1")
 * @param[in] value Signed 16-bit value to append
 * @param[in] unit Unit of measurement (e.g., "A", "V")
 * @param[in] is_last Flag indicating if this is the last field (1) or not (0)
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

/**
 * @brief Build JSON header with metadata
 *
 * Creates the initial JSON structure containing metadata about the fault snapshot
 * including serial number, timestamp, fault ID, and snapshot index.
 *
 * @param[out] str Output buffer for JSON string
 * @param[in] str_size Size of output buffer
 * @param[in] tm DateTime structure with timestamp information
 *
 * @note This function opens the JSON root object and the "data" object.
 *       The caller must add data fields and close the objects.
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

/**
 * @brief Build complete JSON snapshot data
 *
 * Constructs a complete JSON representation of a fault snapshot including all
 * measurement fields (fault code, state, currents, voltages, timestamps).
 * Uses the RTC to get current timestamp and formats all 11 data fields.
 *
 * @param[out] str Output buffer for JSON string
 * @param[in] str_size Size of output buffer
 * @param[in] idx Snapshot buffer index to format
 *
 * @note The output buffer must be large enough to hold the complete JSON structure.
 *       Recommended minimum size is EVENT_BUFFER_SIZE (500 bytes).
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

/* ========================================================================
 * Public Reporting Function Implementations
 * ======================================================================== */

/**
 * @brief Generate web report with fault snapshot data
 *
 * Prepares a JSON-formatted snapshot for transmission to a web API endpoint.
 * Constructs both the JSON payload and the API URL. Automatically advances
 * the tail pointer after successful report generation.
 *
 * @param[out] str Output buffer for JSON data (must be at least EVENT_BUFFER_SIZE bytes)
 * @param[out] api Output buffer for API URL (must be at least 100 bytes)
 *
 * @note If buffer is empty, resets head, tail, and increments snapshot ID.
 *       The API URL is constructed from the server base URL plus "/api/send-event".
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

/**
 * @brief Generate local report with fault snapshot data
 *
 * Transmits the next snapshot in the buffer via the command response interface.
 * If the buffer is empty, sends an "Event Buffer is Empty" message.
 * Automatically advances the tail pointer after successful transmission.
 *
 * @note This function uses the internal iEventBuff buffer for JSON formatting.
 *       When the buffer becomes empty, it resets head/tail pointers and increments the ID.
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

/**
 * @brief Get current snapshot buffer head index
 *
 * Returns the current position of the head pointer in the snapshot buffer.
 * This indicates the next position where new snapshot data will be written.
 *
 * @return Current head index value (0 to SNAP_SHOT_BUFFER_SIZE-1)
 */
u16 inv_fault_recorder_head_report(void)
{
    return gStuSnapshot.head;
}

/* ========================================================================
 * Command Interface Function Implementations
 * ======================================================================== */

/**
 * @brief Process fault recorder debug commands
 *
 * Handles command-line interface commands for controlling the fault recorder.
 * Recognizes "local" and "remote" commands to switch reporting modes.
 *
 * @param[in] str Command string to process
 *
 * @return Command recognition status:
 *         - 0: Command was recognized and processed
 *         - 1: Command was not recognized
 *
 * @par Supported commands:
 *      - "local": Enable local event log reporting via serial interface
 *      - "remote": Enable remote event log reporting to web server
 *
 * @note Uses substring matching, so "local" will match any string containing "local"
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

/**
 * @brief Display help information for fault recorder commands
 *
 * Provides help text for available fault recorder commands via the command interface.
 *
 * @return Always returns 0
 *
 * @todo Implement help text display with command descriptions and usage examples
 *
 * @note This function is currently a placeholder for future help text implementation
 */
u8 inv_fault_recorder_cmd_help(void)
{
    /* TODO: Implement help text display */
    return 0;
}
