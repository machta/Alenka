from distutils.core import setup, Extension

import numpy.distutils.misc_util as mu
import platform

if platform.system() == 'Linux' :
     LIBS = ['biosig']
elif platform.system() == 'Windows' :
     LIBS = ['biosig','stdc++','iconv','z','cholmod']
else:
     LIBS = ['biosig','stdc++','iconv','z','cholmod']

module1 = Extension('biosig',
                    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '8')],
                    include_dirs = ['./..','/usr/local/include',mu.get_numpy_include_dirs()[0]],
                    libraries = LIBS,
                    library_dirs = ['./..','/usr/local/lib'],
                    sources = ['biosigmodule.c'])

setup (name = 'Biosig',
       version = '1.8',
       description = 'This is a biosig package',
       author = 'Alois Schloegl',
       author_email = 'alois.schloegl@ist.ac.at',
       license = 'GPL',
       url = 'http://docs.python.org/extending/building',
       long_description = '''This is the biosig demo package.''',
       ext_modules = [module1])

