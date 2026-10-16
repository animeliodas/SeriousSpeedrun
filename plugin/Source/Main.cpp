/* SeriousSpeedrun - a speedrun timer for Serious Sam Classic (TFE 1.05 / TSE 1.07), Classics Patch plugin.
 *
 * The game as it is - it changes nothing in it - with what runners need on top: a timer that counts real time (RTA)
 * and the game's time (IGT: its ticks, without loads, pauses, menus and Netricsa), splits at every level's end,
 * LiveSplit kept in step through its TCP server, and a speedometer. The timer is the SeriousBridge core's
 * (core/sam/Speedrun.h); it works with other plugins too (SeriousHL's Half-Life movement: speed in its units).
 */
#include "StdH.h"
#include "Speedrun.h"
#include "Player.h"
#include "DevHook.h"

#include <stdio.h>

#define SERIOUSSPEEDRUN_VERSION "0.1.0"

CLASSICSPATCH_DEFINE_PLUGIN(k_EPluginFlagGame, MakeVersion(0, 1, 0),
  "SeriousBridge", "SeriousSpeedrun", "Speedrun timer (RTA, IGT), LiveSplit and a speedometer for Serious Sam.");

// ------------------------------------------------------------------
// The speedometer: the player's speed along the ground, under the crosshair
// ------------------------------------------------------------------
static INDEX sr_bShowSpeed = TRUE;
static INDEX sr_iSpeedUnits = 0; // 0: m/s; 1: Half-Life's units (u/s, 1 u = 1.9/64 m, as SeriousHL's)

static void DrawSpeed(CDrawPort *pdp, CEntity *pen) {
  if (!sr_bShowSpeed || pen == NULL || !(pen->en_ulPhysicsFlags & EPF_MOVABLE)) return;
  const CMovableEntity *pmen = (CMovableEntity *)pen;
  const FLOAT3D v = pmen->en_vCurrentTranslationAbsolute;
  FLOAT3D vUp = -pmen->en_vGravityDir;
  if (vUp.Length() < 0.5f) vUp = FLOAT3D(0, 1, 0);
  const FLOAT fSpeed = (v - vUp * (v % vUp)).Length();
  CTString str;
  if (sr_iSpeedUnits == 1) str.PrintF("%d", (INDEX)floorf(fSpeed / (1.9f / 64.0f) + 0.5f));
  else str.PrintF("%.1f", fSpeed);
  const FLOAT fScale = pdp->GetHeight() / 720.0f;
  pdp->SetFont(_pfdConsoleFont);
  pdp->SetTextAspect(1.0f);
  pdp->SetTextScaling(fScale * 1.5f);
  const PIX pixX = pdp->GetWidth() / 2;
  const PIX pixY = pdp->GetHeight() / 2 + (PIX)(60 * fScale);
  pdp->PutTextC(str, pixX + 1, pixY + 1, 0x000000A0);
  // green above a run (Sam's 10 m/s)
  pdp->PutTextC(str, pixX, pixY, fSpeed > 10.5f ? 0x80FF80E0 : 0xFFFFFFC0);
};

// ------------------------------------------------------------------
// Console
// ------------------------------------------------------------------
static void SR_Status(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  CPrintF("[SeriousSpeedrun] %s %s\n", SERIOUSSPEEDRUN_VERSION, speedrun::Status().str_String);
};

// --- test hooks (tools/test_speedrun.py), only with the dev hook on (SERIOUSBRIDGE_DEV=1): they'd let a run be faked
static INDEX sr_iTestFire = 0;

// ETrigger (Engine's base events: code 7), to take a level's way out
class CTriggerEventLocal : public CEntityEvent {
public:
  CEntityPointer penCaused;
  CTriggerEventLocal(void) : CEntityEvent(0x00000007) {};
  CEntityEvent *MakeCopy(void) { return new CTriggerEventLocal(*this); };
};

static void SR_Where(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  devhook::Where();
};

static void SR_CloseNetricsa(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  devhook::CloseNetricsa();
};

