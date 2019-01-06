
// Header files
#include "nvafx.h"
#include "GUI.h"
#include "Scene.h"
#include <vector>

namespace GUI
{

/*----------------------------------------------------------------------------------------------------------------------
    Menu button
 ----------------------------------------------------------------------------------------------------------------------- */

#define DXUT_CONTROL_MENU_BUTTON    static_cast<DXUT_CONTROL_TYPE>(DXUT_CONTROL_SCROLLBAR + 1)
class CDXUTMenuButton : public CDXUTButton
{

public:
    CDXUTMenuButton(CDXUTDialog*, bool);
    virtual bool HandleKeyboard(UINT, WPARAM, LPARAM);
    virtual bool HandleMouse(UINT, POINT, WPARAM, LPARAM);
    virtual void OnHotkey() { SetOpen(!m_Open); }
    virtual void Render(IDirect3DDevice9*, float);
    virtual void UpdateRects();
    virtual BOOL ContainsPoint(POINT);
    bool IsOpen() const { return m_Open; }
    void SetOpen(bool);
    void SetDialogVisible(bool);

protected:
    bool m_Open;
    bool m_HasMoved;
    RECT m_rcText;
    RECT m_rcButton;
    POINT m_LastMouse;
    POINT m_LastLocation;
};

/*----------------------------------------------------------------------------------------------------------------------
    Menu
 ----------------------------------------------------------------------------------------------------------------------- */

class CDXUTMenu : public CDXUTDialog
{

public:
    void Initialize(LPCWSTR, int, int, int, int, PCALLBACKDXUTGUIEVENT = 0, bool = false);
    void AddBoldStatic(int, LPCWSTR);
    void Reset(int, int);
    void ResetItems();
    void SetVisible(bool);
    bool IsOpen() { return static_cast<CDXUTMenuButton*>(GetControl(0))->IsOpen(); }
    void SetOpen(bool value) { static_cast<CDXUTMenuButton*>(GetControl(0))->SetOpen(value); }
    static int ItemWidth() { return m_ItemWidth; }

private:
    static const int m_ItemWidth;
    static const int m_ItemHeight;
    void AddMenuButton(LPCWSTR, bool, int = 0, bool = false);
};
const int CDXUTMenu::m_ItemWidth = 165;    // 155;
const int CDXUTMenu::m_ItemHeight = 22;

/*----------------------------------------------------------------------------------------------------------------------
    Bold static control
 ----------------------------------------------------------------------------------------------------------------------- */

#define DXUT_CONTROL_BOLD_STATIC    static_cast<DXUT_CONTROL_TYPE>(DXUT_CONTROL_SCROLLBAR + 2)
class CDXUTBoldStatic : public CDXUTStatic
{

public:
    CDXUTBoldStatic(CDXUTDialog*);
};

/*----------------------------------------------------------------------------------------------------------------------
    Menu IDs
 ----------------------------------------------------------------------------------------------------------------------- */

enum
{
    MENU_RENDER,
    MENU_SIMULATE,
    MENU_EDIT,
    MENU_MOUSE,
    MENU_DEVICE,
    MENU_NUM,
};

enum
{
    RENDER_CAMERA_CENTER_LABEL  = 1,
    RENDER_CAMERA_CENTER,
    RENDER_CURTAIN,
    RENDER_FLAG,
    RENDER_CAPE,
    RENDER_SKIRT,
    RENDER_CHARACTER_LABEL,
    RENDER_CHARACTER,
    RENDER_WIREFRAME_LABEL,
    RENDER_WIREFRAME,
    RENDER_CLOTH_NORMALS,
    RENDER_THINNING_LABEL,
    RENDER_THINNING,
    RENDER_TOGGLEFULLSCREEN,
};

enum
{
    SIMULATE_RUN                                = 1,
    SIMULATE_STEP,
    SIMULATE_RESET,
    SIMULATE_FIXED_TIMESTEP,
    SIMULATE_RATE_LABEL,
    SIMULATE_RATE,
    SIMULATE_CLOTH_SELECTION_FREE,
    SIMULATE_CURTAIN,
    SIMULATE_CURTAIN_WIDTH_LABEL,
    SIMULATE_CURTAIN_WIDTH,
    SIMULATE_CURTAIN_HEIGHT_LABEL,
    SIMULATE_CURTAIN_HEIGHT,
    SIMULATE_CURTAIN_RELAXATION_ITERATIONS_LABEL,
    SIMULATE_CURTAIN_RELAXATION_ITERATIONS,
    SIMULATE_CURTAIN_WITH_SHEAR,
    SIMULATE_CURTAIN_GRAVITY_LABEL,
    SIMULATE_CURTAIN_GRAVITY,
    SIMULATE_FLAG,
    SIMULATE_FLAG_WIDTH_LABEL,
    SIMULATE_FLAG_WIDTH,
    SIMULATE_FLAG_HEIGHT_LABEL,
    SIMULATE_FLAG_HEIGHT,
    SIMULATE_FLAG_RELAXATION_ITERATIONS_LABEL,
    SIMULATE_FLAG_RELAXATION_ITERATIONS,
    SIMULATE_FLAG_WITH_SHEAR,
    SIMULATE_FLAG_WIND_HEADING_LABEL,
    SIMULATE_FLAG_WIND_HEADING,
    SIMULATE_CAPE,
    SIMULATE_CAPE_WIDTH_LABEL,
    SIMULATE_CAPE_WIDTH,
    SIMULATE_CAPE_HEIGHT_LABEL,
    SIMULATE_CAPE_HEIGHT,
    SIMULATE_CAPE_RELAXATION_ITERATIONS_LABEL,
    SIMULATE_CAPE_RELAXATION_ITERATIONS,
    SIMULATE_CAPE_WITH_SHEAR,
    SIMULATE_CAPE_GRAVITY_LABEL,
    SIMULATE_CAPE_GRAVITY,
    SIMULATE_SKIRT,
    SIMULATE_SKIRT_RELAXATION_ITERATIONS_LABEL,
    SIMULATE_SKIRT_RELAXATION_ITERATIONS,
    SIMULATE_SKIRT_WITH_SHEAR,
    SIMULATE_SKIRT_GRAVITY_LABEL,
    SIMULATE_SKIRT_GRAVITY,
    SIMULATE_NUM,
};

enum
{
    EDIT_ADD_PLANE                              = 1,
    EDIT_ADD_SPHERE,
    EDIT_ADD_BOX,
    EDIT_ADD_ELLIPSOID,
    EDIT_SELECT_MODE,
    EDIT_UNSELECT_ALL,
    EDIT_REMOVE_SELECTION,
    EDIT_SCALE_SELECTION,
    EDIT_MOVE_SELECTION,
    EDIT_MOVE_ENVIRONMENT_ONLY,
    EDIT_KEEP_ROTATING,
    EDIT_CUT,
    EDIT_UNCUT,
};

enum
{
    MOUSE_LEFT_LABEL                            = 1,
    MOUSE_LEFT,
    MOUSE_MIDDLE_LABEL,
    MOUSE_MIDDLE,
    MOUSE_RIGHT_LABEL,
    MOUSE_RIGHT,
    MOUSE_LEFT_CLICK_LABEL,
    MOUSE_LEFT_CLICK,
    MOUSE_RIGHT_CLICK_LABEL,
    MOUSE_RIGHT_CLICK,
    MOUSE_DEFAULT,
};

enum
{
    DEVICE_TOGGLEREF                            = 1,
    DEVICE_CHANGEDEVICE,
};

enum
{
    NONE,
    ALL,
    ALWAYS,
    NEVER,
    CAMERA_CENTER_CURTAIN,
    CAMERA_CENTER_FLAG,
    CAMERA_CENTER_CHARACTER,
    CLOTHES_ONLY,
    ENVIRONMENT_ONLY,
    MODEL_ONLY,
    COLLISION_OBJECTS_ONLY,
    WHEN_DYNAMIC_SELECTION,
    WHEN_STATIC_SELECTION,
    ROTATE_CAMERA,
    PAN_SELECTION,
    DOLLY_SELECTION,
    ROTATE_SELECTION,
    X_SCALE_SELECTION,
    Y_SCALE_SELECTION,
    Z_SCALE_SELECTION,
    CUT,
    SELECT,
    NAIL,
};

/*----------------------------------------------------------------------------------------------------------------------
    Static variables
 ----------------------------------------------------------------------------------------------------------------------- */

// Menus
static CDXUTMenu g_Menu[MENU_NUM];
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg g_SettingsDlg;          // Device settings dialog

#define SHEAR_LABEL L"Shear springs"
#define GRAVITY_LABEL L"Gravity strength"
static void CALLBACK OnGUIEventViewMenu(unsigned int, int, CDXUTControl*, void*);
static void CALLBACK OnGUIEventSimulateMenu(unsigned int, int, CDXUTControl*, void*);
static void CALLBACK OnGUIEventEditMenu(unsigned int, int, CDXUTControl*, void*);
static void CALLBACK OnGUIEventMouseMenu(unsigned int, int, CDXUTControl*, void*);
static void CALLBACK OnGUIEventDeviceMenu(unsigned int, int, CDXUTControl*, void*);
static ID3DXFont* g_Font;
static ID3DXSprite* g_TextSprite;

// Simulation
static float g_TimeStep = 0.01f;
static bool g_IsPaused;
static int g_StepSimulation;
static bool g_ResetSimulation = false;

// Rendering
static D3DXVECTOR3 g_SceneCenter;
static D3DXMATRIX g_View;
static D3DXMATRIX g_ViewInv;
static D3DXMATRIX g_ViewProjection;
static CModelViewerCamera g_Camera;
static bool g_CharacterModel;
static bool g_CharacterCollisionObjects;
static bool g_WireframeCloth;
static bool g_WireframeEnvironment;
static LPDIRECT3DSURFACE9 g_IDRTSurface;
static LPDIRECT3DSURFACE9 g_WorldPositionRTSurface;
static bool g_ShowHelp;
static bool g_ShowUI = true;

// Selection
static int g_MouseX;
static int g_MouseY;
static float g_MouseIDX;
static float g_MouseIDY;
static unsigned short g_MouseID;
static bool g_SelectionIsMoved;
static bool g_SelectionIsScaled;

// Cut
static void Cut(IDirect3DDevice9*, HWND hWnd = 0, int = 0, LPARAM = 0);

// Backbuffer
static LPDIRECT3DSURFACE9 g_BackBuffer;
static int g_BackBufferWidth;
static int g_BackBufferHeight;

/*----------------------------------------------------------------------------------------------------------------------
    Initialization and cleanup
 ----------------------------------------------------------------------------------------------------------------------- */

void Initialize(int screenWidth, int screenHeight)
{

    // Scene
    Scene::Initialize();

    // Dialog
    g_SettingsDlg.Init(&g_DialogResourceManager);
    g_Menu[MENU_RENDER].Init(&g_DialogResourceManager);
    g_Menu[MENU_SIMULATE].Init(&g_DialogResourceManager);
    g_Menu[MENU_EDIT].Init(&g_DialogResourceManager);
    g_Menu[MENU_MOUSE].Init(&g_DialogResourceManager);
    g_Menu[MENU_DEVICE].Init(&g_DialogResourceManager);

    // Menu bar
    const int horizontalSpacing = CDXUTMenu::ItemWidth() + 20;
    int x = 10;
    int y = screenHeight - 30;
    g_Menu[MENU_RENDER].Initialize(L"Render", x, y, screenWidth, screenHeight, OnGUIEventViewMenu, false);
    x += horizontalSpacing;
    g_Menu[MENU_SIMULATE].Initialize(L"Simulate", x, y, screenWidth, screenHeight, OnGUIEventSimulateMenu, false);
    x += horizontalSpacing;
    g_Menu[MENU_EDIT].Initialize(L"Edit", x, y, screenWidth, screenHeight, OnGUIEventEditMenu);
    x += horizontalSpacing;
    g_Menu[MENU_MOUSE].Initialize(L"Mouse", x, y, screenWidth, screenHeight, OnGUIEventMouseMenu);
    x += horizontalSpacing;
    g_Menu[MENU_DEVICE].Initialize(L"Device", x, y, screenWidth, screenHeight, OnGUIEventDeviceMenu);
    CDXUTComboBox* combo;

    // Render menu
    g_Menu[MENU_RENDER].AddButton(RENDER_TOGGLEFULLSCREEN, L"Toggle full screen (Alt+Enter)", 0, 0, 0, 0);
    g_Menu[MENU_RENDER].AddStatic(RENDER_CAMERA_CENTER_LABEL, L"Camera center (C):", 0, 0, 0, 0);
    g_Menu[MENU_RENDER].AddComboBox(RENDER_CAMERA_CENTER, 0, 0, 0, 0, 'C', false, &combo);
    if (combo) {
        combo->AddItem(L"Curtain", IntToPtr(CAMERA_CENTER_CURTAIN));
        combo->AddItem(L"Flag", IntToPtr(CAMERA_CENTER_FLAG));
        combo->AddItem(L"Character", IntToPtr(CAMERA_CENTER_CHARACTER));
    }
    g_Menu[MENU_RENDER].AddCheckBox(RENDER_CURTAIN, L"Curtain (J)", 0, 0, 0, 0, true, 'J');
    g_Menu[MENU_RENDER].AddCheckBox(RENDER_FLAG, L"Flag (K)", 0, 0, 0, 0, true, 'K');
    g_Menu[MENU_RENDER].AddCheckBox(RENDER_CAPE, L"Cape (L)", 0, 0, 0, 0, true, 'L');
    g_Menu[MENU_RENDER].AddCheckBox(RENDER_SKIRT, L"Skirt (P)", 0, 0, 0, 0, true, 'P');
    g_Menu[MENU_RENDER].AddStatic(RENDER_CHARACTER_LABEL, L"Character (G):", 0, 0, 0, 0);
    g_Menu[MENU_RENDER].AddComboBox(RENDER_CHARACTER, 0, 0, 0, 0, 'G', false, &combo);
    if (combo) {
        combo->AddItem(L"Model only", IntToPtr(MODEL_ONLY));
        combo->AddItem(L"Collision objects only", IntToPtr(COLLISION_OBJECTS_ONLY));
        combo->AddItem(L"All", IntToPtr(ALL));
    }
    g_Menu[MENU_RENDER].AddStatic(RENDER_WIREFRAME_LABEL, L"Wireframe (W):", 0, 0, 0, 0);
    g_Menu[MENU_RENDER].AddComboBox(RENDER_WIREFRAME, 0, 0, 0, 0, 'W', false, &combo);
    if (combo) {
        combo->AddItem(L"None", IntToPtr(NONE));
        combo->AddItem(L"Cloth only", IntToPtr(CLOTHES_ONLY));
        combo->AddItem(L"Environment only", IntToPtr(ENVIRONMENT_ONLY));
        combo->AddItem(L"All", IntToPtr(ALL));
    }
    g_Menu[MENU_RENDER].AddCheckBox(RENDER_CLOTH_NORMALS, L"Cloth normals (N)", 0, 0, 0, 0, false, 'N');
    g_Menu[MENU_RENDER].AddStatic(RENDER_THINNING_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_RENDER].AddSlider(RENDER_THINNING, 0, 0, 0, 0, 0, 100, static_cast<int>(-1000 * Scene::g_Thinning));

    // Simulate menu
    g_Menu[MENU_SIMULATE].AddButton(SIMULATE_RUN, L"Pause (Space)", 0, 0, 0, 0, VK_SPACE);
    g_Menu[MENU_SIMULATE].AddButton(SIMULATE_STEP, L"Step (1..9)", 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddButton(SIMULATE_RESET, L"Reset (0)", 0, 0, 0, 0, '0');
    g_Menu[MENU_SIMULATE].AddCheckBox(SIMULATE_FIXED_TIMESTEP, L"Fixed time step", 0, 0, 0, 0, true);
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_RATE_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_RATE, 0, 0, 0, 0, 20, 200, static_cast<int>(1 / g_TimeStep));
    g_Menu[MENU_SIMULATE].AddCheckBox(SIMULATE_CLOTH_SELECTION_FREE, L"Cloth selection free (F)", 0, 0, 0, 0, false, 'F');
    g_Menu[MENU_SIMULATE].AddBoldStatic(SIMULATE_CURTAIN, L"Curtain:");
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_CURTAIN_WIDTH_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_CURTAIN_WIDTH, 0, 0, 0, 0, 4, 100,
        Scene::g_Cloth[Scene::CLOTH_CURTAIN] ? Scene::g_Cloth[Scene::CLOTH_CURTAIN]->Width() / 2 : 0);
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_CURTAIN_HEIGHT_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_CURTAIN_HEIGHT, 0, 0, 0, 0, 4, 100,
        Scene::g_Cloth[Scene::CLOTH_CURTAIN] ? Scene::g_Cloth[Scene::CLOTH_CURTAIN]->Height() / 2 : 0);
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_CURTAIN_RELAXATION_ITERATIONS_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_CURTAIN_RELAXATION_ITERATIONS, 0, 0, 0, 0, 1, 50,
        Scene::g_Cloth[Scene::CLOTH_CURTAIN] ? Scene::g_Cloth[Scene::CLOTH_CURTAIN]->RelaxationIterations() : 0);
    g_Menu[MENU_SIMULATE].AddCheckBox(SIMULATE_CURTAIN_WITH_SHEAR, SHEAR_LABEL, 0, 0, 0, 0, true);
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_CURTAIN_GRAVITY_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_CURTAIN_GRAVITY, 0, 0, 0, 0, 0, 100,
        Scene::g_Cloth[Scene::CLOTH_CURTAIN] ? static_cast<int>(10 * Scene::g_Cloth[Scene::CLOTH_CURTAIN]->GravityStrength()) : 0);
    g_Menu[MENU_SIMULATE].AddBoldStatic(SIMULATE_FLAG, L"Flag:");
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_FLAG_WIDTH_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_FLAG_WIDTH, 0, 0, 0, 0, 4, 100,
        Scene::g_Cloth[Scene::CLOTH_FLAG] ? Scene::g_Cloth[Scene::CLOTH_FLAG]->Width() / 2 : 0);
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_FLAG_HEIGHT_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_FLAG_HEIGHT, 0, 0, 0, 0, 4, 100,
        Scene::g_Cloth[Scene::CLOTH_FLAG] ? Scene::g_Cloth[Scene::CLOTH_FLAG]->Height() / 2 : 0);
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_FLAG_RELAXATION_ITERATIONS_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_FLAG_RELAXATION_ITERATIONS, 0, 0, 0, 0, 1, 50,
        Scene::g_Cloth[Scene::CLOTH_FLAG] ? Scene::g_Cloth[Scene::CLOTH_FLAG]->RelaxationIterations() : 0);
    g_Menu[MENU_SIMULATE].AddCheckBox(SIMULATE_FLAG_WITH_SHEAR, SHEAR_LABEL, 0, 0, 0, 0, false);
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_FLAG_WIND_HEADING_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_FLAG_WIND_HEADING, 0, 0, 0, 0, 0, 100,
        Scene::g_Cloth[Scene::CLOTH_FLAG] ? static_cast<int>(100 * (Scene::g_Cloth[Scene::CLOTH_FLAG]->WindHeading() / (2 * D3DX_PI) + 0.5f)) : 0);
    g_Menu[MENU_SIMULATE].AddBoldStatic(SIMULATE_CAPE, L"Cape:");
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_CAPE_WIDTH_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_CAPE_WIDTH, 0, 0, 0, 0, 4, 100,
        Scene::g_Cloth[Scene::CLOTH_CAPE] ? Scene::g_Cloth[Scene::CLOTH_CAPE]->Width() / 2 : 0);
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_CAPE_HEIGHT_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_CAPE_HEIGHT, 0, 0, 0, 0, 4, 100,
        Scene::g_Cloth[Scene::CLOTH_CAPE] ? Scene::g_Cloth[Scene::CLOTH_CAPE]->Height() / 2 : 0);
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_CAPE_RELAXATION_ITERATIONS_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_CAPE_RELAXATION_ITERATIONS, 0, 0, 0, 0, 1, 50,
        Scene::g_Cloth[Scene::CLOTH_CAPE] ? Scene::g_Cloth[Scene::CLOTH_CAPE]->RelaxationIterations() : 0);
    g_Menu[MENU_SIMULATE].AddCheckBox(SIMULATE_CAPE_WITH_SHEAR, SHEAR_LABEL, 0, 0, 0, 0, false);
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_CAPE_GRAVITY_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_CAPE_GRAVITY, 0, 0, 0, 0, 0, 100,
        Scene::g_Cloth[Scene::CLOTH_CAPE] ? static_cast<int>(10 * Scene::g_Cloth[Scene::CLOTH_CAPE]->GravityStrength()) : 0);
    g_Menu[MENU_SIMULATE].AddBoldStatic(SIMULATE_SKIRT, L"Skirt:");
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_SKIRT_RELAXATION_ITERATIONS_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_SKIRT_RELAXATION_ITERATIONS, 0, 0, 0, 0, 1, 50,
        Scene::g_Cloth[Scene::CLOTH_SKIRT] ? Scene::g_Cloth[Scene::CLOTH_SKIRT]->RelaxationIterations() : 0);
    g_Menu[MENU_SIMULATE].AddCheckBox(SIMULATE_SKIRT_WITH_SHEAR, SHEAR_LABEL, 0, 0, 0, 0, false);
    g_Menu[MENU_SIMULATE].AddStatic(SIMULATE_SKIRT_GRAVITY_LABEL, 0, 0, 0, 0, 0);
    g_Menu[MENU_SIMULATE].AddSlider(SIMULATE_SKIRT_GRAVITY, 0, 0, 0, 0, 0, 100,
        Scene::g_Cloth[Scene::CLOTH_SKIRT] ? static_cast<int>(10 * Scene::g_Cloth[Scene::CLOTH_SKIRT]->GravityStrength()) : 0);

    // Edit menu
    g_Menu[MENU_EDIT].AddButton(EDIT_ADD_PLANE, L"Add plane", 0, 0, 0, 0);
    g_Menu[MENU_EDIT].AddButton(EDIT_ADD_SPHERE, L"Add sphere", 0, 0, 0, 0);
    g_Menu[MENU_EDIT].AddButton(EDIT_ADD_BOX, L"Add box", 0, 0, 0, 0);
    g_Menu[MENU_EDIT].AddButton(EDIT_ADD_ELLIPSOID, L"Add ellipsoid", 0, 0, 0, 0);
    g_Menu[MENU_EDIT].AddCheckBox(EDIT_SELECT_MODE, L"Select mode (I)", 0, 0, 0, 0, false, 'I');
    g_Menu[MENU_EDIT].AddButton(EDIT_UNSELECT_ALL, L"Unselect all (U)", 0, 0, 0, 0, 'U');
    g_Menu[MENU_EDIT].AddButton(EDIT_REMOVE_SELECTION, L"Remove selection (Del)", 0, 0, 0, 0, VK_DELETE);
    g_Menu[MENU_EDIT].AddButton(EDIT_SCALE_SELECTION, L"Scale selection (S)", 0, 0, 0, 0, 'S');
    g_Menu[MENU_EDIT].AddButton(EDIT_MOVE_SELECTION, L"Move selection (A)", 0, 0, 0, 0, 'A');
    g_Menu[MENU_EDIT].AddCheckBox(EDIT_MOVE_ENVIRONMENT_ONLY, L"Move environment only (E)", 0, 0, 0, 0, false, 'E');
    g_Menu[MENU_EDIT].AddCheckBox(EDIT_KEEP_ROTATING, L"Keep rotating (R)", 0, 0, 0, 0, true, 'R');
    g_Menu[MENU_EDIT].AddButton(EDIT_CUT, L"Cut (X)", 0, 0, 0, 0, 'X');
    g_Menu[MENU_EDIT].AddButton(EDIT_UNCUT, L"Uncut (Z)", 0, 0, 0, 0, 'Z');

    // Mouse menu
    g_Menu[MENU_MOUSE].AddStatic(MOUSE_LEFT_LABEL, L"Mouse left drag:", 0, 0, 0, 0);
    g_Menu[MENU_MOUSE].AddComboBox(MOUSE_LEFT, 0, 0, 0, 0, 0, false, &combo);
    if (combo) {
        combo->AddItem(L"Rotate camera", IntToPtr(ROTATE_CAMERA));
        combo->AddItem(L"Pan selection", IntToPtr(PAN_SELECTION));
        combo->AddItem(L"Dolly selection", IntToPtr(DOLLY_SELECTION));
        combo->AddItem(L"Rotate selection", IntToPtr(ROTATE_SELECTION));
        combo->AddItem(L"X-Scale selection", IntToPtr(X_SCALE_SELECTION));
        combo->AddItem(L"Y-Scale selection", IntToPtr(Y_SCALE_SELECTION));
        combo->AddItem(L"Z-Scale selection", IntToPtr(Z_SCALE_SELECTION));
        combo->AddItem(L"Cut", IntToPtr(CUT));
    }
    g_Menu[MENU_MOUSE].AddStatic(MOUSE_MIDDLE_LABEL, L"Mouse middle drag:", 0, 0, 0, 0);
    g_Menu[MENU_MOUSE].AddComboBox(MOUSE_MIDDLE, 0, 0, 0, 0, 0, false, &combo);
    if (combo) {
        combo->AddItem(L"Rotate camera", IntToPtr(ROTATE_CAMERA));
        combo->AddItem(L"Pan selection", IntToPtr(PAN_SELECTION));
        combo->AddItem(L"Dolly selection", IntToPtr(DOLLY_SELECTION));
        combo->AddItem(L"Rotate selection", IntToPtr(ROTATE_SELECTION));
        combo->AddItem(L"X-Scale selection", IntToPtr(X_SCALE_SELECTION));
        combo->AddItem(L"Y-Scale selection", IntToPtr(Y_SCALE_SELECTION));
        combo->AddItem(L"Z-Scale selection", IntToPtr(Z_SCALE_SELECTION));
        combo->AddItem(L"Cut", IntToPtr(CUT));
    }
    g_Menu[MENU_MOUSE].AddStatic(MOUSE_RIGHT_LABEL, L"Mouse right drag:", 0, 0, 0, 0);
    g_Menu[MENU_MOUSE].AddComboBox(MOUSE_RIGHT, 0, 0, 0, 0, 0, false, &combo);
    if (combo) {
        combo->AddItem(L"Rotate camera", IntToPtr(ROTATE_CAMERA));
        combo->AddItem(L"Pan selection", IntToPtr(PAN_SELECTION));
        combo->AddItem(L"Dolly selection", IntToPtr(DOLLY_SELECTION));
        combo->AddItem(L"Rotate selection", IntToPtr(ROTATE_SELECTION));
        combo->AddItem(L"X-Scale selection", IntToPtr(X_SCALE_SELECTION));
        combo->AddItem(L"Y-Scale selection", IntToPtr(Y_SCALE_SELECTION));
        combo->AddItem(L"Z-Scale selection", IntToPtr(Z_SCALE_SELECTION));
        combo->AddItem(L"Cut", IntToPtr(CUT));
    }
    g_Menu[MENU_MOUSE].AddStatic(MOUSE_LEFT_CLICK_LABEL, L"Mouse left click:", 0, 0, 0, 0);
    g_Menu[MENU_MOUSE].AddComboBox(MOUSE_LEFT_CLICK, 0, 0, 0, 0, 0, false, &combo);
    if (combo) {
        combo->AddItem(L"Select", IntToPtr(SELECT));
        combo->AddItem(L"Nail", IntToPtr(NAIL));
    }
    g_Menu[MENU_MOUSE].AddStatic(MOUSE_RIGHT_CLICK_LABEL, L"Mouse right click:", 0, 0, 0, 0);
    g_Menu[MENU_MOUSE].AddComboBox(MOUSE_RIGHT_CLICK, 0, 0, 0, 0, 0, false, &combo);
    if (combo) {
        combo->AddItem(L"Select", IntToPtr(SELECT));
        combo->AddItem(L"Nail", IntToPtr(NAIL));
    }
    combo->SetSelectedByData(IntToPtr(NAIL));
    g_Menu[MENU_MOUSE].AddButton(MOUSE_DEFAULT, L"Default settings (D)", 0, 0, 0, 0, 'D');
    g_Menu[MENU_MOUSE].GetButton(MOUSE_DEFAULT)->OnHotkey();

    g_Menu[MENU_DEVICE].AddButton(DEVICE_TOGGLEREF, L"Toggle REF (F3)", 0, 0, 0, 0, VK_F3);
    g_Menu[MENU_DEVICE].AddButton(DEVICE_CHANGEDEVICE, L"Change device (F2)", 0, 0, 0, 0, VK_F2);

    g_Menu[MENU_RENDER].GetComboBox(RENDER_CAMERA_CENTER)->SetSelectedByData(IntToPtr(CAMERA_CENTER_CHARACTER));

    // Random generator
    srand(::timeGetTime());
}

