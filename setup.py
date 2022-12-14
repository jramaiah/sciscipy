#!/usr/bin/env python

from distutils.core import setup, Extension, Command
from unittest import TextTestRunner, TestLoader
from glob import glob
from os.path import splitext, basename, join as pjoin
import os, sys

# This should be customized for specific instals

def get_scilab_folder():
	"""
	@return: scilab root folder as defined by the SCI env variable 
	eg: C:\Program Files\scilab-5.4.1
	"""
	if os.getenv("SCI") is None:
		raise RuntimeError("YOU MUST SET A 'SCI' ENVIRONMENT VARIABLE TO POINT TO SCILAB ROOT FOLDER")
		
	return os.getenv("SCI")

if os.name == 'nt':

	#common_include_base = r"C:\Program Files\scilab-5.4.1\modules"
	sci_root = get_scilab_folder()
	common_include_base = os.path.join(sci_root, "modules")
	sci_include = [
			os.path.join(common_include_base, "core", "includes"),
			os.path.join(common_include_base, "call_scilab", "includes"),
			os.path.join(common_include_base, "api_scilab", "includes"),
			os.path.join(common_include_base, "output_stream", "includes"),
			   ]

	sci_lib_dir =  [os.path.join(sci_root, "bin")]
	sci_librairies = ['core', 'api_scilab', 'call_scilab', 'scilab_windows', 'dynamic_link', 'io', 'fileio', 'output_stream']
	sci_extra_link_args = []
        
elif os.name == 'posix':
	common_include_base = os.path.join("/", "usr", "include", "scilab")
	sci_include = [ common_include_base,
			os.path.join(common_include_base, "core"),
			os.path.join(common_include_base, "call_scilab")
			  ]
	sci_lib_dir = [os.path.join("/","usr", "lib", "scilab")]
	sci_librairies = []
	sci_extra_link_args = ['-Wl,--no-as-needed',  '-lscilab',  '-lsciapi_scilab', '-lscicall_scilab', '-lscioutput_stream',  '-lscicore', '-lscilinear_algebra', '-lsciconsole', '-lscilocalization', '-lscipolynomials', '-lsciio', '-lscielementary_functions', '-lscisparse', '-lscihistory_manager', '-lscihistory_browser', '-lscigraphics', '-lscicompletion', '-lscifunctions', '-lsciboolean', '-lsciwindows_tools', '-lscitime', '-lscifftw', '-lsciintersci', '-lscidouble', '-lscicommons']
else:
	raise NotImplementedError, "Only 'nt' and 'posix' are supported"

sci_sources = ['sciscipy.c', 'sciconv_read.c', 'sciconv_write.c', 'util.c']

if os.environ.get('SCI'):
	common_include_base_call=os.path.join("/",os.environ.get('SCI'),"..","..","include","scilab")
	sci_include.append(os.path.join("/", common_include_base_call))
	sci_include.append(os.path.join("/", common_include_base_call, "core"))
	sci_include.append(os.path.join("/",common_include_base_call, "call_scilab"))
	sci_lib_dir.append(os.path.join("/",os.environ.get('SCI'),"..","..","lib","scilab"))


sci_install = os.path.join("/", "usr", "local", "share", "scilab")

list_of_macros = [('SCI', '"' + sci_install + '"'), ('SCIDEBUG', 0)]

# Test for python version
if sys.version_info[0] >= 3:
    list_of_macros += [('PYTHON3', 1)]

# Test for numpy
try:
    import numpy
    import numpy.distutils.misc_util as misc
    sci_include += os.path.join(misc.get_numpy_include_dirs())
    numpy_is_avail = 1
    sci_sources += ['deallocator.c']
except ImportError:
    numpy_is_avail = 0

list_of_macros += [('NUMPY', numpy_is_avail)]


class TestCommand(Command):
    user_options = [ ]

    def initialize_options(self):
        self._dir = os.getcwd()

    def finalize_options(self):
        pass

    def run(self):
        '''
        Finds all the tests modules in test/, and runs them.
        '''
        testfiles = [ ]
        for t in glob(pjoin(self._dir, 'tests', '*.py')):
            if not t.endswith('__init__.py'):
                testfiles.append('.'.join(
                    ['tests', splitext(basename(t))[0]])
                )

        print(testfiles)
        tests = TestLoader().loadTestsFromNames(testfiles)
        t = TextTestRunner(verbosity = 2)
        t.run(tests)

module1 = Extension('sciscipy',
			sources = sci_sources,
			include_dirs = sci_include,
			libraries = sci_librairies,
			library_dirs = sci_lib_dir,
			extra_link_args = sci_extra_link_args,
			define_macros = list_of_macros
)

long_description = r"""
The goal of sciscipy is to give an access to Scilab features inside python.

from scilab import Scilab
sci = Scilab()

x = sci.rand(20, 20)
y = x*x.transpose()
y_inv = sci.inv(y)

The function func in sci.func(x, y) can be a Scilab built-in or any user
defined function so that Scilab libraries can be reused easily in python.
"""
setup (	name = 'sciscipy',
       	version = '1.0.0',
		author = 'Vincent Guffens <vincent.guffens@gmail.com>, Sylvestre Ledru <sylvestre.ledru@scilab-enterprises.com>',
		url = "http://forge.scilab.org/index.php/p/sciscipy/",
		license = "GPL",
       	description = 'Scilab binding',
		long_description = long_description,
       	ext_modules = [module1],
        py_modules = ['scilab'],
		data_files = [(os.path.join(get_scilab_folder(), 'etc/sciscipy'), ['scilab.cfg'])],
        cmdclass = { 'test': TestCommand}
)
