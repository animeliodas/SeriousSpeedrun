"""Build the player's download: release/SeriousSpeedrun-<version>.zip with Install.cmd, both games' DLLs and a readme.

    python tools/make_release.py        (after plugin/build.sh)
"""
import hashlib
import re
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAIN = (ROOT / "plugin" / "Source" / "Main.cpp").read_text(encoding="utf-8")
VERSION = ".".join(re.search(r"MakeVersion\((\d+), (\d+), (\d+)\)", MAIN).groups())

README = f"""SeriousSpeedrun {VERSION} - speedrun timer (RTA, IGT), LiveSplit and a speedometer for Serious Sam Classic
https://github.com/animeliodas/SeriousSpeedrun

It doesn't change the game: for runs of the game as it is, and alongside other mods.

Install: double-click Install.cmd (finds TFE/TSE in Steam or GOG, or drop the game's folder on it; adds Serious Sam
Classics Patch 1.9.4 if it isn't there, SHA-256 checked; puts SeriousSpeedrun.dll into Bin\\Plugins).

Timer (top right): RTA - the PC's clock from the start; IGT - the game's ticks (no level loads, pauses, menus or
Netricsa). Starts on the first control after a new game, splits at every level's end, ends with the game; a new game
resets it.
LiveSplit: right-click it -> Control -> Start TCP Server (port 16834). The game starts, splits, ends and resets it, and
keeps its Game Time equal to the IGT (Compare Against -> Game Time to run by IGT).
Speedometer under the crosshair (m/s; sr_iSpeedUnits 1: Half-Life units).
Console: sr_bShowTimer, sr_bShowSpeed, sr_iSpeedUnits, sr_bLiveSplit, sr_strLiveSplit, sr_iStartOn, sr_Status().

Made with Claude Code (AI); tested by scripts in the real games. GPL-2.0.
"""


def main() -> None:
    out = ROOT / "release" / f"SeriousSpeedrun-{VERSION}.zip"
    out.parent.mkdir(exist_ok=True)
    files = {
        "Install.cmd": ROOT / "install" / "Install.cmd",
        "Uninstall.cmd": ROOT / "install" / "Uninstall.cmd",
        "install.ps1": ROOT / "install" / "install.ps1",
        "TFE/SeriousSpeedrun.dll": ROOT / "plugin" / "dist" / "Release_TFE105" / "SeriousSpeedrun.dll",
        "TSE/SeriousSpeedrun.dll": ROOT / "plugin" / "dist" / "Release_TSE107" / "SeriousSpeedrun.dll",
    }
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
        for name, path in files.items():
            z.write(path, f"SeriousSpeedrun/{name}")
        z.writestr("SeriousSpeedrun/README.txt", README.replace("\n", "\r\n"))
    print(f"{out} ({out.stat().st_size} bytes)\nsha256 {hashlib.sha256(out.read_bytes()).hexdigest()}")
    for game in ("TFE", "TSE"):
        dll = files[f"{game}/SeriousSpeedrun.dll"]
        print(f"  {game} SeriousSpeedrun.dll sha256 {hashlib.sha256(dll.read_bytes()).hexdigest()}")


if __name__ == "__main__":
    main()