void Cleanup()
{
    Scene::Cleanup();
}

/*----------------------------------------------------------------------------------------------------------------------
    Device management
 ----------------------------------------------------------------------------------------------------------------------- */

HRESULT CALLBACK OnCreateDevice(IDirect3DDevice9* device, const D3DSURFACE_DESC*, void* pUserContext)
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( device ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( device ) );
    D3DXCreateFont(device, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
                   DEFAULT_PITCH | FF_DONTCARE, L"Arial", &g_Font);
    g_Camera.SetViewParams(&D3DXVECTOR3(0, 0, -4), &D3DXVECTOR3(0, 0, 0));
    g_Camera.SetRadius(6, 0.1f);
    V_RETURN(Scene::OnCreateDevice(device));
    return S_OK;
}

void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();
    SAFE_RELEASE(g_Font);
    SAFE_RELEASE(g_TextSprite);
    Scene::OnDestroyDevice();
}

HRESULT CALLBACK OnResetDevice(IDirect3DDevice9* device, const D3DSURFACE_DESC*, void* pUserContext)
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

    // Backbuffer
    device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &g_BackBuffer);

    // Backbuffer size
    const D3DSURFACE_DESC* backBuffer = DXUTGetD3D9BackBufferSurfaceDesc();
    g_BackBufferWidth = backBuffer->Width;
    g_BackBufferHeight = backBuffer->Height;

    // Render target to render ID and select objects
    V_RETURN(device->CreateRenderTarget(1, 1, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, true,
             &g_IDRTSurface, 0));

    // Render target to get pixel world position
    V_RETURN(device->CreateRenderTarget(1, 1, D3DFMT_A32B32G32R32F, D3DMULTISAMPLE_NONE, 0, true,
             &g_WorldPositionRTSurface, 0));

    // Camera
    float aspectRatio = g_BackBufferWidth / static_cast<FLOAT>(g_BackBufferHeight);
    g_Camera.SetProjParams(D3DX_PI / 4, aspectRatio, 0.1f, 1000);
    g_Camera.SetWindow(g_BackBufferWidth, g_BackBufferHeight);

    // Menus
    for (int i = 0; i < MENU_NUM; ++i)
        g_Menu[i].Reset(g_BackBufferWidth, g_BackBufferHeight);

    // Font
    if (g_Font)
        g_Font->OnResetDevice();

    // Text
    if (g_TextSprite)
        g_TextSprite->OnResetDevice();
    else
        D3DXCreateSprite(device, &g_TextSprite);

    // Mouse
    g_MouseX = g_MouseY = 0;

    // Scene
    V_RETURN(Scene::OnResetDevice(device));

    return S_OK;
}

