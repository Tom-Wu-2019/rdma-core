/*
 * Copyright (c) 2004-2007 Voltaire Inc.  All rights reserved.
 * Copyright (c) 2009 HNR Consulting.  All rights reserved.
 * Copyright (c) 2009 Mellanox Technologies LTD.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <infiniband/mad.h>

/*
 * BITSOFFS and BE_OFFS are required due the fact that the bit offsets are inconsistently
 * encoded in the IB spec - IB headers are encoded such that the bit offsets
 * are in big endian convention (BE_OFFS), while the SMI/GSI queries data fields bit
 * offsets are specified using real bit offset (?!)
 * The following macros normalize everything to big endian offsets.
 */
#define BITSOFFS(o, w)	(((o) & ~31) | ((32 - ((o) & 31) - (w)))), (w)
#define BE_OFFS(o, w)	(o), (w)
#define BE_TO_BITSOFFS(o, w)	(((o) & ~31) | ((32 - ((o) & 31) - (w))))

static const ib_field_t ib_mad_f[] = {
	{0, 0},			/* IB_NO_FIELD - reserved as invalid */

	{0, 64, "GidPrefix", mad_dump_rhex},
	{64, 64, "GidGuid", mad_dump_rhex},

	/*
	 * MAD: common MAD fields (IB spec 13.4.2)
	 * SMP: Subnet Management packets - lid routed (IB spec 14.2.1.1)
	 * DSMP: Subnet Management packets - direct route (IB spec 14.2.1.2)
	 * SA: Subnet Administration packets (IB spec 15.2.1.1)
	 */

	/* first MAD word (0-3 bytes) */
	{BE_OFFS(0, 7), "MadMethod", mad_dump_hex},	/* TODO: add dumper */
	{BE_OFFS(7, 1), "MadIsResponse", mad_dump_uint},	/* TODO: add dumper */
	{BE_OFFS(8, 8), "MadClassVersion", mad_dump_uint},
	{BE_OFFS(16, 8), "MadMgmtClass", mad_dump_uint},	/* TODO: add dumper */
	{BE_OFFS(24, 8), "MadBaseVersion", mad_dump_uint},

	/* second MAD word (4-7 bytes) */
	{BE_OFFS(48, 16), "MadStatus", mad_dump_hex},	/* TODO: add dumper */

	/* DR SMP only */
	{BE_OFFS(32, 8), "DrSmpHopCnt", mad_dump_uint},
	{BE_OFFS(40, 8), "DrSmpHopPtr", mad_dump_uint},
	{BE_OFFS(48, 15), "DrSmpStatus", mad_dump_hex},	/* TODO: add dumper */
	{BE_OFFS(63, 1), "DrSmpDirection", mad_dump_uint},	/* TODO: add dumper */

	/* words 3,4,5,6 (8-23 bytes) */
	{64, 64, "MadTRID", mad_dump_hex},
	{BE_OFFS(144, 16), "MadAttr", mad_dump_hex},	/* TODO: add dumper */
	{160, 32, "MadModifier", mad_dump_hex},	/* TODO: add dumper */

	/* word 7,8 (24-31 bytes) */
	{192, 64, "MadMkey", mad_dump_hex},

	/* word 9 (32-37 bytes) */
	{BE_OFFS(256, 16), "DrSmpDLID", mad_dump_uint},
	{BE_OFFS(272, 16), "DrSmpSLID", mad_dump_uint},

	/* word 10,11 (36-43 bytes) */
	{288, 64, "SaSMkey", mad_dump_hex},

	/* word 12 (44-47 bytes) */
	{BE_OFFS(46 * 8, 16), "SaAttrOffs", mad_dump_uint},

	/* word 13,14 (48-55 bytes) */
	{48 * 8, 64, "SaCompMask", mad_dump_hex},

	/* word 13,14 (56-255 bytes) */
	{56 * 8, (256 - 56) * 8, "SaData", mad_dump_hex},

	/* bytes 64 - 127 */
	{0, 0},			/* IB_SM_DATA_F - reserved as invalid */

	/* bytes 64 - 256 */
	{64 * 8, (256 - 64) * 8, "GsData", mad_dump_hex},

	/* bytes 128 - 191 */
	{1024, 512, "DrSmpPath", mad_dump_hex},

	/* bytes 192 - 255 */
	{1536, 512, "DrSmpRetPath", mad_dump_hex},

	/*
	 * PortInfo fields
	 */
	{0, 64, "Mkey", mad_dump_hex},
	{64, 64, "GidPrefix", mad_dump_hex},
	{BITSOFFS(128, 16), "Lid", mad_dump_uint},
	{BITSOFFS(144, 16), "SMLid", mad_dump_uint},
	{160, 32, "CapMask", mad_dump_portcapmask},
	{BITSOFFS(192, 16), "DiagCode", mad_dump_hex},
	{BITSOFFS(208, 16), "MkeyLeasePeriod", mad_dump_uint},
	{BITSOFFS(224, 8), "LocalPort", mad_dump_uint},
	{BITSOFFS(232, 8), "LinkWidthEnabled", mad_dump_linkwidthen},
	{BITSOFFS(240, 8), "LinkWidthSupported", mad_dump_linkwidthsup},
	{BITSOFFS(248, 8), "LinkWidthActive", mad_dump_linkwidth},
	{BITSOFFS(256, 4), "LinkSpeedSupported", mad_dump_linkspeedsup},
	{BITSOFFS(260, 4), "LinkState", mad_dump_portstate},
	{BITSOFFS(264, 4), "PhysLinkState", mad_dump_physportstate},
	{BITSOFFS(268, 4), "LinkDownDefState", mad_dump_linkdowndefstate},
	{BITSOFFS(272, 2), "ProtectBits", mad_dump_uint},
	{BITSOFFS(277, 3), "LMC", mad_dump_uint},
	{BITSOFFS(280, 4), "LinkSpeedActive", mad_dump_linkspeed},
	{BITSOFFS(284, 4), "LinkSpeedEnabled", mad_dump_linkspeeden},
	{BITSOFFS(288, 4), "NeighborMTU", mad_dump_mtu},
	{BITSOFFS(292, 4), "SMSL", mad_dump_uint},
	{BITSOFFS(296, 4), "VLCap", mad_dump_vlcap},
	{BITSOFFS(300, 4), "InitType", mad_dump_hex},
	{BITSOFFS(304, 8), "VLHighLimit", mad_dump_uint},
	{BITSOFFS(312, 8), "VLArbHighCap", mad_dump_uint},
	{BITSOFFS(320, 8), "VLArbLowCap", mad_dump_uint},
	{BITSOFFS(328, 4), "InitReply", mad_dump_hex},
	{BITSOFFS(332, 4), "MtuCap", mad_dump_mtu},
	{BITSOFFS(336, 3), "VLStallCount", mad_dump_uint},
	{BITSOFFS(339, 5), "HoqLife", mad_dump_uint},
	{BITSOFFS(344, 4), "OperVLs", mad_dump_opervls},
	{BITSOFFS(348, 1), "PartEnforceInb", mad_dump_uint},
	{BITSOFFS(349, 1), "PartEnforceOutb", mad_dump_uint},
	{BITSOFFS(350, 1), "FilterRawInb", mad_dump_uint},
	{BITSOFFS(351, 1), "FilterRawOutb", mad_dump_uint},
	{BITSOFFS(352, 16), "MkeyViolations", mad_dump_uint},
	{BITSOFFS(368, 16), "PkeyViolations", mad_dump_uint},
	{BITSOFFS(384, 16), "QkeyViolations", mad_dump_uint},
	{BITSOFFS(400, 8), "GuidCap", mad_dump_uint},
	{BITSOFFS(408, 1), "ClientReregister", mad_dump_uint},
	{BITSOFFS(411, 5), "SubnetTimeout", mad_dump_uint},
	{BITSOFFS(419, 5), "RespTimeVal", mad_dump_uint},
	{BITSOFFS(424, 4), "LocalPhysErr", mad_dump_uint},
	{BITSOFFS(428, 4), "OverrunErr", mad_dump_uint},
	{BITSOFFS(432, 16), "MaxCreditHint", mad_dump_uint},
	{BITSOFFS(456, 24), "RoundTrip", mad_dump_uint},
	{0, 0},			/* IB_PORT_LAST_F */

	/*
	 * NodeInfo fields
	 */
	{BITSOFFS(0, 8), "BaseVers", mad_dump_uint},
	{BITSOFFS(8, 8), "ClassVers", mad_dump_uint},
	{BITSOFFS(16, 8), "NodeType", mad_dump_node_type},
	{BITSOFFS(24, 8), "NumPorts", mad_dump_uint},
	{32, 64, "SystemGuid", mad_dump_hex},
	{96, 64, "Guid", mad_dump_hex},
	{160, 64, "PortGuid", mad_dump_hex},
	{BITSOFFS(224, 16), "PartCap", mad_dump_uint},
	{BITSOFFS(240, 16), "DevId", mad_dump_hex},
	{256, 32, "Revision", mad_dump_hex},
	{BITSOFFS(288, 8), "LocalPort", mad_dump_uint},
	{BITSOFFS(296, 24), "VendorId", mad_dump_hex},
	{0, 0},			/* IB_NODE_LAST_F */

	/*
	 * SwitchInfo fields
	 */
	{BITSOFFS(0, 16), "LinearFdbCap", mad_dump_uint},
	{BITSOFFS(16, 16), "RandomFdbCap", mad_dump_uint},
	{BITSOFFS(32, 16), "McastFdbCap", mad_dump_uint},
	{BITSOFFS(48, 16), "LinearFdbTop", mad_dump_uint},
	{BITSOFFS(64, 8), "DefPort", mad_dump_uint},
	{BITSOFFS(72, 8), "DefMcastPrimPort", mad_dump_uint},
	{BITSOFFS(80, 8), "DefMcastNotPrimPort", mad_dump_uint},
	{BITSOFFS(88, 5), "LifeTime", mad_dump_uint},
	{BITSOFFS(93, 1), "StateChange", mad_dump_uint},
	{BITSOFFS(94, 2), "OptSLtoVLMapping", mad_dump_uint},
	{BITSOFFS(96, 16), "LidsPerPort", mad_dump_uint},
	{BITSOFFS(112, 16), "PartEnforceCap", mad_dump_uint},
	{BITSOFFS(128, 1), "InboundPartEnf", mad_dump_uint},
	{BITSOFFS(129, 1), "OutboundPartEnf", mad_dump_uint},
	{BITSOFFS(130, 1), "FilterRawInbound", mad_dump_uint},
	{BITSOFFS(131, 1), "FilterRawOutbound", mad_dump_uint},
	{BITSOFFS(132, 1), "EnhancedPort0", mad_dump_uint},
	{BITSOFFS(144, 16), "MulticastFDBTop", mad_dump_hex},
	{0, 0},			/* IB_SW_LAST_F */

	/*
	 * SwitchLinearForwardingTable fields
	 */
	{0, 512, "LinearForwTbl", mad_dump_array},

	/*
	 * SwitchMulticastForwardingTable fields
	 */
	{0, 512, "MulticastForwTbl", mad_dump_array},

	/*
	 * NodeDescription fields
	 */
	{0, 64 * 8, "NodeDesc", mad_dump_string},

	/*
	 * Notice/Trap fields
	 */
	{BITSOFFS(0, 1), "NoticeIsGeneric", mad_dump_uint},
	{BITSOFFS(1, 7), "NoticeType", mad_dump_uint},
	{BITSOFFS(8, 24), "NoticeProducerType", mad_dump_node_type},
	{BITSOFFS(32, 16), "NoticeTrapNumber", mad_dump_uint},
	{BITSOFFS(48, 16), "NoticeIssuerLID", mad_dump_uint},
	{BITSOFFS(64, 1), "NoticeToggle", mad_dump_uint},
	{BITSOFFS(65, 15), "NoticeCount", mad_dump_uint},
	{80, 432, "NoticeDataDetails", mad_dump_array},
	{BITSOFFS(80, 16), "NoticeDataLID", mad_dump_uint},
	{BITSOFFS(96, 16), "NoticeDataTrap144LID", mad_dump_uint},
	{BITSOFFS(128, 32), "NoticeDataTrap144CapMask", mad_dump_uint},

	/*
	 * Port counters
	 */
	{BITSOFFS(8, 8), "PortSelect", mad_dump_uint},
	{BITSOFFS(16, 16), "CounterSelect", mad_dump_hex},
	{BITSOFFS(32, 16), "SymbolErrors", mad_dump_uint},
	{BITSOFFS(48, 8), "LinkRecovers", mad_dump_uint},
	{BITSOFFS(56, 8), "LinkDowned", mad_dump_uint},
	{BITSOFFS(64, 16), "RcvErrors", mad_dump_uint},
	{BITSOFFS(80, 16), "RcvRemotePhysErrors", mad_dump_uint},
	{BITSOFFS(96, 16), "RcvSwRelayErrors", mad_dump_uint},
	{BITSOFFS(112, 16), "XmtDiscards", mad_dump_uint},
	{BITSOFFS(128, 8), "XmtConstraintErrors", mad_dump_uint},
	{BITSOFFS(136, 8), "RcvConstraintErrors", mad_dump_uint},
	{BITSOFFS(144, 8), "CounterSelect2", mad_dump_hex},
	{BITSOFFS(152, 4), "LinkIntegrityErrors", mad_dump_uint},
	{BITSOFFS(156, 4), "ExcBufOverrunErrors", mad_dump_uint},
	{BITSOFFS(176, 16), "VL15Dropped", mad_dump_uint},
	{192, 32, "XmtData", mad_dump_uint},
	{224, 32, "RcvData", mad_dump_uint},
	{256, 32, "XmtPkts", mad_dump_uint},
	{288, 32, "RcvPkts", mad_dump_uint},
	{320, 32, "XmtWait", mad_dump_uint},
	{0, 0},			/* IB_PC_LAST_F */

	/*
	 * SMInfo
	 */
	{0, 64, "SmInfoGuid", mad_dump_hex},
	{64, 64, "SmInfoKey", mad_dump_hex},
	{128, 32, "SmActivity", mad_dump_uint},
	{BITSOFFS(160, 4), "SmPriority", mad_dump_uint},
	{BITSOFFS(164, 4), "SmState", mad_dump_uint},

	/*
	 * SA RMPP
	 */
	{BE_OFFS(24 * 8 + 24, 8), "RmppVers", mad_dump_uint},
	{BE_OFFS(24 * 8 + 16, 8), "RmppType", mad_dump_uint},
	{BE_OFFS(24 * 8 + 11, 5), "RmppResp", mad_dump_uint},
	{BE_OFFS(24 * 8 + 8, 3), "RmppFlags", mad_dump_hex},
	{BE_OFFS(24 * 8 + 0, 8), "RmppStatus", mad_dump_hex},

	/* data1 */
	{28 * 8, 32, "RmppData1", mad_dump_hex},
	{28 * 8, 32, "RmppSegNum", mad_dump_uint},
	/* data2 */
	{32 * 8, 32, "RmppData2", mad_dump_hex},
	{32 * 8, 32, "RmppPayload", mad_dump_uint},
	{32 * 8, 32, "RmppNewWin", mad_dump_uint},

	/*
	 * SA Get Multi Path
	 */
	{BITSOFFS(41, 7), "MultiPathNumPath", mad_dump_uint},
	{BITSOFFS(120, 8), "MultiPathNumSrc", mad_dump_uint},
	{BITSOFFS(128, 8), "MultiPathNumDest", mad_dump_uint},
	{192, 128, "MultiPathGid", mad_dump_array},

	/*
	 * SA Path rec
	 */
	{64, 128, "PathRecDGid", mad_dump_array},
	{192, 128, "PathRecSGid", mad_dump_array},
	{BITSOFFS(320, 16), "PathRecDLid", mad_dump_uint},
	{BITSOFFS(336, 16), "PathRecSLid", mad_dump_uint},
	{BITSOFFS(393, 7), "PathRecNumPath", mad_dump_uint},
	{BITSOFFS(428, 4), "PathRecSL", mad_dump_uint},

	/*
	 * MC Member rec
	 */
	{0, 128, "McastMemMGid", mad_dump_array},
	{128, 128, "McastMemPortGid", mad_dump_array},
	{256, 32, "McastMemQkey", mad_dump_hex},
	{BITSOFFS(288, 16), "McastMemMLid", mad_dump_hex},
	{BITSOFFS(352, 4), "McastMemSL", mad_dump_uint},
	{BITSOFFS(306, 6), "McastMemMTU", mad_dump_uint},
	{BITSOFFS(338, 6), "McastMemRate", mad_dump_uint},
	{BITSOFFS(312, 8), "McastMemTClass", mad_dump_uint},
	{BITSOFFS(320, 16), "McastMemPkey", mad_dump_uint},
	{BITSOFFS(356, 20), "McastMemFlowLbl", mad_dump_uint},
	{BITSOFFS(388, 4), "McastMemJoinState", mad_dump_uint},
	{BITSOFFS(392, 1), "McastMemProxyJoin", mad_dump_uint},

	/*
	 * Service record
	 */
	{0, 64, "ServRecID", mad_dump_hex},
	{64, 128, "ServRecGid", mad_dump_array},
	{BITSOFFS(192, 16), "ServRecPkey", mad_dump_hex},
	{224, 32, "ServRecLease", mad_dump_hex},
	{256, 128, "ServRecKey", mad_dump_hex},
	{384, 512, "ServRecName", mad_dump_string},
	{896, 512, "ServRecData", mad_dump_array},	/* ATS for example */

	/*
	 * ATS SM record - within SA_SR_DATA
	 */
	{12 * 8, 32, "ATSNodeAddr", mad_dump_hex},
	{BITSOFFS(16 * 8, 16), "ATSMagicKey", mad_dump_hex},
	{BITSOFFS(18 * 8, 16), "ATSNodeType", mad_dump_hex},
	{32 * 8, 32 * 8, "ATSNodeName", mad_dump_string},

	/*
	 * SLTOVL MAPPING TABLE
	 */
	{0, 64, "SLToVLMap", mad_dump_hex},

	/*
	 * VL ARBITRATION TABLE
	 */
	{0, 512, "VLArbTbl", mad_dump_array},

	/*
	 * IB vendor classes range 2
	 */
	{BE_OFFS(36 * 8, 24), "OUI", mad_dump_array},
	{40 * 8, (256 - 40) * 8, "Vendor2Data", mad_dump_array},

	/*
	 * Extended port counters
	 */
	{BITSOFFS(8, 8), "PortSelect", mad_dump_uint},
	{BITSOFFS(16, 16), "CounterSelect", mad_dump_hex},
	{64, 64, "PortXmitData", mad_dump_uint},
	{128, 64, "PortRcvData", mad_dump_uint},
	{192, 64, "PortXmitPkts", mad_dump_uint},
	{256, 64, "PortRcvPkts", mad_dump_uint},
	{320, 64, "PortUnicastXmitPkts", mad_dump_uint},
	{384, 64, "PortUnicastRcvPkts", mad_dump_uint},
	{448, 64, "PortMulticastXmitPkts", mad_dump_uint},
	{512, 64, "PortMulticastRcvPkts", mad_dump_uint},
	{0, 0},			/* IB_PC_EXT_LAST_F */

	/*
	 * GUIDInfo fields
	 */
	{0, 64, "GUID0", mad_dump_hex},

	/*
	 * ClassPortInfo fields
	 */
	{BITSOFFS(0, 8), "BaseVersion", mad_dump_uint},
	{BITSOFFS(8, 8), "ClassVersion", mad_dump_uint},
	{BITSOFFS(16, 16), "CapabilityMask", mad_dump_hex},
	{BITSOFFS(32, 27), "CapabilityMask2", mad_dump_hex},
	{BITSOFFS(59, 5), "RespTimeVal", mad_dump_uint},
	{64, 128, "RedirectGID", mad_dump_array},
	{BITSOFFS(192, 8), "RedirectTC", mad_dump_hex},
	{BITSOFFS(200, 4), "RedirectSL", mad_dump_uint},
	{BITSOFFS(204, 20), "RedirectFL", mad_dump_hex},
	{BITSOFFS(224, 16), "RedirectLID", mad_dump_uint},
	{BITSOFFS(240, 16), "RedirectPKey", mad_dump_hex},
	{BITSOFFS(264, 24), "RedirectQP", mad_dump_hex},
	{288, 32, "RedirectQKey", mad_dump_hex},
	{320, 128, "TrapGID", mad_dump_array},
	{BITSOFFS(448, 8), "TrapTC", mad_dump_hex},
	{BITSOFFS(456, 4), "TrapSL", mad_dump_uint},
	{BITSOFFS(460, 20), "TrapFL", mad_dump_hex},
	{BITSOFFS(480, 16), "TrapLID", mad_dump_uint},
	{BITSOFFS(496, 16), "TrapPKey", mad_dump_hex},
	{BITSOFFS(512, 8), "TrapHL", mad_dump_uint},
	{BITSOFFS(520, 24), "TrapQP", mad_dump_hex},
	{544, 32, "TrapQKey", mad_dump_hex},

	/*
	 * PortXmitDataSL fields
	 */
	{32, 32, "XmtDataSL0", mad_dump_uint},
	{64, 32, "XmtDataSL1", mad_dump_uint},
	{96, 32, "XmtDataSL2", mad_dump_uint},
	{128, 32, "XmtDataSL3", mad_dump_uint},
	{160, 32, "XmtDataSL4", mad_dump_uint},
	{192, 32, "XmtDataSL5", mad_dump_uint},
	{224, 32, "XmtDataSL6", mad_dump_uint},
	{256, 32, "XmtDataSL7", mad_dump_uint},
	{288, 32, "XmtDataSL8", mad_dump_uint},
	{320, 32, "XmtDataSL9", mad_dump_uint},
	{352, 32, "XmtDataSL10", mad_dump_uint},
	{384, 32, "XmtDataSL11", mad_dump_uint},
	{416, 32, "XmtDataSL12", mad_dump_uint},
	{448, 32, "XmtDataSL13", mad_dump_uint},
	{480, 32, "XmtDataSL14", mad_dump_uint},
	{512, 32, "XmtDataSL15", mad_dump_uint},
	{0, 0},			/* IB_PC_XMT_DATA_SL_LAST_F */

	/*
	 * PortRcvDataSL fields
	 */
	{32, 32, "RcvDataSL0", mad_dump_uint},
	{64, 32, "RcvDataSL1", mad_dump_uint},
	{96, 32, "RcvDataSL2", mad_dump_uint},
	{128, 32, "RcvDataSL3", mad_dump_uint},
	{160, 32, "RcvDataSL4", mad_dump_uint},
	{192, 32, "RcvDataSL5", mad_dump_uint},
	{224, 32, "RcvDataSL6", mad_dump_uint},
	{256, 32, "RcvDataSL7", mad_dump_uint},
	{288, 32, "RcvDataSL8", mad_dump_uint},
	{320, 32, "RcvDataSL9", mad_dump_uint},
	{352, 32, "RcvDataSL10", mad_dump_uint},
	{384, 32, "RcvDataSL11", mad_dump_uint},
	{416, 32, "RcvDataSL12", mad_dump_uint},
	{448, 32, "RcvDataSL13", mad_dump_uint},
	{480, 32, "RcvDataSL14", mad_dump_uint},
	{512, 32, "RcvDataSL15", mad_dump_uint},
	{0, 0},			/* IB_PC_RCV_DATA_SL_LAST_F */

	/*
	 * PortXmitDiscardDetails fields
	 */
	{32, 16, "PortInactiveDiscards", mad_dump_uint},
	{48, 16, "PortNeighborMTUDiscards", mad_dump_uint},
	{64, 16, "PortSwLifetimeLimitDiscards", mad_dump_uint},
	{80, 16, "PortSwHOQLifetimeLimitDiscards", mad_dump_uint},
	{0, 0},			/* IB_PC_XMT_DISC_LAST_F */

	{0, 0}			/* IB_FIELD_LAST_ */

};

