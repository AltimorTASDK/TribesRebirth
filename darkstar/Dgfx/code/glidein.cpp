/*******************************************************************
 * THIS FILE HAS BEEN HACKED BY AN EXPERIMENTAL PERL SCRIPT
 * ..\dynafy.pl
 * on 12 Aug 97
 *
 * The new functionality allows glide2x.dll to be loaded dynamically.
 * To do this, 
 * 1. #define DYNAHEADER wherever you include the new header ,
 *    glide.h.  
 * 2. Compile and link in glidein.c
 * 3. call LoadGlide();
 */


#define DYNAHEADER
/* Do not declare the externs */
#define DYNAHEADER_CREATE_STORAGE
#include <glide.h>

#include <windows.h>
static HINSTANCE glideDLLInst = NULL;
ConvertAndDownloadRle_fpt ConvertAndDownloadRle=NULL;
grAADrawLine_fpt grAADrawLine=NULL;
grAADrawPoint_fpt grAADrawPoint=NULL;
grAADrawPolygon_fpt grAADrawPolygon=NULL;
grAADrawPolygonVertexList_fpt grAADrawPolygonVertexList=NULL;
grAADrawTriangle_fpt grAADrawTriangle=NULL;
grAlphaBlendFunction_fpt grAlphaBlendFunction=NULL;
grAlphaCombine_fpt grAlphaCombine=NULL;
grAlphaControlsITRGBLighting_fpt grAlphaControlsITRGBLighting=NULL;
grAlphaTestFunction_fpt grAlphaTestFunction=NULL;
grAlphaTestReferenceValue_fpt grAlphaTestReferenceValue=NULL;
grBufferClear_fpt grBufferClear=NULL;
grBufferNumPending_fpt grBufferNumPending=NULL;
grBufferSwap_fpt grBufferSwap=NULL;
grCheckForRoom_fpt grCheckForRoom=NULL;
grChromakeyMode_fpt grChromakeyMode=NULL;
grChromakeyValue_fpt grChromakeyValue=NULL;
grClipWindow_fpt grClipWindow=NULL;
grColorCombine_fpt grColorCombine=NULL;
grColorMask_fpt grColorMask=NULL;
grConstantColorValue_fpt grConstantColorValue=NULL;
grConstantColorValue4_fpt grConstantColorValue4=NULL;
grCullMode_fpt grCullMode=NULL;
grDepthBiasLevel_fpt grDepthBiasLevel=NULL;
grDepthBufferFunction_fpt grDepthBufferFunction=NULL;
grDepthBufferMode_fpt grDepthBufferMode=NULL;
grDepthMask_fpt grDepthMask=NULL;
grDisableAllEffects_fpt grDisableAllEffects=NULL;
grDitherMode_fpt grDitherMode=NULL;
grDrawLine_fpt grDrawLine=NULL;
grDrawPlanarPolygon_fpt grDrawPlanarPolygon=NULL;
grDrawPlanarPolygonVertexList_fpt grDrawPlanarPolygonVertexList=NULL;
grDrawPoint_fpt grDrawPoint=NULL;
grDrawPolygon_fpt grDrawPolygon=NULL;
grDrawPolygonVertexList_fpt grDrawPolygonVertexList=NULL;
grDrawTriangle_fpt grDrawTriangle=NULL;
grErrorSetCallback_fpt grErrorSetCallback=NULL;
grFogColorValue_fpt grFogColorValue=NULL;
grFogMode_fpt grFogMode=NULL;
grFogTable_fpt grFogTable=NULL;
grGammaCorrectionValue_fpt grGammaCorrectionValue=NULL;
grGlideGetState_fpt grGlideGetState=NULL;
grGlideGetVersion_fpt grGlideGetVersion=NULL;
grGlideInit_fpt grGlideInit=NULL;
grGlideSetState_fpt grGlideSetState=NULL;
grGlideShamelessPlug_fpt grGlideShamelessPlug=NULL;
grGlideShutdown_fpt grGlideShutdown=NULL;
grHints_fpt grHints=NULL;
grLfbConstantAlpha_fpt grLfbConstantAlpha=NULL;
grLfbConstantDepth_fpt grLfbConstantDepth=NULL;
grLfbLock_fpt grLfbLock=NULL;
grLfbReadRegion_fpt grLfbReadRegion=NULL;
grLfbUnlock_fpt grLfbUnlock=NULL;
grLfbWriteColorFormat_fpt grLfbWriteColorFormat=NULL;
grLfbWriteColorSwizzle_fpt grLfbWriteColorSwizzle=NULL;
grLfbWriteRegion_fpt grLfbWriteRegion=NULL;
grRenderBuffer_fpt grRenderBuffer=NULL;
grResetTriStats_fpt grResetTriStats=NULL;
grSplash_fpt grSplash=NULL;
grSstControl_fpt grSstControl=NULL;
grSstIdle_fpt grSstIdle=NULL;
grSstIsBusy_fpt grSstIsBusy=NULL;
grSstOrigin_fpt grSstOrigin=NULL;
grSstPerfStats_fpt grSstPerfStats=NULL;
grSstQueryBoards_fpt grSstQueryBoards=NULL;
grSstQueryHardware_fpt grSstQueryHardware=NULL;
grSstResetPerfStats_fpt grSstResetPerfStats=NULL;
grSstScreenHeight_fpt grSstScreenHeight=NULL;
grSstScreenWidth_fpt grSstScreenWidth=NULL;
grSstSelect_fpt grSstSelect=NULL;
grSstStatus_fpt grSstStatus=NULL;
grSstVRetraceOn_fpt grSstVRetraceOn=NULL;
grSstVideoLine_fpt grSstVideoLine=NULL;
grSstWinClose_fpt grSstWinClose=NULL;
grSstWinOpen_fpt grSstWinOpen=NULL;
grTexCalcMemRequired_fpt grTexCalcMemRequired=NULL;
grTexClampMode_fpt grTexClampMode=NULL;
grTexCombine_fpt grTexCombine=NULL;
grTexCombineFunction_fpt grTexCombineFunction=NULL;
grTexDetailControl_fpt grTexDetailControl=NULL;
grTexDownloadMipMap_fpt grTexDownloadMipMap=NULL;
grTexDownloadMipMapLevel_fpt grTexDownloadMipMapLevel=NULL;
grTexDownloadMipMapLevelPartial_fpt grTexDownloadMipMapLevelPartial=NULL;
grTexDownloadTable_fpt grTexDownloadTable=NULL;
grTexDownloadTablePartial_fpt grTexDownloadTablePartial=NULL;
grTexFilterMode_fpt grTexFilterMode=NULL;
grTexLodBiasValue_fpt grTexLodBiasValue=NULL;
grTexMaxAddress_fpt grTexMaxAddress=NULL;
grTexMinAddress_fpt grTexMinAddress=NULL;
grTexMipMapMode_fpt grTexMipMapMode=NULL;
grTexMultibase_fpt grTexMultibase=NULL;
grTexMultibaseAddress_fpt grTexMultibaseAddress=NULL;
grTexNCCTable_fpt grTexNCCTable=NULL;
grTexSource_fpt grTexSource=NULL;
grTexTextureMemRequired_fpt grTexTextureMemRequired=NULL;
grTriStats_fpt grTriStats=NULL;
guTexAllocateMemory_fpt guTexAllocateMemory=NULL;
guTexChangeAttributes_fpt guTexChangeAttributes=NULL;
guTexCombineFunction_fpt guTexCombineFunction=NULL;
guTexDownloadMipMap_fpt guTexDownloadMipMap=NULL;
guTexDownloadMipMapLevel_fpt guTexDownloadMipMapLevel=NULL;
guTexGetCurrentMipMap_fpt guTexGetCurrentMipMap=NULL;
guTexGetMipMapInfo_fpt guTexGetMipMapInfo=NULL;
guTexMemQueryAvail_fpt guTexMemQueryAvail=NULL;
guTexMemReset_fpt guTexMemReset=NULL;
guTexSource_fpt guTexSource=NULL;