void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();
    SAFE_RELEASE(g_BackBuffer);
    if (g_Font)
        g_Font->OnLostDevice();
    if (g_TextSprite)
        g_TextSprite->OnLostDevice();
    SAFE_RELEASE(g_IDRTSurface);
    SAFE_RELEASE(g_WorldPositionRTSurface);

    Scene::OnLostDevice();
}

/*----------------------------------------------------------------------------------------------------------------------
    Frame event
 ----------------------------------------------------------------------------------------------------------------------- */

static void MoveSelection(float, HWND = 0, int msg = 0, WPARAM = 0, LPARAM = 0);

void CALLBACK OnFrameMove(IDirect3DDevice9* device, double time_d, float elapsedTime, void* pUserContext)
{
    static float g_CurrentTime;
    static float g_OldStateTime = -1;
    static float g_NewStateTime;
    static float g_OldTimeStep = -1;

    // Camera
    g_Camera.FrameMove(elapsedTime);

    // Simulation control
    if (g_IsPaused)
        return;

    float time = static_cast<float>(time_d);

    // State time initialization
    if (g_OldStateTime < 0) {
        g_CurrentTime = time;
        g_OldStateTime = g_CurrentTime - g_TimeStep;
        g_NewStateTime = g_CurrentTime;
        return;
    }

    // Time step clamping
    if (time - g_CurrentTime > 0.1f) {
        elapsedTime = g_TimeStep;
        float currentTime = time - elapsedTime;
        g_OldStateTime += currentTime - g_CurrentTime;
        g_NewStateTime += currentTime - g_CurrentTime;
        g_CurrentTime = currentTime;
    }

    // Simulation on GPU
    if (SUCCEEDED(device->BeginScene())) {

        // Simulation loop
        float timeStep;
        bool fixedTimestep = g_Menu[MENU_SIMULATE].GetCheckBox(SIMULATE_FIXED_TIMESTEP)->GetChecked();
        timeStep = fixedTimestep ? g_TimeStep : elapsedTime;
        while ((time - g_CurrentTime >= g_TimeStep) || !fixedTimestep) {
            g_CurrentTime += timeStep;

            // Animation
            MoveSelection(timeStep);
            Scene::Animate(timeStep);

            // Cutting
            Cut(device);

            // Simulation
            Scene::Simulate(g_CurrentTime, timeStep, g_OldTimeStep < 0 ? timeStep : g_OldTimeStep);
            g_OldTimeStep = timeStep;
            g_OldStateTime = g_NewStateTime;
            g_NewStateTime = g_CurrentTime;
            if (g_StepSimulation > 0) {
                --g_StepSimulation;
                if (g_StepSimulation == 0)
                    g_IsPaused = true;
            }
            if (!fixedTimestep)
                break;
        }

        // Interpolation
        Scene::Interpolate((time - g_OldStateTime) / (g_NewStateTime - g_OldStateTime));

        device->EndScene();
    }
}

static void Step(int n)
{
    if (g_IsPaused) {
        g_StepSimulation = n;
        g_IsPaused = false;
    }
}

/*----------------------------------------------------------------------------------------------------------------------
    Render event
 ----------------------------------------------------------------------------------------------------------------------- */

void Render(IDirect3DDevice9*, float);

void CALLBACK OnFrameRender(IDirect3DDevice9* device, double time, float elapsedTime, void* pUserContext)
{
   // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( elapsedTime );
        return;
    }
    if (SUCCEEDED(device->BeginScene())) {
        device->SetRenderTarget(0, g_BackBuffer);
        Render(device, elapsedTime);
        device->EndScene();
    }
}

static void SetMatrices();
static void GetMouseTarget(IDirect3DDevice9*);
static void RenderText();

void Render(IDirect3DDevice9* device, float elapsedTime)
{
    SetMatrices();
    if (g_Menu[MENU_EDIT].GetCheckBox(EDIT_SELECT_MODE)->GetChecked())
        GetMouseTarget(device);

    // Clear targets and z
    device->Clear(0L, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0L);

    Scene::Render(device, g_View, g_ViewProjection, g_WireframeCloth, g_WireframeEnvironment,
                  g_Menu[MENU_RENDER].GetCheckBox(RENDER_CLOTH_NORMALS)->GetChecked(),
                  g_Menu[MENU_EDIT].GetCheckBox(EDIT_SELECT_MODE)->GetChecked(), g_CharacterModel,
                  g_CharacterCollisionObjects);

    // Render stats and help text
    RenderText();
    for (int i = 0; i < MENU_NUM; ++i)
        g_Menu[i].OnRender(elapsedTime);
}

void SetMatrices()
{
    g_View = *g_Camera.GetWorldMatrix() * *g_Camera.GetViewMatrix();
    D3DXMatrixInverse(&g_ViewInv, 0, &g_View);
    g_ViewProjection = g_View * *g_Camera.GetProjMatrix();
}

static void WorldToScreen(const D3DXVECTOR3& position, int& x, int& y)
{
    D3DXVECTOR3 screenPosition;
    D3DXVec3TransformCoord(&screenPosition, &position, &g_ViewProjection);
    x = static_cast<int>((screenPosition.x +1)*g_BackBufferWidth / 2 + 0.5f);
    y = static_cast<int>((-screenPosition.y +1)*g_BackBufferHeight / 2 + 0.5f);
}

static void ScreenToWorld(int x, int y, D3DXVECTOR3& position, float distance)
{
    D3DXVECTOR3 screenPosition(
        (2 * static_cast<float>(x) / g_BackBufferWidth - 1) / (*g_Camera.GetProjMatrix())(0, 0),
        (1 - 2 * static_cast<float>(y) / g_BackBufferHeight) / (*g_Camera.GetProjMatrix())(1, 1),
        1);
    screenPosition *= distance;
    D3DXVec3TransformCoord(&position, &screenPosition, &g_ViewInv);
}

static D3DXMATRIX GetPixelTargetTransform(int x, int y)
{
    float w = static_cast<float>(g_BackBufferWidth);
    float h = static_cast<float>(g_BackBufferHeight);
    D3DXVECTOR3 translation(2 * x / w - 1, 1 - 2 * y / h, 0);
    D3DXVECTOR3 scaling(w, h, 1);
    D3DXVECTOR3 translationInv = - translation;
    D3DXMATRIX mouseTargetTransform;
    D3DXMatrixTransformation(&mouseTargetTransform, &translation, 0, &scaling, 0, 0, &translationInv);
    return mouseTargetTransform;
}

