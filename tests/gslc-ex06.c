//
// microSDL GUI Library Examples
// - Calvin Hass
// - http://www.impulseadventure.com/elec/microsdl-sdl-gui.html
// - Example 06: Example of viewport and image draw
//

#include "microsdl.h"
#include "microsdl_ex.h"

#include <math.h>
#include <libgen.h>       // For path parsing

// Defines for resources
#define FONT_DROID_SANS "/usr/share/fonts/truetype/droid/DroidSans.ttf"
#define IMG_LOGO        "/res/logo1-200x40.bmp"

// Enumerations for pages, elements, fonts, images
enum {E_PG_MAIN};
enum {E_ELEM_BTN_QUIT,E_ELEM_TXT_COUNT,E_ELEM_PROGRESS,
      E_ELEM_DATAX,E_ELEM_DATAY,E_ELEM_DATAZ,E_ELEM_SCAN};
enum {E_FONT_BTN,E_FONT_TXT};
enum {E_VIEW};

bool                  m_bQuit = false;

// Free-running counter for display
unsigned  m_nCount = 0;

float     m_fCoordX = 0;
float     m_fCoordY = 0;
float     m_fCoordZ = 0;

// Instantiate the GUI
#define MAX_FONT    10
#define MAX_VIEW    5
microSDL_tsGui      m_gui;
microSDL_tsFont     m_asFont[MAX_FONT];
microSDL_tsView     m_asView[MAX_VIEW];
microSDL_tsXGauge   m_sXGauge;

#define MAX_PAGE            1
#define MAX_ELEM_PG_MAIN    30
microSDL_tsPage             m_asPage[MAX_PAGE];
microSDL_tsElem             m_asPageElem[MAX_ELEM_PG_MAIN];

#define MAX_STR             100


// Scanner drawing callback function
// - This is called when E_ELEM_SCAN is being rendered
// - The scanner implements a custom element that replaces
//   the Box element type with a custom rendering function.
// - It uses a viewport (E_VIEW) to allow for a local
//   coordinate system and clipping.
bool CbDrawScanner(void* pvGui,void* pvElem)
{
  int nInd;

  // Typecast the parameters to match the GUI and element types
  microSDL_tsGui*   pGui  = (microSDL_tsGui*)(pvGui);
  microSDL_tsElem*  pElem = (microSDL_tsElem*)(pvElem);
  
  // Draw the background
  microSDL_DrawFillRect(pGui,pElem->rElem,pElem->colElemFill);
  
  // Draw the frame
  microSDL_DrawFrameRect(pGui,pElem->rElem,pElem->colElemFrame);
  

  // Switch to drawing coordinate space within the viewport
  // - Until the next ViewSet() command, all drawing
  //   will be done with local coordinates that
  //   are remapped and clipped to match the viewport.
  microSDL_ViewSet(&m_gui,E_VIEW);

  // Perform the drawing of example graphic primitives
  microSDL_DrawLine(&m_gui,0,-200,0,+200,MSDL_COL_GRAY_DK);
  microSDL_DrawLine(&m_gui,-200,0,+200,0,MSDL_COL_GRAY_DK);

  microSDL_DrawFrameRect(&m_gui,(microSDL_Rect){-30,-20,60,40},MSDL_COL_BLUE_DK);
  for (nInd=-5;nInd<=5;nInd++) {
    microSDL_DrawLine(&m_gui,0,0,0+nInd*20,100,MSDL_COL_PURPLE);
  }

  microSDL_DrawFillRect(&m_gui,(microSDL_Rect){1,1,10,10},MSDL_COL_RED_DK);
  microSDL_DrawFillRect(&m_gui,(microSDL_Rect){1,-10,10,10},MSDL_COL_GREEN_DK);
  microSDL_DrawFillRect(&m_gui,(microSDL_Rect){-10,1,10,10},MSDL_COL_BLUE_DK);
  microSDL_DrawFillRect(&m_gui,(microSDL_Rect){-10,-10,10,10},MSDL_COL_YELLOW);

  // Restore the drawing coordinate space to the screen
  microSDL_ViewSet(&m_gui,MSDL_VIEW_ID_SCREEN);
  
  // Clear the redraw flag
  microSDL_ElemSetRedraw(pElem,false);
  
  return true;
}

// Button callbacks
bool CbBtnQuit(void* pvGui,void *pvElem,microSDL_teTouch eTouch,int nX,int nY)
{
  if (eTouch == MSDL_TOUCH_UP_IN) {
    m_bQuit = true;
  }
  return true;
}

