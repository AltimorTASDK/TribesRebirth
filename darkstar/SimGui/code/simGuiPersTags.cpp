#include "simGuiCanvas.h"
#include "simGuiCtrl.h"
#include "simGuiTestCtrl.h"
#include "simGuiBitmapCtrl.h"
#include "simGuiPaletteCtrl.h"
#include "simGuiActiveCtrl.h"
#include "simGuiTSCtrl.h"
#include "simGuiSimpleText.h"
#include "simGuiTestButton.h"
#include "simGuiTestRadial.h"
#include "simGuiTestCheck.h"
#include "simGuiTextEdit.h"
#include "simGuiTimerCtrl.h"
#include "simGuiBitmapBox.h"
#include "simGuiTextList.h"
#include "simGuiSlider.h"
#include "simGuiComboBox.h"
#include "simGuiScrollCtrl.h"
#include "simGuiMatrixCtrl.h"
#include "simGuiTextWrap.h"
#include "simGuiTextFormat.h"
#include "simGuiArrayCtrl.h"
#include "simguiprogressctrl.h"
#include "SimGuiAnimateBMA.h"
#include "SimGuiHelpCtrl.h"
#include "SimGuiEditCtrl.h"

IMPLEMENT_PERSISTENT_TAG( SimGui::Control,     FOURCC('S','G','c','t') );
IMPLEMENT_PERSISTENT_TAG( SimGui::ArrayCtrl,   FOURCC('S','G','A','c') );
IMPLEMENT_PERSISTENT_TAG( SimGui::TestControl, FOURCC('S','G','t','c') );
IMPLEMENT_PERSISTENT_TAG( SimGui::BitmapCtrl,  FOURCC('S','G','b','m') );
IMPLEMENT_PERSISTENT_TAG( SimGui::PaletteCtrl, FOURCC('S','G','p','l') );
IMPLEMENT_PERSISTENT_TAG( SimGui::ActiveCtrl,  FOURCC('S','G','a','c') );
IMPLEMENT_PERSISTENT_TAG( SimGui::SimpleText,  FOURCC('S','G','s','t') );
IMPLEMENT_PERSISTENT_TAG( SimGui::TestButton,  FOURCC('S','G','t','b') );
IMPLEMENT_PERSISTENT_TAG( SimGui::TestRadial,  FOURCC('S','G','t','r') );
IMPLEMENT_PERSISTENT_TAG( SimGui::TestCheck,   FOURCC('S','G','t','k') );
IMPLEMENT_PERSISTENT_TAG( SimGui::TSControl,   FOURCC('S','G','t','s') );
IMPLEMENT_PERSISTENT_TAG( SimGui::TextEdit,    FOURCC('S','G','t','e') );
IMPLEMENT_PERSISTENT_TAG( SimGui::TimerCtrl,   FOURCC('S','G','t','m') );
IMPLEMENT_PERSISTENT_TAG( SimGui::BitmapBox,   FOURCC('S','G','b','b') );
IMPLEMENT_PERSISTENT_TAG( SimGui::TextList,    FOURCC('S','G','t','l') );
IMPLEMENT_PERSISTENT_TAG( SimGui::TextWrap,    FOURCC('S','G','t','w') );
IMPLEMENT_PERSISTENT_TAG( SimGui::TextFormat,  FOURCC('S','G','t','f') );
IMPLEMENT_PERSISTENT_TAG( SimGui::Slider,      FOURCC('S','G','s','l') );
IMPLEMENT_PERSISTENT(SimGui::Canvas);
IMPLEMENT_PERSISTENT_TAG( SimGui::ComboBox,    FOURCC('S','G','c','b') );

IMPLEMENT_PERSISTENT_TAG( SimGui::ScrollCtrl,         FOURCC('S','G','s','c') );
IMPLEMENT_PERSISTENT_TAG( SimGui::ScrollContentCtrl,  FOURCC('S','G','s','C') );
IMPLEMENT_PERSISTENT_TAG( SimGui::MatrixCtrl,         FOURCC('S','G','m','c') );
IMPLEMENT_PERSISTENT_TAG( SimGui::ProgressCtrl,       FOURCC('S', 'G', 'p', 'c') );
IMPLEMENT_PERSISTENT_TAG( SimGui::AnimateBMA,         FOURCC('S', 'G', 'a', 'b') );
IMPLEMENT_PERSISTENT_TAG( SimGui::HelpCtrl,           FOURCC('S', 'G', 'h', 'c') );

IMPLEMENT_PERSISTENT(SimGui::EditControl);

namespace SimGui
{
int dummy_link;
};