void GetMouseTarget(IDirect3DDevice9* device)
{
    device->SetRenderTarget(0, g_IDRTSurface);
    device->Clear(0L, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L);
    Scene::RenderID(g_ViewProjection * GetPixelTargetTransform(g_MouseX, g_MouseY));
    if (g_Menu[MENU_EDIT].GetCheckBox(EDIT_SELECT_MODE)->GetChecked())
        if (g_MouseID)
            for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin();
                 o != Scene::g_ObjectList.end(); ++o)
                if ((*o)->ID() == g_MouseID) {
                    (*o)->Target(g_MouseIDX, g_MouseIDY);
                    break;
                }
    device->SetRenderTarget(0, g_BackBuffer);
    D3DLOCKED_RECT rect;
    g_IDRTSurface->LockRect(&rect, 0, D3DLOCK_READONLY);
    unsigned char* pixel = static_cast<unsigned char*>(rect.pBits);
    g_MouseID = pixel[3];
    g_MouseIDX = pixel[2] / 255.0f;
    g_MouseIDY = pixel[1] / 255.0f;
    g_IDRTSurface->UnlockRect();
    if (g_Menu[MENU_EDIT].GetCheckBox(EDIT_SELECT_MODE)->GetChecked())
        if (g_MouseID)
            for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin();
                 o != Scene::g_ObjectList.end(); ++o)
                if ((*o)->ID() == g_MouseID) {
                    (*o)->Target(g_MouseIDX, g_MouseIDY);
                    break;
                }
}

void RenderText()
{
    CDXUTTextHelper txtHelper(g_Font, g_TextSprite, 15);

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos(5, 5);
    txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));

    // Show UI
    if (g_ShowUI) {
        txtHelper.DrawTextLine(DXUTGetFrameStats());
        txtHelper.DrawTextLine(DXUTGetDeviceStats());
        if (!g_ShowHelp) {
            txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
            txtHelper.DrawTextLine(TEXT("F1 - Show/Hide help text"));
        }
    }

    // Show basic help message
    if (g_ShowHelp) {
        txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
        txtHelper.DrawTextLine(TEXT("F1 - Show/Hide help text"));
        txtHelper.DrawTextLine(TEXT("H - Show/Hide UI"));
        txtHelper.DrawTextLine(TEXT("M - Open/Close UI menus"));
        txtHelper.DrawTextLine(TEXT("Left drag - Move UI menus"));
        txtHelper.DrawTextLine(TEXT("Wheel - Zoom"));
        txtHelper.DrawTextLine(TEXT("Hold Ctrl - Default mouse settings"));
        txtHelper.DrawTextLine(TEXT("Shift Ctrl - Swap rotate and pan selection"));
        txtHelper.DrawTextLine(TEXT("ESC - Quit"));
    }
    txtHelper.End();

    if (g_ShowUI) {
        txtHelper.Begin();
        txtHelper.SetInsertionPos(5, g_BackBufferHeight - 25);
        txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f));
        txtHelper.End();
    }
}

D3DXVECTOR3 GetWorldPosition(int x, int y, IDirect3DDevice9* device)
{
    device->SetRenderTarget(0, g_WorldPositionRTSurface);
    device->Clear(0L, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L);
    Scene::RenderWorldPosition(device, g_ViewProjection * GetPixelTargetTransform(x, y),
                               g_CharacterModel, g_CharacterCollisionObjects);
    device->SetRenderTarget(0, g_BackBuffer);
    D3DLOCKED_RECT rect;
    g_WorldPositionRTSurface->LockRect(&rect, 0, D3DLOCK_READONLY);
    float* position = static_cast<float*>(rect.pBits);
    g_WorldPositionRTSurface->UnlockRect();
    return D3DXVECTOR3(position[0], position[1], position[2]);
}

/*----------------------------------------------------------------------------------------------------------------------
    Mouse event
 ----------------------------------------------------------------------------------------------------------------------- */

void CALLBACK Mouse(bool, bool, bool, bool, bool, int, int xPos, int yPos, void* pUserContext)
{
    g_MouseX = xPos;
    g_MouseX = max(0, g_MouseX);
    g_MouseX = min(g_BackBufferWidth - 1, g_MouseX);
    g_MouseY = yPos;
    g_MouseY = max(0, g_MouseY);
    g_MouseY = min(g_BackBufferHeight - 1, g_MouseY);
}

/*----------------------------------------------------------------------------------------------------------------------
    Keyboard event
 ----------------------------------------------------------------------------------------------------------------------- */

void CALLBACK KeyboardProc(unsigned int key, bool keyDown, bool altDown, void* pUserContext)
{
    static int g_MouseComboBox[] = { MOUSE_LEFT, MOUSE_MIDDLE, MOUSE_RIGHT };
    static int g_MouseComboBoxOldValue[3];
    static bool g_ShiftDown;
    static bool g_CtrlDown;
    if (keyDown) {
        bool drag = g_Camera.IsBeingDragged() || g_SelectionIsMoved || g_SelectionIsScaled;
        switch (key) {
        case VK_SHIFT:
            if (!drag && !g_ShiftDown) {
                g_ShiftDown = true;
                for (int i = 0; i < 3; ++i) {
                    if (!g_CtrlDown)
                        g_MouseComboBoxOldValue[i] = PtrToInt(g_Menu[MENU_MOUSE].GetComboBox(g_MouseComboBox[i])->GetSelectedData());

                    int value = g_MouseComboBoxOldValue[i];
                    switch (g_MouseComboBoxOldValue[i]) {
                    case PAN_SELECTION:
                        value = ROTATE_SELECTION;
                        break;
                    case ROTATE_SELECTION:
                        value = PAN_SELECTION;
                        break;
                    default:
                        break;
                    }
                    g_Menu[MENU_MOUSE].GetComboBox(g_MouseComboBox[i])->SetSelectedByData(IntToPtr(value));
                }
            }
            break;
        case VK_CONTROL:
            if (!drag && !g_CtrlDown) {
                g_CtrlDown = true;
                for (int i = 0; i < 3; ++i)
                    if (!g_ShiftDown)
                        g_MouseComboBoxOldValue[i] = PtrToInt(g_Menu[MENU_MOUSE].GetComboBox(g_MouseComboBox[i])->GetSelectedData());
                g_Menu[MENU_MOUSE].GetButton(MOUSE_DEFAULT)->OnHotkey();
            }
            break;
        case VK_F1:
            g_ShowHelp = !g_ShowHelp;
            break;
        case 'H':
        case 'h':
            g_ShowUI = !g_ShowUI;
            for (int i = 0; i < MENU_NUM; ++i)
                g_Menu[i].SetVisible(g_ShowUI);
            break;
        case 'M':
        case 'm':
			int i = 0;
            for (i = 0; i < MENU_NUM; ++i)
                if (!g_Menu[i].IsOpen())
                    break;
            if (i < MENU_NUM)
                for (int i = 0; i < MENU_NUM; ++i)
                    g_Menu[i].SetOpen(true);
            else
                for (int i = 0; i < MENU_NUM; ++i)
                    g_Menu[i].SetOpen(false);
            break;
        }
        if ('1' <= key && key <= '9')
            Step(key - '0');
    }
    else
        switch (key) {
        case VK_SHIFT:
            if (g_ShiftDown) {
                g_ShiftDown = false;
                if (!g_CtrlDown)
                    for (int i = 0; i < 3; ++i)
                        g_Menu[MENU_MOUSE].GetComboBox(g_MouseComboBox[i])->SetSelectedByData(IntToPtr(g_MouseComboBoxOldValue[i]));
            }
            break;
        case VK_CONTROL:
            if (g_CtrlDown) {
                g_CtrlDown = false;
                if (!g_ShiftDown)
                    for (int i = 0; i < 3; ++i)
                        g_Menu[MENU_MOUSE].GetComboBox(g_MouseComboBox[i])->SetSelectedByData(IntToPtr(g_MouseComboBoxOldValue[i]));
            }
            break;
        }
}

/*----------------------------------------------------------------------------------------------------------------------
    Menu event
 ----------------------------------------------------------------------------------------------------------------------- */

static HRESULT ToggleFullScreen();
static void SetEnabledItems(int, int, bool, bool);

void CALLBACK OnGUIEventViewMenu(unsigned int event, int controlID, CDXUTControl* control, void* pUserContext)
{
    WCHAR text[256];
    D3DXQUATERNION q;
    float pitch = 0;
    switch (controlID) {
    case RENDER_TOGGLEFULLSCREEN:
        ToggleFullScreen();
        break;
    case RENDER_CAMERA_CENTER:
        switch (PtrToInt (static_cast<CDXUTComboBox*>(control)->GetSelectedData ())) {
        case CAMERA_CENTER_CURTAIN:
        default:
            g_SceneCenter = Scene::g_SceneCenter[Scene::CLOTH_CURTAIN];
            pitch = -D3DX_PI / 16;
            SetEnabledItems(SIMULATE_CURTAIN, SIMULATE_FLAG, true, g_Menu[MENU_SIMULATE].IsOpen());
            SetEnabledItems(SIMULATE_FLAG, SIMULATE_CAPE, false, g_Menu[MENU_SIMULATE].IsOpen());
            SetEnabledItems(SIMULATE_CAPE, SIMULATE_SKIRT, false, g_Menu[MENU_SIMULATE].IsOpen());
            SetEnabledItems(SIMULATE_SKIRT, SIMULATE_NUM, false, g_Menu[MENU_SIMULATE].IsOpen());
            break;
        case CAMERA_CENTER_FLAG:
            g_SceneCenter = Scene::g_SceneCenter[Scene::CLOTH_FLAG];
            SetEnabledItems(SIMULATE_CURTAIN, SIMULATE_FLAG, false, g_Menu[MENU_SIMULATE].IsOpen());
            SetEnabledItems(SIMULATE_FLAG, SIMULATE_CAPE, true, g_Menu[MENU_SIMULATE].IsOpen());
            SetEnabledItems(SIMULATE_CAPE, SIMULATE_SKIRT, false, g_Menu[MENU_SIMULATE].IsOpen());
            SetEnabledItems(SIMULATE_SKIRT, SIMULATE_NUM, false, g_Menu[MENU_SIMULATE].IsOpen());
            break;
        case CAMERA_CENTER_CHARACTER:
            g_SceneCenter = Scene::g_SceneCenter[Scene::CLOTH_CAPE];
            SetEnabledItems(SIMULATE_CURTAIN, SIMULATE_FLAG, false, g_Menu[MENU_SIMULATE].IsOpen());
            SetEnabledItems(SIMULATE_FLAG, SIMULATE_CAPE, false, g_Menu[MENU_SIMULATE].IsOpen());
            SetEnabledItems(SIMULATE_CAPE, SIMULATE_NUM, true, g_Menu[MENU_SIMULATE].IsOpen());
            break;
        }
        g_Menu[MENU_SIMULATE].ResetItems();
        g_Camera.SetModelCenter(g_SceneCenter);
        D3DXQuaternionRotationYawPitchRoll(&q, -atan2f(-g_SceneCenter[0], -g_SceneCenter[2]), pitch, 0);
        g_Camera.SetWorldQuat(q);
        break;
    case RENDER_CURTAIN:
        Scene::g_RenderScene[Scene::CLOTH_CURTAIN] = static_cast<CDXUTCheckBox*>(control)->GetChecked();
        break;
    case RENDER_FLAG:
        Scene::g_RenderScene[Scene::CLOTH_FLAG] = static_cast<CDXUTCheckBox*>(control)->GetChecked();
        break;
    case RENDER_CAPE:
        Scene::g_RenderScene[Scene::CLOTH_CAPE] = static_cast<CDXUTCheckBox*>(control)->GetChecked();
        break;
    case RENDER_SKIRT:
        Scene::g_RenderScene[Scene::CLOTH_SKIRT] = static_cast<CDXUTCheckBox*>(control)->GetChecked();
        break;
    case RENDER_CHARACTER:
        switch (PtrToInt (static_cast<CDXUTComboBox*>(control)->GetSelectedData ())) {
        case MODEL_ONLY:
            g_CharacterModel = true;
            g_CharacterCollisionObjects = false;
            break;
        case COLLISION_OBJECTS_ONLY:
            g_CharacterModel = false;
            g_CharacterCollisionObjects = true;
            break;
        case ALL:
        default:
            g_CharacterModel = true;
            g_CharacterCollisionObjects = true;
            break;
        }
        break;
    case RENDER_WIREFRAME:
        switch (PtrToInt (static_cast<CDXUTComboBox*>(control)->GetSelectedData ())) {
        case NONE:
        default:
            g_WireframeCloth = false;
            g_WireframeEnvironment = false;
            break;
        case CLOTHES_ONLY:
            g_WireframeCloth = true;
            g_WireframeEnvironment = false;
            break;
        case ENVIRONMENT_ONLY:
            g_WireframeCloth = false;
            g_WireframeEnvironment = true;
            break;
        case ALL:
            g_WireframeCloth = true;
            g_WireframeEnvironment = true;
            break;
        }
        break;
    case RENDER_THINNING:
        Scene::g_Thinning = -static_cast<CDXUTSlider*>(control)->GetValue() / 1000.0f;
        _snwprintf(text, 255, L"Collision objects thinning: %.3f", -Scene::g_Thinning);
        g_Menu[MENU_RENDER].GetStatic(RENDER_THINNING_LABEL)->SetText(text);
        break;
    default:
        break;
    }
}