// Create the default elements on each page
// - strPath: Path to executable passed in to locate resource files
bool InitOverlays(char *strPath)
{
  microSDL_tsElem*  pElem = NULL;

  microSDL_PageAdd(&m_gui,E_PG_MAIN,m_asPageElem,MAX_ELEM_PG_MAIN);
  
  // Background flat color
  microSDL_SetBkgndColor(&m_gui,MSDL_COL_BLACK);

  // Graphic logo
  // - Extra code to demonstrate path generation based on location of executable  
  char* strImgLogoPath = (char*)malloc(strlen(strPath)+strlen(IMG_LOGO)+1);
  strcpy(strImgLogoPath, strPath);
  strcat(strImgLogoPath, IMG_LOGO);  
  pElem = microSDL_ElemCreateImg(&m_gui,MSDL_ID_AUTO,E_PG_MAIN,(microSDL_Rect){160-100,5,200,40},
    strImgLogoPath);

  // Create background box
  pElem = microSDL_ElemCreateBox(&m_gui,MSDL_ID_AUTO,E_PG_MAIN,(microSDL_Rect){10,50,300,150});
  microSDL_ElemSetCol(pElem,MSDL_COL_WHITE,MSDL_COL_BLACK,MSDL_COL_BLACK);

  // Create Quit button with text label
  pElem = microSDL_ElemCreateBtnTxt(&m_gui,E_ELEM_BTN_QUIT,E_PG_MAIN,
    (microSDL_Rect){40,210,50,20},"QUIT",E_FONT_BTN,&CbBtnQuit);

  // Create counter
  pElem = microSDL_ElemCreateTxt(&m_gui,MSDL_ID_AUTO,E_PG_MAIN,(microSDL_Rect){20,60,50,10},
    "Searches:",E_FONT_TXT);
  pElem = microSDL_ElemCreateTxt(&m_gui,E_ELEM_TXT_COUNT,E_PG_MAIN,(microSDL_Rect){80,60,50,10},
    "",E_FONT_TXT);
  microSDL_ElemSetTxtCol(pElem,MSDL_COL_GRAY_LT);

  // Create progress bar
  pElem = microSDL_ElemCreateTxt(&m_gui,MSDL_ID_AUTO,E_PG_MAIN,(microSDL_Rect){20,80,50,10},
    "Progress:",E_FONT_TXT);
  pElem = microSDL_ElemXGaugeCreate(&m_gui,E_ELEM_PROGRESS,E_PG_MAIN,&m_sXGauge,(microSDL_Rect){80,80,50,10},
    0,100,0,MSDL_COL_GREEN_DK,false);

  
  // Create other labels
  pElem = microSDL_ElemCreateTxt(&m_gui,MSDL_ID_AUTO,E_PG_MAIN,(microSDL_Rect){40,100,50,10},
    "Coord X:",E_FONT_TXT);
  microSDL_ElemSetTxtCol(pElem,MSDL_COL_WHITE);
  pElem = microSDL_ElemCreateTxt(&m_gui,E_ELEM_DATAX,E_PG_MAIN,(microSDL_Rect){100,100,50,10},
    "",E_FONT_TXT);
  microSDL_ElemSetTxtCol(pElem,MSDL_COL_GRAY_LT);

  pElem = microSDL_ElemCreateTxt(&m_gui,MSDL_ID_AUTO,E_PG_MAIN,(microSDL_Rect){40,120,50,10},
    "Coord Y:",E_FONT_TXT);
  microSDL_ElemSetTxtCol(pElem,MSDL_COL_WHITE);
  pElem = microSDL_ElemCreateTxt(&m_gui,E_ELEM_DATAY,E_PG_MAIN,(microSDL_Rect){100,120,50,10},
    "",E_FONT_TXT);
  microSDL_ElemSetTxtCol(pElem,MSDL_COL_GRAY_LT);

  pElem = microSDL_ElemCreateTxt(&m_gui,MSDL_ID_AUTO,E_PG_MAIN,(microSDL_Rect){40,140,50,10},
    "Coord Z:",E_FONT_TXT);
  microSDL_ElemSetTxtCol(pElem,MSDL_COL_WHITE);
  pElem = microSDL_ElemCreateTxt(&m_gui,E_ELEM_DATAZ,E_PG_MAIN,(microSDL_Rect){100,140,50,10},
    "",E_FONT_TXT);
  microSDL_ElemSetTxtCol(pElem,MSDL_COL_GRAY_LT);

  pElem = microSDL_ElemCreateTxt(&m_gui,MSDL_ID_AUTO,E_PG_MAIN,(microSDL_Rect){120,210,170,20},
    "Example of microSDL GUI C library",E_FONT_BTN);
  microSDL_ElemSetTxtAlign(pElem,MSDL_ALIGN_MID_LEFT);
  microSDL_ElemSetTxtCol(pElem,MSDL_COL_RED_LT);

  // --------------------------------------------------------------------------
  // Create scanner with viewport
  pElem = microSDL_ElemCreateBox(&m_gui,MSDL_ID_AUTO,E_PG_MAIN,(microSDL_Rect){190-1-2,75-1-12,100+2+4,100+2+10+4});
  microSDL_ElemSetCol(pElem,MSDL_COL_BLUE_LT,MSDL_COL_BLACK,MSDL_COL_BLACK);
  
  pElem = microSDL_ElemCreateTxt(&m_gui,MSDL_ID_AUTO,E_PG_MAIN,(microSDL_Rect){190,75-11,100,10},
    "SCANNER",E_FONT_TXT);
  microSDL_ElemSetTxtCol(pElem,MSDL_COL_BLUE_DK);
  microSDL_ElemSetTxtAlign(pElem,MSDL_ALIGN_MID_MID);
  
  pElem = microSDL_ElemCreateBox(&m_gui,E_ELEM_SCAN,E_PG_MAIN,(microSDL_Rect){190-1,75-1,100+2,100+2});
  microSDL_ElemSetCol(pElem,MSDL_COL_BLUE_LT,MSDL_COL_BLACK,MSDL_COL_BLACK);
  // Set the callback function to handle all drawing for the element
  microSDL_ElemSetDrawFunc(pElem,&CbDrawScanner);
  
  microSDL_ViewCreate(&m_gui,E_VIEW,(microSDL_Rect){190,75,100,100},50,50);
  // --------------------------------------------------------------------------

  return true;
}

