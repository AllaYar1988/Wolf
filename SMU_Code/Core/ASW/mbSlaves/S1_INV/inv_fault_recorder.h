/*
 * ***************************************************
 * Model File: Model::StateModel::Event::fault_recorder
 * Model Path: C:\AL\INV\Model\SWSYS-INV.qea
 *
 * 2025-09-07  - 20:29
 * ***************************************************
 */
#ifndef H_INV_FAULT_RECORDER
#define H_INV_FAULT_RECORDER

#include "platform.h"

#define NO_SNAP_SHOT 0

#define SNAP_MODE_SHIFT 8

#define SNAP_SHOT_ENABLE 1

#define SNAP_SHOT_MODE 1

#define SNAP_STATUS_SHIFT 15

#define BEFORE_SNAP_SHOT 0

#define DURING_SNAP_SHOT 1

#define AFTER_SNAP_SHOT 2

#define SNAP_SHOT_BUFFER_SIZE 100

typedef enum EnuSnapshotEnum
{
    ENU_SNAP_ID = 0, /* Explicit value specification added by code generator */
    ENU_SNAP_LEN,
    ENU_SNAP_SHOT,
    ENU_SNAP_FCODE,
    ENU_SNAP_STATE,
    ENU_SNAP_CU_INV1,
    ENU_SNAP_CU_INV2,
    ENU_SNAP_CU_G1,
    ENU_SNAP_CU_G2,
    ENU_SNAP_VC1,
    ENU_SNAP_VC2,
    ENU_SNAP_VDC_FIL,
    ENU_SNAP_STIME,
    ENU_SNAP_UTIME,
    ENU_SNAP_DSIZE
} EnuSnapshot;

typedef union UnionMiexTypeUnion
{
    u16 U;
    int16_t S;
} UnionMiexType;

typedef struct StuNodeDataStruct
{
    UnionMiexType data[ENU_SNAP_DSIZE];
} StuNodeData;

typedef struct StuSnapShotTypeStruct
{
    StuNodeData node[SNAP_SHOT_BUFFER_SIZE];
    u16 head;
    u16 tail;
    u16 id;
} StuSnapShotType;

void inv_fault_recorder_set_data(u16 index,u16 invData);


u8 inv_fault_recorder_inc_idx(void);


u8 inv_fault_recorder_status(void);

void inv_fault_recorder_report();

/*!
 **************************************************************************************************
 *
 *  @fn         u8 debugCommand( char *str)
 *
 *  @par        This function is for handling debug commands.
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 inv_fault_recorder_cmd( char *str);
/*!
 **************************************************************************************************
 *
 *  @fn         u8 debugHelp( void)
 *
 *  @par        This function is for handling debug commands help.
 *
 *  @param      None.
 *
 *  @return     None.
 *
 *  @par        Design Info
 *              WCET            : Enter Worst Case Execution Time heres
 *              Sync/Async      : sync
 *
 **************************************************************************************************
 */
u8 inv_fault_recorder_cmd_help( void);
/*
 *
 * */
void inv_fault_recorder_web_report(u8 *str,u8 *api);

u16 inv_fault_recorder_head_report(void);

#endif /* H_FAULT_RECORDER */