void CALLBACK OnGUIEventEditMenu(unsigned int event, int controlID, CDXUTControl* control, void* pUserContext)
{
#define RAND()  (rand() / static_cast<float>(RAND_MAX))
    IDirect3DDevice9* device = DXUTGetD3D9Device();
    switch (controlID) {
    case EDIT_ADD_PLANE:
        Scene::AddPlane(device, D3DXVECTOR3(0, 1, 0), 2 + RAND());
        break;
    case EDIT_ADD_SPHERE:
        Scene::AddSphere(device, g_SceneCenter + D3DXVECTOR3(0.5f * RAND(), 0.5f * RAND(), 0.5f * RAND()), 1);
        break;
    case EDIT_ADD_BOX:
        Scene::AddBox(device, g_SceneCenter + D3DXVECTOR3(0.5f * RAND(), 0.5f * RAND(), 0.5f * RAND()),
                      D3DXVECTOR3(1.5f, 1, 0.5f));
        break;
    case EDIT_ADD_ELLIPSOID:
        Scene::AddEllipsoid(device, g_SceneCenter + D3DXVECTOR3(0.5f * RAND(), 0.5f * RAND(), 0.5f * RAND()),
                            D3DXVECTOR3(1.5f, 1, 0.5f));
        break;
    case EDIT_UNSELECT_ALL:
        for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin();
             o != Scene::g_ObjectList.end(); ++o)
            if ((*o)->IsSelected())
                (*o)->Select();
        break;
    case EDIT_REMOVE_SELECTION:
        Scene::RemoveSelection();
        break;
    case EDIT_SCALE_SELECTION:
        g_Menu[MENU_MOUSE].GetComboBox(MOUSE_LEFT)->SetSelectedByData(IntToPtr(X_SCALE_SELECTION));
        g_Menu[MENU_MOUSE].GetComboBox(MOUSE_MIDDLE)->SetSelectedByData(IntToPtr(Y_SCALE_SELECTION));
        g_Menu[MENU_MOUSE].GetComboBox(MOUSE_RIGHT)->SetSelectedByData(IntToPtr(Z_SCALE_SELECTION));
        break;
    case EDIT_MOVE_SELECTION:
        g_Menu[MENU_MOUSE].GetComboBox(MOUSE_LEFT)->SetSelectedByData(IntToPtr(ROTATE_SELECTION));
        g_Menu[MENU_MOUSE].GetComboBox(MOUSE_MIDDLE)->SetSelectedByData(IntToPtr(DOLLY_SELECTION));
        g_Menu[MENU_MOUSE].GetComboBox(MOUSE_RIGHT)->SetSelectedByData(IntToPtr(PAN_SELECTION));
        break;
    case EDIT_MOVE_ENVIRONMENT_ONLY:
        Scene::g_MoveEnvironmentOnly = static_cast<CDXUTCheckBox*>(control)->GetChecked ();
        break;
    case EDIT_CUT:
        g_Menu[MENU_MOUSE].GetComboBox(MOUSE_LEFT)->SetSelectedByData(IntToPtr(CUT));
        break;
    case EDIT_UNCUT:
        for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin();
             o != Scene::g_ObjectList.end(); ++o)
            (*o)->Uncut();
        break;
    default:
        break;
    }
}

void CALLBACK OnGUIEventMouseMenu(unsigned int event, int controlID, CDXUTControl* control, void* pUserContext)
{
    IDirect3DDevice9* device = DXUTGetD3D9Device();
    switch (controlID) {
    case MOUSE_DEFAULT:
        g_Menu[MENU_MOUSE].GetComboBox(MOUSE_LEFT)->SetSelectedByData(IntToPtr(ROTATE_CAMERA));
        g_Menu[MENU_MOUSE].GetComboBox(MOUSE_MIDDLE)->SetSelectedByData(IntToPtr(DOLLY_SELECTION));
        g_Menu[MENU_MOUSE].GetComboBox(MOUSE_RIGHT)->SetSelectedByData(IntToPtr(PAN_SELECTION));
        break;
    default:
        break;
    }
}

static int ClothIndex(int);

void CALLBACK OnGUIEventSimulateMenu(unsigned int event, int controlID, CDXUTControl* control, void* pUserContext)
{
    WCHAR text[256];
    switch (controlID) {
    case SIMULATE_RUN:
        if (g_IsPaused) {
            g_IsPaused = false;
            static_cast<CDXUTButton*>(control)->SetText(L"Pause (Space)");
        }
        else {
            g_IsPaused = true;
            static_cast<CDXUTButton*>(control)->SetText(L"Run (Space)");
        }
        break;
    case SIMULATE_STEP:
        Step(1);
        break;
    case SIMULATE_RESET:
        Scene::Reset();
        MoveSelection(-1);
        Step(1);
        break;
    case SIMULATE_FIXED_TIMESTEP:
        if (g_Menu[MENU_SIMULATE].GetStatic(SIMULATE_RATE_LABEL))
            g_Menu[MENU_SIMULATE].GetStatic(SIMULATE_RATE_LABEL)->SetEnabled (static_cast<CDXUTCheckBox*>(control)->GetChecked ());
        if (g_Menu[MENU_SIMULATE].GetSlider(SIMULATE_RATE))
            g_Menu[MENU_SIMULATE].GetSlider(SIMULATE_RATE)->SetEnabled (static_cast<CDXUTCheckBox*>(control)->GetChecked ());
        break;
    case SIMULATE_RATE:
        g_TimeStep = 1.0f / static_cast<CDXUTSlider*>(control)->GetValue();
        _snwprintf(text, 255, L"Simulation rate: %d", static_cast<CDXUTSlider*>(control)->GetValue());
        g_Menu[MENU_SIMULATE].GetStatic(SIMULATE_RATE_LABEL)->SetText(text);
        break;
    case SIMULATE_CLOTH_SELECTION_FREE:
        if (!g_SelectionIsMoved)
            Scene::SetSelectionFree(static_cast<CDXUTCheckBox*>(control)->GetChecked ());
        break;
    case SIMULATE_CURTAIN_WIDTH:
    case SIMULATE_FLAG_WIDTH:
    case SIMULATE_CAPE_WIDTH:
        if (Scene::g_Cloth[ClothIndex(controlID)]) {
            Scene::g_Cloth[ClothIndex(controlID)]->SetSize(2 * static_cast<CDXUTSlider*>(control)->GetValue(),
                                                        Scene::g_Cloth[ClothIndex(controlID)]->Height());
            _snwprintf(text, 255, L"Width: %d", Scene::g_Cloth[ClothIndex(controlID)]->Width());
            g_Menu[MENU_SIMULATE].GetStatic(controlID - 1)->SetText(text);
        }
        break;
    case SIMULATE_CURTAIN_HEIGHT:
    case SIMULATE_FLAG_HEIGHT:
    case SIMULATE_CAPE_HEIGHT:
        if (Scene::g_Cloth[ClothIndex(controlID)]) {
            Scene::g_Cloth[ClothIndex(controlID)]->SetSize(Scene::g_Cloth[ClothIndex(controlID)]->Width(),
                                                        2 * static_cast<CDXUTSlider*>(control)->GetValue());
            _snwprintf(text, 255, L"Height: %d", Scene::g_Cloth[ClothIndex(controlID)]->Height());
            g_Menu[MENU_SIMULATE].GetStatic(controlID - 1)->SetText(text);
        }
        break;
    case SIMULATE_CURTAIN_RELAXATION_ITERATIONS:
    case SIMULATE_FLAG_RELAXATION_ITERATIONS:
    case SIMULATE_CAPE_RELAXATION_ITERATIONS:
    case SIMULATE_SKIRT_RELAXATION_ITERATIONS:
        if (Scene::g_Cloth[ClothIndex(controlID)]) {
            Scene::g_Cloth[ClothIndex(controlID)]->SetRelaxationIterations (static_cast<CDXUTSlider*>(control)->GetValue ());
            _snwprintf(text, 255, L"Relaxation iterations: %d",
                    Scene::g_Cloth[ClothIndex(controlID)]->RelaxationIterations());
            g_Menu[MENU_SIMULATE].GetStatic(controlID - 1)->SetText(text);
        }
        break;
    case SIMULATE_CURTAIN_WITH_SHEAR:
    case SIMULATE_FLAG_WITH_SHEAR:
    case SIMULATE_CAPE_WITH_SHEAR:
    case SIMULATE_SKIRT_WITH_SHEAR:
        if (Scene::g_Cloth[ClothIndex(controlID)])
            Scene::g_Cloth[ClothIndex(controlID)]->SetShearConstraint (static_cast<CDXUTCheckBox*>(control)->GetChecked ());
        break;
    case SIMULATE_CURTAIN_GRAVITY:
    case SIMULATE_CAPE_GRAVITY:
    case SIMULATE_SKIRT_GRAVITY:
        if (Scene::g_Cloth[ClothIndex(controlID)]) {
            Scene::g_Cloth[ClothIndex(controlID)]->SetGravityStrength (static_cast<CDXUTSlider*>(control)->GetValue () / 10.0f);
            _snwprintf(text, 255, GRAVITY_LABEL L": %.1f", Scene::g_Cloth[ClothIndex(controlID)]->GravityStrength());
            g_Menu[MENU_SIMULATE].GetStatic(controlID - 1)->SetText(text);
        }
        break;
    case SIMULATE_FLAG_WIND_HEADING:
        if (Scene::g_Cloth[Scene::CLOTH_FLAG]) {
            Scene::g_Cloth[Scene::CLOTH_FLAG]->SetWindHeading(D3DX_PI * (static_cast<CDXUTSlider*>(control)->GetValue () / 50.0f -1));
            _snwprintf(text, 255, L"Wind heading: %.1f", D3DXToDegree(Scene::g_Cloth[Scene::CLOTH_FLAG]->WindHeading()));
            g_Menu[MENU_SIMULATE].GetStatic(controlID - 1)->SetText(text);
        }
        break;
    default:
        break;
    }
}

void CALLBACK OnGUIEventDeviceMenu(unsigned int event, int controlID, CDXUTControl* control, void* pUserContext)
{
    switch (controlID) {
    case DEVICE_TOGGLEREF:
        DXUTToggleREF();
        break;
    case DEVICE_CHANGEDEVICE:
        g_SettingsDlg.SetActive(!g_SettingsDlg.IsActive());
        break;
    default:
        break;
    }
}

static void SetEnabledItems(int from, int to, bool enabled, bool open)
{
    for (int i = from; i < to; ++i)
        if (g_Menu[MENU_SIMULATE].GetControl(i)) {
            g_Menu[MENU_SIMULATE].GetControl(i)->SetUserData(IntToPtr(enabled ? 0 : 1));
            if (open)
                g_Menu[MENU_SIMULATE].GetControl(i)->SetVisible(enabled);
        }
}

static int ClothIndex(int controlID)
{
    if (controlID < SIMULATE_FLAG)
        return 0;
    else if (controlID < SIMULATE_CAPE)
        return 1;
    else if (controlID < SIMULATE_SKIRT)
        return 2;
    else
        return 3;
}

/*----------------------------------------------------------------------------------------------------------------------
    Message event
 ----------------------------------------------------------------------------------------------------------------------- */

static void MapMouseDrag(int, int&, int&, int&, int&, WPARAM&, LPARAM&);
static void EditSelection(int, LPARAM);
static void ScaleSelection(int msg, LPARAM);

LRESULT CALLBACK MsgProc(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, bool* noFurtherProcessing, void* pUserContext)
{
    IDirect3DDevice9* device = DXUTGetD3D9Device();

    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    g_DialogResourceManager.MsgProc( hWnd, msg, wParam, lParam );

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, msg, wParam, lParam );
        return 0;
    }

    // Menus
    for (int i = 0; i < MENU_NUM; ++i) {
        *noFurtherProcessing = g_Menu[i].MsgProc(hWnd, msg, wParam, lParam);
        if (*noFurtherProcessing)
            return 0;
    }

    // Edit selection
    if (g_Menu[MENU_EDIT].GetCheckBox(EDIT_SELECT_MODE)->GetChecked())
        EditSelection(msg, lParam);

    // Mouse mapping
    int cameraMsg;
    int moveSelectionMsg;
    int scaleSelectionMsg;
    int cutMsg;
    MapMouseDrag(msg, cameraMsg, moveSelectionMsg, scaleSelectionMsg, cutMsg, wParam, lParam);

    // Camera
    g_Camera.HandleMessages(hWnd, cameraMsg, wParam, lParam);

    // Transform selection
    MoveSelection(0, hWnd, moveSelectionMsg, wParam, lParam);
    ScaleSelection(scaleSelectionMsg, lParam);

    // Cut
    Cut(device, hWnd, cutMsg, lParam);

    return 0;
}