static void _set_field64(void *buf, int base_offs, const ib_field_t * f,
			 uint64_t val)
{
	uint64_t nval;

	nval = htonll(val);
	memcpy((char *)buf + base_offs + f->bitoffs / 8, &nval,
	       sizeof(uint64_t));
}

static uint64_t _get_field64(void *buf, int base_offs, const ib_field_t * f)
{
	uint64_t val;
	memcpy(&val, ((char *)buf + base_offs + f->bitoffs / 8),
	       sizeof(uint64_t));
	return ntohll(val);
}

static void _set_field(void *buf, int base_offs, const ib_field_t * f,
		       uint32_t val)
{
	int prebits = (8 - (f->bitoffs & 7)) & 7;
	int postbits = (f->bitoffs + f->bitlen) & 7;
	int bytelen = f->bitlen / 8;
	unsigned idx = base_offs + f->bitoffs / 8;
	char *p = (char *)buf;

	if (!bytelen && (f->bitoffs & 7) + f->bitlen < 8) {
		p[3 ^ idx] &= ~((((1 << f->bitlen) - 1)) << (f->bitoffs & 7));
		p[3 ^ idx] |=
		    (val & ((1 << f->bitlen) - 1)) << (f->bitoffs & 7);
		return;
	}

	if (prebits) {		/* val lsb in byte msb */
		p[3 ^ idx] &= (1 << (8 - prebits)) - 1;
		p[3 ^ idx++] |= (val & ((1 << prebits) - 1)) << (8 - prebits);
		val >>= prebits;
	}

	/* BIG endian byte order */
	for (; bytelen--; val >>= 8)
		p[3 ^ idx++] = val & 0xff;

	if (postbits) {		/* val msb in byte lsb */
		p[3 ^ idx] &= ~((1 << postbits) - 1);
		p[3 ^ idx] |= val;
	}
}