// the level's way out (a WorldLink to another world) triggered, as if the player reached it
static void SR_TestLevelExit(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  CWorld *pwo = IWorld::GetWorld();
  CEntity *penPlayer = player::Local();
  if (pwo == NULL || penPlayer == NULL) return;
  FOREACHINDYNAMICCONTAINER(pwo->wo_cenEntities, CEntity, iten) {
    CEntity *pen = &*iten;
    const char *strClass = pen->GetClass()->ec_pdecDLLClass->dec_strName;
    if ((pen->GetFlags() & ENF_DELETED) || (stricmp(strClass, "WorldLink") != 0 && stricmp(strClass, "World link") != 0)) continue;
    CTString *pstrWorld = (CTString *)player::Prop(pen, (214UL << 8) | 2); // WorldLink.es m_strWorld
    if (pstrWorld != NULL && pstrWorld->Length() == 0) continue;
    CTriggerEventLocal ee;
    ee.penCaused = penPlayer;
    pen->SendEvent(ee);
    CPrintF("[SeriousSpeedrun] level exit %u triggered\n", pen->en_ulID);
    return;
  }
  CPrintF("[SeriousSpeedrun] level exit: none\n");
};

// the game's end, as Player.es marks it (m_bEndOfGame)
static void SR_TestGameEnd(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX *pb = (INDEX *)player::Prop(player::Local(), (401UL << 8) | 97);
  if (pb != NULL) *pb = TRUE;
};

// the game paused / going on, as the Pause key does
static void SR_TestPause(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  _pNetwork->TogglePause();
};

// ------------------------------------------------------------------
// Events
// ------------------------------------------------------------------
static void SR_OnStep(void) {
  speedrun::Step(GetGameAPI()->IsGameOn() ? player::Local() : NULL);
};

static void SR_OnFrame(CDrawPort *pdp) {
  devhook::Poll();
  speedrun::Frame();
};

static void SR_OnPostDraw(CDrawPort *pdp) {
  if (!GetGameAPI()->IsGameOn()) return;
  DrawSpeed(pdp, player::Local());
  speedrun::Draw(pdp);
};

static void SR_OnPlayerAction(INDEX iClient, INDEX iPlayer, CPlayerAction &pa, INDEX iResent) {
  // (tests: fire skips a level's intro)
  if (iResent < 0 && sr_iTestFire > 0) {
    sr_iTestFire--;
    pa.pa_ulButtons |= 1UL << 0;
  }
};

static void SR_OnGameStart(void) { speedrun::OnGameStart(); };
static void SR_OnChangeLevel(void) { speedrun::OnChangeLevel(); };
static void SR_OnGameLoad(const CTFileName &fnmSave) { speedrun::OnGameLoad(); };

CLASSICSPATCH_PLUGIN_STARTUP(HIniConfig props, PluginEvents_t &events) {
  events.m_processing->OnStep = &SR_OnStep;
  events.m_processing->OnFrame = &SR_OnFrame;
  events.m_rendering->OnPostDraw = &SR_OnPostDraw;
  events.m_game->OnGameStart = &SR_OnGameStart;
  events.m_game->OnChangeLevel = &SR_OnChangeLevel;
  events.m_game->OnGameLoad = &SR_OnGameLoad;

  speedrun::Init();
  _pShell->DeclareSymbol("persistent user INDEX sr_bShowSpeed;", &sr_bShowSpeed);
  _pShell->DeclareSymbol("persistent user INDEX sr_iSpeedUnits;", &sr_iSpeedUnits);
  ClassicsPlugins()->RegisterMethod(TRUE, "void", "sr_Status", "void", &SR_Status);

  if (devhook::Init("SERIOUSBRIDGE_DEV")) {
    events.m_packet->OnPlayerAction = &SR_OnPlayerAction;
    ClassicsPlugins()->RegisterMethod(TRUE, "void", "sr_Where", "void", &SR_Where);
    ClassicsPlugins()->RegisterMethod(TRUE, "void", "sr_CloseNetricsa", "void", &SR_CloseNetricsa);
    ClassicsPlugins()->RegisterMethod(TRUE, "void", "sr_TestLevelExit", "void", &SR_TestLevelExit);
    ClassicsPlugins()->RegisterMethod(TRUE, "void", "sr_TestGameEnd", "void", &SR_TestGameEnd);
    ClassicsPlugins()->RegisterMethod(TRUE, "void", "sr_TestPause", "void", &SR_TestPause);
    _pShell->DeclareSymbol("user INDEX sr_iTestFire;", &sr_iTestFire);
  }
  CPrintF("^c40c0ff[SeriousSpeedrun]^r %s loaded: timer (RTA, IGT), LiveSplit %s, speedometer\n", SERIOUSSPEEDRUN_VERSION,
    speedrun::LiveSplitConnected() ? "connected" : "(Control -> Start TCP Server)");
};

CLASSICSPATCH_PLUGIN_SHUTDOWN(HIniConfig props) {
  speedrun::Shutdown();
  devhook::Shutdown();
};