void EditSelection(int msg, LPARAM lParam)
{
    static bool g_LeftClick;
    static LPARAM g_LeftClickMouse;
    static bool g_RightClick;
    static LPARAM g_RightClickMouse;
    bool click = false;
    bool nail = false;
    if (g_LeftClick) {
        if (msg == WM_MOUSEMOVE) {
            if (lParam != g_LeftClickMouse)
                g_LeftClick = false;
        }
        else if (msg == WM_LBUTTONUP) {
            click = true;
            nail = PtrToInt(g_Menu[MENU_MOUSE].GetComboBox(MOUSE_LEFT_CLICK)->GetSelectedData()) == NAIL;
            g_LeftClick = false;
        }
    }
    else if (msg == WM_LBUTTONDOWN) {
        g_LeftClick = true;
        g_LeftClickMouse = lParam;
    }
    if (g_RightClick) {
        if (msg == WM_MOUSEMOVE) {
            if (lParam != g_RightClickMouse)
                g_RightClick = false;
        }
        else if (msg == WM_RBUTTONUP) {
            click = true;
            nail = PtrToInt(g_Menu[MENU_MOUSE].GetComboBox(MOUSE_RIGHT_CLICK)->GetSelectedData()) == NAIL;
            g_RightClick = false;
        }
    }
    else if (msg == WM_RBUTTONDOWN) {
        g_RightClick = true;
        g_RightClickMouse = lParam;
    }
    if (click) {
        for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin();
             o != Scene::g_ObjectList.end(); ++o)
            if ((*o)->ID() == g_MouseID)
                (*o)->Select(g_MouseIDX, g_MouseIDY, nail);
        MoveSelection(-1);
    }
}

void MapMouseDrag(int msg, int& cameraMsg, int& moveSelectionMsg, int& scaleSelectionMsg, int& cutMsg,
                  WPARAM& wParam, LPARAM& lParam)
{
    static int g_MouseMask;

    cameraMsg = 0;
    moveSelectionMsg = 0;
    scaleSelectionMsg = 0;
    cutMsg = 0;
    if (msg == WM_LBUTTONUP || msg == WM_MBUTTONUP || msg == WM_RBUTTONUP) {
        if (g_Camera.IsBeingDragged()) {
            cameraMsg = msg;
            return;
        }
        if (g_SelectionIsMoved) {
            moveSelectionMsg = msg;
            return;
        }
        if (g_SelectionIsScaled) {
            scaleSelectionMsg = msg;
            return;
        }
    }
    switch (msg) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        g_MouseMask = 0;
        switch (PtrToInt(g_Menu[MENU_MOUSE].GetComboBox(MOUSE_LEFT)->GetSelectedData())) {
        case ROTATE_CAMERA:
            cameraMsg = msg;
            break;
        case PAN_SELECTION:
            moveSelectionMsg = msg == WM_LBUTTONDOWN ? WM_RBUTTONDOWN : WM_RBUTTONUP;
            g_MouseMask = MK_RBUTTON;
            break;
        case DOLLY_SELECTION:
            moveSelectionMsg = msg == WM_LBUTTONDOWN ? WM_MBUTTONDOWN : WM_MBUTTONUP;
            g_MouseMask = MK_MBUTTON;
            break;
        case ROTATE_SELECTION:
            moveSelectionMsg = msg;
            break;
        case X_SCALE_SELECTION:
            scaleSelectionMsg = msg;
            break;
        case Y_SCALE_SELECTION:
            scaleSelectionMsg = msg == WM_LBUTTONDOWN ? WM_MBUTTONDOWN : WM_MBUTTONUP;
            g_MouseMask = MK_MBUTTON;
            break;
        case Z_SCALE_SELECTION:
            scaleSelectionMsg = msg == WM_LBUTTONDOWN ? WM_RBUTTONDOWN : WM_RBUTTONUP;
            g_MouseMask = MK_RBUTTON;
            break;
        case CUT:
            cutMsg = msg;
            break;
        }
        break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        g_MouseMask = 0;
        switch (PtrToInt(g_Menu[MENU_MOUSE].GetComboBox(MOUSE_MIDDLE)->GetSelectedData())) {
        case ROTATE_CAMERA:
            cameraMsg = msg == WM_MBUTTONDOWN ? WM_LBUTTONDOWN : WM_LBUTTONUP;
            g_MouseMask = MK_LBUTTON;
            break;
        case PAN_SELECTION:
            moveSelectionMsg = msg == WM_MBUTTONDOWN ? WM_RBUTTONDOWN : WM_RBUTTONUP;
            g_MouseMask = MK_RBUTTON;
            break;
        case DOLLY_SELECTION:
            moveSelectionMsg = msg;
            break;
        case ROTATE_SELECTION:
            moveSelectionMsg = msg == WM_MBUTTONDOWN ? WM_LBUTTONDOWN : WM_LBUTTONUP;
            g_MouseMask = MK_LBUTTON;
            break;
        case X_SCALE_SELECTION:
            scaleSelectionMsg = msg == WM_MBUTTONDOWN ? WM_LBUTTONDOWN : WM_LBUTTONUP;
            g_MouseMask = MK_LBUTTON;
            break;
        case Y_SCALE_SELECTION:
            scaleSelectionMsg = msg;
            break;
        case Z_SCALE_SELECTION:
            scaleSelectionMsg = msg == WM_MBUTTONDOWN ? WM_RBUTTONDOWN : WM_RBUTTONUP;
            g_MouseMask = MK_RBUTTON;
            break;
        case CUT:
            cutMsg = msg;
            break;
        }
        break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        g_MouseMask = 0;
        switch (PtrToInt(g_Menu[MENU_MOUSE].GetComboBox(MOUSE_RIGHT)->GetSelectedData())) {
        case ROTATE_CAMERA:
            cameraMsg = msg;
            break;
        case PAN_SELECTION:
            moveSelectionMsg = msg;
            break;
        case DOLLY_SELECTION:
            moveSelectionMsg = msg == WM_RBUTTONDOWN ? WM_MBUTTONDOWN : WM_MBUTTONUP;
            g_MouseMask = MK_MBUTTON;
            break;
        case ROTATE_SELECTION:
            moveSelectionMsg = msg == WM_RBUTTONDOWN ? WM_LBUTTONDOWN : WM_LBUTTONUP;
            g_MouseMask = MK_LBUTTON;
            break;
        case X_SCALE_SELECTION:
            scaleSelectionMsg = msg == WM_RBUTTONDOWN ? WM_LBUTTONDOWN : WM_LBUTTONUP;
            g_MouseMask = MK_LBUTTON;
            break;
        case Y_SCALE_SELECTION:
            scaleSelectionMsg = msg == WM_RBUTTONDOWN ? WM_MBUTTONDOWN : WM_MBUTTONUP;
            g_MouseMask = MK_MBUTTON;
            break;
        case Z_SCALE_SELECTION:
            scaleSelectionMsg = msg;
            break;
        case CUT:
            cutMsg = msg;
            break;
        }
        break;
    case WM_MOUSEMOVE:
        cameraMsg = msg;
        moveSelectionMsg = msg;
        scaleSelectionMsg = msg;
        cutMsg = msg;
        if (g_MouseMask) {
            wParam &= ~(MK_LBUTTON | MK_MBUTTON | MK_RBUTTON);
            wParam |= g_MouseMask;
        }
        break;
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
        break;
    default:
        cameraMsg = msg;
        moveSelectionMsg = msg;
        scaleSelectionMsg = msg;
        cutMsg = msg;
        break;
    }
}

void ScaleSelection(int msg, LPARAM lParam)
{
    static float g_StartMouse;
    static std::vector<D3DXVECTOR3> g_DimensionList;
    static int g_Index;
    float mouse = static_cast<float>(LOWORD(lParam));
    if (msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN) {
        g_DimensionList.clear();
        for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin();
             o != Scene::g_ObjectList.end(); ++o)
            if ((*o)->IsSelected()) {
                D3DXVECTOR3 dimension = (*o)->Dimension();
                switch (msg) {
                case WM_LBUTTONDOWN:
                    g_Index = 0;
                    break;
                case WM_MBUTTONDOWN:
                    g_Index = 1;
                    break;
                case WM_RBUTTONDOWN:
                    g_Index = 2;
                    break;
                }
                g_DimensionList.push_back(dimension);
            }
        if (g_DimensionList.size() == 0)
            return;
        g_SelectionIsScaled = true;
        g_StartMouse = mouse;
    }
    if (msg == WM_MOUSEMOVE)
        if (g_SelectionIsScaled) {
            D3DXVECTOR3 offset(0, 0, 0);
            offset[g_Index] = 0.05f * (mouse - g_StartMouse);
            std::vector<D3DXVECTOR3>::const_iterator d = g_DimensionList.begin();
            for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin();
                 o != Scene::g_ObjectList.end(); ++o)
                if ((*o)->IsSelected()) {
                    D3DXVECTOR3 dimension = *d + offset;
                    (*o)->SetDimension(dimension);
                    ++d;
                }
        }
    if (msg == WM_LBUTTONUP || msg == WM_MBUTTONUP || msg == WM_RBUTTONUP)
        g_SelectionIsScaled = false;
}

void Cut(IDirect3DDevice9* device, HWND hWnd, int msg, LPARAM lParam)
{
    static bool g_IsCut;
    static LPARAM g_LastMouse;
    static LPARAM g_CurrentMouse;

    if (hWnd) {
        if (msg == WM_LBUTTONDOWN) {
            g_IsCut = true;
            g_LastMouse = lParam;
            g_CurrentMouse = lParam;
        }
        if (msg == WM_MOUSEMOVE)
            if (g_IsCut)
                g_CurrentMouse = lParam;
        if (msg == WM_LBUTTONUP)
            g_IsCut = false;
    }
    else if (g_LastMouse != g_CurrentMouse) {
        D3DXVECTOR3 cutter[3];
        cutter[0] = D3DXVECTOR3(g_ViewInv(3, 0), g_ViewInv(3, 1), g_ViewInv(3, 2));
        cutter[1] = GetWorldPosition(LOWORD(g_LastMouse), HIWORD(g_LastMouse), device);
        cutter[2] = GetWorldPosition(LOWORD(g_CurrentMouse), HIWORD(g_CurrentMouse), device);
        Scene::Cut(cutter);
        g_LastMouse = g_CurrentMouse;
    }
}

/*----------------------------------------------------------------------------------------------------------------------
    Slightly modified version of CD3DArcBall used to move the selection
 ----------------------------------------------------------------------------------------------------------------------- */

class ArcBall
{

public:
    ArcBall();

    // Functions to change behavior
    void Reset();
    void SetTranslationRadius(FLOAT fRadiusTranslation) { m_fRadiusTranslation = fRadiusTranslation; }
    void SetWindow(INT nWidth, INT nHeight, FLOAT fRadius = 0.9f)
    {
        m_nWidth = nWidth;
        m_nHeight = nHeight;
        m_fRadius = fRadius;
        m_vCenter.x = m_nWidth / 2;
        m_vCenter.y = m_nHeight / 2;
    }

    void SetCenter(INT nX, INT nY) { m_vCenter.x = nX; m_vCenter.y = nY; }

    // Call these from client and use GetRotationMatrix() to read new rotation matrix
    void OnBegin(int nX, int nY);       // start the rotation (pass current mouse position)
    void OnMove(int nX, int nY);        // continue the rotation (pass current mouse position)
    void OnEnd();                       // end the rotation

    // Or call this to automatically handle left, middle, right buttons
    LRESULT HandleMessages(HWND hWnd, int msg, WPARAM wParam, LPARAM lParam);

    // Functions to get/set state
    D3DXMATRIX* GetRotationMatrix() { return D3DXMatrixRotationQuaternion(&m_mRotation, &m_qNow); };

    D3DXMATRIX* GetTranslationMatrix() { return &m_mTranslation; }
    D3DXMATRIX* GetTranslationDeltaMatrix() { return &m_mTranslationDelta; }
    bool IsBeingDragged() { return m_bDrag; }
    D3DXQUATERNION GetQuatNow() { return m_qNow; }
    void SetQuatNow(D3DXQUATERNION q) { m_qNow = q; }
    static D3DXQUATERNION QuatFromBallPoints(const D3DXVECTOR3& vFrom, const D3DXVECTOR3& vTo);

protected:
    D3DXMATRIXA16 m_mRotation;          // Matrix for arc ball's orientation
    D3DXMATRIXA16 m_mTranslation;       // Matrix for arc ball's position
    D3DXMATRIXA16 m_mTranslationDelta;  // Matrix for arc ball's position
    INT m_nWidth;                       // arc ball's window width
    INT m_nHeight;                      // arc ball's window height
    POINT m_vCenter;                    // center of arc ball
    FLOAT m_fRadius;                    // arc ball's radius in screen coords
    FLOAT m_fRadiusTranslation;         // arc ball's radius for translating the target
    D3DXQUATERNION m_qDown;             // Quaternion before button down
    D3DXQUATERNION m_qNow;              // Composite quaternion for current drag
    bool m_bDrag;                       // Whether user is dragging arc ball
    POINT m_ptLastMouse;                // position of last mouse point
    D3DXVECTOR3 m_vDownPt;              // starting point of rotation arc
    D3DXVECTOR3 m_vCurrentPt;           // current point of rotation arc
    D3DXVECTOR3 ScreenToVector(float fScreenPtX, float fScreenPtY);
};