FxBool LoadGlide()
{
    glideDLLInst = LoadLibrary("glide2x.dll");
    if(!glideDLLInst) return FALSE;

	ConvertAndDownloadRle = (ConvertAndDownloadRle_fpt)GetProcAddress(glideDLLInst,"_ConvertAndDownloadRle@64");
	if(! ConvertAndDownloadRle) return FALSE;
	grAADrawLine = (grAADrawLine_fpt)GetProcAddress(glideDLLInst,"_grAADrawLine@8");
	if(! grAADrawLine) return FALSE;
	grAADrawPoint = (grAADrawPoint_fpt)GetProcAddress(glideDLLInst,"_grAADrawPoint@4");
	if(! grAADrawPoint) return FALSE;
	grAADrawPolygon = (grAADrawPolygon_fpt)GetProcAddress(glideDLLInst,"_grAADrawPolygon@12");
	if(! grAADrawPolygon) return FALSE;
	grAADrawPolygonVertexList = (grAADrawPolygonVertexList_fpt)GetProcAddress(glideDLLInst,"_grAADrawPolygonVertexList@8");
	if(! grAADrawPolygonVertexList) return FALSE;
	grAADrawTriangle = (grAADrawTriangle_fpt)GetProcAddress(glideDLLInst,"_grAADrawTriangle@24");
	if(! grAADrawTriangle) return FALSE;
	grAlphaBlendFunction = (grAlphaBlendFunction_fpt)GetProcAddress(glideDLLInst,"_grAlphaBlendFunction@16");
	if(! grAlphaBlendFunction) return FALSE;
	grAlphaCombine = (grAlphaCombine_fpt)GetProcAddress(glideDLLInst,"_grAlphaCombine@20");
	if(! grAlphaCombine) return FALSE;
	grAlphaControlsITRGBLighting = (grAlphaControlsITRGBLighting_fpt)GetProcAddress(glideDLLInst,"_grAlphaControlsITRGBLighting@4");
	if(! grAlphaControlsITRGBLighting) return FALSE;
	grAlphaTestFunction = (grAlphaTestFunction_fpt)GetProcAddress(glideDLLInst,"_grAlphaTestFunction@4");
	if(! grAlphaTestFunction) return FALSE;
	grAlphaTestReferenceValue = (grAlphaTestReferenceValue_fpt)GetProcAddress(glideDLLInst,"_grAlphaTestReferenceValue@4");
	if(! grAlphaTestReferenceValue) return FALSE;
	grBufferClear = (grBufferClear_fpt)GetProcAddress(glideDLLInst,"_grBufferClear@12");
	if(! grBufferClear) return FALSE;
	grBufferNumPending = (grBufferNumPending_fpt)GetProcAddress(glideDLLInst,"_grBufferNumPending@0");
	if(! grBufferNumPending) return FALSE;
	grBufferSwap = (grBufferSwap_fpt)GetProcAddress(glideDLLInst,"_grBufferSwap@4");
	if(! grBufferSwap) return FALSE;
	grCheckForRoom = (grCheckForRoom_fpt)GetProcAddress(glideDLLInst,"_grCheckForRoom@4");
	if(! grCheckForRoom) return FALSE;
	grChromakeyMode = (grChromakeyMode_fpt)GetProcAddress(glideDLLInst,"_grChromakeyMode@4");
	if(! grChromakeyMode) return FALSE;
	grChromakeyValue = (grChromakeyValue_fpt)GetProcAddress(glideDLLInst,"_grChromakeyValue@4");
	if(! grChromakeyValue) return FALSE;
	grClipWindow = (grClipWindow_fpt)GetProcAddress(glideDLLInst,"_grClipWindow@16");
	if(! grClipWindow) return FALSE;
	grColorCombine = (grColorCombine_fpt)GetProcAddress(glideDLLInst,"_grColorCombine@20");
	if(! grColorCombine) return FALSE;
	grColorMask = (grColorMask_fpt)GetProcAddress(glideDLLInst,"_grColorMask@8");
	if(! grColorMask) return FALSE;
	grConstantColorValue = (grConstantColorValue_fpt)GetProcAddress(glideDLLInst,"_grConstantColorValue@4");
	if(! grConstantColorValue) return FALSE;
	grConstantColorValue4 = (grConstantColorValue4_fpt)GetProcAddress(glideDLLInst,"_grConstantColorValue4@16");
	if(! grConstantColorValue4) return FALSE;
	grCullMode = (grCullMode_fpt)GetProcAddress(glideDLLInst,"_grCullMode@4");
	if(! grCullMode) return FALSE;
	grDepthBiasLevel = (grDepthBiasLevel_fpt)GetProcAddress(glideDLLInst,"_grDepthBiasLevel@4");
	if(! grDepthBiasLevel) return FALSE;
	grDepthBufferFunction = (grDepthBufferFunction_fpt)GetProcAddress(glideDLLInst,"_grDepthBufferFunction@4");
	if(! grDepthBufferFunction) return FALSE;
	grDepthBufferMode = (grDepthBufferMode_fpt)GetProcAddress(glideDLLInst,"_grDepthBufferMode@4");
	if(! grDepthBufferMode) return FALSE;
	grDepthMask = (grDepthMask_fpt)GetProcAddress(glideDLLInst,"_grDepthMask@4");
	if(! grDepthMask) return FALSE;
	grDisableAllEffects = (grDisableAllEffects_fpt)GetProcAddress(glideDLLInst,"_grDisableAllEffects@0");
	if(! grDisableAllEffects) return FALSE;
	grDitherMode = (grDitherMode_fpt)GetProcAddress(glideDLLInst,"_grDitherMode@4");
	if(! grDitherMode) return FALSE;
	grDrawLine = (grDrawLine_fpt)GetProcAddress(glideDLLInst,"_grDrawLine@8");
	if(! grDrawLine) return FALSE;
	grDrawPlanarPolygon = (grDrawPlanarPolygon_fpt)GetProcAddress(glideDLLInst,"_grDrawPlanarPolygon@12");
	if(! grDrawPlanarPolygon) return FALSE;
	grDrawPlanarPolygonVertexList = (grDrawPlanarPolygonVertexList_fpt)GetProcAddress(glideDLLInst,"_grDrawPlanarPolygonVertexList@8");
	if(! grDrawPlanarPolygonVertexList) return FALSE;
	grDrawPoint = (grDrawPoint_fpt)GetProcAddress(glideDLLInst,"_grDrawPoint@4");
	if(! grDrawPoint) return FALSE;
	grDrawPolygon = (grDrawPolygon_fpt)GetProcAddress(glideDLLInst,"_grDrawPolygon@12");
	if(! grDrawPolygon) return FALSE;
	grDrawPolygonVertexList = (grDrawPolygonVertexList_fpt)GetProcAddress(glideDLLInst,"_grDrawPolygonVertexList@8");
	if(! grDrawPolygonVertexList) return FALSE;
	grDrawTriangle = (grDrawTriangle_fpt)GetProcAddress(glideDLLInst,"_grDrawTriangle@12");
	if(! grDrawTriangle) return FALSE;
	grErrorSetCallback = (grErrorSetCallback_fpt)GetProcAddress(glideDLLInst,"_grErrorSetCallback@4");
	if(! grErrorSetCallback) return FALSE;
	grFogColorValue = (grFogColorValue_fpt)GetProcAddress(glideDLLInst,"_grFogColorValue@4");
	if(! grFogColorValue) return FALSE;
	grFogMode = (grFogMode_fpt)GetProcAddress(glideDLLInst,"_grFogMode@4");
	if(! grFogMode) return FALSE;
	grFogTable = (grFogTable_fpt)GetProcAddress(glideDLLInst,"_grFogTable@4");
	if(! grFogTable) return FALSE;
	grGammaCorrectionValue = (grGammaCorrectionValue_fpt)GetProcAddress(glideDLLInst,"_grGammaCorrectionValue@4");
	if(! grGammaCorrectionValue) return FALSE;
	grGlideGetState = (grGlideGetState_fpt)GetProcAddress(glideDLLInst,"_grGlideGetState@4");
	if(! grGlideGetState) return FALSE;
	grGlideGetVersion = (grGlideGetVersion_fpt)GetProcAddress(glideDLLInst,"_grGlideGetVersion@4");
	if(! grGlideGetVersion) return FALSE;
	grGlideInit = (grGlideInit_fpt)GetProcAddress(glideDLLInst,"_grGlideInit@0");
	if(! grGlideInit) return FALSE;
	grGlideSetState = (grGlideSetState_fpt)GetProcAddress(glideDLLInst,"_grGlideSetState@4");
	if(! grGlideSetState) return FALSE;
	grGlideShamelessPlug = (grGlideShamelessPlug_fpt)GetProcAddress(glideDLLInst,"_grGlideShamelessPlug@4");
	if(! grGlideShamelessPlug) return FALSE;
	grGlideShutdown = (grGlideShutdown_fpt)GetProcAddress(glideDLLInst,"_grGlideShutdown@0");
	if(! grGlideShutdown) return FALSE;
	grHints = (grHints_fpt)GetProcAddress(glideDLLInst,"_grHints@8");
	if(! grHints) return FALSE;
	grLfbConstantAlpha = (grLfbConstantAlpha_fpt)GetProcAddress(glideDLLInst,"_grLfbConstantAlpha@4");
	if(! grLfbConstantAlpha) return FALSE;
	grLfbConstantDepth = (grLfbConstantDepth_fpt)GetProcAddress(glideDLLInst,"_grLfbConstantDepth@4");
	if(! grLfbConstantDepth) return FALSE;
	grLfbLock = (grLfbLock_fpt)GetProcAddress(glideDLLInst,"_grLfbLock@24");
	if(! grLfbLock) return FALSE;
	grLfbReadRegion = (grLfbReadRegion_fpt)GetProcAddress(glideDLLInst,"_grLfbReadRegion@28");
	if(! grLfbReadRegion) return FALSE;
	grLfbUnlock = (grLfbUnlock_fpt)GetProcAddress(glideDLLInst,"_grLfbUnlock@8");
	if(! grLfbUnlock) return FALSE;
	grLfbWriteColorFormat = (grLfbWriteColorFormat_fpt)GetProcAddress(glideDLLInst,"_grLfbWriteColorFormat@4");
	if(! grLfbWriteColorFormat) return FALSE;
	grLfbWriteColorSwizzle = (grLfbWriteColorSwizzle_fpt)GetProcAddress(glideDLLInst,"_grLfbWriteColorSwizzle@8");
	if(! grLfbWriteColorSwizzle) return FALSE;
	grLfbWriteRegion = (grLfbWriteRegion_fpt)GetProcAddress(glideDLLInst,"_grLfbWriteRegion@32");
	if(! grLfbWriteRegion) return FALSE;
	grRenderBuffer = (grRenderBuffer_fpt)GetProcAddress(glideDLLInst,"_grRenderBuffer@4");
	if(! grRenderBuffer) return FALSE;
	grResetTriStats = (grResetTriStats_fpt)GetProcAddress(glideDLLInst,"_grResetTriStats@0");
	if(! grResetTriStats) return FALSE;
	grSplash = (grSplash_fpt)GetProcAddress(glideDLLInst,"_grSplash@20");
	if(! grSplash) return FALSE;
	grSstControl = (grSstControl_fpt)GetProcAddress(glideDLLInst,"_grSstControl@4");
	if(! grSstControl) return FALSE;
	grSstIdle = (grSstIdle_fpt)GetProcAddress(glideDLLInst,"_grSstIdle@0");
	if(! grSstIdle) return FALSE;
	grSstIsBusy = (grSstIsBusy_fpt)GetProcAddress(glideDLLInst,"_grSstIsBusy@0");
	if(! grSstIsBusy) return FALSE;
	grSstOrigin = (grSstOrigin_fpt)GetProcAddress(glideDLLInst,"_grSstOrigin@4");
	if(! grSstOrigin) return FALSE;
	grSstPerfStats = (grSstPerfStats_fpt)GetProcAddress(glideDLLInst,"_grSstPerfStats@4");
	if(! grSstPerfStats) return FALSE;
	grSstQueryBoards = (grSstQueryBoards_fpt)GetProcAddress(glideDLLInst,"_grSstQueryBoards@4");
	if(! grSstQueryBoards) return FALSE;
	grSstQueryHardware = (grSstQueryHardware_fpt)GetProcAddress(glideDLLInst,"_grSstQueryHardware@4");
	if(! grSstQueryHardware) return FALSE;
	grSstResetPerfStats = (grSstResetPerfStats_fpt)GetProcAddress(glideDLLInst,"_grSstResetPerfStats@0");
	if(! grSstResetPerfStats) return FALSE;
	grSstScreenHeight = (grSstScreenHeight_fpt)GetProcAddress(glideDLLInst,"_grSstScreenHeight@0");
	if(! grSstScreenHeight) return FALSE;
	grSstScreenWidth = (grSstScreenWidth_fpt)GetProcAddress(glideDLLInst,"_grSstScreenWidth@0");
	if(! grSstScreenWidth) return FALSE;
	grSstSelect = (grSstSelect_fpt)GetProcAddress(glideDLLInst,"_grSstSelect@4");
	if(! grSstSelect) return FALSE;
	grSstStatus = (grSstStatus_fpt)GetProcAddress(glideDLLInst,"_grSstStatus@0");
	if(! grSstStatus) return FALSE;
	grSstVRetraceOn = (grSstVRetraceOn_fpt)GetProcAddress(glideDLLInst,"_grSstVRetraceOn@0");
	if(! grSstVRetraceOn) return FALSE;
	grSstVideoLine = (grSstVideoLine_fpt)GetProcAddress(glideDLLInst,"_grSstVideoLine@0");
	if(! grSstVideoLine) return FALSE;
	grSstWinClose = (grSstWinClose_fpt)GetProcAddress(glideDLLInst,"_grSstWinClose@0");
	if(! grSstWinClose) return FALSE;
	grSstWinOpen = (grSstWinOpen_fpt)GetProcAddress(glideDLLInst,"_grSstWinOpen@28");
	if(! grSstWinOpen) return FALSE;
	grTexCalcMemRequired = (grTexCalcMemRequired_fpt)GetProcAddress(glideDLLInst,"_grTexCalcMemRequired@16");
	if(! grTexCalcMemRequired) return FALSE;
	grTexClampMode = (grTexClampMode_fpt)GetProcAddress(glideDLLInst,"_grTexClampMode@12");
	if(! grTexClampMode) return FALSE;
	grTexCombine = (grTexCombine_fpt)GetProcAddress(glideDLLInst,"_grTexCombine@28");
	if(! grTexCombine) return FALSE;
	grTexCombineFunction = (grTexCombineFunction_fpt)GetProcAddress(glideDLLInst,"_grTexCombineFunction@8");
	if(! grTexCombineFunction) return FALSE;
	grTexDetailControl = (grTexDetailControl_fpt)GetProcAddress(glideDLLInst,"_grTexDetailControl@16");
	if(! grTexDetailControl) return FALSE;
	grTexDownloadMipMap = (grTexDownloadMipMap_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadMipMap@16");
	if(! grTexDownloadMipMap) return FALSE;
	grTexDownloadMipMapLevel = (grTexDownloadMipMapLevel_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadMipMapLevel@32");
	if(! grTexDownloadMipMapLevel) return FALSE;
	grTexDownloadMipMapLevelPartial = (grTexDownloadMipMapLevelPartial_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadMipMapLevelPartial@40");
	if(! grTexDownloadMipMapLevelPartial) return FALSE;
	grTexDownloadTable = (grTexDownloadTable_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadTable@12");
	if(! grTexDownloadTable) return FALSE;
	grTexDownloadTablePartial = (grTexDownloadTablePartial_fpt)GetProcAddress(glideDLLInst,"_grTexDownloadTablePartial@20");
	if(! grTexDownloadTablePartial) return FALSE;
	grTexFilterMode = (grTexFilterMode_fpt)GetProcAddress(glideDLLInst,"_grTexFilterMode@12");
	if(! grTexFilterMode) return FALSE;
	grTexLodBiasValue = (grTexLodBiasValue_fpt)GetProcAddress(glideDLLInst,"_grTexLodBiasValue@8");
	if(! grTexLodBiasValue) return FALSE;
	grTexMaxAddress = (grTexMaxAddress_fpt)GetProcAddress(glideDLLInst,"_grTexMaxAddress@4");
	if(! grTexMaxAddress) return FALSE;
	grTexMinAddress = (grTexMinAddress_fpt)GetProcAddress(glideDLLInst,"_grTexMinAddress@4");
	if(! grTexMinAddress) return FALSE;
	grTexMipMapMode = (grTexMipMapMode_fpt)GetProcAddress(glideDLLInst,"_grTexMipMapMode@12");
	if(! grTexMipMapMode) return FALSE;
	grTexMultibase = (grTexMultibase_fpt)GetProcAddress(glideDLLInst,"_grTexMultibase@8");
	if(! grTexMultibase) return FALSE;
	grTexMultibaseAddress = (grTexMultibaseAddress_fpt)GetProcAddress(glideDLLInst,"_grTexMultibaseAddress@20");
	if(! grTexMultibaseAddress) return FALSE;
	grTexNCCTable = (grTexNCCTable_fpt)GetProcAddress(glideDLLInst,"_grTexNCCTable@8");
	if(! grTexNCCTable) return FALSE;
	grTexSource = (grTexSource_fpt)GetProcAddress(glideDLLInst,"_grTexSource@16");
	if(! grTexSource) return FALSE;
	grTexTextureMemRequired = (grTexTextureMemRequired_fpt)GetProcAddress(glideDLLInst,"_grTexTextureMemRequired@8");
	if(! grTexTextureMemRequired) return FALSE;
	grTriStats = (grTriStats_fpt)GetProcAddress(glideDLLInst,"_grTriStats@8");
	if(! grTriStats) return FALSE;
	guTexAllocateMemory = (guTexAllocateMemory_fpt)GetProcAddress(glideDLLInst,"_guTexAllocateMemory@60");
	if(! guTexAllocateMemory) return FALSE;
	guTexChangeAttributes = (guTexChangeAttributes_fpt)GetProcAddress(glideDLLInst,"_guTexChangeAttributes@48");
	if(! guTexChangeAttributes) return FALSE;
	guTexCombineFunction = (guTexCombineFunction_fpt)GetProcAddress(glideDLLInst,"_guTexCombineFunction@8");
	if(! guTexCombineFunction) return FALSE;
	guTexDownloadMipMap = (guTexDownloadMipMap_fpt)GetProcAddress(glideDLLInst,"_guTexDownloadMipMap@12");
	if(! guTexDownloadMipMap) return FALSE;
	guTexDownloadMipMapLevel = (guTexDownloadMipMapLevel_fpt)GetProcAddress(glideDLLInst,"_guTexDownloadMipMapLevel@12");
	if(! guTexDownloadMipMapLevel) return FALSE;
	guTexGetCurrentMipMap = (guTexGetCurrentMipMap_fpt)GetProcAddress(glideDLLInst,"_guTexGetCurrentMipMap@4");
	if(! guTexGetCurrentMipMap) return FALSE;
	guTexGetMipMapInfo = (guTexGetMipMapInfo_fpt)GetProcAddress(glideDLLInst,"_guTexGetMipMapInfo@4");
	if(! guTexGetMipMapInfo) return FALSE;
	guTexMemQueryAvail = (guTexMemQueryAvail_fpt)GetProcAddress(glideDLLInst,"_guTexMemQueryAvail@4");
	if(! guTexMemQueryAvail) return FALSE;
	guTexMemReset = (guTexMemReset_fpt)GetProcAddress(glideDLLInst,"_guTexMemReset@0");
	if(! guTexMemReset) return FALSE;
	guTexSource = (guTexSource_fpt)GetProcAddress(glideDLLInst,"_guTexSource@4");
	if(! guTexSource) return FALSE;

	return TRUE;
}
extern "C" {
void 
#ifdef GFX_DLL
__declspec(dllexport)
#endif
__cdecl UnloadGlide()
{
   if (glideDLLInst) {
      if (grGlideShutdown != NULL)
         grGlideShutdown();

      FreeLibrary(glideDLLInst);
   }
}
}
