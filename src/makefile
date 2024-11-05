#
# Template makefile for modules with many targets
# TARGETS = names of sub-modules (1 directory for each module)
# Ex:
# TARGETS = spv_stat stat_viewer s_collector
#
# Directory containing libraries first
#

TARGETS = libdebug liblog libmos librouter libsm \
shell_x28 sm_mgr x25_argotel x25_call_mgr x25_caller x25_easyway \
x25_incall x25_listener x25_shmgr x25_start libsm_test asyn_mgr \
asyn_x28 x25_spv 

SHELL = /bin/sh

compile debug install depend clean run_install tag all installd run_installd::
			for i in $(TARGETS) ; do cd $$i ; echo $$i; make $@ ; cd .. ; done
