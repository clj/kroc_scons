@SET_MAKE@

AUTOMAKE_OPTIONS = foreign no-dependencies
ACLOCAL_AMFLAGS = -I ../../../m4

OCCBUILD = @OCCBUILD@

EXTRA_DIST = \
	g3dcubes.occ \
	g3dcube.occ \
	g3dtest.occ

if BUILD_GRAPHICS3D_EXAMPLES
examples = \
	g3dcubes \
	g3dcube \
	g3dtest
endif

all-local: $(examples)

g3dtest: g3dtest.occ
	$(OCCBUILD) --program g3dtest.occ

g3dcubes: g3dcubes.occ
	$(OCCBUILD) --program g3dcubes.occ

g3dcube: g3dcube.occ
	$(OCCBUILD) --program g3dcube.occ

install-exec-local: $(examples)
	$(OCCBUILD) --install-examples graphics3d --prefix "$(prefix)" --destdir "$(DESTDIR)" $(examples)

clean-local:
	$(OCCBUILD) --clean $(examples)

SUFFIXES = .occ .tce .lib

.occ.tce:
	$(OCCBUILD) --object $<
