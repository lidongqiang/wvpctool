// BEGIN CCCT SECTION
/* -------------------------------------------------------------------------
 Copyright (C) 2013-2015 Intel Mobile Communications GmbH

 Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
 $File name:  /msw_tools/calibration_rf/src/Core/Engine/src/ipicom_v6.cpp $
 $CC-Version: .../sunesimX_xmm_cal_tool_14_40/3 $
 $Date:       2014-09-26    11:40:09 UTC $
 ----------------------------------------------------------------------
 by CCCT (0.12h)
 ---------------------------------------------------------------------- */
// END CCCT SECTION

#include "stdafx.h"
#include "ipicom_v6.h"
#include <sstream >
#include <algorithm> 
extern void trim(std::string& input);

//////////////////////////////////////////////////////////////////////////
/// class CDynamicLibrary
//////////////////////////////////////////////////////////////////////////

CDynamicLibrary::CDynamicLibrary(std::string _name, int _detach)
{
	name = _name;
	detach = _detach;
	h_lib = 0;
}

CDynamicLibrary::CDynamicLibrary()
{
	detach = 0;
	h_lib = 0;
}

CDynamicLibrary::~CDynamicLibrary()
{
	if (!detach)
	{
		unload();
	}
}

int CDynamicLibrary::load()
{
	h_lib = LoadLibraryA(name.c_str());
	return h_lib != 0;
}

int CDynamicLibrary::load(std::string _name)
{
	name = _name;
	return load();
}

void CDynamicLibrary::unload()
{
	if (h_lib)
	{
		FreeLibrary(h_lib);
		h_lib = 0;
	}
}

int CDynamicLibrary::is_loaded()
{
	return h_lib != 0;
}

void* CDynamicLibrary::get_function(std::string name)
{
	if (h_lib)
	{
		return (void*)GetProcAddress((HINSTANCE)h_lib, name.c_str());
	}
	return 0;
}

/*****************************************************************************/
/*                       CIPICOMLibV6                                       */
/*****************************************************************************/

CIPICOMLibV6* CIPICOMLibV6::inst = 0;
int CIPICOMLibV6::fcount = 0;
int CIPICOMLibV6::isloaded = 0;

CIPICOMLibV6::CIPICOMLibV6()
{
}

CIPICOMLibV6::~CIPICOMLibV6()
{
	if (isloaded)
	{
		hModule.unload();
		isloaded = 0;
	}
}

CIPICOMLibV6* CIPICOMLibV6::Inst(std::string dll)
{
	if (!inst)
	{
		inst = new CIPICOMLibV6();
		if (!inst->Load(dll))
		{
			delete inst;
			inst = 0;
			return 0;
		}
	}

	return inst;
}

void CIPICOMLibV6::Destroy()
{
	if (inst)
	{
		delete inst;
		inst = 0;
	}
}

int CIPICOMLibV6::Load(std::string dll)
{
	if (hModule.is_loaded())
	{
		hModule.unload();
	}

	isloaded = 0;

	if (dll.size())
	{ // Path specified, try this first
		dll_location = dll;
		hModule.load(dll_location);
	}

	if (!hModule.is_loaded())
	{
		dll_location = ".\\libipcs.dll";
		hModule.load(dll_location);
	}

	if (!hModule.is_loaded())
	{
		isloaded = 0;
		dll_location = "No communications dll loaded!";
		return 0;
	}

	fcount = 0;

	pIPICOM_socket = (int(*)(int domain, int type, int protocol))get_function("IPICOM_socket");
	pIPICOM_socket_set_nonblocking = (T_IPICOM_ERROR(*)(T_IPICOM_SH shdl, T_IPICOM_SOCKET_IND_CBF cb_socket_ind))get_function("IPICOM_socket_set_nonblocking");
	pIPICOM_socket_bind = (T_IPICOM_ERROR(*)(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *bind_addr, unsigned int addrlen))get_function("IPICOM_socket_bind");
	pIPICOM_socket_close = (T_IPICOM_ERROR(*)(T_IPICOM_SH shdl))get_function("IPICOM_socket_close");
	pIPICOM_socket_connect = (T_IPICOM_ERROR(*)(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *addr, unsigned int addrlen))get_function("IPICOM_socket_connect");
	pIPICOM_socket_listen = (T_IPICOM_ERROR(*)(T_IPICOM_SH shdl, unsigned char max_pending))get_function("IPICOM_socket_listen");
	pIPICOM_socket_accept = (int(*)(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *cli_addr, unsigned int *addrlen))get_function("IPICOM_socket_accept");
	pIPICOM_send = (int(*)(int handle, void *p_data, unsigned int len))get_function("IPICOM_send");
	pIPICOM_recv = (int(*)(int handle, void *p_data, unsigned int len))get_function("IPICOM_recv");
	pIPICOM_socket_sendto = (int(*)(T_IPICOM_SH shdl, void *p_data, unsigned int len, T_IPICOM_SOCKET_SEND_FLAGS flags, T_IPICOM_SOCK_ADDR_IN *p_addr, unsigned int *addrlen))get_function("IPICOM_socket_sendto");
	pIPICOM_socket_recv_ref = (int(*)(T_IPICOM_SH shdl, void ** const p_data))get_function("IPICOM_socket_recv_ref");
	pIPICOM_socket_getsockname = (T_IPICOM_ERROR(*)(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *p_adr, unsigned int *p_addr_len))get_function("IPICOM_socket_getsockname");
	pIPICOM_socket_set_usrval = (T_IPICOM_ERROR(*)(T_IPICOM_SH shdl, unsigned int usr_val))get_function("IPICOM_socket_set_usrval");
	pIPICOM_socket_add_plp = (int(*)(T_IPICOM_SH shdl, T_IPICOM_PLP_ID plp_id, T_IPICOM_SOCKET_PLP_IND_CBF socket_ind_cbf))get_function("IPICOM_socket_add_plp");
	pIPICOM_register_plp = (T_IPICOM_ERROR(*)(T_IPICOM_PLP_ID plp_id, T_IPICOM_SOCKET_PLP_IND_CBF protocol_ind_cbf))get_function("IPICOM_register_plp");
	pIPICOM_datalink_open = (int(*)(const char *connection_name, const char *connection_option, bool mountable))get_function("IPICOM_datalink_open");
	pIPICOM_datalink_options = (T_IPICOM_ERROR(*)(T_IPICOM_DLH dl_id, const char * protocol_description))get_function("IPICOM_datalink_options");
	pIPICOM_datalink_close = (T_IPICOM_ERROR(*)(T_IPICOM_DLH dl_id))get_function("IPICOM_datalink_close");
	pIPICOM_datalink_callback_register = (T_IPICOM_ERROR(*)(T_IPICOM_DATALINK_IND_CBF cbf))get_function("IPCIOM_datalink_callback_register");
	//pIPICOM_datalink_set_transparent = (T_IPICOM_ERROR  (*)(T_IPICOM_DLH dl_id))get_function("IPICOM_datalink_set_transparent");
	pIPICOM_sapn_set_node_name = (T_IPICOM_ERROR(*)(const char * node_name))get_function("IPICOM_sapn_set_node_name");
	pIPICOM_sapn_mount = (T_IPICOM_SAPN_RH(*)(T_IPICOM_DLH dl_id))get_function("IPICOM_sapn_mount");
	pIPICOM_sapn_unmount = (T_IPICOM_ERROR(*)(T_IPICOM_DLH dl_hdl))get_function("IPICOM_sapn_unmount");
	pIPICOM_sapn_get_route = (T_IPICOM_SAPN_RH(*)(const char *path))get_function("IPICOM_sapn_get_route");
	pIPICOM_sapn_route_read_info = (T_IPICOM_ERROR(*)(T_IPICOM_SAPN_RH route_hdl, char * buffer, unsigned int buf_len, unsigned int flags))get_function("IPICOM_sapn_route_read_info");
	pIPICOM_socket_set_opt = (T_IPICOM_ERROR(*)(T_IPICOM_SH shdl, T_IPICOM_SOCKET_OPT_LEVEL level, T_IPICOM_SOCKET_OPT opt, const void* p_opt_val, unsigned int opt_len))get_function("IPICOM_socket_set_opt");

	pIPICOM_sapn_add_signage = (T_IPICOM_ERROR(*) (T_IPICOM_SAPN_RH route_hdl, const char* signage))get_function("IPICOM_sapn_add_signage");
	pIPICOM_sapn_get_addr_info = (T_IPICOM_ERROR(*) (const char* path, T_IPICOM_SAPN_ADDRESS* p_sapn_addr, unsigned int timeout_ms))get_function("IPICOM_sapn_get_addr_info");
	pIPICOM_start_trace = (int(*)())get_function("IPICOM_trace_start");
	pIPICOM_stop_trace = (void(*)(void))get_function("IPICOM_trace_stop");
	pIPICOM_trace_setSeverityLevel = (void(*)(int severity))get_function("IPICOM_trace_set_severity_level");

	pIPICOM_trace_setGroupFilter = (void(*)(char *group))get_function("IPICOM_trace_setGroupFilter");
	pIPICOM_trace = (void(*)(int severity, const char *group, const char *trace, ...))get_function("IPICOM_trace");

	pIPICOM_GetLastError = (char*(*)(void))get_function("IPICOM_GetLastError");;
	pIPICOM_GetDllVersion = (bool(*)(char *dll_version))get_function("IPICOM_GetDllVersion");
	pIPICOM_get_version = (int(*)(T_IPICOM_VERSION_INFO info_type, unsigned int *p_value))get_function("IPICOM_get_version");

	unsigned int ver_major = 0;
	unsigned int ver_minor = 0;
	unsigned int ver_rev = 0;
	if (pIPICOM_get_version)
	{
		pIPICOM_get_version(IPICOM_VERSION_MAJOR, &ver_major);
		pIPICOM_get_version(IPICOM_VERSION_MINOR, &ver_minor);
		pIPICOM_get_version(IPICOM_VERSION_REVISION, &ver_rev);
	}

	if (fcount)
	{
		isloaded = 1;
	}

	return isloaded;
}

