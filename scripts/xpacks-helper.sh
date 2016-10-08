#!/bin/bash
#set -euo pipefail
#IFS=$'\n\t'

# -----------------------------------------------------------------------------
# Bash helper script used in project generate.sh scripts.
# -----------------------------------------------------------------------------

do_add_micro_os_plus_iii_cortexm_xpack() {
  local pack_name='micro-os-plus-iii-cortexm'
  do_tell_xpack "${pack_name}-xpack"

  do_select_pack_folder "ilg/${pack_name}.git"

  # Exception, folder with diferent name;
  # Package to be renamed.
  do_prepare_dest "${pack_name}/include"
  do_add_content "${pack_folder}/include"/* 

  do_prepare_dest "${pack_name}/src"
  do_add_content "${pack_folder}/src"/* 
}

# -----------------------------------------------------------------------------
