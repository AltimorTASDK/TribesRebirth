function createToolBar()
{
  	// --------------------------------
	// Create the general main edit bar
	// --------------------------------

	addToolButton(GuiEditBar,	LeftJustify,	"tb_leftJustify.bmp",		0, 0);
	addToolButton(GuiEditBar,	RightJustify,	"tb_rightJustify.bmp",		0, 0);
	addToolButton(GuiEditBar,	CenterJustify,	"tb_centerJustify.bmp",		0, 0);
	
	addToolGap(GuiEditBar, gap1);
	
	addToolButton(GuiEditBar,	TopAlign,		"tb_topAlign.bmp",			0,	0); 
	addToolButton(GuiEditBar,	BottomAlign,	"tb_bottomAlign.bmp",		0,	0); 
	
	addToolGap(GuiEditBar, gap2);
	
	addToolButton(GuiEditBar,	AutoVSpace,    "tb_vSpace.bmp",				0, 0); 
	addToolButton(GuiEditBar,	AutoHSpace,    "tb_hSpace.bmp",				0, 0);
	
	addToolGap(GuiEditBar, gap3);
	
	addToolButton(GuiEditBar,	BringToFront,  "tb_front.bmp",				0, 0);
	addToolButton(GuiEditBar,	SendToBack,    "tb_back.bmp",					0, 0);
	
	addToolGap(GuiEditBar, gap4);
	
	addToolButton(GuiEditBar,	EditMode,      "tb_edit.bmp",					0, 0);
	
	addToolGap(GuiEditBar, gap6);
	
	addToolButton(GuiEditBar,	KillBar,       "tb_killTB.bmp",				0, 0);
	
	// --------------------------------------------
	// set commands for each button in the edit bar
	// --------------------------------------------

	setToolCommand(GuiEditBar, LeftJustify,    "GuiJustify($SimGui::TBRoot, 0);");
	setToolCommand(GuiEditBar, RightJustify,   "GuiJustify($SimGui::TBRoot, 2);");
	setToolCommand(GuiEditBar, CenterJustify,  "GuiJustify($SimGui::TBRoot, 1);");
	setToolCommand(GuiEditBar, TopAlign,       "GuiJustify($SimGui::TBRoot, 3);");
	setToolCommand(GuiEditBar, BottomAlign,    "GuiJustify($SimGui::TBRoot, 4);");
	setToolCommand(GuiEditBar, AutoVSpace,     "GuiJustify($SimGui::TBRoot, 5);");
	setToolCommand(GuiEditBar, AutoHSpace,     "GuiJustify($SimGui::TBRoot, 6);");
	setToolCommand(GuiEditBar, BringToFront,   "GuiBringToFront($SimGui::TBRoot);");
	setToolCommand(GuiEditBar, SendToBack,     "GuiSendToBack($SimGui::TBRoot);");
	setToolCommand(GuiEditBar, EditMode,       "GuiEditMode($SimGui::TBRoot);");
	setToolCommand(GuiEditBar, KillBar,        "deleteObject(GuiEditBar);");
}

createToolBar();