void* CIPICOMLibV6::get_function(std::string name)
{
	void* fptr;
	fptr = hModule.get_function(name);
	if (fptr)
	{
		++fcount;
		return fptr;
	}

	return NULL;
}


const char* CIPICOMLibV6::get_last_error(int retval)
{
	const char* err_list[] = { "SUCCESS",
		"FAILURE",
		"TIMEOUT",
		"INVALID_PARAM",
		"SEND_Q_FULL",
		"OUT_OF_MEMORY",
		"RESPONSE_NOTIF",
		"SOCKET_ERR_INVALID",
		"SOCKET_ERR_PORT_IN_USE",
		"SOCKET_INVALID_PARAM",
		"SOCKET_WOULD_BLOCK",
		"SEND_BLOCK_LIMIT_REACHED",
		"RECV_Q_EMPTY" };
	retval *= -1;
	if ((unsigned int)retval < sizeof(err_list) / sizeof(char*))
	{
		return err_list[retval];
	}
	return "Unknown IPICOM error";
}

int CIPICOMLibV6::socket(int domain, int type, int protocol)
{
	int iRet = 0;

	iRet = (pIPICOM_socket)(domain, type, protocol);
	if (iRet == IPICOM_SOCKET_INVALID_HANDLE)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::socket_set_nonblocking(T_IPICOM_SH shdl, T_IPICOM_SOCKET_IND_CBF cb_socket_ind)
{
	int iRet = 0;

	iRet = (pIPICOM_socket_set_nonblocking)(shdl, cb_socket_ind);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::socket_bind(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *bind_addr, unsigned int addrlen)
{
	int iRet = 0;

	iRet = (pIPICOM_socket_bind)(shdl, bind_addr, addrlen);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::socket_close(T_IPICOM_SH shdl)
{
	int iRet = 0;

	iRet = (pIPICOM_socket_close)(shdl);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::socket_connect(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *addr, unsigned int addrlen)
{
	int iRet = 0;

	iRet = (pIPICOM_socket_connect)(shdl, addr, addrlen);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::socket_listen(T_IPICOM_SH shdl, unsigned char max_pending)
{
	int iRet = 0;

	iRet = (pIPICOM_socket_listen)(shdl, max_pending);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::socket_accept(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *cli_addr, unsigned int *addrlen)
{
	int iRet = 0;

	iRet = (pIPICOM_socket_accept)(shdl, cli_addr, addrlen);
	if (iRet == IPICOM_SOCKET_INVALID_HANDLE)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::send(int handle, std::string input)
{
	int iRet = 0;
	iRet = (pIPICOM_send)(handle, (void*)input.c_str(), input.length());
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::socket_sendto(T_IPICOM_SH shdl, std::string input, T_IPICOM_SOCKET_SEND_FLAGS flags, T_IPICOM_SOCK_ADDR_IN *p_addr, unsigned int *addrlen)
{
	int iRet = 0;

	iRet = (pIPICOM_socket_sendto)(shdl, (void*)input.c_str(), input.length(), flags, p_addr, addrlen);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::recv(int handle, std::string& output)
{
	int iRet = 0;

	char* p_data = new char[102400];
	iRet = (pIPICOM_recv)(handle, p_data, 102400);

	if (iRet < 0)
	{
		delete[] p_data;
		return iRet;
	}
	((char*)p_data)[iRet] = 0;
	output = (char*)p_data;
	delete[] p_data;

	return iRet;
}

int CIPICOMLibV6::socket_recv_ref(T_IPICOM_SH shdl, std::string& output)
{
	int iRet = 0;
	void *p_data;

	iRet = (pIPICOM_socket_recv_ref)(shdl, &p_data);
	if (iRet < 0)
	{
		return iRet;
	}
	output.assign((char*)p_data, iRet);
	return iRet;
}

int CIPICOMLibV6::socket_getsockname(T_IPICOM_SH shdl, T_IPICOM_SOCK_ADDR_IN *p_adr, unsigned int *p_addr_len)
{
	int iRet = 0;

	iRet = (pIPICOM_socket_getsockname)(shdl, p_adr, p_addr_len);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::socket_set_usrval(T_IPICOM_SH shdl, unsigned int usr_val)
{
	int iRet = 0;

	iRet = (pIPICOM_socket_set_usrval)(shdl, usr_val);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::socket_add_plp(T_IPICOM_SH shdl, T_IPICOM_PLP_ID plp_id, T_IPICOM_SOCKET_PLP_IND_CBF socket_ind_cbf)
{
	int iRet = 0;

	iRet = (pIPICOM_socket_add_plp)(shdl, plp_id, socket_ind_cbf);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::register_plp(T_IPICOM_PLP_ID plp_id, T_IPICOM_SOCKET_PLP_IND_CBF protocol_ind_cbf)
{
	int iRet = 0;

	iRet = (pIPICOM_register_plp)(plp_id, protocol_ind_cbf);
	if (iRet == IPICOM_SOCKET_INVALID_HANDLE)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::datalink_open(const char *connection_name, const char *connection_option, bool mountable)
{
	int iRet = 0;

	iRet = (pIPICOM_datalink_open)(connection_name, connection_option, mountable);
	if (iRet == IPICOM_DL_INVALID_ID)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::datalink_options(T_IPICOM_DLH dl_id, const char * protocol_description)
{
	int iRet = 0;

	iRet = (pIPICOM_datalink_options)(dl_id, protocol_description);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::datalink_close(T_IPICOM_DLH dl_id)
{
	int iRet = 0;

	iRet = (pIPICOM_datalink_close)(dl_id);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::datalink_callback_register(T_IPICOM_DATALINK_IND_CBF cbf)
{
	int iRet = 0;

	iRet = (pIPICOM_datalink_callback_register)(cbf);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::datalink_set_transparent(T_IPICOM_DLH dl_id)
{
	int iRet = 0;

	iRet = (pIPICOM_datalink_set_transparent)(dl_id);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::sapn_set_node_name(const char * node_name)
{
	int iRet = 0;

	iRet = (pIPICOM_sapn_set_node_name)(node_name);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

T_IPICOM_SAPN_RH CIPICOMLibV6::sapn_mount(T_IPICOM_DLH dl_id)
{
	int iRet = 0;

	iRet = (pIPICOM_sapn_mount)(dl_id);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::sapn_unmount(T_IPICOM_DLH dl_hdl)
{
	int iRet = 0;

	iRet = (pIPICOM_sapn_unmount)(dl_hdl);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

T_IPICOM_SAPN_RH CIPICOMLibV6::sapn_get_route(const char *path)
{
	int iRet = 0;

	iRet = (pIPICOM_sapn_get_route)(path);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::sapn_route_read_info(T_IPICOM_SAPN_RH route_hdl, char * buffer, unsigned int buf_len, unsigned int flags)
{
	int iRet = 0;

	iRet = (pIPICOM_sapn_route_read_info)(route_hdl, buffer, buf_len, flags);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::socket_set_opt(T_IPICOM_SH shdl, T_IPICOM_SOCKET_OPT_LEVEL level, T_IPICOM_SOCKET_OPT opt, const void* p_opt_val, unsigned int opt_len)
{
	int iRet = 0;

	iRet = (pIPICOM_socket_set_opt)(shdl, level, opt, p_opt_val, opt_len);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}
int CIPICOMLibV6::start_trace(void)
{
	int iRet = 0;

	(pIPICOM_trace_setSeverityLevel)(IPICOM_TRACE_SEVERITY_DEBUG);
	iRet = (pIPICOM_start_trace)();
	if (iRet < 0)
	{
		return iRet;
	}

	pIPICOM_trace_setSeverityLevel(-1);
	return iRet;
}

void CIPICOMLibV6::stop_trace(void)
{
	if (pIPICOM_stop_trace)
	{
		(pIPICOM_stop_trace)();
	}
}

int CIPICOMLibV6::sapn_add_signage(T_IPICOM_SAPN_RH route_hdl, std::string signage)
{
	int iRet = 0;

	iRet = (pIPICOM_sapn_add_signage)(route_hdl, signage.c_str());
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}

int CIPICOMLibV6::sapn_get_addr_info(std::string path, T_IPICOM_SAPN_ADDRESS& sapn_addr)
{
	int iRet = 0;

	iRet = (pIPICOM_sapn_get_addr_info)(path.c_str(), &sapn_addr, DEFAULT_TIMEOUT);
	if (iRet < 0)
	{
		return iRet;
	}

	return iRet;
}
/*****************************************************************************/
/*                       CIPICOMConnectionV6                                */
/*****************************************************************************/

CIPICOMConnectionV6::CIPICOMConnectionV6(CLogger *p)
{
    m_pLogger       = p;

    tid             = 1;

    socket_ded      = 0;
    socket_pipe     = 0;
    socket_shared   = 0;
    datalink_id     = 0;
    timeout_count   = 0;
    active_channel  = shared_channel;
    opened          = 0;
    ipicom          = NULL;
    m_Curoute       = -1;
	//retry_count = 0;
}

CIPICOMConnectionV6::~CIPICOMConnectionV6()
{
	close();
/*lanshh added **/
    if(ipicom) {
        ipicom->Destroy();
        ipicom = NULL;
    }
}

int CIPICOMConnectionV6::open_socket(T_IPICOM_SOCK_ADDR_IN& addr)
{
	int sock = ipicom->socket(PF_IPICOM, IPICOM_SOCKET_TYPE_SEQPACKET, IPICOM_SAPN_PROTO);
	//addr.sapn.route_id = route_id;
	//addr.sapn.port = 0;
	if (ipicom->socket_connect(sock, &addr, sizeof(addr)) < 0)
	{
		//new_options.clear();
		ipicom->socket_close(sock);
		return IPICOM_SOCKET_INVALID_HANDLE;
	}
	return sock;
}

int CIPICOMConnectionV6::open_socket(int route_id)
{
	int sock = ipicom->socket(PF_IPICOM, IPICOM_SOCKET_TYPE_SEQPACKET, IPICOM_SAPN_PROTO);
	addr.sapn.route_id = route_id;
	addr.sapn.port = 0;
	if (ipicom->socket_connect(sock, &addr, sizeof(addr)) < 0)
	{
		//new_options.clear();
		ipicom->socket_close(sock);
		return IPICOM_SOCKET_INVALID_HANDLE;
	}
	return sock;
}

int CIPICOMConnectionV6::open_socket(std::string route)
{
	int sock;
	sock = ipicom->socket(PF_IPICOM, IPICOM_SOCKET_TYPE_SEQPACKET, IPICOM_SAPN_PROTO);
	route_id = ipicom->sapn_get_route(route.c_str());
	if (route_id == (int)IPICOM_SOCKET_INVALID_HANDLE)
	{
		return IPICOM_SOCKET_INVALID_HANDLE;
	}
	addr.sapn.route_id = route_id;

	addr.sapn.port = 0;
	if (ipicom->socket_connect(sock, &addr, sizeof(addr)) < 0)
	{
		//new_options.clear();
		ipicom->socket_close(sock);
		return IPICOM_SOCKET_INVALID_HANDLE;
	}
	return sock;
}

bool CIPICOMConnectionV6::open(int nPort,const char * node_name)
{
	static int once = 1;

	if(!ipicom)
	{
		ipicom = CIPICOMLibV6::Inst();

		if (!ipicom)
		{
			LOGER((CLogger::DEBUG_ERROR,"Failed to initialize IPCS library!"));
			return false;
		}

		LOGER((CLogger::DEBUG_INFO,"Initialize IPCS library OK!"));

		if (once)
		{
            stop_trace(); /*no need trace **/
			ipicom->sapn_set_node_name(node_name);
			once = 0;
		}
	}

	bCheckConnectionOK = false;

	char full_port[128];
	char session_options[128];

	sprintf_s(full_port, "COM%d", nPort);
	sprintf_s(session_options, "BAUD=115200;RTS=0;DTR=1");

    LOGER((CLogger::DEBUG_INFO,"open data link(%s,%s,true)",full_port,session_options));
	datalink_id = ipicom->datalink_open(full_port, session_options, true);
	if (datalink_id < 0)
	{
		datalink_id = 0;
		LOGER((CLogger::DEBUG_ERROR,"Failed to open data link"));
		return false;
	}

	LOGER((CLogger::DEBUG_INFO,"Data link opened"));

	// create shared socket
	socket_shared = ipicom->socket(PF_IPICOM, IPICOM_SOCKET_TYPE_DGRAM, IPICOM_AT_PROTO);

	if (socket_shared == IPICOM_SOCKET_INVALID_HANDLE)
	{
		LOGER((CLogger::DEBUG_ERROR,"Failed to create shared socket"));
		return false;
	}

	LOGER((CLogger::DEBUG_INFO,"Shared socket created"));

	timeout_count = 0;

	T_IPICOM_SOCK_ADDR_IN bind_addr;
	bind_addr.at.datalink_id = datalink_id;
	ipicom->socket_bind(socket_shared, &bind_addr, sizeof(T_IPICOM_AT_ADDRESS));

	opened = 1;
	//E_LOG->TimeLine(eEngine, eLogTargetComm, "Configured socket %X as AT", socket_shared);

	// active shared channel
	channel_map[shared_channel] = socket_shared;
	active_channel = shared_channel;

	//stop_trace();
    m_iPort = nPort;
	return true;
}
bool CIPICOMConnectionV6::close_and_unload()
{
    LOGER((CLogger::DEBUG_INFO,"close_and_unload()"));
    close();
    if(ipicom) {
        ipicom->Destroy();
        ipicom = 0;
    }
    return true;
}

bool CIPICOMConnectionV6::close()
{
	response_map.clear();
    LOGER((CLogger::DEBUG_INFO,"IPICOM close()"));
	if (socket_ded&&(IPICOM_SOCKET_INVALID_HANDLE != socket_ded))
	{
		ipicom->socket_close(socket_ded);
		socket_ded = 0;
	}
	if (socket_pipe&&(IPICOM_SOCKET_INVALID_HANDLE != socket_pipe))
	{
		ipicom->socket_close(socket_pipe);
		socket_pipe = 0;
	}
	if (socket_shared&&(IPICOM_SOCKET_INVALID_HANDLE != socket_shared))
	{
		ipicom->socket_close(socket_shared);
		socket_shared = 0;
	}
	if (datalink_id)
	{
		ipicom->datalink_close(datalink_id);
		datalink_id = 0;
	}

	opened = 0;
	tid = 1;
    m_Curoute   = -1;
    LOGER((CLogger::DEBUG_INFO,"Connection closed"));
	return true;
}

int CIPICOMConnectionV6::send(std::string input)
{
	if (!opened)
	{
        LOGER((CLogger::DEBUG_ERROR,"Send Error, No Session!"));
		return 0;
	}

	int retval = 0;
	if (active_channel == shared_channel)
	{
		retval = send_internal(input);
	}
	else
	{
		int nof_commands;
		int id;
		retval = send_tid_internal(input, nof_commands, id, channel_to_socket());
	}

    if (retval) {
        LOGER((CLogger::DEBUG_INFO,"Send on %s OK: '%s'",get_channel_name(active_channel).c_str(),input.c_str()));
	} else {
        LOGER((CLogger::DEBUG_ERROR,"Send on %s ERROR: '%s'",get_channel_name(active_channel).c_str(),input.c_str()));
    }

	return retval;
}

int CIPICOMConnectionV6::send(std::string input, int& nof_cmd, int& tx_id, channel_type channel)
{
	if (!opened)
	{
        LOGER((CLogger::DEBUG_ERROR,"Send Error, No Session!"));
		return 0;
	}

	int retval = 0;
	if ((active_channel == shared_channel && channel == default_channel) || channel == shared_channel)
	{
		tx_id = 0;
		nof_cmd = 1;
		retval = send_internal(input);
	}
	else
	{
		retval = send_tid_internal(input, nof_cmd, tx_id, channel_to_socket(channel));
	}

    if (retval) {
        LOGER((CLogger::DEBUG_INFO,"Send on %s OK: '%s'",get_channel_name(channel).c_str(),input.c_str()));
	} else {
        LOGER((CLogger::DEBUG_ERROR,"Send on %s ERROR: '%s'",get_channel_name(channel).c_str(),input.c_str()));
    }
	return retval;
}

int CIPICOMConnectionV6::receive(std::string& output, int tx_id, int nof_exp_resp, int expect_unsolicited, channel_type channel)
{
	if (!opened)
	{
		LOGER((CLogger::DEBUG_ERROR,"Recv Error, No Session!"));
		return 0;
	}

	int retval = 0;
	if ((active_channel == shared_channel && channel == default_channel) || channel == shared_channel)
	{
		retval = receive_internal(output, expect_unsolicited);
	}
	else
	{
		retval = receive_tid_internal(tx_id, nof_exp_resp, output, channel_to_socket(channel), expect_unsolicited);
	}

	if (retval) {
        LOGER((CLogger::DEBUG_INFO,"Recv on %s OK: '%s'",get_channel_name(channel).c_str(),output.c_str()));
	} else {
        LOGER((CLogger::DEBUG_ERROR,"Recv on %s ERROR: '%s'",get_channel_name(channel).c_str(),output.c_str()));
    }
	return retval;
}
bool CIPICOMConnectionV6::query(std::string input, std::string& output, int expect_unsolicited,int retry)
{
	if (!opened)
	{
        LOGER((CLogger::DEBUG_ERROR,"Query Error, No Session!"));
		return false;
	}

	bool retval = false;
	if (active_channel == shared_channel)
	{
		send_internal(input);
		retval = receive_internal(output, expect_unsolicited);
		while (--retry > 0 && !retval)
		{
            LOGER((CLogger::DEBUG_INFO,"Retrying query"));

			send_internal(input);
			retval = receive_internal(output, expect_unsolicited);
		}
		return retval;
	}
	else
	{
		int nof_commands = 0;
		int id = 0;

		send_tid_internal(input, nof_commands, id, channel_to_socket());
        //Sleep(200);
		retval = receive_tid_internal(id, nof_commands, output, channel_to_socket(), expect_unsolicited);
		while (--retry > 0 && !retval)
		{
            LOGER((CLogger::DEBUG_INFO,"Retrying query"));
			send_tid_internal(input, nof_commands, id, channel_to_socket());
			// clear response cache for any incoherency
			if (id > 0 && !expect_unsolicited)
			{
				for (int i = id; i < id + nof_commands; ++i)
				{
					response_map.erase(id);
				}
			}
            Sleep(200);
			retval = receive_tid_internal(id, nof_commands, output, channel_to_socket(), expect_unsolicited);
		}

		return retval;
	}

	return retval;
}

bool CIPICOMConnectionV6::query(std::string input, std::string& output, int expect_unsolicited)
{
	int retry = 5;
	if (!opened)
	{
        LOGER((CLogger::DEBUG_ERROR,"Query Error, No Session!"));
		return false;
	}

	bool retval = false;
	if (active_channel == shared_channel)
	{
		send_internal(input);
		retval = receive_internal(output, expect_unsolicited);
		while (--retry > 0 && !retval)
		{
            LOGER((CLogger::DEBUG_INFO,"Retrying query"));

			send_internal(input);
			retval = receive_internal(output, expect_unsolicited);
		}
		return retval;
	}
	else
	{
		int nof_commands = 0;
		int id = 0;

		send_tid_internal(input, nof_commands, id, channel_to_socket());
		retval = receive_tid_internal(id, nof_commands, output, channel_to_socket(), expect_unsolicited);
		while (--retry > 0 && !retval)
		{
            LOGER((CLogger::DEBUG_INFO,"Retrying query"));

			send_tid_internal(input, nof_commands, id, channel_to_socket());
			// clear response cache for any incoherency
			if (id > 0 && !expect_unsolicited)
			{
				for (int i = id; i < id + nof_commands; ++i)
				{
					response_map.erase(id);
				}
			}
            Sleep(200);
			retval = receive_tid_internal(id, nof_commands, output, channel_to_socket(), expect_unsolicited);
		}

		return retval;
	}

	return retval;
}

int CIPICOMConnectionV6::timeout(int delay)
{
	if (!delay) delay = 2500*2;

	int retval = ipicom->socket_set_opt(channel_to_socket(), IPICOM_SOCK_OPT_LVL_SESSION, IPICOM_SOCK_OPT_TIMEOUT, &delay, sizeof(delay));
	if (!retval)
	{
        LOGER((CLogger::DEBUG_INFO,"Set timeout for %s socket to %dms",get_channel_name(active_channel).c_str(),delay));
	}

	return !retval;
}
/*lanshh **/
static const char * gti_url[3] ={
"DUT/CP:GTICOM",
"DUT/rsnode:GTICOM",
"DUT:GTICOM",
}; 

bool CIPICOMConnectionV6::reconfigure(int route)
{
#if 0
    LOGER((CLogger::DEBUG_INFO,"++reconfigure(%s)",gti_url[route> 2?2:route]));
    if( m_Curoute == route) {
        return true;
    }
    if(-1 != m_Curoute) {
        LOGER((CLogger::DEBUG_INFO,"close and reopen"));
        close();
        if(!open(m_iPort)){
            LOGER((CLogger::DEBUG_INFO,"IPICOM open fail"));
            goto reconfigure_exit;
        }
        timeout(0);
        Sleep(10);
    } else {
        LOGER((CLogger::DEBUG_INFO,"direct open route"));
    }
    if(!configure(route)){
        LOGER((CLogger::DEBUG_INFO,"IPICOM configure fail"));
        goto reconfigure_exit;
    }
    /*m_Curoute = route;**/
    return true;
reconfigure_exit:
    LOGER((CLogger::DEBUG_INFO,"--reconfigure():fail"));
    return false;   
#else 
    if( m_Curoute == route) return true; /*lanshh  **/
    response_map.clear();
    active_channel = shared_channel;
    if (socket_ded&&(IPICOM_SOCKET_INVALID_HANDLE != socket_ded)) {
        ipicom->socket_close(socket_ded);
        socket_ded = 0;
	}
    if (socket_pipe&&(IPICOM_SOCKET_INVALID_HANDLE != socket_pipe)) {
        ipicom->socket_close(socket_pipe);
        socket_pipe = 0;
	}
	bool bUseDedicatedCH = true;
	if (bUseDedicatedCH) {
    /*reopen **/
    if ((route_id = ipicom->sapn_mount(datalink_id)) < 0) {
        LOGER((CLogger::DEBUG_ERROR,"Mount fail"));
        return false;
    }
    ipicom->sapn_add_signage(route_id, "DUT");
    T_IPICOM_SOCK_ADDR_IN   addr;
        std::string             sapn_addr = gti_url[route> 2?2:route];

    ipicom->sapn_get_addr_info(sapn_addr, addr.sapn);
    socket_ded  = open_socket(addr);
    socket_pipe = open_socket(addr);
    if (socket_ded == IPICOM_SOCKET_INVALID_HANDLE || socket_pipe == IPICOM_SOCKET_INVALID_HANDLE) {
        return false;
    }
	// there are no priorities in v6
	// The real use of priority codes in test cases also was only to choose
	// normal dedicated vs pipe dedicated, Low is set for backwards compatibility.
	// Mid is normal, High is pipe
	channel_map[normal_channel] = socket_ded;
	channel_map[pipe_channel]   = socket_pipe;

	active_channel              = pipe_channel;
	timeout(0);
	active_channel              = normal_channel;
	timeout(0);
	//To shift the connection to pipe mode give ?
	//  set_ch_mode(pipe)
	//  To shift the connection back to normal mode give ?
	//  set_ch_mode(normal)
	int         nof_commands;
	int         id;
	std::string output;
	send_tid_internal("set_ch_mode(pipe)", nof_commands, id, socket_pipe);
	receive_tid_internal(id, nof_commands, output, socket_pipe, 0);
    m_Curoute = route;
    } else {
		active_channel = shared_channel;
        LOGER((CLogger::DEBUG_INFO,"Using shared channel"));
    }
	return true;
#endif
}
bool CIPICOMConnectionV6::configure(int route)
{
	bool bUseDedicatedCH = true;
	if (bUseDedicatedCH)
    {
		if ((route_id = ipicom->sapn_mount(datalink_id)) < 0)
		{
            LOGER((CLogger::DEBUG_ERROR,"Mount fail"));
			return false;
		}

		ipicom->sapn_add_signage(route_id, "DUT");
		char buff[2560];
		ipicom->sapn_route_read_info(route_id, buff, 2560, IPICOM_SAPN_ROUTE_INFO_ALL);
        /*
            0 :"DUT/CP:GTICOM"
            1 :"DUT/rsnode:GTICOM"
            2 :"DUT:GTICOM"
        **/
		std::string sapn_addr = gti_url[route> 2?2:route];
		T_IPICOM_SOCK_ADDR_IN addr;
		ipicom->sapn_get_addr_info(sapn_addr, addr.sapn);
		socket_ded = open_socket(addr);
		socket_pipe = open_socket(addr);

		if (socket_ded == IPICOM_SOCKET_INVALID_HANDLE || socket_pipe == IPICOM_SOCKET_INVALID_HANDLE)
		{
			return false;
		}

		// there are no priorities in v6
		// The real use of priority codes in test cases also was only to choose
		// normal dedicated vs pipe dedicated, Low is set for backwards compatibility.
		// Mid is normal, High is pipe

		channel_map[normal_channel] = socket_ded;
		channel_map[pipe_channel] = socket_pipe;

		active_channel = pipe_channel;
		timeout(0);
		active_channel = normal_channel;
		timeout(0);

		//To shift the connection to pipe mode give ?
		//  set_ch_mode(pipe)

		//  To shift the connection back to normal mode give ?
		//  set_ch_mode(normal)

		int nof_commands;
		int id;
		std::string output;
		send_tid_internal("set_ch_mode(pipe)", nof_commands, id, socket_pipe);
		receive_tid_internal(id, nof_commands, output, socket_pipe, 0);
	}
	else
	{
		active_channel = shared_channel;
        LOGER((CLogger::DEBUG_INFO,"Using shared channel"));
        
	}
    m_Curoute = route;
	return true;
}
bool CIPICOMConnectionV6::configure(bool bGTI)
{
	bool bUseDedicatedCH = true;
	if (bUseDedicatedCH)
    {
		if ((route_id = ipicom->sapn_mount(datalink_id)) < 0)
		{
            LOGER((CLogger::DEBUG_ERROR,"Mount fail"));
			return false;
		}

		ipicom->sapn_add_signage(route_id, "DUT");
		char buff[2560];
		ipicom->sapn_route_read_info(route_id, buff, 2560, IPICOM_SAPN_ROUTE_INFO_ALL);

		// DUT:GTICOM
		// DUT/CP:GTICOM
		std::string sapn_addr = bGTI ? "DUT/CP:GTICOM" :  "DUT:GTICOM";
		T_IPICOM_SOCK_ADDR_IN addr;
		ipicom->sapn_get_addr_info(sapn_addr, addr.sapn);
		socket_ded = open_socket(addr);
		socket_pipe = open_socket(addr);

		if (socket_ded == IPICOM_SOCKET_INVALID_HANDLE || socket_pipe == IPICOM_SOCKET_INVALID_HANDLE)
		{
			return false;
		}

		// there are no priorities in v6
		// The real use of priority codes in test cases also was only to choose
		// normal dedicated vs pipe dedicated, Low is set for backwards compatibility.
		// Mid is normal, High is pipe

		channel_map[normal_channel] = socket_ded;
		channel_map[pipe_channel] = socket_pipe;

		active_channel = pipe_channel;
		timeout(0);
		active_channel = normal_channel;
		timeout(0);

		//To shift the connection to pipe mode give ?
		//  set_ch_mode(pipe)

		//  To shift the connection back to normal mode give ?
		//  set_ch_mode(normal)

		int nof_commands;
		int id;
		std::string output;
		send_tid_internal("set_ch_mode(pipe)", nof_commands, id, socket_pipe);
		receive_tid_internal(id, nof_commands, output, socket_pipe, 0);
	}
	else
	{
		active_channel = shared_channel;
        LOGER((CLogger::DEBUG_INFO,"Using shared channel"));

	}

	return true;
}

int CIPICOMConnectionV6::cleanup()
{
	return 1;
}

bool CIPICOMConnectionV6::CheckServiceReady()
{
    LOGER((CLogger::DEBUG_INFO,"check(rsnode cp ap)"));

	if ((route_id = ipicom->sapn_mount(datalink_id)) < 0)
	{
        LOGER((CLogger::DEBUG_ERROR,"Mount fail"));
		return false;
	}

	std::string sRet;
	char buff[512];
	int nMaxTryCount = 10;
	for (int i = 0; i < nMaxTryCount; i++)
	{
		memset(buff, 0, sizeof(buff));
		ipicom->sapn_route_read_info(route_id, buff, 512, IPICOM_SAPN_ROUTE_INFO_REMOTE_SIGNAGES);

		LOG(CString(buff));

		sRet.assign(buff);
		transform(sRet.begin(), sRet.end(), sRet.begin(), tolower);

		if (sRet.find("rsnode") != std::string::npos && sRet.find("cp") != std::string::npos && sRet.find("ap") != std::string::npos)
			return true;

		Sleep(1500);
	}

	return false;
}
/*
check specifyed node is ok?
**/
bool CIPICOMConnectionV6::CheckServiceReady(std::string node)
{
    LOGER((CLogger::DEBUG_INFO,"check(%s)\r\n",node.c_str()));
	if ((route_id = ipicom->sapn_mount(datalink_id)) < 0)
	{
        LOGER((CLogger::DEBUG_ERROR,"Mount fail"));
		return false;
	}

	std::string sRet;
	char buff[512];
	int nMaxTryCount = 5;
	for (int i = 0; i < nMaxTryCount; i++)
	{
		memset(buff, 0, sizeof(buff));
		ipicom->sapn_route_read_info(route_id, buff, 512, IPICOM_SAPN_ROUTE_INFO_REMOTE_SIGNAGES);

		LOG(CString(buff));

		sRet.assign(buff);
		transform(sRet.begin(), sRet.end(), sRet.begin(), tolower);
        LOGER((CLogger::DEBUG_INFO,"CheckServiceReady:%s\r\n",sRet.c_str()));
		if (sRet.find(node.c_str()) != std::string::npos)
			return true;

		Sleep(1500);
	}

	return false;
}
bool CIPICOMConnectionV6::CheckServicesReady(std::vector<std::string> &nodes)
{
	std::string sRet;
	char		buff[512];
	int			nMaxTryCount = 5;
	for(int k = 0;k < nodes.size();k ++ ) {
        LOGER((CLogger::DEBUG_INFO,"checks(%s)",nodes[k].c_str()));
	}
	/* if(route_id < 0) { **/
		if ((route_id = ipicom->sapn_mount(datalink_id)) < 0) {
            LOGER((CLogger::DEBUG_ERROR,"Mount fail"));
			return false;
		}
	/*} **/
	for (int i = 0; i < nMaxTryCount; i++) {
        int j;
		memset(buff, 0, sizeof(buff));
		ipicom->sapn_route_read_info(route_id, buff, 512, IPICOM_SAPN_ROUTE_INFO_REMOTE_SIGNAGES);
		sRet.assign(buff);
		transform(sRet.begin(), sRet.end(), sRet.begin(), tolower);
        LOGER((CLogger::DEBUG_INFO,"sapn_route_read_info():%s",sRet.c_str()));
		for(j = 0;j < nodes.size(); j ++ ) {
			if (sRet.find(nodes[j].c_str()) == std::string::npos) {
				break; 	/*Could not find service ,then break**/
			}
		}
		if(nodes.size() == j ) {
			return true;   /*find servie count **/
		}
		Sleep(1500);
	}
	return false;
}

bool CIPICOMConnectionV6::check_services_on(std::vector<std::string> &nodes,int retry,int delay)
{
	timeout(1000);
	timeout_count = 0;
	if (!nodes.empty()) {
		if (!CheckServicesReady(nodes)) {
            LOGER((CLogger::DEBUG_INFO,"Failed to check USB conntection"));
			return false;
		}
	}
    timeout_count = 0;
    timeout(0);
	return true;
}
bool CIPICOMConnectionV6::check(bool bGTIPort)
{
	timeout(1000);
	timeout_count = 0;

	if (bGTIPort)
	{

#if TOOL_ENABLE_ON
		if (!CheckServiceReady("cp"))
#else 
        if (!CheckServiceReady())
#endif
		{
            LOGER((CLogger::DEBUG_ERROR,"Failed to check USB conntection"));
			return false;
		}
	} /*
	else
		Sleep(2000); **/
    
	std::string output;
	send_internal("ate0");
	if (!receive_internal(output, 0)) return false;

	output.clear();
	if (!query("at@", output)) return false;

	timeout_count = 0;
    timeout(0);
	return TRUE;
}

int CIPICOMConnectionV6::start_trace()
{
	return ipicom->start_trace();
}

int CIPICOMConnectionV6::stop_trace()
{
	ipicom->stop_trace();
	return 1;

}

int CIPICOMConnectionV6::set_channel(channel_type _ch_type)
{
	// Do not allow switching between modes
	// 1. Shared channel mode = Normal channel
	// 2. Dedicated channel mode = Low, Mid, High channels
	// The mode is determined in the configure step, else switching outside it is not possible since only 1 mode is getting configure()d
	if (active_channel != shared_channel)
	{
		active_channel = _ch_type;
		return 1;
	}
	return 0;
}

std::string CIPICOMConnectionV6::get_channel_name(channel_type channel)
{
	std::string names[] = { "-", "AT", "Normal", "Normal", "Pipe" };
	if (channel == default_channel)
	{
		return names[active_channel];
	}
	return names[channel];
}

channel_type CIPICOMConnectionV6::get_channel()
{
	return active_channel;
}

int CIPICOMConnectionV6::enumerate_ports(std::map<std::string, std::string>& /*ports*/, std::string /*type*/)
{
	return 0;
}

int CIPICOMConnectionV6::channel_to_socket(channel_type channel)
{
	if (channel == default_channel)
	{
		return channel_map[active_channel];
	}
	else
	{
		return channel_map[channel];
	}
}

channel_type CIPICOMConnectionV6::socket_to_channel(int sock)
{
	for (std::map<channel_type, int>::iterator it = channel_map.begin(); it != channel_map.end(); ++it)
	{
		if (sock == it->second)
		{
			return it->first;
		}
	}
	return default_channel;
}

bool CIPICOMConnectionV6::send_internal(std::string input)
{
	// on AT socket commands must be terminated with \r
	input.append("\r");
	int retval = ipicom->send(socket_shared, input) != 0;
	if (retval) {
        LOGER((CLogger::DEBUG_INFO,"Send on %s OK: '%s'",get_channel_name(active_channel).c_str(), process_for_log(input).c_str()));
	} else {
        LOGER((CLogger::DEBUG_ERROR,"Send on %s ERROR: '%s'",get_channel_name(active_channel).c_str(), process_for_log(input).c_str()));
    }
	return retval == 1;
}

bool CIPICOMConnectionV6::receive_internal(std::string& output, int expect_unsolicited)
{
	int recv_result;
	int dummy;

	recv_result = receive_wrapp(socket_shared, output, dummy);
	// Remove newlines in the string
	//std::string::size_type pos_line = output.find("\r\n");
	//while(pos_line != std::string::npos)
	//{
	//  output.replace(pos_line, 2, " ");
	//  pos_line = output.find("\r\n");
	//}
	if (recv_result == 0)  // Means OK
	{
		if (output.find("OK") != std::string::npos)
		{
            LOGER((CLogger::DEBUG_INFO,"Recv on %s OK: '%s'", get_channel_name(shared_channel).c_str(),process_for_log(output).c_str()));
			return true;
		} else {
            LOGER((CLogger::DEBUG_INFO,"Recv on %s ERROR: '%s'", get_channel_name(shared_channel).c_str(),process_for_log(output).c_str()));
		return false;
        }

	}
	else if (recv_result > 0) // means unsolicited data is received
	{
		if (expect_unsolicited)
		{
			return true;
		}

		output.clear();
		return receive_internal(output, expect_unsolicited);
	}
	else if (recv_result == -1) // Error from ipicom
	{
		return false;
	}
	else if (recv_result == -2) // Error from target
	{
		return false;
	}
	else if (recv_result == -3) // Received partial response
	{
		std::string incomplete_output;
		if (receive_internal(incomplete_output, expect_unsolicited))
		{
			output += incomplete_output;
			return true;
		}
		return false;
	}
	return false;
}

bool CIPICOMConnectionV6::send_tid_internal(std::string input, int& nof_commands_sent, int& tx_id, int sock)
{
	nof_commands_sent = 0;

	// Must strip at@ from command when sending over sapn socket
	std::string::size_type pos;
	if ((pos = input.find("at@")) != std::string::npos ||
		(pos = input.find("AT@")) != std::string::npos)
	{
		input.erase(pos, 3);
	}

	// Command format:
	// 1. at@ is removed
	// 2. <tid><space><command><0>

	std::stringstream ss;
	ss << tid << " " << input;
	std::string out = ss.str();
	out.append("\0", 1);
	tx_id = tid;

	if (ipicom->send(sock, out) < 0)
	{
        LOGER((CLogger::DEBUG_ERROR,"Send on %s ERROR: '%s'", get_channel_name(active_channel).c_str(), process_for_log(input).c_str()));
		return false;
	}
	else
        LOGER((CLogger::DEBUG_INFO,"Send on %s OK: '%s'", get_channel_name(active_channel).c_str(), process_for_log(input).c_str()));
	nof_commands_sent = std::count(input.begin(), input.end(), ';') + 1;
	tid += nof_commands_sent;

	return true;
}

bool CIPICOMConnectionV6::receive_tid_internal(int tx_id, int nof_exp_resp, std::string& output, int channel, int expect_unsolicited)
{
	int nof_completed_resp = 0;
	// Check if we already have all required responses in the cache. Does not work for unsolicited data.
	if (tx_id > 0 && !expect_unsolicited)
	{
		for (int i = tx_id; i < tx_id + nof_exp_resp; ++i)
		{
			if (response_map.find(i) != response_map.end())
			{
				nof_completed_resp++;
                LOGER((CLogger::DEBUG_ERROR,"Cache hit for (%d) '%s'", response_map[i].first.c_str()));
			}
			else
			{
				break;  // a (piled) response is missing, must receive it
			}
		}
	}
	// Receive loop
	while (nof_completed_resp < nof_exp_resp || expect_unsolicited)
	{
		int rx_tid = -3;
		int recv_result;

		recv_result = receive_wrapp(channel, output, rx_tid);

		// Remove newlines in the string
		std::string::size_type pos_line = output.find("\r\n");
		while (pos_line != std::string::npos)
		{
			output.replace(pos_line, 2, " ");
			pos_line = output.find("\r\n");
		}

		if (recv_result == 0)  // Means OK
		{
			if (!tx_id ||  // no tx_id specified or
				(rx_tid >= tx_id && rx_tid < tx_id + nof_exp_resp)) // rx id is with expected range
			{
				nof_completed_resp++;
			}
			if (nof_completed_resp < nof_exp_resp)
			{
				// piled command, not last reponse, concatenate new line, since v4 did it and we want it back so much
				output += "\r\n";
			}
			if (response_map.find(rx_tid) != response_map.end())
			{
				response_map[rx_tid].first += output;
			}
			else
			{
				response_map[rx_tid].first = output;
			}
			response_map[rx_tid].second = recv_result;
		}
		else if (recv_result > 0) // unsolicited data is received
		{
			if (expect_unsolicited)
			{
				return true;
			}
		}
		else if (recv_result == -1) // Error from ipicom
		{
			return false;
		}
		else if (recv_result == -2) // Error from target
		{
			return false;
		}
		else if (recv_result == -3) // Received partial response
		{
			response_map[rx_tid].first += output;
			response_map[rx_tid].second = recv_result;
		}
	} // End of receive loop
	// Format output
	output.clear();
	if (tx_id > 0)
	{
		for (int i = tx_id; i < tx_id + nof_exp_resp; ++i)
		{
			output += response_map[i].first;
			response_map.erase(i);
		}
	}
	else
	{
		// case when tx_id not specified and unsolicited data is not expected.
		// nof_exp_resp is not taken into account, instead all available are fetched
		for (std::map<int, std::pair<std::string, int> >::iterator it_resp = response_map.begin(); it_resp != response_map.end(); ++it_resp)
		{
			output += it_resp->second.first;
		}
		response_map.clear();
	}
	return true;
}

int CIPICOMConnectionV6::receive_wrapp(int socket, std::string& buff, int& rxid)
{
	int nMaxTimeoutCount = bCheckConnectionOK ? 2 : 30;
	if (timeout_count >= nMaxTimeoutCount)
	{
		return handle_target_crash();
	}

	// main receive
	std::string in_buff;
	int recv_status;
	if ((recv_status = ipicom->socket_recv_ref(socket, in_buff)) < 0)
	{
		//if(recv_status == IPICOM_TIMEOUT)
		{
			timeout_count++;
			if (timeout_count >= nMaxTimeoutCount)
			{
				return handle_target_crash();
			}
		}
		return -1;
	}
	timeout_count = 0;

	// normal channel handling (at socket): receive until terminator
	if (active_channel == shared_channel)
	{
		buff = in_buff;
		while (buff.find("\r\nOK\r\n") == std::string::npos && buff.find("\r\nERROR\r\n") == std::string::npos)
		{
			if ((recv_status = ipicom->socket_recv_ref(socket, in_buff)) < 0)
			{
				//if(recv_status == IPICOM_TIMEOUT)
				{
					timeout_count++;

					if (timeout_count >= nMaxTimeoutCount)
					{
						return handle_target_crash();
					}
				}
				return -1;
			}
			timeout_count = 0;
			buff += in_buff;
		}
		return 0;
	}

	//O:@<tid> <response string>    OK response. <tid> is present only if command had a tid. Response string is also only present if command
	//                              handler had something to print.
	//
	//E:@<tid> <response string>    ERROR response. <tid> is present only if command had a tid. Response string is also only present if
	//                              command handler had something to print.
	//
	//I:@<tid> <response string>    Intermediate response. <tid> is present only if command had a tid. Response string is always present.
	//                              This type response comes when command handler prints something intermediate before giving the final
	//                              ok or error response.
	//
	//U:@<tid> <response string>    Unsolicited response. <tid> is present only if command had a tid. Response string is always present.
	//                              This type response comes when command handler prints something after giving the final ok or error response.

	rxid = 0;
	size_t pos = in_buff.find_first_of(' ');
	std::string head = in_buff.substr(0, pos);
	if (pos == std::string::npos)
	{
		return 0;
	}

	std::stringstream ss;
	int retval = -1; // "Error from ipicom" == error in parsing response here
	buff = in_buff.substr(head.size() + 1);

	// remove any possible 0 chars from the string
	while ((pos = buff.find('\0')) != std::string::npos)
	{
		buff.erase(pos, 1);
	}
	// received rx_id
	if (head.size() > 3)
	{
		ss << head.substr(3);
		ss >> rxid;
	}

	if (head.find("O:") == 0)
	{
        LOGER((CLogger::DEBUG_INFO,"Recv on %s OK: '%s'", get_channel_name(socket_to_channel(socket)).c_str(),process_for_log(in_buff).c_str()));
		retval = 0; // OK
	}
	else if (head.find("E:") == 0)
	{
        LOGER((CLogger::DEBUG_ERROR,"Recv on %s ERROR: '%s'", get_channel_name(socket_to_channel(socket)).c_str(),process_for_log(in_buff).c_str()));
		retval = -2; // ERROR
	}
	else if (head.find("I:") == 0)
	{
        LOGER((CLogger::DEBUG_ERROR,"Recv on %s Incomlete: '%s'", get_channel_name(socket_to_channel(socket)).c_str(),process_for_log(in_buff).c_str()));
		retval = -3; // Intermediate/ Partial
	}
	else if (head.find("U:") == 0)
	{
        LOGER((CLogger::DEBUG_ERROR,"Recv on %s Unsolicited: '%s'", get_channel_name(socket_to_channel(socket)).c_str(),process_for_log(in_buff).c_str()));
		retval = buff.size(); // Unsolicited
	}

	return retval;
}

int CIPICOMConnectionV6::handle_target_crash()
{
    LOGER((CLogger::DEBUG_ERROR,"Target crash, aborting sequence."));
	close();
	return -1;
}

std::string CIPICOMConnectionV6::process_for_log(std::string& in)
{
	std::string out = in;
	std::map<char, std::string> replace_map;
	replace_map[0] = "<\\0>";
	replace_map['\r'] = "<\\r>";
	replace_map['\n'] = "<\\n>";

	size_t pos;
	for (std::map<char, std::string>::iterator it = replace_map.begin(); it != replace_map.end(); ++it)
	{
		while ((pos = out.find_first_of(it->first)) != std::string::npos)
		{
			out.replace(pos, 1, it->second);
		}
	}
	return out;
}