static uint32_t _get_field(void *buf, int base_offs, const ib_field_t * f)
{
	int prebits = (8 - (f->bitoffs & 7)) & 7;
	int postbits = (f->bitoffs + f->bitlen) & 7;
	int bytelen = f->bitlen / 8;
	unsigned idx = base_offs + f->bitoffs / 8;
	uint8_t *p = (uint8_t *) buf;
	uint32_t val = 0, v = 0, i;

	if (!bytelen && (f->bitoffs & 7) + f->bitlen < 8)
		return (p[3 ^ idx] >> (f->bitoffs & 7)) & ((1 << f->bitlen) -
							   1);

	if (prebits)		/* val lsb from byte msb */
		v = p[3 ^ idx++] >> (8 - prebits);

	if (postbits) {		/* val msb from byte lsb */
		i = base_offs + (f->bitoffs + f->bitlen) / 8;
		val = (p[3 ^ i] & ((1 << postbits) - 1));
	}

	/* BIG endian byte order */
	for (idx += bytelen - 1; bytelen--; idx--)
		val = (val << 8) | p[3 ^ idx];

	return (val << prebits) | v;
}

/* field must be byte aligned */
static void _set_array(void *buf, int base_offs, const ib_field_t * f,
		       void *val)
{
	int bitoffs = f->bitoffs;

	if (f->bitlen < 32)
		bitoffs = BE_TO_BITSOFFS(bitoffs, f->bitlen);

	memcpy((uint8_t *) buf + base_offs + bitoffs / 8, val, f->bitlen / 8);
}

