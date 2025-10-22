/**
 * @file inv_fault_recorder.h
 * @brief Inverter fault recorder module for capturing and reporting fault snapshots
 *
 * This module provides functionality to record inverter fault data snapshots including
 * currents, voltages, and timing information. It supports both local serial reporting
 * and remote web-based reporting.
 *
 * Model File: Model::StateModel::Event::fault_recorder
 * Model Path: C:\AL\INV\Model\SWSYS-INV.qea
 *
 * @date 2025-09-07 20:29
 * @author Allayar Moazami
 */
#ifndef H_INV_FAULT_RECORDER
#define H_INV_FAULT_RECORDER

#include "platform.h"

/** @brief Snapshot feature disabled */
#define NO_SNAP_SHOT 0

/** @brief Bit shift for snapshot mode */
#define SNAP_MODE_SHIFT 8

/** @brief Snapshot feature enabled */
#define SNAP_SHOT_ENABLE 1

/** @brief Snapshot mode active */
#define SNAP_SHOT_MODE 1

/** @brief Bit shift for snapshot status */
#define SNAP_STATUS_SHIFT 15

/** @brief Snapshot state: before fault event */
#define BEFORE_SNAP_SHOT 0

/** @brief Snapshot state: during fault event */
#define DURING_SNAP_SHOT 1

/** @brief Snapshot state: after fault event */
#define AFTER_SNAP_SHOT 2

/** @brief Maximum number of snapshots that can be buffered */
#define SNAP_SHOT_BUFFER_SIZE 100

/**
 * @enum EnuSnapshot
 * @brief Enumeration of snapshot data field indices
 *
 * Defines the indices for accessing different fields within a snapshot data array.
 * Each field represents a specific measurement or status value captured during a fault event.
 */
typedef enum EnuSnapshotEnum
{
    ENU_SNAP_ID = 0,        /**< Snapshot identifier */
    ENU_SNAP_LEN,           /**< Snapshot data length */
    ENU_SNAP_SHOT,          /**< Snapshot status */
    ENU_SNAP_FCODE,         /**< Fault code */
    ENU_SNAP_STATE,         /**< System state at fault time */
    ENU_SNAP_CU_INV1,       /**< Inverter 1 current */
    ENU_SNAP_CU_INV2,       /**< Inverter 2 current */
    ENU_SNAP_CU_G1,         /**< Grid 1 current */
    ENU_SNAP_CU_G2,         /**< Grid 2 current */
    ENU_SNAP_VC1,           /**< Capacitor 1 voltage */
    ENU_SNAP_VC2,           /**< Capacitor 2 voltage */
    ENU_SNAP_VDC_FIL,       /**< Filtered DC voltage */
    ENU_SNAP_STIME,         /**< Timestamp in seconds */
    ENU_SNAP_UTIME,         /**< Timestamp in microseconds */
    ENU_SNAP_DSIZE          /**< Total number of data fields */
} EnuSnapshot;

/**
 * @union UnionMiexType
 * @brief Union for storing 16-bit values as either signed or unsigned
 *
 * Allows the same 16-bit value to be interpreted as either unsigned or signed,
 * depending on the measurement type (e.g., voltages may be signed, states unsigned).
 */
typedef union UnionMiexTypeUnion
{
    u16 U;          /**< Unsigned 16-bit representation */
    int16_t S;      /**< Signed 16-bit representation */
} UnionMiexType;

/**
 * @struct StuNodeData
 * @brief Container for a single snapshot's data fields
 *
 * Holds all measured values for one fault snapshot event.
 */
typedef struct StuNodeDataStruct
{
    UnionMiexType data[ENU_SNAP_DSIZE];     /**< Array of snapshot data fields */
} StuNodeData;

/**
 * @struct StuSnapShotType
 * @brief Circular buffer structure for managing fault snapshots
 *
 * Implements a circular buffer to store multiple fault snapshots with head/tail indexing.
 */
typedef struct StuSnapShotTypeStruct
{
    StuNodeData node[SNAP_SHOT_BUFFER_SIZE];    /**< Array of snapshot data nodes */
    u16 head;                                    /**< Write index (next position to write) */
    u16 tail;                                    /**< Read index (next position to read) */
    u16 id;                                      /**< Snapshot sequence identifier */
} StuSnapShotType;

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
void inv_fault_recorder_set_data(u16 index, u16 invData);

/**
 * @brief Increment snapshot buffer head index
 *
 * Advances the head pointer to prepare for the next snapshot. This should be called
 * after all data fields for the current snapshot have been set.
 *
 * @return 1 if increment was successful, 0 if buffer is full
 */
u8 inv_fault_recorder_inc_idx(void);

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
u8 inv_fault_recorder_status(void);

/**
 * @brief Generate local report with fault snapshot data
 *
 * Transmits the next snapshot in the buffer via the command response interface.
 * If the buffer is empty, sends an "Event Buffer is Empty" message.
 * Automatically advances the tail pointer after successful transmission.
 */
void inv_fault_recorder_report(void);

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
 */
u8 inv_fault_recorder_cmd(char *str);

/**
 * @brief Display help information for fault recorder commands
 *
 * Provides help text for available fault recorder commands via the command interface.
 *
 * @return Always returns 0
 *
 * @note This function is currently a placeholder for future help text implementation
 */
u8 inv_fault_recorder_cmd_help(void);

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
 * @note If buffer is empty, resets head, tail, and increments snapshot ID
 */
void inv_fault_recorder_web_report(u8 *str, u8 *api);

/**
 * @brief Get current snapshot buffer head index
 *
 * Returns the current position of the head pointer in the snapshot buffer.
 * This indicates the next position where new snapshot data will be written.
 *
 * @return Current head index value (0 to SNAP_SHOT_BUFFER_SIZE-1)
 */
u16 inv_fault_recorder_head_report(void);

#endif /* H_FAULT_RECORDER */
