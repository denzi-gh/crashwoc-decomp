cmake_minimum_required(VERSION 3.11)
project(OpenCrashWOC LANGUAGES C CXX)

message(FATAL_ERROR
  "this repo uses decomp-toolkit + ninja, not cmake.\n"
  "use:\n"
  "  py -3 configure.py --version GCBE7D --toolchain prodg35\n"
  "  ninja progress\n"
)