static void _get_array(void *buf, int base_offs, const ib_field_t * f,
		       void *val)
{
	int bitoffs = f->bitoffs;

	if (f->bitlen < 32)
		bitoffs = BE_TO_BITSOFFS(bitoffs, f->bitlen);

	memcpy(val, (uint8_t *) buf + base_offs + bitoffs / 8, f->bitlen / 8);
}

uint32_t mad_get_field(void *buf, int base_offs, enum MAD_FIELDS field)
{
	return _get_field(buf, base_offs, ib_mad_f + field);
}

void mad_set_field(void *buf, int base_offs, enum MAD_FIELDS field,
		   uint32_t val)
{
	_set_field(buf, base_offs, ib_mad_f + field, val);
}

uint64_t mad_get_field64(void *buf, int base_offs, enum MAD_FIELDS field)
{
	return _get_field64(buf, base_offs, ib_mad_f + field);
}

void mad_set_field64(void *buf, int base_offs, enum MAD_FIELDS field,
		     uint64_t val)
{
	_set_field64(buf, base_offs, ib_mad_f + field, val);
}

void mad_set_array(void *buf, int base_offs, enum MAD_FIELDS field, void *val)
{
	_set_array(buf, base_offs, ib_mad_f + field, val);
}

void mad_get_array(void *buf, int base_offs, enum MAD_FIELDS field, void *val)
{
	_get_array(buf, base_offs, ib_mad_f + field, val);
}

