#if _WIN32
#include <windows.h>
#pragma comment(lib, "Winmm.lib")
#include <Mmreg.h>
#pragma comment(lib, "dsound.lib")
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

////////////////////////////////////
// FLTK Includes
///////////////////////////////////
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Choice.H>
//#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/fl_draw.H> 
#include <FL/Fl_Input.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/x.H>

////////////////////////////////////
// BasicSynth Includes
///////////////////////////////////
#include <BasicSynth.h>
#if !NO_LIVE_PLAY
# if _WIN32
#  include <dsound.h>
#  include <WaveOutDirect.h>
# endif
# if UNIX
#  include <WaveOutALSA.h>
#  include <alsa/asoundlib.h>
# endif
#endif
#include <Instruments.h>
#include <NLConvert.h>
#include <ComposerCore.h>

