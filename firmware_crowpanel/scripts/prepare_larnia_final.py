#!/usr/bin/env python3
from __future__ import annotations
import shutil, sys, tempfile, urllib.request, zipfile
from pathlib import Path

REPO_ZIP_URL = "https://github.com/Elecrow-RD/CrowPanel-Advanced-7inch-ESP32-P4-HMI-AI-Display-1024x600-IPS-Touch-Screen/archive/refs/heads/master.zip"
OFFICIAL_EXAMPLE_PATH = Path("example/V1.0/Arduino_Code/Lesson09-LVGL_Lighting_Control")
PROJECT_NAME = "Larnia_Figma_Final"

def copytree_overwrite(src: Path, dst: Path) -> None:
    for item in src.iterdir():
        target = dst / item.name
        if item.is_dir():
            target.mkdir(parents=True, exist_ok=True)
            copytree_overwrite(item, target)
        else:
            shutil.copy2(item, target)

def main() -> int:
    script_dir = Path(__file__).resolve().parent
    root = script_dir.parent.parent
    overlay = root / "firmware_crowpanel" / "overlay" / PROJECT_NAME
    sdcard = root / "sdcard"
    out_root = root / "generated"
    out_project = out_root / PROJECT_NAME
    out_root.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        zpath = tmp / "elecrow.zip"
        print("Sťahujem oficiálny Elecrow základ pre 7\" ESP32-P4...")
        urllib.request.urlretrieve(REPO_ZIP_URL, zpath)
        with zipfile.ZipFile(zpath) as z: z.extractall(tmp)
        repo = next(p for p in tmp.iterdir() if p.is_dir())
        official = repo / OFFICIAL_EXAMPLE_PATH
        if not official.exists():
            print(f"Nenašiel som Elecrow Lesson09: {official}", file=sys.stderr)
            return 1
        if out_project.exists(): shutil.rmtree(out_project)
        shutil.copytree(official, out_project)
        old = out_project / "Lesson09-LVGL_Lighting_Control.ino"
        if old.exists(): old.rename(out_project / "Elecrow_Lesson09_original.ino.disabled")
        copytree_overwrite(overlay, out_project)
        if (out_root / "sdcard").exists(): shutil.rmtree(out_root / "sdcard")
        shutil.copytree(sdcard, out_root / "sdcard")
        print("HOTOVO")
        print(f"Arduino projekt: {out_project}")
        print(f"Otvor v Arduino IDE: {out_project / (PROJECT_NAME + '.ino')}")
        print(f"SD karta: skopíruj obsah priečinka {out_root / 'sdcard'} na FAT32 SD kartu.")
    return 0
if __name__ == "__main__": raise SystemExit(main())