void mad_decode_field(uint8_t * buf, enum MAD_FIELDS field, void *val)
{
	const ib_field_t *f = ib_mad_f + field;

	if (!field) {
		*(int *)val = *(int *)buf;
		return;
	}
	if (f->bitlen <= 32) {
		*(uint32_t *) val = _get_field(buf, 0, f);
		return;
	}
	if (f->bitlen == 64) {
		*(uint64_t *) val = _get_field64(buf, 0, f);
		return;
	}
	_get_array(buf, 0, f, val);
}

void mad_encode_field(uint8_t * buf, enum MAD_FIELDS field, void *val)
{
	const ib_field_t *f = ib_mad_f + field;

	if (!field) {
		*(int *)buf = *(int *)val;
		return;
	}
	if (f->bitlen <= 32) {
		_set_field(buf, 0, f, *(uint32_t *) val);
		return;
	}
	if (f->bitlen == 64) {
		_set_field64(buf, 0, f, *(uint64_t *) val);
		return;
	}
	_set_array(buf, 0, f, val);
}

/************************/

static char *_mad_dump_val(const ib_field_t * f, char *buf, int bufsz,
			   void *val)
{
	f->def_dump_fn(buf, bufsz, val, ALIGN(f->bitlen, 8) / 8);
	buf[bufsz - 1] = 0;

	return buf;
}

