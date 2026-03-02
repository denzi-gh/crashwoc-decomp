#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.normalize_splits import (
    sync_generated_splits_back,
    write_generated_config,
    write_normalized_splits,
)
from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "GCBE7D",  # 0
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--toolchain",
    choices=["mw", "prodg35"],
    default="prodg35",
    help="compiler/linker profile (default: prodg35)",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--ninja",
    metavar="BINARY",
    type=Path,
    help="path to ninja binary (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--warn",
    dest="warn",
    type=str,
    choices=["all", "off", "error"],
    help="how to handle warnings",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()


def _needs_regen(output_path: Path, *input_paths: Path) -> bool:
    if not output_path.exists():
        return True
    output_mtime = output_path.stat().st_mtime_ns
    for input_path in input_paths:
        if input_path.exists() and input_path.stat().st_mtime_ns > output_mtime:
            return True
    return False

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.ninja_path = args.ninja
config.progress = args.progress
config.objdiff_build_target = True
config.objdiff_build_base = True
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-1"
config.compilers_tag = "20251118"
config.dtk_tag = "v1.8.0"
config.objdiff_tag = "v3.5.1"
config.sjiswrap_tag = "v1.2.2"
config.wibo_tag = "1.0.0"

# Project
source_config_path = Path("config") / config.version / "config.yml"
source_splits_path = Path("config") / config.version / "splits.txt"
generated_config_path = Path("config") / config.version / "config.generated.yml"
generated_splits_path = Path("config") / config.version / "splits.generated.txt"
normalize_tool_path = Path("tools") / "normalize_splits.py"
if generated_splits_path.exists() and source_splits_path.exists():
    if generated_splits_path.stat().st_mtime_ns > source_splits_path.stat().st_mtime_ns:
        sync_generated_splits_back(source_splits_path, generated_splits_path)
if _needs_regen(generated_splits_path, source_splits_path, normalize_tool_path):
    write_normalized_splits(source_splits_path, generated_splits_path)
if _needs_regen(generated_config_path, source_config_path, generated_splits_path, normalize_tool_path):
    write_generated_config(source_config_path, generated_config_path, generated_splits_path)

config.config_path = generated_config_path
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym BUILD_VERSION={version_num}",
]
if args.toolchain == "prodg35":
    # ProDG ngcld does not accept Metrowerks linker flags like -fp/-nodefaults.
    config.ldflags = []
else:
    config.ldflags = [
        "-fp hardware",
        "-nodefaults",
    ]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    if args.toolchain != "prodg35":
        config.ldflags.append("-mapunused")
        # config.ldflags.append("-listclosure") # For Wii linkers

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = [
    source_config_path,
    source_splits_path,
    normalize_tool_path,
]

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = None

project_root = Path(__file__).resolve().parent

# Project include roots needed by the recovered source tree.
project_include_dirs = [
    project_root / "src",
    project_root / "src" / "gamecode",
    project_root / "src" / "gamelib",
    project_root / "src" / "nu3dx",
    project_root / "src" / "nucore",
    project_root / "src" / "numath",
    project_root / "src" / "nuraster",
    project_root / "src" / "nusound",
    project_root / "src" / "system",
    project_root / "src" / "system" / "gc",
    project_root / "src" / "system" / "gs",
    project_root / "src" / "system" / "ss",
]
project_include_dir = project_root / "include"
project_build_include_dir = project_root / "build" / config.version / "include"

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
if args.toolchain == "prodg35":
    # From AutoDecomp_WOC/flags.json: "-O2 -mps-float -g2"
    cflags_base = [
        "-O2",
        "-mps-float",
        "-g2",
        *[f"-I {d.as_posix()}" for d in project_include_dirs],
        f"-I {project_include_dir.as_posix()}",
        f"-I {project_build_include_dir.as_posix()}",
        f"-DBUILD_VERSION={version_num}",
        f"-DVERSION_{config.version}",
    ]
