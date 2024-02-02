#pragma once
#include <ff/net/middleware/ntpackage.h>
#include <ff/util/ntobject.h>

define_nt(RYXXBZ, std::string, "RYXXBZ");
define_nt(XM, std::string, "XM");
define_nt(CYM, std::string, "CYM");
define_nt(XBDM, std::string, "XBDM");
define_nt(FWXXBZ, std::string, "FWXXBZ");
define_nt(XP, std::string, "XP");
define_nt(DWMC, std::string, "DWMC");
define_nt(ZJHM, std::string, "ZJHM");
define_nt(GJDM, std::string, "GJDM");
define_nt(MZDM, std::string, "MZDM");
define_nt(JGSSXDM, std::string, "JGSSXDM");
define_nt(HKXZFLYDM, std::string, "HKXZFLYDM");
define_nt(HLXDM, std::string, "HLXDM");
define_nt(HJDZ_XZQHDM, std::string, "HJDZXZQHDM");
define_nt(SJJZD_XZQHDM, std::string, "SJJZDXZQHDM");
define_nt(SJJZD_QHNXXDZ, std::string, "SJJZDQHNXXDZ");
define_nt(XLDM, std::string, "XLDM");
define_nt(TSSFDM, std::string, "TSSFDM");
define_nt(CSQR, std::string, "CSQR");
define_nt(LXDH, std::string, "LXDH");
define_nt(HYZKDM, std::string, "HYZKDM");
define_nt(DJR_XM, std::string, "DJR_XM");
define_nt(DJR_GMSFZHM, std::string, "DJRGMSFZHM");
define_nt(DJR_LXDH, std::string, "DJRLXDH");
define_nt(GXSJ, std::string, "GXSJ");
define_nt(SJZT, std::string, "SJZT");

typedef ff::util::ntobject<
    RYXXBZ, XM, CYM, XBDM, FWXXBZ, XP, DWMC, ZJHM, GJDM, MZDM, JGSSXDM,
    HKXZFLYDM, HLXDM, HJDZ_XZQHDM, SJJZD_XZQHDM, SJJZD_QHNXXDZ, XLDM, TSSFDM,
    CSQR, LXDH, HYZKDM, DJR_XM, DJR_GMSFZHM, DJR_LXDH, GXSJ, SJZT>
    row_t;