static char *_mad_dump_field(const ib_field_t * f, const char *name, char *buf,
			     int bufsz, void *val)
{
	char dots[128];
	int l, n;

	if (bufsz <= 32)
		return NULL;	/* buf too small */

	if (!name)
		name = f->name;

	l = strlen(name);
	if (l < 32) {
		memset(dots, '.', 32 - l);
		dots[32 - l] = 0;
	}

	n = snprintf(buf, bufsz, "%s:%s", name, dots);
	_mad_dump_val(f, buf + n, bufsz - n, val);
	buf[bufsz - 1] = 0;

	return buf;
}

static int _mad_dump(ib_mad_dump_fn * fn, const char *name, void *val,
		     int valsz)
{
	ib_field_t f;
	char buf[512];

	f.def_dump_fn = fn;
	f.bitlen = valsz * 8;

	return printf("%s\n", _mad_dump_field(&f, name, buf, sizeof buf, val));
}

static int _mad_print_field(const ib_field_t * f, const char *name, void *val,
			    int valsz)
{
	return _mad_dump(f->def_dump_fn, name ? name : f->name, val,
			 valsz ? valsz : ALIGN(f->bitlen, 8) / 8);
}

int mad_print_field(enum MAD_FIELDS field, const char *name, void *val)
{
	if (field <= IB_NO_FIELD || field >= IB_FIELD_LAST_)
		return -1;
	return _mad_print_field(ib_mad_f + field, name, val, 0);
}

char *mad_dump_field(enum MAD_FIELDS field, char *buf, int bufsz, void *val)
{
	if (field <= IB_NO_FIELD || field >= IB_FIELD_LAST_)
		return NULL;
	return _mad_dump_field(ib_mad_f + field, 0, buf, bufsz, val);
}

char *mad_dump_val(enum MAD_FIELDS field, char *buf, int bufsz, void *val)
{
	if (field <= IB_NO_FIELD || field >= IB_FIELD_LAST_)
		return NULL;
	return _mad_dump_val(ib_mad_f + field, buf, bufsz, val);
}

const char *mad_field_name(enum MAD_FIELDS field)
{
	return (ib_mad_f[field].name);
}
