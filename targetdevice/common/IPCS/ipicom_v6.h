// BEGIN CCCT SECTION
/* -------------------------------------------------------------------------
 Copyright (C) 2013-2015 Intel Mobile Communications GmbH

 Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
 $File name:  /msw_tools/calibration_rf/src/Core/Engine/inc/ipicom_v6.h $
 $CC-Version: .../sunesimX_xmm_cal_tool_14_40/2 $
 $Date:       2014-09-26    11:39:50 UTC $
 ----------------------------------------------------------------------
 by CCCT (0.12h)
 ---------------------------------------------------------------------- */
// END CCCT SECTION

#ifndef __IPICOM_V6_H__
#define __IPICOM_V6_H__
#define TOOL_ENABLE_ON 1
#define TOOL_SIGN_ON   1
#include <string>
#include <map>
#include <vector>
#include "..\inc\debug.h"
//#include "..\Common.h"
//#include "..\Log.h"
#define LOG(A)
#define LOGER(a) if(m_pLogger) m_pLogger->PrintMSGA a
//LDEGMSGW(DEBUG_TARGET, (a))
/*if(m_pLogger)m_pLogger->Log(a) **/

/* IPICOM Small Adhoc Path based Network (SAPN) Address */
typedef struct
{
	unsigned int route_id;
	unsigned char  port;
} T_IPICOM_SAPN_ADDRESS;

typedef struct
{
	unsigned int datalink_id;
}T_IPICOM_HOP_ADDRESS;

typedef struct
{
	unsigned int datalink_id;
}T_IPICOM_AT_ADDRESS;

/*! Socket address (currently only SAPN) */
typedef union
{
	T_IPICOM_SAPN_ADDRESS  sapn;
	T_IPICOM_HOP_ADDRESS hop;
	T_IPICOM_AT_ADDRESS at;
	//T_IPICOM_NETWORK_ADDRESS naddr; //to be replaced by SAPN routes. Will be depricated.
	//FFS
	//T_IPICOM_IP_ADDRESS;
} T_IPICOM_SOCK_ADDR_IN;


/* Socket domains (currently only SAPN) */
typedef enum
{
	PF_IPICOM, /* based protocol */
	//PF_IP      /* IP based protocol */
} T_IPICOM_SOCKET_DOMAIN;

/* Types of socket data types */
typedef enum
{
	IPICOM_SOCKET_TYPE_SEQPACKET = 0,  // send and recv datagrams in connected mode, with flow control
	IPICOM_SOCKET_TYPE_DGRAM = 1,      // send and recv datagrams in connectionless mode, without flow control
	IPICOM_SOCKET_TYPE_BYTESTREAM = 2, // send and recv bytestream

	/* Future (FFS)*/
} T_IPICOM_SOCKET_TYPE;

/*! Protocol used on thesocket domain for
*/
typedef enum
{
	IPICOM_DEFAULT_PROTO = 0,
	IPICOM_SAPN_PROTO,  /* Reliable path based protocol (no packet loss, packets delivered in order) */
	IPICOM_HOP_PROTO,   /* Binary Hop protocol which works across a single datalinks. Can be used with DGRAM type sockets.*/
	IPICOM_AT_PROTO,    /* Send and receive AT commands (terminated by \r). Can be used with DGRAM type sockets. */
} T_IPICOM_NETWORK_PROTOCOL;


/*! Socket related flags
*/
typedef unsigned int T_IPICOM_SOCKET_SEND_FLAGS;
/* TODO: Define flag values */


/*! IPICOM socket handle
*/
typedef int T_IPICOM_SH;

/*! Invalid socket handle */
#define IPICOM_SOCKET_INVALID_HANDLE ((T_IPICOM_SH)-1)

/*! IPICOM datalink handle
*/
typedef int T_IPICOM_DLH;
#define IPICOM_DL_INVALID_ID ((T_IPICOM_DLH)-1)