else:
    cflags_base = [
        "-nodefaults",
        "-proc gekko",
        "-align powerpc",
        "-enum int",
        "-fp hardware",
        "-Cpp_exceptions off",
        # "-W all",
        "-O4,p",
        "-inline auto",
        '-pragma "cats off"',
        '-pragma "warn_notinlined off"',
        "-maxerrors 1",
        "-RTTI off",
        "-fp_contract on",
        "-str reuse",
        "-multibyte",  # For Wii compilers, replace with `-enc SJIS`
        *[f"-i {d.as_posix()}" for d in project_include_dirs],
        f"-i {project_include_dir.as_posix()}",
        f"-i {project_build_include_dir.as_posix()}",
        f"-DBUILD_VERSION={version_num}",
        f"-DVERSION_{config.version}",
    ]

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Warning flags
if args.warn == "all":
    cflags_base.append("-W all")
elif args.warn == "off":
    cflags_base.append("-W off")
elif args.warn == "error":
    cflags_base.append("-W error")

if args.toolchain == "prodg35":
    cflags_runtime = [*cflags_base]
    cflags_rel = [*cflags_base]
else:
    # Metrowerks library flags
    cflags_runtime = [
        *cflags_base,
        "-use_lmw_stmw on",
        "-str reuse,pool,readonly",
        "-gccinc",
        "-common off",
        "-inline auto",
    ]

    # REL flags
    cflags_rel = [
        *cflags_base,
        "-sdata 0",
        "-sdata2 0",
    ]

if args.toolchain == "prodg35":
    config.linker_version = "ProDG/3.5"
    config.prodg_ldscript = Path("config") / config.version / "ldscript.ld"
    # Used by the ProDG linker-script adapter in tools/project.py.
    # Values can be overridden later once better symbols/map data is available.
    config.prodg_sda_base = 0x803DA260
    config.prodg_sda2_base = 0x803EA260
    config.prodg_stack_end = 0x803D4BD4
else:
    config.linker_version = "GC/1.3.2"


# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": objects,
    }


# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_rel,
        "progress_category": "gameplay",
        "objects": objects,
    }


Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions


config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    {
        "lib": "CrashWOC_Gameplay",
        "mw_version": config.linker_version,
        "cflags": cflags_base,
        "progress_category": "gameplay",
        "objects": [
          Object(NonMatching, "ai.c" , source="gamecode/ai.c"),
          Object(NonMatching, "bug.c" , source="gamecode/bug.c"),
          Object(NonMatching, "camera.c" , source="gamecode/camera.c"),
          Object(NonMatching, "chase.c" , source="gamecode/chase.c"),
          Object(NonMatching, "cloudfx.c" , source="gamecode/cloudfx.c"),
          Object(NonMatching, "crate.c" , source="gamecode/crate.c"),
          Object(NonMatching, "creature.c" , source="gamecode/creature.c"),
          Object(NonMatching, "credits.c" , source="gamecode/credits.c"),
          Object(NonMatching, "cut.c" , source="gamecode/cut.c"),
          Object(NonMatching, "deb3.c" , source="gamecode/deb3.c"),
          Object(NonMatching, "font3d.c" , source="gamecode/font3d.c"),
          Object(NonMatching, "game.c" , source="gamecode/game.c"),
          Object(NonMatching, "game_deb.c" , source="gamecode/game_deb.c"),
          Object(NonMatching, "game_obj.c" , source="gamecode/game_obj.c"),
          Object(NonMatching, "inst.c" , source="gamecode/inst.c"),
          Object(NonMatching, "jeep.c" , source="gamecode/jeep.c"),
          Object(NonMatching, "lights.c" , source="gamecode/lights.c"),
          Object(NonMatching, "listman.c" , source="gamecode/listman.c"),
          Object(NonMatching, "loadsave.c" , source="gamecode/loadsave.c"),
          Object(NonMatching, "main.c" , source="gamecode/main.c"),
          Object(NonMatching, "move.c" , source="gamecode/move.c"),
          Object(NonMatching, "panel.c" , source="gamecode/panel.c"),
          Object(NonMatching, "text.c" , source="gamecode/text.c"),
          Object(NonMatching, "vehicle.c" , source="gamecode/vehicle.c"),
          Object(NonMatching, "vehsupp.c" , source="gamecode/vehsupp.c"),
          Object(NonMatching, "vehterr.c" , source="gamecode/vehterr.c"),
          Object(NonMatching, "visi.c" , source="gamecode/visi.c"),
          Object(NonMatching, "visidat.c" , source="gamecode/visidat.c"),
          Object(NonMatching, "debris.c" , source="gamelib/debris.c"),
          Object(NonMatching, "edanim.c" , source="gamelib/edanim.c"),
          Object(NonMatching, "edbits.c" , source="gamelib/edbits.c"),
          Object(NonMatching, "edfile.c" , source="gamelib/edfile.c"),
          Object(NonMatching, "edgra.c" , source="gamelib/edgra.c"),
          Object(NonMatching, "edobj.c" , source="gamelib/edobj.c"),
          Object(NonMatching, "edptl.c" , source="gamelib/edptl.c"),
          Object(NonMatching, "gcutscn.c" , source="gamelib/gcutscn.c"),
          Object(NonMatching, "glutils.c" , source="gamelib/glutils.c"),
          Object(NonMatching, "nubridge.c" , source="gamelib/nubridge.c"),
          Object(NonMatching, "nuwind.c" , source="gamelib/nuwind.c"),
          Object(NonMatching, "terrain.c" , source="gamelib/terrain.c", extra_cflags=["-O0"]),
        ]
    },
    {
        "lib": "CrashWOC_Engine3D",
        "mw_version": config.linker_version,
        "cflags": cflags_base,
        "progress_category": "engine_3d",
        "objects": [
            Object(NonMatching, "nuanim.c", source="nu3dx/nuanim.c"),
            Object(NonMatching, "nucamera.c", source="nu3dx/nucamera.c"),
            Object(NonMatching, "nucvtskn.c", source="nu3dx/nucvtskn.c"),
            Object(NonMatching, "nuglass.c", source="nu3dx/nuglass.c"),
            Object(NonMatching, "nugobj.c", source="nu3dx/nugobj.c"),
            Object(NonMatching, "nuhaze.c", source="nu3dx/nuhaze.c"),
            Object(NonMatching, "nuhgobj.c", source="nu3dx/nuhgobj.c"),
            Object(NonMatching, "nulight.c", source="nu3dx/nulight.c"),
            Object(NonMatching, "numtl.c", source="nu3dx/numtl.c"),
            Object(NonMatching, "nurndr.c", source="nu3dx/nurndr.c"),
            Object(NonMatching, "nuscene.c", source="nu3dx/nuscene.c"),
            Object(NonMatching, "nutex.c", source="nu3dx/nutex.c"),
            Object(NonMatching, "nutexanm.c", source="nu3dx/nutexanm.c"),
            Object(NonMatching, "nuvport.c", source="nu3dx/nuvport.c"),
            Object(NonMatching, "nuwater.c", source="nu3dx/nuwater.c"),
            Object(NonMatching, "nuerror.c", source="nucore/nuerror.c"),
            Object(NonMatching, "nufile.c", source="nucore/nufile.c"),
            Object(NonMatching, "nufpar.c", source="nucore/nufpar.c"),
            Object(NonMatching, "numem.c", source="nucore/numem.c"),
            Object(NonMatching, "nu_asm.c", source="numath/nu_asm.c"),
            Object(NonMatching, "nufloat.c", source="numath/nufloat.c"),
            Object(NonMatching, "numtx.c", source="numath/numtx.c"),
            Object(NonMatching, "nuplane.c", source="numath/nuplane.c"),
            Object(NonMatching, "nuquat.c", source="numath/nuquat.c"),
            Object(NonMatching, "nurand.c", source="numath/nurand.c"),
            Object(NonMatching, "nutrig.c", source="numath/nutrig.c"),
            Object(NonMatching, "nuvec.c", source="numath/nuvec.c"),
            Object(NonMatching, "nuvec4.c", source="numath/nuvec4.c"),
            Object(NonMatching, "dxframe.c", source="nuraster/dxframe.c"),
        ],
    },
    {
        "lib": "CrashWOC_EngineCore",
        "mw_version": config.linker_version,
        "cflags": cflags_base,
        "progress_category": "engine_core",
        "objects": [
            Object(NonMatching, "NuSound.c", source="nusound/nusound.c"),
            Object(NonMatching, "sfx.c", source="nusound/sfx.c"),
        ],
    },
    {
        "lib": "CrashWOC_PlatformGC",
        "mw_version": config.linker_version,
        "cflags": cflags_base,
        "progress_category": "platform_gc",
        "objects": [
            Object(NonMatching, "crashlib.c", source="system/crashlib.c"),
            Object(NonMatching, "gsmatrix.c", source="system/gsmatrix.c"),
            Object(NonMatching, "memcard.c", source="system/memcard.c"),
            Object(NonMatching, "pointspr.c", source="system/pointspr.c"),
            Object(NonMatching, "port.c", source="system/port.c"),
            Object(NonMatching, "skinning.c", source="system/skinning.c"),
            Object(NonMatching, "syserror.c", source="system/syserror.c"),
            Object(NonMatching, "Version.c", source="system/Version.c"),
            Object(NonMatching, "xform.c", source="system/xform.c"),
            Object(NonMatching, "stats.c", source="system/gc/stats.c"),
            Object(NonMatching, "gcStats.c", source="system/gc/gcStats.c"),
            Object(NonMatching, "fs.c", source="system/gc/fs.c"),
            Object(NonMatching, "pad.c", source="system/gc/pad.c"),
            Object(NonMatching, "syfont.c", source="system/gc/syfont.c"),
            Object(NonMatching, "gs.c", source="system/gs/gs.c"),
            Object(NonMatching, "gsbuffer.c", source="system/gs/gsbuffer.c"),
            Object(NonMatching, "OSFont.c", source="system/gc/OsFont.c"),
            Object(NonMatching, "gcInit.c", source="system/gc/gcInit.c"),
            Object(NonMatching, "gcPuts.c", source="system/gc/gcPuts.c"),
            Object(NonMatching, "gsinit.c", source="system/gs/gsinit.c"),
            Object(NonMatching, "gslight.c", source="system/gs/gslight.c"),
            Object(NonMatching, "gsprim.c", source="system/gs/gsprim.c"),
            Object(NonMatching, "gstex.c", source="system/gs/gstex.c"),
            Object(NonMatching, "ss.c", source="system/ss/ss.c"),
            Object(NonMatching, "ssam.c", source="system/ss/ssam.c"),
            Object(NonMatching, "ssdtk.c", source="system/ss/ssdtk.c"),
            Object(NonMatching, "ssmix.c", source="system/ss/ssmix.c"),
            Object(NonMatching, "sssfx.c", source="system/ss/sssfx.c"),
            Object(NonMatching, "sssp.c", source="system/ss/sssp.c"),
            Object(NonMatching, "sstrack.c", source="system/ss/sstrack.c"),
        ],
    },
    {
        "lib": "CrashWOC_PlatformRuntime",
        "mw_version": config.linker_version,
        "cflags": cflags_base,
        "progress_category": "platform_runtime",
        "objects": [
            Object(NonMatching, "dummy.c", source="runtime/dummy.c"),
            Object(NonMatching, "sbrk.c", source="runtime/sbrk.c"),
            Object(NonMatching, "atoi.c", source="runtime/libc/atoi.c"),
            Object(NonMatching, "exit.c", source="runtime/libc/exit.c"),
            Object(NonMatching, "rand.c", source="runtime/libc/rand.c"),
            Object(NonMatching, "strtol.c", source="runtime/libc/strtol.c"),
            Object(NonMatching, "vfprintf.c", source="runtime/libc/vfprintf.c"),
            Object(NonMatching, "vsprintf.c", source="runtime/libc/vsprintf.c"),
            Object(NonMatching, "memchr.c", source="runtime/libc/memchr.c"),
            Object(NonMatching, "memcmp.c", source="runtime/libc/memcmp.c"),
            Object(NonMatching, "memcpy.c", source="runtime/libc/memcpy.c"),
            Object(NonMatching, "memset.c", source="runtime/libc/memset.c"),
            Object(NonMatching, "strcat.c", source="runtime/libc/strcat.c"),
            Object(NonMatching, "strchr.c", source="runtime/libc/strchr.c"),
            Object(NonMatching, "strcmp.c", source="runtime/libc/strcmp.c"),
            Object(NonMatching, "strcasecmp.c", source="runtime/libc/strcasecmp.c"),
            Object(NonMatching, "strcpy.c", source="runtime/libc/strcpy.c"),
            Object(NonMatching, "strlen.c", source="runtime/libc/strlen.c"),
            Object(NonMatching, "strlwr.c", source="runtime/libc/strlwr.c"),
            Object(NonMatching, "strncmp.c", source="runtime/libc/strncmp.c"),
            Object(NonMatching, "strncasecmp.c", source="runtime/libc/strncasecmp.c"),
            Object(NonMatching, "strncpy.c", source="runtime/libc/strncpy.c"),
            Object(NonMatching, "strstr.c", source="runtime/libc/strstr.c"),
            Object(NonMatching, "mbtowc_r.c", source="runtime/libc/mbtowc_r.c"),
            Object(NonMatching, "tolower.c", source="runtime/libc/tolower.c"),
            Object(NonMatching, "vprintf.c", source="runtime/libc/vprintf.c"),
            Object(NonMatching, "memmove.c", source="runtime/libc/memmove.c"),
            Object(NonMatching, "locale.c", source="runtime/libc/locale.c"),
            Object(NonMatching, "impure.c", source="runtime/libc/impure.c"),
            Object(NonMatching, "ctype_.c", source="runtime/libc/ctype_.c"),
            Object(NonMatching, "printf.c", source="runtime/libc/printf.c"),
            Object(NonMatching, "sprintf.c", source="runtime/libc/sprintf.c"),
            Object(NonMatching, "List.c", source="runtime/libc/List.c"),
            Object(NonMatching, "string.c", source="runtime/libc/string.c"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_e_acos56102144002.s", source="runtime/libm/e_acos.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_e_atan256132251482.s", source="runtime/libm/e_atan2.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_e_fmod5617231292.s", source="runtime/libm/e_fmod.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_e_pow56232246252.s", source="runtime/libm/e_pow.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_e_sqrt5626226062.s", source="runtime/libm/e_sqrt.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_s_atan56432235802.s", source="runtime/libm/s_atan.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_s_cos56432235802.s", source="runtime/libm/s_cos.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_s_fabs5646215602.s", source="runtime/libm/s_fabs.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_s_floor5646215602.s", source="runtime/libm/s_floor.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_s_sin56492123092.s", source="runtime/libm/s_sin.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_s_tan56492123092.s", source="runtime/libm/s_tan.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_ef_sqrt56722220122.s", source="runtime/libm/ef_sqrt.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_s_scalbn5698296952.s", source="runtime/libm/s_scalbn.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_k_cos56102144002.s", source="runtime/libm/k_cos.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_k_sin56102144002.s", source="runtime/libm/k_sin.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_k_tan56102144002.s", source="runtime/libm/k_tan.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_e_rem_pio25626226062.s", source="runtime/libm/e_rem_pio2.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_s_copysign5698296952.s", source="runtime/libm/s_copysign.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_k_rem_pio25607236512.s", source="runtime/libm/k_rem_pio2.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_e_log1056232246252.s", source="runtime/libm/e_log10.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_sf_cos56882102182.s", source="runtime/libm/sf_cos.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_sf_tan56952317142.s", source="runtime/libm/sf_tan.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_e_log56232246252.s", source="runtime/libm/e_log.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_kf_cos56532230572.s", source="runtime/libm/kf_cos.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_kf_sin5656210372.s", source="runtime/libm/kf_sin.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_kf_tan5656210372.s", source="runtime/libm/kf_tan.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_ef_rem_pio256692112632.s", source="runtime/libm/ef_rem_pio2.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_sf_fabs56922209662.s", source="runtime/libm/sf_fabs.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_kf_rem_pio256532230572.s", source="runtime/libm/kf_rem_pio2.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_sf_floor56922209662.s", source="runtime/libm/sf_floor.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_sf_scalbn5708291722.s", source="runtime/libm/sf_scalbn.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_sf_copysign5708291722.s", source="runtime/libm/sf_copysign.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_sf_sin56952317142.s", source="runtime/libm/sf_sin.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_libgcc216200263302.s", source="runtime/libgcc/DOCUME_1_bill_LOCALS_1_Temp_libgcc216200263302.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_libgcc216200263302_1.s", source="runtime/libgcc/DOCUME_1_bill_LOCALS_1_Temp_libgcc216200263302_1.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_libgcc2162042170782_1.s", source="runtime/libgcc/DOCUME_1_bill_LOCALS_1_Temp_libgcc2162042170782_1.s"),
            Object(NonMatching, "libgcc2.s", source="runtime/libgcc/libgcc2.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_libgcc216191268532.s", source="runtime/libgcc/DOCUME_1_bill_LOCALS_1_Temp_libgcc216191268532.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_libgcc216191268532_1.s", source="runtime/libgcc/DOCUME_1_bill_LOCALS_1_Temp_libgcc216191268532_1.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_libgcc2161972283492.s", source="runtime/libgcc/DOCUME_1_bill_LOCALS_1_Temp_libgcc2161972283492.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_libgcc2161972283492_1.s", source="runtime/libgcc/DOCUME_1_bill_LOCALS_1_Temp_libgcc2161972283492_1.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_libgcc216171278982.s", source="runtime/libgcc/DOCUME_1_bill_LOCALS_1_Temp_libgcc216171278982.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_libgcc2162042170782.s", source="runtime/libgcc/DOCUME_1_bill_LOCALS_1_Temp_libgcc2162042170782.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_libgcc2161772293952.s", source="runtime/libgcc/DOCUME_1_bill_LOCALS_1_Temp_libgcc2161772293952.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_libgcc216181273752.s", source="runtime/libgcc/DOCUME_1_bill_LOCALS_1_Temp_libgcc216181273752.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_libgcc2161872288722.s", source="runtime/libgcc/DOCUME_1_bill_LOCALS_1_Temp_libgcc2161872288722.s"),
            Object(NonMatching, "DOCUME_1_bill_LOCALS_1_Temp_libgcc2161872288722_1.s", source="runtime/libgcc/DOCUME_1_bill_LOCALS_1_Temp_libgcc2161872288722_1.s"),
            Object(NonMatching, "GameCube_sn_libSN_crt0.s", source="runtime/sn/GameCube_sn_libSN_crt0.s"),
            Object(NonMatching, "GameCube_sn_libSN_fileserver.s", source="runtime/sn/GameCube_sn_libSN_fileserver.s"),
            Object(NonMatching, "GameCube_sn_libSN_ppcdown.s", source="runtime/sn/GameCube_sn_libSN_ppcdown.s"),
            Object(NonMatching, "GameCube_sn_libSN_tealeaf.s", source="runtime/sn/GameCube_sn_libSN_tealeaf.s"),
        ],
    },
    {
        "lib": "CrashWOC_LegacyPort",
        "mw_version": config.linker_version,
        "cflags": cflags_base,
        "progress_category": "legacy_port",
        "objects": [
            Object(NonMatching, "crash_xbox.c", source="nuxbox/crash_xbox.c"),
            Object(NonMatching, "dummyfunc.c", source="nuxbox/dummyfunc.c"),
            Object(NonMatching, "ps2dma.c", source="nuxbox/ps2dma.c"),
            Object(NonMatching, "gba.c", source="nuxbox/gba.c"),
            Object(NonMatching, "xboxlibs.c", source="system/xboxlibs.c"),
        ],
    },
    {
        "lib": "CrashWOC_SDK",
        "mw_version": config.linker_version,
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": [
            Object(NonMatching, "OS.c", source="system/gc/OS.c"),
            Object(NonMatching, "OSAlarm.c", source="system/gc/OSAlarm.c"),
            Object(NonMatching, "OSAlloc.c", source="system/gc/OSAlloc.c"),
            Object(NonMatching, "OSContext.c", source="system/gc/OSContext.c"),
            Object(NonMatching, "OSInterrupt.c", source="system/gc/OSInterrupt.c"),
            Object(NonMatching, "OSMemory.c", source="system/gc/OSMemory.c"),
            Object(NonMatching, "OSReset.c", source="system/gc/OSReset.c"),
            Object(NonMatching, "OSRtc.c", source="system/gc/OSRtc.c"),
            Object(NonMatching, "OSSync.c", source="system/gc/OSSync.c"),
            Object(NonMatching, "OSThread.c", source="system/gc/OSThread.c"),
            Object(NonMatching, "OSTime.c", source="system/gc/OSTime.c"),
            Object(NonMatching, "OSReboot.c", source="system/gc/OSReboot.c"),
            Object(NonMatching, "OSAudioSystem.c", source="system/gc/OSAudioSystem.c"),
            Object(NonMatching, "OSCache.c", source="system/gc/OSCache.c"),
            Object(NonMatching, "OSError.c", source="system/gc/OSError.c"),
            Object(NonMatching, "OSLink.c", source="system/gc/OSLink.c"),
            Object(NonMatching, "OSArena.c", source="system/gc/OSArena.c"),
            Object(NonMatching, "OSResetSW.c", source="system/gc/OSResetSW.c"),
            Object(NonMatching, "OSMutex.c", source="system/gc/OSMutex.c"),
            Object(NonMatching, "OSFont_data.c", source="system/gc/OSFont_data.c"),
            Object(NonMatching, "dvdfs.c", source="system/gc/dvdfs.c"),
            Object(NonMatching, "dvd.c", source="system/gc/dvd.c"),
            Object(NonMatching, "dvderror.c", source="system/gc/dvderror.c"),
            Object(NonMatching, "fstload.c", source="system/gc/fstload.c"),
            Object(NonMatching, "dvdlow.c", source="system/gc/dvdlow.c"),
            Object(NonMatching, "dvdqueue.c", source="system/gc/dvdqueue.c"),
            Object(NonMatching, "fileCache.c", source="system/gc/fileCache.c"),
            Object(NonMatching, "vi.c", source="system/gc/vi.c"),
            Object(NonMatching, "Padclamp.c", source="system/gc/Padclamp.c"),
            Object(NonMatching, "Pad.c", source="system/gc/Pad.c"),
            Object(NonMatching, "ai_1.c", source="system/gc/ai.c"),
            Object(NonMatching, "ar.c", source="system/gc/ar.c"),
            Object(NonMatching, "arq.c", source="system/gc/arq.c"),
            Object(NonMatching, "EXIBios.c", source="system/gc/EXIBios.c"),
            Object(NonMatching, "EXIUart.c", source="system/gc/EXIUart.c"),
            Object(NonMatching, "SIBios.c", source="system/gc/SIBios.c"),
            Object(NonMatching, "SISamplingRate.c", source="system/gc/SISamplingRate.c"),
            Object(NonMatching, "GBA_1.c", source="system/gc/GBA.c"),
            Object(NonMatching, "GBAJoyBoot.c", source="system/gc/GBAJoyBoot.c"),
            Object(NonMatching, "GBARead.c", source="system/gc/GBARead.c"),
            Object(NonMatching, "GBAWrite.c", source="system/gc/GBAWrite.c"),
            Object(NonMatching, "GBAXfer.c", source="system/gc/GBAXfer.c"),
            Object(NonMatching, "GBAKey.c", source="system/gc/GBAKey.c"),
            Object(NonMatching, "GBAGetProcessStatus.c", source="system/gc/GBAGetProcessStatus.c"),
            Object(NonMatching, "CARDBios.c", source="system/gc/CARDBios.c"),
            Object(NonMatching, "CARDBlock.c", source="system/gc/CARDBlock.c"),
            Object(NonMatching, "CARDDir.c", source="system/gc/CARDDir.c"),
            Object(NonMatching, "CARDCheck.c", source="system/gc/CARDCheck.c"),
            Object(NonMatching, "CARDMount.c", source="system/gc/CARDMount.c"),
            Object(NonMatching, "CARDFormat.c", source="system/gc/CARDFormat.c"),
            Object(NonMatching, "CARDCreate.c", source="system/gc/CARDCreate.c"),
            Object(NonMatching, "CARDRead.c", source="system/gc/CARDRead.c"),
            Object(NonMatching, "CARDWrite.c", source="system/gc/CARDWrite.c"),
            Object(NonMatching, "CARDDelete.c", source="system/gc/CARDDelete.c"),
            Object(NonMatching, "CARDStat.c", source="system/gc/CARDStat.c"),
            Object(NonMatching, "CARDUnlock.c", source="system/gc/CARDUnlock.c"),
            Object(NonMatching, "CARDRdwr.c", source="system/gc/CARDRdwr.c"),
            Object(NonMatching, "CARDOpen.c", source="system/gc/CARDOpen.c"),
            Object(NonMatching, "CARDNet.c", source="system/gc/CARDNet.c"),
            Object(NonMatching, "CARDStatEx.c", source="system/gc/CARDStatEx.c"),
            Object(NonMatching, "AXOut.c", source="system/gc/AXOut.c"),
            Object(NonMatching, "AXVPB.c", source="system/gc/AXVPB.c"),
            Object(NonMatching, "AXAlloc.c", source="system/gc/AXAlloc.c"),
            Object(NonMatching, "AXAux.c", source="system/gc/AXAux.c"),
            Object(NonMatching, "AXCL.c", source="system/gc/AXCL.c"),
            Object(NonMatching, "AXSPB.c", source="system/gc/AXSPB.c"),
            Object(NonMatching, "AXProf.c", source="system/gc/AXProf.c"),
            Object(NonMatching, "AX.c", source="system/gc/AX.c"),
            Object(NonMatching, "dsp.c", source="system/gc/dsp.c"),
            Object(NonMatching, "dsp_task.c", source="system/gc/dsp_task.c"),
            Object(NonMatching, "DSPCode.c", source="system/gc/DSPCode.c"),
            Object(NonMatching, "dsp_debug.c", source="system/gc/dsp_debug.c"),
            Object(NonMatching, "GXInit.c", source="system/gc/GXInit.c"),
            Object(NonMatching, "GXFifo.c", source="system/gc/GXFifo.c"),
            Object(NonMatching, "GXAttr.c", source="system/gc/GXAttr.c"),
            Object(NonMatching, "GXMisc.c", source="system/gc/GXMisc.c"),
            Object(NonMatching, "GXTexture.c", source="system/gc/GXTexture.c"),
            Object(NonMatching, "GXTransform.c", source="system/gc/GXTransform.c"),
            Object(NonMatching, "GXLight.c", source="system/gc/GXLight.c"),
            Object(NonMatching, "GXTev.c", source="system/gc/GXTev.c"),
            Object(NonMatching, "GXPixel.c", source="system/gc/GXPixel.c"),
            Object(NonMatching, "GXPerf.c", source="system/gc/GXPerf.c"),
            Object(NonMatching, "GXFrameBuf.c", source="system/gc/GXFrameBuf.c"),
            Object(NonMatching, "GXBump.c", source="system/gc/GXBump.c"),
            Object(NonMatching, "GXGeometry.c", source="system/gc/GXGeometry.c"),
            Object(NonMatching, "GXStubs.c", source="system/gc/GXStubs.c"),
            Object(NonMatching, "texPalette.c", source="system/gc/texPalette.c"),
            Object(NonMatching, "DebuggerDriver.c", source="system/gc/DebuggerDriver.c"),
            Object(NonMatching, "db.c", source="system/gc/db.c"),
            Object(NonMatching, "mtx44.c", source="system/gc/mtx44.c"),
            Object(NonMatching, "mtx.c", source="system/gc/mtx.c"),
            Object(NonMatching, "vec.c", source="system/gc/vec.c"),
            Object(NonMatching, "PPCArch.c", source="system/gc/PPCArch.c"),
            Object(NonMatching, "AmcExi2Stubs.c", source="system/gc/AmcExi2Stubs.c"),
        ],
    },
    {
        "lib": "Runtime.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "progress_category": "sdk",  # str | List[str]
        "objects": [
            Object(NonMatching, "Runtime.PPCEABI.H/global_destructor_chain.c"),
            Object(NonMatching, "Runtime.PPCEABI.H/__init_cpp_exceptions.cpp"),
        ],
    },
]


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    if not config.non_matching:
        return objects
    if module_id == 0:  # DOL
        return objects + ["dummy.c"]
    return objects


# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("gameplay", "Gameplay"),
    ProgressCategory("engine_3d", "Engine 3D"),
    ProgressCategory("engine_core", "Engine Core"),
    ProgressCategory("platform_gc", "Platform GC"),
    ProgressCategory("platform_runtime", "Platform Runtime"),
    ProgressCategory("legacy_port", "Legacy Port"),
    ProgressCategory("sdk", "SDK Code"),
]
config.progress_each_module = args.verbose
# Optional extra arguments to `objdiff-cli report generate`
config.progress_report_args = [
    # Marks relocations as mismatching if the target value is different
    # Default is "functionRelocDiffs=none", which is most lenient
    # "--config functionRelocDiffs=data_value",
]

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress information
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