void MoveSelection(float elapsedTime, HWND hWnd, int msg, WPARAM wParam, LPARAM lParam)
{
    static ArcBall g_SelectionArcBall;
    static std::vector<D3DXMATRIX> g_SelectionArcBallMatrixList;
    static D3DXVECTOR3 g_Pivot;
    static D3DXMATRIX g_PivotMatrix, g_PivotMatrixInv, g_ArcBallMatrix;
    static bool g_SelectionHasMoved;
    static bool g_SelectionIsRotated;
    static D3DXVECTOR3 g_RotationAxis;
    static LPARAM g_LastMouse;
    static float g_MouseSpeed;
    static float g_Angle;

    if (msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN) {
        if (msg == WM_LBUTTONDOWN)
            g_SelectionIsRotated = false;
        if (!g_SelectionIsRotated) {
            g_Pivot = D3DXVECTOR3(0, 0, 0);
            g_SelectionArcBallMatrixList.clear();
            for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin();
                o != Scene::g_ObjectList.end(); ++o)
                if ((*o)->IsSelected()) {
                    const D3DXMATRIX& world = (*o)->World();
                    g_SelectionArcBallMatrixList.push_back(world);
                    g_Pivot += D3DXVECTOR3(world(3, 0), world(3, 1), world(3, 2));
                }
            if (g_SelectionArcBallMatrixList.size() == 0)
                return;
            g_Pivot /= static_cast<float>(g_SelectionArcBallMatrixList.size());
        }
        g_SelectionArcBall.Reset();
        g_SelectionArcBall.SetWindow(g_BackBufferWidth, g_BackBufferHeight, 1);
        g_SelectionArcBall.SetTranslationRadius(1);
        int x, y;
        WorldToScreen(g_Pivot, x, y);
        g_SelectionArcBall.SetCenter(x, y);
        D3DXMATRIX pivotTranslation;
        D3DXMatrixTranslation(&pivotTranslation, -g_Pivot.x, -g_Pivot.y, -g_Pivot.z);
        D3DXMATRIX pivotRotation = g_View;
        pivotRotation(3, 0) = pivotRotation(3, 1) = pivotRotation(3, 2) = 0;
        g_PivotMatrix = pivotTranslation * pivotRotation;
        D3DXMatrixInverse(&g_PivotMatrixInv, 0, &g_PivotMatrix);
        g_LastMouse = lParam;
        g_SelectionIsMoved = true;
        g_SelectionHasMoved = false;
        Scene::SetSelectionFree(false);
    }
    if (hWnd)
        g_SelectionArcBall.HandleMessages(hWnd, msg, wParam, lParam);
    if (msg == WM_MOUSEMOVE) {
        if (g_SelectionIsMoved) {
            g_ArcBallMatrix = g_PivotMatrix **g_SelectionArcBall.GetRotationMatrix()
                    **g_SelectionArcBall.GetTranslationMatrix() *
                g_PivotMatrixInv;
            if (lParam != g_LastMouse)
                g_SelectionHasMoved = true;
            if (!g_SelectionIsRotated) {
                std::vector<D3DXMATRIX>::const_iterator m = g_SelectionArcBallMatrixList.begin();
                for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin();
                    o != Scene::g_ObjectList.end(); ++o)
                    if ((*o)->IsSelected()) {
                        D3DXMATRIX world = *m * g_ArcBallMatrix;
                        (*o)->SetWorld(world);
                        ++m;
                    }
                int dx = (short)LOWORD(lParam) - (short)LOWORD(g_LastMouse);
                int dy = (short)HIWORD(lParam) - (short)HIWORD(g_LastMouse);
                g_MouseSpeed = max(sqrtf (static_cast<float>(dx*dx) +static_cast<float>(dy*dy)) -1, 0);
                g_LastMouse = lParam;
            }
        }
    }
    if (elapsedTime) {
        if (elapsedTime < 0)
            g_SelectionIsRotated = false;
        if (g_SelectionIsRotated) {
            g_Angle += g_MouseSpeed * elapsedTime;
            D3DXMATRIX pivotTranslation;
            D3DXMatrixTranslation(&pivotTranslation, -g_Pivot.x, -g_Pivot.y, -g_Pivot.z);
            D3DXMATRIX pivotTranslationInv;
            D3DXMatrixTranslation(&pivotTranslationInv, g_Pivot.x, g_Pivot.y, g_Pivot.z);
            D3DXMATRIX rotation;
            D3DXMatrixRotationAxis(&rotation, &g_RotationAxis, g_Angle);
            rotation = pivotTranslation * rotation * pivotTranslationInv;
            std::vector<D3DXMATRIX>::const_iterator m = g_SelectionArcBallMatrixList.begin();
            for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin();
                 o != Scene::g_ObjectList.end(); ++o)
                if ((*o)->IsSelected()) {
                    D3DXMATRIX world = *m * rotation * g_ArcBallMatrix;
                    (*o)->SetWorld(world);
                    ++m;
                }
        }
    }
    if (msg == WM_LBUTTONUP || msg == WM_MBUTTONUP || msg == WM_RBUTTONUP) {
        if (g_SelectionIsMoved) {
            if (g_SelectionHasMoved) {
                if (g_SelectionIsRotated) {
                    D3DXVec3TransformCoord(&g_Pivot, &g_Pivot, &g_ArcBallMatrix);
                    for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin();
                        o != Scene::g_ObjectList.end(); ++o)
                        if ((*o)->IsSelected()) {
                            g_SelectionArcBallMatrixList.push_back(g_SelectionArcBallMatrixList.front() * g_ArcBallMatrix);
                            g_SelectionArcBallMatrixList.erase(g_SelectionArcBallMatrixList.begin());
                        }
                }
                else {
                    g_SelectionIsRotated = g_Menu[MENU_EDIT].GetCheckBox(EDIT_KEEP_ROTATING)->GetChecked();
                    D3DXQUATERNION q;
                    D3DXQuaternionRotationMatrix(&q, &g_ArcBallMatrix);
                    float angle;
                    D3DXQuaternionToAxisAngle(&q, &g_RotationAxis, &angle);
                    if (D3DXVec3Length(&g_RotationAxis) < 1e-7f) {
                        g_SelectionIsRotated = false;
                        g_MouseSpeed = 0;
                    }
                    g_Angle = 0;
                    g_SelectionArcBallMatrixList.clear();
                    for (std::list < Scene::Object * >::const_iterator o = Scene::g_ObjectList.begin();
                        o != Scene::g_ObjectList.end(); ++o)
                        if ((*o)->IsSelected()) {
                            const D3DXMATRIX& world = (*o)->World();
                            g_SelectionArcBallMatrixList.push_back(world);
                        }
                }
                D3DXMatrixIdentity(&g_ArcBallMatrix);
            }
            g_SelectionIsMoved = false;
            Scene::SetSelectionFree(g_Menu[MENU_SIMULATE].GetCheckBox(SIMULATE_CLOTH_SELECTION_FREE)->GetChecked());
        }
    }
}

ArcBall::ArcBall()
{
    Reset();

    m_vDownPt = D3DXVECTOR3(0, 0, 0);
    m_vCurrentPt = D3DXVECTOR3(0, 0, 0);

    RECT rc;
    GetClientRect(GetForegroundWindow(), &rc);
    SetWindow(rc.right, rc.bottom);
}

void ArcBall::Reset()
{
    D3DXQuaternionIdentity(&m_qDown);
    D3DXQuaternionIdentity(&m_qNow);
    D3DXMatrixIdentity(&m_mRotation);
    D3DXMatrixIdentity(&m_mTranslation);
    D3DXMatrixIdentity(&m_mTranslationDelta);

    m_bDrag = FALSE;
    m_fRadiusTranslation = 1.0f;
    m_fRadius = 1.0f;
}

D3DXVECTOR3 ArcBall::ScreenToVector(float fScreenPtX, float fScreenPtY)
{

    // Scale to screen
    FLOAT x = -(fScreenPtX - m_vCenter.x) / (m_fRadius * m_nWidth / 2);
    FLOAT y = (fScreenPtY - m_vCenter.y) / (m_fRadius * m_nHeight / 2);

    FLOAT z = 0.0f;
    FLOAT mag = x * x + y * y;

    if (mag > 1.0f) {
        FLOAT scale = 1.0f / sqrtf(mag);
        x *= scale;
        y *= scale;
    }
    else
        z = sqrtf(1.0f - mag);

    // Return vector
    return D3DXVECTOR3(x, y, z);
}

D3DXQUATERNION ArcBall::QuatFromBallPoints(const D3DXVECTOR3& vFrom, const D3DXVECTOR3& vTo)
{
    D3DXVECTOR3 vPart;
    float fDot = D3DXVec3Dot(&vFrom, &vTo);
    D3DXVec3Cross(&vPart, &vFrom, &vTo);

    return D3DXQUATERNION(vPart.x, vPart.y, vPart.z, fDot);
}

void ArcBall::OnBegin(int nX, int nY)
{
    m_bDrag = true;
    m_qDown = m_qNow;
    m_vDownPt = ScreenToVector((float)nX, (float)nY);
}

void ArcBall::OnMove(int nX, int nY)
{
    if (m_bDrag) {
        m_vCurrentPt = ScreenToVector((float)nX, (float)nY);
        m_qNow = m_qDown * QuatFromBallPoints(m_vDownPt, m_vCurrentPt);
    }
}

void ArcBall::OnEnd()
{
    m_bDrag = false;
}

LRESULT ArcBall::HandleMessages(HWND hWnd, int msg, WPARAM wParam, LPARAM lParam)
{

    // Current mouse position
    int iMouseX = (short)LOWORD(lParam);
    int iMouseY = (short)HIWORD(lParam);

    switch (msg) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        SetCapture(hWnd);
        OnBegin(iMouseX, iMouseY);
        return TRUE;
    case WM_LBUTTONUP:
        ReleaseCapture();
        OnEnd();
        return TRUE;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
        SetCapture(hWnd);

        // Store off the position of the cursor when the button is
        // pressed
        m_ptLastMouse.x = iMouseX;
        m_ptLastMouse.y = iMouseY;
        return TRUE;
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        ReleaseCapture();
        return TRUE;
    case WM_MOUSEMOVE:
        if (MK_LBUTTON & wParam) {
            OnMove(iMouseX, iMouseY);
        }
        else if ((MK_RBUTTON & wParam) || (MK_MBUTTON & wParam)) {

            // Normalize based on size of window and bounding sphere
            // radius
            FLOAT fDeltaX = (m_ptLastMouse.x - iMouseX) * m_fRadiusTranslation / m_nWidth;
            FLOAT fDeltaY = (m_ptLastMouse.y - iMouseY) * m_fRadiusTranslation / m_nHeight;

            if (wParam & MK_RBUTTON) {
                D3DXMatrixTranslation(&m_mTranslationDelta, -2 * fDeltaX, 2 * fDeltaY, 0.0f);
                D3DXMatrixMultiply(&m_mTranslation, &m_mTranslation, &m_mTranslationDelta);
            }
            else {
                D3DXMatrixTranslation(&m_mTranslationDelta, 0.0f, 0.0f, 5 * fDeltaY);
                D3DXMatrixMultiply(&m_mTranslation, &m_mTranslation, &m_mTranslationDelta);
            }

            // Store mouse coordinate
            m_ptLastMouse.x = iMouseX;
            m_ptLastMouse.y = iMouseY;
        }
        return TRUE;
    }
    return FALSE;
}

/*----------------------------------------------------------------------------------------------------------------------
    Menu button
 ----------------------------------------------------------------------------------------------------------------------- */

CDXUTMenuButton::CDXUTMenuButton(CDXUTDialog* dialog, bool open) :
    CDXUTButton(dialog),
    m_Open(open)
{
    m_Type = DXUT_CONTROL_MENU_BUTTON;
    SetDialogVisible(m_Open);
}

void CDXUTMenuButton::UpdateRects()
{
    CDXUTButton::UpdateRects();

    int height = m_rcBoundingBox.bottom - m_rcBoundingBox.top;
    m_rcText = m_rcBoundingBox;
    m_rcText.right -= height;

    const int margin = 4;
    m_rcButton.left = m_rcBoundingBox.right - height + margin - 1;
    m_rcButton.right = m_rcBoundingBox.right - margin;
    m_rcButton.top = m_rcBoundingBox.top + margin;
    m_rcButton.bottom = m_rcBoundingBox.bottom - margin;
}

BOOL CDXUTMenuButton::ContainsPoint(POINT pt)
{
    if (m_bPressed)
        return true;
    else
        return PtInRect(&m_rcBoundingBox, pt);
}