/*! IPICOM Route handle
*/
typedef int T_IPICOM_SAPN_RH;
#define IPICOM_SAPN_ROUTE_INVALID_HANDLE ((T_IPICOM_SAPN_RH)-1)
#define IPICOM_SAPN_ROUTE_ANY       0xFFFF /* used for binding server sockets to any route  */
#define IPICOM_SAPN_ROUTE_LOCALROUTE 0xc000      /* used for binding server sockets to local host */

/** Enumeration of different information types that can be read from a remote node */
typedef enum
{
	IPICOM_SAPN_ROUTE_INFO_REMOTE_NODENAME = 0x1, /** Name of Remote node*/
	IPICOM_SAPN_ROUTE_INFO_REMOTE_SIGNAGES = 0x2, /** Signages at remote node*/
	IPICOM_SAPN_ROUTE_INFO_REMOTE_PORTNAMES = 0x4, /** Portnames at remote node*/
	IPICOM_SAPN_ROUTE_INFO_ALL = 0x7 /**Read all info types*/
}T_IPICOM_SAPN_ROUTE_INFO;

/*! IPICOM error codes
*/
typedef enum
{
	IPICOM_SUCCESS = 0,
	IPICOM_FAILURE = -1,
	IPICOM_TIMEOUT = -2,
	IPICOM_INVALID_PARAM = -3,
	IPICOM_SEND_Q_FULL = -4,
	IPICOM_OUT_OF_MEMORY = -5,
	IPICOM_RESPONSE_NOTIF = -6,
	IPICOM_SOCKET_ERR_INVALID = -7,
	IPICOM_SOCKET_ERR_PORT_IN_USE = -8,
	IPICOM_SOCKET_INVALID_PARAM = -9,
	IPICOM_SOCKET_WOULD_BLOCK = -10,    /* indicates for a non-blocking socket that it would have blocked if the socket was blocking */
	IPICOM_SEND_BLOCK_LIMIT_REACHED = -11,
	IPICOM_RECV_Q_EMPTY = -12,          /* indicates that receive queue is empty */
} T_IPICOM_ERROR;

typedef enum  {
	IPICOM_TRACE_SEVERITY_DEBUG = -1, /** Debug level. Allows debug, info, warning and error traces */
	IPICOM_TRACE_SEVERITY_INFO = 0, /** information level. Default severity . Allows info, warning and error traces*/
	IPICOM_TRACE_SEVERITY_WARNING = 1, /** warning level. Allows warning and error traces*/
	IPICOM_TRACE_SEVERITY_ERROR = 2 /** error level. Allows only error traces*/
} T_IPICOM_TRACE_SEVERITY;

/*! IPICOM version information types*/
typedef enum
{
	IPICOM_VERSION_MAJOR, /** Major number */
	IPICOM_VERSION_MINOR, /** Minor Number */
	IPICOM_VERSION_REVISION /**Revision number */
}T_IPICOM_VERSION_INFO;


/*! Indications which are provided by IPICOM on the non-blocking socket callback function on the specified events.
*/
typedef enum
{
	IPICOM_SOCKET_CONNECT_IND,      /* Connection established to listening socket. */
	IPICOM_SOCKET_CONNECT_CNF,      /* Connection req confirmed by server. */
	IPICOM_SOCKET_DISCONNECT_IND,   /* Connection to socket disconnected. The socket handle passed is invalid and cannot be used anymore. */
	IPICOM_SOCKET_CLOSE_IND,        /* Socket is closed. */
	IPICOM_SOCKET_UNBIND_IND,       /* Socket got unbound because the route or datalink was closed. */
	IPICOM_SOCKET_RECV_IND          /* Data is ready in receive queue. */
} T_IPICOM_SOCKET_CBF_IND_TYPE;

