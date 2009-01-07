import os

try:
    os.unlink('.coverage')
except Exception, e:
    print e

# TODO:
#   * The make driver scripts seem to always be called.


# Create a global environment
tools = ['default',
         'subst',
	 'scheme']
env  = Environment(tools=tools,
		   ENV = {'PATH' : os.environ['PATH']})

env['OCCBUILD_TOOLCHAIN'] = 'kroc'
env['OCCAM_TOOLCHAIN'] = env['OCCBUILD_TOOLCHAIN']

# Pretty builds
comstrings = dict(CCCOMSTR              = 'Compiling $TARGET',
                  LINKCOMSTR            = 'Linking $TARGET',
		  ARCOMSTR              = 'Creating archive $TARGET',
		  RANLIBCOMSTR          = 'Indexing archive $TARGET',
		  MZCCOMSTR             = 'Compiling $TARGET',
		  OCCBUILDCOMSTR        = 'Compiling $TARGET',
		  OCCBUILDLIBRARYCOMSTR = 'Linking $TARGET',
		  OCCBUILDPROGRAMCOMSTR = 'Compiling $TARGET',
		  TBCHEADERCOMSTR       = 'Compiling bytecode header $TARGET')
if int(ARGUMENTS.get('VERBOSE', 0)) < 1:
    for k in comstrings:
	env[k] = comstrings[k]


# Export it for use in the SConscripts
Export('env')

# Build mkoccdeps
SConscript('tools/mkoccdeps/SConscript')
SConscript('tools/ilibr/SConscript')

if env['OCCAM_TOOLCHAIN'] == 'tvm':
    SConscript('tools/schemescanner/SConscript')
    SConscript('tools/tinyswig/SConscript')
    SConscript('tools/skroc/SConscript')
    SConscript('tools/slinker/SConscript')
if env['OCCAM_TOOLCHAIN'] == 'kroc':
    SConscript('tools/tranx86/SConscript')
SConscript('tools/occ21/SConscript')
SConscript('tools/kroc/SConscript')

if env['OCCAM_TOOLCHAIN'] == 'tvm':
    SConscript('runtime/libtvm/SConscript')
if env['OCCAM_TOOLCHAIN'] == 'kroc':
    SConscript('runtime/ccsp/SConscript')

# Ensure that things that build with occbuild have triggered the building of all
# the required tools, there might be a better palce and better way to do this
if env['OCCAM_TOOLCHAIN'] == 'tvm':
    env.Depends(env['SKROC'],    env['SCHEMESCANNER'])
    env.Depends(env['SLINKER'],  env['SCHEMESCANNER'])
    env.Depends(env['LIBRARY2'], env['SCHEMESCANNER'])
    env.Depends(env['OCCBUILD'], env['SLINKER'])
    env.Depends(env['OCCBUILD'], env['LIBRARY2'])
    env.Depends(env['OCCBUILD'], env['SKROC'])
if env['OCCAM_TOOLCHAIN'] == 'kroc':    
    env.Depends(env['OCCBUILD'], env['CCSP'])
    env.Depends(env['OCCBUILD'], env['TRANX86'])
    env.Depends(env['OCCBUILD'], env['KROC'])
env.Depends(env['OCCBUILD'], env['ILIBR'])
env.Depends(env['OCCBUILD'], env['OCC21'])
env.Tool('occbuild', occbuild=env['OCCBUILD'])

# Be even more verbose :)
if int(ARGUMENTS.get('VERBOSE', 0)) >= 2:
    env.AppendUnique(OCCBUILDFLAGS='$( --verbose $)')
if int(ARGUMENTS.get('VERBOSE', 0)) >= 3:
    if env['OCCAM_TOOLCHAIN'] == 'tvm':
        env.AppendUnique(OCCBUILDFLAGS='$( --skroc-opts --verbose $)')
    if env['OCCAM_TOOLCHAIN'] == 'kroc':
        env.AppendUnique(OCCBUILDFLAGS='$( --kroc-opts --verbose $)')




# Not sure if this is necessary
if env['OCCAM_TOOLCHAIN'] == 'tvm':
    env.Depends(env['SKROC'], env['TVM_CONFIG_H'])



SConscript('modules/inmoslibs/libsrc/SConscript')
SConscript('modules/course/SConscript')

if env['OCCAM_TOOLCHAIN'] == 'tvm':
    SConscript('tvm/posix/SConscript')

# A bit tedious, and not needed right now.
# SConscript('tools/occamdoc/SConscript')

