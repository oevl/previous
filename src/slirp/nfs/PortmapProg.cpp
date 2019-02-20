#include <string.h>
#include <socket.h>

#include "PortmapProg.h"
#include "nfsd.h"
#include "XDRStream.h"

#define MIN_PROG_NUM 100000
enum
{
	MAPPROC_NULL = 0,
	MAPPROC_SET = 1,
	MAPPROC_UNSET = 2,
	MAPPROC_GETPORT = 3,
	MAPPROC_DUMP = 4,
	MAPPROC_CALLIT = 5
};

CPortmapProg::CPortmapProg() : CRPCProg(PROG_PORTMAP, 2, "portmapd") {
    memset(m_nProgTable, 0, PORT_NUM * sizeof(CRPCProg*));
}

CPortmapProg::~CPortmapProg() {}

void CPortmapProg::Add(CRPCProg* prog) {
    m_nProgTable[prog->GetProgNum() - MIN_PROG_NUM] = prog;
}

int CPortmapProg::Process(void)
{
    static PPROC pf[] = {
        &CRPCProg::Null,
        &CRPCProg::Notimp, // SET
        &CRPCProg::Notimp, // UNSET
        (PPROC)&CPortmapProg::ProcedureGETPORT,
        (PPROC)&CPortmapProg::ProcedureDUMP,
        (PPROC)&CPortmapProg::ProcedureCALLIT
    };
        
    if (m_param->proc >= sizeof(pf) / sizeof(PPROC)) return PRC_NOTIMP;
    
    return (this->*pf[m_param->proc])();
}

const CRPCProg* CPortmapProg::GetProg(int prog) {
    const CRPCProg* result = prog >= MIN_PROG_NUM && prog < MIN_PROG_NUM + PORT_NUM ? m_nProgTable[prog - MIN_PROG_NUM] : NULL;
    if(!(result))
        Log("no program %d", prog);
    return result;
}

int CPortmapProg::ProcedureGETPORT(void) {
    uint32_t prog;
    uint32_t vers;
    uint32_t proto;
    uint32_t port;

    m_in->Read(&prog);
    m_in->Read(&vers);
    m_in->Read(&proto);
    m_in->Read(&port);
    
    const CRPCProg* cprog = GetProg(prog);
    if(!(cprog)) return PRC_FAIL;
    
    switch(proto) {
        case IPPROTO_TCP:
            Log("GETPORT TCP %d %d", prog, cprog->GetPortTCP());
            m_out->Write(cprog->GetPortTCP());
            return PRC_OK;
        case IPPROTO_UDP:
            Log("GETPORT UDP %d %d", prog, cprog->GetPortUDP());
            m_out->Write(cprog->GetPortUDP());
            return PRC_OK;
        default:
            return PRC_FAIL;
    }
}

int CPortmapProg::ProcedureDUMP(void) {
    Log("DUMP");
    
    for(int i = 0; i < PORT_NUM; i++)
        if(m_nProgTable[i]) Write(m_nProgTable[i]);
    
    m_out->Write(0);
    
    return PRC_OK;
}

int CPortmapProg::ProcedureCALLIT(void)
{
    uint32_t prog;
    uint32_t vers;
    uint32_t proc;

    m_in->Read(&prog);
    m_in->Read(&vers);
    m_in->Read(&proc);
    
    XDROpaque in;
    m_in->Read(in);
    
    CRPCProg* cprog = (CRPCProg*)GetProg(prog);
    if(!(cprog)) return PRC_FAIL;
    
    ProcessParam param;
    param.proc       = proc;
    param.version    = vers;
    param.remoteAddr = m_param->remoteAddr;

    Log("CALLIT %s %d:%d:%d(...)", cprog->GetName(), cprog->GetProgNum(), cprog->GetVersion(), proc);

    XDRInput  din(in);
    XDROutput dout;
    
    cprog->Setup(&din, &dout, &param);
    int result = cprog->Process();
    
    m_out->Write(m_param->sockType == SOCK_STREAM ? cprog->GetPortTCP() : cprog->GetPortUDP());
    XDROpaque out(XDROpaque(dout.GetBuffer(), dout.GetSize()));
    m_out->Write(out);
    return result;
}
 

void CPortmapProg::Write(const CRPCProg* prog) {
    m_out->Write(1);
    m_out->Write(prog->GetProgNum());
    m_out->Write(prog->GetVersion());
    m_out->Write(IPPROTO_TCP);
    m_out->Write(prog->GetPortTCP());
    
    m_out->Write(1);
    m_out->Write(prog->GetProgNum());
    m_out->Write(prog->GetVersion());
    m_out->Write(IPPROTO_UDP);
    m_out->Write(prog->GetPortUDP());
}