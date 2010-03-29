#!/usr/bin/env python

from distutils.core import setup, Extension, Command
from unittest import TextTestRunner, TestLoader
from glob import glob
from os.path import splitext, basename, join as pjoin
import os, sys

# This should be customized for specific instals

if os.name == 'nt':
	common_include_base = r"C:\Program Files (x86)\scilab-5.2.1\modules"
	sci_include = [
			os.path.join(common_include_base, "core", "includes"), 
			os.path.join(common_include_base, "call_scilab", "includes")
                        
		       ]

	
	
	sci_lib_dir =  [r"C:\Program Files (x86)\scilab-5.2.1\bin"]
	sci_librairies = ['LibScilab']

elif os.name == 'posix':
	common_include_base = os.path.join("/","usr", "include", "scilab")
	sci_include = [
			os.path.join(common_include_base, "core"), 
			os.path.join(common_include_base, "call_scilab")
			  ]
	sci_lib_dir = [os.path.join("/","usr", "lib", "scilab")]
	sci_librairies = ['scilab']
else:
	raise NotImplementedError, "Only 'nt' and 'posix' are supported"
	
sci_sources = ['sciscipy.c', 'sciconv_read.c', 'sciconv_write.c', 'util.c']

if os.environ.get('SCI'):
	common_include_base_call=os.path.join("/",os.environ.get('SCI'),"..","..","include","scilab")

	sci_include.append(os.path.join("/", common_include_base_call, "core"))
	sci_include.append(os.path.join("/",common_include_base_call, "call_scilab"))
	sci_lib_dir.append(os.path.join("/",os.environ.get('SCI'),"..","..","lib","scilab"))


sci_install = os.path.join("/", "usr", "local", "share", "scilab")

list_of_macros = [('SCI', '"' + sci_install + '"'), ('SCIDEBUG', 0),]

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
			define_macros = list_of_macros
)

setup (	name = 'sciscipy',
       	version = '0.4.0',
		author = 'Vincent Guffens',
       	author_email = 'vincent.guffens@gmail.com',
		url = "http://forge.scilab.org/index.php/p/sciscipy/",
		license = "GPL",
       	description = 'Scilab binding',
       	ext_modules = [module1],
        py_modules = ['scilab'], 
        cmdclass = { 'test': TestCommand}
)