/*! Indications which are provided by IPICOM on the PLP callback function on the specified events.
*/
typedef enum
{
	IPICOM_PLP_CONNECT_IND,      /* Connection established to socket. */
	IPICOM_PLP_DISCONNECT_IND,   /* Connection to socket disconnected. The handle passed is invalid and cannot be used anymore. */
	IPICOM_PLP_RECV_IND          /* Data is ready in receive queue. */
} T_IPICOM_PLP_CBF_IND_TYPE;

typedef enum
{
	IPICOM_DATALINK_OPEN_IND,    /* Connection established to socket. */
	IPICOM_DATALINK_CLOSE_IND,   /* Connection to socket disconnected. The handle passed is invalid and cannot be used anymore. */
	IPICOM_DATALINK_SAPN_MOUNT_IND,  /* datalink is now part of sapn network*/
	IPICOM_DATALINK_SAPN_UNMOUNT_IND /* datalink is no longer a part of sapn network */
} T_IPICOM_DATALINK_CBF_IND_TYPE;

/*! Presentation Layer Protocol defines - extends the T_IPICOM_NETWORK_PROTOCOL_TYPE enumeration */
typedef enum
{
	PLP_DEFAULT,   /* Default presentation layer protocol on a socket*/
	PLP_GTI,       /* GTI protocol "gti" */
	PLP_ATHASH,    /* AT# protocol "athash" */
	PLP_WCSBIN,
	/***/
	PLP_UNDEFINED = 0xff
} T_IPICOM_PLP_ID;

/*! Socket options which could be set.
*/
typedef enum
{
	IPICOM_SOCK_OPT_TIMEOUT      /** Option to set socket timeout interval in ms. */
} T_IPICOM_SOCKET_OPT;

/*! Socket option level.
*/
typedef enum
{
	IPICOM_SOCK_OPT_LVL_SESSION      /** Session level. */
} T_IPICOM_SOCKET_OPT_LEVEL;

/*!
\brief Socket owner callback prototype.
\param[in] shdl Socket handle
\param[in] cb_socket_ind Socket indication type.
\param[in] handshake_id Arbitrary user value which was registered for this session. Useful for setting "context" if e.g. the same cb is used for multiple sockets/sessions.
\return None
*/
typedef void(*T_IPICOM_SOCKET_IND_CBF)(T_IPICOM_SH shdl, T_IPICOM_SOCKET_CBF_IND_TYPE ind_type, unsigned int user_value);

/*!
\brief Socket presentation layer callback prototype which is used to indicate events from IPICOM towards the PLP.
\param[in] shdl Socket handle
\param[in] presentation layer socket indication type.
\param[in] handshake_id Arbitrary user value which was registered for this PLP on the session. Useful for setting "context" if e.g. the same cb is used for multiple sockets/sessions.
\return None
*/
typedef void(*T_IPICOM_SOCKET_PLP_IND_CBF)(T_IPICOM_SH shdl, T_IPICOM_PLP_CBF_IND_TYPE ind_type, unsigned int user_value);

/*!
\brief Datalink callbacks prototype which is used to indicate events from IPICOM concerning datalinks.
\param[in] dlid Datalink handle
\param[in] ind_type
*/
typedef void(*T_IPICOM_DATALINK_IND_CBF)(T_IPICOM_DLH dlid, T_IPICOM_DATALINK_CBF_IND_TYPE ind_type);

//enum  ipicom_trace_severity_type{
//  TRACE_DEBUG=-1, /** Debug information */
//  TRACE_INFO=0, /** information. Default severity */
//  TRACE_WARN=1, /** warning */
//  TRACE_ERROR=2 /** error */
//} ;

/**
 * @brief Check if function pointer is loaded from DLL
 *
 * If the function pointer is zero, the function does not exist in the DWDIO.dll
 * This is printed to the log and error (0) is returned
 */
