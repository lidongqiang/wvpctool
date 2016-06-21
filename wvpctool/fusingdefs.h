#ifndef _FUSINGDEFS_H_

/*
* Function ID
* This section contains an overview of the main functions used in the fuse scripts.
**/
#define FUS_FUNC_DEFINE_DATA 0x30000001 /*Define a value used in subscripts and for writing values.**/
#define FUS_FUNC_SUBFUNCTION 0x30000002 /*Call a build in sub script function (simple fusing).**/
#define FUS_FUNC_WAIT4_0BITS 0x30000100 /*Wait for 0 bits in register according to bits in mask.**/
#define FUS_FUNC_WAIT4_1BITS 0x30000101 /*Wait for 1 bits in register according to bits in mask.**/
#define FUS_FUNC_WRITE_0BITS 0x30000102 /*Unset the bits in register according to bits in value.**/
#define FUS_FUNC_WRITE_1BITS 0x30000103 /*Set the bits in register according to bits in value.**/
#define FUS_FUNC_WRITE_VALUE 0x30000104 /*Write value into bit mask using indicated value.**/
#define FUS_FUNC_TEST4_0BITS 0x30000105 /*Test for 0 bits in register according to bits in mask.**/
#define FUS_FUNC_TEST4_1BITS 0x30000106 /*Test for 1 bits in register according to bits in mask.**/
#define FUS_FUNC_TEST4_VALUE 0x30000107 /*Test for value in register according to bits in mask.**/
#define FUS_FUNC_REGISTERVAL 0x30000200 /*Read out register value, located at mask.**/
#define FUS_FUNC_ATTACH_POWR 0x30000201 /*Attach the power to the fuse pad.**/
#define FUS_FUNC_REMOVE_POWR 0x30000202 /*Remove power from the fuse pad.**/

/*
*Subscript ID
*The Subscript ID is the build in fuse scripts, and they are invoked with the script function:
*FUS_FUNC_SUBFUNCTION
**/
#define FUS_SUB_PROGRAM_FUSES   0x40000001 /*Activate the program fuses build-in script.**/
#define FUS_SUB_RESET_CONFIGS   0x40000002 /*Reset configuration registers.**/
#define FUS_SUB_CONF_FUSE_LOCK  0x40000100 /*Configure master lock to prevent further fusing.**/
#define FUS_SUB_CONF_CERT_BOOT  0x40000211 /*Configure certificate boot. Need values:FUS_VALUE_SEC_CUSTOMCFGFUS_VALUE_SEC_CBOOTNUMYFUS_VALUE_SEC_CBOOTNUMX**/
#define FUS_SUB_CONF_SECURE_DBG 0x40000212 /*Configure secure debug.**/
#define FUS_SUB_CONF_USIF2BOOT  0x40000213 /*Configure USIF2BOOT.**/
#define FUS_SUB_TEST_FUSE_LOCK  0x40001100 /*Test fusing of master lock.**/
#define FUS_SUB_TEST_CERT_BOOT  0x40001211 /*Test fusing of certificate boot. Need values:FUS_VALUE_SEC_CUSTOMCFGFUS_VALUE_SEC_CBOOTNUMY**/
#define FUS_SUB_TEST_SECURE_DBG 0x40001212 /*Test fusing of secure debug.**/
#define FUS_SUB_TEST_USIF2BOOT  0x40001213 /*Test fusing of USIF2BOOT.#define **/

/*
*Register ID
*The following register identifiers are defined for use in the FUS script definition. These are not used when
*using simple fuse scripts.
**/
#define FUS_REGID_FUSE_CFG      0x10100001 /*Fuse configuration register.**/
#define FUS_REGID_FUSE_STAT     0x10000002 /*Fuse status register (read only).**/
#define FUS_REGID_AUX_CFG       0x10100101 /*AUX configuration register.**/
#define FUS_REGID_AUX           0x10000101 /*AUX real register (read only).**/
#define FUS_REGID_AUX_MISC_CFG  0x10100102 /*AUX_MISC configuration register.**/
#define FUS_REGID_AUX_MISC      0x10000102 /*AUX_MISC real register (read only).**/
#define FUS_REGID_CUS_CFG       0x10100103 /*CUS configuration register.**/
#define FUS_REGID_CUS           0x10000103 /*CUS real register (read only).**/
#define FUS_REGID_SEC1_CFG      0x10100104 /*SEC1 configuration register.**/
#define FUS_REGID_SEC1          0x10000104 /*SEC1 real register (read only).**/

/*
*Result Cause
*The following result causes may be returned as result from calling on of the functions ¡°FUS Script¡± or ¡°FUS
*Attach¡±.
**/
#define FUS_RES_SUCCESS               0x00000000 /*Operation successful.**/
#define FUS_RES_INTERNAL_ERROR        0x00001000 /*Internal error.**/
#define FUS_RES_SIZE_LIMIT            0x00001001 /*Interface size limit protection.**/
#define FUS_RES_INVALID_DATA          0x00001002 /*Some data was invalid.**/
#define FUS_RES_MALLOC_ERROR          0x00001003 /*Memory allocation failed.**/
#define FUS_RES_SCU_IF_ERROR          0x00001004 /*SCU interface error.**/
#define FUS_RES_SCRIPT_TIMEOUT        0x00001020 /*Fuse operation timed out.**/
#define FUS_RES_SCRIPT_PENDING        0x00001021 /*Existing script not finished.**/
#define FUS_RES_SCRIPT_FINISHED       0x00001022 /*No more script to execute.**/
#define FUS_RES_SCRIPT_BAD_SUBSCRIPT  0x00001023 /*Subscript id was not found.**/
#define FUS_RES_SCRIPT_BAD_FUNCTION   0x00001024 /*Function id does not exist.**/
#define FUS_RES_SCRIPT_BAD_REGISTER   0x00001025 /*Register id does not exist.**/
#define FUS_RES_SCRIPT_BAD_REGACTION  0x00001026 /*The resister operation is not allowed.**/
#define FUS_RES_SCRIPT_VALUE_MISSING  0x00001027 /*Required script input value is missing.**/
#define FUS_RES_SCRIPT_VALUE_BADMASK  0x00001028 /*Script value does not fit into bitmask.**/
#define FUS_RES_SCRIPT_VALUE_CONFLICT 0x00001029 /*Script value does not fit existing value.**/
#define FUS_RES_SCRIPT_TEST_FAILED    0x00001030 /*Script test command failed.#define **/


/*
*Result Action
*The following result causes may be returned as result from calling one of the functions ¡°FUS Script¡± or ¡°FUS
*Attach¡±. Whenever the result action is non-zero, the fusing operation must be resumed with a call to the
*¡°FUS Attach¡± command.
**/
#define ZERO                    0x00000000 /*No action defined.**/
#define FUS_FUNC_REGISTERVAL    0x30000200 /*¡°Result Value¡± contains the requested value.**/
#define FUS_FUNC_ATTACH_POWR    0x30000201 /*Attach the power to the fuse pad.**/
#define FUS_FUNC_REMOVE_POWR    0x30000202 /*Remove power from the fuse pad.#define **/


/*
*When doing programming, a script data input buffer must be created. The byte size of the buffer must be
*16 times the number of commands in the script.
**/
#define SCRIPT_ITEM_SIZE 16

#define IDX_FUN 0
#define IDX_RID 1
#define IDX_BMK 2
#define IDX_VLU 3
#define IDX_CNT 4

typedef struct
{
    unsigned int function;
    unsigned int regid;
    unsigned int bitmask;
    unsigned int value;
} T_FUS_SCRIPT;




#endif
