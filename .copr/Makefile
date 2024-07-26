THIS_DIR := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

project_dir := $(abspath $(THIS_DIR)/..)

outdir ?= $(project_dir)
name := tdnf
spec_file := $(project_dir)/$(name).spec
version := $(shell cat $(project_dir)/VERSION)
tarball := $(name)-$(version).tar.gz

prereq:
	which git || dnf -y install git

spec:
	@echo THISDIR=$(THIS_DIR)
	sed s/@PROJECT_VERSION@/$(version)/g $(spec_file).in > $(spec_file)

srpm: spec prereq
	cd $(project_dir) && \
		git archive --format=tar.gz -o $(tarball) --prefix=$(name)-$(version)/ HEAD && \
		rpmbuild -D'_srcrpmdir $(outdir)' -D'_sourcedir $(project_dir)' -bs $(spec_file)
