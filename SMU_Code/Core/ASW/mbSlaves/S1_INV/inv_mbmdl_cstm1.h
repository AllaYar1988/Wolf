/*
 * ***************************************************
 * Model File: Model::StateModel::Modbus::mbmdl_cstm1
 * Model Path: C:\AL\INV\Model\SWSYS-INV.qea
 *
 * 2025-09-07  - 20:20
 * ***************************************************
 */
#ifndef H_INV_MBMDL_CSTM1
#define H_INV_MBMDL_CSTM1



typedef struct StuCstmModel1Struct
{
    f32 data;
    f32 scale;
} StuCstmModel1;

typedef enum EnuMonitoringDataEnum
{
    ENU_MNT_ID = 0, /* Explicit value specification added by code generator */
    ENU_MNT_LEN,
    ENU_MNT_SNAP,
    ENU_MNT_VDC_AVG,
    ENU_MNT_CU_INV1_RMS,
    ENU_MNT_CU_INV2_RMS,
    ENU_MNT_VC1_RMS,
    ENU_MNT_VC2_RMS,
    ENU_MNT_VG1_RMS,
    ENU_MNT_VG2_RMS,
    ENU_MNT_FRQ_INV,
    ENU_MNT_FRQ_GRID,
    ENU_MNT_SSR_STS,
    ENU_MNT_FCODE,
    ENU_MNT_FAULT_TRIG,
    ENU_MNT_FAULT_FLAG,
    ENU_MNT_TEMP1,
    ENU_MNT_TEMP2,
    ENU_MNT_VBAT_FIL,
    ENU_MNT_STATE,
    ENU_MNT_AR_SIZE
} EnuMonitoringData;

typedef enum EnuRefDataEnum
{
    ENU_REF_WRITE_EN = 0, /* Explicit value specification added by code generator */
    ENU_REF_SYS_MODE,
    ENU_REF_PREF_GC,
    ENU_REF_RATE_PREF_GC,
    ENU_REF_Q_STAR,
    ENU_REF_Q_UP_LMT,
    ENU_REF_Q_LW_LMT,
    ENU_REF_VBF,
    ENU_REF_EXT_VM1,
    ENU_REF_EXT_VM2,
    ENU_REF_EXT_WPLL,
    ENU_REF_V_AMP_UPLMT,
    ENU_REF_V_AMP_LWLMT,
    ENU_REF_VDC_START,
    ENU_REF_VDC_STOP,
    ENU_REF_VBAT_CH,
    ENU_REF_I_TH,
    ENU_REF_RMS_OVT,
    ENU_REF_I_TH_RMS,
    ENU_REF_AR_SIZE
} EnuRefData;



#endif /* H_MBMDL_CSTM1 */
