
import codecs
from setuptools import setup, Extension, find_packages

m1 = Extension(
    'mrworkserver.internals',
     sources = [ 
      './src/mrworkserver/internals/dec.c',
      './src/mrworkserver/internals/module.c',
      './src/mrworkserver/internals/protocol.c',
     ],
     include_dirs = ['./src/mrworkserver/internals'],
     extra_compile_args = ['-msse4.2', '-mavx2', '-mbmi2', '-Wunused-variable','-std=c99','-Wno-discarded-qualifiers', '-Wno-unused-variable','-Wno-unused-function'],
     extra_link_args = [],
     #extra_link_args = ['-lasan'],
     define_macros = [('DEBUG_PRINT',1)]
)

with codecs.open('README.md', encoding='utf-8') as f:
    README = f.read()


setup(
  name="mrworkserver", 
  version="0.2",
  license='MIT',
  description='A python work server written in C',
  long_description = README,
  long_description_content_type='text/markdown',
  ext_modules = [m1],
  package_dir={'':'src'},
  packages=find_packages('src'),# + ['prof'],
  #package_data={'prof': ['prof.so']},
  install_requires=[
    #'uvloop<0.9.0',
    'uvloop>0.9.0',
  ],
  platforms='x86_64 Linux and MacOS X',
  url='http://github.com/MarkReedZ/mrworkserver/',
  author='Mark Reed',
  author_email='markreed99@gmail.com',
  keywords=['web', 'asyncio'],
  classifiers=[
    'Development Status :: 2 - Pre-Alpha',
    'Intended Audience :: Developers',
    'Environment :: Web Environment',
    'License :: OSI Approved :: MIT License',
    'Operating System :: MacOS :: MacOS X',
    'Operating System :: POSIX :: Linux',
    'Programming Language :: C',
    'Programming Language :: Python :: 3.5',
    'Programming Language :: Python :: 3.6',
    'Programming Language :: Python :: Implementation :: CPython',
    'Topic :: Internet :: WWW/HTTP'
   ]
)