/*
#define CHECKPTR(a) \
if (!isloaded || !a) \
{ \
	E_LOG->TimeLine(eEngine, eLogTargetComm, \
	"Function pointer in %s is NULL. Should be initialised by ipicom_v6_lib.", __FUNCTION__); \
	return false; \
}
*/
#define GTI_RESPONSE_SIZE 5000
#define GTI_SIZE 1000
#define BUF_SIZE 256
#define DEFAULT_TIMEOUT 2500

class CDynamicLibrary
{
private:

	std::string name;
	HMODULE h_lib;
	int detach;

public:

	CDynamicLibrary(std::string name, int detach = 1);
	CDynamicLibrary();
	~CDynamicLibrary();

	int       load();
	int       load(std::string);
	void      unload();
	int       is_loaded();
	void*     get_function(std::string name);
};

class CIPICOMLibV6
{
private:

	char*(*pIPICOM_GetLastError)(void);
	bool(*pIPICOM_GetDllVersion)(char *dll_version);
	int(*pIPICOM_get_version)(T_IPICOM_VERSION_INFO info_type, unsigned int *p_value);

	int(*pIPICOM_socket)(int domain, int type, int protocol);
	T_IPICOM_ERROR(*pIPICOM_socket_set_nonblocking)(T_IPICOM_SH shdl, T_IPICOM_SOCKET_IND_CBF cb_socket_ind);
	T_IPICOM_ERROR(*pIPICOM_socket_bind)(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *bind_addr, unsigned int addrlen);
	T_IPICOM_ERROR(*pIPICOM_socket_close)(T_IPICOM_SH shdl);
	T_IPICOM_ERROR(*pIPICOM_socket_connect)(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *addr, unsigned int addrlen);
	T_IPICOM_ERROR(*pIPICOM_socket_listen)(T_IPICOM_SH shdl, unsigned char max_pending);
	int(*pIPICOM_socket_accept)(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *cli_addr, unsigned int *addrlen);
	int(*pIPICOM_send)(int handle, void *p_data, unsigned int len);
	int(*pIPICOM_recv)(int handle, void *p_data, unsigned int len);
	int(*pIPICOM_socket_sendto)(T_IPICOM_SH shdl, void *p_data, unsigned int len, T_IPICOM_SOCKET_SEND_FLAGS flags, T_IPICOM_SOCK_ADDR_IN *p_addr, unsigned int *addrlen);
	int(*pIPICOM_socket_recv_ref)(T_IPICOM_SH shdl, void ** const p_data);
	T_IPICOM_ERROR(*pIPICOM_socket_getsockname)(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *p_adr, unsigned int *p_addr_len);
	T_IPICOM_ERROR(*pIPICOM_socket_set_usrval)(T_IPICOM_SH shdl, unsigned int usr_val);
	int(*pIPICOM_socket_add_plp)(T_IPICOM_SH shdl, T_IPICOM_PLP_ID plp_id, T_IPICOM_SOCKET_PLP_IND_CBF socket_ind_cbf);
	T_IPICOM_ERROR(*pIPICOM_register_plp)(T_IPICOM_PLP_ID plp_id, T_IPICOM_SOCKET_PLP_IND_CBF protocol_ind_cbf);
	int(*pIPICOM_datalink_open)(const char *connection_name, const char *connection_option, bool mountable);
	T_IPICOM_ERROR(*pIPICOM_datalink_options)(T_IPICOM_DLH dl_id, const char * protocol_description);
	T_IPICOM_ERROR(*pIPICOM_datalink_close)(T_IPICOM_DLH dl_id);
	T_IPICOM_ERROR(*pIPICOM_datalink_callback_register)(T_IPICOM_DATALINK_IND_CBF cbf);
	T_IPICOM_ERROR(*pIPICOM_datalink_set_transparent)(T_IPICOM_DLH dl_id);
	T_IPICOM_ERROR(*pIPICOM_sapn_set_node_name)(const char * node_name);
	T_IPICOM_SAPN_RH(*pIPICOM_sapn_mount)(T_IPICOM_DLH dl_id);
	T_IPICOM_ERROR(*pIPICOM_sapn_unmount)(T_IPICOM_DLH dl_hdl);
	T_IPICOM_SAPN_RH(*pIPICOM_sapn_get_route)(const char *path);
	T_IPICOM_ERROR(*pIPICOM_socket_set_opt)(T_IPICOM_SH shdl, T_IPICOM_SOCKET_OPT_LEVEL level, T_IPICOM_SOCKET_OPT opt, const void* p_opt_val, unsigned int opt_len);