int main( int argc, char* args[] )
{
  bool              bOk = true;
  char              acTxt[100];

  // -----------------------------------
  // Initialize

  microSDL_InitEnv(&m_gui);
  if (!microSDL_Init(&m_gui,m_asPage,MAX_PAGE,m_asFont,MAX_FONT,m_asView,MAX_VIEW)) { exit(1); }

  microSDL_InitTs(&m_gui,"/dev/input/touchscreen");

  // Load Fonts
  bOk = microSDL_FontAdd(&m_gui,E_FONT_BTN,FONT_DROID_SANS,12);
  if (!bOk) { fprintf(stderr,"ERROR: FontAdd failed\n"); exit(1); }
  bOk = microSDL_FontAdd(&m_gui,E_FONT_TXT,FONT_DROID_SANS,10);
  if (!bOk) { fprintf(stderr,"ERROR: FontAdd failed\n"); exit(1); }


  // -----------------------------------
  // Create the graphic elements
  InitOverlays(dirname(args[0])); // Pass executable path to find resource files

  // Start up display on main page
  microSDL_SetPageCur(&m_gui,E_PG_MAIN);

  // Save some element references for quick access
  microSDL_tsElem*  pElemScan       = microSDL_PageFindElemById(&m_gui,E_PG_MAIN,E_ELEM_SCAN);
  microSDL_tsElem*  pElemCount      = microSDL_PageFindElemById(&m_gui,E_PG_MAIN,E_ELEM_TXT_COUNT);
  microSDL_tsElem*  pElemDataX      = microSDL_PageFindElemById(&m_gui,E_PG_MAIN,E_ELEM_DATAX);
  microSDL_tsElem*  pElemDataY      = microSDL_PageFindElemById(&m_gui,E_PG_MAIN,E_ELEM_DATAY);
  microSDL_tsElem*  pElemDataZ      = microSDL_PageFindElemById(&m_gui,E_PG_MAIN,E_ELEM_DATAZ);
  microSDL_tsElem*  pElemProgress   = microSDL_PageFindElemById(&m_gui,E_PG_MAIN,E_ELEM_PROGRESS);
  
  // -----------------------------------
  // Main event loop

  m_bQuit = false;
  while (!m_bQuit) {
    
    // Update the data display values
    m_nCount++;
    m_fCoordX = 50+25.0*(sin(m_nCount/250.0));
    m_fCoordY = 50+15.0*(cos(m_nCount/175.0));
    m_fCoordZ = 13.02;

    // Adjust the scanner's viewport origin for fun
    int16_t nOriginX = (int16_t)m_fCoordX;
    int16_t nOriginY = (int16_t)m_fCoordY;
    microSDL_ViewSetOrigin(&m_gui,E_VIEW,nOriginX,nOriginY);
    // Manually mark the scanner view as needing redraw
    // since it depends on E_VIEW
    microSDL_ElemSetRedraw(pElemScan,true);

    // -----------------------------------------------

    // Perform any immediate updates on active page
    snprintf(acTxt,MAX_STR,"%u",m_nCount);
    microSDL_ElemSetTxtStr(pElemCount,acTxt);

    snprintf(acTxt,MAX_STR,"%4.2f",m_fCoordX-50);
    microSDL_ElemSetTxtStr(pElemDataX,acTxt);
    snprintf(acTxt,MAX_STR,"%4.2f",m_fCoordY-50);
    microSDL_ElemSetTxtStr(pElemDataY,acTxt);
    snprintf(acTxt,MAX_STR,"%4.2f",m_fCoordZ);
    microSDL_ElemSetTxtStr(pElemDataZ,acTxt);
    microSDL_ElemSetTxtCol(pElemDataZ,(m_fCoordY>50)?MSDL_COL_GREEN_LT:MSDL_COL_RED_DK);

    microSDL_ElemXGaugeUpdate(pElemProgress,50+50*sin(m_nCount/500.0));

    // -----------------------------------------------

    // Periodically call microSDL update function    
    microSDL_Update(&m_gui);

  } // bQuit


  // -----------------------------------
  // Close down display

  microSDL_Quit(&m_gui);

  return 0;
}
