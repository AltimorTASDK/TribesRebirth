GFXPolyDrawContext STRUC
	pdc_dwdx				dq ?
	pdc_dwdy				dq ?
	pdc_w0					dq ?
	pdc_dudx				dq ?
	pdc_dudy				dq ?
	pdc_u0					dq ?
	pdc_dvdx				dq ?
	pdc_dvdy				dq ?
	pdc_v0					dq ?
	pdc_dsdx				dq ?
	pdc_dsdy				dq ?
	pdc_s0					dq ?
	pdc_shadeMap			dd ?
	pdc_shadeScale			dd ?
	pdc_textureMap			dd ?
	pdc_textureMapStride	dd ?
	pdc_textureMapHeight	dd ?
	pdc_color				dd ?
	pdc_rowAddr				dd ?
	pdc_rowStride			dd ?
	pdc_currentY			dd ?
	pdc_destY				dd ?
	pdc_xRight				dd ?
	pdc_deltaXR				dd ?
	pdc_xLeft				dd ?
	pdc_deltaXL				dd ?
	pdc_rendSpanEntry		dd ?
	pdc_rendSpanExit		dd ?
	pdc_x					dd ?
	pdc_y					dd ?
	pdc_count				dd ?
	pdc_strideTable			dd ?
	pdc_strideLookup		dd ?
	pdc_dstLast				dd ?
	pdc_dstLastWord			dd ?
	pdc_fu					dd ?
	pdc_fv					dd ?
	pdc_fsu					dd ?
	pdc_fsv					dd ?
	pdc_subDivSize			dd ?
	pdc_spanLen				dd ?
	pdc_subSpanLen			dd ?
	pdc_colorsv				dd ?
	pdc_s_s_fraction		dd ?
	pdc_shadeSave			dd ?
	pdc_fs					dd ?
	pdc_fss					dd ?
	pdc_textureMapShift		dd ?
GFXPolyDrawContext ENDS

GFXRenderContext STRUC
	rc_rowAddr				dd ?
	rc_stride				dd ?
	rc_textureMap			dd ?
	rc_textureMapStride	dd ?
	rc_shadeMap			    dd ?
	rc_translucencyMap		dd ?
	rc_redMap				dd ?
	rc_greenMap			    dd ?
	rc_blueMap				dd ?
	rc_edgeList			    dd ?
	rc_currentEdgeLine		dd ?
	rc_curEdge				dd ?
	rc_freeEdge				dd ?
	rc_color				dd ?
	rc_hazeShift			dd ?
	rc_hazeMask				dd ?
	rc_rasterLoop			dd ?
	rc_clipSpanHook		    dd ?
	rc_spanDoneHook		    dd ?
	rc_spanClippedHook		dd ?
	rc_renderSpanHook		dd ?
	rc_nextEdgeHook		    dd ?
	rc_dstLast				dd ?
	rc_dstLastWord			dd ?
	rc_edgeList_topy		dd ?
	rc_edgeList_boty		dd ?
	rc_primitiveDoneHook	dd ? 	; Called by the edge clipper if it crossed the
									; bottom edge of the edge table
	rc_strideTable			dd ?
	rc_strideLookup		dd ?
GFXRenderContext ENDS

GFXSpan STRUC
	sp_x					dd ?
	sp_y					dd ?
	sp_cnt					dd ?
	sp_u					dd ?
	sp_su					dd ?
	sp_v					dd ?
	sp_sv					dd ?
	sp_fsu					dd ?
	sp_fsv					dd ?
	sp_w					dd ?
	sp_sw					dd ?
	sp_z					dd ?
	sp_sz					dd ?
	sp_s					dd ?
	sp_s_s					dd ?
	sp_h					dd ?
	sp_sh					dd ?
	sp_r					dd ?
	sp_sr					dd ?
	sp_g					dd ?
	sp_sg					dd ?
	sp_b					dd ?
	sp_sb					dd ?
	sp_intForwardStep		dd ?
	sp_fu					dd ?
	sp_fv					dd ?
	sp_subDivSize			dd ?
	sp_spanLen				dd ?
	sp_subSpanLen			dd ?
	sp_lx					dd ?
	sp_rx					dd ?
	sp_su_whole				dd ?
	sp_s_s_fraction			dd ?
	sp_shadeSave			dd ?
GFXSpan ENDS

GFXPolyRasterContext STRUC
	prc_hazeLevel			dd ?
	prc_deltaHaze			dd ?
	prc_currentY			dd ?
	prc_destY				dd ?
	prc_xRight				dd ?
	prc_deltaXR				dd ?
	prc_xLeft				dd ?
	prc_deltaXL				dd ?
	prc_iLeft				dd ?
	prc_deltaIL				dd ?
	prc_iRight				dd ?
	prc_deltaIR				dd ?
	prc_vLeft				dd ?
	prc_deltaVL				dd ?
	prc_vRight				dd ?
	prc_deltaVR				dd ?
	prc_uLeft				dd ?
	prc_deltaUL				dd ?
	prc_uRight				dd ?
	prc_deltaUR				dd ?
	prc_wLeft				dd ?
	prc_deltaWL				dd ?
	prc_wRight				dd ?
	prc_deltaWR				dd ?
	prc_zLeft				dd ?
	prc_deltaZL				dd ?
	prc_zRight				dd ?
	prc_deltaZR				dd ?
	prc_hLeft				dd ?
	prc_deltaHL				dd ?
	prc_hRight				dd ?
	prc_deltaHR				dd ?
	prc_rLeft				dd ?
	prc_deltaRL				dd ?
	prc_rRight				dd ?
	prc_deltaRR				dd ?
	prc_gLeft				dd ?
	prc_deltaGL				dd ?
	prc_gRight				dd ?
	prc_deltaGR				dd ?
	prc_bLeft				dd ?
	prc_deltaBL				dd ?
	prc_bRight				dd ?
	prc_deltaBR				dd ?

GFXPolyRasterContext ENDS

EXTRN  C gfxPRC : GFXPolyRasterContext
EXTRN  C gfxRC : GFXRenderContext
EXTRN  C gfxSpan : GFXSpan
EXTRN  C gfxPDC : GFXPolyDrawContext