	T_IPICOM_ERROR(*pIPICOM_sapn_add_signage) (T_IPICOM_SAPN_RH route_hdl, const char* signage);
	T_IPICOM_ERROR(*pIPICOM_sapn_get_addr_info) (const char* path, T_IPICOM_SAPN_ADDRESS* p_sapn_addr, unsigned int timeout_ms);
	T_IPICOM_ERROR(*pIPICOM_sapn_route_read_info)(T_IPICOM_SAPN_RH route_hdl, char * buffer, unsigned int buf_len, unsigned int flags);
	int(*pIPICOM_start_trace)(void);
	void(*pIPICOM_stop_trace)(void);
	void(*pIPICOM_trace_setSeverityLevel)(int severity);

	void(*pIPICOM_trace_setGroupFilter)(char *group);
	void(*pIPICOM_trace)(int severity, const char *group, const char *trace, ...);
	void(*pIPICOM_shutdown)();

	void* get_function(std::string name);
	const char*	get_last_error(int);

	CDynamicLibrary hModule;

	std::string dll_location;
	static int fcount, isloaded;
    static CIPICOMLibV6* inst;

	CIPICOMLibV6();
public:

	~CIPICOMLibV6();
	// Methods
	int Load(std::string);
	int Isloaded();
	int Version();
	static CIPICOMLibV6* Inst(std::string = "");
	static void Destroy();

	int socket(int domain, int type, int protocol);
	int socket_set_nonblocking(T_IPICOM_SH shdl, T_IPICOM_SOCKET_IND_CBF cb_socket_ind);
	int socket_bind(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *bind_addr, unsigned int addrlen);
	int socket_close(T_IPICOM_SH shdl);
	int socket_connect(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *addr, unsigned int addrlen);
	int socket_listen(T_IPICOM_SH shdl, unsigned char max_pending);
	int socket_accept(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *cli_addr, unsigned int *addrlen);
	int send(int handle, std::string);
	int recv(int handle, std::string&);
	int socket_sendto(T_IPICOM_SH shdl, std::string, T_IPICOM_SOCKET_SEND_FLAGS flags, T_IPICOM_SOCK_ADDR_IN *p_addr, unsigned int *addrlen);
	int socket_recv_ref(T_IPICOM_SH shdl, std::string&);
	int socket_getsockname(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *p_adr, unsigned int *p_addr_len);
	int socket_set_usrval(T_IPICOM_SH shdl, unsigned int usr_val);
	int socket_add_plp(T_IPICOM_SH shdl, T_IPICOM_PLP_ID plp_id, T_IPICOM_SOCKET_PLP_IND_CBF socket_ind_cbf);
	int register_plp(T_IPICOM_PLP_ID plp_id, T_IPICOM_SOCKET_PLP_IND_CBF protocol_ind_cbf);
	int datalink_open(const char *connection_name, const char *connection_option, bool mountable);
	int datalink_options(T_IPICOM_DLH dl_id, const char * protocol_description);
	int datalink_close(T_IPICOM_DLH dl_id);
	int datalink_callback_register(T_IPICOM_DATALINK_IND_CBF cbf);
	int datalink_set_transparent(T_IPICOM_DLH dl_id);
	int sapn_set_node_name(const char * node_name);
	T_IPICOM_SAPN_RH sapn_mount(T_IPICOM_DLH dl_id);
	int sapn_unmount(T_IPICOM_DLH dl_hdl);
	T_IPICOM_SAPN_RH sapn_get_route(const char *path);
	int socket_set_opt(T_IPICOM_SH shdl, T_IPICOM_SOCKET_OPT_LEVEL level, T_IPICOM_SOCKET_OPT opt, const void* p_opt_val, unsigned int opt_len);
	int sapn_route_read_info(T_IPICOM_SAPN_RH route_hdl, char * buffer, unsigned int buf_len, unsigned int flags);
	int shutdown();
	int  start_trace(void);
	void  stop_trace(void);
	int sapn_add_signage(T_IPICOM_SAPN_RH route_hdl, std::string signage);
	int sapn_get_addr_info(std::string path, T_IPICOM_SAPN_ADDRESS& sapn_addr);
};