void CDXUTMenuButton::Render(IDirect3DDevice9* pd3dDevice, float fElapsedTime)
{
    DXUT_CONTROL_STATE iState = DXUT_STATE_NORMAL;
    int nOffsetX = 0;
    int nOffsetY = 0;
    if (!m_bVisible)
        iState = DXUT_STATE_HIDDEN;
    else if (!m_bEnabled)
        iState = DXUT_STATE_DISABLED;
    else if (m_bPressed) {
        iState = DXUT_STATE_PRESSED;
        nOffsetX = 1;
        nOffsetY = 2;
    }
    else if (m_bMouseOver) {
        iState = DXUT_STATE_MOUSEOVER;
        nOffsetX = -1;
        nOffsetY = -2;
    }
    else if (m_bHasFocus)
        iState = DXUT_STATE_FOCUS;
    float fBlendRate = (iState == DXUT_STATE_PRESSED) ? 0.0f : 0.8f;
    CDXUTElement* pElement = m_Elements.GetAt(0);
    pElement->TextureColor.Blend(iState, fElapsedTime, fBlendRate);
    pElement->FontColor.Blend(iState, fElapsedTime, fBlendRate);

    RECT rcWindow = m_rcText;
    OffsetRect(&rcWindow, nOffsetX, nOffsetY);
    m_pDialog->DrawSprite(pElement, &rcWindow,0.7); //depth add by myselef
    m_pDialog->DrawText(m_strText, pElement, &rcWindow);

    pElement = m_Elements.GetAt(m_Open ? 2 : 3);
    pElement->TextureColor.Blend(iState, fElapsedTime, fBlendRate);

    rcWindow = m_rcButton;
    OffsetRect(&rcWindow, nOffsetX, nOffsetY);
    m_pDialog->DrawSprite(pElement, &rcWindow,0.7);
}

bool CDXUTMenuButton::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!m_bEnabled || !m_bVisible)
        return false;
    switch (uMsg) {
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_SPACE:
            m_bPressed = true;
            return true;
        }
    case WM_KEYUP:
        switch (wParam) {
        case VK_SPACE:
            if (m_bPressed == true) {
                m_bPressed = false;
                SetOpen(!m_Open);
            }
            return true;
        }
    }
    return false;
}

bool CDXUTMenuButton::HandleMouse(UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
{
    if (!m_bEnabled || !m_bVisible)
        return false;
    POINT absoluteMouse;
    absoluteMouse.x = (short)LOWORD(lParam);
    absoluteMouse.y = (short)HIWORD(lParam);
    switch (uMsg) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        if (ContainsPoint(pt)) {
            m_bPressed = true;
            SetCapture(DXUTGetHWND());
            if (!m_bHasFocus && m_pDialog->m_bKeyboardInput)
                m_pDialog->RequestFocus(this);
            m_LastMouse = absoluteMouse;
            m_pDialog->GetLocation(m_LastLocation);
            m_HasMoved = false;
            return true;
        }
        break;
    case WM_MOUSEMOVE:
        if (m_bPressed) {
            POINT move;
            move.x = absoluteMouse.x - m_LastMouse.x;
            move.y = absoluteMouse.y - m_LastMouse.y;
            m_pDialog->SetLocation(m_LastLocation.x + move.x, m_LastLocation.y + move.y);
            static_cast<CDXUTMenu*>(m_pDialog)->ResetItems();
            if (move.x || move.y)
                m_HasMoved = true;
        }
        break;
    case WM_LBUTTONUP:
        if (m_bPressed) {
            m_bPressed = false;
            ReleaseCapture();
            if (!m_HasMoved)
                SetOpen(!m_Open);
            return true;
        }
        break;
    }
    return false;
}

void CDXUTMenuButton::SetOpen(bool value)
{
    m_Open = value;
    SetDialogVisible(m_Open);
}

void CDXUTMenuButton::SetDialogVisible(bool value)
{
    for (int i = 1;; ++i) {
        CDXUTControl* control = m_pDialog->GetControl(i);
        if (control) {
            if (control->GetUserData() == 0 || !value)
                control->SetVisible(value);
        }
        else
            break;
    }
}

CDXUTBoldStatic::CDXUTBoldStatic(CDXUTDialog* dialog) :
    CDXUTStatic(dialog)
{
    m_Type = DXUT_CONTROL_BOLD_STATIC;
}

/*----------------------------------------------------------------------------------------------------------------------
    Menu
 ----------------------------------------------------------------------------------------------------------------------- */

void CDXUTMenu::Initialize(LPCWSTR title, int x, int y, int screenWidth, int screenHeight,
                           PCALLBACKDXUTGUIEVENT callback, bool isOpen)
{
    SetLocation(x, y);
    SetSize(screenWidth, screenHeight);

    SetCallback(callback);
    EnableNonUserEvents(true);

    SetFont(1, L"Arial", 14, FW_BOLD);
    SetFont(2, L"Arial", 15, FW_BOLD);

    GetDefaultElement(DXUT_CONTROL_STATIC, 0)->dwTextFormat = DT_LEFT | DT_BOTTOM;

    SetDefaultElement(DXUT_CONTROL_MENU_BUTTON, 0, GetDefaultElement(DXUT_CONTROL_COMBOBOX, 0));
    SetDefaultElement(DXUT_CONTROL_MENU_BUTTON, 1, GetDefaultElement(DXUT_CONTROL_COMBOBOX, 0));
    SetDefaultElement(DXUT_CONTROL_MENU_BUTTON, 2, GetDefaultElement(DXUT_CONTROL_SCROLLBAR, 1));
    SetDefaultElement(DXUT_CONTROL_MENU_BUTTON, 3, GetDefaultElement(DXUT_CONTROL_SCROLLBAR, 2));
    GetDefaultElement(DXUT_CONTROL_MENU_BUTTON, 0)->iFont = 2;

    CDXUTElement element;
    element.SetFont(1);
    element.dwTextFormat = DT_LEFT | DT_BOTTOM;
    SetDefaultElement(DXUT_CONTROL_BOLD_STATIC, 0, &element);
    AddMenuButton(title, isOpen);
}

void CDXUTMenu::Reset(int screenWidth, int screenHeight)
{
    POINT currentLocation;
    GetLocation(currentLocation);
    POINT center;
    center.x = currentLocation.x + m_ItemWidth / 2;
    center.y = currentLocation.y + m_ItemHeight / 2;
    POINT newLocation = currentLocation;
    if (GetHeight() - static_cast<int>(m_ItemHeight) < center.y)
        newLocation.y = screenHeight - (GetHeight() - currentLocation.y);
    else if (GetWidth() - static_cast<int>(m_ItemWidth) < center.x)
        newLocation.x = screenWidth - (GetWidth() - currentLocation.x);
    else if ((m_ItemWidth < center.x) && (m_ItemHeight < center.y)) {
        newLocation.x = static_cast<int>(screenWidth * currentLocation.x / static_cast<float>(GetWidth()));
        newLocation.y = static_cast<int>(screenHeight * currentLocation.y / static_cast<float>(GetHeight()));
    }
    SetLocation(newLocation.x, newLocation.y);
    SetSize(screenWidth, screenHeight);
    ResetItems();
    CDXUTMenuButton* menuButton = static_cast<CDXUTMenuButton*>(GetControl(0));
    menuButton->SetDialogVisible(menuButton->IsOpen());
}

void CDXUTMenu::ResetItems()
{
    POINT currentLocation;
    GetLocation(currentLocation);
    int numControls;
    int numVisibleControls;
    for (numControls = 0, numVisibleControls = 0;; ++numControls) {
        CDXUTControl* control = GetControl(numControls);
        if (control) {
            if (control->GetUserData() == 0)
                ++numVisibleControls;
        }
        else
            break;
    }
    int verticalSpacing = m_ItemHeight + 2;
    int iStart = 1;
    int yStart = verticalSpacing;
    int inc = 1;
    if (currentLocation.y + numVisibleControls * m_ItemHeight > static_cast<int>(GetHeight())) {
        iStart = numControls - 1;
        yStart = 0;
        inc = -inc;
        verticalSpacing = -verticalSpacing;
    }
    CDXUTControl* control = GetControl(0);
    control->SetSize(m_ItemWidth, m_ItemHeight);
    control->SetLocation(0, 0);
    for (int i = iStart, y = yStart;; i += inc) {
        control = GetControl(i);
        if (control && i > 0) {
            if (control->GetUserData() == 0) {
                int spacing = verticalSpacing;
                control->SetSize(m_ItemWidth, m_ItemHeight);
                if (inc < 0)
                    y += spacing;
                control->SetLocation(0, y);
                if (inc > 0)
                    y += spacing;
                if (control->GetType() == DXUT_CONTROL_COMBOBOX) {
                    CDXUTComboBox* comboBox = static_cast<CDXUTComboBox*>(control);
                    comboBox->SetDropHeight(comboBox->GetNumItems() * 15);
                }
            }
        }
        else
            break;
    }
}

void CDXUTMenu::SetVisible(bool visible)
{
    CDXUTMenuButton* menuButton = static_cast<CDXUTMenuButton*>(GetControl(0));
    menuButton->SetVisible(visible);
    menuButton->SetDialogVisible(visible ? menuButton->IsOpen() : false);
}

void CDXUTMenu::AddMenuButton(LPCWSTR strText, bool isOpen, int nHotkey, bool bIsDefault)
{
    CDXUTMenuButton* menuButton = new CDXUTMenuButton(this, isOpen);
    menuButton->SetID(0);
    menuButton->SetText(strText);
    menuButton->SetHotkey(nHotkey);
    menuButton->m_bIsDefault = bIsDefault;
    AddControl(menuButton);
}

void CDXUTMenu::AddBoldStatic(int ID, LPCWSTR strText)
{
    CDXUTBoldStatic* boldStatic = new CDXUTBoldStatic(this);
    boldStatic->SetID(ID);
    boldStatic->SetText(strText);
    AddControl(boldStatic);
}

/*----------------------------------------------------------------------------------------------------------------------
    Toggle from windowed to fullscreen mode
 ----------------------------------------------------------------------------------------------------------------------- */

HRESULT ToggleFullScreen()
{
    HRESULT hr;

    DXUTPause(true, true);

    // Get the current device settings and flip the windowed state then
    // find the closest valid device settings with this change
    DXUTDeviceSettings deviceSettings = DXUTGetDeviceSettings();

    // A few adjustment to avoid triple buffering and vsync on.
    deviceSettings.d3d9.pp.Windowed = !deviceSettings.d3d9.pp.Windowed;
    deviceSettings.d3d9.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    deviceSettings.d3d9.pp.BackBufferCount = 1;

    DXUTMatchOptions matchOptions;
    matchOptions.eAdapterOrdinal = DXUTMT_PRESERVE_INPUT;
    matchOptions.eDeviceType = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eWindowed = DXUTMT_PRESERVE_INPUT;
    matchOptions.eAdapterFormat = DXUTMT_IGNORE_INPUT;
    matchOptions.eVertexProcessing = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eBackBufferFormat = DXUTMT_IGNORE_INPUT;
    matchOptions.eBackBufferCount = DXUTMT_PRESERVE_INPUT;
    matchOptions.eMultiSample = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eSwapEffect = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eDepthFormat = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eStencilFormat = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.ePresentFlags = DXUTMT_CLOSEST_TO_INPUT;
    matchOptions.eRefreshRate = DXUTMT_IGNORE_INPUT;
    matchOptions.ePresentInterval = DXUTMT_PRESERVE_INPUT;

    RECT rcWindowClient;
    if (deviceSettings.d3d9.pp.Windowed) {

        // Going to windowed mode
        rcWindowClient = DXUTGetWindowClientRect(); 
        int nWidth  = rcWindowClient.right  - rcWindowClient.left;
        int nHeight = rcWindowClient.bottom - rcWindowClient.top;
        if (nWidth > 0 && nHeight > 0) {
            matchOptions.eResolution = DXUTMT_CLOSEST_TO_INPUT;
            deviceSettings.d3d9.pp.BackBufferWidth  = nWidth;
            deviceSettings.d3d9.pp.BackBufferHeight = nHeight;
        }
        else
            matchOptions.eResolution = DXUTMT_IGNORE_INPUT;
    }
    else
        // Hack to force DXUT to fetch the desktop resolution
        matchOptions.eResolution = DXUTMT_IGNORE_INPUT; // rcWindowClient.right = rcWindowClient.left = rcWindowClient.bottom = rcWindowClient.top = 0;

    // Now let DXUT to do its thing
    hr = DXUTFindValidDeviceSettings(&deviceSettings, &deviceSettings, &matchOptions);
    if (SUCCEEDED(hr)) 
        // Create a Direct3D device using the new device settings.  
        // If there is an existing device, then it will either reset or recreate the scene.
        hr = DXUTCreateDeviceFromSettings( &deviceSettings, false );

    DXUTPause(false, false);

    return hr;
}

}