enum channel_type
{
	default_channel,
	shared_channel,
	normal_channel,
	pipe_channel
};

class CIPICOMConnectionV6
{
public:
	CIPICOMConnectionV6(CLogger *p = NULL);
	virtual ~CIPICOMConnectionV6();

    bool    open(int nPort,const char * node_name = "snwtool");
	bool close();
    bool close_and_unload();
	int send(std::string);
	int send(std::string, int& nof_cmd, int& tx_id, channel_type ch);
	int receive(std::string&, int tx_id, int nof_exp_resp, int expect_unsolicited, channel_type ch);
	bool query(std::string in, std::string& out, int expect_unsolicited = 0);
    bool query(std::string input, std::string& output, int expect_unsolicited,int retry);
	int timeout(int);
	bool configure(bool bGTI);
    bool configure(int route);
    bool reconfigure(int route);
	int cleanup();
	bool check(bool bGTIPort);
    bool check_services_on(std::vector<std::string> &nodes,int retry,int delay);
	int start_trace();
	int stop_trace();
	int set_channel(channel_type);
	channel_type get_channel();
	int enumerate_ports(std::map<std::string, std::string>& ports, std::string type);

protected:

	CIPICOMLibV6* ipicom;

	CLogger *m_pLogger;

    /*lanshh add **/
    int m_Curoute;
    int m_iPort;
	int datalink_id;
	int route_id;
	T_IPICOM_SOCK_ADDR_IN addr;
	T_IPICOM_SOCK_ADDR_IN addr_shared;
	int socket_shared;
	int socket_ded;
	int socket_pipe;

	char* LastErrorMsg;
	channel_type active_channel;
	std::map<channel_type, int> channel_map;

	int tid;
	int timeout_count;
	int opened;
	//int retry_count;

	std::map<int, std::pair<std::string, int> > response_map; //[tid]=[[response][return value]]

	int open_socket(std::string route);
	int open_socket(int route_id);
	int open_socket(T_IPICOM_SOCK_ADDR_IN& addr);
	int channel_to_socket(channel_type channel = default_channel);
	channel_type socket_to_channel(int);
	std::string get_channel_name(channel_type channel);
	std::string process_for_log(std::string&);

	bool send_internal(std::string input);
	bool send_tid_internal(std::string input, int& nof_commands_sent, int& tx_id, int channel);
	bool receive_internal(std::string& output, int expect_unsolicited);
	bool receive_tid_internal(int tx_id, int nof_exp_resp, std::string& output, int channel, int expect_unsolicited);

	int receive_wrapp(int socket, std::string& buff, int& rxid);
	int handle_target_crash();

	bool CheckServiceReady();
    bool CheckServiceReady(std::string node);
    bool CheckServicesReady(std::vector<std::string> &nodes);

	bool bCheckConnectionOK;
public:
    void StartLog(CLogger *pLogger) {
        m_pLogger = pLogger;
    }
    void StopLog (){
        m_pLogger = NULL;
    }
};

#endif
